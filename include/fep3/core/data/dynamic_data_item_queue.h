/**
* Declaration of DynamicDataItemQueue.
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
#include <deque>
#include <memory>
#include <mutex>
#include "data_item_queue_base.h"
#include "fep3/components/data_registry/data_registry_intf.h"

namespace fep3
{
namespace core
{
namespace arya
{
namespace detail
{
/**
 * @brief Dynamic Data Item queue
 * This implementation will provide a FIFO queue to read samples and types in the same order it was added.
 * The capacity and size of the data item queue changes dynamically. Therefore no samples are being dropped due to missing
 * queue capacity reasons.
 *
 * @tparam IDataRegistry::IDataSample class for samples
 * @tparam IStreamType class for types
 */
template<class SAMPLE_TYPE = const fep3::arya::IDataSample, class STREAM_TYPE = const IStreamType>
class DynamicDataItemQueue : public DataItemQueueBase<SAMPLE_TYPE, STREAM_TYPE>
{
private:
    /// @cond no_documentation
    using typename DataItemQueueBase<SAMPLE_TYPE, STREAM_TYPE>::DataItem;
    using typename DataItemQueueBase<SAMPLE_TYPE, STREAM_TYPE>::QueueType;
    /// @endcond no_documentation

public:
    /**
     * @brief CTOR
     *
     */
    DynamicDataItemQueue(size_t /*capacity*/) : DataItemQueueBase<SAMPLE_TYPE, STREAM_TYPE>()
    {
    }

    /**
    * @brief DTOR
    *
    */
    virtual ~DynamicDataItemQueue() = default;

    /**
    * @brief pushes a sample data read pointer to the queue
    *
    * @param sample the samples read pointer to push
    * @param time_of_receiving the timestamp at which the sample was received
    * @remark this is threadsafe against push and other pop calls
    */
    void push(const data_read_ptr<SAMPLE_TYPE>& sample, timestamp_t time_of_receiving) override
    {
        std::lock_guard<std::mutex> lock_guard(_mutex);

        _items.emplace_back(sample, time_of_receiving);
    }

    /**
    * @brief pushes a stream type data read pointer to the queue
    *
    * @param type the types read pointer to push
    * @param time_of_receiving the timestamp at which the sample was received
    * @remark this is threadsafe against push and other pop calls
    */
    void pushType(const data_read_ptr<STREAM_TYPE>& type, timestamp_t time_of_receiving) override
    {
        std::lock_guard<std::mutex> lock_guard(_mutex);

        _items.emplace_back(type, time_of_receiving);
    }

    timestamp_t topTime() override
    {
        std::lock_guard<std::mutex> lock_guard(_mutex);
        if (!_items.empty())
        {
            return _items.front().getTime();
        }
        else
        {
            return INVALID_timestamp_t_fep;
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
        std::lock_guard<std::mutex> lock_guard(_mutex);
        if (!_items.empty())
        {
            _items.erase(_items.begin());
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
        std::lock_guard<std::mutex> lock_guard(_mutex);
        if (!_items.empty())
        {
            DataItem& ref = *_items.begin();
            if (ref.getItemType() == DataItem::Type::sample)
            {
                receiver.onReceive(ref.getSample());
            }
            else if (ref.getItemType() == DataItem::Type::type)
            {
                receiver.onReceive(ref.getStreamType());
            }
            _items.erase(_items.begin());

            return true;
        }
        else
        {
            return false;
        }
    }

    /**
     * @brief capacity Returns the capacity, which means the maximum possible size, of the queue.
     * @return size_t the capacity.
     * @remark While in theory the maximum possible size may be reached by the queue,
     * in practice we would most likely face an out-of-memory situation before.
     */
    size_t capacity() const override
    {
        std::lock_guard<std::mutex> lock_guard(_mutex);
        return _items.max_size();
    }

    size_t size() const override
    {
        std::lock_guard<std::mutex> lock_guard(_mutex);
        return _items.size();
    }

    void clear() override
    {
        std::lock_guard<std::mutex> lock_guard(_mutex);
        _items.clear();
    }

    QueueType getQueueType() const override
    {
        return QueueType::dynamic;
    }

    private:
        std::deque<DataItem>        _items;
        mutable std::mutex          _mutex;
};
}
}
}
}
