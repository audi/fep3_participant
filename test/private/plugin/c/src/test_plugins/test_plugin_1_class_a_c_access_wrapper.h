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

#include "test_plugin_1/class_a_c_intf.h"
#include "test_plugin_1/class_a_intf.h"
#include <fep3/plugin/c/c_access/c_access_helper.h>
#include <fep3/plugin/c/c_wrapper/c_wrapper_helper.h>
#include <fep3/plugin/c/c_wrapper/destructor_c_wrapper.h>

namespace test_plugin_1
{
namespace access
{

/**
 * @brief Access class for test_plugin_1::ITestPlugin1ClassA
 * Use this class to access an object behind ITestPlugin1ClassA that resides in another binary (e. g. a shared library).
 */
class TestPlugin1ClassA : public ITestPlugin1ClassA
{
public:
    /// Type of access structure
    using Access = SITestPlugin1ClassA;

    TestPlugin1ClassA(Access&& access, const std::shared_ptr<fep3::plugin::c::ISharedBinary>& shared_binary);
    virtual ~TestPlugin1ClassA() override = default;

    // methods implementing test_plugin_1::ITestPlugin1ClassA
    virtual void set(int32_t value) override;
    virtual int32_t get() const override;

private:
    Access _access;
};

} // namespace access

namespace wrapper
{

class TestPlugin1ClassA
    : private fep3::plugin::c::wrapper::Helper<::test_plugin::ITestPlugin1ClassA>
{
private:
    using Helper = Helper<::test_plugin::ITestPlugin1ClassA>;
    using Handle = HITestPlugin1ClassA;

    // interface ITestPlugin1ClassA
    static inline fep3_plugin_c_CInterfaceError set(Handle handle, int32_t value) noexcept
    {
        return Helper::call
            (handle
            , &ITestPlugin1ClassA::set
            , value
            );
    }
    static inline fep3_plugin_c_CInterfaceError get(Handle handle, int32_t* result) noexcept
    {
        return Helper::callWithResultParameter
            (handle
            , &ITestPlugin1ClassA::get
            , [](int32_t result)
                {
                    return result;
                }
            , result
            );
    }

public:
    /// Type of access structure
    using Access = SITestPlugin1ClassA;

    /**
     * Creates an object of type \p TestPlugin1ClassA
     * @pre \p TestPlugin1ClassA derives from ITestPlugin1ClassA
     * @note This template function must be instantiated in the plugin's code (cpp file).
     *
     * @tparam TestPlugin1ClassA The type of the object A to be created
     * @param [out] result Pointer to an access structure to be filled; if null, no object will be created
     * @param shared_binary_access Access structure to the shared binary the created object will reside in
     * @return C Interface error: C_INTERFACE_ERROR_NONE if no error occurred, error code otherwise
     */
    template<typename TestPlugin1ClassA>
    static fep3_plugin_c_CInterfaceError create(Access* result, const fep3_plugin_c_arya_SISharedBinary& shared_binary_access) noexcept
    {
        return Helper::create<TestPlugin1ClassA>
            (result
            , shared_binary_access
            , [](const auto& pointer_to_object)
                {
                    return Access
                        {reinterpret_cast<Handle>(pointer_to_object)
                        , &set
                        , &get
                        };
                }
            );
    }
};

} // namespace wrapper

namespace access
{

TestPlugin1ClassA::TestPlugin1ClassA
    (Access&& access
    , const std::shared_ptr<fep3::plugin::c::ISharedBinary>& shared_binary
)
    : _access(std::move(access))
{
    setSharedBinary(shared_binary);
}

void TestPlugin1ClassA::set(int32_t value)
{
    return call(_access._handle, _access.set1, value);
}

int32_t TestPlugin1ClassA::get() const
{
    return callWithResultParameter(_access._handle, _access.get1);
};

} // namespace access
} // namespace test_plugin_1