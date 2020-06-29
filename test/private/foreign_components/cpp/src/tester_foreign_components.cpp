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

#include <fep3/fep3_macros.h>

#include <fep3/plugin/cpp/cpp_host_plugin.h>
#include <fep3/participant/component_factories/cpp/component_creator_cpp_plugin.h>
#include "test_plugins/test_plugin_1_intf.h"

const std::string test_plugin_1_path = PLUGIN_1;

/**
 * Test the loading and creating of a class from a CPPPlugin
 * @req_id 
*/
TEST(ForeignComponentsCPPPluginTester, testLoading)
{
    using namespace fep3::arya;
    std::unique_ptr<::fep3::plugin::cpp::HostPlugin> plugin;
    ASSERT_NO_THROW
    (
        plugin = std::make_unique<::fep3::plugin::cpp::HostPlugin>(test_plugin_1_path);
    );
    ASSERT_TRUE(plugin);
    EXPECT_EQ(plugin->getVersionNamespace(), "arya");
    EXPECT_EQ(plugin->getPluginVersion(), "0.0.1");
    EXPECT_EQ(plugin->getParticipantLibraryVersion()
        , (::fep3::plugin::arya::ParticipantLibraryVersion
            {FEP3_PARTICIPANT_LIBRARY_VERSION_ID
            , FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR
            , FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR
            , FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH
            , FEP3_PARTICIPANT_LIBRARY_VERSION_BUILD + 0 // + 0 because macro value is not set in developer versions
            })
        );

    auto component = ComponentCreatorCPPPlugin()(*plugin.get(), ITestPlugin1::getComponentIID());
    ASSERT_TRUE(component);
    ITestPlugin1* testinterface = reinterpret_cast<ITestPlugin1*>(component->getInterface(ITestPlugin1::getComponentIID()));

    testinterface->set1(5);
    ASSERT_EQ(testinterface->get1(), 5);

    testinterface->set1(2000);
    ASSERT_EQ(testinterface->get1(), 2000);
}