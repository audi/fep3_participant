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

#include "component_b_c_intf.h"
#include "component_b_intf.h"
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
 * @brief Access class for @ref test_plugin::IComponentB
 * Use this class to access an object behind IComponentB that resides in another binary (e. g. a shared library).
 */
class ComponentB
    : public ::fep3::plugin::c::access::arya::ComponentBase<IComponentB> // access wrapper for base class @ref fep3::arya::IComponent
{
public:
    /// Symbol name of the create function that is capable to create a component a
    static constexpr const char* const create_function_name = SYMBOL_test_plugin_createComponentB;
    /// Gets the function to get an instance of a component b that resides in a C plugin
    static decltype(&test_plugin_1_getComponentB) getGetterFunction()
    {
         return test_plugin_1_getComponentB;
    }
    /// Type of access structure
    using Access = test_plugin_SIComponentB;

    inline ComponentB(const Access& access, const std::shared_ptr<fep3::plugin::c::ISharedBinary>& shared_binary);
    ~ComponentB() override = default;
    
    // methods implementing ::test_plugin::IComponentB
    inline int32_t get() const override;

private:
    Access _access;
};

} // namespace access

namespace wrapper
{

class ComponentB
    : private ::fep3::plugin::c::wrapper::Helper<::test_plugin_1::IComponentB>
    , public ::fep3::plugin::c::wrapper::arya::Component
{
private:
    using Helper = Helper<::test_plugin_1::IComponentB>;
    using Handle = test_plugin_HIComponentB;

public:
    // interface IComponentB
    static inline fep3_plugin_c_InterfaceError get(Handle handle, int32_t* result) noexcept
    {
        return Helper::callWithResultParameter
            (handle
            , &IComponentB::get
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
 * Creates an object of type \p component_b_type
 * @pre \p component_b_type derives from IComponentB
 * @note This template function must be instantiated in the plugin's code (cpp file).
 *
 * @tparam component_b_type The type of the component to be created
 * @param [out] result Pointer to an access structure to be filled; if null, no object will be created
 * @param shared_binary_access Access structure to the shared binary the created component will reside in
 * @return C Interface error: fep3_plugin_c_interface_error_none if no error occurred, error code otherwise
 */
template<typename component_b_type>
fep3_plugin_c_InterfaceError createComponentB
    (const std::function<component_b_type*()>& factory
    , test_plugin_SIComponentB* result
    , const fep3_plugin_c_arya_SISharedBinary& shared_binary_access
    , const char* iid
    ) noexcept
{
    if(0 == strcmp(component_b_type::getComponentIID(), iid))
    {
        return ::fep3::plugin::c::wrapper::create
            (factory
            , result
            , shared_binary_access
            , [](component_b_type* pointer_to_object)
                {
                    return test_plugin_SIComponentB
                        {reinterpret_cast<test_plugin_HIComponentB>(static_cast<IComponentB*>(pointer_to_object))
                        , ::fep3::plugin::c::wrapper::arya::Component::AccessCreator()(pointer_to_object)
                        , ComponentB::get
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

template<typename component_b_type>
fep3_plugin_c_InterfaceError createComponentB
    (test_plugin_SIComponentB* result
    , const fep3_plugin_c_arya_SISharedBinary& shared_binary_access
    , const char* iid
    ) noexcept
{
    return detail::createComponentB
        (std::function<component_b_type*()>([]()
            {
                return new component_b_type;
            })
        , result
        , shared_binary_access
        , iid
        );
}

inline fep3_plugin_c_InterfaceError getComponentB
    (test_plugin_SIComponentB* access_result
    , const char* iid
    , fep3_arya_HIComponent handle_to_component
    ) noexcept
{
    if(0 == strcmp(::test_plugin_1::access::ComponentB::getComponentIID(), iid))
    {
        return ::fep3::plugin::c::wrapper::arya::get<::fep3::arya::IComponent, ::test_plugin_1::IComponentB>
            (access_result
            , handle_to_component
            , [](::test_plugin_1::IComponentB* pointer_to_object)
                {
                    return test_plugin_SIComponentB
                        {reinterpret_cast<test_plugin_HIComponentB>(static_cast<IComponentB*>(pointer_to_object))
                        , {} // don't provide access to IComponent interface
                        , ComponentB::get
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

ComponentB::ComponentB
    (const Access& access
    , const std::shared_ptr<::fep3::plugin::c::ISharedBinary>& shared_binary
    )
    : ::fep3::plugin::c::access::arya::ComponentBase<IComponentB>
        (access._component
        , shared_binary
        )
    , _access(std::move(access))
{}

int32_t ComponentB::get() const
{
    return ::fep3::plugin::c::access::Helper::callWithResultParameter(_access._handle, _access.get);
}

} // namespace access
} // namespace test_plugin_1

inline fep3_plugin_c_InterfaceError test_plugin_1_getComponentB
    (test_plugin_SIComponentB* access_result
    , const char* iid
    , fep3_arya_HIComponent handle_to_component
    )
{
    return ::test_plugin_1::wrapper::getComponentB
        (access_result
        , iid
        , handle_to_component
        );
}