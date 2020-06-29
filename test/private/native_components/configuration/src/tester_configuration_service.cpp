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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <common/gtest_asserts.h>
#include <common/properties_test_helper.h>

#include <fep3/native_components/configuration/configuration_service.h>
#include <fep3/rpc_services/configuration/configuration_rpc_intf_def.h>
#include <fep3/base/properties/properties.h>
#include <fep3/components/base/component_registry.h>
#include <fep3/components/service_bus/mock/mock_service_bus.h>

using namespace ::testing;
using namespace fep3;

using ServiceBusComponentMock = NiceMock<fep3::mock::ServiceBusComponent>;
using RPCServerMock = NiceMock<fep3::mock::RPCServer>;

struct NativeConfigurationService : public testing::Test
{
	NativeConfigurationService()
		: _component_registry(std::make_shared<fep3::ComponentRegistry>())
		, _service_bus(std::make_shared<ServiceBusComponentMock>())
		, _rpc_server(std::make_shared<RPCServerMock>())
		, _configuration_service_impl(std::make_shared<native::ConfigurationService>())
	{
	}

	void SetUp() override
	{
		ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IServiceBus>(
			_service_bus));
		ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IConfigurationService>(
			_configuration_service_impl));
		
		EXPECT_CALL(*_service_bus, getServer()).Times(1).WillOnce(::testing::Return(_rpc_server));
		EXPECT_CALL(*_rpc_server,
			registerService(fep3::rpc::arya::IRPCConfigurationDef::getRPCDefaultName(),
				_)).Times(1).WillOnce(::testing::Return(fep3::Result()));
				::testing::Return(Result());

		ASSERT_FEP3_NOERROR(_component_registry->create());

		_configuration_service_intf = _component_registry->getComponent<IConfigurationService>();
	}

	std::shared_ptr<fep3::ComponentRegistry> _component_registry{nullptr};
	std::shared_ptr<ServiceBusComponentMock> _service_bus{ nullptr };
	std::shared_ptr<RPCServerMock> _rpc_server{nullptr};
	std::shared_ptr<fep3::native::ConfigurationService> _configuration_service_impl{ nullptr };
	fep3::IConfigurationService* _configuration_service_intf{ nullptr };
};

/**
 * @brief The method registerNode of the configuration service is tested
 * 
 */
TEST(ConfigurationService, registerNode)
{
    fep3::native::ConfigurationService service;
    auto properties_clock = createTestProperties();
    ASSERT_FEP3_NOERROR(service.registerNode(properties_clock));

    std::shared_ptr<fep3::IPropertyNode> props;
    auto node = service.getNode("Clock");
    ASSERT_NE(node, nullptr);

    EXPECT_EQ(node->getChild("Clocks")->getChild("Clock1")->getChild("CycleTime")->getValue(), "1");
}

/**
 * @brief It is tested that with registerNode a node can not be registered twice
 * 
 */
TEST(ConfigurationService, registerNodeTwiceFails)
{
    fep3::native::ConfigurationService service;

    auto properties_clock = createTestProperties();
    ASSERT_FEP3_NOERROR(service.registerNode(properties_clock));
    EXPECT_FEP3_RESULT(service.registerNode(properties_clock), ERR_RESOURCE_IN_USE);
}

/**
 * @brief The method unregisterNode of the configuration service is tested
 * 
 */
TEST(ConfigurationService, unregisterNode)
{
    fep3::native::ConfigurationService service;

    auto properties_clock = createTestProperties();
    ASSERT_FEP3_NOERROR(service.registerNode(properties_clock));
    ASSERT_FEP3_NOERROR(service.unregisterNode(properties_clock->getName()));
   
    EXPECT_EQ(service.getNode(properties_clock->getName()), nullptr);
}

/**
 * @brief It is tested that unregisterNode returns an error if the property to unregister does not exist
 * 
 */
TEST(ConfigurationService, unregisterNodeNotExisting)
{
    fep3::native::ConfigurationService service;   
    ASSERT_FEP3_RESULT(service.unregisterNode("not_existing"), fep3::ERR_NOT_FOUND);    
}

/**
 * @brief The method getNode of the configuration service is tested for a property name
 * 
 */
