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

#include <fstream>
#include <iostream>

//this is an internal test ... so we test the internal implementation and include it!
#include "./../../../../../src/fep3/participant/core/component_factories/cpp/cpp_plugin.h"
#include "./../../../../../src/fep3/participant/core/component_factories/cpp/component_factory_cpp_plugins.h"
#include "./../../../../../src/fep3/participant/core/component_factories/components_factory.h"
#include "test_plugins/test_plugin_1_intf.h"
#include "test_plugins/test_plugin_2_intf.h"
#include "test_plugins/test_plugin_2_additional_intf.h"

const std::string test_plugin_1_path = PLUGIN_1;
const std::string test_plugin_2_path = PLUGIN_2;

/**
 * Test the loading and creating of a class from a CPPPlugin
 * @req_id 
*/
TEST(BaseCPPPluginTester, testLoading)
{
    using namespace fep3::arya;
    std::unique_ptr<CPPPlugin> _plugin;
    ASSERT_NO_THROW
    (
        _plugin = std::make_unique<CPPPlugin>(test_plugin_1_path);
    );
    auto component = _plugin->createComponent(ITestPlugin1::getComponentIID());
    ASSERT_TRUE(component);
    ITestPlugin1* testinterface = reinterpret_cast<ITestPlugin1*>(component->getInterface(ITestPlugin1::getComponentIID()));

    testinterface->set1(5);
    ASSERT_EQ(testinterface->get1(), 5);

    testinterface->set1(2000);
    ASSERT_EQ(testinterface->get1(), 2000);
}

/**
 * Test the loading and creating of a class from a CPPPluginFactory
 * @req_id 
*/
TEST(BaseCPPPluginTester, testComponentFactory)
{
    using namespace fep3::arya;
    std::unique_ptr<ComponentFactoryCPPPlugin> _factory;
    std::cout << "plugin path" << test_plugin_1_path;
    std::vector<std::string> plugins = { test_plugin_1_path, test_plugin_2_path };
    
    ASSERT_NO_THROW
    (
        _factory = std::make_unique<ComponentFactoryCPPPlugin>(plugins);
    );

    {
        //plugin 1
        std::unique_ptr<IComponent> component = _factory->createComponent(ITestPlugin1::getComponentIID());
        ASSERT_TRUE(component);
        ITestPlugin1* testinterface = reinterpret_cast<ITestPlugin1*>(component->getInterface(ITestPlugin1::getComponentIID()));

        testinterface->set1(5);
        ASSERT_EQ(testinterface->get1(), 5);

        testinterface->set1(2000);
        ASSERT_EQ(testinterface->get1(), 2000);
    }

    {
        //plugin 2
        std::unique_ptr<IComponent> component = _factory->createComponent(ITestPlugin2::getComponentIID());
        ASSERT_TRUE(component);
        ITestPlugin2* testinterface = reinterpret_cast<ITestPlugin2*>(component->getInterface(ITestPlugin2::getComponentIID()));

        testinterface->set2(5);
        ASSERT_EQ(testinterface->get2(), 5);

        testinterface->set2(2000);
        ASSERT_EQ(testinterface->get2(), 2000);
    }
}


const std::string components_file_path_target = std::string(TEST_BUILD_DIR) + "/test.fep_components";
const std::string components_file_path_source = "files/test.fep_components";
const std::string components_file_path_source_invalid = "files/test_invalid_type.fep_components";

std::string getExistingTestFile(const std::string& source_path)
{
    std::string found_source = source_path;
    if (!a_util::filesystem::exists(found_source))
    {
        found_source = "../" + source_path;
        if (!a_util::filesystem::exists(found_source))
        {
            return "";
        }
    }
    return found_source;
}

/**
 * Test the loading and creating of a class from ComponentsFactory which internally uses the CCPPluginFactory
 * @req_id FEPSDK-Factory
 */
TEST(BaseCPPPluginTester, testComponentsFactoryUsingCPP)
{
    {
        std::string found_source = getExistingTestFile(components_file_path_source);
        std::cout << "dir_source: " << found_source << "\n";
        std::ifstream in(found_source);
        std::ofstream out(components_file_path_target);

        out << in.rdbuf();
    }

    std::shared_ptr<fep3::arya::ComponentRegistry> registry;
    std::cout << "dir_target: " << components_file_path_target << "\n";
    ASSERT_NO_THROW
    (
        registry = fep3::arya::ComponentsFactory::createComponentsByFile(components_file_path_target);
    );

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
}


/**
 * Test the loading and creating of a class from ComponentsFactory
 * which internally uses the CCPPluginFactory but this test check if the source type is invalid
 * @req_id FEPSDK-Factory
 */
TEST(BaseCPPPluginTester, testComponentsFactoryUsingCPPAndFails)
{
    {
        std::string found_source = getExistingTestFile(components_file_path_source_invalid);
        std::ifstream in(found_source);
        std::ofstream out(components_file_path_target);

        out << in.rdbuf();
    }
    
    ASSERT_ANY_THROW
    (
        auto registry = fep3::arya::ComponentsFactory::createComponentsByFile(components_file_path_target);
    );
    
}
