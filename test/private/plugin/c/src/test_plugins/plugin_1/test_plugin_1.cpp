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

#include <fep3/plugin/c/c_plugin.h>

void fep3_plugin_getPluginVersion
    (void(*callback)(void*, const char*), void* destination)
{
    callback(destination, "0.0.1");
}
