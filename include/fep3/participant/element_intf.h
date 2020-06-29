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
#include <fep3/fep3_participant_export.h>
#include <fep3/components/base/components_intf.h>


namespace fep3
{
namespace arya
{

/**
 * This is the interface of one element the participant can deal with class for every FEP3 Element.
 * To implement your own FEP3 Element, derive from this class.
 */
class IElement
{

public:
    /**
     * Default DTOR
     *
     */
    virtual ~IElement() = default;

public:
    /**
     * return the typename of the element.
     * the typename represents the type the element implementing.
     * @remark This is not the instance name of the element!
     *         the instance name is usually the same as the participant this element is loaded in.
     *
     * @return std::string the typename
     */
    virtual std::string getTypename() = 0;
    /**
     * return the version of the element.
     * the version of the element implementation
     * @remark This is the instance version of the element type 
     *         and will be used only for information at the moment! 
     *         There is no further functionality or checks on that!
     *
     * @return std::string the version as string (this is vendor dedendent, and only for information!)
     */
    virtual std::string getVersion() = 0;
    /**
     * internal callback to load the element
     *
     * @param[in] components reference to the components. this pointer is valid until unload was called.
     *
     * @return Result error code
     * @retval NO_ERROR if succeded
     */
    virtual Result loadElement(const IComponents& components) = 0;
    /**
     * Initializes the element
     *
     * @return Result error code
     * @retval NO_ERROR if succeded
     */
    virtual void unloadElement() = 0;
    /**
     * Initializes the element
     *
     * @return Result error code
     * @retval NO_ERROR if succeded
     */
    virtual Result initialize() = 0;
    /**
     * Deinitializes the element
     *
     */
    virtual void deinitialize() = 0;
   
    /**
     * Runs the element
     *
     * @return Result error code
     * @retval NO_ERROR if succeded
     */
    virtual Result run() = 0;
    /**
     * Stops the element
     */
    virtual void stop() = 0;
};

} // namespace arya
using arya::IElement;
} // namespace fep3
