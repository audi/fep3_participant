/**
* @file
* Copyright &copy; Audi AG. All rights reserved.
*
* This Source Code Form is subject to the terms of the
* Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
*/

#include <iostream>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <common/gtest_asserts.h>
#include <common/properties_test_helper.h>

#include "test_configuration_proxy_stub.h"

#include <fep3/components/base/component_registry.h>
#include <fep3/components/service_bus/rpc/fep_rpc.h>
#include <fep3/native_components/configuration/configuration_service.h>

#include <fep3/native_components/service_bus/service_bus.h>
#include <fep3/native_components/service_bus/testing/service_bus_testing.hpp>

#include <fep3/rpc_services/base/fep_rpc_client.h>
#include <fep3/rpc_services/configuration/configuration_rpc_intf_def.h>

using namespace fep3;
using namespace ::testing;
using namespace a_util::strings;

const std::string default_type = fep3::PropertyType<std::string>::getTypeName();

class TestClient : public fep3::rpc::RPCServiceClient<::test::rpc_stubs::TestConfigurationServiceProxy, fep3::rpc::IRPCConfigurationDef>
{
private:
    using base_type = fep3::rpc::RPCServiceClient<::test::rpc_stubs::TestConfigurationServiceProxy, fep3::rpc::IRPCConfigurationDef> ;

public:
    using base_type::GetStub;

    TestClient(const char* server_object_name,
        std::shared_ptr<fep3::rpc::IRPCRequester> rpc) : base_type(server_object_name, rpc)
    {
    }    
};


struct NativeConfigurationServiceRPC : public ::testing::Test
{
    NativeConfigurationServiceRPC() :
        _configuration_service{ std::make_shared<fep3::native::ConfigurationService>() },
        _service_bus{ std::make_shared<fep3::native::ServiceBus>() },
        
        _component_registry { std::make_shared<fep3::ComponentRegistry>()}
    {}

    void SetUp() override
    {
        ASSERT_TRUE(fep3::native::testing::prepareServiceBusForTestingDefault(*_service_bus));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IServiceBus>(_service_bus));        
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IConfigurationService>(_configuration_service));
      
        ASSERT_FEP3_NOERROR(_component_registry->create());
    }

    std::shared_ptr<fep3::native::ConfigurationService> _configuration_service{};
    std::shared_ptr<fep3::native::ServiceBus> _service_bus{};    
    std::shared_ptr<fep3::ComponentRegistry> _component_registry{};
};

/**
 * @brief Tests the method getProperty of rpc configuration service
 * 
 */
TEST_F(NativeConfigurationServiceRPC, getProperty)
{
    TestClient client(fep3::rpc::IRPCConfigurationDef::getRPCDefaultName(),
                        _service_bus->getRequester(fep3::native::testing::test_participant_name));
                        
    auto properties_clock = createTestProperties();
    ASSERT_FEP3_NOERROR(_configuration_service->registerNode(properties_clock));    

    {
        EXPECT_EQ(client.getProperty("/Clock/Clocks/Clock1/CycleTime")["value"], "1");    
        EXPECT_EQ(client.getProperty("/Clock/Clocks/Clock1/CycleTime")["type"], "int");    

        EXPECT_EQ(client.getProperty("Clock/Clocks/Clock2/CycleTime")["value"], "2");
        EXPECT_EQ(client.getProperty("Clock/Clocks/Clock1/CycleTime")["type"], "int");

        EXPECT_EQ(client.getProperty("Clock/Clocks/Clock1")["value"], "my name");
        EXPECT_EQ(client.getProperty("Clock/Clocks/Clock1")["type"], "string");

        EXPECT_EQ(client.getProperty("Clock/Clocks")["value"], "2");
        EXPECT_EQ(client.getProperty("Clock/Clocks")["type"], "int");

        EXPECT_EQ(client.getProperty("Clock/Clocks/Clock2")["value"], "");
        EXPECT_EQ(client.getProperty("Clock/Clocks/Clock2")["type"], PropertyType<NodePropertyType>::getTypeName().c_str());
    
        EXPECT_EQ(client.getProperty("Clock")["value"], "");
        EXPECT_EQ(client.getProperty("Clock")["type"], PropertyType<NodePropertyType>::getTypeName().c_str());
    }

    // some special paths
    {
        EXPECT_EQ(client.getProperty("/Clock/Clocks/Clock1/CycleTime")["value"], "1");
        EXPECT_EQ(client.getProperty("Clock/Clocks/Clock1/CycleTime")["value"], "1");

        EXPECT_EQ(client.getProperty("Clock/Clocks\\Clock1\\CycleTime")["type"], "");


        EXPECT_EQ(client.getProperty("")["type"].asString(), "node");
        EXPECT_EQ(client.getProperty("/")["type"].asString(), "node");

        EXPECT_EQ(client.getProperty("\\")["type"], "");
    }
}

