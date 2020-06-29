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

#include <memory>
#include <fep3/components/base/component_intf.h>

namespace fep3
{
namespace arya
{
    /**
     * The ICPPPluginComponentFactory interface must be provided by a cpp-plugin. 
     */
    class ICPPPluginComponentFactory
    {
    public:
        /**
         * @brief DTOR
         * 
         */
        virtual ~ICPPPluginComponentFactory() = default;
        /**
         * @brief creates and returns one instance of the given \p component_iid (component interface identifier).
         * @param component_iid component interface identifier
         * @return std::unique_ptr<IComponent> valid component instance for the component_iid (component interface identifier).
         * @retval std::unique_ptr<IComponent>() empty if not created 
         */
        virtual std::unique_ptr<IComponent> createComponent(const std::string& component_iid) const = 0;
    };
}
using arya::ICPPPluginComponentFactory;
}





