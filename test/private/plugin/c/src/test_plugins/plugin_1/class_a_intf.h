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

#include <stdint.h>

#include <fep3/plugin/c/shared_binary_manager.h>

namespace test_plugin_1
{

class IClassA
    : public ::fep3::plugin::c::SharedBinaryManager // enable lifetime management of binary
{
public:
    virtual ~IClassA() = default;

public:
    virtual int32_t get() const = 0;
    virtual void set(int32_t value) = 0;
};

} // namespace test_plugin_1
