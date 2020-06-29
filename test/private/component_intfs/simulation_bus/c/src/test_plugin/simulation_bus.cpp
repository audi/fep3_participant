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

#include <fep3/components/simulation_bus/c_access_wrapper/simulation_bus_c_access_wrapper.h>
// Note: a C plugin must not link against the fep3 participant private (object) library, so we need an 
// explicit relative path to the mock class file; a better solution might be to create dedicated header-only 
// private library containing the mock files.
#include "../../../../../../../src/fep3/components/simulation_bus/mock/mock_simulation_bus.h"

extern ::fep3::mock::SimulationBus<fep3::plugin::c::TransferableComponentBase>* g_mock_simulation_bus;

fep3_plugin_c_InterfaceError fep3_plugin_c_arya_createSimulationBus
    (fep3_arya_SISimulationBus* access
    , fep3_plugin_c_arya_SISharedBinary shared_binary_access
    , const char* iid
    )
{
    // for testing we must use the factory function in "detail" because we need a custom factory
    // to be able to return the mock object
    auto result = ::fep3::plugin::c::wrapper::arya::detail::createSimulationBus
        ([]()
            {
                if(nullptr != g_mock_simulation_bus)
                {
                    return g_mock_simulation_bus;
                }
                else
                {
                    // if the simulation bus has not explicitly been set from outside
                    // we assume that a dummy mock is needed, so we use NiceMock
                    const auto& mock_simulation_bus 
                        = new ::testing::NiceMock<::fep3::mock::SimulationBus<fep3::plugin::c::TransferableComponentBase>>();
                    return static_cast<::fep3::mock::SimulationBus<fep3::plugin::c::TransferableComponentBase>*>(mock_simulation_bus);
                }
            }
        , access
        , shared_binary_access
        , iid
        );
    return result;
}
