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

#include <deque>
#include <memory>
#include <utility>

#include <fep3/plugin/c/c_intf/c_intf_errors.h>
#include <fep3/plugin/c/shared_binary_intf.h>
#include <fep3/plugin/c/c_wrapper/shared_binary_c_wrapper.h>
#include <fep3/plugin/c/c_access/c_access_exception.h>
#include <fep3/plugin/c/c_access/destructor_c_access.h>
#include <fep3/plugin/c/c_wrapper/destructor_c_wrapper.h>
#include <fep3/plugin/c/destruction_manager.h>

namespace fep3
{
namespace plugin
{
namespace c
{
namespace access
{
namespace arya
{

/**
 * @brief Access helper to access a remote object from within C++ through a binary boundary.
 * The (static) methods of this class provide facilities to write an access class for a
 * C++ interface by one single statement per access class method.
 * Example: For a C++ interface to be wrapped:
 * class IMyStuff
 * {
 * public:
 *     virtual ~IMyStuff() = default;
 *     virtual int32_t getValue() const = 0;
 * };
 * The access class looks like this when using the Helper class:
 * class AccessToIMyStuff
 *     : public IMyStuff
 *     , private ::fep3::plugin::c::access::arya::Helper
 * {
 * public:
 *     /// Type of C access structure
 *     using Access = SIMyStuff;
 *     AccessToIMyStuff(Access&& access);
 *     ~AccessToIMyStuff() override = default;
 *     // methods implementing IMyStuff
 *     virtual int32_t getValue() const override
 *     {
 *         // using the Helper class the method implementation can be done in one single statement
 *         return callWithResultParameter(_access._handle, _access.getValue);
 *     }
 * private:
 *     Access _access;
 * };
 */
class Helper
{
protected:
    /// Type traits to get the type of the first and second function parameter out of a function pointer type
    template<typename> struct FunctionParameters;
    /// Template specialization for a function pointer taking two or more parameters
    template<typename return_type, typename parameter_1, typename parameter_2, typename... args>
    struct FunctionParameters<return_type(*)(parameter_1, parameter_2, args...)>
    {
        /// Alias for first parameter
        using Parameter1Type = parameter_1;
        /// Alias for second parameter
        using Parameter2Type = parameter_2;
    };

    /**
     * Calls the passed \p function by passing the \p handle as well as the \p arguments to it.
     * This method can be used to wrap access to a remote object's method that returns void.
     * @pre The passed \p function must be implicitly convertible to:
     *      fep3_plugin_c_InterfaceError(*)(handle_type, argument_types...)
     *
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam function_type The type of the function to be called
     * @tparam argument_types Parameter pack holding all parameters to be passed to \p function
     * @param handle The handle of the object to call the corresponding member function on
     * @param function The pointer to the function to be called
     * @param arguments The arguments to be passed to the \p function
     * @throw Throws an exception of type \ref Exception if the \p function returns other than fep3_plugin_c_interface_error_none
     */
    template<typename handle_type, typename function_type, typename... argument_types>
    static void call(handle_type handle, function_type function, argument_types... arguments)
    {
        fep3_plugin_c_InterfaceError error = (*function)
            (handle
            , std::forward<argument_types>(arguments)...
            );
        if(fep3_plugin_c_interface_error_none != error)
        {
            throw Exception(error);
        }
    }

    /**
     * Calls the passed \p function and passes the \p handle, a pointer to a result object
     * as well as the \p arguments to it.
     * This method can be used to wrap access to a remote object's method that returns non-void.
     * @pre The passed \p function must be implicitly convertible to:
     *      fep3_plugin_c_InterfaceError(*)(handle_type, *result_type, argument_types...)
     *      where result_type can be any type that is default constructible.
     *
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam function_type The type of the function to be called
     * @tparam argument_types Parameter pack holding all parameters to be passed to \p function
     * @param handle The handle of the object to call the corresponding member function on
     * @param function The function to be called
     * @param arguments The arguments to be passed to the \p function
     * @throw Throws an exception of type \ref Exception if the \p function returns other than fep3_plugin_c_interface_error_none
     * @return The result of the function call, i. e. the content that was filled to the second parameter
     *          of \p function during the function call
     */
    template<typename handle_type, typename function_type, typename... argument_types>
    static auto callWithResultParameter(handle_type handle, function_type function, argument_types... arguments)
    {
        // the second parameter of function_type is the result pointer
        typename std::remove_pointer<typename FunctionParameters<function_type>::Parameter2Type>::type result{};
        fep3_plugin_c_InterfaceError error = (*function)
            (handle
            , &result
            , std::forward<argument_types>(arguments)...
            );
        if(fep3_plugin_c_interface_error_none != error)
        {
            throw Exception(error);
        }
        return result;
    }

