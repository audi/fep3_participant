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
#include <string.h>

#include "component_a_c_intf.h"
#include "component_a_intf.h"
#include <fep3/plugin/c/c_access/c_access_helper.h>
#include <fep3/plugin/c/c_wrapper/c_wrapper_helper.h>
#include <fep3/plugin/c/c_wrapper/destructor_c_wrapper.h>
#include <fep3/plugin/c/shared_binary_intf.h>
#include <fep3/components/base/component_base.h>
#include <fep3/components/base/c_access_wrapper/component_base_c_access.h>
#include <fep3/components/base/c_access_wrapper/component_c_wrapper.h>

namespace test_plugin_1
{
namespace access
{

/**
 * @brief Access class for @ref test_plugin::IComponentA
 * Use this class to access an object behind IComponentA that resides in another binary (e. g. a shared library).
 */
class ComponentA
    : public fep3::plugin::c::access::ComponentBase<IComponentA> // access wrapper for base class @ref fep3::arya::IComponent
{
public:
    /// Symbol name of the create function that is capable to create a component a
    static constexpr const char* const create_function_name = SYMBOL_test_plugin_createComponentA;
    /// Gets the function to get an instance of a component a that resides in a C plugin
    static decltype(&test_plugin_1_getComponentA) getGetterFunction()
    {
         return test_plugin_1_getComponentA;
    }
    /// Type of access structure
    using Access = test_plugin_SIComponentA;

    inline ComponentA(const Access& access, const std::shared_ptr<fep3::plugin::c::ISharedBinary>& shared_binary);
    ~ComponentA() override = default;

    // methods implementing test_plugin::IComponentA
    inline void set(int32_t value) override;
    inline int32_t get() const override;
    inline int32_t getFromComponentB() override;
    inline int32_t getFromComponentC() override;
    
private:
    Access _access;
};

} // namespace access

namespace wrapper
{

class ComponentA
    : private ::fep3::plugin::c::wrapper::Helper<::test_plugin_1::IComponentA>
    , public ::fep3::plugin::c::wrapper::arya::Component
{
private:
    using Helper = Helper<::test_plugin_1::IComponentA>;
    using Handle = test_plugin_HIComponentA;

public:
    // interface IComponentA
    static inline fep3_plugin_c_InterfaceError set(Handle handle, int32_t value) noexcept
    {
        return Helper::call
            (handle
            , &IComponentA::set
            , value
            );
    }
    static inline fep3_plugin_c_InterfaceError get(Handle handle, int32_t* result) noexcept
    {
        return Helper::callWithResultParameter
            (handle
            , &IComponentA::get
            , [](int32_t result)
                {
                    return result;
                }
            , result
            );
    }
    static inline fep3_plugin_c_InterfaceError getFromComponentB(Handle handle, int32_t* result) noexcept
    {
        return Helper::callWithResultParameter
            (handle
            , &IComponentA::getFromComponentB
            , [](int32_t result)
                {
                    return result;
                }
            , result
            );
    }
    static inline fep3_plugin_c_InterfaceError getFromComponentC(Handle handle, int32_t* result) noexcept
    {
        return Helper::callWithResultParameter
            (handle
            , &IComponentA::getFromComponentC
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
 * Creates an object of type \p component_a_type
 * @pre \p component_a_type derives from IComponentA
 * @note This template function must be instantiated in the plugin's code (cpp file).
 *
 * @tparam component_a_type The type of the component to be created
 * @param [out] result Pointer to an access structure to be filled; if null, no object will be created
 * @param shared_binary_access Access structure to the shared binary the created component will reside in
 * @return C Interface error: fep3_plugin_c_interface_error_none if no error occurred, error code otherwise
 */
template<typename component_a_type>
fep3_plugin_c_InterfaceError createComponentA
    (const std::function<component_a_type*()>& factory
    , test_plugin_SIComponentA* result
    , const fep3_plugin_c_arya_SISharedBinary& shared_binary_access
    , const char* iid
    ) noexcept
{
    if(0 == strcmp(component_a_type::getComponentIID(), iid))
    {
        return ::fep3::plugin::c::wrapper::create
            (factory
            , result
            , shared_binary_access
            , [](component_a_type* pointer_to_object)
                {
                    return test_plugin_SIComponentA
                        {reinterpret_cast<test_plugin_HIComponentA>(static_cast<IComponentA*>(pointer_to_object))
                        , ::fep3::plugin::c::wrapper::arya::Component::AccessCreator()(pointer_to_object)
                        , ComponentA::set
                        , ComponentA::get
                        , ComponentA::getFromComponentB
                        , ComponentA::getFromComponentC
                        };
                }
            );
    }
    else
    {
        // Note: not an error, this function is just not capable of creating the component for the passed IID
        return fep3_plugin_c_interface_error_none;
    }
}

} // namespace detail

template<typename component_a_type>
fep3_plugin_c_InterfaceError createComponentA
    (test_plugin_SIComponentA* access_result
    , const fep3_plugin_c_arya_SISharedBinary& shared_binary_access
    , const char* iid
    ) noexcept
{
    return detail::createComponentA
        (std::function<component_a_type*()>([]()
            {
                return new component_a_type;
            })
        , access_result
        , shared_binary_access
        , iid
        );
}

inline fep3_plugin_c_InterfaceError getComponentA
    (test_plugin_SIComponentA* access_result
    , const char* iid
    , fep3_arya_HIComponent handle_to_component
    ) noexcept
{
    if(0 == strcmp(::test_plugin_1::access::ComponentA::getComponentIID(), iid))
    {
        return ::fep3::plugin::c::wrapper::arya::get<::fep3::arya::IComponent, ::test_plugin_1::IComponentA>
            (access_result
            , handle_to_component
            , [](::test_plugin_1::IComponentA* pointer_to_object)
                {
                    return test_plugin_SIComponentA
                        {reinterpret_cast<test_plugin_HIComponentA>(static_cast<IComponentA*>(pointer_to_object))
                        , {} // don't provide access to IComponent interface
                        , ComponentA::set
                        , ComponentA::get
                        , ComponentA::getFromComponentB
                        , ComponentA::getFromComponentC
                        };
                }
            );
    }
    else
    {
        // Note: not an error, this function is just not capable of getting the component for the passed IID
        return fep3_plugin_c_interface_error_none;
    }
}

} // namespace wrapper

namespace access
{

ComponentA::ComponentA
    (const Access& access
    , const std::shared_ptr<::fep3::plugin::c::ISharedBinary>& shared_binary
    )
    : ::fep3::plugin::c::access::ComponentBase<IComponentA>
        (access._component
        , shared_binary
        )
    , _access(std::move(access))
{}

void ComponentA::set(int32_t value)
{
    return ::fep3::plugin::c::access::Helper::call(_access._handle, _access.set, value);
}

int32_t ComponentA::get() const
{
    return ::fep3::plugin::c::access::Helper::callWithResultParameter(_access._handle, _access.get);
}

int32_t ComponentA::getFromComponentB()
{
    return ::fep3::plugin::c::access::Helper::callWithResultParameter(_access._handle, _access.getFromComponentB);
}

int32_t ComponentA::getFromComponentC()
{
    return ::fep3::plugin::c::access::Helper::callWithResultParameter(_access._handle, _access.getFromComponentC);
}

} // namespace access
} // namespace test_plugin_1

inline fep3_plugin_c_InterfaceError test_plugin_1_getComponentA
    (test_plugin_SIComponentA* access_result
    , const char* iid
    , fep3_arya_HIComponent handle_to_component
    )
{
    return ::test_plugin_1::wrapper::getComponentA
        (access_result
        , iid
        , handle_to_component
        );
}