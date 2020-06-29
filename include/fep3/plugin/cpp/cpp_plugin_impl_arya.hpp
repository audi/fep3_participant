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

#include <fep3/plugin/base/fep3_calling_convention.h>
#include <fep3/plugin/base/fep3_plugin_export.h>
#include "cpp_plugin_impl_base.hpp"

extern "C"
{
    /**
     * Returns the version namespace the plugin interface resides in.
     * @note This function has to be implemented in the plugin
     * @return The version namespace "arya" as string
     */
    FEP3_PLUGIN_EXPORT const char* FEP3_PLUGIN_CALL fep3_plugin_getVersionNamespace()
    {
        return "arya";
    }
}