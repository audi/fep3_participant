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

#include "data_registry.h"

#include <a_util/strings.h>

#include "data_io.h"
#include "data_signal.h"
#include "fep3/fep3_errors.h"
#include "fep3/components/service_bus/service_bus_intf.h"

using namespace fep3;
using namespace fep3::native;

std::string RPCDataRegistryService::getSignalInNames()
{
    return a_util::strings::join(_data_registry.getSignalInNames(), ",");
}

std::string RPCDataRegistryService::getSignalOutNames()
{
    return a_util::strings::join(_data_registry.getSignalOutNames(), ",");
}

Json::Value RPCDataRegistryService::getStreamType(const std::string& signal_name)
{
    StreamType streamtype = _data_registry.getStreamType(signal_name);
    Json::Value value;
    value["meta_type"] = streamtype.getMetaTypeName();
    value["properties"]["names"] = a_util::strings::join(streamtype.getPropertyNames(), ",");
    value["properties"]["values"] = a_util::strings::join(streamtype.getPropertyValues(), ",");
    value["properties"]["types"] = a_util::strings::join(streamtype.getPropertyTypes(), ",");
    return value;
}

DataRegistry::DataRegistry() : ComponentBase()
{
}

fep3::Result DataRegistry::create()
{
    auto components = _components.lock();
    if (components)
    {
        //Note: i do not care which meta type is supported by the bus ...
        //the meta type is part of the data ... if there is some unsupported things we will not check this here!
        //the data registry supports every streamtype you can imagine!
        auto service_bus = components->getComponent<IServiceBus>();
        if (service_bus)
        {
            auto rpc_server = service_bus->getServer();
            if (rpc_server)
            {
                if (!_rpc_service)
                {
                    _rpc_service = std::make_shared<RPCDataRegistryService>(*this);
                    FEP3_RETURN_IF_FAILED(
                        rpc_server->registerService(::fep3::rpc::IRPCDataRegistryDef::getRPCDefaultName(), _rpc_service));
                }
            }
            else
            {
                RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "RPC Server not found");
            }
        }
        else
        {
            RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Service Bus is not registered");
        }
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Component pointer is invalid");
    }
    return{};
}

fep3::Result DataRegistry::tense()
{
    // Get simulation bus connection
    ISimulationBus* simulation_bus{ nullptr };
    auto components = _components.lock();
    if (components)
    {
        simulation_bus = components->getComponent<ISimulationBus>();
    }
    if (!simulation_bus || !components)
    {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Simulation Bus is not registered");
    }

    // Register ALL signals IN
    for (auto& current_in : _ins)
    {
        auto res = current_in.second->registerAtSimulationBus(*simulation_bus);
        if (fep3::isFailed(res))
        {
            return res;
        }
    }
    // Register ALL signals OUT
    for (auto& current_out : _outs)
    {
        auto res = current_out.second->registerAtSimulationBus(*simulation_bus);
        if (fep3::isFailed(res))
        {
            return res;
        }
    }
    return{};
}

fep3::Result DataRegistry::relax()
{
    // Unregister ALL signals OUT
    for (auto& current_out : _outs)
    {
        current_out.second->unregisterFromSimulationBus();
    }
    // Unregister ALL signals IN
    for (auto& current_in : _ins)
    {
        current_in.second->unregisterFromSimulationBus();
    }
    return{};
}

fep3::Result DataRegistry::registerDataIn(const std::string& name,
    const IStreamType& type,
    bool is_dynamic_meta_type)
{
    return addDataIn(name, type, is_dynamic_meta_type);
}

fep3::Result DataRegistry::registerDataOut(const std::string& name,
    const IStreamType& type,
    bool is_dynamic_meta_type)
{
    return addDataOut(name, type, is_dynamic_meta_type);
}

fep3::Result DataRegistry::unregisterDataIn(const std::string& name)
{
    if (removeDataIn(name))
    {
        return{};
    }
    else
    {
        std::string description = "Data Registry does not have an input signal named " + name + " registered";
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, description.c_str());
    }
}

fep3::Result DataRegistry::unregisterDataOut(const std::string& name)
{
    if (removeDataOut(name))
    {
        return{};
    }
    else
    {
        std::string description = "Data Registry does not have an output signal named " + name + " registered";
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, description.c_str());
    }
}

fep3::Result DataRegistry::registerDataReceiveListener(const std::string& name,
    const std::shared_ptr<IDataRegistry::IDataReceiver>& listener)
{
    DataSignalIn* found = getDataIn(name);
    if (found)
    {
        found->registerDataListener(listener);
        return{};
    }
    std::string description = "Data Registry does not have an input signal named " + name + " registered";
    RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, description.c_str());
}

