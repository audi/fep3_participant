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

#include <fep3/participant/component_registry_factory/components_file.h>
#include <fep3/participant/component_source_type.h>

/**
 * Test the loading and creating of a class from a CPPPlugin
 * @req_id FEPSDK-Plugin
 */
TEST(TestComponentsFile, testLoadingValidFiles)
{
    fep3::arya::ComponentsFile file_to_test;
    ASSERT_NO_THROW(
        file_to_test.load(CURRENT_TEST_DIR "files/valid.fep_components");
    );
    ASSERT_EQ(file_to_test.getItems().size(), 8);
    ASSERT_EQ(file_to_test.getFiles(fep3::arya::ComponentSourceType::unknown).size(), 0);
    ASSERT_EQ(file_to_test.getFiles(fep3::arya::ComponentSourceType::built_in).size(), 1);
    auto built_in_files = file_to_test.getFiles(fep3::arya::ComponentSourceType::built_in);
    for (auto& ref : built_in_files)
    {
        ASSERT_TRUE(ref.empty());
    }
    ASSERT_EQ(file_to_test.getFiles(fep3::arya::ComponentSourceType::c_plugin).size(), 3);
    auto cplugin_files = file_to_test.getFiles(fep3::arya::ComponentSourceType::c_plugin);
    for (auto& ref : cplugin_files)
    {
        ASSERT_NE(ref.find_first_of(fep3::getString(fep3::arya::ComponentSourceType::c_plugin), 0), std::string::npos);
    }
    ASSERT_EQ(file_to_test.getFiles(fep3::arya::ComponentSourceType::cpp_plugin).size(), 2);
    ASSERT_EQ(file_to_test.getFiles(fep3::arya::ComponentSourceType::unknown).size(), 0);
}

/**
 * Test the loading and creating of a class from a CPPPlugin
 * @req_id FEPSDK-Plugin
 */
TEST(TestComponentsFile, testLoadingInvalidFiles)
{
    fep3::arya::ComponentsFile file_to_test;
    ASSERT_ANY_THROW(
        file_to_test.load(CURRENT_TEST_DIR "files/doesnotexists.fep_components");
    );
    ASSERT_EQ(file_to_test.getItems().size(), 0);
    ASSERT_TRUE(file_to_test.getCurrentPath().empty());

    ASSERT_ANY_THROW(
        file_to_test.load(CURRENT_TEST_DIR "files/invalid_xml_syntax.fep_components");
    );
    ASSERT_EQ(file_to_test.getItems().size(), 0);
    ASSERT_TRUE(file_to_test.getCurrentPath().empty());

    ASSERT_ANY_THROW(
        file_to_test.load(CURRENT_TEST_DIR "files/invalid_sematic.fep_components");
    );
    ASSERT_EQ(file_to_test.getItems().size(), 0);
    ASSERT_TRUE(file_to_test.getCurrentPath().empty());

    ASSERT_ANY_THROW(
        file_to_test.load(CURRENT_TEST_DIR "files/invalid_schema_version.fep_components");
    );
    ASSERT_EQ(file_to_test.getItems().size(), 0);
    ASSERT_TRUE(file_to_test.getCurrentPath().empty());
}
