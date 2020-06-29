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
#include "component_factory_cpp_plugins.h"
#include "component_creator_cpp_plugin.h"
#include <fep3/plugin/cpp/cpp_host_plugin.h>

namespace fep3
{
namespace arya
{
    struct ComponentFactoryCPPPlugin::Implementation
    {
        Implementation()
        {
        }
        ~Implementation()
        {
        }

        void loadAllPlugins(const std::vector<std::string>& files)
        {
            for (const auto& file : files)
            {
                _plugins.emplace_back(file);
            }
        }
        std::vector<plugin::cpp::HostPlugin> _plugins;
    };

    ComponentFactoryCPPPlugin::ComponentFactoryCPPPlugin(const std::vector<std::string>& files) : 
        _impl(std::make_unique<Implementation>())
    {
        _impl->loadAllPlugins(files);
    }
    ComponentFactoryCPPPlugin::~ComponentFactoryCPPPlugin()
    {
    }
    std::unique_ptr<fep3::arya::IComponent> ComponentFactoryCPPPlugin::createComponent(const std::string& iid) const
    {
        for (const auto& plugin : _impl->_plugins)
        {
            if(auto component = ComponentCreatorCPPPlugin()(plugin, iid))
            {
                return component;
            }
        }
        return {};
    }
}
}
