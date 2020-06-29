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

#include <map>
#include <memory>

#include <fep3/components/base/component_intf.h>
#include <fep3/plugin/c/c_host_plugin.h>
//#include <fep3/components/base/c_access_wrapper/component_getter.h>
#include <fep3/components/base/c_access_wrapper/component_getter_function_getter_intf.h>

namespace fep3
{
namespace arya
{

/**
 * Class creating components for specific access object types
 */
template<typename... component_access_object_types>
struct TypedComponentCreatorCPlugin
{
    /**
     * Does not create a component because no specific access object types are set
     * @note This class is the end of the compile time recursion for the specific access object types; see template specialization below
     * @return Empty unique pointer
     */
    std::unique_ptr<::fep3::IComponent> operator()
        (const std::shared_ptr<plugin::c::HostPlugin>& /*plugin*/
        , const std::shared_ptr<fep3::plugin::c::arya::IComponentGetterFunctionGetter>& /*component_getter_function_getters*/
        , const std::string& /*iid*/
        )
    {
        return {};
    }
};
/**
 * Specialization of above class for more than zero specific access object types
 */
template<typename component_access_object_type, typename... remaining_component_access_object_types>
struct TypedComponentCreatorCPlugin<component_access_object_type, remaining_component_access_object_types...>
{
    /**
     * Creates a component with @p iid in the plugin at @p plugin_file_path
     * @param plugin_file_path Path to the plugin file
     * @param iid The interface identifier of the component to be created from within the plugin at \p plugin_file_path
     * @return Unique pointer to the component if the plugin is capable to create a component with \p iid
     *          , empty unique pointer otherwise
     */
    std::unique_ptr<::fep3::IComponent> operator()
        (const std::shared_ptr<plugin::c::HostPlugin>& plugin
        , const std::shared_ptr<fep3::plugin::c::arya::IComponentGetterFunctionGetter>& component_getter_function_getters
        , const std::string& iid
        )
    {
        // for each component access object type try to create a component in the plugin for the given iid
        if(::fep3::getComponentIID<component_access_object_type>() == iid)
        {
            auto component = plugin->create<component_access_object_type>
                (component_access_object_type::create_function_name
                , iid.c_str()
                );
            // TODO maybe pass component_getter_function_getter to constructor of component_access_object_type
            if(component)
            {
                component->setComponentGetterFunctionGetter(component_getter_function_getters);
            }
            return component;
        }
        else
        {
            // compile time recursion: go on with remaining component access object types
            return TypedComponentCreatorCPlugin<remaining_component_access_object_types...>()
                (plugin
                , component_getter_function_getters
                , iid
                );
        }
    }
};

} // namespace arya
using arya::TypedComponentCreatorCPlugin;
} // namespace fep3
