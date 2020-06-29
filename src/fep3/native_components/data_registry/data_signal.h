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

#include <utility>
#include <vector>

#include "data_io.h"
#include "data_registry.h"

namespace fep3
{
namespace native
{
namespace arya
{
/**
 * Base class for all data signals. Has a name and a stream type.
 */
class DataRegistry::DataSignal
{
public:
    DataSignal() = delete;
    DataSignal(DataSignal&&) = default;
    DataSignal(const DataSignal&) = default;
    DataSignal& operator=(DataSignal&&) = default;
    DataSignal& operator=(const DataSignal&) = default;
    DataSignal(const std::string name, const IStreamType& type, bool dynamic_type) :
        _name(std::move(name)), _type(type), _dynamic_type(dynamic_type){}
    virtual ~DataSignal() = default;

    std::string getName() const;
    StreamType getType() const;

    bool hasDynamicType() const;

private:
    std::string _name{};
    StreamType  _type{ fep3::arya::meta_type_raw };
    bool _dynamic_type{ false };
};

/**
 * Internal input signal class that holds a list of all readers registered at the simulation bus
 * and that redirects all incoming data of its signal to all registered data receive listeners.
 */
class DataRegistry::DataSignalIn : public DataSignal,
    public ISimulationBus::IDataReceiver
{
public:
    DataSignalIn() = delete;
    DataSignalIn(DataSignalIn&&) = default;
    DataSignalIn(const DataSignalIn&) = default;
    DataSignalIn& operator=(DataSignalIn&&) = default;
    DataSignalIn& operator=(const DataSignalIn&) = default;
    DataSignalIn(const std::string& name, const IStreamType& type, bool dynamic_type) : DataSignal(name, type, dynamic_type) {}
    ~DataSignalIn() override;

    fep3::Result registerAtSimulationBus(ISimulationBus& simulation_bus);
    void unregisterFromSimulationBus();

    void registerDataListener(const std::shared_ptr<IDataReceiver>& listener);
    void unregisterDataListener(const std::shared_ptr<IDataReceiver>& listener);

    std::unique_ptr<IDataRegistry::IDataReader> getReader(const size_t queue_capacity);

public:
    void operator()(const data_read_ptr<const IStreamType>& type) override;
    void operator()(const data_read_ptr<const IDataSample>& sample) override;

private:
    std::unique_ptr<ISimulationBus::IDataReader> _sim_bus_reader;

    typedef std::list<std::weak_ptr<DataRegistry::DataReader>> DataReaderList;
    std::shared_ptr<DataReaderList> _readers{ std::make_shared<DataReaderList>() };
    std::vector<std::shared_ptr<IDataReceiver>> _listeners{};
    std::thread _receive_thread;

    size_t getMaxQueueSize() const;
    fep3::Result startReceiving();
    fep3::Result stopReceiving();
};

/**
 * Internal output signal class that holds a list of all writers registered at the simulation bus.
 */
class DataRegistry::DataSignalOut : public DataSignal, public ISimulationBus::IDataWriter
{
public:
    DataSignalOut() = delete;
    DataSignalOut(DataSignalOut&&) = default;
    DataSignalOut(const DataSignalOut&) = default;
    DataSignalOut& operator=(DataSignalOut&&) = default;
    DataSignalOut& operator=(const DataSignalOut&) = default;
    DataSignalOut(const std::string& name, const IStreamType& type, bool dynamic_type) : DataSignal(name, type, dynamic_type) {}
    ~DataSignalOut() override;

    fep3::Result registerAtSimulationBus(ISimulationBus& simulation_bus);
    void unregisterFromSimulationBus();

    std::unique_ptr<IDataRegistry::IDataWriter> getWriter(const size_t queue_capacity);

public:
    fep3::Result write(const IDataSample& data_sample) override;
    fep3::Result write(const IStreamType& stream_type) override;
    fep3::Result transmit() override;

private:
    std::unique_ptr<ISimulationBus::IDataWriter> _sim_bus_writer;
    typedef std::list<std::weak_ptr<DataRegistry::DataWriter>> DataWriterList;
    std::shared_ptr<DataWriterList> _writers{ std::make_shared<DataWriterList>() };
    size_t getMaxQueueSize() const;
};
} // namespace arya
} // namespace native
} // namespace fep3