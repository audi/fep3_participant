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

#include <fep3/plugin/c/c_host_plugin.h>
#include <fep3/participant/component_factories/c/component_creator_c_plugin.h>
#include "test_plugins/plugin_1/component_a_intf.h"
#include "test_plugins/plugin_1/component_a_c_access_wrapper.h"
#include "test_plugins/plugin_1/mock_component_a.h"
#include "test_plugins/plugin_1/component_b_intf.h"
#include "test_plugins/plugin_1/component_b_c_access_wrapper.h"
#include "test_plugins/plugin_1/mock_component_b.h"
#include "test_plugins/plugin_2/component_c_intf.h"
#include "test_plugins/plugin_2/component_c_c_access_wrapper.h"
#include "test_plugins/plugin_2/mock_component_c.h"
#include <fep3/components/base/component_intf.h>
#include <fep3/components/base/component_registry.h>
#include <fep3/components/base/c_access_wrapper/component_getter_function_getter.h>
#include <helper/component_c_plugin_helper.h>

struct Plugin1PathGetter
{
    std::string operator()() const
    {
        return PLUGIN_1;
    }
};
struct SetMockComponentAFunctionSymbolGetter
{
    std::string operator()() const
    {
        return "setMockComponentA";
    }
};
struct ComponentABCGetterFunctionGetterFactory
{
public:
    std::shared_ptr<fep3::plugin::c::arya::IComponentGetterFunctionGetter> operator()()
    {
        return std::make_shared<fep3::plugin::c::arya::ComponentGetterFunctionGetter
            <::test_plugin_1::access::ComponentA
            , ::test_plugin_1::access::ComponentB
            , ::test_plugin_2::access::ComponentC
            >>();
    }
};

/**
 * Test fixture class loading a mocked component A from within a C plugin
 */
using ComponentALoader = MockedComponentCPluginLoader
    <::test_plugin_1::IComponentA
    , ::test_plugin_1::mock::MockComponentA
    , ::test_plugin_1::access::ComponentA
    , Plugin1PathGetter
    , SetMockComponentAFunctionSymbolGetter
    , ComponentABCGetterFunctionGetterFactory
    >;
using ComponentALoaderFixture = MockedComponentCPluginLoaderFixture<ComponentALoader>;

/**
 * Test creation of a component from within a C plugin
 * @req_id FEPSDK-1907 FEPSDK-1915
 * @note The "ForeignComponent" is a facility to enable the implementation of these requirements
 */
TEST_F(ComponentALoaderFixture, testComponentCreation)
{
    auto& mock_component_a = getMockComponent();
    EXPECT_CALL(mock_component_a, get()).WillOnce(::testing::Return(33));
    EXPECT_CALL(mock_component_a, die()).WillOnce(::testing::Return());
    
    ::test_plugin_1::IComponentA* component_a = getComponent();
    ASSERT_NE(nullptr, component_a);
    // test calling a method
    EXPECT_EQ(33, component_a->get());
}

/**
 * @detail Test getting pointer to the public component interface for a component that resides in a C plugin
 * @req_id FEPSDK-1907 FEPSDK-1915
 * @note The "ForeignComponent" is a facility to enable the implementation of these requirements
 */
TEST_F(ComponentALoaderFixture, testGettingSpecificComponent)
{
    auto& mock_component_a = getMockComponent();
    EXPECT_CALL(mock_component_a, set(44)).WillOnce(::testing::Return());
    EXPECT_CALL(mock_component_a, get()).WillOnce(::testing::Return(55));
    EXPECT_CALL(mock_component_a, die()).WillOnce(::testing::Return());

    const auto& component_registry = std::make_shared<::fep3::ComponentRegistry>();
    {
        // test registering at component registry (this is a precondition for getting the component via "getComponent" from the component registry)
        ASSERT_EQ(::fep3::Result(), component_registry->registerComponent<test_plugin_1::IComponentA>(extractComponent()));

        test_plugin_1::IComponentA* pointer_to_component_a = component_registry->getComponent<test_plugin_1::IComponentA>();
        ASSERT_NE(pointer_to_component_a, nullptr);

        // call some methods of the specific component interface
        pointer_to_component_a->set(44);
        EXPECT_EQ(pointer_to_component_a->get(), 55);
    }
}

struct SetMockComponentBFunctionSymbolGetter
{
    std::string operator()() const
    {
        return "setMockComponentB";
    }
};

/**
 * @detail Test accessing a component that resides in the host from within the implementation of a component that resides in a C plugin
 * @req_id FEPSDK-1907 FEPSDK-1915
 * @note The "ForeignComponent" is a facility to enable the implementation of these requirements
 */
TEST_F(ComponentALoaderFixture, testAccessingHostComponent)
{
    // component A resides in the plugin
    auto& mock_component_a = ComponentALoader::getMockComponent();
    EXPECT_CALL(mock_component_a, die()).WillOnce(::testing::Return());
    
    // component B resides in the host
    auto mock_component_b = std::make_unique<::test_plugin_1::mock::MockComponentB>();
    EXPECT_CALL(*mock_component_b, get()).WillOnce(::testing::Return(55));
    EXPECT_CALL(*mock_component_b, die()).WillOnce(::testing::Return());
    
    const auto& component_registry = std::make_shared<::fep3::ComponentRegistry>();
    {
        ASSERT_EQ(::fep3::Result(), component_registry->registerComponent<test_plugin_1::IComponentA>
            (ComponentALoader::extractComponent()
            ));
        ASSERT_EQ(::fep3::Result(), component_registry->registerComponent<test_plugin_1::IComponentB>(std::move(mock_component_b)));
        // create the components through the component registry (this is a precondition for accessing the component from within another component)
        component_registry->create();
    }
    
    ::test_plugin_1::IComponentA* pointer_to_component_a = component_registry->getComponent<test_plugin_1::IComponentA>();
    ASSERT_NE(pointer_to_component_a, nullptr);
    EXPECT_EQ(55, pointer_to_component_a->getFromComponentB());
}

