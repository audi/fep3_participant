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

#include "configuration_service.h"

#include <fep3/components/service_bus/service_bus_intf.h>

#include <algorithm>


namespace fep3
{
namespace native
{

std::shared_ptr<IPropertyNode> getPropertyNodeByPath(const IConfigurationService& config_service, const std::string & property_path);
std::shared_ptr<IPropertyNode> getPropertyNodeByPath(std::shared_ptr<IPropertyNode> node, const std::string & property_path);
std::vector<std::string> collectAllChildPropertyNames(const IPropertyNode& property, const std::string& parent_path);
std::vector<std::string> collectChildPropertyNames(const IPropertyNode& property, const std::string& parent_path);
std::shared_ptr<arya::IPropertyWithExtendedAccess> setPropertyNodeByPath(
	const std::shared_ptr<arya::IPropertyWithExtendedAccess>& node,
	const std::string& property_path,
	const std::string& type,
	const std::string& value);

ConfigurationService::ConfigurationService()
    : _root_node(std::make_shared<PropertyNode<IPropertyNode>>("Root"))
    , _system_properties_node(std::make_shared<NativePropertyNode>("system"))
{
}

fep3::Result ConfigurationService::create()
{
	FEP3_RETURN_IF_FAILED(registerNode(_system_properties_node));
	
    const auto components = _components.lock();
    if (!components)
    {  
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Component pointer is invalid");
    }

    auto service_bus = components->getComponent<fep3::IServiceBus>();
    if (!service_bus)
    {        
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Service Bus is not registered");
    }        
        
    auto rpc_server = service_bus->getServer();
    if (!rpc_server)
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "RPC Server not found");
    }       
    
    if (!_rpc_service)
    {
        _rpc_service = std::make_shared<RPCConfigurationService>(*this);
        const auto rpc_name = ::fep3::rpc::IRPCConfigurationDef::getRPCDefaultName();
        FEP3_RETURN_IF_FAILED(rpc_server->registerService(rpc_name, _rpc_service));
    }

    return{};
}

fep3::Result ConfigurationService::destroy()
{
    const auto components = _components.lock();
    if (!components)
    {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Component pointer is invalid");
    }

    FEP3_RETURN_IF_FAILED(unregisterService(*components));

    return {};
}

fep3::Result ConfigurationService::unregisterService(const IComponents& components)
{
    const auto service_bus = components.getComponent<IServiceBus>();
    if (service_bus)
    {
        auto rpc_server = service_bus->getServer();
        if (rpc_server)
        {
            rpc_server->unregisterService(rpc::IRPCConfigurationDef::getRPCDefaultName());
        }
    }

    return {};
}


fep3::Result ConfigurationService::registerNode(std::shared_ptr<fep3::IPropertyNode> property_node)
{  
    const auto name = property_node->getName();
    if (_root_node->isChild(name))
    {
        RETURN_ERROR_DESCRIPTION(ERR_RESOURCE_IN_USE,
            "Registering property node failed. Node with the name '%s' is already registered.", name.c_str());
    }

    _root_node->setChild(property_node);
    return {};
}


fep3::Result ConfigurationService::unregisterNode(const std::string& name)
{         
    if (!_root_node->isChild(name))
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND,
            "Unregisterung property node failed. Node with the name '%s' is not registered.", name.c_str());
    }

    _root_node->removeChild(name);
    return {};
}


std::shared_ptr<fep3::IPropertyNode> ConfigurationService::getNode(const std::string& path) const
{  
    if (path.empty())
    {
        return {};
    }
    try
    {        
        return getPropertyNodeByPath(_root_node, path);       
    }
    catch (...)
    {
        return {};
    }  
}


std::shared_ptr<const fep3::arya::IPropertyNode> ConfigurationService::getConstNode(const std::string& path) const
{  
    if (path.empty())
    {       
        return _root_node;
    }

    try
    {
        return getPropertyNodeByPath(_root_node, path);           
    }
    catch (...)
    {
        return {};
    }
}

bool ConfigurationService::isNodeRegistered(const std::string& path) const
{
    const auto node = getNode(path);
    return node != nullptr;
}

fep3::Result ConfigurationService::createSystemProperty(const std::string& path,
	const std::string& type,
	const std::string& value) const
{
	if (path.empty())
	{
		RETURN_ERROR_DESCRIPTION(ResultType_ERR_INVALID_ARG::getCode(), "System property path may not be empty.");
	}
	try
	{
		if (setPropertyNodeByPath(_system_properties_node, path, type, value))
		{
			return {};
		}
		RETURN_ERROR_DESCRIPTION(ResultType_ERR_POINTER::getCode(),
			"Creating system property '%s' failed.", path.c_str());
	}
	catch (std::invalid_argument& ex)
	{
		RETURN_ERROR_DESCRIPTION(ResultType_ERR_INVALID_ARG::getCode(),
			"Creating system property '%s' failed: %s", path.c_str(), ex.what());
	}
	catch (...)
	{
		RETURN_ERROR_DESCRIPTION(ResultType_ERR_UNKNOWN::getCode(), 
			"Creating system property '%s' failed.", path.c_str());
	}
}