fep3::Result DataRegistry::unregisterDataReceiveListener(const std::string& name,
    const std::shared_ptr<IDataRegistry::IDataReceiver>& listener)
{
    DataSignalIn* found = getDataIn(name);
    if (found)
    {
        found->unregisterDataListener(listener);
        return fep3::Result();
    }
    std::string description = "Data Registry does not have an input signal named " + name + " registered";
    RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, description.c_str());
}

std::unique_ptr<IDataRegistry::IDataReader> DataRegistry::getReader(const std::string& name)
{
    return getReader(name, size_t(1));
}

std::unique_ptr<IDataRegistry::IDataReader> DataRegistry::getReader(const std::string& name,
    size_t queue_capacity)
{
    std::unique_ptr<IDataRegistry::IDataReader> reader{ nullptr };
    DataSignalIn* found = getDataIn(name);
    if (found)
    {
        reader = found->getReader(queue_capacity);
    }
    return reader;
}

std::unique_ptr<IDataRegistry::IDataWriter> DataRegistry::getWriter(const std::string& name)
{
    return getWriter(name, size_t(0));
}

std::unique_ptr<IDataRegistry::IDataWriter> DataRegistry::getWriter(const std::string& name, size_t queue_capacity)
{
    std::unique_ptr<IDataRegistry::IDataWriter> writer{ nullptr };
    DataSignalOut* found = getDataOut(name);
    if (found)
    {
        writer = found->getWriter(queue_capacity);
    }
    return writer;
}

std::vector<std::string> DataRegistry::getSignalInNames()
{
    std::vector<std::string> retval;
    for (const auto& signal : _ins)
    {
        retval.push_back(signal.first);
    }
    return retval;
}

std::vector<std::string> DataRegistry::getSignalOutNames()
{
    std::vector<std::string> retval;
    for (const auto& signal : _outs)
    {
        retval.push_back(signal.first);
    }
    return retval;
}

StreamType DataRegistry::getStreamType(const std::string& name)
{
    DataSignal* signal{nullptr};
    signal = getDataIn(name);
    if (signal)
    {
        return signal->getType();
    }
    signal = getDataOut(name);
    if (signal)
    {
        return signal->getType();
    }
    return StreamType{ StreamMetaType{"hook"} };
}

fep3::Result DataRegistry::addDataIn(const std::string& name,
    const IStreamType& type,
    bool is_dynamic_meta_type)
{
    auto found = _ins.find(name);
    if (found != _ins.end())
    {
        if (::operator==(found->second->getType(), type))
        {
            return{};
        }
        else
        {
            std::string description = "The input signal " + name + " does already exist, but with a different type: Passed type " + 
                type.getMetaTypeName() + " but found type " + found->second->getType().getMetaTypeName();
            RETURN_ERROR_DESCRIPTION(ERR_INVALID_TYPE, description.c_str());
        }
    }
    else
    {
        _ins.emplace(name, std::make_shared<DataSignalIn>(name, type, is_dynamic_meta_type));
        return{};
    }
}

fep3::Result DataRegistry::addDataOut(const std::string& name,
    const IStreamType& type,
    bool is_dynamic_meta_type)
{
    auto found = _outs.find(name);
    if (found != _outs.end())
    {
        if (::operator==(found->second->getType(), type))
        {
            return{};
        }
        else
        {
            std::string description = "The output signal " + name + " does already exist, but with a different type: Passed type " +
                type.getMetaTypeName() + " but found type " + found->second->getType().getMetaTypeName();
            RETURN_ERROR_DESCRIPTION(ERR_INVALID_TYPE, description.c_str());
        }
    }
    else
    {
        _outs.emplace(name, std::make_shared<DataSignalOut>(name, type, is_dynamic_meta_type));
        return{};
    }
}

DataRegistry::DataSignalIn* DataRegistry::getDataIn(const std::string& name)
{
    auto found = _ins.find(name);
    if (found != _ins.end())
    {
        return found->second.get();
    }
    return nullptr;
}

DataRegistry::DataSignalOut* DataRegistry::getDataOut(const std::string& name)
{
    auto found = _outs.find(name);
    if (found != _outs.end())
    {
        return found->second.get();
    }
    return nullptr;
}

bool DataRegistry::removeDataIn(const std::string& name)
{
    return (_ins.erase(name) > 0);
}

bool DataRegistry::removeDataOut(const std::string& name)
{
    return (_outs.erase(name) > 0);
}