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
#include <string>
#include <vector>

#include <fep3/participant/component_factories/component_factory_base.h>
#include <fep3/participant/component_source_type.h>

namespace fep3
{
namespace arya
{

/**
 * @brief Factory class creating components from within a C plugin
 */
class ComponentFactoryCPlugin : public ComponentFactoryBase
{
public:
    /**
     * CTOR
     * @param plugin_file_paths File paths to all C plugins that contain components to be created by this factory
     */
    ComponentFactoryCPlugin(const std::vector<std::string>& plugin_file_paths);
    /// DTOR
    virtual ~ComponentFactoryCPlugin() override;
    /**
     * Creates a component identified by \p iid from within a C plugin
     * @param iid The interface identifier of the component to be created
     * @return Unique pointer to the created component (if any)
     */
    std::unique_ptr<fep3::IComponent> createComponent(const std::string& iid) const override;
    /**
     * @brief Gets the type of the plugins as string
     * @return The type as string
     */
    static ComponentSourceType getType()
    {
        return ComponentSourceType::c_plugin;
    }

private:
    class Implementation;
    std::unique_ptr<Implementation> _impl;
};

}
}
