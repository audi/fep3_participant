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

#include "test_plugin.h"

void fep3_plugin_getPluginVersion
    (void(*callback)(void*, const char*), void* destination)
{
    callback(destination, "SimulationBus interface test c plugin 0.0.1");
}

::fep3::mock::SimulationBus<fep3::plugin::c::TransferableComponentBase>* g_mock_simulation_bus = nullptr;

void setMockSimulationBus(::fep3::mock::SimulationBus<fep3::plugin::c::TransferableComponentBase>* mock_simulation_bus)
{
    g_mock_simulation_bus = mock_simulation_bus;
}