TEST(ConfigurationService, getNode)
{
    fep3::native::ConfigurationService service;
    auto properties_clock = createTestProperties();
    service.registerNode(properties_clock);
   
    EXPECT_NE(service.getConstNode("Clock"), nullptr);
   
    EXPECT_NE(service.getNode("Clock"), nullptr);
}

/**
 * @brief The method getNode of the configuration service is tested for a property path
 * 
 */
TEST(ConfigurationService, getNodeByPath)
{
    fep3::native::ConfigurationService service;
    auto properties_clock = createTestProperties();
    service.registerNode(properties_clock);
 
    auto get_node_const = service.getConstNode("Clock/Clocks/Clock1");
    ASSERT_NE(get_node_const, nullptr);
    EXPECT_EQ(get_node_const->getName(), "Clock1");
       
    auto get_node = service.getNode("Clock/Clocks/Clock1");
    ASSERT_NE(get_node, nullptr);
    EXPECT_EQ(get_node->getName(), "Clock1");
}

/**
 * @brief It is tested that getConstNode returns the root node if no path is provided
 * and that getNode returns a nulltpr if no path is provided
 * 
 */
TEST(ConfigurationService, getNodeRoot)
{
    fep3::native::ConfigurationService service;
    auto properties_clock = createTestProperties();

    ASSERT_FEP3_NOERROR(service.registerNode(properties_clock));
    ASSERT_FEP3_NOERROR(service.registerNode(std::make_shared<NativePropertyNode>("some_node")));

    {       
        auto root_node = service.getConstNode();       
        ASSERT_NE(root_node, nullptr);

        EXPECT_TRUE(root_node->isChild("some_node"));
        EXPECT_TRUE(root_node->isChild("Clock"));
    }   
    {
        std::shared_ptr<IPropertyNode> root_node;
        root_node = service.getNode("");
        EXPECT_EQ(root_node, nullptr);
    }
}


/**
 * @brief The method isNodeRegistered of the configuration service is tested
 * 
 */
TEST(ConfigurationService, isNodeRegistered)
{
    fep3::native::ConfigurationService service;
    auto properties_clock = createTestProperties();
    ASSERT_FEP3_NOERROR(service.registerNode(properties_clock));

    {
        EXPECT_FALSE(service.isNodeRegistered(""));
        EXPECT_FALSE(service.isNodeRegistered("/"));
        EXPECT_FALSE(service.isNodeRegistered("not_existing"));
        EXPECT_FALSE(service.isNodeRegistered("not_existing/not_existing"));
        EXPECT_FALSE(service.isNodeRegistered("Clocks"));

        EXPECT_TRUE(service.isNodeRegistered("Clock"));
        EXPECT_TRUE(service.isNodeRegistered("Clock/Clocks"));
        EXPECT_TRUE(service.isNodeRegistered("Clock/Clocks/Clock1"));
        EXPECT_TRUE(service.isNodeRegistered("Clock/Clocks/Clock1/CycleTime"));
        EXPECT_TRUE(service.isNodeRegistered("Clock/Clocks/Clock2"));
    }

    {
        ASSERT_FEP3_NOERROR(service.unregisterNode(properties_clock->getName()));
        EXPECT_FALSE(service.isNodeRegistered("Clock/Clocks"));
        EXPECT_FALSE(service.isNodeRegistered("not_existing"));
    }
}

/**
 * @brief Check whether system properties may be set if they do not exist yet.
 * Checks for properties of type:
 * * string
 * * int
 * * double
 * * bool
 */
TEST_F(NativeConfigurationService, createSystemProperty)
{
	ASSERT_FEP3_NOERROR(_configuration_service_intf->createSystemProperty("pathString", "string", "value"));
	ASSERT_FEP3_NOERROR(_configuration_service_intf->createSystemProperty("pathInt", "int", "123"));
	ASSERT_FEP3_NOERROR(_configuration_service_intf->createSystemProperty("pathDouble", "double", "1.23"));
	ASSERT_FEP3_NOERROR(_configuration_service_intf->createSystemProperty("pathBool", "bool", "true"));
	const auto property_string_value = getPropertyValue<std::string>(*_configuration_service_intf, "/system/pathString");
	const auto property_int_value = getPropertyValue<int>(*_configuration_service_intf, "/system/pathInt");
	const auto property_double_value = getPropertyValue<double>(*_configuration_service_intf, "/system/pathDouble");
	const auto property_bool_value = getPropertyValue<bool>(*_configuration_service_intf, "/system/pathBool");
	ASSERT_EQ("value", property_string_value.value());
	ASSERT_EQ(123, property_int_value.value());
	ASSERT_EQ(1.23, property_double_value.value());
	ASSERT_EQ(true, property_bool_value.value());
}

