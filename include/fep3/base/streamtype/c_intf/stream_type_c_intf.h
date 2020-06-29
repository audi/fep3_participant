/**
 * @file
 * Copyright &copy; AUDI AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * @attention Changes in this file must be reflected in the corresponding C++ interface file stream_type_intf.h
 *
 */

#pragma once

#include <fep3/plugin/c/c_intf/c_intf_errors.h>
#include <fep3/base/properties/c_intf/properties_c_intf.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Handle to stream type
typedef struct fep3_arya_OIStreamType* fep3_arya_HIStreamType;

/// Access structure for @ref fep3::arya::StreamMetaType
typedef struct
{
    /// Handle to the wrapped object
    fep3_arya_HIStreamType _handle;
    /// base Class fep3::arya::IProperties
    fep3_arya_SIProperties _properties;
    // function pointers wrapping the interface
    /// @cond no_documentation
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getMetaTypeName)(fep3_arya_HIStreamType, void(*)(void*, const char*), void*);
    /// @endcond no_documentation
} fep3_arya_SIStreamType;

#ifdef __cplusplus
}
#endif
