/**
 * @file
 * Copyright &copy; AUDI AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include "data_item_queue_base.h"

#include <atomic>
#include <mutex>
#include <vector>
#include <memory>

namespace fep3
{
namespace native
{

/**
 * @brief Data Item queue (at the moment a locked queue)
 * This implementation will provide a FIFO queue to read samples and types in the same order it was added.
 * The capacity of the DataItemQueue is fixed. If samples are pushed into the queue while the queue's capacity is reached
 * old samples are dropped.
 *
 * @tparam IDataRegistry::IDataSample class for samples
 * @tparam IStreamType class for types
 */
template<class SAMPLE_TYPE = const IDataSample, class STREAM_TYPE = const IStreamType>
class DataItemQueue: public DataItemQueueBase<SAMPLE_TYPE, STREAM_TYPE>
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
    DataItemQueue(size_t capacity) : DataItemQueueBase<SAMPLE_TYPE, STREAM_TYPE>()
    {
        if (capacity <= 0)
        {
            capacity = 1;
        }
        _items.resize(capacity);
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
     * @remark this is threadsafe against push and other pop calls
     */
    void push(const data_read_ptr<SAMPLE_TYPE>& sample) override
    {
        std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);
        if (_next_write_idx == _items.size())
        {
            _next_write_idx = 0;
        }

        DataItem& ref = _items[_next_write_idx];
        ref.set(sample);

        ++_next_write_idx;
        ++_current_size;

        //if queue is full we need to change read index ... item was dropped
        if (_current_size > capacity())
        {
            if (_next_read_idx == _items.size())
            {
                _next_read_idx = 0;
            }
            _current_size = capacity();
            ++_next_read_idx;
        }
    }
    /**
     * @brief pushes a stream type data read pointer to the queue
     *
     * @param type the types read pointer to push
     * @param time_of_receiving the timestamp at which the sample was received
     * @remark this is threadsafe against push and other pop calls
     */
    void push(const data_read_ptr<STREAM_TYPE>& type) override
    {
        std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);
        if (_next_write_idx == _items.size())
        {
            _next_write_idx = 0;
        }
        DataItem& ref = _items[_next_write_idx];
        ref.set(type);

        ++_next_write_idx;
        ++_current_size;
        //if queue is full we need to change read index ... item was dropped
        if (_current_size > capacity())
        {
            if (_next_read_idx == _items.size())
            {
                _next_read_idx = 0;
            }
            _current_size = capacity();
            ++_next_read_idx;
        }
    }

    Optional<Timestamp> getFrontTime() override
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
                return ref.getSample()->getTime();
            }
            else
            {
                return {};
            }
        }
        else
        {
            return {};
        }
    }

    /**
     * @brief pops the item at the front of the queue
     *
     * @return {nullptr, nullptr} if queue is empty
     * @remark this is threadsafe against push and pop calls
     */
    std::tuple<data_read_ptr<SAMPLE_TYPE>, data_read_ptr<STREAM_TYPE> > pop() override
    {
        data_read_ptr<SAMPLE_TYPE> sample = nullptr;
        data_read_ptr<STREAM_TYPE> stream_type = nullptr;

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
                    sample = std::move(ref.getSample());
                }
                else if (DataItem::Type::type == ref.getItemType())
                {
                    stream_type = std::move(ref.getStreamType());
                }
                ++_next_read_idx;
                --_current_size;
            }
        }

        return std::make_tuple(std::move(sample), std::move(stream_type));
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
        _next_write_idx = 0;
        _next_read_idx = 0;
        _current_size = 0;
    }

    QueueType getQueueType() const override
    {
        return QueueType::fixed;
    }

private:
    std::vector<DataItem> _items;

    volatile size_t _next_write_idx;
    volatile size_t _next_read_idx;
    volatile size_t _current_size;
    mutable std::recursive_mutex _recursive_mutex;
};

} // namespace native
} // namespace fep3
