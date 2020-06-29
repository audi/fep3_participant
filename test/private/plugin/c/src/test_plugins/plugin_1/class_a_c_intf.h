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

#include <fep3/plugin/base/fep3_calling_convention.h>
#include <fep3/plugin/base/fep3_plugin_export.h>
#include <fep3/plugin/c/c_intf/shared_binary_c_intf.h>
#include <fep3/plugin/c/c_intf/c_intf_errors.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// Handle to test_plugin_1::IClassA
typedef struct test_plugin_1_OIClassA* test_plugin_1_HIClassA;

/// Access structure for @ref test_plugin_1::IClassA
typedef struct
{
    // the handle to the object
    test_plugin_1_HIClassA _handle;
    // function pointers wrapping the interface
    fep3_plugin_c_InterfaceError(FEP3_PLUGIN_CALL *set)(test_plugin_1_HIClassA, int32_t);
    fep3_plugin_c_InterfaceError(FEP3_PLUGIN_CALL *get)(test_plugin_1_HIClassA, int32_t*);
} test_plugin_1_SIClassA;

/** @brief Creates an object of type test_plugin_1::ClassA and provides access to it via \p access
 *
 * @param[in,out] access Pointer to an access structure providing access to the created object; if null, no object will be created and the parameter remains unchanged
 * @return error code (if any)
 */
FEP3_PLUGIN_EXPORT fep3_plugin_c_InterfaceError FEP3_PLUGIN_CALL createClassA(test_plugin_1_SIClassA* access, fep3_plugin_c_arya_SISharedBinary shared_binary_access);

#ifdef __cplusplus
}
#endif

