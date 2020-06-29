/**
 * @file
 * Copyright &copy; AUDI AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Declaration of error codes and types for the C interface.
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Enumeration of errors occurring on the C interface of the C Plugin System
 * @note The enum is not in a version namespace as the enum values would be anyway.
 *       Thus, enum values must never be removed or changed, only adding new values is allowed.
 */
typedef enum
{
    ///no error 
    fep3_plugin_c_interface_error_none = 0,
    ///_INVALID_HANDLE error 
    fep3_plugin_c_interface_error_invalid_handle= 1,
    ///_INVALID_RESULT_POINTER error 
    fep3_plugin_c_interface_error_invalid_result_pointer= 2,
    ///_ERROR_OUT_OF_MEMORY error 
    fep3_plugin_c_interface_error_out_of_memory= 3,
    ///_ERROR_EXCEPTION_CAUGHT error
    fep3_plugin_c_interface_error_exception_caught= 4,
    ///_ERROR_NOT_IMPLEMENTED error
    fep3_plugin_c_interface_error_not_implemented= 5,
} fep3_plugin_c_EnumInterfaceError;

/// using an int32 to ensure binary compatibility among compilers (enums might be handled differently)
typedef int32_t fep3_plugin_c_InterfaceError;

#ifdef __cplusplus
}
#endif
