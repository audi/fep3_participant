/**
 * @file
 * Copyright &copy; AUDI AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * @attention Changes in this file must be reflected in the corresponding C++ interface file data_sample_intf.h
 *
 */

#pragma once

#include <fep3/plugin/base/fep3_calling_convention.h>
#include <fep3/plugin/c/c_intf/c_intf_errors.h>
#include "raw_memory_c_intf.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Handle to data sample
typedef struct fep3_arya_OIDataSample* fep3_arya_HIDataSample;

/// Access structure for @ref fep3::arya::IDataSample
typedef struct
{
    /// Handle to the wrapped object
    fep3_arya_HIDataSample _handle;
    // function pointers wrapping the interface
    /// @cond no_documentation
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getTime)(fep3_arya_HIDataSample, int64_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getSize)(fep3_arya_HIDataSample, size_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *getCounter)(fep3_arya_HIDataSample, uint32_t*);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *read)(fep3_arya_HIDataSample, size_t*, fep3_arya_SIRawMemory);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *setTime)(fep3_arya_HIDataSample, int64_t);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *setCounter)(fep3_arya_HIDataSample, uint32_t);
    fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL *write)(fep3_arya_HIDataSample, size_t*, fep3_arya_SIRawMemory);
    /// @endcond no_documentation
} fep3_arya_SIDataSample;

#ifdef __cplusplus
}
#endif
