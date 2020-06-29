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

#include <fep3/fep3_participant_types.h>
#include <fep3/components/base/component_intf.h>
#include <string>

namespace fep3
{
namespace arya
{
    class ComponentFactoryBase
    {
        public:
            ComponentFactoryBase() = default;
            virtual ~ComponentFactoryBase() = default;
            ComponentFactoryBase(ComponentFactoryBase&&) = default;
            ComponentFactoryBase& operator=(ComponentFactoryBase&&) = default;

        protected:
            ComponentFactoryBase(const ComponentFactoryBase&) = delete;
            ComponentFactoryBase& operator=(const ComponentFactoryBase&) = delete;

        public:
            virtual std::unique_ptr<fep3::arya::IComponent> createComponent(const std::string& /*iid*/) const 
            {
                return {};
            }
    };
}
}