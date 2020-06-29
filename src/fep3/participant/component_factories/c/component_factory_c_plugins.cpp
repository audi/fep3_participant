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

#include <fep3/plugin/c/c_host_plugin.h>
#include "component_factory_c_plugins.h"
#include "component_creator_c_plugin.h"
#include <fep3/components/base/c_access_wrapper/component_getter_function_getter.h>

// include access classes of all components that are exchangeable via Component C Plugin System here
#include <fep3/components/clock/c_access_wrapper/clock_service_c_access_wrapper.h>
#include <fep3/components/job_registry/c_access_wrapper/job_registry_c_access_wrapper.h>
#include <fep3/components/scheduler/c_access_wrapper/scheduler_service_c_access_wrapper.h>
#include <fep3/components/simulation_bus/c_access_wrapper/simulation_bus_c_access_wrapper.h>

namespace fep3
{
namespace arya
{

class ComponentFactoryCPlugin::Implementation
{
public:
    Implementation(const std::vector<std::string>& plugin_file_paths)
        : _plugin_file_paths(plugin_file_paths)
    {}
    std::vector<std::string> _plugin_file_paths;
    TypedComponentCreatorCPlugin
        // list of access object types of all components that are exchangeable via Component C Plugin System
        // note: the version namespace (arya, bronn, etc.) must be incorporated to support the creation of Components for different namespaces
        <fep3::plugin::c::access::arya::ClockService
        , fep3::plugin::c::access::arya::JobRegistry
        , fep3::plugin::c::access::arya::SchedulerService
        , fep3::plugin::c::access::arya::SimulationBus
        //, fep3::plugin::c::access::bronn::SimulationBus
        //, fep3::plugin::c::access::bronn::SchedulerService
        > _typed_component_creator;
};

ComponentFactoryCPlugin::ComponentFactoryCPlugin(const std::vector<std::string>& files) :
    _impl(std::make_unique<Implementation>(files))
{}

ComponentFactoryCPlugin::~ComponentFactoryCPlugin() = default;

std::unique_ptr<fep3::arya::IComponent> ComponentFactoryCPlugin::createComponent(const std::string& iid) const
{
    for(const auto& plugin_file_path : _impl->_plugin_file_paths)
    {
        const auto& plugin = std::make_shared<plugin::c::HostPlugin>(plugin_file_path);

        if(auto component = _impl->_typed_component_creator
            (plugin
            //, component_getter_function_getters
            // list of access object types of all components that can be accessed from within a C plugin
            // note: the version namespace (arya, bronn, etc.) must be incorporated to support access to components for different namespaces
            , std::make_shared<fep3::plugin::c::arya::ComponentGetterFunctionGetter
                <fep3::plugin::c::access::arya::ClockService
                , fep3::plugin::c::access::arya::JobRegistry
                , fep3::plugin::c::access::arya::SchedulerService
                , fep3::plugin::c::access::arya::SimulationBus
                >>()
            , iid
            ))
        {
            return component;
        }
    }
    return {};
}

}
}
