/**
 * @file
 * Copyright &copy; AUDI AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * @attention Changes in this file must be reflected in the corresponding C++ interface file raw_memory_intf.h
 *
 */

#pragma once

#include <stddef.h>

#include <fep3/plugin/base/fep3_calling_convention.h>
#include <fep3/plugin/c/c_intf/c_intf_errors.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Handle to raw memory
typedef struct fep3_arya_OIRawMemory* fep3_arya_HIRawMemory;

/// Access structure for @ref fep3::arya::IRawMemory
typedef struct
{
    /// Handle to the wrapped object
    fep3_arya_HIRawMemory _handle;
    // function pointers wrapping the interface
    /// @cond no_documentation
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *capacity)(fep3_arya_HIRawMemory, size_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *cdata)(fep3_arya_HIRawMemory, const void**);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *size)(fep3_arya_HIRawMemory, size_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *set)(fep3_arya_HIRawMemory, size_t*, const void*, size_t);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *resize)(fep3_arya_HIRawMemory, size_t*, size_t);
    /// @endcond no_documentation
} fep3_arya_SIRawMemory;

#ifdef __cplusplus
}
#endif
