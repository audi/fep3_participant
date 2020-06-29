/**
 * @file
 * Copyright &copy; AUDI AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * @attention Changes in this file must be reflected in the corresponding C++ interface file clock_service_intf.h
 *
 */

#pragma once

#include <fep3/plugin/base/fep3_calling_convention.h>
#include <fep3/plugin/base/fep3_plugin_export.h>
#include <fep3/plugin/c/c_intf/c_intf_errors.h>
#include <fep3/plugin/c/c_intf/shared_binary_c_intf.h>
#include <fep3/plugin/c/c_intf/destruction_manager_c_intf.h>
#include <fep3/components/base/c_intf/component_c_intf.h>
// C interface dependencies
#include "clock_c_intf.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Handle to @ref fep3::arya::IClockService
typedef struct fep3_arya_OIClockService* fep3_arya_HIClockService;
/// Access structure for fep3::arya::IClockService
typedef struct
{
    /// Handle to the wrapped object
    fep3_arya_HIClockService _handle;
    /// Base class IComponent
    fep3_arya_SIComponent _component;
    // function pointers wrapping methods of fep3::arya::IClockService
    /// @cond no_documentation
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getTime)(fep3_arya_HIClockService, int64_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getTimeByClockName)(fep3_arya_HIClockService, int64_t*, bool*, const char*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getType)(fep3_arya_HIClockService, int32_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getTypeByClockName)(fep3_arya_HIClockService, int32_t*, bool*, const char*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getMainClockName)(fep3_arya_HIClockService, void(*)(void*, const char*), void*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *registerEventSink)(fep3_arya_HIClockService, int32_t*, fep3_plugin_c_arya_SDestructionManager*, fep3_arya_IClock_SIEventSink);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *unregisterEventSink)(fep3_arya_HIClockService, int32_t*, fep3_plugin_c_arya_SDestructionManager*, fep3_arya_IClock_SIEventSink);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *registerClock)
        (fep3_arya_HIClockService
        , int32_t*
        , fep3_plugin_c_arya_SDestructionManager
        , fep3_arya_SIClock
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *unregisterClock)
        (fep3_arya_HIClockService
        , int32_t*
        , const char*
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getClockNames)
        (fep3_arya_HIClockService
        , void(*)(void*, const char*)
        , void*
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *findClock)
        (fep3_arya_HIClockService
        , fep3_plugin_c_arya_SDestructionManager*
        , fep3_arya_SIClock*
        , const char*
        );
    /// @endcond no_documentation
} fep3_arya_SIClockService;

/** @brief Gets a clock service that implements the interface identified by @p iid and provides access to it via @p access_result
 *
 * @param[in,out] access_result Pointer to an access structure providing access to the component to get;
 *                              if null, no object will be get and the parameter remains unchanged
 * @param iid IID of the component to be created
 * @param handle_to_component Handle to the interface of the component to get
 * @return Interface error code
 * @retval fep3_plugin_c_interface_error_none No error occurred
 * @retval fep3_plugin_c_interface_error_invalid_handle The @p handle_to_component is null
 * @retval fep3_plugin_c_interface_error_invalid_result_pointer The @p access_result is null
 */
fep3_plugin_c_InterfaceError fep3_plugin_c_arya_getClockService
    (fep3_arya_SIClockService* access_result
    , const char* iid
    , fep3_arya_HIComponent handle_to_component
    );
    
/// defines the symbol name of the function that creates a clock service
#define SYMBOL_fep3_plugin_c_arya_createClockService fep3_plugin_c_arya_createClockService

/** @brief Creates a clock service that implements the interface identified by \p iid and provides access to it via \p access
 *
 * @param[in,out] access Pointer to an access structure providing access to the created component;
 *                       if null, no object will be created and the parameter remains unchanged
 * @param shared_binary_access Access structure to the shared binary the component will reside in
 * @param iid IID of the component to be created
 * @return error code (if any)
 */
FEP3_PLUGIN_EXPORT fep3_plugin_c_InterfaceError FEP3_PLUGIN_CALL fep3_plugin_c_arya_createClockService
    (fep3_arya_SIClockService* access
    , fep3_plugin_c_arya_SISharedBinary shared_binary_access
    , const char* iid
    );

#ifdef __cplusplus
}
#endif
