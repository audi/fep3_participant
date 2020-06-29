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

#include <fep3/plugin/c/c_host_plugin.h>
#include "test_plugins/plugin_1/class_a_intf.h"
#include "test_plugins/plugin_1/class_a_c_access_wrapper.h"
#include <fep3/fep3_macros.h>
#include <fep3/fep3_participant_version.h>

const std::string test_plugin_1_path = PLUGIN_1;

/**
 * Test creation of an object from within a C plugin
 * @req_id FEPSDK-1907 FEPSDK-1915
 * @note The CPlugin is a facility to enable the implementation of these requirements
 */
TEST(BaseCPluginTester, testObjectCreation)
{
    using namespace fep3::plugin::c::arya;
    using namespace fep3::plugin::arya;
    using namespace fep3::plugin::c::access::arya;
    std::shared_ptr<HostPlugin> test_plugin_1;
    ASSERT_NO_THROW
    (
        test_plugin_1 = std::make_shared<HostPlugin>(test_plugin_1_path);
    );
    EXPECT_EQ(test_plugin_1->getPluginVersion(), "0.0.1");
    EXPECT_EQ(test_plugin_1->getParticipantLibraryVersion()
        , (ParticipantLibraryVersion
            {FEP3_PARTICIPANT_LIBRARY_VERSION_ID
            , FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR
            , FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR
            , FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH
            , FEP3_PARTICIPANT_LIBRARY_VERSION_BUILD + 0 // + 0 because macro value is not set in developer versions
            })
        );

    std::unique_ptr<::test_plugin_1::IClassA> unique_ptr_to_object_from_plugin;
    ASSERT_NO_THROW
    (
        unique_ptr_to_object_from_plugin =
            test_plugin_1->create<::test_plugin_1::access::ClassA>("createClassA");
    );
    ASSERT_TRUE(unique_ptr_to_object_from_plugin);

    unique_ptr_to_object_from_plugin->set(1);
    ASSERT_EQ(unique_ptr_to_object_from_plugin->get(), 1);

    unique_ptr_to_object_from_plugin->set(2);
    ASSERT_EQ(unique_ptr_to_object_from_plugin->get(), 2);
}

/**
 * Test plugin loading failure
 * @req_id FEPSDK-1907 FEPSDK-1915
 * @note The CPlugin is a facility to enable the implementation of these requirements
 */
TEST(BaseCPluginTester, testPluginLoadingFailure)
{
    using namespace fep3::plugin::c::arya;
    using namespace fep3::plugin::c::access::arya;
    std::unique_ptr<::test_plugin_1::IClassA> unique_ptr_to_object_from_plugin;
    ASSERT_THROW
    (
        unique_ptr_to_object_from_plugin = std::make_shared<HostPlugin>("non_existent_plugin_path")
            ->create<::test_plugin_1::access::ClassA>("non_existent_create_function_name");
        , std::runtime_error
        );
    ASSERT_FALSE(unique_ptr_to_object_from_plugin);
}

/**
 * Test object creation failure
 * @req_id FEPSDK-1907 FEPSDK-1915
 * @note The CPlugin is a facility to enable the implementation of these requirements
 */
TEST(BaseCPluginTester, testObjectCreationFailure)
{
    using namespace fep3::plugin::c::arya;
    using namespace fep3::plugin::c::access::arya;
    std::unique_ptr<::test_plugin_1::IClassA> unique_ptr_to_object_from_plugin;
    ASSERT_THROW
    (
        unique_ptr_to_object_from_plugin = std::make_shared<HostPlugin>(test_plugin_1_path)
            ->create<::test_plugin_1::access::ClassA>("non_existent_create_function_name");
        , std::runtime_error
    );
    ASSERT_FALSE(unique_ptr_to_object_from_plugin);
}
