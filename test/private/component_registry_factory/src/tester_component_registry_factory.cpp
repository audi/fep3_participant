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
#include <gtest/gtest.h>

#include <a_util/filesystem.h>
#include <a_util/system.h>

#include <fstream>
#include <iostream>

#include <fep3/base/environment_variable/environment_variable.h>
#include <fep3/participant/component_registry_factory/component_registry_factory.h>
// interfaces of native components
#include <fep3/components/data_registry/data_registry_intf.h>
#include <fep3/native_components/data_registry/data_registry.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/components/job_registry/job_registry_intf.h>
#include <fep3/components/scheduler/scheduler_service_intf.h>
#include <fep3/components/service_bus/service_bus_intf.h>
#include <fep3/native_components/service_bus/service_bus.h>
#include <fep3/components/simulation_bus/simulation_bus_intf.h>
#include <fep3/native_components/simulation_bus/simulation_bus.h>
#include <fep3/components/clock_sync/clock_sync_service_intf.h>
#include <fep3/components/simulation_bus/c_access_wrapper/simulation_bus_c_access_wrapper.h>
// interfaces of CPP plugins
#include <test_plugins/test_plugin_1_intf.h>
#include <test_plugins/test_plugin_2_intf.h>
#include <test_plugins/test_plugin_2_additional_intf.h>
// interfaces of CPP plugins
#include <test_plugins/plugin_1/component_a_intf.h>

const std::string components_file_path_source = std::string(TEST_BUILD_DIR) + "/files/test.fep_components";
const std::string invalid_components_file_path_source = std::string(TEST_BUILD_DIR) + "/files/test_invalid_type.fep_components";

/**
 * @brief Copies a file from @p source to @p destination
 * @param source The source file path
 * @param destination The destination file path
 * @return true if successful, false otherwise
 */
bool copyFile(const a_util::filesystem::Path& source, const a_util::filesystem::Path& destination)
{
    if(a_util::filesystem::exists(source))
    {
        std::ifstream in(source);
        std::ofstream out(destination);
        out << in.rdbuf();
        return true;
    }
    return false;
}

/**
 * Test the creation of a component registry with default components, i. e. not using a components configuration file
 * @req_id FEPSDK-Factory
 */
TEST(ComponentRegistryFactoryTester, testComponentRegistryCreationDefault)
{
    // make sure the default components configuration file is not in the current working directory 
    // (e. g. due to previous test runs)
    a_util::filesystem::remove(std::string(TEST_BUILD_DIR) + "/fep3_participant.fep_components");
    
    std::shared_ptr<fep3::arya::ComponentRegistry> registry;
    ASSERT_NO_THROW
    (
        registry = fep3::arya::ComponentRegistryFactory::createRegistry();
    );

    // native components
    {
        auto test_interface = registry->getComponent<fep3::IDataRegistry>();
        ASSERT_TRUE(test_interface != nullptr);
        EXPECT_NE(nullptr, dynamic_cast<fep3::native::DataRegistry*>(test_interface));
    }
    {
        auto test_interface = registry->getComponent<fep3::IServiceBus>();
        ASSERT_TRUE(test_interface != nullptr);
        EXPECT_NE(nullptr, dynamic_cast<fep3::native::ServiceBus*>(test_interface));
    }
    {
        auto test_interface = registry->getComponent<fep3::ISimulationBus>();
        ASSERT_TRUE(test_interface != nullptr);
        EXPECT_NE(nullptr, dynamic_cast<fep3::native::SimulationBus*>(test_interface));
    }
    {
        auto test_interface = registry->getComponent<fep3::IClockService>();
        ASSERT_TRUE(test_interface != nullptr);
        EXPECT_NE(nullptr, dynamic_cast<fep3::IClockService*>(test_interface));
    }
    {
        auto test_interface = registry->getComponent<fep3::IClockSyncService>();
        ASSERT_TRUE(test_interface != nullptr);
        EXPECT_NE(nullptr, dynamic_cast<fep3::IClockSyncService*>(test_interface));
    }
    {
        auto test_interface = registry->getComponent<fep3::IJobRegistry>();
        ASSERT_TRUE(test_interface != nullptr);
        EXPECT_NE(nullptr, dynamic_cast<fep3::IJobRegistry*>(test_interface));
    }
    {
        auto test_interface = registry->getComponent<fep3::ISchedulerService>();
        ASSERT_TRUE(test_interface != nullptr);
        EXPECT_NE(nullptr, dynamic_cast<fep3::ISchedulerService*>(test_interface));
    }
}

/**
 * Test the creation of a component registry according to a components configuration file
 * given by an environment variable
 * @req_id FEPSDK-Factory
 */
