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

#include <memory>
#include <vector>

#include <fep3/fep3_participant_types.h>
#include <fep3/fep3_errors.h>
#include <fep3/fep3_timestamp.h>
#include <fep3/fep3_optional.h>
#include <fep3/components/base/component_iid.h>
#include <fep3/base/streamtype/streamtype.h>
#include <fep3/base/sample/data_sample_intf.h>


namespace fep3
{
namespace arya
{

/**
 * @brief Interface for the simulation bus
 *
 * @attention Implementations of this class and subclasses are not necessarily thread safe.
 *  Thus, when using this interface, make sure each object is called from a single thread at a time only.
 *  There might be some exceptions from this rule explicitly described in the interface methods' documentation.
 */
class FEP3_PARTICIPANT_EXPORT ISimulationBus
{
public:
    // Definition of the component interface identifier for the simulation bus
    FEP_COMPONENT_IID("simulation_bus.arya.fep3.iid")

protected:
    /**
     * @brief DTOR
     * @note This DTOR is explicitly protected to prevent destruction via this interface.
     */
    virtual ~ISimulationBus() = default;

public:
    /**
     * @brief Checks if the \p stream_type is supported by the simulation bus implementation
     *
     * @param stream_type The stream type to be checked
     * @return true if the \p stream_type is supported by the simulation bus implementation, false
     *         otherwise
     */
    virtual bool isSupported(const arya::IStreamType& stream_type) const = 0;

    class IDataReceiver;
    /**
     * @brief Class providing access to input data
     * @see fep3::ISimulationBus::getReader
     */
    class FEP3_PARTICIPANT_EXPORT IDataReader
    {
    public:
        /**
         * @brief DTOR
         */
        virtual ~IDataReader() = default;
        /**
         * @brief Gets the current size of the item queue
         *
         * @return The current size of the item queue
         */
        virtual size_t size() const = 0;
        /**
         * @brief Gets the current capacity of the item queue
         *
         * @return The current capacity of the item queue
         */
        virtual size_t capacity() const = 0;
        /**
         * @brief Pops the front item from the reader queue (if not empty) and passes it to the
         * callback of \p receiver. This method is non-blocking.
         * @remark If "data triggered" reception is currently running (see methods \ref receive and \ref stop)
         * the reader queue is always empty because incoming data will immediately be passed to the "data triggered" receivers.
         *
         * @param receiver The receiver object to be called back if an item is in the reader queue
         * @return true if an item has been popped, false otherwise
         */
        virtual bool pop(IDataReceiver& receiver) = 0;
        /**
         * @brief Starts passing all incoming items to the @p receiver. If this method is called
         * multiple times, the incoming items will be passed to the @p receiver of the latest call only.
         * This method blocks until @ref stop is called. Use this method to implement "data triggered" behavior.
         * This method is thread-safe against calls to all other methods of this class.
         * @note The receiver will be called from within the thread context this method is called from.
         *
         * @param receiver The receiver to pass all incoming items to
         */
        virtual void receive(IDataReceiver& receiver) = 0;
        /**
         * @brief Stops all receptions running due to calls of method \ref receive. This causes all blocking calls of
         * method @ref receive to return. If there are currently no blocking calls to \ref receive, this method does nothing.
         */
        virtual void stop() = 0;

        /**
         * @brief Gets the time of the front item in the reader queue
         *
         * @return The timestamp of the front item, if at least one item is in the reader queue,
         *         empty optional otherwise
         */
        virtual Optional<arya::Timestamp> getFrontTime() const = 0;
    };

    /**
     * @brief Class for receiving data and stream types
     *
     */
    class FEP3_PARTICIPANT_EXPORT IDataReceiver
    {
    public:
        /**
         * @brief DTOR
         */
        virtual ~IDataReceiver() = default;

        /**
         * @brief Receives a stream type item
         *
         * @param type The received stream type
         */
        virtual void operator()(const arya::data_read_ptr<const arya::IStreamType>& type) = 0;
        /**
         * @brief Receives a data sample item
         *
         * @param sample The received data sample
         */
        virtual void operator()(const arya::data_read_ptr<const arya::IDataSample>& sample) = 0;
    };

    /**
     * @brief Class providing data transmission facilities
     * @see fep3::ISimulationBus::getWriter
     */
    class FEP3_PARTICIPANT_EXPORT IDataWriter
    {
    public:
        /**
         * @brief DTOR
         */
        virtual ~IDataWriter() = default;
        /**
         * @brief Copies the content of the \p data_sample into the transmit buffer
         *
         * @param data_sample The data sample to copy the content from
         * @return ERR_NOERROR if succeded, error code otherwise:
         * @retval ERR_UNEXPECTED An unexpected error occurred.
         * @retval ERR_MEMORY The transmit buffer's memory is not suitable to hold the \p data_sample's content.
         * @retval ERR_NOT_IMPL There is no functioning implementation of this method.
         * @retval ERR_NOT_CONNECTED The data writer is not connected to a transmit buffer.
         *
         */
        virtual fep3::Result write(const arya::IDataSample& data_sample) = 0;
        /**
         * @brief This is an overloaded member function that copies the content of the \p stream_type into the transmit buffer
         *
         * @param stream_type T stream type to copy the content from
         * @return fep3::Result ERR_NOERROR if succeded, error code otherwise (see above)
         */
        virtual fep3::Result write(const arya::IStreamType& stream_type) = 0;
        /**
         * @brief Transmits the content of the transmit buffer. This method blocks until all content
         * of the transmit buffer has been transmitted.
         *
         * @return fep3::Result ERR_NOERROR if succeded, error code otherwise:
         * @retval ERR_UNEXPECTED An unexpected error occurred.
         * @retval ERR_NOT_IMPL There is no functioning implementation of this method.
         * @retval ERR_NOT_CONNECTED The data writer is not connected to a transmission resource.
         * @retval ERR_FAILED The transmission failed.
         */
        virtual fep3::Result transmit() = 0;
    };

