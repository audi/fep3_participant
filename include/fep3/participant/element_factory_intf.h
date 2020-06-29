/**
 * Declaration of interface IElementFactory
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

#include <memory>
#include <fep3/fep3_participant_types.h>
#include "element_intf.h"
#include <fep3/components/base/components_intf.h>

namespace fep3
{
namespace arya
{

/**
 * Interface for the element factory
 *
 */
class IElementFactory
{
public:
    virtual ~IElementFactory()
    {
    }

    /**
     * Creates the element 
     *
     * @param components components reference to provide the component access
     * @returns Shared pointer to the created element.
     */
    virtual std::unique_ptr<IElement> createElement(const IComponents& components) const = 0;
};

} // namespace arya
using arya::IElementFactory;
} // namespace fep3