    /**
     * Calls the passed \p function and passes the \p handle, a callback function, a pointer to a result object
     * as well as the \p arguments to it.
     * This method can be used to wrap access to a remote object's method that returns non-void.
     * @pre The passed \p function must be implicitly convertible to:
     *      fep3_plugin_c_InterfaceError(*)(handle_type, void(*)(void*, result_type), void*, argument_types...)
     *      where result_type can be any type that is default constructible.
     *
     * @tparam return_type The type of the return value of this method
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam function_type The type of the function to be called
     * @tparam argument_types Parameter pack holding all parameters to be passed to \p function
     * @param handle The handle of the object to call the corresponding member function on
     * @param function The function to be called
     * @param arguments The arguments to be passed to the \p function
     * @throw Throws an exception of type \ref Exception if the \p function returns other than fep3_plugin_c_interface_error_none
     * @return The result of the function call, i. e. the second argument of the callback.
     */
    template<typename return_type, typename handle_type, typename function_type, typename... argument_types>
    static auto callWithResultCallback
        (handle_type handle
        , function_type function
        , argument_types... arguments
        )
    {
        return_type result{};
        fep3_plugin_c_InterfaceError error = (*function)
            (handle
            , [](void* destination, auto local_result)
                {
                    *reinterpret_cast<return_type*>(destination) = local_result;
                }
            , static_cast<void*>(&result)
            , std::forward<argument_types>(arguments)...
            );
        if(fep3_plugin_c_interface_error_none != error)
        {
            throw Exception(error);
        }
        return result;
    }

    /**
     * Calls the passed \p function and passes the \p handle, a callback function, a pointer to a result assigner
     * as well as the \p arguments to it. The callback function might be called multiple times.
     * This method can be used to wrap access to a remote object's method that returns an array or a container.
     * @pre The passed \p function must be implicitly convertible to:
     *      fep3_plugin_c_InterfaceError(*)(handle_type, void(*)(void*, result_type), void*, argument_types...)
     *      where result_type can be any type that is default constructible.
     *
     * @tparam destination_type The type of the destination container where to store all values as passed via the callback
     * @tparam callback_parameter_type The type of the callback parameter
     * @tparam return_type The type of the return value of this method
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam function_type The type of the function to be called
     * @tparam conversion_function_type The type of a conversion function capable to convert the parameter of the
     *          callback to destination_type::value_type
     * @tparam argument_types Parameter pack holding all parameters to be passed to \p function
     * @param handle The handle of the object to call the corresponding member function on
     * @param function The function to be called
     * @param conversion_function The conversion function capable to convert the parameter of the
     *                             callback to destination_type::value_type
     * @param assignment_method The pointer to the method that can perform the assignment of the parameter of the
     *                           callback - converted by the \p conversion_function - to an instance of the
     *                           \p destination_type
     * @param arguments The arguments to be passed to the \p function
     * @throw Throws an exception of type \ref Exception if the \p function returns other than fep3_plugin_c_interface_error_none
     * @return The container filled with the values of the second argument of the recurrings callbacks
     */
    template
        <typename destination_type
        , typename callback_parameter_type
        , typename return_type
        , typename handle_type
        , typename function_type
        , typename conversion_function_type
        , typename... argument_types
        >
    static auto callWithRecurringResultCallback
        (handle_type handle
        , function_type function
        , conversion_function_type&& conversion_function
        , return_type(destination_type::*assignment_method)(typename destination_type::value_type&&)
        , argument_types... arguments
        )
    {
        destination_type result{};
        auto assigner = [conversion_function, &result, assignment_method](const callback_parameter_type& value)
        {
            (result.*assignment_method)(conversion_function(value));
        };
        using assigner_type = decltype(assigner);
        fep3_plugin_c_InterfaceError error = (*function)
            (handle
            , [](void* destination, callback_parameter_type local_result)
                {
                    (*reinterpret_cast<assigner_type*>(destination))(local_result);
                }
            , static_cast<void*>(&assigner)
            , std::forward<argument_types>(arguments)...
            );
        if(fep3_plugin_c_interface_error_none != error)
        {
            throw Exception(error);
        }
        return result;
    }

