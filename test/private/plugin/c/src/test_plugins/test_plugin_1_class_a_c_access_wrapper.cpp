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

#include "plugin_1/class_a_c_intf.h"
#include "plugin_1/class_a.h"
#include "plugin_1/class_a_c_access_wrapper.h"

fep3_c_plugin_CInterfaceError createTestPlugin1ClassA(SITestPlugin1ClassA* access, fep3_c_plugin_SISharedBinary shared_binary_access)
{
    return ::test_plugin_1::wrapper::TestPlugin1ClassA::create<::test_plugin_1::TestPlugin1ClassA>(access, shared_binary_access);
}