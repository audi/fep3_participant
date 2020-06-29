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

#include "component_c_c_intf.h"
#include "mock_component_c.h"
#include "component_c_c_access_wrapper.h"

extern ::test_plugin_2::mock::MockComponentC* g_mock_component_c;

fep3_plugin_c_InterfaceError test_plugin_createComponentC
    (test_plugin_SIComponentC* access_result
    , fep3_plugin_c_arya_SISharedBinary shared_binary_access
    , const char* iid
    )
{
    // for testing we must use the factory function in "detail" because we need a custom factory
    // to be able to return the mock object
    auto result = ::test_plugin_2::wrapper::detail::createComponentC
        (std::function<::test_plugin_2::mock::MockComponentC*()>([]()
            {
                return g_mock_component_c;
            })
        , access_result
        , shared_binary_access
        , iid
        );
    if(fep3::isOk(result))
    {
        // for testing we need get the MockSimulationBus out of the plugin to be able to register mock expectations for it
        auto pointer_to_component_c = reinterpret_cast<::test_plugin_2::IComponentC*>(access_result->_handle);
        g_mock_component_c = dynamic_cast<::testing::StrictMock<::test_plugin_2::mock::MockComponentC>*>(pointer_to_component_c);
    }
    return result;
}