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

#include <stdint.h>

// symbols that must never change, because they are not bound to a namespace version
/// Defines the symbol name of the function that returns the participant library version (major, minor, etc.)
#define SYMBOL_fep3_plugin_getParticipantLibraryVersion "fep3_plugin_getParticipantLibraryVersion"
/// Defines the symbol name of the function that returns the plugin version as string
#define SYMBOL_fep3_plugin_getPluginVersion "fep3_plugin_getPluginVersion"

#ifdef __cplusplus
extern "C"
{
#endif

/// Structure for the participant library version
typedef struct
{
    /// The version identifier of the participant library
    const char* _id;
    /// The major version integer
    int32_t _major;
    /// The minor version integer
    int32_t _minor;
    /// The patch version integer
    int32_t _patch;
    /// The build version integer
    int32_t _build;
} fep3_plugin_base_ParticipantLibraryVersion;

#ifdef __cplusplus
}
#endif