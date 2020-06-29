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

#include <map>
#include <memory>

#include <fep3/plugin/c/c_wrapper/c_wrapper_helper.h>
#include <fep3/components/base/c_intf/component_c_intf.h>
#include <fep3/components/base/c_access_wrapper/component_getter_function_getter_intf.h>
#include <fep3/components/base/component_registry.h>

namespace fep3
{
namespace plugin
{
namespace c
{
namespace wrapper
{
namespace arya
{

/**
 * Wrapper class for interface @ref fep3::arya::IComponents
 */
class Components : private Helper<fep3::arya::IComponents>
{
private:
    using Handle = fep3_arya_HComponents;
public:
    /**
     * Functor creating an access structure for @ref ::fep3::arya::IComponents
     */
    struct AccessCreator
    {
        /**
         * Creates an access structure to the component registry base as pointed to by @p pointer_to_component_registry
         *
         * @tparam components_access_type Access class for the components
         * @param pointer_to_component_registry Pointer to the component registry to create an access structure for
         * @return Access structure to the component registry
         */
        fep3_arya_SComponents operator()(const fep3::arya::IComponents* pointer_to_component_registry)
        {
            return fep3_arya_SComponents
                {reinterpret_cast<fep3_arya_HComponents>(const_cast<fep3::arya::IComponents*>(pointer_to_component_registry))
                , Components::findComponent
                };
        }
    };

    // static methods transferring calls from the C interface to an object of fep3::arya::IComponents
    /**
     * Finds the component
     * @param handle The handle to the component registry object to find the component interface in
     * @param access_result Pointer to the result access structure of finding the component interface
     * @param handle_to_component_getter_function_getters Handle to the component getter function getters
     * @param iid The component interface ID to be found
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within calls to the component registry
     */
    static inline fep3_plugin_c_InterfaceError findComponent
        (Handle handle
        , fep3_arya_SIComponentInterface* access_result
        , fep3_plugin_c_arya_HComponentGetterFunctionGetters handle_to_component_getter_function_getters
        , const char* iid
        ) noexcept
    {
        try
        {
            if(const auto& wrapped_this = reinterpret_cast<fep3::arya::IComponents*>(handle))
            {
                if(const auto& pointer_to_component = wrapped_this->findComponent(iid))
                {
                    if(nullptr != access_result)
                    {
                        void* component_getter_function = nullptr;
                        if(nullptr != handle_to_component_getter_function_getters)
                        {
                            const auto& pointer_to_component_getter_function_getters
                                = reinterpret_cast<std::shared_ptr<fep3::plugin::c::arya::IComponentGetterFunctionGetter>*>(handle_to_component_getter_function_getters);
                            if(nullptr != pointer_to_component_getter_function_getters)
                            {
                                const auto& component_getter_function_getters = *pointer_to_component_getter_function_getters;
                                if(component_getter_function_getters)
                                {
                                    component_getter_function = component_getter_function_getters->operator()(iid);
                                }
                            }
                        }
                        *access_result = fep3_arya_SIComponentInterface
                            {reinterpret_cast<fep3_arya_HIComponent>(pointer_to_component)
                            , component_getter_function
                            };
                        return fep3_plugin_c_interface_error_none;
                        
                    }
                    else
                    {
                        return fep3_plugin_c_interface_error_invalid_result_pointer;
                    }
                }
                else
                {
                    // no error, just didn't find the component
                    return fep3_plugin_c_interface_error_none;
                }
            }
            else
            {
                return fep3_plugin_c_interface_error_invalid_handle;
            }
        }
        catch(...)
        {
            return fep3_plugin_c_interface_error_exception_caught;
        }
    }
};

} // namespace arya
} // namespace wrapper
} // namespace c
} // namespace plugin
} // namespace fep3
