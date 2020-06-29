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

#include <fep3/components/base/component_intf.h>
#include <fep3/plugin/cpp/cpp_host_plugin.h>
#include <fep3/plugin/cpp/cpp_plugin_component_factory_intf.h>

namespace fep3
{
namespace arya
{

/**
 * Functor creating one component from with in a Component CPP Plugin
 */
class ComponentCreatorCPPPlugin
{
public:
    /**
     * Creates one component with @p iid from within the @p plugin
     * @param plugin The plugin to create the component from within
     * @param iid The IID of the component to be created
     * @throw std::runtime_error if the plugin does not expose an appropriate function @ref SYMBOL_fep3_getFactory
     * @return Unique pointer to the component if the plugin was capable to create a component with @ iid
     *         , empty unique pointer otherwise
     */
    std::unique_ptr<::fep3::IComponent> operator()(const plugin::cpp::HostPlugin& plugin, const std::string& iid)
    {
        const auto& plugin_version_namespace = plugin.getVersionNamespace();
        // map version namespace to factory function symbol name
        if("arya" == plugin_version_namespace)
        {
            // create an instance of the component factory from within the plugin
            if(const auto& component_factory = plugin.create<arya::ICPPPluginComponentFactory>(SYMBOL_fep3_plugin_cpp_arya_getFactory))
            {
                return component_factory->createComponent(iid);
            }
            else
            {
                throw std::runtime_error("The plugin '" + plugin.getFilePath() + "' returned an invalid factory.");
            }
        }
        else
        {
            throw std::runtime_error(std::string() + "This participant does not support the creation of "
                + " a component from within a plugin with version namespace '"
                + plugin_version_namespace + "'.");
        }
    }
};

} // namespace arya
using arya::ComponentCreatorCPPPlugin;
} // namespace fep3
