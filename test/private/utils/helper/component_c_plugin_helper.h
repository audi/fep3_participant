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

#include <memory>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fep3/plugin/c/c_host_plugin.h>
#include <fep3/participant/component_factories/c/component_creator_c_plugin.h>
#include <fep3/components/base/c_access_wrapper/component_getter_function_getter.h>

using namespace fep3::plugin::c::arya;

class EmptyComponentGetterFunctionGetterFactory
{
public:
    std::shared_ptr<IComponentGetterFunctionGetter> operator()() const
    {
        return {};
    }
};

/**
 * Test class loading a mocked component from within a C plugin
 */
template
    <typename component_interface_type
    , typename mock_component_type
    , typename component_access_type
    , typename plugin_path_getter_type
    , typename set_mock_component_function_symbol_getter
    , typename component_getter_function_getter_factory = EmptyComponentGetterFunctionGetterFactory
    >
class MockedComponentCPluginLoader
{
protected:
    void SetUp()
    {
        ASSERT_NO_THROW
        (
            _plugin = std::make_shared<HostPlugin>(plugin_path_getter_type()())
        );

        // This is a bit hacky: the instantiation of the mock object must be in the memory of the test
        // (most likely because there are some global variables in gmock e. g. collecting uninteresting mock calls).
        // thus we create the mock component here and then pass it over to the plugin.
        _pointer_to_mock_component = new ::testing::StrictMock<mock_component_type>;
        {
            auto set_mock_component_function = _plugin->get<void(mock_component_type*)>(set_mock_component_function_symbol_getter()());
            ASSERT_NE(nullptr, set_mock_component_function);
            // transfer ownership of the mock component to the plugin
            set_mock_component_function(_pointer_to_mock_component);
        }

        ASSERT_NO_THROW
        (
            _unique_ptr_to_component = ::fep3::TypedComponentCreatorCPlugin<component_access_type>()
                (_plugin
                , component_getter_function_getter_factory()()
                , component_interface_type::getComponentIID()
                );
        );

        ASSERT_TRUE(_unique_ptr_to_component);
    }
    
    std::shared_ptr<HostPlugin> getPlugin() const
    {
        return _plugin;
    }

    component_interface_type* getComponent() const
    {
        // Remark: Downcast to interface is done by the component registry in the fep3 participant context.
        //         For a unit test that does not include testing of the component registry,
        //         we do it manually here:
        return dynamic_cast<component_interface_type*>(_unique_ptr_to_component.get());
    }
    
    /**
     * @brief Extracts the component from this
     * @note Subsequent calls to @ref getComponent will return a nullptr
     * 
     * @return Unique pointer to the component
     */
    std::unique_ptr<::fep3::IComponent> extractComponent()
    {
        return std::move(_unique_ptr_to_component);
    }

    ::testing::StrictMock<mock_component_type>& getMockComponent() const
    {
        return *_pointer_to_mock_component;
    }

private:
    std::shared_ptr<HostPlugin> _plugin;
    std::unique_ptr<::fep3::IComponent> _unique_ptr_to_component;
    ::testing::StrictMock<mock_component_type>* _pointer_to_mock_component;
};

/**
 * Test fixture class loading a single mocked component from within a C plugin
 */
template<typename mocked_component_c_plugin_loader_type>
class MockedComponentCPluginLoaderFixture 
    : public ::testing::Test
    , public mocked_component_c_plugin_loader_type
{
protected:
    void SetUp() override
    {
        mocked_component_c_plugin_loader_type::SetUp();
    }
};
