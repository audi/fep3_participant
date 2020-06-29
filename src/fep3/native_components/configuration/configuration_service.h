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

#include <unordered_map>
#include <memory>

#include <fep3/components/base/component_base.h>
#include <fep3/components/configuration/configuration_service_intf.h>
#include <fep3/components/configuration/propertynode.h>
#include <fep3/rpc_services/configuration/configuration_service_stub.h>
#include <fep3/components/service_bus/rpc/fep_rpc.h>
#include <fep3/rpc_services/configuration/configuration_rpc_intf_def.h>

namespace fep3
{
namespace native
{

class ConfigurationService : public fep3::ComponentBase<fep3::IConfigurationService>
{
public:
    ConfigurationService();
    ~ConfigurationService() = default;

    fep3::Result create() override;    
    fep3::Result destroy() override;    

    // IConfigurationService
    fep3::Result registerNode(std::shared_ptr<fep3::arya::IPropertyNode> property_node) override;
    fep3::Result unregisterNode(const std::string& name) override;
	bool isNodeRegistered(const std::string& path) const override;

    std::shared_ptr<fep3::arya::IPropertyNode> getNode(const std::string& path) const override;
    std::shared_ptr<const fep3::arya::IPropertyNode> getConstNode(const std::string& path = "") const override;   
	fep3::Result createSystemProperty(const std::string& path, const std::string& type, const std::string& value) const override;
	
private:
	fep3::Result unregisterService(const IComponents& components);
	
private:
    std::shared_ptr<PropertyNode<IPropertyNode>> _root_node{ nullptr };
    std::shared_ptr<NativePropertyNode> _system_properties_node{ nullptr };
    std::shared_ptr<rpc::IRPCServer::IRPCService> _rpc_service{ nullptr };
};


class RPCConfigurationService : public rpc::RPCService<fep3::rpc_stubs::RPCConfigurationService, fep3::rpc::IRPCConfigurationDef>
{
public:
    explicit RPCConfigurationService(ConfigurationService& service) : _service(service) {}

    // Inherited via RPCService
    std::string getProperties(const std::string& property_path) override;
    std::string getAllProperties(const std::string& property_path) override;
    bool exists(const std::string & property_path) override;
    Json::Value getProperty(const std::string & property_path) override;
    int setProperty(const std::string & property_path, const std::string & type, const std::string & value) override;
private:
    ConfigurationService& _service;
};

} // namespace detail
} // namespace fep3