/**
* Declaration of the Class ISchedulerService.
*
* @file

   @copyright
   @verbatim
   Copyright @ 2019 Audi AG. All rights reserved.
   
       This Source Code Form is subject to the terms of the Mozilla
       Public License, v. 2.0. If a copy of the MPL was not distributed
       with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
   
   If it is not possible or desirable to put the notice in a particular file, then
   You may include the notice in a location (such as a LICENSE file in a
   relevant directory) where a recipient would be likely to look for such a notice.
   
   You may add additional accurate notices of copyright ownership.
   @endverbatim
*
*/

#pragma once

#include <atomic>
#include <cstddef>
#include <memory>
#include <mutex>
#include <vector>
#include <fep3/fep3_errors.h>
#include <fep3/fep3_optional.h>
#include <fep3/fep3_timestamp.h>
#include <fep3/fep3_participant_export.h>
#include "data_item_queue.h"
#include <fep3/components/data_registry/data_registry_intf.h>

namespace fep3
{
namespace core
{
namespace arya
{
namespace detail
{
///@cond no_documentation
struct WrappedDataItemReceiver : public detail::DataItemQueue<>::IDataItemReceiver
{
    IDataRegistry::IDataReceiver& _receiver;
    WrappedDataItemReceiver(IDataRegistry::IDataReceiver& receiver) : _receiver(receiver)
    {}
    void onReceive(const data_read_ptr<const IDataSample>& sample)
    {
        _receiver(sample);
    }
    void onReceive(const data_read_ptr<const IStreamType>& stream_type)
    {
        _receiver(stream_type);
    }
};
///@endcond no_documentation
}

/**
 * @brief A data reader queue implementation
 *
 */
class DataReaderQueue : public IDataRegistry::IDataReceiver,
                        public IDataRegistry::IDataReader
{
public:
    /**
     * @brief Construct a new Data Reader Queue object
     * 
     * @param capa the initial capacity (if 0 it will be dynamic size!)
     */
    explicit DataReaderQueue(size_t capa): _queue(capa)
    {
    }
    /**
     * @brief DTOR
     * 
     */
    ~DataReaderQueue() override
    {
    }

    /**
     * @brief retrieves the current size of the queue.
     * 
     * @return size_t the size in item count.
     */
    size_t size() const override
    {
        return _queue.size();
    }
    /**
     * @brief retrieves the capacity if the queue
     * 
     * @return size_t the capacity
     */
    size_t capacity() const override
    {
        return _queue.capacity();
    }

    /**
     * @brief Receives a stream type item
     *
     * @param type The received stream type
     */ 
    void operator()(const data_read_ptr<const IStreamType>& type) override
    {
        _queue.pushType(type, std::chrono::milliseconds(0));
    }
    /**
     * @brief Receives a data sample item
     *
     * @param sample The received data sample
     */
    void operator()(const data_read_ptr<const IDataSample>& sample) override
    {
        _queue.push(sample, sample->getTime());
    }

    /**
     * @brief Get the Front Time 
     * 
     * @return fep3::Optional<Timestamp> 
     * @retval valid time queue is not empty 
     * @retval invalid time queue is empty 
     */
    ::fep3::Optional<Timestamp> getFrontTime() const override
    {
        return _queue.topTime();
    }

    /**
     * @brief pops the next item (type or sample)
     * 
     * @param receiver the receiver to "send" the items to
     * @return fep3::Result 
     * @retval ERR_NOERROR received successfully
     * @retval ERR_EMPTY queue is empty
     */
    ::fep3::Result pop(IDataRegistry::IDataReceiver& receiver) override
    {
        detail::WrappedDataItemReceiver wrap(receiver);
        return _queue.pop(wrap) ? fep3::Result() : fep3::ERR_EMPTY;
    }
    /**
     * @brief empty the queue
     * 
     */
    void clear()
    {
        _queue.clear();
    }

private:
    mutable detail::DataItemQueue<> _queue;
};

/**
 * @brief Reader Backlog Queue to keep the last items (capacity) until they are read.
 * each read call will emtpty the backlog.
 */
class DataReaderBacklog : public IDataRegistry::IDataReceiver
{
public:
    /**
     * @brief CTOR for a Data Reader Backlog object
     * 
     * @param capa backlog capacity
     * @param init_type Stream Type at init time
     */
    DataReaderBacklog(size_t capa,
                      const IStreamType& init_type)
    {
        if (capa <= 0)
        {
            capa = 1;
        }
        _samples.resize(capa);
        _last_idx = 0;
        _current_size = 0;
        _init_type.reset(new StreamType(init_type));
    }
    /**
     * @brief move CTOR 
     */
    DataReaderBacklog(DataReaderBacklog&&) = default;
    /**
     * @brief move assignment
     * @return the reference copied to
     */
    DataReaderBacklog& operator=(DataReaderBacklog&&) = default;
    /**
     * @brief DTOR
     * 
     */
    ~DataReaderBacklog()
    {
        _last_idx = 0;
        _current_size = 0;
        _samples.clear();
    }