class PropertyPath
{
public:
    PropertyPath(const std::string value)
        : _value(std::move(value))
        , _path_separator('/')
    {
        normalize();
        validate();
    }
    std::string getValue() const
    {
        return _value;
    }

    void validate() const
    {
        const auto splits = splitPath();
        if (splits.empty())
        {
            throw std::invalid_argument("property path has no main node");
        }

        for (const auto& split : splits)
        {
            validatePropertyName(split);
        }
    }

    PropertyPath& removeLastProperty()
    {
        const auto last_separator = _value.rfind(_path_separator);
        if (std::string::npos == last_separator)
        {
           _value = "";
        }
        else
        {
            _value = _value.substr(0, last_separator);
        }       
        
        return *this;
    }

    PropertyPath& removeFirstProperty()
    {
        const auto first_separator = _value.find(_path_separator);
        if (std::string::npos == first_separator)
        {
            _value = "";
        }
        else
        {
            _value = _value.substr(first_separator, _value.length()-1);
        }     

        return *this;
    }

    PropertyPath& appendProperty(const std::string& property_name)
    {
        if (_value.empty())
        {
            _value = property_name;
        }
        else
        {
            _value = _value + _path_separator + property_name;
        }      
        return *this;
    }

    std::vector<std::string> splitPath() const
    {        
        return a_util::strings::split(_value, std::string(1, _path_separator));
    }

    size_t getPathElementCount() const
    {
        if (_value.empty())
        {
            return 0;
        }

        const size_t separator_count = std::count(_value.begin(), _value.end(), _path_separator);
        if (separator_count == 0)
        {
            return 1;
        }

        return separator_count + 1;
    }

    operator std::string() const
    {
        return _value;
    }

private:
    void normalize()
    {
        if (_value.empty())
        {
            return;
        }

        if (_value.back() == _path_separator)
        {
            _value.pop_back();
        }

        if (!_value.empty() && _value.front() == _path_separator)
        {
            _value = _value.substr(1, _value.size());
        }
    }
private:
    std::string _value;
    char _path_separator;
};


std::shared_ptr<IPropertyNode> getPropertyNodeByPath(std::shared_ptr<IPropertyNode> node, const std::string & property_path)
{
    const PropertyPath path(property_path);   

    const auto split_path = path.splitPath();
    const auto first = *(split_path.begin());

    auto current_property = node; 

    auto iter = split_path.begin();
    while (iter != split_path.end() 
        && current_property != nullptr)
    {
        current_property = current_property->getChild(*iter);
        ++iter;
    }

    return current_property;
}

std::shared_ptr<const IPropertyNode> getConstPropertyNodeByPath(std::shared_ptr<const IPropertyNode> node,
    const std::string & property_path)
{
    const PropertyPath path(property_path);

    const auto split_path = path.splitPath();
    const auto first = *(split_path.begin());

    auto current_property = node;

    auto iter = split_path.begin();
    while (iter != split_path.end()
        && current_property != nullptr)
    {
        current_property = current_property->getChild(*iter);
        ++iter;
    }

    return current_property;
}

std::shared_ptr<IPropertyNode> getPropertyNodeByPath(const IConfigurationService& config_service,
    const std::string & property_path)
{
    using namespace a_util::strings;       

    const PropertyPath path(property_path); 

    const auto root_requested = path.getPathElementCount() == 1;
    const auto split_path = path.splitPath();
    const auto first_node =  *(split_path.begin());    
  
    auto current_property = config_service.getNode(first_node);
    if (!current_property)
    {
       return current_property;
    }        
    
    if (root_requested)
    {
        return current_property;
    }

    auto path_without_root = path;
    path_without_root.removeFirstProperty();

    return getPropertyNodeByPath(current_property, path_without_root);
}

std::shared_ptr<const IPropertyNode> getConstPropertyNodeByPath(const IConfigurationService& config_service,
    const std::string & property_path)
{
    using namespace a_util::strings;

    const PropertyPath path(property_path);

    const auto root_requested = path.getPathElementCount() == 1;
    const auto split_path = path.splitPath();
    const auto first_node = *(split_path.begin());

    auto current_property = config_service.getConstNode(first_node);
    if (!current_property)
    {
        return current_property;
    }

    if (root_requested)
    {
        return current_property;
    }

    auto path_without_root = path;
    path_without_root.removeFirstProperty();

    return getConstPropertyNodeByPath(current_property, path_without_root);
}

