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

#include <fep3/plugin/base/plugin_base_intf.h>

// symbols that must never change, because they are not bound to a namespace version
/// defines the symbol name of the function that returns the participant library version (major, minor, etc.)
#define SYMBOL_fep3_plugin_c_getParticipantLibraryVersion "fep3_plugin_c_getParticipantLibraryVersion"
/// defines the symbol name of the function that returns the version namespace string (e. g. "arya", "bronn", etc.)
#define SYMBOL_fep3_plugin_c_getVersionNamespace "fep3_plugin_c_getVersionNamespace"
/// defines the symbol name of the function that returns the plugin version as string
#define SYMBOL_fep3_plugin_c_getPluginVersion "fep3_plugin_c_getPluginVersion"
