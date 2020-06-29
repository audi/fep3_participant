/**
 * @file
 * Copyright &copy; AUDI AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Declaration of the C interface for the job registry component.
 * @attention Changes in this file must be reflected in the corresponding C++ interface file job_registry_intf.h
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
#include "job_c_intf.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Handle to @ref fep3::arya::IJobRegistry
typedef struct fep3_arya_OIJobRegistry* fep3_arya_HIJobRegistry;
/// Access structure for @ref fep3::arya::IJobRegistry
typedef struct
{
    /// Handle to the wrapped object
    fep3_arya_HIJobRegistry _handle;
    /// Base class IComponent
    fep3_arya_SIComponent _component;
    // function pointers wrapping the interface
    /// @cond no_documentation
    // note: parameters reordered compared to IJobRegistry::addJob to be able to use access::Helper
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *addJob)
        (fep3_arya_HIJobRegistry
        , int32_t*
        , fep3_plugin_c_arya_SDestructionManager
        , fep3_arya_SIJob
        , const char*
        , fep3_arya_SJobConfiguration
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *removeJob)(fep3_arya_HIJobRegistry, int32_t*, const char*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getJobInfos)
        (fep3_arya_HIJobRegistry
        , void(*)(void*, fep3_arya_SJobInfo)
        , void*
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getJobs)
        (fep3_arya_HIJobRegistry
        , void(*)(void*, fep3_arya_SJobEntry)
        , void*
        );
    /// @endcond no_documentation
} fep3_arya_SIJobRegistry;

/** @brief Gets a job registry that implements the interface identified by @p iid and provides access to it via @p access_result
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
fep3_plugin_c_InterfaceError fep3_plugin_c_arya_getJobRegistry
    (fep3_arya_SIJobRegistry* access_result
    , const char* iid
    , fep3_arya_HIComponent handle_to_component
    );

/// defines the symbol name of the function that creates a job registry
#define SYMBOL_fep3_plugin_c_arya_createJobRegistry fep3_plugin_c_arya_createJobRegistry

/** @brief Creates a job registry that implements the interface identified by \p iid and provides access to it via \p access
 *
 * @param[in,out] access Pointer to an access structure providing access to the created component;
 *                       if null, no object will be created and the parameter remains unchanged
 * @param shared_binary_access Access structure to the shared binary the component will reside in
 * @param iid IID of the component to be created
 * @return error code (if any)
 */
FEP3_PLUGIN_EXPORT fep3_plugin_c_InterfaceError FEP3_PLUGIN_CALL fep3_plugin_c_arya_createJobRegistry
    (fep3_arya_SIJobRegistry* access
    , fep3_plugin_c_arya_SISharedBinary shared_binary_access
    , const char* iid
    );

#ifdef __cplusplus
}
#endif
