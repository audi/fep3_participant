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
#include <fep3/components/base/component_base.h>
#include "test_plugin_2_intf.h"
#include "test_plugin_2_additional_intf.h"

class TestPlugin2Comp : public fep3::ComponentBase<ITestPlugin2>
{
    int32_t _value;
    public:
        TestPlugin2Comp() = default;
        ~TestPlugin2Comp() = default;
        
        int32_t get2() override
        {
            return _value;
        }
        void set2(int32_t value)
        {
            _value = value;
        }
};

class TestPlugin2AdditionalComp : public fep3::ComponentBase<ITestPlugin2Additional>
{
    std::string _value = "additional";
public:
    TestPlugin2AdditionalComp() = default;
    ~TestPlugin2AdditionalComp() = default;

    std::string getAdditional() override
    {
        return _value;
    }
};

class MyPluginComponentFactory : public fep3::ICPPPluginComponentFactory
{
private:
    std::unique_ptr<fep3::IComponent> createComponent(const std::string& component_iid) const
    {
        if (component_iid == fep3::getComponentIID<ITestPlugin2Additional>())
        {
            return std::unique_ptr<fep3::IComponent>(new TestPlugin2AdditionalComp());
        }
        else if (component_iid == fep3::getComponentIID<ITestPlugin2>())
        {
            return std::unique_ptr<fep3::IComponent>(new TestPlugin2Comp());
        }
        else
        {
            return {};
        }
    }
};

void fep3_plugin_getPluginVersion(void(*callback)(void*, const char*), void* destination)
{
    callback(destination, "0.0.2");
}

fep3::ICPPPluginComponentFactory* fep3_plugin_cpp_arya_getFactory()
{
    return new MyPluginComponentFactory;
}

