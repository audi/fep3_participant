/**
 * Data Writer class.
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

#include <fep3/components/base/components_intf.h>
#include <fep3/components/data_registry/data_registry_intf.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/base/streamtype/default_streamtype.h>
#include <fep3/base/sample/data_sample.h>

#include <string>

namespace fep3
{
namespace core
{
namespace arya
{

/// Value for queue capactiy definition if dynamic queue is chosen
constexpr size_t DATA_WRITER_QUEUE_SIZE_DYNAMIC = 0;
/// Value for queue capacity defintion if static queue size is chosen
constexpr size_t DATA_WRITER_QUEUE_SIZE_DEFAULT = 1;

/**
 * @brief Data Writer helper class to write data to a fep3::IDataRegistry::IDataWriter 
 * if registered to the fep3::IDataRegistry
 *
 */
class DataWriter
{
public:
    /**
     * @brief Construct a new Data Writer with an infinite capacity queue.
     * Pushing or popping items from the queue increases or decreases the queue's size.
     * This may lead to out-of-memory situations if big numbers of items are pushed into the queue.
     *
     */
    DataWriter();
    /**
     * @brief Construct a new Data Writer with an infinite capacity queue.
     * Pushing or popping items from the queue increases or decreases the queue's size.
     * This may lead to out-of-memory situations if big numbers of items are pushed into the queue.
     *
     * @param name name of the outgoing data
     * @param stream_type type of the outgoing data
     */
    DataWriter(std::string name, const StreamType& stream_type);
    /**
     * @brief Construct a new Data Writer with a fixed capacity queue.
     * If the queue's capacity is exceeded, e.g. the queue is full and more data items are pushed into the queue,
     * old, not yet used data items are dropped.
     *
     * @param name name of the outgoing data
     * @param stream_type type of the outgoing data
     * @param queue_capacity the size of the underlying data sample queue
     */
    DataWriter(std::string name, const StreamType& stream_type, size_t queue_capacity);
    /**
     * @brief Construct a new Data Writer with an infinite capacity queue.
     * Pushing or popping items from the queue increases or decreases the queue's size.
     * This may lead to out-of-memory situations if big numbers of items are pushed into the queue.
     *
     * @tparam PLAIN_RAW_TYPE plain old c-type for the outgoing data
     * @param name name of the outgoing data
     * @see fep::StreamTypePlain
     */
    template<typename PLAIN_RAW_TYPE>
    DataWriter(std::string name);

    /**
     * @brief Construct a new Data Writer with a fixed capacity queue.
     * If the queue's capacity is exceeded, e.g. the queue is full and more data items are pushed into the queue,
     * old, not yet used data items are dropped.
    *
    * @tparam PLAIN_RAW_TYPE plain old c-type for the outgoing data
    * @param name name of the outgoing data
    * @param queue_capacity capacity of the queue
    * @see fep::StreamTypePlain
    */
    template<typename PLAIN_RAW_TYPE>
    DataWriter(std::string name, size_t queue_capacity);

    /**
     * @brief copy construct a new Data Writer
     * @remark this will not copy the content of the writer queue !!
     *
     * @param other
     */
    DataWriter(const DataWriter& other);

    /**
     * @brief assignment for a data writer.
     * @remark this will not copy the connected writer within the data registry
     * @remark this will not copy the content of the writer queue
     *
     * @param other
     * @return DataWriter&
     */
    DataWriter& operator=(const DataWriter& other);

    /**
     * @brief move construct a new Data Writer
     *
     * @param other the data writer to move
     */
    DataWriter(DataWriter&& other) = default;
    /**
     * @brief move assignement
     *
     * @param other the data writer to move
     * @return DataWriter& the data writer moved to
     */
    DataWriter& operator=(DataWriter&& other) = default;

    /**
     * @brief registers and retrieves a data writer within the data registry
     *
     * @param data_registry the data registry to register to
     * @return fep::Result
     * @see fep::addDataOut
     */
    fep3::Result addToDataRegistry(fep3::arya::IDataRegistry& data_registry);

    /**
     * @brief registers the clockservice to set the time of the samples if not set
     *
     * @param clock_service the clock service to get the time from
     * @return fep::Result
     */
    fep3::Result addClock(fep3::arya::IClockService& clock_service);

    /**
     * @brief removes data writer from the registry
     *
     * @return fep::Result
     */
    fep3::Result removeFromDataRegistry();

    /**
     * @brief removes the clock reference
     *
     * @return fep::Result
     */
    fep3::Result removeClock();

    /**
     * @brief writes a data sample to the writer
     *
     * @param data_sample the sample to write
     * @return fep::Result
     */
    fep3::Result write(const fep3::arya::IDataSample& data_sample);

    /**
     * @brief writes a given raw memory to the writer, if an implementation of fep3::RawMemoryClassType or fep3::RawMemoryStandardType exists
     *
     * @tparam T the type of the memory
     * @param data_to_write the memory value
     * @return fep3::Result
     * @see IDataRegistry::IDataWriter::write
     */
    template<typename T>
    fep3::Result writeByType(T& data_to_write);

    /**
     * @brief writes a stream types to the data writer
     *
     * @param stream_type the stream type to write
     * @return fep::Result
     * @see IDataRegistry::IDataWriter::write
     */
    fep3::Result write(const IStreamType& stream_type);

