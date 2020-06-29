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

#include <fep3/components/base/component_base.h>
#include <fep3/components/base/c_intf/component_c_intf.h>
#include <fep3/plugin/c/destruction_manager.h>
#include <fep3/plugin/c/shared_binary_manager.h>
#include "component_getter.h"
#include <fep3/components/base/c_access_wrapper/components_c_access.h>

namespace fep3
{
namespace plugin
{
namespace c
{
namespace arya
{

/**
 * @brief Decorator class to make a component transferable over a C interface.
 * 
 * @tparam interface_type The type of the interface to be transferred
 */
template<typename interface_type>
class TransferableComponentBase
    : public ::fep3::arya::ComponentBase<interface_type>
    , public SharedBinaryManager // enable lifetime management of binary
    , public DestructionManager // enable destruction of remote objects (i. e. objects on other side of binary boundaries)
{
public:
    /**
     * Default CTOR
     */
    inline TransferableComponentBase() = default;
    /**
     * CTOR taking deductors for the type deduction of the component access types
     * 
     * @param component_getter The component getter providing access to other components
     */
    inline TransferableComponentBase(const std::shared_ptr<::fep3::plugin::c::arya::IComponentGetter>& component_getter)
        : _component_getter(component_getter)
    {}
    /**
     * Default DTOR
     */
    inline virtual ~TransferableComponentBase() = default;
    
    // methods overriding ::fep3::arya::ComponentBase
    /**
     * Creates a component and sets up access to remote components
     * 
     * @param components Weak pointer to all components to setup access for
     * @return FEP Result
     */
    inline ::fep3::Result createComponent(const std::weak_ptr<const fep3::arya::IComponents>& components) override
    {
        // note: non-lockable weak_ptr is not an error, there are just no components to access
        if(const auto& shared_components = components.lock())
        {
            // need downcast because the IComponents interface has no setComponentGetter
            // and this method shall not be added because it is specific to the C plugin system
            if(auto remote_components = dynamic_cast<::fep3::plugin::c::access::arya::Components*>
                (const_cast<fep3::arya::IComponents*>(shared_components.get())))
            {
                remote_components->setComponentGetter(_component_getter);
            }
        }
        return ::fep3::arya::ComponentBaseImpl::createComponent(components);
    }

private:
    std::shared_ptr<::fep3::plugin::c::arya::IComponentGetter> _component_getter;
};

} // namespace arya
using arya::TransferableComponentBase;
} // namespace c
} // namespace plugin
} // namespace fep3