/**
 * @brief Tests the method existProperty of rpc configuration service
 * 
 */
TEST_F(NativeConfigurationServiceRPC, existProperty)
{
    TestClient client(fep3::rpc::IRPCConfigurationDef::getRPCDefaultName(),
        _service_bus->getRequester(fep3::native::testing::test_participant_name));

    auto properties_clock = createTestProperties();
    ASSERT_FEP3_NOERROR(_configuration_service->registerNode(properties_clock));

    EXPECT_TRUE(client.exists("/Clock/Clocks/Clock1/CycleTime"));
    EXPECT_TRUE(client.exists("Clock/Clocks/Clock2/CycleTime"));
    EXPECT_TRUE(client.exists("Clock/Clocks/Clock1"));
    EXPECT_TRUE(client.exists("Clock/Clocks"));
    EXPECT_TRUE(client.exists("Clock"));  
    EXPECT_TRUE(client.exists("Clock/Clocks/Clock2"));

    //the root node must exist because otherwise we have no possibility to find out the values of the first nodes
    EXPECT_TRUE(client.exists(""));
    EXPECT_TRUE(client.exists("/"));
    
    EXPECT_FALSE(client.exists("Clock/Clocks\\Clock1\\CycleTime"));
    EXPECT_FALSE(client.exists("not"));
    EXPECT_FALSE(client.exists("not/existing"));
     
    
    EXPECT_FALSE(client.exists("\\"));
}

/**
 * @brief Tests the method getProperties of rpc configuration service
 * 
 */
TEST_F(NativeConfigurationServiceRPC, getProperties)
{
    TestClient client(fep3::rpc::IRPCConfigurationDef::getRPCDefaultName(),
        _service_bus->getRequester(fep3::native::testing::test_participant_name));

    const auto properties_clock = createTestProperties("Clock");
    const auto properties_second = createTestProperties("second");
    ASSERT_FEP3_NOERROR(_configuration_service->registerNode(properties_clock));
    ASSERT_FEP3_NOERROR(_configuration_service->registerNode(properties_second));
    
    EXPECT_EQ(split(client.getProperties("Clock/Clocks/Clock2/CycleTime"), ",")
        , std::vector<std::string>());

    EXPECT_EQ(split(client.getProperties("Clock/Clocks/Clock2/"), ",")
        , std::vector<std::string>({ "CycleTime" }));

    EXPECT_EQ(split(client.getProperties("Clock/Clocks"), ","), std::vector<std::string>({
        "Clock1",
        "Clock2"}));

    EXPECT_EQ(split(client.getProperties("Clock"), ","), std::vector<std::string>({
       "Clocks"}));

    EXPECT_EQ(split(client.getProperties(""), ","), std::vector<std::string>({
       "system", "Clock" , "second"}));

    EXPECT_EQ(split(client.getProperties("/"), ","), std::vector<std::string>({
	   "system", "Clock" , "second"}));
}

/**
 * @brief Tests the method getProperties of rpc configuration service
 *
 */
