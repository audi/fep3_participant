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

#include "test_plugin_1.h"

void fep3_plugin_getPluginVersion
    (void(*callback)(void*, const char*), void* destination)
{
    callback(destination, "test plugin 1, version 0.0.1");
}

::test_plugin_1::mock::MockComponentA* g_mock_component_a = nullptr;
::test_plugin_1::mock::MockComponentB* g_mock_component_b = nullptr;

void setMockComponentA(::test_plugin_1::mock::MockComponentA* mock_component_a)
{
    g_mock_component_a = mock_component_a;
}
void setMockComponentB(::test_plugin_1::mock::MockComponentB* mock_component_b)
{
    g_mock_component_b = mock_component_b;
}