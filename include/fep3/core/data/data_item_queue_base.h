/**
* Declaration of DataItemQueueBase.
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
#include <memory>
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
 * @brief Data Item queue base
 * Base class for data item queue implementations
 *
 * @tparam IDataRegistry::IDataSample class for samples
 * @tparam IStreamType class for types
 */
template<class SAMPLE_TYPE = const IDataSample, class STREAM_TYPE = const IStreamType>
class DataItemQueueBase
{
protected:
    /**
     * @brief internal queue type
     *
     */
    enum QueueType
    {
        fixed,
        dynamic
    };

    /**
     * @brief internal data item
     *
     */
    class DataItem
    {
    public:
        /**
         * content type of the data item
         */
        enum Type
        {
            ///is a sample
            sample,
            ///is a streamtype
            type
        };

    public:
        /**
         * CTOR
         */
        DataItem() = default;

        /**
         * CTOR for a sample data item
         *
         * @param sample the sample to be stored in the data item
         * @param time the timestamp of the data item
         */
        DataItem(const data_read_ptr<SAMPLE_TYPE>& sample, Timestamp time)
            : _item_type(Type::sample),
            _sample(sample),
            _time(time)
        {
        }

        /**
        * CTOR for a stream type data item
        *
        * @param stream_type the stream_type to be stored in the data item
        * @param time the timestamp of the data item
        */
        DataItem(const data_read_ptr<STREAM_TYPE>& stream_type, Timestamp time)
            : _item_type(Type::type),
            _stream_type(stream_type),
            _time(time)
        {
        }

    public:
        /**
        * @brief Setter for a new data sample
        * Resets the stream type member of the data item and handles the remaining members accordingly
        *
        * @param sample the sample to be stored in the data item
        * @param time the timestamp of the data item
        */
        void set(const data_read_ptr<SAMPLE_TYPE>& sample, Timestamp time)
        {
            _sample = sample;
            _time = time;
            _stream_type.reset();
            _item_type = Type::sample;
        }

        /**
         * @brief Setter for a new stream_type
         * Resets the sample member of the data item and handles the remaining members accordingly
         *
         * @param stream_type the stream_type to be stored in the data item
         * @param time the timestamp of the data item
         */
        void set(const data_read_ptr<STREAM_TYPE>& stream_type, Timestamp time)
        {
            _stream_type = stream_type;
            _time = time;
            _sample.reset();
            _item_type = Type::type;
        }

        /**
         * gets the type information of the current item.
         * @return the type (sample or streamtype)
         */
        Type getItemType() const
        {
            return _item_type;
        }

        /**
         * Gets the time 
         * @return the time
         */
        Timestamp getTime() const
        {
            return _time;
        }

        /**
         * gets the sample (if set).
         * @return the sample or nullptr
         */
        data_read_ptr<SAMPLE_TYPE> getSample() const
        {
            return _sample;
        }

        /**
         * gets the streamtype (if set).
         * @return the type or nullptr
         */
        data_read_ptr<STREAM_TYPE> getStreamType() const
        {
            return _stream_type;
        }

        /**
         * resets the sample
         */
        void resetSample()
        {
            _sample.reset();
        }

        /**
         * resets the stream type
         */
        void resetStreamType()
        {
            _stream_type.reset();
        }

    private:
        ///@cond no_documentation
        Type _item_type;
        Timestamp  _time;
        data_read_ptr<SAMPLE_TYPE> _sample;
        data_read_ptr<STREAM_TYPE> _stream_type;
        ///@endcond no_documentation
    };

public:
    /**
     * @brief Item receiver for pop call.
     *
     */
    class IDataItemReceiver
    {
    protected:
        /**
         * @brief Destroy the IDataItemReceiver
         *
         */
        virtual ~IDataItemReceiver() = default;
    public:
        /**
         * @brief callback to receive a data read pointer to a sample
         *
         * @param sample the sample currently retrieved by the pop call
         */
        virtual void onReceive(const data_read_ptr<SAMPLE_TYPE>& sample) = 0;
        /**
         * @brief callback to receive a data read pointer to a stream_type
         *
         * @param stream_type the stream type currently retrieved by the pop call
         */
        virtual void onReceive(const data_read_ptr<STREAM_TYPE>& stream_type) = 0;
    };

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
     * @brief pushes a sample data read pointer to the queue
     *
     * @param sample the samples read pointer to push
     * @param time_of_receiving the timestamp at which the sample was received
     * @remark this is threadsafe against pop and other push calls
     */
    virtual void push(const data_read_ptr<SAMPLE_TYPE>& sample, Timestamp time_of_receiving) = 0;
    /**
     * @brief pushes a stream type data read pointer to the queue
     *
     * @param type the types read pointer to push
     * @param time_of_receiving the timestamp at which the type was received
     * @remark this is threadsafe against pop and other push calls
     */
    virtual void pushType(const data_read_ptr<STREAM_TYPE>& type, Timestamp time_of_receiving) = 0;

    /**
     * @brief returns the timestamp of the oldest available sample of the item queue,
     * which is the sample at the front of the queue
     *
     * @return the sample timestamp
     */
    virtual Optional<Timestamp> topTime() = 0;

    /**
     * @brief pops the item at the front of the queue
     *
     * @return true if item is popped
     * @return false if the queue is empty
     * @remark this is threadsafe against push and pop calls
     */
    virtual bool pop() = 0;

    /**
     * @brief pops the item at the front of the queue after putting the item to the given \p receiver
     *
     * @param receiver receiver reference where to callback and put the item of the front of the queue before the item is popped.
     * @return true if item is popped
     * @return false if the queue is empty
     * @remark this is threadsafe against push and pop calls
     */
    virtual bool pop(IDataItemReceiver& receiver) = 0;

    /**
     * @brief return the maximum capacity of the queue
     *
     * @return  maximum capacity of the queue
     */
    virtual size_t capacity() const = 0;

    /**
     * @brief return the current size of the queue
     *
     * @return current size of the queue
     */
    virtual size_t size() const = 0;

    /**
     * @brief remove all elements of the queue
     */
    virtual void clear() = 0;

    /**
     * @brief return the type of the queue
     *
     * @return QueueType the type of the queue
     * Either QueueType::fixed or QueueType::dynamic
     */
    virtual QueueType getQueueType() const = 0;
};
}
}
}
}
