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
#include <fep3/participant/component_factories/component_factory_base.h>
#include <fep3/participant/component_source_type.h>
#include <fep3/components/base/component_registry.h>

namespace fep3
{
namespace arya
{
    class ComponentFactoryBuiltIn : public ComponentFactoryBase
    {
        public:
            ComponentFactoryBuiltIn();
            virtual ~ComponentFactoryBuiltIn();
            std::unique_ptr<fep3::arya::IComponent> createComponent(const std::string& iid) const override;
            void createDefaults(ComponentRegistry& components) const;
            static ComponentSourceType getType()
            {
                return ComponentSourceType::built_in;
            }
    };
}
}