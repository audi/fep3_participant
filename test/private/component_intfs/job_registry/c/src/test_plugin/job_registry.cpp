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

#include <fep3/components/job_registry/c_access_wrapper/job_registry_c_access_wrapper.h>
// Note: a C plugin must not link against the fep3 participant private (object) library, so we need an 
// explicit relative path to the mock class file; a better solution might be to create dedicated header-only 
// private library containing the mock files.
#include "../../../../../../../src/fep3/components/job_registry/mock/mock_transferable_job_registry_with_access_to_jobs.h"

extern fep3::mock::TransferableJobRegistryWithAccessToJobs* g_mock_job_registry;

fep3_plugin_c_InterfaceError fep3_plugin_c_arya_createJobRegistry
    (fep3_arya_SIJobRegistry* access
    , fep3_plugin_c_arya_SISharedBinary shared_binary_access
    , const char* iid
    )
{
    // for testing we must use the factory function in "detail" because we need a custom factory
    // to be able to return the mock object
    auto result = fep3::plugin::c::wrapper::arya::detail::createJobRegistry
        ([]()
            {
                return g_mock_job_registry;
            }
        , access
        , shared_binary_access
        , iid
        );
    return result;
}
