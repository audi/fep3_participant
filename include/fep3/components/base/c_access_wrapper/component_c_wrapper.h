/**
 * @file
 * Copyright &copy; AUDI AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * @attention Changes in this file must be reflected in the corresponding C++ interface file component_intf.h
 */

#pragma once

#include <memory>

#include <fep3/components/base/component_intf.h>
#include <fep3/components/base/c_intf/component_c_intf.h>
#include <fep3/plugin/c/c_wrapper/c_wrapper_helper.h>
#include <fep3/components/base/c_access_wrapper/components_c_access.h>

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
 * Wrapper class for interface @ref ::fep3::arya::IComponent
 */
class Component : private Helper<fep3::arya::IComponent>
{
private:
    using Handle = fep3_arya_HIComponent;

public:
    /**
     * Functor creating an access structure for @ref ::fep3::arya::IComponent
     */
    struct AccessCreator
    {
        /**
         * Creates an access structure to the component as pointed to by @p pointer_to_component
         *
         * @tparam component_access_type Access class for the component
         * @param pointer_to_component Pointer to the component to create an access structure for
         * @return Access structure to the component
         */
        template<typename component_access_type>
        fep3_arya_SIComponent operator()(component_access_type* pointer_to_component)
        {
            return fep3_arya_SIComponent
                {reinterpret_cast<fep3_arya_HIComponent>(static_cast<::fep3::arya::IComponent*>(pointer_to_component))
                , fep3_plugin_c_arya_SDestructionManager
                    {reinterpret_cast<fep3_plugin_c_arya_HDestructionManager>(static_cast<::fep3::plugin::c::arya::DestructionManager*>(pointer_to_component))
                    , Destructor::destroy
                    }
                , ::fep3::plugin::c::wrapper::arya::Component::createComponent
                , ::fep3::plugin::c::wrapper::arya::Component::destroyComponent
                , ::fep3::plugin::c::wrapper::arya::Component::initialize
                , ::fep3::plugin::c::wrapper::arya::Component::tense
                , ::fep3::plugin::c::wrapper::arya::Component::relax
                , ::fep3::plugin::c::wrapper::arya::Component::start
                , ::fep3::plugin::c::wrapper::arya::Component::stop
                , ::fep3::plugin::c::wrapper::arya::Component::pause
                , ::fep3::plugin::c::wrapper::arya::Component::deinitialize
                };
        }
    };

