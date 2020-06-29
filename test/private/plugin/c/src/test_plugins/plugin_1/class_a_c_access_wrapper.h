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

#include <functional>

#include "class_a_c_intf.h"
#include "class_a_intf.h"
#include <fep3/plugin/c/c_access/c_access_helper.h>
#include <fep3/plugin/c/c_wrapper/c_wrapper_helper.h>
#include <fep3/plugin/c/c_wrapper/destructor_c_wrapper.h>

namespace test_plugin_1
{
namespace access
{

/**
 * @brief Access class for test_plugin_1::IClassA
 * Use this class to access an object behind IClassA that resides in another binary (e. g. a shared library).
 */
class ClassA
    : public IClassA
    , private ::fep3::plugin::c::access::Helper
{
public:
    /// Type of C access structure
    using Access = test_plugin_1_SIClassA;

    ClassA(Access&& access, const std::shared_ptr<::fep3::plugin::c::ISharedBinary>& shared_binary);
    ~ClassA() override = default;

    // methods implementing test_plugin_1::IClassA
    virtual void set(int32_t value) override;
    virtual int32_t get() const override;

private:
    Access _access;
};

} // namespace access

namespace wrapper
{

class ClassA
    : private ::fep3::plugin::c::wrapper::Helper<IClassA>
{
private:
    using Handle = test_plugin_1_HIClassA;

public:
    // static methods transferring calls from the C interface to an object of IClassA
    static inline fep3_plugin_c_InterfaceError set(Handle handle, int32_t value) noexcept
    {
        return call
            (handle
            , &IClassA::set
            , value
            );
    }
    static inline fep3_plugin_c_InterfaceError get(Handle handle, int32_t* result) noexcept
    {
        return callWithResultParameter
            (handle
            , &IClassA::get
            , [](int32_t result)
                {
                    return result;
                }
            , result
            );
    }
};

namespace detail
{

/**
 * Creates an object of type \p ClassA
 * @pre \p ClassA derives from IClassA
 * @note This template function must be instantiated in the plugin's code (cpp file).
 *
 * @tparam ClassA The type of the object A to be created
 * @param [out] result Pointer to an access structure to be filled; if null, no object will be created
 * @param shared_binary_access Access structure to the shared binary the created object will reside in
 * @return C Interface error: fep3_plugin_c_interface_error_none if no error occurred, error code otherwise
 */
template<typename class_a_factory_type>
fep3_plugin_c_InterfaceError createClassA
    (class_a_factory_type&& factory
    , test_plugin_1_SIClassA* result
    , const fep3_plugin_c_arya_SISharedBinary& shared_binary_access
    ) noexcept
{
    return ::fep3::plugin::c::wrapper::create
        (factory
        , result
        , shared_binary_access
        , [](const auto& pointer_to_object)
            {
                return test_plugin_1_SIClassA
                    {reinterpret_cast<test_plugin_1_HIClassA>(pointer_to_object)
                    , &ClassA::set
                    , &ClassA::get
                    };
            }
        );
}

} // namespace detail

template<typename class_a>
fep3_plugin_c_InterfaceError createClassA
    (test_plugin_1_SIClassA* result
    , const fep3_plugin_c_arya_SISharedBinary& shared_binary_access
    ) noexcept
{
    return detail::createClassA
        (std::function<class_a*()>([]() -> class_a*
            {
                return new class_a;
            })
        , result
        , shared_binary_access
        );
}

} // namespace wrapper

namespace access
{

ClassA::ClassA
    (Access&& access
    , const std::shared_ptr<::fep3::plugin::c::ISharedBinary>& shared_binary
)
    : _access(std::move(access))
{
    setSharedBinary(shared_binary);
}

void ClassA::set(int32_t value)
{
    return call(_access._handle, _access.set, value);
}

int32_t ClassA::get() const
{
    return callWithResultParameter(_access._handle, _access.get);
}

} // namespace access
} // namespace test_plugin_1
