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

#include <fep3/components/scheduler/c_access_wrapper/scheduler_service_c_access_wrapper.h>
// Note: a C plugin must not link against the fep3 participant private (object) library, so we need an 
// explicit relative path to the mock class file; a better solution might be to create dedicated header-only 
// private library containing the mock files.
#include "../../../../../../../src/fep3/components/scheduler/mock/mock_transferable_scheduler_service_with_access_to_schedulers.h"

extern fep3::mock::TransferableSchedulerServiceWithAccessToSchedulers* g_mock_scheduler_service;

fep3_plugin_c_InterfaceError fep3_plugin_c_arya_createSchedulerService
    (fep3_arya_SISchedulerService* access
    , fep3_plugin_c_arya_SISharedBinary shared_binary_access
    , const char* iid
    )
{
    // for testing we must use the factory function in "detail" because we need a custom factory
    // to be able to return the mock object
    auto result = ::fep3::plugin::c::wrapper::arya::detail::createSchedulerService
        ([]()
            {
                return g_mock_scheduler_service;
            }
        , access
        , shared_binary_access
        , iid
        );
    return result;
}