    // static methods transferring calls from the C interface to an object of fep3::arya::IComponent
    /**
     * Calls @ref fep3::arya::IComponent::createComponent on the object identified by \p handle
     * @param handle The handle to the component object to call @ref fep3::arya::IComponent::createComponent on
     * @param result Pointer to the result of the call of @ref fep3::arya::IComponent::createComponent on the component object
     * @param destruction_manager_access_result Pointer to the result of a destruction manager access for the remote object
     * @param components_access Access to components
     * @param handle_to_component_getter_function_getters Handle to the component getter function getters
     * @param component_getter_function_getters_destruction_manager_access Access to the destruction manager for the component getter function getters
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within @ref fep3::arya::IComponent::createComponent
     */
    static inline fep3_plugin_c_InterfaceError createComponent
        (Handle handle
        , int32_t* result
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_access_result
        , fep3_arya_SComponents components_access
        , fep3_plugin_c_arya_HComponentGetterFunctionGetters handle_to_component_getter_function_getters
        , fep3_plugin_c_arya_SDestructionManager component_getter_function_getters_destruction_manager_access
        )
    {
        try
        {
            if(const auto& wrapped_this = reinterpret_cast<fep3::arya::IComponent*>(handle))
            {
                if(nullptr != destruction_manager_access_result)
                {
                    std::shared_ptr<fep3::plugin::c::access::arya::Components> shared_ptr_to_components;
                    // create the local object only if the (handle to the) remote object is valid
                    if(nullptr != components_access._handle)
                    {
                        std::deque<std::unique_ptr<IDestructor>> destructors;
                        destructors.push_back(std::make_unique<fep3::plugin::c::access::arya::Destructor<fep3_plugin_c_arya_SDestructionManager>>
                            (component_getter_function_getters_destruction_manager_access));
                        auto pointer_to_shared_ptr_to_object = new std::shared_ptr<fep3::plugin::c::access::arya::Components>
                            (new fep3::plugin::c::access::arya::Components
                                (components_access
                                , std::move(destructors)
                                , handle_to_component_getter_function_getters
                                )
                            );
                        shared_ptr_to_components = *pointer_to_shared_ptr_to_object;
    
                        // reference to the local object must be released when the remote object is destroyed, so we add a (new) shared reference to the reference manager
                        auto destruction_manager = new DestructionManager;
                        destruction_manager->addDestructor
                            (std::make_unique<OtherDestructor<typename std::remove_pointer<typename std::remove_reference<decltype(pointer_to_shared_ptr_to_object)>::type>::type>>
                            (pointer_to_shared_ptr_to_object));
                        *destruction_manager_access_result = fep3_plugin_c_arya_SDestructionManager
                            {reinterpret_cast<fep3_plugin_c_arya_HDestructionManager>(static_cast<DestructionManager*>(destruction_manager))
                            , wrapper::Destructor::destroy
                            };
                    }
                    if(nullptr != result)
                    {
                        *result = wrapped_this->createComponent(shared_ptr_to_components).getErrorCode();
                        return fep3_plugin_c_interface_error_none;
                    }
                    else
                    {
                        return fep3_plugin_c_interface_error_invalid_result_pointer;
                    }
                }
                else
                {
                    return fep3_plugin_c_interface_error_invalid_result_pointer;
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
    /**
     * Calls @ref fep3::arya::IComponent::destroyComponent(...) on the object identified by \p handle
     * @param handle The handle to the component object to call @ref fep3::arya::IComponent::destroyComponent on
     * @param result Pointer to the result of the call of @ref fep3::arya::IComponent::destroyComponent on the component object
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IComponent::destroyComponent
     */
    static inline fep3_plugin_c_InterfaceError destroyComponent(Handle handle, int32_t* result)
    {
        return callWithResultParameter
            (handle
            , &fep3::arya::IComponent::destroyComponent
            , [](const Result& fep_result)
                {
                    // note: other information than error code is lost here; maybe improve it
                    // by returning a structure with all information from Result
                    return fep_result.getErrorCode();
                }
            , result
            );
    }
    /**
     * Calls @ref fep3::arya::IComponent::initialize(...) on the object identified by \p handle
     * @param handle The handle to the component object to call @ref fep3::arya::IComponent::initialize on
     * @param result Pointer to the result of the call of @ref fep3::arya::IComponent::initialize on the component object
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IComponent::initialize
     */
    static inline fep3_plugin_c_InterfaceError initialize(Handle handle, int32_t* result)
    {
        return callWithResultParameter
            (handle
            , &fep3::arya::IComponent::initialize
            , [](const Result& result)
                {
                    // note: other information than error code is lost here; maybe improve it
                    // by returning a structure with all information from Result
                    return result.getErrorCode();
                }
            , result
            );
    }
    /**
     * Calls @ref fep3::arya::IComponent::tense(...) on the object identified by \p handle
     * @param handle The handle to the component object to call @ref fep3::arya::IComponent::tense on
     * @param result Pointer to the result of the call of @ref fep3::arya::IComponent::tense on the component object
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IComponent::tense
     */
    static inline fep3_plugin_c_InterfaceError tense(Handle handle, int32_t* result)
    {
        return callWithResultParameter
            (handle
            , &fep3::arya::IComponent::tense
            , [](const Result& result)
                {
                    // note: other information than error code is lost here; maybe improve it
                    // by returning a structure with all information from Result
                    return result.getErrorCode();
                }
            , result
            );
    }
    /**
     * Calls @ref fep3::arya::IComponent::relax(...) on the object identified by \p handle
     * @param handle The handle to the component object to call @ref fep3::arya::IComponent::relax on
     * @param result Pointer to the result of the call of @ref fep3::arya::IComponent::relax on the component object
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IComponent::relax
     */
    static inline fep3_plugin_c_InterfaceError relax(Handle handle, int32_t* result)
    {
        return callWithResultParameter
        (handle
            , &fep3::arya::IComponent::relax
            , [](const Result& result)
        {
            // note: other information than error code is lost here; maybe improve it
            // by returning a structure with all information from Result
            return result.getErrorCode();
        }
            , result
            );
    }
    /**
     * Calls @ref fep3::arya::IComponent::start(...) on the object identified by \p handle
     * @param handle The handle to the component object to call @ref fep3::arya::IComponent::start on
     * @param result Pointer to the result of the call of @ref fep3::arya::IComponent::start on the component object
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IComponent::start
     */
    static inline fep3_plugin_c_InterfaceError start(Handle handle, int32_t* result)
    {
        return callWithResultParameter
            (handle
            , &fep3::arya::IComponent::start
            , [](const Result& result)
                {
                    // note: other information than error code is lost here; maybe improve it
                    // by returning a structure with all information from Result
                    return result.getErrorCode();
                }
            , result
            );
    }
    /**
     * Calls @ref fep3::arya::IComponent::stop(...) on the object identified by \p handle
     * @param handle The handle to the component object to call @ref fep3::arya::IComponent::stop on
     * @param result Pointer to the result of the call of @ref fep3::arya::IComponent::stop on the component object
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IComponent::stop
     */
    static inline fep3_plugin_c_InterfaceError stop(Handle handle, int32_t* result)
    {
        return callWithResultParameter
            (handle
            , &fep3::arya::IComponent::stop
            , [](const Result& fep_result)
                {
                    // note: other information than error code is lost here; maybe improve it
                    // by returning a structure with all information from Result
                    return fep_result.getErrorCode();
                }
            , result
            );
    }
    /**
     * Calls @ref fep3::arya::IComponent::pause(...) on the object identified by \p handle
     * @param handle The handle to the component object to call @ref fep3::arya::IComponent::pause on
     * @param result Pointer to the result of the call of @ref fep3::arya::IComponent::stop on the component object
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IComponent::pause
     */
    static inline fep3_plugin_c_InterfaceError pause(Handle handle, int32_t* result)
    {
        return callWithResultParameter
        (handle
            , &fep3::arya::IComponent::pause
            , [](const Result& fep_result)
        {
            // note: other information than error code is lost here; maybe improve it
            // by returning a structure with all information from Result
            return fep_result.getErrorCode();
        }
            , result
            );
    }
    /**
     * Calls @ref fep3::arya::IComponent::deinitialize(...) on the object identified by \p handle
     * @param handle The handle to the component object to call @ref fep3::arya::IComponent::deinitialize on
     * @param result Pointer to the result of the call of @ref fep3::arya::IComponent::deinitialize on the component object
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IComponent::deinitialize
     */
    static inline fep3_plugin_c_InterfaceError deinitialize(Handle handle, int32_t* result)
    {
        return callWithResultParameter
            (handle
            , &fep3::arya::IComponent::deinitialize
            , [](const Result& result)
                {
                    // note: other information than error code is lost here; maybe improve it
                    // by returning a structure with all information from Result
                    return result.getErrorCode();
                }
            , result
            );
    }
};

} // namespace arya
} // namespace wrapper
} // namespace c
} // namespace plugin
} // namespace fep3
