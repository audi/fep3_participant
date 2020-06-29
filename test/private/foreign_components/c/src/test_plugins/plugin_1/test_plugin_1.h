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
#include "mock_component_a.h"
#include "mock_component_b.h"

extern ::test_plugin_1::mock::MockComponentA* g_mock_component_a;
extern ::test_plugin_1::mock::MockComponentB* g_mock_component_b;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Sets the mock component a
 * @param mock_component_a Pointer to the mock component a to be set to the plugin
 * @note This destroys binary compatibilty of the plugin, because a C++ interface is introduced.
 *       This is ok, as long as plugin and test are compiled with the same compiler and compiler settings
 *       (which is guaranteed in the unit test context).
 */
FEP3_PLUGIN_EXPORT void FEP3_PLUGIN_CALL setMockComponentA(::test_plugin_1::mock::MockComponentA* mock_component_a);
/**
 * Sets the mock component b
 * @param mock_component_b Pointer to the mock component b to be set to the plugin
 * @note This destroys binary compatibilty of the plugin, because a C++ interface is introduced.
 *       This is ok, as long as plugin and test are compiled with the same compiler and compiler settings
 *       (which is guaranteed in the unit test context).
 */
FEP3_PLUGIN_EXPORT void FEP3_PLUGIN_CALL setMockComponentB(::test_plugin_1::mock::MockComponentB* mock_component_b);

#ifdef __cplusplus
}
#endif

