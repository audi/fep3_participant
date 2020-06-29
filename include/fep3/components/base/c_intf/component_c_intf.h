/**
 * @file
 * Copyright &copy; AUDI AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * @attention Changes in this file must be reflected in the corresponding C++ interface file component_intf.h
 *
 */

#pragma once

#include <fep3/plugin/base/fep3_calling_convention.h>
#include <fep3/plugin/c/c_intf/c_intf_errors.h>
#include <fep3/plugin/c/c_intf/destruction_manager_c_intf.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// Handle to @ref fep3::arya::IComponent
typedef struct fep3_arya_OIComponent* fep3_arya_HIComponent;

/// Handle to @ref fep3::arya::IComponents
typedef struct fep3_arya_OComponents* fep3_arya_HComponents;

/// Handle to component getter function getters
typedef struct fep3_plugin_c_arya_OComponentGetterFunctionGetters* fep3_plugin_c_arya_HComponentGetterFunctionGetters;

/// Structure representing access to a component interface
typedef struct
{
    /// Handle to the component
    fep3_arya_HIComponent _handle;
    /// Function pointer to a function capable to get the component
    void* getComponent;
} fep3_arya_SIComponentInterface;

/// Access structure for @ref fep3::arya::IComponents
typedef struct
{
    /// Handle to the wrapped object
    fep3_arya_HComponents _handle;
    /// function pointers wrapping the interface
    /// @cond no_documentation
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *findComponent)
        (fep3_arya_HComponents handle
        , fep3_arya_SIComponentInterface* access_result
        , fep3_plugin_c_arya_HComponentGetterFunctionGetters handle_to_component_getter_function_getters
        , const char* iid
        );
    /// @endcond no_documentation
} fep3_arya_SComponents;

/// Access structure for @ref fep3::arya::IComponent
typedef struct
{
    /// Handle to the wrapped object
    fep3_arya_HIComponent _handle;
    /// Base class DestructionManager
    fep3_plugin_c_arya_SDestructionManager _destruction_manager;
    /// function pointers wrapping the interface
    /// @cond no_documentation
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *createComponent)
        (fep3_arya_HIComponent handle
        , int32_t* result
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_access_result
        , fep3_arya_SComponents
        , fep3_plugin_c_arya_HComponentGetterFunctionGetters
        , fep3_plugin_c_arya_SDestructionManager component_getter_function_getters_destruction_manager_access
        );
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *destroyComponent)(fep3_arya_HIComponent, int32_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *initialize)(fep3_arya_HIComponent, int32_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *tense)(fep3_arya_HIComponent, int32_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *relax)(fep3_arya_HIComponent, int32_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *start)(fep3_arya_HIComponent, int32_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *stop)(fep3_arya_HIComponent, int32_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *pause)(fep3_arya_HIComponent, int32_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *deinitialize)(fep3_arya_HIComponent, int32_t*);
    /// @endcond no_documentation
    // note: as pointers to objects must not be transferred over binary boundaries, the "getInterface" method cannot be wrapped
    //fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getInterface)(fep3_plugin_c_HIComponent, void(*)(void*, void*), void*, const char*);
} fep3_arya_SIComponent;

#ifdef __cplusplus
}
#endif
