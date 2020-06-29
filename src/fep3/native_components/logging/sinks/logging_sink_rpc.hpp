/**
* @file
* Copyright &copy; Audi AG. All rights reserved.
*
* This Source Code Form is subject to the terms of the
* Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <fep3/base/properties/properties.h>
#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/components/service_bus/rpc/fep_rpc.h>
#include <fep3/components/service_bus/service_bus_intf.h>
#include <fep3/rpc_services/logging/logging_service_rpc_intf_def.h>
#include <fep3/rpc_services/logging/logging_rpc_sink_client_client_stub.h>
#include <fep3/rpc_services/logging/logging_rpc_sink_service_service_stub.h>

#include <map>
#include <tuple>

namespace fep3
{
namespace native
{

//this is the class to send messages to a registered sink far away on another process
//this client MUST register !!
using RPCSinkClientClient = rpc::RPCServiceClient<fep3::rpc_stubs::RPCLoggingRPCSinkClientClientStub,
    fep3::rpc::IRPCLoggingSinkClientDef>;
using RPCSinkClientService = rpc::RPCService<fep3::rpc_stubs::RPCLoggingRPCSinkServiceServiceStub,
    fep3::rpc::IRPCLoggingSinkServiceDef>;

class LoggingSinkRPC;

class RPCSinkClientServiceImpl : public RPCSinkClientService
{
public:
    explicit RPCSinkClientServiceImpl(LoggingSinkRPC& logging_sink);
    int registerRPCLoggingSinkClient(const std::string& address,
        const std::string& logger_name_filter,
        int severity) override;
    int unregisterRPCLoggingSinkClient(const std::string& address) override;

private:
    LoggingSinkRPC& _logging_sink;
};

/**
* @brief Implementation of the rpc logging. Can be used as a base class for a custom sink.
*        Logs will be send to the system library.
*/
class LoggingSinkRPC : public Properties<ILoggingService::ILoggingSink>
{
public:
    explicit LoggingSinkRPC(IServiceBus& service_bus) : _service_bus(&service_bus)
    {
        _service_bus->getServer()->registerService(
            fep3::rpc::IRPCLoggingSinkServiceDef::getRPCDefaultName(),
            std::make_shared<RPCSinkClientServiceImpl>(*this));
    }

    void releaseServiceBus()
    {
        std::lock_guard<std::mutex> lock(_sync_filters);
        _service_bus->getServer()->unregisterService(fep3::rpc::IRPCLoggingSinkServiceDef::getRPCDefaultName());
        _service_bus = nullptr;
        _client_filters.clear();
    }

    fep3::Result log(logging::LogMessage log) const override
    {
        fep3::Result result = ERR_NOERROR;
        std::lock_guard<std::mutex> lock(_sync_filters);

        for (const auto& current_client : _client_filters)
        {
            //TODO: Filter here! 
            try
            {
                result = static_cast<int32_t>(current_client.second._client->onLog(log._message,
                    log._logger_name,
                    log._participant_name,
                    static_cast<int>(log._severity),
                    log._timestamp));
            }
            catch (jsonrpc::JsonRpcException)
            {
                result = ERR_EXCEPTION_RAISED;
            }
        }
        return result;
    }


public:
    int registerRPCLoggingSinkClient(const std::string& address,
        const std::string& logger_name_filter,
        int severity)
    {
        std::lock_guard<std::mutex> lock(_sync_filters);
        if (_service_bus)
        {
            std::unique_ptr<RPCSinkClientClient> new_client(
                new RPCSinkClientClient(rpc::IRPCLoggingSinkClientDef::getRPCDefaultName(),
                    _service_bus->getRequester(address, true)));

            auto& new_filter = _client_filters[address];
            new_filter._name_filter = logger_name_filter;
            new_filter._severity_filter = static_cast<fep3::logging::Severity>(severity);
            new_filter._client.reset(new_client.release());
            return 0;
        }
        else
        {
            //this call is while shutting down
            return ERR_INVALID_STATE.getCode();
        }
    }

    int unregisterRPCLoggingSinkClient(const std::string& address)
    {
        std::lock_guard<std::mutex> lock(_sync_filters);
        _client_filters.erase(address);
        return 0;
    }

private:
    struct ClientFilter
    {
        std::string _name_filter;
        fep3::logging::Severity _severity_filter;
        std::unique_ptr<RPCSinkClientClient> _client;
    };
    /// RPC client to send the logs to the system library
    std::map<std::string, ClientFilter> _client_filters;
    mutable std::mutex _sync_filters;

    IServiceBus* _service_bus;
};

inline RPCSinkClientServiceImpl::RPCSinkClientServiceImpl(LoggingSinkRPC& logging_sink)
    : _logging_sink(logging_sink)
{
}
inline int RPCSinkClientServiceImpl::registerRPCLoggingSinkClient(const std::string& address, const std::string& logger_name_filter, int severity)
{
    return _logging_sink.registerRPCLoggingSinkClient(address, logger_name_filter, severity);
}

inline int RPCSinkClientServiceImpl::unregisterRPCLoggingSinkClient(const std::string& address)
{
    return _logging_sink.unregisterRPCLoggingSinkClient(address);
}

}
} // namespace fep3