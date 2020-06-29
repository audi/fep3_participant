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
#include <fep3/components/base/c_intf/component_c_intf.h>
#include <fep3/plugin/c/c_intf/shared_binary_c_intf.h>
#include <fep3/plugin/c/c_intf/c_intf_errors.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// Handle to @ref ::test_plugin::IComponentB
typedef struct test_plugin_OIComponentB* test_plugin_HIComponentB;

/// Access structure for @ref ::test_plugin::IComponentB
typedef struct
{
    // the handle to the object
    test_plugin_HIComponentB _handle;
    /// Base class @ref fep3::arya::IComponent
    fep3_arya_SIComponent _component;
    // function pointers wrapping the interface
    fep3_plugin_c_InterfaceError(FEP3_PLUGIN_CALL *get)(test_plugin_HIComponentB, int32_t*);
} test_plugin_SIComponentB;

fep3_plugin_c_InterfaceError test_plugin_1_getComponentB
    (test_plugin_SIComponentB* access_result
    , const char* iid
    , fep3_arya_HIComponent handle_to_component
    );

/// defines the symbol name of the function that creates a component that is implemented against IComponentB
#define SYMBOL_test_plugin_createComponentB "test_plugin_createComponentB"

/** @brief Creates a component A that implements the interface identified by \p iid and provides access to it via \p access
 *
 * @param[in,out] access Pointer to an access structure providing access to the created component; if null, no object will be created and the parameter remains unchanged
 * @param shared_binary_access Access structure to the shared binary the component will reside in
 * @param iid IID of the component to be created
 * @return error code (if any)
 */
FEP3_PLUGIN_EXPORT fep3_plugin_c_InterfaceError FEP3_PLUGIN_CALL test_plugin_createComponentB
    (test_plugin_SIComponentB* access
    , fep3_plugin_c_arya_SISharedBinary shared_binary_access
    , const char* iid
    );

#ifdef __cplusplus
}
#endif

