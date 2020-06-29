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

#include "component_a_c_intf.h"
#include "mock_component_a.h"
#include "component_a_c_access_wrapper.h"

extern ::test_plugin_1::mock::MockComponentA* g_mock_component_a;

fep3_plugin_c_InterfaceError test_plugin_createComponentA
    (test_plugin_SIComponentA* access_result
    , fep3_plugin_c_arya_SISharedBinary shared_binary_access
    , const char* iid
    )
{
    // for testing we must use the factory function in "detail" because we need a custom factory
    // to be able to return the mock object
    auto result = ::test_plugin_1::wrapper::detail::createComponentA
        (std::function<::test_plugin_1::mock::MockComponentA*()>([]()
            {
                return g_mock_component_a;
            })
        , access_result
        , shared_binary_access
        , iid
        );
    if(fep3::isOk(result))
    {
        // for testing we need get the MockSimulationBus out of the plugin to be able to register mock expectations for it
        auto pointer_to_component_a = reinterpret_cast<::test_plugin_1::IComponentA*>(access_result->_handle);
        g_mock_component_a = dynamic_cast<::testing::StrictMock<::test_plugin_1::mock::MockComponentA>*>(pointer_to_component_a);
    }
    return result;
}