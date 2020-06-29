/**
 * @file
 * @copyright AUDI AG
 *            All right reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 */

#include <fep3/plugin/cpp/cpp_plugin_impl_arya.hpp>
#include <fep3/plugin/cpp/cpp_plugin_component_factory.h>
#include <fep3/components/base/component_base.h>
#include "test_plugin_1_intf.h"

class TestPlugin1Comp : public fep3::ComponentBase<ITestPlugin1>
{
    int32_t _value;
    public:
        TestPlugin1Comp() = default;
        ~TestPlugin1Comp() = default;
        
        int32_t get1() override
        {
            return _value;
        }
        void set1(int32_t value)
        {
            _value = value;
        }
};

void fep3_plugin_getPluginVersion(void(*callback)(void*, const char*), void* destination)
{
    callback(destination, "0.0.1");
}

fep3::ICPPPluginComponentFactory* fep3_plugin_cpp_arya_getFactory()
{
    return new fep3::CPPPluginComponentFactory<TestPlugin1Comp>;
}
