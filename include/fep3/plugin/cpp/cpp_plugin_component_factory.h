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

#pragma once

#include <fep3/components/base/component_iid.h>
#include <fep3/components/base/component_intf.h>

#include "cpp_plugin_component_factory_intf.h"

namespace fep3
{
namespace arya
{
    /**
     * @brief CPP plugin Compoennt factroy helper template to create one instance of the implementation \p component_impl_type.
     * 
     * @tparam component_impl_type the implementation type
     */
    template<class component_impl_type>
    class CPPPluginComponentFactory : public ICPPPluginComponentFactory
    {
    private:
        std::unique_ptr<IComponent> createComponent(const std::string& component_iid) const
        {
            if (component_iid == getComponentIID<component_impl_type>())
            {
                return std::unique_ptr<IComponent>(new component_impl_type());
            }
            else
            {
                return {};
            }
        }
    };
}
using arya::CPPPluginComponentFactory;
}

