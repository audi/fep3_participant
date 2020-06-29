/**
 * Declaration of class ElementBase
 *
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

#include <string>
#include <memory>
#include <list>
#include <map>

#include "element_base.h"
#include <fep3/components/configuration/propertynode.h>


namespace fep3
{
namespace core
{
namespace arya
{

using fep3::arya::PropertyVariable;

/**
 * This is the base class for every FEP3 Element which is able to register for properties 
 * To implement your own FEP3 Element, derive from this class.
 */
class ElementConfigurable : public ElementBase,
                            public Configuration
{
public:
    /**
     * Deleted default CTOR
     */
    ElementConfigurable() = delete;
protected:
    /**
     * CTOR
     * this ctor will also create one property node called "element"
     *
     * @param type_name Name of the type the element presents
     * @param version_info Version information of the element
     */
    ElementConfigurable(std::string type_name, std::string version_info)
        : ElementBase(type_name, version_info),
          Configuration("element")
    {
    }
public:
    /**
     * Default DTOR
     *
     */
    virtual ~ElementConfigurable() = default;

private:
    /**
     * internal callback to load the element
     * this callback will create one property node called "element" within the configuration service
     * @remark the configuration service is required as component if using this class
     *
     * @param[in] components reference to the components. this pointer is valid until unload was called.
     *
     * @return Result error code
     * @retval ERR_NO_ERROR if succeded
     * @retval ERR_NOT_FOUND Configuration service required and not found
     */
    Result loadElement(const IComponents& components) override
    {
        auto load_res = ElementBase::loadElement(components);
        if (isFailed(load_res))
        {
            return load_res;
        }
        else
        {
            auto config_service = getComponents()->getComponent<fep3::arya::IConfigurationService>();
            if (config_service)
            {
                //we register ourself at the configuration service!
                return initConfiguration(*config_service);
            }
            else
            {
                RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "configuration service required for the configurable elements");
            }
        }
    }
    /**
     * deinitializes the element
     *
     * @return Result error code
     * @retval ERR_NO_ERROR if succeded
     */
    void unloadElement() override
    {
       deinitConfiguration();
       ElementBase::unloadElement();       
    }

};

} // namespace arya
using arya::ElementConfigurable;
using arya::PropertyVariable;
} // namespace core
} // namespace fep3