TEST_F(NativeConfigurationServiceRPC, getAllProperties)
{
    TestClient client(fep3::rpc::IRPCConfigurationDef::getRPCDefaultName(),
        _service_bus->getRequester(fep3::native::testing::test_participant_name));

    const auto properties_clock = createTestProperties();
    ASSERT_FEP3_NOERROR(_configuration_service->registerNode(properties_clock));

    EXPECT_EQ(split(client.getAllProperties("Clock/Clocks/Clock2/CycleTime"), ",")
        , std::vector<std::string>({ }));

    EXPECT_EQ(split(client.getAllProperties("Clock/Clocks/Clock2"), ",")
        , std::vector<std::string>({ "Clock/Clocks/Clock2/CycleTime" }));

    EXPECT_EQ(split(client.getAllProperties("Clock/Clocks"), ","), std::vector<std::string>({
        "Clock/Clocks/Clock1",
        "Clock/Clocks/Clock1/CycleTime",
        "Clock/Clocks/Clock2",
        "Clock/Clocks/Clock2/CycleTime" }));

    EXPECT_EQ(split(client.getAllProperties("Clock"), ","), std::vector<std::string>({
       "Clock/Clocks",
       "Clock/Clocks/Clock1",
       "Clock/Clocks/Clock1/CycleTime",
       "Clock/Clocks/Clock2",
       "Clock/Clocks/Clock2/CycleTime" }));
}

/**
 * @brief It will be tested that the value of a property that exists can be changed 
 * via setProperty of rpc configuration service.
 *
 */
TEST_F(NativeConfigurationServiceRPC, setProperty_thatIsExisting)
{
    TestClient client(fep3::rpc::IRPCConfigurationDef::getRPCDefaultName(),
        _service_bus->getRequester(fep3::native::testing::test_participant_name));

    const auto properties_clock = createTestProperties();
    ASSERT_FEP3_NOERROR(_configuration_service->registerNode(properties_clock));       

    {
        EXPECT_EQ(properties_clock->getChild("Clocks")->getChild("Clock1")->getChild("CycleTime")->getValue(), "1");
        EXPECT_EQ(client.setProperty("Clock/Clocks/Clock1/CycleTime", "", "2"), 0);
        EXPECT_EQ(properties_clock->getChild("Clocks")->getChild("Clock1")->getChild("CycleTime")->getValue(), "2");
    }
}

/**
 * @brief It will be tested that the value of a property that has a different type
 * than provided via setProperty can not be set and an error is returned
 *
 */
TEST_F(NativeConfigurationServiceRPC, setProperty_differentType)
{
    TestClient client(fep3::rpc::IRPCConfigurationDef::getRPCDefaultName(),
        _service_bus->getRequester(fep3::native::testing::test_participant_name));

    const auto properties_clock = createTestProperties();
    ASSERT_FEP3_NOERROR(_configuration_service->registerNode(properties_clock));

    {       
        EXPECT_EQ(client.setProperty("Clock/Clocks/Clock1/CycleTime", "double", "2.0"), fep3::ResultType_ERR_INVALID_TYPE::getCode());
    }
}

/**
* @brief It is tested that a property that does not exist yet can not be created 
* via setProperty of rpc configuration service.
*
*/
TEST_F(NativeConfigurationServiceRPC, setProperty_thatIsNotExisting)
{
    TestClient client(fep3::rpc::IRPCConfigurationDef::getRPCDefaultName(),
        _service_bus->getRequester(fep3::native::testing::test_participant_name));

    const auto properties_clock = createTestProperties();
    ASSERT_FEP3_NOERROR(_configuration_service->registerNode(properties_clock));            
    
    EXPECT_EQ(client.setProperty("Clock/Clocks/Clock1/CycleTimeNew", "", "2"), fep3::ResultType_ERR_NOT_FOUND::getCode());
    EXPECT_EQ(client.setProperty("/", "", "2"), fep3::ResultType_ERR_INVALID_ARG::getCode());
    EXPECT_EQ(client.setProperty("\\", "", "2"), fep3::ResultType_ERR_INVALID_ARG::getCode());
    EXPECT_EQ(client.setProperty("", "", "2"), fep3::ResultType_ERR_INVALID_ARG::getCode());
}