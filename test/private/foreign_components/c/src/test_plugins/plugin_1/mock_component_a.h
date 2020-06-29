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

#include "component_a_intf.h"
#include <fep3/components/base/c_access_wrapper/transferable_component_base.h>
#include "component_b_intf.h"
#include "component_b_c_access_wrapper.h"
#include <test_plugins/plugin_2/component_c_intf.h>
#include <test_plugins/plugin_2/component_c_c_access_wrapper.h>

namespace test_plugin_1
{
namespace mock
{

class MockComponentA
    : public ::fep3::plugin::c::TransferableComponentBase<IComponentA>
{
public:
    MockComponentA()
        : fep3::plugin::c::TransferableComponentBase<IComponentA>
            (fep3::plugin::c::arya::makeComponentGetter
                <::test_plugin_1::access::ComponentB
                , ::test_plugin_2::access::ComponentC
                >()
            )
    {}
    MOCK_METHOD0(die, void());
    virtual ~MockComponentA()
    {
        die();
    }

    MOCK_CONST_METHOD0(get, int32_t());
    MOCK_METHOD1(set, void(int32_t));
    int32_t getFromComponentB() override
    {
        if(const auto& components = _components.lock())
        {
            if(IComponentB* component_b = components->getComponent<IComponentB>())
            {
                return component_b->get();
            }
        }
        return 0;
    }
    int32_t getFromComponentC() override
    {
        if(const auto& components = _components.lock())
        {
            if(::test_plugin_2::IComponentC* component_c = components->getComponent<::test_plugin_2::IComponentC>())
            {
                return component_c->get();
            }
        }
        return 0;
    }
};

} // namespace mock
} // namespace test_plugin_1
