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

#include <fep3/plugin/c/c_access/c_access_helper.h>
#include <fep3/components/base/c_intf/component_c_intf.h>
#include <fep3/components/base/components_intf.h>
#include <fep3/components/base/c_access_wrapper/component_getter.h>

namespace fep3
{
namespace plugin
{
namespace c
{
namespace access
{
namespace arya
{

/**
 * @brief Access class for @ref fep3::arya::IComponents
 */
class Components
    : public fep3::arya::IComponents
    , private fep3::plugin::c::arya::DestructionManager
    , private fep3::plugin::c::access::arya::Helper
{
public:
    /// Type of access structure
    using Access = fep3_arya_SComponents;

    /**
     * CTOR
     * @param access The C access structure for the remote object
     * @param destructors List of destructors to be called upon destruction of this
     * @param handle_to_component_getter_function_getters The handle to the component getter function getters to be set
     */
    inline Components
        (const Access& access
        , std::deque<std::unique_ptr<IDestructor>> destructors
        , fep3_plugin_c_arya_HComponentGetterFunctionGetters handle_to_component_getter_function_getters
        );
    /// Default DTOR
    inline virtual ~Components() override = default;

    /**
     * @brief Sets the @p component_getter to this
     * 
     * @param component_getter The component getter to set
     */
    inline void setComponentGetter
        (const std::shared_ptr<fep3::plugin::c::arya::IComponentGetter>& component_getter);

    // methods implementing ::fep3::arya::IComponents
    inline IComponent* findComponent(const std::string& fep_iid) const override;

private:
    Access _access;
    std::shared_ptr<fep3::plugin::c::arya::IComponentGetter> _component_getter;
    fep3_plugin_c_arya_HComponentGetterFunctionGetters _handle_to_component_getter_function_getters;
};

Components::Components
    (const Access& access
    , std::deque<std::unique_ptr<IDestructor>> destructors
    , fep3_plugin_c_arya_HComponentGetterFunctionGetters handle_to_component_getter_function_getters
    )
    : _access(access)
    , _handle_to_component_getter_function_getters(handle_to_component_getter_function_getters)
{
    addDestructors(std::move(destructors));
}

void Components::setComponentGetter
    (const std::shared_ptr<::fep3::plugin::c::arya::IComponentGetter>& component_getter)
{
    _component_getter = component_getter;
}

IComponent* Components::findComponent(const std::string& fep_iid) const
{
    auto component_interface_access = Helper::callWithResultParameter
        (_access._handle
        , _access.findComponent
        , _handle_to_component_getter_function_getters
        , fep_iid.c_str()
        );
    if (_component_getter)
    {
        return _component_getter->operator()
            (component_interface_access.getComponent
                , fep_iid
                , component_interface_access._handle
            );
    }

    return nullptr;
}

} // namespace arya
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3
