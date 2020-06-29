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

#include "../c_intf/destruction_manager_c_intf.h"
#include "../destruction_manager.h"

namespace fep3
{
namespace plugin
{
namespace c
{
namespace wrapper
{
namespace arya
{

/**
 * Wrapper class for destruction of an object through a C interface
 */
class Destructor
{
public:
    /**
     * Destroys the object identified by \p handle
     * @param handle The handle identifying the object to be destroyed
     */
    static inline void destroy(fep3_plugin_c_arya_HDestructionManager handle) noexcept
    {
        delete reinterpret_cast<::fep3::plugin::c::arya::DestructionManager*>(handle);
    }
};

} // namespace arya
using arya::Destructor;
} // namespace wrapper
} // namespace c
} // namespace plugin
} // namespace fep3