/**
 * Test fixture class loading a mocked component B from within a C plugin
 */
using ComponentBLoader = MockedComponentCPluginLoader
    <::test_plugin_1::IComponentB
    , ::test_plugin_1::mock::MockComponentB
    , ::test_plugin_1::access::ComponentB
    , Plugin1PathGetter
    , SetMockComponentBFunctionSymbolGetter
    , ComponentABCGetterFunctionGetterFactory
    >;
class ComponentABLoaderFixture
    : public ::testing::Test
    , public ComponentALoader
    , public ComponentBLoader
{
protected:
    void SetUp() override
    {
        ComponentALoader::SetUp();
        ComponentBLoader::SetUp();
    }
};

/**
 * @detail Test accessing a component that resides in a C plugin from within the implementation of a component that resides in the same C plugin
 * @req_id FEPSDK-1907 FEPSDK-1915
 * @note The "ForeignComponent" is a facility to enable the implementation of these requirements
 */
TEST_F(ComponentABLoaderFixture, testAccessingOtherPluginComponent)
{
    auto& mock_component_a = ComponentALoader::getMockComponent();
    EXPECT_CALL(mock_component_a, die()).WillOnce(::testing::Return());
    
    auto& mock_component_b = ComponentBLoader::getMockComponent();
    EXPECT_CALL(mock_component_b, get()).WillOnce(::testing::Return(55));
    EXPECT_CALL(mock_component_b, die()).WillOnce(::testing::Return());
    
    const auto& component_registry = std::make_shared<::fep3::ComponentRegistry>();
    {
        ASSERT_EQ(::fep3::Result(), component_registry->registerComponent<test_plugin_1::IComponentA>(ComponentALoader::extractComponent()));
        ASSERT_EQ(::fep3::Result(), component_registry->registerComponent<test_plugin_1::IComponentB>(ComponentBLoader::extractComponent()));
        // create the components through the component registry (this is a precondition for accessing the component from within another component)
        component_registry->create();
    }
    
    ::test_plugin_1::IComponentA* pointer_to_component_a = component_registry->getComponent<test_plugin_1::IComponentA>();
    ASSERT_NE(pointer_to_component_a, nullptr);
    EXPECT_EQ(55, pointer_to_component_a->getFromComponentB());
}

struct Plugin2PathGetter
{
    std::string operator()() const
    {
        return PLUGIN_2;
    }
};
struct SetMockComponentCFunctionSymbolGetter
{
    std::string operator()() const
    {
        return "setMockComponentC";
    }
};

/**
 * Test fixture class loading a mocked component C from within a C plugin
 */
using ComponentCLoader = MockedComponentCPluginLoader
    <::test_plugin_2::IComponentC
    , ::test_plugin_2::mock::MockComponentC
    , ::test_plugin_2::access::ComponentC
    , Plugin2PathGetter
    , SetMockComponentCFunctionSymbolGetter
    >;
class ComponentACLoaderFixture
    : public ::testing::Test
    , public ComponentALoader
    , public ComponentCLoader
{
protected:
    void SetUp() override
    {
        ComponentALoader::SetUp();
        ComponentCLoader::SetUp();
    }
};

/**
 * @detail Test accessing one component that resides in a C plugin from within the implementation of another component that resides in another C plugin
 * @req_id FEPSDK-1907 FEPSDK-1915
 * @note The "ForeignComponent" is a facility to enable the implementation of these requirements
 */
TEST_F(ComponentACLoaderFixture, testAccessingOtherPluginComponent)
{
    auto& mock_component_a = ComponentALoader::getMockComponent();
    EXPECT_CALL(mock_component_a, die()).WillOnce(::testing::Return());
    
    auto& mock_component_c = ComponentCLoader::getMockComponent();
    EXPECT_CALL(mock_component_c, get()).WillOnce(::testing::Return(66));
    EXPECT_CALL(mock_component_c, die()).WillOnce(::testing::Return());
    
    const auto& component_registry = std::make_shared<::fep3::ComponentRegistry>();
    {
        ASSERT_EQ(::fep3::Result(), component_registry->registerComponent<test_plugin_1::IComponentA>(ComponentALoader::extractComponent()));
        ASSERT_EQ(::fep3::Result(), component_registry->registerComponent<test_plugin_2::IComponentC>(ComponentCLoader::extractComponent()));
        // create the components through the component registry (this is a precondition for accessing the component from within another component)
        component_registry->create();
    }
    
    ::test_plugin_1::IComponentA* pointer_to_component_a = component_registry->getComponent<test_plugin_1::IComponentA>();
    ASSERT_NE(pointer_to_component_a, nullptr);
    EXPECT_EQ(66, pointer_to_component_a->getFromComponentC());
}
