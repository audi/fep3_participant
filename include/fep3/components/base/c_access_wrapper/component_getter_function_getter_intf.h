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

#include <string>

namespace fep3
{
namespace plugin
{
namespace c
{
namespace arya
{

/**
 * @brief Interface for the component getter function getter
 */
class IComponentGetterFunctionGetter
{
public:
    /**
     * Default DTOR
     */
    virtual ~IComponentGetterFunctionGetter() = default;
    /**
     * Parenthesis operator to get the component getter function
     * @param iid Interface ID of the component to get the getter function of
     * @return void pointer to the component getter function
     */
    virtual void* operator()(const std::string& iid) const = 0;
};

} // namespace arya
} // namespace c
} // namespace plugin
} // namespace fep3