    /**
     * Calls the passed \p function and passes the \p handle, a pointer to an access structure to a shared binary
     * , a reference to the access structure for the object to get the wrapped unique_ptr to
     * , as well as the \p arguments to it.
     * This method can be used to wrap access to a remote object's method that returns a unique_ptr.
     * @pre The passed \p function must be implicitly convertible to:
     *      fep3_plugin_c_InterfaceError(*)(handle_type, fep3_plugin_c_arya_SISharedBinary*, access_type*, argument_types...)
     *
     * @tparam access_object_type The type of the access object that wraps access to the remote object
     * @tparam access_type The type of the C access structure that provides access to the remote object
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam function_type The type of the function to be called
     * @tparam argument_types Parameter pack holding all parameters to be passed to \p function
     * @param handle The handle of the object to call the corresponding member function on
     * @param function The function to be called
     * @param arguments The arguments to be passed to the \p function
     * @throw Throws an exception of type \ref Exception if the \p function returns other than fep3_plugin_c_interface_error_none
     * @return Unique pointer the access object providing access to the remote object
     */
    template
        <typename access_object_type
        , typename access_type
        , typename handle_type
        , typename function_type
        , typename... argument_types
        >
    static std::unique_ptr<access_object_type> getUniquePtr
        (handle_type handle
        , function_type function
        , argument_types... arguments
        )
    {
        fep3_plugin_c_arya_SDestructionManager destruction_manager_access{};
        access_type access{};
        fep3_plugin_c_InterfaceError error = (*function)
            (handle
            , &destruction_manager_access
            , &access
            , std::forward<argument_types>(arguments)...
            );
        if(fep3_plugin_c_interface_error_none != error)
        {
            throw Exception(error);
        }
        if(nullptr != access._handle)
        {
            std::deque<std::unique_ptr<IDestructor>> destructors;
            // ownership transfer: when this object is destroyed, also destroy the remote object
            destructors.push_back(std::make_unique<Destructor<fep3_plugin_c_arya_SDestructionManager>>(destruction_manager_access));
            return std::make_unique<access_object_type>
                (access
                , std::move(destructors)
                );
        }
        else
        {
            return {};
        }
    }

    /**
     * Calls the passed \p function and passes the \p handle
     * , a reference to the access structure for the object to get the wrapped unique_ptr to
     * , as well as the \p arguments to it.
     * This method can be used to wrap access to a remote object's method that returns a unique_ptr.
     * @pre The passed \p function must be implicitly convertible to:
     *      fep3_plugin_c_InterfaceError(*)(handle_type, fep3_plugin_c_arya_SISharedBinary*, access_type*, argument_types...)
     *
     * @tparam access_object_type The type of the access object that wraps access to the remote object
     * @tparam access_type The type of the C access structure that provides access to the remote object
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam function_type The type of the function to be called
     * @tparam argument_types Parameter pack holding all parameters to be passed to \p function
     * @param handle The handle of the object to call the corresponding member function on
     * @param function The function to be called
     * @param arguments The arguments to be passed to the \p function
     * @throw Throws an exception of type \ref Exception if the \p function returns other than fep3_plugin_c_interface_error_none
     * @return Unique pointer the access object providing access to the remote object
     */
    template
        <typename access_object_type
        , typename access_type
        , typename handle_type
        , typename function_type
        , typename... argument_types
        >
    static std::shared_ptr<access_object_type> getSharedPtr
        (handle_type handle
        , function_type function
        , argument_types... arguments
        )
    {
        fep3_plugin_c_arya_SDestructionManager destruction_manager_access{};
        access_type access{};
        fep3_plugin_c_InterfaceError error = (*function)
            (handle
            , &destruction_manager_access
            , &access
            , std::forward<argument_types>(arguments)...
            );
        if(fep3_plugin_c_interface_error_none != error)
        {
            throw Exception(error);
        }
        if(nullptr != access._handle)
        {
            std::deque<std::unique_ptr<IDestructor>> destructors;
            // shared ownership: when the local object is destroyed, release reference to the remote object
            destructors.push_back(std::make_unique<Destructor<fep3_plugin_c_arya_SDestructionManager>>(destruction_manager_access));
            return std::make_shared<access_object_type>
                (access
                , std::move(destructors)
                );
        }
        else
        {
            return {};
        }
    }