    /**
     * @brief writes raw memory + a time stamp to the connected IDataWriter as sample
     *
     * @param time the time of the sample (usually simulation time)
     * @param data pointer to the data to copy from
     * @param data_size size in bytes to write
     * @return fep::Result
     * @see IDataRegistry::IDataWriter::write
     */
    fep3::Result write(Timestamp time, const void* data, size_t data_size);

    /**
     * @brief will flush the writers queue
     *        usually this is called while the executeDataOut call of the scheduler !
     *        
     *
     *
     * @param tmtime the current simtime of the flush call. 
     * @return fep3::Result
     */
    virtual fep3::Result flushNow(Timestamp tmtime);

    /**
     * @brief return the size of the writer queue
     * @return size_t the size of the writer queue
     */
    size_t getQueueSize()
    {
        return _queue_size;
    }

    /**
     * @brief get name of the reader
     * @return the name
     */
    virtual std::string getName() const;

private:
    ///the name of outgoing data
    std::string _name;
    ///the type of outgoing data
    StreamType _stream_type;
    ///the writer if registered to the data registry
    std::unique_ptr<IDataRegistry::IDataWriter> _connected_writer;
    size_t _queue_size;

    IClockService* _clock = nullptr;
    uint32_t _counter     = 0;
};

/**
 * @brief helper function to register a data writer to a data registry which is part of the given component registry
 *
 * @param components the components registry to get the data registry from where to register the data writer
 * @param writer the writer to register
 * @return fep::Result
 */
fep3::Result addToComponents(DataWriter& writer, const IComponents& components);

/**
 * @brief helper function to unregister a data writer from a data registry which is part of the given component registry
 *
 * @param components the components registry to get the data registry from where to unregister the data writer
 * @param writer the writer to unregister
 * @return fep::Result
 */
fep3::Result removeFromComponents(DataWriter& writer, const IComponents& components);


/**
* @brief Construct a new Data Writer with dynamic queue
*
* @tparam PLAIN_RAW_TYPE plain old c-type for the outgoing data
* @param name name of the outgoing data
* @see fep::StreamTypePlain
*/
template<typename PLAIN_RAW_TYPE>
DataWriter::DataWriter(std::string name) :
    _name(std::move(name), StreamTypePlain<PLAIN_RAW_TYPE>()),
    _queue_size(DATA_WRITER_QUEUE_SIZE_DYNAMIC)
{
}

/**
* @brief Construct a new Data Writer with fixed queue size
*
* @tparam PLAIN_RAW_TYPE plain old c-type for the outgoing data
* @param name name of the outgoing data
* @param queue_capacity capacity of the queue
* @see fep3::StreamTypePlain
*/
template<typename PLAIN_RAW_TYPE>
DataWriter::DataWriter(std::string name, size_t queue_capacity) :
    _name(std::move(name), StreamTypePlain<PLAIN_RAW_TYPE>()),
    _queue_size(queue_capacity)
{
}

/**
* @brief writes a given raw memory to the writer, if an implementation of RawMemoryClassType or RawMemoryStandardType exists
*
* @tparam T the type of the memory
* @param data_to_write the memory value
* @return fep3::Result
* @see IDataWriter::write
*/
template<typename T>
fep3::Result DataWriter::writeByType(T& data_to_write)
{
    DataSampleType<T> sample_wrapup(data_to_write);
    return write(sample_wrapup);
}

} // end of namespace arya
using arya::DataWriter;

inline fep3::Result addToComponents(DataWriter& writer, const IComponents& components)
{
    return arya::addToComponents(writer, components);
}

inline fep3::Result removeFromComponents(DataWriter& writer, const IComponents& components)
{
    return arya::removeFromComponents(writer, components);
}

} // end of namespace core
} // end of namespace fep3

/**
 * @brief streaming operator to write a stream type
 * 
 * @param writer 
 * @param stream_type 
 * @return fep::DataWriter& 
 */
inline fep3::core::DataWriter& operator<<(fep3::core::DataWriter& writer,
    const fep3::IStreamType& stream_type)
{
    writer.write(stream_type);
    return writer;
}

/**
 * @brief streaming operator to write a sample
 * 
 * @param writer 
 * @param value 
 * @return fep::DataWriter& 
 */
inline fep3::core::DataWriter& operator<<(fep3::core::DataWriter& writer,
    const fep3::IDataSample& value)
{
    writer.write(value);
    return writer;
}

/**
 * @brief streaming operator to write data
 *
 * @tparam T the type to write
 * @param writer the writer to write to
 * @param value  the value of type \p T to writer
 * @throw  std::runtime_error if the sample to write to is not suitable for writing \p value (e. g. if memory of sample is too small)
 * @return fep::DataWriter& the writer
 * @see fep::DataWriter::writeByType
 */
template<typename T>
fep3::core::DataWriter& operator<< (fep3::core::DataWriter& writer,
    T& value)
{
    auto writing_result = writer.writeByType(value);
    if(fep3::isFailed(writing_result))
    {
        throw std::runtime_error
            (std::string()
            + "writing value to writer '" + writer.getName()
            + "' failed with error code " + std::to_string(writing_result.getErrorCode())
            + " and error description: " + writing_result.getDescription()
            );
    }
    return writer;
}