std::shared_ptr<arya::IPropertyWithExtendedAccess> setPropertyNodeByPath(
	const std::shared_ptr<arya::IPropertyWithExtendedAccess>& node,
	const std::string& property_path,
	const std::string& type,
	const std::string& value)
{
	const PropertyPath path(property_path);
	auto split_path = path.splitPath();

	if (1 == split_path.size())
	{
		auto property = node->getChildImpl(split_path.front());
		if (property)
		{
			property->setValue(value, type);
			return property;
		}
		
		return node->setChild(std::make_shared<NativePropertyNode>(split_path.front(), value, type));
	}
	
	auto child_node = node->getChildImpl(split_path.front());
	if (!child_node)
	{
		child_node = node->setChild(std::make_shared<NativePropertyNode>(split_path.front(), "", "node"));
	}

	auto path_without_root = path;
	path_without_root.removeFirstProperty();
	
	return setPropertyNodeByPath(child_node, path_without_root, type, value);
}

std::vector<std::string> collectAllChildPropertyNames(const IPropertyNode& property,
    PropertyPath parent_path)
{
    std::vector<std::string> names{};

    parent_path.appendProperty(property.getName());

    const auto& properties = property.getChildren();

    for (const auto& iter : properties)
    {
        names.push_back(PropertyPath(parent_path).appendProperty(iter->getName()));
        auto iter_names = collectAllChildPropertyNames(*iter, parent_path);
        for (const auto& current : iter_names)
        {
            names.push_back(current);
        }
    }
    return names;
}

std::vector<std::string> collectChildPropertyNames(const IPropertyNode& property)
{
    std::vector<std::string> names{};

    const auto& properties = property.getChildren();

    for (const auto& iter : properties)
    {
        names.push_back(iter->getName());
    }

    return names;
}

std::string RPCConfigurationService::getProperties(const std::string& property_path)
{
    try
    {
        if (property_path.empty() || property_path == "/")
        {
            return a_util::strings::join(
                collectChildPropertyNames(
                *_service.getConstNode("")), ",");
        }
        else
        {
            const PropertyPath path(property_path);
            const auto property = getConstPropertyNodeByPath(_service, path);
            if (nullptr == property)
            {
                return {};
            }

            const auto names = collectChildPropertyNames(*property);
            return a_util::strings::join(names, ",");
        }
    }
    catch (...)
    {
        return {};
    }
}


std::string RPCConfigurationService::getAllProperties(const std::string& property_path)
{
    try
    {
        const PropertyPath path(property_path);         

        const auto property = getPropertyNodeByPath(_service, path);
        if (nullptr == property)
        {
            return {};
        }

        auto path_of_found_property = path;
        if (path_of_found_property.getPathElementCount() > 0)
        {
            /// remove the property we just searched for from path, to make property names absolute
            path_of_found_property.removeLastProperty();
        }
        
        const auto names = collectAllChildPropertyNames(*property, path_of_found_property);
        return a_util::strings::join(names, ",");
    }
    catch (...)
    {
        return {};
    }
 }

bool RPCConfigurationService::exists(const std::string & property_path)
{
    try
    {
        if (property_path.empty() || property_path == "/")
        {
            return true;
        }
        else
        {
            const PropertyPath path(property_path);

            const auto property = getPropertyNodeByPath(_service, property_path);
            return property != nullptr;
        }
    }
    catch (...)
    {
        return false;
    }
}

Json::Value RPCConfigurationService::getProperty(const std::string& property_path)
{
    Json::Value property_to_return;
    property_to_return["value"] = "";
    property_to_return["type"] = "";
    
    try
    {
        std::shared_ptr<const IPropertyNode> property_node;
        if (property_path.empty() || property_path == "/")
        {
            property_node = _service.getConstNode("");
        }
        else
        {
            const PropertyPath path(property_path);
            property_node = getConstPropertyNodeByPath(_service, property_path);
        }
        if (nullptr == property_node)
        {          
            return property_to_return;
        }

        property_to_return["value"] = property_node->getValue();
        property_to_return["type"] = property_node->getTypeName();
    }
    catch (...)
    {
        property_to_return["value"] = "";
        property_to_return["type"] = "";
    }  

    return property_to_return;   
}

int RPCConfigurationService::setProperty(const std::string& property_path, const std::string& type, const std::string& value)
{
    if (property_path.empty())
    {
        return fep3::ResultType_ERR_INVALID_ARG::getCode();
    }    
    try
    {
        const PropertyPath path(property_path);
        
        auto property = getPropertyNodeByPath(_service, property_path);
        if (nullptr == property)
        {
            return fep3::ResultType_ERR_NOT_FOUND::getCode();
        }
        const auto result = property->setValue(value, type);
        return result.getErrorCode();
    }
    catch (std::invalid_argument& /*ex*/)
    {
        return fep3::ResultType_ERR_INVALID_ARG::getCode();
    }
    catch (...)
    {
        return fep3::ResultType_ERR_UNKNOWN::getCode();
    }    
 }
 
} // namespace detail
} // namespace fep3