    /**
     * Calls the passed \p function and passes the \p handle, an access structure to the object to be transferred
     * as well as the \p arguments to it.
     * This method can be used to wrap access to a remote object's method that takes a unique_ptr as parameter
     * (implies potential ownership transfer).
     * @pre The passed \p function must be implicitly convertible to:
     *      fep3_plugin_c_InterfaceError(*)(handle_type, result_type*, fep3_plugin_c_arya_SISharedBinary, access_type, argument_types...)
     *
     * @tparam return_type The type of the return value of this method
     * @tparam object_type The type of the object to be transferred
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam function_type The type of the function to be called
     * @tparam access_creator_type The type of the callable that creates an instance of the C access structure to \p unique_ptr_to_object
     * @tparam argument_types Parameter pack holding all parameters to be passed to \p function
     * @param unique_ptr_to_object Unique pointer to the object to be transferred
     * @param handle The handle of the object to call the corresponding member function on
     * @param function The function to be called
     * @param access_creator Callable that creates an instance of the C access structure to \p unique_ptr_to_object
     * @param arguments The arguments to be passed to the \p function
     * @throw Throws an exception of type \ref Exception if the \p function returns other than fep3_plugin_c_interface_error_none
     * @return The result of the function call, i. e. the content that was filled to the second parameter
     *          of \p function during the function call
     */
    template
        <typename return_type
        , typename object_type
        , typename handle_type
        , typename function_type
        , typename access_creator_type
        , typename... argument_types
        >
    static return_type transferUniquePtrWithResultParameter
        (std::unique_ptr<object_type> unique_ptr_to_object
        , handle_type&& handle
        , function_type function
        , access_creator_type&& access_creator
        , argument_types&&... arguments
        )
    {
        // the second parameter of function_type is the result pointer
        typename std::remove_pointer<typename FunctionParameters<function_type>::Parameter2Type>::type result{};
        const auto& pointer_to_object = unique_ptr_to_object.release();
        auto destruction_manager = new DestructionManager;
        // the local object must be destroyed when the remote object is destroyed, so we add a (new) shared reference to the reference manager
        destruction_manager->addDestructor
            (std::make_unique<OtherDestructor<typename std::remove_pointer<typename std::remove_reference<decltype(pointer_to_object)>::type>::type>>
            (pointer_to_object));
        auto destruction_manager_access = fep3_plugin_c_arya_SDestructionManager
            {reinterpret_cast<fep3_plugin_c_arya_HDestructionManager>(static_cast<DestructionManager*>(destruction_manager))
            , wrapper::Destructor::destroy
            };
        fep3_plugin_c_InterfaceError error = (*function)
            (handle
            , &result
            , destruction_manager_access
            , access_creator(pointer_to_object)
            , std::forward<argument_types>(arguments)...
            );
        if(fep3_plugin_c_interface_error_none != error)
        {
            throw Exception(error);
        }
        return result;
    }

    /**
     * Calls the passed \p function and passes the \p handle, an access structure to the object to be transferred
     * , as well as the \p arguments to it.
     * This method can be used to wrap access to a remote object's method that takes a shared_ptr as parameter (implies shared ownership)
     * and returns void.
     * @pre The passed \p function must be implicitly convertible to:
     *      fep3_plugin_c_InterfaceError(*)(handle_type, fep3_plugin_c_arya_SDestructionManager, access_type, argument_types...)
     *
     * @tparam object_type The type of the object to be transferred
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam function_type The type of the function to be called
     * @tparam access_creator_type The type of the callable that creates an instance of the C access structure to \p unique_ptr_to_object
     * @tparam argument_types Parameter pack holding all parameters to be passed to \p function
     * @param shared_ptr_to_object Shared pointer to the object to be transferred
     * @param handle The handle of the object to call the corresponding member function on
     * @param function The function to be called
     * @param access_creator Callable that creates an instance of the C access structure to \p shared_ptr_to_object
     * @param arguments The arguments to be passed to the \p function
     * @throw Throws an exception of type \ref Exception if the \p function returns other than fep3_plugin_c_interface_error_none
     */
    template
        <typename object_type
        , typename handle_type
        , typename function_type
        , typename access_creator_type
        , typename... argument_types
        >
    static void transferSharedPtr
        (const std::shared_ptr<object_type>& shared_ptr_to_object
        , handle_type&& handle
        , function_type function
        , access_creator_type&& access_creator
        , argument_types&&... arguments
        )
    {
        auto reference_manager = new DestructionManager;
        // reference to the local object must be released when the remote object is destroyed, so we add a (new) shared reference to the reference manager
        reference_manager->addDestructor
            (std::make_unique<OtherDestructor<typename std::remove_reference<decltype(shared_ptr_to_object)>::type>>
            (new std::shared_ptr<object_type>(shared_ptr_to_object)));
        auto reference_manager_access = fep3_plugin_c_arya_SDestructionManager
            {reinterpret_cast<fep3_plugin_c_arya_HDestructionManager>(static_cast<DestructionManager*>(reference_manager))
            , wrapper::Destructor::destroy
            };
        auto pointer_to_object = shared_ptr_to_object.get();
        fep3_plugin_c_InterfaceError error = (*function)
            (handle
            , reference_manager_access
            , access_creator(pointer_to_object)
            , std::forward<argument_types>(arguments)...
            );
        if(fep3_plugin_c_interface_error_none != error)
        {
            throw Exception(error);
        }
    }

