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

#include <thread>

#include "data_registry.h"
#include "data_queue_reuse.hpp"

namespace fep3
{
namespace native
{
namespace arya
{
/**
 * Internal data reader class that holds the unique_ptr to the data reader of the simulation bus.
 * It also manages a thread for data trigger events if given a data receive listener.
 */
class DataRegistry::DataReader : public fep3::native::DataReaderQueue
{
public:
    explicit DataReader(const size_t queue_capacity) : fep3::native::DataReaderQueue(queue_capacity) { }
    ~DataReader() override;
};

/**
 * Internal data writer class that holds the unique_ptr to the data writer of the simulation bus.
 */
class DataRegistry::DataWriter : public IDataRegistry::IDataWriter
{
public:
    DataWriter() = delete;
    explicit DataWriter(ISimulationBus::IDataWriter& _writer_ref,
        const size_t queue_capacity) : _dataout_writer_ref(_writer_ref), _queue_capacity(queue_capacity) {}
    ~DataWriter() override = default;

    fep3::Result write(const IDataSample& data_sample) override;
    fep3::Result write(const IStreamType& stream_type) override;
    fep3::Result flush() override;

    size_t capacity() const;

private:
    ISimulationBus::IDataWriter& _dataout_writer_ref;
    size_t _queue_capacity{ 0 };
};

/**
 * Proxy class that forwards all function calls to the data reader object shared between this and the data registry.
 */
class DataRegistry::DataReaderProxy : public IDataRegistry::IDataReader
{
public:
    DataReaderProxy() = delete;
    explicit DataReaderProxy(std::shared_ptr<IDataRegistry::IDataReader> reader);
    ~DataReaderProxy() = default;

    size_t size() const override;
    size_t capacity() const override;
    fep3::Result pop(IDataReceiver& receiver) override;
    fep3::Optional<Timestamp> getFrontTime() const override;

private:
    const std::shared_ptr<IDataRegistry::IDataReader> _data_reader{ nullptr };
};

/**
 * Proxy class that forwards all function calls to the data writer object shared between this and the data registry.
 */
class DataRegistry::DataWriterProxy : public IDataRegistry::IDataWriter
{
public:
    DataWriterProxy() = delete;
    explicit DataWriterProxy(std::shared_ptr<IDataRegistry::IDataWriter> writer);
    ~DataWriterProxy() = default;

    fep3::Result write(const IDataSample& data_sample) override;
    fep3::Result write(const IStreamType& stream_type) override;
    fep3::Result flush() override;

private:
    const std::shared_ptr<IDataRegistry::IDataWriter> _data_writer{ nullptr };
};
} // namespace arya
} // namespace native
} // namespace fep3