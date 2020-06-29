/**
 * @file
 * Copyright &copy; AUDI AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * @attention Changes in this file must be reflected in the corresponding C++ interface file job_info.h
 *
 */

#pragma once

// C interface dependencies
#include "job_configuration_c_intf.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Access structure for @ref fep3::arya::JobInfo
typedef struct
{
    /// @cond no_documentation
    void(*_name_callback)
        (void* name_source
        , void(*name_callback)(void*, const char*)
        , void* name_destination
        );
    //fep3_arya_SJobConfiguration _configuration;
    void(*_config_callback)
        (void* config_source
        , void(*config_callback)(void*, fep3_arya_SJobConfiguration)
        , void* config_destination
        );
    void* _job_info_source;
    /// @endcond no_documentation
} fep3_arya_SJobInfo;

#ifdef __cplusplus
}
#endif
