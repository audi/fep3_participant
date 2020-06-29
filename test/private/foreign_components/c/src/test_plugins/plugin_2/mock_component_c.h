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

#include <memory>

#include <gmock/gmock.h>

#include "component_c_intf.h"
#include <fep3/components/base/c_access_wrapper/transferable_component_base.h>

namespace test_plugin_2
{
namespace mock
{

class MockComponentC
    : public ::fep3::plugin::c::TransferableComponentBase<IComponentC>
{
public:
    MockComponentC()
        : ::fep3::plugin::c::TransferableComponentBase<IComponentC>()
    {}
    MOCK_METHOD0(die, void());
    virtual ~MockComponentC()
    {
        die();
    }

    MOCK_CONST_METHOD0(get, int32_t());
};

} // namespace mock
} // namespace test_plugin_2