    /**
     * Calls the passed \p function and passes the \p handle, an access structure to the object to be transferred
     * , as well as the \p arguments to it.
     * This method can be used to wrap access to a remote object's method that takes a shared_ptr as parameter (implies shared ownership)
     * and returns non-void.
     * @pre The passed \p function must be implicitly convertible to:
     *      fep3_plugin_c_InterfaceError(*)(handle_type, return_type*, fep3_plugin_c_arya_SDestructionManager, access_type, argument_types...)
     *
     * @tparam return_type The type of the return value of this method
     * @tparam object_type The type of the object to be transferred
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam function_type The type of the function to be called
     * @tparam access_creator_type The type of the callable that creates an instance of the C access structure to \p unique_ptr_to_object
     * @tparam argument_types Parameter pack holding all parameters to be passed to \p function
     * @param shared_ptr_to_object Shared pointer to the object to be transferred
     * @param handle The handle of the object to call the corresponding member function on
     * @param function The function to be called
     * @param access_creator Callable that creates an instance of the C access structure to \p shared_ptr_to_object
     * @param arguments The arguments to be passed to the \p function
     * @throw Throws an exception of type \ref Exception if the \p function returns other than fep3_plugin_c_interface_error_none
     * @return The result of the function call, i. e. the content that was filled to the second parameter
     *          of \p function during the function call
     */
    template
        <typename return_type
        , typename object_type
        , typename handle_type
        , typename function_type
        , typename access_creator_type
        , typename... argument_types
        >
    static return_type transferSharedPtrWithResultParameter
        (const std::shared_ptr<object_type>& shared_ptr_to_object
        , handle_type&& handle
        , function_type function
        , access_creator_type&& access_creator
        , argument_types&&... arguments
        )
    {
        // the second parameter of function_type is the result pointer
        typename std::remove_pointer<typename FunctionParameters<function_type>::Parameter2Type>::type result{};
        auto pointer_to_object = shared_ptr_to_object.get();
        auto reference_manager = new DestructionManager;
        // reference to the local object must be released when the remote object is destroyed, so we add a (new) shared reference to the reference manager
        reference_manager->addDestructor
            (std::make_unique<OtherDestructor<typename std::remove_reference<decltype(shared_ptr_to_object)>::type>>
            (new std::shared_ptr<object_type>(shared_ptr_to_object)));
        auto reference_manager_access = fep3_plugin_c_arya_SDestructionManager
            {reinterpret_cast<fep3_plugin_c_arya_HDestructionManager>(static_cast<DestructionManager*>(reference_manager))
            , wrapper::Destructor::destroy
            };
        fep3_plugin_c_InterfaceError error = (*function)
            (handle
            , &result
            , reference_manager_access
            , access_creator(pointer_to_object)
            , std::forward<argument_types>(arguments)...
            );
        if(fep3_plugin_c_interface_error_none != error)
        {
            throw Exception(error);
        }
        return result;
    }

