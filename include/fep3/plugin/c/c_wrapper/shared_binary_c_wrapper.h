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

#include "../shared_binary_intf.h"
#include "../c_intf/shared_binary_c_intf.h"

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
 * @brief Class wrapping a shared binary to make it accessible via free functions
 */
class SharedBinary
{
public:
    /**
     * Destroys the object identified by \p handle
     * @param handle The handle identifying the object to be destroyed
     */
    static inline void destroy(fep3_plugin_c_arya_HISharedBinary handle)
    {
        delete reinterpret_cast<std::shared_ptr<ISharedBinary>*>(handle);
    }
};

} // namespace arya
using arya::SharedBinary;
} // namespace wrapper
} // namespace c
} // namespace plugin
} // namespace fep3
