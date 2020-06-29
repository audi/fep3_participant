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

#include <fep3/plugin/c/c_plugin.h>
#include <fep3/plugin/base/fep3_calling_convention.h>
#include <fep3/plugin/base/fep3_plugin_export.h>
// Note: a C plugin must not link against the fep3 participant private (object) library, so we need an 
// explicit relative path to the mock class file; a better solution might be to create dedicated header-only 
// private library containing the mock files.
#include "../../../../../../../src/fep3/components/simulation_bus/mock/mock_simulation_bus.h"
#include <fep3/components/base/c_access_wrapper/transferable_component_base.h>

extern fep3::mock::SimulationBus<fep3::plugin::c::TransferableComponentBase>* g_mock_simulation_bus;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Sets the mock simulation bus
 * @param mock_simulation_bus Pointer to the mock simulation bus to be set to the plugin
 * @note This destroys binary compatibilty of the plugin, because a C++ interface is introduced.
 *       This is ok, as long as plugin and test are compiled with the same compiler and compiler settings
 *       (which is guaranteed in the unit test context).
 */
FEP3_PLUGIN_EXPORT void FEP3_PLUGIN_CALL setMockSimulationBus(::fep3::mock::SimulationBus<fep3::plugin::c::TransferableComponentBase>* mock_simulation_bus);

#ifdef __cplusplus
}
#endif

