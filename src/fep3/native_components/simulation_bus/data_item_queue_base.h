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

#include <atomic>
#include <memory>

namespace fep3
{
namespace native
{
/**
 * @brief Data Item queue base
 * Base class for data item queue implementations
 *
 * @tparam IDataSample class for samples
 * @tparam IStreamType class for types
 */
template<class SAMPLE_TYPE = const arya::IDataSample, class STREAM_TYPE = const arya::IStreamType>
class DataItemQueueBase
{
protected:
    /**
     * @brief Internal queue type
     *
     */
    enum class QueueType
    {
        fixed,
        dynamic
    };

    /**
     * @brief Internal data item
     * 
     */
    class DataItem
    {
    public:
        enum class Type
        {
            sample,
            type,
            none
        };

    public:
        DataItem()
        : _item_type(Type::none)
        {
        }

        /**
         * CTOR for a sample data item
         *
         * @param sample The sample to be stored in the data item
         */
        DataItem(const data_read_ptr<SAMPLE_TYPE>& sample)
                : _item_type(Type::sample), _sample(sample)
        {
        }

        /**
         * CTOR for a stream type data item
         *
         * @param stream_type the stream_type to be stored in the data item
         */
        DataItem(const data_read_ptr<STREAM_TYPE>& stream_type)
                : _item_type(Type::type), _stream_type(stream_type)
        {
        }

    public:
        /**
         * @brief Setter for a new data sample
         * Resets the stream type member of the data item and handles the remaining members accordingly
         *
         * @param sample The sample to be stored in the data item
         */
        void set(const data_read_ptr<SAMPLE_TYPE>& sample)
        {
            _sample = sample;
            _stream_type.reset();
            _item_type = Type::sample;
        }

        /**
         * @brief Setter for a new stream_type
         * Resets the sample member of the data item and handles the remaining members accordingly
         *
         * @param stream_type The stream_type to be stored in the data item
         */
        void set(const data_read_ptr<STREAM_TYPE>& stream_type)
        {
            _stream_type = stream_type;
            _sample.reset();
            _item_type = Type::type;
        }

        Type getItemType() const
        {
            return _item_type;
        }

        data_read_ptr<SAMPLE_TYPE> getSample() const
        {
            return _sample;
        }

        data_read_ptr<STREAM_TYPE> getStreamType() const
        {
            return _stream_type;
        }

        void resetSample()
        {
            _sample.reset();
        }

        void resetStreamType()
        {
            _stream_type.reset();
        }

    private:
        Type _item_type;
        data_read_ptr<SAMPLE_TYPE> _sample;
        data_read_ptr<STREAM_TYPE> _stream_type;
    };

    /**
     * @brief Return the type of the queue
     *
     * @return QueueType The type of the queue
     * Either QueueType::fixed or QueueType::dynamic
     */
    virtual QueueType getQueueType() const = 0;

public:
    /**
     * @brief CTOR
     */
    DataItemQueueBase() = default;

    /**
     * @brief DTOR
     *
     */
    virtual ~DataItemQueueBase() = default;

    /**
     * @brief Pushes a sample data read pointer to the queue
     *
     * @param sample The samples read pointer to push
     * @remark This is threadsafe against pop and other push calls
     */
    virtual void push(const data_read_ptr<SAMPLE_TYPE>& sample) = 0;
    /**
     * @brief Pushes a stream type data read pointer to the queue
     *
     * @param type The types read pointer to push
     * @remark This is threadsafe against pop and other push calls
     */
    virtual void push(const data_read_ptr<STREAM_TYPE>& type) = 0;

    /**
     * @brief Returns the timestamp of the oldest available sample of the item queue,
     * which is the sample at the front of the queue
     *
     * @return The sample timestamp
     */
    virtual Optional<Timestamp> getFrontTime() = 0;

    /**
     * @brief Pops the item at the front of the queue
     *
     * @return {nullptr, nullptr} if queue is empty
     * @remark This is threadsafe against push and pop calls
     */
    virtual std::tuple<data_read_ptr<SAMPLE_TYPE>, data_read_ptr<STREAM_TYPE> > pop() = 0;

    /**
     * @brief Return the maximum capacity of the queue
     *
     * @return Maximum capacity of the queue
     */
    virtual size_t capacity() const = 0;

    /**
     * @brief Return the current size of the queue
     *
     * @return Current size of the queue
     */
    virtual size_t size() const = 0;

    /**
     * @brief Remove all elements of the queue
     */
    virtual void clear() = 0;
};

} // namespace native
} // namespace fep3
