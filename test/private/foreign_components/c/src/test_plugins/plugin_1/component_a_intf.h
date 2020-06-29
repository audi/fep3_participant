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

#include <fep3/components/base/component_iid.h>

namespace test_plugin_1
{

class IComponentA
{
protected:
    virtual ~IComponentA() = default;
public:
    FEP_COMPONENT_IID("component_a.iid")

public:
    virtual int32_t get() const = 0;
    virtual void set(int32_t value) = 0;
    virtual int32_t getFromComponentB() = 0;
    virtual int32_t getFromComponentC() = 0;
};

} // namespace test_plugin_1
