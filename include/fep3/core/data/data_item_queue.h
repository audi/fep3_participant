/**
* Declaration of DataItemQueue.
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
#include <mutex>
#include <vector>
#include "data_item_queue_base.h"
#include "fep3/components/data_registry/data_registry_intf.h"

namespace fep3
{
namespace core
{
namespace arya
{
/**
 * @brief namespace API components where the API compatibility is NOT guaranteed.
 *
 */
namespace detail
{
/**
 * @brief Data Item queue (at the moment a locked queue)
 * This implementation will provide a FIFO queue to read samples and types in the same order it was added.
 * The capacity of the DataItemQueue is fixed. If samples are pushed into the queue while the queue's capacity is reached
 * old samples are dropped.
 *
 * @tparam SAMPLE_TYPE IDataSample class for samples
 * @tparam STREAM_TYPE IStreamType class for types
 */
template<class SAMPLE_TYPE = const IDataSample, class STREAM_TYPE = const IStreamType>
class DataItemQueue : public DataItemQueueBase<SAMPLE_TYPE, STREAM_TYPE>
{
private:
    using typename DataItemQueueBase<SAMPLE_TYPE, STREAM_TYPE>::DataItem;
    using typename DataItemQueueBase<SAMPLE_TYPE, STREAM_TYPE>::QueueType;

public:
    /**
    * @brief CTOR
    *
    * @param capacity capacity by item count of the queue (there are sample + streamtype covered)
    */
    DataItemQueue(size_t capacity)
        : DataItemQueueBase<SAMPLE_TYPE, STREAM_TYPE>()
        , _capacity(capacity)
    {
        if (_capacity <= 0)
        {
            _capacity = 1;
        }
        _items.resize(_capacity);
        _next_write_idx = 0;
        _next_read_idx = 0;
        _current_size = 0;
    }

    /**
    * @brief DTOR
    *
    */
    virtual ~DataItemQueue() = default;

    /**
    * @brief pushes a sample data read pointer to the queue
    *
    * @param sample the samples read pointer to push
    * @param time_of_receiving the timestamp at which the sample was received
    * @remark this is threadsafe against push and other pop calls
    */
    void push(const data_read_ptr<SAMPLE_TYPE>& sample, Timestamp time_of_receiving) override
    {
        std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);
        if (_next_write_idx == _items.size())
        {
            _next_write_idx = 0;
        }

        DataItem& ref = _items[_next_write_idx];
        ref.set(sample, time_of_receiving);

        _next_write_idx++;
        _current_size++;

        //if queue is full we need to change read index ... item was dropped
        if (_current_size > capacity())
        {
            if (_next_read_idx == _items.size())
            {
                _next_read_idx = 0;
            }
            _current_size = capacity();
            _next_read_idx++;
        }
    }
    /**
    * @brief pushes a stream type data read pointer to the queue
    *
    * @param type the types read pointer to push
    * @param time_of_receiving the timestamp at which the sample was received
    * @remark this is threadsafe against push and other pop calls
    */
    void pushType(const data_read_ptr<STREAM_TYPE>& type, Timestamp time_of_receiving) override
    {
        std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);
        if (_next_write_idx == _items.size())
        {
            _next_write_idx = 0;
        }
        DataItem& ref = _items[_next_write_idx];
        ref.set(type, time_of_receiving);

        _next_write_idx++;
        _current_size++;
        //if queue is full we need to change read index ... item was dropped
        if (_current_size > capacity())
        {
            if (_next_read_idx == _items.size())
            {
                _next_read_idx = 0;
            }
            _current_size = capacity();
            _next_read_idx++;
        }
    }

    Optional<Timestamp> topTime() override
    {
        std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);
        if (_current_size > 0)
        {
            if (_next_read_idx == _items.size())
            {
                _next_read_idx = 0;
            }
            DataItem& ref = _items[_next_read_idx];
            return ref.getTime();
        }
        else
        {
            return {};
        }
    }

    /**
    * @brief pops an item from the queue
    *
    * @return true if item is popped
    * @return false if the queue is empty
    * @remark this is threadsafe against push and other pop calls
    */
    bool pop() override
    {
        std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);
        if (_current_size > 0)
        {
            if (_next_read_idx == _items.size())
            {
                _next_read_idx = 0;
            }
            DataItem& ref = _items[_next_read_idx];
            ref.resetSample();
            ref.resetStreamType();
            _next_read_idx++;
            _current_size--;
            return true;
        }
        else
        {
            return false;
        }
    }
    /**
    * @brief pops the item from the front of the queue after putting the item to the given \p receiver
    *
    * @param receiver receiver reference where to callback and put the item before the item is popped.
    * @return true if item is popped
    * @return false if the queue is empty
    * @remark this is threadsafe against push and other pop calls
    */
    bool pop(typename DataItemQueueBase<SAMPLE_TYPE, STREAM_TYPE>::IDataItemReceiver& receiver) override
    {
        std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);
        if (_current_size > 0)
        {
            if (_next_read_idx == _items.size())
            {
                _next_read_idx = 0;
            }
            DataItem& ref = _items[_next_read_idx];
            if (DataItem::Type::sample == ref.getItemType())
            {
                receiver.onReceive(ref.getSample());
                ref.resetSample();
            }
            else if (DataItem::Type::type == ref.getItemType())
            {
                receiver.onReceive(ref.getStreamType());
                ref.resetStreamType();
            }
            _next_read_idx++;
            _current_size--;
            return true;
        }
        else
        {
            return false;
        }
    }

    size_t capacity() const override
    {
        std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);
        return _items.size();
    }

    size_t size() const override
    {
        std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);
        return _current_size;
    }

    void clear() override
    {
        std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);
        _items.clear();
        _items.resize(_capacity);
        _next_write_idx = 0;
        _next_read_idx = 0;
        _current_size = 0;
    }

    QueueType getQueueType() const override
    {
        return QueueType::fixed;
    }

private:
    std::vector<DataItem>                       _items;
#ifndef __QNX__
    std::atomic<size_t>                              _capacity;
    std::atomic<size_t>                              _next_write_idx;
    std::atomic<size_t>                              _next_read_idx;
    std::atomic<size_t>                              _current_size;
#else
    std::atomic_size_t                               _capacity;
    std::atomic_size_t                               _next_write_idx;
    std::atomic_size_t                               _next_read_idx;
    std::atomic_size_t                               _current_size;
#endif
    mutable std::recursive_mutex                _recursive_mutex;
};

}
}
}
}