/**
* @brief It is tested that setting a system property with a leading slash in the path
* may be set.
*
*/
TEST_F(NativeConfigurationService, createSystemPropertyLeadingSlash)
{

	ASSERT_FEP3_NOERROR(_configuration_service_intf->createSystemProperty("/path/to/property", "string", "value"));
	const auto property_string_value = getPropertyValue<std::string>(*_configuration_service_intf, "/system/path/to/property");
	ASSERT_EQ("value", property_string_value.value());
}

/**
* @brief It is tested that setting a system property with a trailing slash in the path
* may be set.
*
*/
TEST_F(NativeConfigurationService, createSystemPropertyTrailingSlash)
{

	ASSERT_FEP3_NOERROR(_configuration_service_intf->createSystemProperty("path/to/property/", "string", "value"));
	const auto property_string_value = getPropertyValue<std::string>(*_configuration_service_intf, "/system/path/to/property");
	ASSERT_EQ("value", property_string_value.value());
}

/**
* @brief It is tested that setting a system property with an empty path
* returns the corresponding error code.
*
*/
TEST_F(NativeConfigurationService, createSystemPropertyInvalidPath)
{

	EXPECT_EQ(_configuration_service_intf->createSystemProperty("", "string", "value").getErrorCode(), fep3::ResultType_ERR_INVALID_ARG::getCode());
}

/**
 * @brief Check whether system properties may be set if they do exist already.
 */
TEST_F(NativeConfigurationService, createSystemPropertyAlreadyExisting)
{
	// initial creation of property
	{
		ASSERT_FEP3_NOERROR(_configuration_service_intf->createSystemProperty("pathString", "string", "value"));
		const auto property_string_value = getPropertyValue<std::string>(*_configuration_service_intf, "/system/pathString");
		ASSERT_EQ("value", property_string_value.value());
	}

	// set already created properties
	{
		ASSERT_FEP3_NOERROR(_configuration_service_intf->createSystemProperty("pathString", "string", "value2"));
		const auto property_string_value = getPropertyValue<std::string>(*_configuration_service_intf, "/system/pathString");
		ASSERT_EQ("value2", property_string_value.value());
	}
}

/**
 * @brief Check whether nested system properties may be set while property paths may overlap.
 * Checks for properties of type:
 * * string
 * * int
 * * double
 * * bool
 */
TEST_F(NativeConfigurationService, setNestedSystemProperty)
{
	ASSERT_FEP3_NOERROR(_configuration_service_intf->createSystemProperty("path/to/string/property", "string", "value"));
	ASSERT_FEP3_NOERROR(_configuration_service_intf->createSystemProperty("path/to/int/property", "int", "123"));
	ASSERT_FEP3_NOERROR(_configuration_service_intf->createSystemProperty("path/to/double/property", "double", "1.23"));
	ASSERT_FEP3_NOERROR(_configuration_service_intf->createSystemProperty("path/to/bool/property", "bool", "true"));
	const auto property_string_value = getPropertyValue<std::string>(*_configuration_service_intf, "/system/path/to/string/property");
	const auto property_int_value = getPropertyValue<int>(*_configuration_service_intf, "/system/path/to/int/property");
	const auto property_double_value = getPropertyValue<double>(*_configuration_service_intf, "/system/path/to/double/property");
	const auto property_bool_value = getPropertyValue<bool>(*_configuration_service_intf, "/system/path/to/bool/property");
	ASSERT_EQ("value", property_string_value.value());
	ASSERT_EQ(123, property_int_value.value());
	ASSERT_EQ(1.23, property_double_value.value());
	ASSERT_EQ(true, property_bool_value.value());
}

