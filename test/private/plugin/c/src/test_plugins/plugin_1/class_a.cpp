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

#include <stdint.h>

#include "class_a.h"

namespace test_plugin_1
{

int32_t ClassA::get() const
{
    return _value;
}
void ClassA::set(int32_t value)
{
    _value = value;
}

} // namespace test_plugin_1
