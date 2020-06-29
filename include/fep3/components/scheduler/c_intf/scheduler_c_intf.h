/**
 * @file
 * Copyright &copy; AUDI AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * @attention Changes in this file must be reflected in the corresponding C++ interface file scheduler_intf.h
 *
 */

#pragma once

#include <fep3/plugin/base/fep3_calling_convention.h>
#include <fep3/plugin/base/fep3_plugin_export.h>
#include <fep3/plugin/c/c_intf/c_intf_errors.h>
#include <fep3/plugin/c/c_intf/destruction_manager_c_intf.h>
// C interface dependencies
#include <fep3/components/clock/c_intf/clock_service_c_intf.h>
#include <fep3/components/job_registry/c_intf/job_c_intf.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Handle to @ref fep3::arya::IScheduler
typedef struct fep3_arya_OIScheduler* fep3_arya_HIScheduler;
/// Access structure for @ref fep3::arya::IScheduler
typedef struct
{
    /// Handle to the wrapped object
    fep3_arya_HIScheduler _handle;
    // function pointers wrapping the interface
    /// @cond no_documentation
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getName)
        (fep3_arya_HIScheduler
        , void(*)(void*, const char*)
        , void*
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *initialize)
        (fep3_arya_HIScheduler
        , int32_t*
        , fep3_arya_SIClockService
        // callback accepting a callback for recursive job passing
        , void(*)(const void*, void(*)(void*, const char*, fep3_arya_SJobEntry), void*)
        , const void* // jobs_source
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *start)(fep3_arya_HIScheduler, int32_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *stop)(fep3_arya_HIScheduler, int32_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *deinitialize)(fep3_arya_HIScheduler, int32_t*);
    /// @endcond no_documentation
} fep3_arya_SIScheduler;

#ifdef __cplusplus
}
#endif