/**
 * @brief Check whether system properties may be set if they do exist already.
 */
TEST_F(NativeConfigurationService, setNestedSystemPropertyAlreadyExisting)
{
	// initial creation of property
	{
		ASSERT_FEP3_NOERROR(_configuration_service_intf->createSystemProperty("path/to/string/property", "string", "value"));
		const auto property_string_value = getPropertyValue<std::string>(*_configuration_service_intf, "/system/path/to/string/property");
		ASSERT_EQ("value", property_string_value.value());
	}

	// set already created properties
	{
		ASSERT_FEP3_NOERROR(_configuration_service_intf->createSystemProperty("path/to/string/property", "string", "value2"));
		const auto property_string_value = getPropertyValue<std::string>(*_configuration_service_intf, "/system/path/to/string/property");
		ASSERT_EQ("value2", property_string_value.value());
	}
}

/**
 * @brief Check whether a parent system property may be set without overriding connected child system properties.
 */
TEST_F(NativeConfigurationService, setSystemParentPropertyNoOverrideChild)
{
	ASSERT_FEP3_NOERROR(_configuration_service_intf->createSystemProperty("path/to", "string", "value_parent"));
	ASSERT_FEP3_NOERROR(_configuration_service_intf->createSystemProperty("path/to/property", "string", "value_child"));
	ASSERT_FEP3_NOERROR(_configuration_service_intf->createSystemProperty("path/to", "string", "value_parent2"));
	const auto property_child_value = getPropertyValue<std::string>(*_configuration_service_intf, "/system/path/to/property");
	const auto property_parent_value = getPropertyValue<std::string>(*_configuration_service_intf, "/system/path/to");
	ASSERT_EQ("value_parent2", property_parent_value.value());
	ASSERT_EQ("value_child", property_child_value.value());
}

/**
 * @brief The helper function getPropertyValue taking the configuration service is tested
 * 
 */
TEST(PropertiesHelper, getPropertyValue)
{
    fep3::native::ConfigurationService service;
    auto properties_clock = createTestProperties();
    service.registerNode(properties_clock);
    
    EXPECT_EQ(getPropertyValue<int32_t>(service, "Clock/Clocks/Clock1/CycleTime").value(), 1);
    EXPECT_EQ(getPropertyValue<int32_t>(service, "Clock/Clocks/Clock2/CycleTime").value(), 2);
    EXPECT_EQ(getPropertyValue<std::string>(service, "Clock").value(), "");

    EXPECT_EQ(getPropertyValue<int32_t>(service, "").has_value(), false); 
    EXPECT_EQ(getPropertyValue<int32_t>(service, "/").has_value(), false);
    EXPECT_EQ(getPropertyValue<int32_t>(service, "not_existing").has_value(), false);   
}

/**
 * @brief The helper function setPropertyValue for a node is tested
 * 
 */
TEST(PropertiesHelper, setPropertyValue)
{
   fep3::native::ConfigurationService service;
   auto properties_clock = createTypeTestProperties();
   service.registerNode(properties_clock);

   EXPECT_FEP3_NOERROR(setPropertyValue<int32_t>(service, "types/int", 3));
   EXPECT_EQ(service.getConstNode("types/int")->getValue(), DefaultPropertyTypeConversion<int32_t>::toString(3));

   EXPECT_FEP3_RESULT(setPropertyValue<double>(service, "types/int", 3), ERR_INVALID_TYPE);

   EXPECT_FEP3_RESULT(setPropertyValue<double>(service, "types/not_existing", 3), ERR_NOT_FOUND);
   EXPECT_FEP3_RESULT(setPropertyValue<double>(service, "/", 3), ERR_NOT_FOUND);
   EXPECT_FEP3_RESULT(setPropertyValue<double>(service, "", 3), ERR_NOT_FOUND);   
}

/**
 * @brief The helper function makeNativePropertyNode is tested
 * 
 */
TEST(PropertiesHelper, makeNativePropertyNode)
{
    auto node = fep3::makeNativePropertyNode("node_name", 3);
    EXPECT_EQ(node->getValue(), "3");
    EXPECT_EQ(node->getTypeName(), fep3::PropertyType<int32_t>::getTypeName());
}