    /**
     * Calls the passed \p function and passes the \p handle, an access structure to the object to be transferred
     * , as well as the \p arguments to it.
     * This method can be used to wrap access to a remote object's method that takes a weak_ptr as parameter (implies shared ownership)
     * and returns void.
     * @pre The passed \p function must be implicitly convertible to:
     *      fep3_plugin_c_InterfaceError(*)(handle_type, fep3_plugin_c_arya_SDestructionManager, access_type, argument_types...)
     *
     * @tparam object_type The type of the object to be transferred
     * @tparam destructor_container_type The type of the destructor container
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam function_type The type of the function to be called
     * @tparam access_creator_type The type of the callable that creates an instance of the C access structure to \p unique_ptr_to_object
     * @tparam argument_types Parameter pack holding all parameters to be passed to \p function
     * @param weak_ptr_to_object Weak pointer to the object to be transferred
     * @param [in,out] destructor_container A container for destructors to control the remote object's lifetime
     * @param handle The handle of the object to call the corresponding member function on
     * @param function The function to be called
     * @param access_creator Callable that creates an instance of the C access structure to \p weak_ptr_to_object
     * @param arguments The arguments to be passed to the \p function
     * @throw Throws an exception of type \ref Exception if the \p function returns other than fep3_plugin_c_interface_error_none
     */
    template
        <typename object_type
        , typename destructor_container_type
        , typename handle_type
        , typename function_type
        , typename access_creator_type
        , typename... argument_types
        >
    static void transferWeakPtr
        (const std::weak_ptr<object_type>& weak_ptr_to_object
        , destructor_container_type& destructor_container
        , handle_type&& handle
        , function_type function
        , access_creator_type&& access_creator
        , argument_types&&... arguments
        )
    {
        auto pointer_to_object = weak_ptr_to_object.lock().get();
        fep3_plugin_c_arya_SDestructionManager destruction_manager_access{};
        fep3_plugin_c_InterfaceError error = (*function)
            (handle
            , &destruction_manager_access
            , access_creator(pointer_to_object)
            , std::forward<argument_types>(arguments)...
            );
        if(fep3_plugin_c_interface_error_none != error)
        {
            throw Exception(error);
        }
        // weak ownership: when the local weak_ptr has expires, we must destroy the remote shared pointer
        destructor_container.emplace_back(weak_ptr_to_object, destruction_manager_access);
    }

    /**
     * Calls the passed \p function and passes the \p handle, an access structure to the object to be transferred
     * , as well as the \p arguments to it.
     * This method can be used to wrap access to a remote object's method that takes a weak_ptr as parameter (implies shared ownership)
     * and returns void.
     * @pre The passed \p function must be implicitly convertible to:
     *      fep3_plugin_c_InterfaceError(*)(handle_type, fep3_plugin_c_arya_SDestructionManager, access_type, argument_types...)
     *
     * @tparam object_type The type of the object to be transferred
     * @tparam destructor_container_type The type of the destructor container
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam function_type The type of the function to be called
     * @tparam access_creator_type The type of the callable that creates an instance of the C access structure to \p unique_ptr_to_object
     * @tparam argument_types Parameter pack holding all parameters to be passed to \p function
     * @param weak_ptr_to_object Weak pointer to the object to be transferred
     * @param [in,out] destructor_container A container for destructors to control the remote object's lifetime
     * @param handle The handle of the object to call the corresponding member function on
     * @param function The function to be called
     * @param access_creator Callable that creates an instance of the C access structure to \p weak_ptr_to_object
     * @param arguments The arguments to be passed to the \p function
     * @throw Throws an exception of type \ref Exception if the \p function returns other than fep3_plugin_c_interface_error_none
     * @return The result of the function call, i. e. the content that was filled to the second parameter
     *         of \p function during the function call
     */
    template
        <typename return_type
        , typename object_type
        , typename destructor_container_type
        , typename handle_type
        , typename function_type
        , typename access_creator_type
        , typename... argument_types
        >
    static return_type transferWeakPtrWithResultParameter
        (const std::weak_ptr<object_type>& weak_ptr_to_object
        , destructor_container_type& destructor_container
        , handle_type&& handle
        , function_type function
        , access_creator_type&& access_creator
        , argument_types&&... arguments
        )
    {
        // the second parameter of function_type is the result pointer
        typename std::remove_pointer<typename FunctionParameters<function_type>::Parameter2Type>::type result{};
        auto pointer_to_object = weak_ptr_to_object.lock().get();
        fep3_plugin_c_arya_SDestructionManager destruction_manager_access{};
        fep3_plugin_c_InterfaceError error = (*function)
            (handle
            , &result
            , &destruction_manager_access
            , access_creator(pointer_to_object)
            , std::forward<argument_types>(arguments)...
            );
        if(fep3_plugin_c_interface_error_none != error)
        {
            throw Exception(error);
        }
        // weak ownership: when the local weak_ptr has expires, we must destroy the remote shared pointer
        destructor_container.emplace_back(weak_ptr_to_object, destruction_manager_access);
        return result;
    }