    /**
     * @brief Gets a reader for data on an input signal of the given static \p stream_type with the
     * given signal \p name whose queue capacity is 1.
     *
     * @param name The name of the input signal (must be unique)
     * @param stream_type The stream type of the input signal (see @ref fep3::arya::IStreamType)
     * @return Data reader if succeeded, nullptr otherwise (e. g. if this method has already been
     *         called for the signal with the given signal \p name); unregistration is automatically
     *         performed when the data reader is destroyed
     */
    virtual std::unique_ptr<IDataReader> getReader
        (const std::string& name
        , const arya::IStreamType& stream_type
        ) = 0;
    /**
     * This is an overloaded member function that gets a reader for data on an input
     * signal with the given signal \p name whose queue capacity is \p queue_capacity. The queue
     * behaves like a FIFO: If the queue is full the oldest sample (according to the samples' timestamp)
     * will be discarded upon arrival of a new sample.
     *
     * @param name The name of the input signal (must be unique)
     * @param stream_type The stream type of the input signal (see @ref fep3::arya::IStreamType)
     * @param queue_capacity The capacity of the queue.
     * @return Data reader if succeeded, nullptr otherwise (e. g. if this method has already been
     *         called for the signal with the given signal \p name); unregistration is automatically
     *         performed when the data reader is destroyed
     */
    virtual std::unique_ptr<IDataReader> getReader
        (const std::string& name
        , const arya::IStreamType& stream_type
        , size_t queue_capacity
        ) = 0;

    /**
     * @brief This is an overloaded member function that gets a reader for data on an input
     * signal of dynamic stream type with the given signal \p name.
     *
     * @param name The name of the input signal (must be unique)
     * @return Data reader if succeeded, nullptr otherwise (e. g. if this method has already been
     *         called for the signal with the given signal \p name); unregistration is automatically
     *         performed when the data reader is destroyed
     */
    virtual std::unique_ptr<IDataReader> getReader(const std::string& name) = 0;
    /**
     * This is an overloaded member function that gets a reader for data on an input
     * signal of dynamic stream type with the given signal \p name whose queue capacity is \p
     * queue_capacity. The queue behaves like a FIFO: If the queue is full the oldest sample
     * (according to the samples' timestamp) will be discarded upon arrival of a new sample.
     *
     * @param name The name of the input signal (must be unique)
     * @param queue_capacity The capacity of the queue.
     * @return Data reader if succeeded, nullptr otherwise (e. g. if this method has already been
     *         called for the signal with the given signal \p name); unregistration is automatically
     *         performed when the data reader is destroyed
     */
    virtual std::unique_ptr<IDataReader> getReader(const std::string& name, size_t queue_capacity) = 0;

    /**
     * @brief Gets a writer for data on an output signal of the given static \p stream_type with
     * the given signal \p name.
     *
     * @param name The name of the output signal (must be unique)
     * @param stream_type The stream type of the output signal (see @ref fep3::arya::IStreamType)
     * @return Data writer if succeeded, nullptr otherwise  (e. g. if this method has already been
     *         called for the signal with the given signal \p name)
     */
    virtual std::unique_ptr<IDataWriter> getWriter
        (const std::string& name
        , const arya::IStreamType& stream_type
        ) = 0;
    /**
     * This is an overloaded member function that gets a writer for data on an output signal
     * of the given static \p stream_type with the given signal \p name whose queue capacity is \p
     * queue_capacity. The queue behaves like a FIFO: If the queue is full the oldest sample
     * (according to the samples' timestamp) will be discarded upon arrival of a new sample.
     *
     * @param name The name of the output signal (must be unique)
     * @param stream_type The stream type of the output signal (see @ref fep3::arya::IStreamType)
     * @param queue_capacity The capacity of the queue.
     * @return Data writer if succeeded, nullptr otherwise (e. g. if this method has already been
     *         called for the signal with the given signal \p name)
     */
    virtual std::unique_ptr<IDataWriter> getWriter
        (const std::string& name
        , const arya::IStreamType& stream_type
        , size_t queue_capacity
        ) = 0;
    /**
     * @brief This is an overloaded member function that gets a writer for data on an output
     * signal of dynamic stream type with the given signal \p name.
     *
     * @param name The name of the output signal (must be unique)
     * @return Data writer if succeeded, nullptr otherwise (e. g. if this method has already been
     *         called for the signal with the given signal \p name)
     */
    virtual std::unique_ptr<IDataWriter> getWriter(const std::string& name) = 0;
    /**
     * @brief This is an overloaded member function that gets a writer for data on an output
     * signal of dynamic stream type with the given signal \p name whose queue capacity is \p
     * queue_capacity. The queue behaves like a FIFO: If the queue is full the oldest sample
     * (according to the samples' timestamp) will be discarded upon arrival of a new sample.
     *
     * @param name The name of the output signal (must be unique)
     * @param queue_capacity The capacity of the queue.
     * @return Data writer if succeeded, nullptr otherwise (e. g. if this method has already been
     *         called for the signal with the given signal \p name)
     */
    virtual std::unique_ptr<IDataWriter> getWriter(const std::string& name, size_t queue_capacity) = 0;
};

}
using arya::ISimulationBus;
}
