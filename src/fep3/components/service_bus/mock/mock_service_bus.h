/**
*
* @file
* Copyright &copy; Audi AG. All rights reserved.
*
* This Source Code Form is subject to the terms of the
* Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
*/

#pragma once

#include <gmock/gmock.h>

#include <fep3/components/base/component_base.h>
#include <fep3/components/service_bus/rpc/rpc_intf.h>
#include <fep3/components/service_bus/service_bus_intf.h>

namespace fep3
{
namespace mock 
{

struct RPCRequester : fep3::rpc::arya::IRPCRequester
{
    MOCK_CONST_METHOD3(sendRequest, fep3::Result(
        const std::string& service_name,
        const std::string& request_message,
        IRPCResponse& response_callback));
};

struct RPCServer : fep3::rpc::arya::IRPCServer
{
    MOCK_CONST_METHOD0(getRPCServiceIIDs, std::string());
    MOCK_CONST_METHOD0(getRPCInterfaceDefinition, std::string());
    MOCK_METHOD3(handleRequest, fep3::Result(
        const std::string& content_type,
        const std::string& request_message,
        fep3::rpc::arya::IRPCRequester::IRPCResponse&));
    MOCK_CONST_METHOD0(getUrl, std::string());
    MOCK_CONST_METHOD0(getName, std::string());
    MOCK_METHOD1(setName, void(const std::string&));
    MOCK_METHOD2(registerService, fep3::Result(const std::string&,
        const std::shared_ptr<IRPCService>&));
    MOCK_METHOD1(unregisterService, fep3::Result(const std::string&));
};

struct RPCResponse : fep3::rpc::arya::IRPCRequester::IRPCResponse
{
    MOCK_METHOD1(set, fep3::Result(const std::string& response));
};

struct ServiceBusComponent : public fep3::ComponentBase<fep3::IServiceBus>
{
    MOCK_METHOD3(createSystemAccess, fep3::Result(const std::string&, const std::string&, bool));
    MOCK_METHOD1(releaseSystemAccess, fep3::Result(const std::string&));
    MOCK_CONST_METHOD0(getServer, std::shared_ptr<IParticipantServer>());
    MOCK_CONST_METHOD1(getRequester, std::shared_ptr<IParticipantRequester>(const std::string&));
    MOCK_CONST_METHOD1(getSystemAccess, std::shared_ptr<ISystemAccess>(const std::string&));
    MOCK_CONST_METHOD2(getRequester, std::shared_ptr<IParticipantRequester>(const std::string&, bool));
};

}
} 