    /**
     * Calls the passed \p function and passes the \p handle, an access structure to the object to be passed
     * , as well as the \p arguments to it.
     * This method can be used to wrap access to a remote object's method that takes a reference to an object as parameter
     * and returns void (implies that ownership is not passed).
     * @pre The passed \p function must be implicitly convertible to:
     *      fep3_plugin_c_InterfaceError(*)(handle_type, result_type*, fep3_plugin_c_arya_SISharedBinary, access_type, argument_types...)
     *
     * @tparam return_type The type of the return value of this method
     * @tparam object_type The type of the object to be passed
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam function_type The type of the function to be called
     * @tparam access_creator_type The type of the callable that creates an instance of the C access structure to \p reference_to_object
     * @tparam argument_types Parameter pack holding all parameters to be passed to \p function
     * @param reference_to_object Reference to the object to be passed
     * @param handle The handle of the object to call the corresponding member function on
     * @param function The function to be called
     * @param access_creator Callable that creates an instance of the C access structure to \p reference_to_object
     * @param arguments The arguments to be passed to the \p function
     * @throw Throws an exception of type \ref Exception if the \p function returns other than fep3_plugin_c_interface_error_none
     * @return The result of the function call, i. e. the content that was filled to the second parameter
     *          of \p function during the function call
     */
    template<typename object_type, typename handle_type, typename function_type, typename access_creator_type, typename... argument_types>
    static void passReference
        (object_type& reference_to_object
        , handle_type&& handle
        , function_type function
        , access_creator_type&& access_creator
        , argument_types&&... arguments
        )
    {
        auto pointer_to_object = &reference_to_object;
        fep3_plugin_c_InterfaceError error = (*function)
            (handle
            , access_creator(pointer_to_object)
            , std::forward<argument_types>(arguments)...
            );
        if(fep3_plugin_c_interface_error_none != error)
        {
            throw Exception(error);
        }
    }

    /**
     * Calls the passed \p function and passes the \p handle, a pointer to a result object
     * , an access structure to the object to be passed as well as the \p arguments to it.
     * This method can be used to wrap access to a remote object's method that takes a reference to an object as parameter
     * and returns non-void (implies that ownership is not passed).
     * @pre The passed \p function must be implicitly convertible to:
     *      fep3_plugin_c_InterfaceError(*)(handle_type, result_type*, fep3_plugin_c_arya_SISharedBinary, access_type, argument_types...)
     *
     * @tparam return_type The type of the return value of this method
     * @tparam object_type The type of the object to be passed
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam function_type The type of the function to be called
     * @tparam access_creator_type The type of the callable that creates an instance of the C access structure to \p reference_to_object
     * @tparam argument_types Parameter pack holding all parameters to be passed to \p function
     * @param reference_to_object Reference to the object to be passed
     * @param handle The handle of the object to call the corresponding member function on
     * @param function The function to be called
     * @param access_creator Callable that creates an instance of the C access structure to \p reference_to_object
     * @param arguments The arguments to be passed to the \p function
     * @throw Throws an exception of type \ref Exception if the \p function returns other than fep3_plugin_c_interface_error_none
     * @return The result of the function call, i. e. the content that was filled to the second parameter
     *          of \p function during the function call
     */
    template<typename return_type, typename object_type, typename handle_type, typename function_type, typename access_creator_type, typename... argument_types>
    static return_type passReferenceWithResultParameter
        (object_type& reference_to_object
        , handle_type&& handle
        , function_type function
        , access_creator_type&& access_creator
        , argument_types&&... arguments
        )
    {
        // the second parameter of function_type is the result pointer
        typename std::remove_pointer<typename FunctionParameters<function_type>::Parameter2Type>::type result{};
        auto pointer_to_object = &reference_to_object;
        fep3_plugin_c_InterfaceError error = (*function)
            (handle
            , &result
            , access_creator(pointer_to_object)
            , std::forward<argument_types>(arguments)...
            );
        if(fep3_plugin_c_interface_error_none != error)
        {
            throw Exception(error);
        }
        return result;
    }
};

} // namespace arya
using arya::Helper;
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3
