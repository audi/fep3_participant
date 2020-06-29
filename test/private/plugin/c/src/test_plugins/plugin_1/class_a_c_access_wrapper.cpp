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

#include "class_a_c_intf.h"
#include "class_a.h"
#include "class_a_c_access_wrapper.h"

fep3_plugin_c_InterfaceError createClassA(test_plugin_1_SIClassA* access, fep3_plugin_c_arya_SISharedBinary shared_binary_access)
{
    return ::test_plugin_1::wrapper::createClassA<::test_plugin_1::ClassA>(access, shared_binary_access);
}