    /**
     * @brief Receives a stream type item
     *
     * @param type The received stream type
     */ 
    void operator()(const data_read_ptr<const IStreamType>& type) override
    {
        std::lock_guard<std::mutex> lock_guard(_mutex);
        _init_type = type;
    }

    /**
     * @brief Receives a data sample item
     *
     * @param sample The received data sample
     */
    void operator()(const data_read_ptr<const IDataSample>& sample) override
    {
        std::lock_guard<std::mutex> lock_guard(_mutex);
        _last_idx++;
        if (_last_idx == _samples.size())
        {
            _last_idx = 0;
        }
        if (_current_size < _samples.size())
        {
            _current_size++;
        }
        _samples[_last_idx] = sample;
    }
    
    /**
     * @brief retrieves the current size of the queue.
     * 
     * @return size_t the size in item count.
     */
    size_t size() const
    {
        return _current_size;
    }
    /**
     * @brief retrieves the capacity if the queue
     * 
     * @return size_t the capacity
     */
    size_t capacity() const
    {
        return _samples.size();
    }

    /**
     * @brief reads one sample if available. if read the size decreases exactly 1.
     * 
     * @return data_read_ptr<const IDataSample> 
     * @retval valid pointer the sample read
     * @retval invalid pointer the queue was empty
     */
    data_read_ptr<const IDataSample> read() const
    {
        std::lock_guard<std::mutex> lock_guard(_mutex);
        return _samples[_last_idx];
    }

    /**
     * @brief reads the current type
     * 
     * @return data_read_ptr<const IStreamType> 
     * @retval valid pointer the streamtype read
     */
    data_read_ptr<const IStreamType> readType() const
    {
        std::lock_guard<std::mutex> lock_guard(_mutex);
        return _init_type;
    }

    /**
     * @brief reads the sample until upper_bound is reached
     * 
     * @param upper_bound time looking for
     * @return data_read_ptr<const IDataSample>  
     * @retval valid pointer the sample read
     * @retval invalid pointer the queue was empty
     */
    data_read_ptr<const IDataSample> readBefore(Timestamp upper_bound) const
    {
        std::lock_guard<std::mutex> lock_guard(_mutex);
        size_t loop_count = size();
        size_t current_idx = _last_idx;
        while (loop_count-- > 0)
        {
            if (_samples[current_idx]->getTime() <= upper_bound)
            {
                return _samples[current_idx];
            }
            else
            {
                //we look backwards (usually the queue is sorted by time, id the samples are received)
                //with continues times //otherwise we have something to do here!
                if (current_idx == 0)
                {
                    current_idx = size() - 1;
                }
                else
                {
                    current_idx--;
                }
            }
        }
        return data_read_ptr<const IDataSample>();
    }

    /**
     * @brief reads the type until upper_bound is reached
     * 
     * upper_bound time looking for
     * @return data_read_ptr<const IStreamType> 
     * @retval valid pointer the streamtype read
     */
    data_read_ptr<const IStreamType> readTypeBefore(Timestamp /*upper_bound*/) const
    {
        //TODO: create a ITEM Queue to receive the right type for the right sample
        std::lock_guard<std::mutex> lock_guard(_mutex);
        return _init_type;
    }

    /**
     * @brief resizes the queue
     * 
     * @param queue_size resized capacity of the queue
     * @return size_t the new capacity
     */
    size_t resize(size_t queue_size)
    {
        if (queue_size <= 0)
        {
            queue_size = 1;
        }
        if (capacity() != queue_size)
        {
            std::lock_guard<std::mutex> lock_guard(_mutex);
            _last_idx = 0;
            _current_size = 0;
            _samples.clear();
            _samples.resize(queue_size);
        }
        return queue_size;
    }

private:
    ///@cond no_documentation
    std::vector<data_read_ptr<const IDataSample>>            _samples;
    data_read_ptr<const IStreamType>                         _init_type;

    volatile size_t                                          _last_idx;
    volatile size_t                                          _current_size;

    mutable std::mutex                                       _mutex;
    ///@endcond no_documentation
};
}
using arya::DataReaderQueue;
using arya::DataReaderBacklog;
}
}
