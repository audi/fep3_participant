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
#include <fep3/participant/component_factories/component_factory_base.h>
#include <fep3/participant/component_source_type.h>
#include <vector>
#include <string>
#include <memory>

namespace fep3
{
    namespace arya
    {
        class ComponentFactoryCPPPlugin : public ComponentFactoryBase
        {
        public:
            ComponentFactoryCPPPlugin(const std::vector<std::string>& files);
            virtual ~ComponentFactoryCPPPlugin();
            std::unique_ptr<fep3::arya::IComponent> createComponent(const std::string& iid) const override;
            static ComponentSourceType getType()
            {
                return ComponentSourceType::cpp_plugin;
            }

        private:
            struct Implementation;
            std::unique_ptr<Implementation> _impl;
        };
    }
}