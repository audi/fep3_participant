/**
 * @file
 * @copyright AUDI AG
 *            All right reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 */
#include "rti_dds_server.h"
#include <a_util/result.h>

using namespace fep3::arya;

namespace fep3
{
namespace rti_dds
{
/*******************************************************************************************
 * this implementation will be done with FEPSDK-2132
 *******************************************************************************************/
/*
struct RPCResponseToFEPResponse : public IRPCRequester::IRPCResponse
{
    rpc::IResponse& _bounded_response;
    RPCResponseToFEPResponse(rpc::IResponse& response_to_bind) : _bounded_response(response_to_bind)
    {
    }
    fep3::Result set(const std::string& response)
    {
        _bounded_response.Set(response.c_str(), response.size());
        return {};
    }
};*/

/*******************************************************************************************
 ** this implementation will be done with FEPSDK-2132
 *******************************************************************************************/
DDSServer::DDSReceiverToRPCServiceWrapper::DDSReceiverToRPCServiceWrapper(const std::shared_ptr<IRPCService>& service)
    : _service(service)
{
    
}

a_util::result::Result DDSServer::DDSReceiverToRPCServiceWrapper::HandleCall(const char* ,
    size_t )
{
    return {};
}

std::shared_ptr<IServiceBus::IParticipantServer::IRPCService> DDSServer::DDSReceiverToRPCServiceWrapper::getService() const
{
    return _service;
}


/*******************************************************************************************
 ** this implementation will be done with FEPSDK-2132
 *******************************************************************************************/

DDSServer::DDSServer(const std::string& name,
                     const std::string& url, 
                     const std::string& system_name) 
    : fep3::base::arya::ServiceRegistryBase(name, system_name),
      _url(url)
{
    
}
DDSServer::~DDSServer()
{
    
}

std::vector<std::string> DDSServer::getRegisteredServiceNames() const 
{
    std::vector<std::string> services_return_value;
    std::lock_guard<std::recursive_mutex> lock(_sync_list);
    for (const auto& service : _service_wrappers)
    {
        services_return_value.push_back(service.first);
    }
    return services_return_value;
}
std::shared_ptr<rpc::arya::IRPCServer::IRPCService> DDSServer::getServiceByName(const std::string& service_name) const
{
    std::lock_guard<std::recursive_mutex> lock(_sync_list);
    const auto iterator = _service_wrappers.find(service_name);
    if (iterator != _service_wrappers.cbegin())
    {
        return iterator->second->getService();
    }
    return {};
}

fep3::Result DDSServer::registerService(const std::string& service_name,
    const std::shared_ptr<IRPCService>& )
{
    std::lock_guard<std::recursive_mutex> lock(_sync_list);
    const auto& service_found = _service_wrappers.find(service_name);
    if (service_found != _service_wrappers.cend())
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG,
            "Service with the name %s already exists",
            service_name.c_str());
    }
    else
    {
     //  *this implementation will be done with https ://www.cip.audi.de/jira/browse/FEPSDK-2132
     //  auto wrapper = std::make_shared<DDSServer::RPCObjectToRPCServerWrapper>(service);
      //  auto res = _http_server.RegisterRPCObject(service_name.c_str(), wrapper.get());
     //   if (fep3::isOk())
     //   {
     //       _service_wrappers[service_name] = wrapper;
            return {};
     //   }
     //   else
     //   {
     //       return res;
     //   }
    }
}


fep3::Result DDSServer::unregisterService(const std::string& service_name)
{
    const auto& service_found = _service_wrappers.find(service_name);
    if (service_found == _service_wrappers.end())
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG,
            "Service with the name %s does not exists",
            service_name.c_str());
    }
    else
    {
       // DDSServer.UnregisterRPCObject(servcie_name.c_str());
        _service_wrappers.erase(service_name);
        return {};
    }
}

std::string DDSServer::getUrl() const
{
    return _url;
}
}
}