TEST(ComponentRegistryFactoryTester, testComponentRegistryCreationByEnvVar)
{
    // make sure the default components configuration file is not in the current working directory 
    // (e. g. due to previous test runs)
    a_util::filesystem::remove(std::string(TEST_BUILD_DIR) + "/fep3_participant.fep_components");
    // copy source to the non-default file path
    const std::string non_default_file_path(std::string(TEST_BUILD_DIR) + "/non-default-file-name.fep_components");
    ASSERT_TRUE(copyFile(components_file_path_source, non_default_file_path));
    ASSERT_EQ(::fep3::Result{}, ::fep3::environment_variable::set("FEP3_PARTICIPANT_COMPONENTS_FILE_PATH", non_default_file_path));
    
    std::shared_ptr<fep3::arya::ComponentRegistry> registry;
    ASSERT_NO_THROW
    (
        registry = fep3::arya::ComponentRegistryFactory::createRegistry();
    );

    // check one of the non-default components
    {
        auto test_interface = registry->getComponent<ITestPlugin1>();
        ASSERT_NE(nullptr, test_interface);
    }
}

/**
 * Test the creation of a component registry according to a components configuration file 
 * in the current working directory containing components of types
 * * native
 * * cpp-plugin
 * * c-plugin
 * @req_id FEPSDK-Factory
 */
TEST(ComponentRegistryFactoryTester, testComponentRegistryCreationByFile)
{
    ASSERT_TRUE(copyFile(components_file_path_source, std::string(TEST_BUILD_DIR) + "/fep3_participant.fep_components"));
    
    std::shared_ptr<fep3::arya::ComponentRegistry> registry;
    ASSERT_NO_THROW
    (
        registry = fep3::arya::ComponentRegistryFactory::createRegistry();
    );

    // native components
    {
        auto test_interface = registry->getComponent<fep3::IDataRegistry>();
        ASSERT_TRUE(test_interface != nullptr);
        EXPECT_NE(nullptr, dynamic_cast<fep3::native::DataRegistry*>(test_interface));
    }
    {
        auto test_interface = registry->getComponent<fep3::IServiceBus>();
        ASSERT_TRUE(test_interface != nullptr);
        EXPECT_NE(nullptr, dynamic_cast<fep3::native::ServiceBus*>(test_interface));
    }

    // components from CPP plugins
    {
        auto testinterface = registry->getComponent<ITestPlugin1>();
        ASSERT_TRUE(testinterface != nullptr);

        testinterface->set1(5);
        ASSERT_EQ(testinterface->get1(), 5);

        testinterface->set1(2000);
        ASSERT_EQ(testinterface->get1(), 2000);
    }
    {
        //plugin 2
        auto testinterface = registry->getComponent<ITestPlugin2>();
        ASSERT_TRUE(testinterface != nullptr);

        testinterface->set2(5);
        ASSERT_EQ(testinterface->get2(), 5);

        testinterface->set2(2000);
        ASSERT_EQ(testinterface->get2(), 2000);
    }
    {
        //plugin 2
        auto testinterface = registry->getComponent<ITestPlugin2Additional>();
        ASSERT_TRUE(testinterface != nullptr);

        ASSERT_EQ(testinterface->getAdditional(), "additional");
    }
    
    // components from C plugins
    {
        auto test_interface = registry->getComponent<::fep3::ISimulationBus>();
        ASSERT_TRUE(test_interface != nullptr);
        EXPECT_NE(nullptr, dynamic_cast<::fep3::plugin::c::access::arya::SimulationBus*>(test_interface));
    }
}

/**
 * Test failure of component registry creation if the components configuration file does not exist
 * @req_id FEPSDK-Factory
 */
TEST(ComponentRegistryFactoryTester, testComponentRegistryCreationFailureOnNonExistingFileInEnvVar)
{
    ::fep3::environment_variable::set("FEP3_PARTICIPANT_COMPONENTS_FILE_PATH", "non-existing-file-path");
    ASSERT_ANY_THROW
    (
        auto registry = fep3::arya::ComponentRegistryFactory::createRegistry();
    );
}

/**
 * Test failure of component registry creation if the components configuration file is invalid
 * @req_id FEPSDK-Factory
 */
TEST(ComponentRegistryFactoryTester, testComponentRegistryCreationFailureOnInvalidFile)
{
    ASSERT_TRUE(copyFile(invalid_components_file_path_source, std::string(TEST_BUILD_DIR) + "/fep3_participant.fep_components"));
    ASSERT_ANY_THROW
    (
        auto registry = fep3::arya::ComponentRegistryFactory::createRegistry();
    );

}


