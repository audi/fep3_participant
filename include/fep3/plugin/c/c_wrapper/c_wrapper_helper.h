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

#include <fep3/plugin/c/destructor_intf.h>
#include <fep3/plugin/c/c_intf/c_intf_errors.h>
#include <fep3/plugin/c/c_access/destructor_c_access.h>
#include <fep3/plugin/c/c_wrapper/destructor_c_wrapper.h>
#include <fep3/plugin/c/c_access/shared_binary_c_access.h>
#include <fep3/plugin/c/c_wrapper/shared_binary_c_wrapper.h>

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
 * Creates a new object using the passed \p factory by passing the \p arguments to the latter.
 * The pointer to the newly created object is used to create a C access structure via \p access_creator
 * which is then written to \p result.
 * @note The C access structure as written to \p result takes ownership of the object.
 * This method can be used to create an object and return a C access structure for it over a C interface.
 *
 * @tparam object_type The type of the object to be created
 * @tparam result_type The type of result to write the C access structure to
 * @tparam access_creator_type The type of the access creator to be used to create the C access structure
 * @tparam argument_types Parameter pack holding all parameters to be passed to the \p method
 * @param factory Callable factory capable to create an object
 * @param result Pointer to the result to write the C access structure to
 * @param shared_binary_access C access structure for the shared binary the caller resides in
 * @param access_creator The callable that creates the C access structure for the created object
 * @param arguments The arguments to be passed to the constructor of \p object_type
 * @return Interface error code
 * @retval fep3_plugin_c_interface_error_none No error occurred
 * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
 * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p access_result is null
 */
template<typename factory_type, typename result_type, typename access_creator_type, typename... argument_types>
fep3_plugin_c_InterfaceError create
    (factory_type&& factory
    , result_type* result
    , const fep3_plugin_c_arya_SISharedBinary& shared_binary_access
    , access_creator_type&& access_creator
    , argument_types&&... arguments
    )
{
    try
    {
        auto pointer_to_object = factory(std::forward<argument_types>(arguments)...);
        // set the shared binary to the object to prevent unloading of binary while object exists
        pointer_to_object->setSharedBinary(std::make_shared<access::SharedBinary>(shared_binary_access));

        if(nullptr != result)
        {
            *result = access_creator(pointer_to_object);
            return fep3_plugin_c_interface_error_none;
        }
        else
        {
            return fep3_plugin_c_interface_error_invalid_result_pointer;
        }
    }
    catch(...)
    {
        return fep3_plugin_c_interface_error_exception_caught;
    }
}

/**
 * Gets access to an object as identified by \p interface_handle.
 * This method can be used to get an object and return a C access structure for it over a C interface.
 *
 * @tparam interace_type The type of the interface of the object as identified by \p interface_handle
 * @tparam object_type The type of the object to get
 * @tparam result_type The type of result to write the C access structure to
 * @tparam access_creator_type The type of the access creator to be used to create the C access structure
 * @param access_result Pointer to the result to write the C access structure to
 * @param interface_handle Handle to the interface of the object to get
 * @param access_creator The callable that creates the C access structure for the object to get
 * @return Interface error code
 * @retval fep3_plugin_c_interface_error_none No error occurred
 * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
 * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p access_result is null
 */
template<typename interface_type, typename object_type, typename interface_handle_type, typename result_type, typename access_creator_type>
fep3_plugin_c_InterfaceError get
    (result_type* access_result
    , const interface_handle_type& interface_handle
    , access_creator_type&& access_creator
    )
{
    try
    {
        const auto& pointer_to_interface = reinterpret_cast<interface_type*>(interface_handle);
        const auto& pointer_to_object = dynamic_cast<object_type*>(pointer_to_interface);
        if(nullptr != access_result)
        {
            *access_result = access_creator(pointer_to_object);
            return fep3_plugin_c_interface_error_none;
        }
        else
        {
            return fep3_plugin_c_interface_error_invalid_result_pointer;
        }
    }
    catch(...)
    {
        return fep3_plugin_c_interface_error_exception_caught;
    }
}

///@cond nodoc
namespace detail
{

template<typename interface_type>
class Helper
{
public:
    /**
     * Transfers a remote shared pointer wrapped in a C \p access structure by creating a new
     * object of type \p access_object_type in a shared pointer, connecting it to the remote
     * shared pointer and passing it to the \p invoker.
     *
     * @tparam access_object_type The type of the access object to be created in a shared pointer
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam invoker_type The type of the invoker
     * @tparam access_type The type of the C access structure of the remote object
     * @tparam argument_types Parameter pack holding all parameters to be passed to the \p invoker
     * @param handle The handle of the object to be passed to the \p invoker
     * @param invoker Callable that invokes method on the created (local) object to transfer the shared pointer to
     * @param reference_manager_access Access structure to the manager releasing a reference to the remote object when the local object is destroyed
     * @param access C access structure reference
     * @param arguments The arguments to be passed to the \p function
     * @throw Throws if the \p invoker throws
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     */
    template<typename access_object_type, typename handle_type, typename invoker_type, typename access_type, typename... argument_types>
    static fep3_plugin_c_InterfaceError transferSharedPtr
        (handle_type&& handle
        , invoker_type&& invoker
        , const fep3_plugin_c_arya_SDestructionManager& reference_manager_access
        , const access_type& access
        , argument_types&&... arguments
        )
    {
        if(const auto& wrapped_this = reinterpret_cast<interface_type*>(handle))
        {
            std::shared_ptr<access_object_type> shared_ptr_to_object;
            // create the local object only if the (handle to the) remote object is valid
            if(nullptr != access._handle)
            {
                std::deque<std::unique_ptr<IDestructor>> destructors;
                // shared ownership: release reference to remote object when local object is destroyed
                destructors.push_back(std::make_unique<access::Destructor<fep3_plugin_c_arya_SDestructionManager>>(reference_manager_access));
                shared_ptr_to_object = std::make_shared<access_object_type>
                    (access
                    , std::move(destructors)
                    );
            }
            if(invoker(wrapped_this, std::move(shared_ptr_to_object), std::forward<argument_types>(arguments)...))
            {
                return fep3_plugin_c_interface_error_none;
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

    /**
     * Transfers a remote weak pointer wrapped in a C \p access structure by creating a new
     * shared pointer of type \p access_object_type, connecting it to the remote
     * shared pointer and passing it to the \p invoker.
     *
     * @tparam access_object_type The type of the access object to be created in a shared pointer
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam invoker_type The type of the invoker
     * @tparam access_type The type of the C access structure of the remote object
     * @tparam argument_types Parameter pack holding all parameters to be passed to the \p invoker
     * @param handle The handle of the object to be passed to the \p invoker
     * @param invoker Callable that invokes method on the created (local) object to transfer the shared pointer to
     * @param destruction_manager_access_result Pointer to the result of a destruction manager access for the remote object
     * @param access C access structure reference
     * @param arguments The arguments to be passed to the \p function
     * @throw Throws if the \p invoker throws
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     */
    template<typename access_object_type, typename handle_type, typename invoker_type, typename access_type, typename... argument_types>
    static fep3_plugin_c_InterfaceError transferWeakPtr
        (handle_type&& handle
        , invoker_type&& invoker
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_access_result
        , const access_type& access
        , argument_types&&... arguments
        )
    {
        if(const auto& wrapped_this = reinterpret_cast<interface_type*>(handle))
        {
            if(nullptr != destruction_manager_access_result)
            {
                std::shared_ptr<access_object_type> shared_ptr_to_object;
                // create the local object only if the (handle to the) remote object is valid
                if(nullptr != access._handle)
                {
                    auto pointer_to_shared_ptr_to_object = new std::shared_ptr<access_object_type>
                        (new access_object_type(access
                        , {} // weak ownership: nothing to be done, when the local object is destroyed
                        ));
                    shared_ptr_to_object = *pointer_to_shared_ptr_to_object;

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
                if(invoker(wrapped_this, std::move(shared_ptr_to_object), std::forward<argument_types>(arguments)...))
                {
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
};

} // namespace detail
///@endcond nodoc

/**
 * @brief Wrapper helper to transfer function calls on a C interface to the corresponding C++ object identified by a handle.
 * The (static) methods of this class provide facilities to write a wrapper class for a
 * C interface by one single statement per wrapper class method.
 * Example: For a C++ interface to be wrapped:
 * class IMyStuff
 * {
 * public:
 *     virtual ~IMyStuff() = default;
 *     virtual int32_t getValue() const = 0;
 * };
 * The access class looks like this when using the Helper class:
 * class WrapperOfIMyStuff
 *     : private fep3::plugin::c::wrapper::arya::Helper<IMyStuff>
 * {
 * private:
 *    using Handle = HIMyStuff;
 * public:
 *    // static methods transferring calls from the C interface to an object of IMyStuff
 *    static inline fep3_plugin_c_InterfaceError getValue(Handle handle, int32_t* result) noexcept
 *    {
 *        return callWithResultParameter
 *            (handle
 *            , &IMyStuff::get
 *            , [](int32_t result)
 *                {
 *                    return result;
 *                }
 *            , result
 *            );
 *    }
 * };
 */
template<typename interface_type>
class Helper
{
protected:
    /**
     * Calls the \p method on the object as referenced by \p handle and passes the \p arguments to it.
     * This method can be used to transfer a function call on a C interface to the corresponding method
     * of the wrapper object returning void.
     *
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam method_type The type of the method to be called
     * @tparam argument_types Parameter pack holding all parameters to be passed to the \p method
     * @param handle The handle of the object to call the method on
     * @param method Pointer to member method to be called
     * @param arguments The arguments to be passed to the \p method
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \p method
     */
    template<typename handle_type, typename method_type, typename... argument_types>
    static fep3_plugin_c_InterfaceError call
        (handle_type&& handle
        , method_type&& method
        , argument_types&&... arguments
        ) noexcept
    {
        try
        {
            if(const auto& pointer_to_object = reinterpret_cast<interface_type*>(handle))
            {
                (pointer_to_object->*method)(std::forward<argument_types>(arguments)...);
                return fep3_plugin_c_interface_error_none;
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
     * Calls the \p method on the object as referenced by \p handle and passes the \p arguments to it.
     * The return value of the call to \p method is converted by the \p converter and then assigned to the \p result.
     * This method can be used to transfer a function call on a C interface to the corresponding method
     * of the wrapper object returning non-void.
     *
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam method_type The type of the method to be called
     * @tparam converter_type The type of the result converter
     * @tparam result_type The type of the converted result of the method call
     * @tparam argument_types Parameter pack holding all parameters to be passed to the \p method
     * @param handle The handle of the object to call the method on
     * @param method Pointer to member method to be called
     * @param converter The result converter
     * @param result Pointer to the result
     * @param arguments The arguments to be passed to the \p method
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \p method
     */
    template<typename handle_type, typename method_type, typename converter_type, typename result_type, typename... argument_types>
    static fep3_plugin_c_InterfaceError callWithResultParameter
        (handle_type&& handle
        , method_type&& method
        , converter_type&& converter
        , result_type* result
        , argument_types&&... arguments
        ) noexcept
    {
        try
        {
            if(const auto& pointer_to_object = reinterpret_cast<interface_type*>(handle))
            {
                if(nullptr != result)
                {
                    *result = converter((pointer_to_object->*method)(std::forward<argument_types>(arguments)...));
                    return fep3_plugin_c_interface_error_none;
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
     * Calls the \p method on the object as referenced by \p handle and passes the \p arguments to it.
     * The return value of the call to \p method is converted by the \p converter and then passed to the \p callback.
     * This method can be used to transfer a function call on a C interface to the corresponding method
     * of the wrapper object returning non-void.
     *
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam method_type The type of the method to be called
     * @tparam callback_type The type of the callback to be called to return the result
     * @tparam converter_type The type of the result converter
     * @tparam result_type The type of the converted result of the method call
     * @tparam argument_types Parameter pack holding all parameters to be passed to the \p method
     * @param handle The handle of the object to call the method on
     * @param method Pointer to member method to be called
     * @param callback The callback to be called to return the result
     * @param destination Void pointer to the destination to be passed to the callback
     * @param converter The result converter
     * @param arguments The arguments to be passed to the \p method
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \p method
     */
    template<typename handle_type, typename method_type, typename callback_type, typename converter_type, typename... argument_types>
    static fep3_plugin_c_InterfaceError callWithResultCallback
        (handle_type&& handle
        , method_type&& method
        , callback_type&& callback
        , void* destination
        , converter_type&& converter
        , argument_types&&... arguments
        ) noexcept
    {
        try
        {
            if(const auto& pointer_to_object = reinterpret_cast<interface_type*>(handle))
            {
                const auto& result = (pointer_to_object->*method)(std::forward<argument_types>(arguments)...);
                callback(destination, converter(result));
                return fep3_plugin_c_interface_error_none;
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
     * Calls the \p method on the object as referenced by \p handle and passes the \p arguments to it.
     * The return value of the call to \p method is iterated over and the elements are converted
     * by the \p converter and then passed to the \p callback.
     * This method can be used to transfer a function call on a C interface to the corresponding method
     * of the wrapper object returning an iteratable container.
     *
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam method_type The type of the method to be called
     * @tparam callback_type The type of the callback to be called to return the result
     * @tparam converter_type The type of the result converter
     * @tparam result_type The type of the converted result of the method call
     * @tparam argument_types Parameter pack holding all parameters to be passed to the \p method
     * @param handle The handle of the object to call the method on
     * @param method Pointer to member method to be called
     * @param callback The callback to be called to return the result
     * @param destination Void pointer to the destination to be passed to the callback
     * @param converter The result converter
     * @param arguments The arguments to be passed to the \p method
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \p method
     */
    template<typename handle_type, typename method_type, typename callback_type, typename converter_type, typename... argument_types>
    static fep3_plugin_c_InterfaceError callWithRecurringResultCallback
        (handle_type&& handle
        , method_type&& method
        , callback_type&& callback
        , void* destination
        , converter_type&& converter
        , argument_types&&... arguments
        ) noexcept
    {
        try
        {
            if(const auto& pointer_to_object = reinterpret_cast<interface_type*>(handle))
            {
                const auto& iteratable_result = (pointer_to_object->*method)(std::forward<argument_types>(arguments)...);
                for(const auto& result : iteratable_result)
                {
                    callback(destination, converter(result));
                }
                return fep3_plugin_c_interface_error_none;
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
     * Calls the \p method on the object as referenced by \p handle and passes the \p arguments to it.
     * From the unique pointer, as returned by \p method, the pointer is released and the latter is used
     * to create a C access structure via \p access_creator which is then written to \p access_result.
     * This method can be used to transfer a function call on a C interface to the corresponding method
     * of the wrapper object returning a unique pointer.
     *
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam method_type The type of the method to be called
     * @tparam access_type The type of the C access structure to be filled
     * @tparam access_creator_type The type of the access creator to be used to create the C access structure
     * @tparam argument_types Parameter pack holding all parameters to be passed to the \p method
     * @param handle The handle of the object to call the method on
     * @param method Pointer to member method to be called
     * @param destruction_manager_access_result Pointer to the result of a destruction manager access for the remote object
     * @param access_result The result C access structure providing access to the object
     *         , as pointed to by the return value of the \p method
     * @param access_creator The callable that creates the C access structure for the object
     *                        , as pointed to by the return value of the \p method
     * @param arguments The arguments to be passed to the \p method
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \p method
     */
    template
        <typename handle_type
        , typename method_type
        , typename access_type
        , typename access_creator_type
        , typename... argument_types
        >
    static fep3_plugin_c_InterfaceError getUniquePtr
        (handle_type&& handle
        , method_type&& method
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_access_result
        , access_type* access_result
        , access_creator_type&& access_creator
        , argument_types&&... arguments
        ) noexcept
    {
        try
        {
            if(const auto& wrapped_this = reinterpret_cast<interface_type*>(handle))
            {
                // transfer ownership (std::unique_ptr), so release the object from the original unique pointer
                const auto& pointer_to_object = (wrapped_this->*method)(std::forward<argument_types>(arguments)...).release();
                if
                    ((nullptr != destruction_manager_access_result)
                    && (nullptr != access_result)
                    )
                {
                    auto destruction_manager = new DestructionManager;
                    // local object must be destroyed when the remote object is destroyed, so we transfer the ownership to the destruction manager
                    destruction_manager->addDestructor(std::make_unique<OtherDestructor<typename std::remove_pointer<typename std::remove_reference<decltype(pointer_to_object)>::type>::type>>
                        (pointer_to_object));
                    *destruction_manager_access_result = fep3_plugin_c_arya_SDestructionManager
                            {reinterpret_cast<fep3_plugin_c_arya_HDestructionManager>(static_cast<DestructionManager*>(destruction_manager))
                            , Destructor::destroy
                            };
                    *access_result = access_creator(pointer_to_object);
                    return fep3_plugin_c_interface_error_none;
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
     * Calls the \p method on the object as referenced by \p handle and passes the \p arguments to it.
     * From the unique pointer, as returned by \p method, the pointer is released and the latter is used
     * to create a C access structure via \p access_creator which is then written to \p access_result.
     * This method can be used to transfer a function call on a C interface to the corresponding method
     * of the wrapper object returning a unique pointer.
     *
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam method_type The type of the method to be called
     * @tparam access_type The type of the C access structure to be filled
     * @tparam access_creator_type The type of the access creator to be used to create the C access structure
     * @tparam argument_types Parameter pack holding all parameters to be passed to the \p method
     * @param handle The handle of the object to call the method on
     * @param method Pointer to member method to be called
     * @param destruction_manager_access_result Pointer to the result of a destruction manager access for the remote object
     * @param access_result The result C access structure providing access to the object
     *         , as pointed to by the return value of the \p method
     * @param access_creator The callable that creates the C access structure for the object
     *                        , as pointed to by the return value of the \p method
     * @param arguments The arguments to be passed to the \p method
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \p method
     */
    template
        <typename handle_type
        , typename method_type
        , typename access_type
        , typename access_creator_type
        , typename... argument_types
        >
    static fep3_plugin_c_InterfaceError getSharedPtr
        (handle_type&& handle
        , method_type&& method
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_access_result
        , access_type* access_result
        , access_creator_type&& access_creator
        , argument_types&&... arguments
        ) noexcept
    {
        try
        {
            if(const auto& wrapped_this = reinterpret_cast<interface_type*>(handle))
            {
                const auto& shared_ptr_to_object = (wrapped_this->*method)(std::forward<argument_types>(arguments)...);
                if
                    ((nullptr != destruction_manager_access_result)
                    && (nullptr != access_result)
                    )
                {
                    // transfer shared ownership (std::shared_ptr), so create a new (strong) reference to the local object that will be released when remote object is destroyed
                    auto local_shared_ptr_to_object
                        = new std::shared_ptr<typename std::remove_reference<decltype(shared_ptr_to_object)>::type::element_type>(shared_ptr_to_object);
                    auto destruction_manager = new DestructionManager;
                    // local object must be destroyed when the remote object is destroyed, so we transfer the ownership to the destruction manager
                    destruction_manager->addDestructor(std::make_unique<OtherDestructor<typename std::remove_pointer<typename std::remove_reference<decltype(local_shared_ptr_to_object)>::type>::type>>
                        (local_shared_ptr_to_object));
                    *destruction_manager_access_result = fep3_plugin_c_arya_SDestructionManager
                            {reinterpret_cast<fep3_plugin_c_arya_HDestructionManager>(static_cast<DestructionManager*>(destruction_manager))
                            , Destructor::destroy
                            };
                    *access_result = access_creator(shared_ptr_to_object.get());
                    return fep3_plugin_c_interface_error_none;
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
     * Creates a unique pointer of type \p access_object_type by using the passed C \p access structure
     * to the remote object and then calls the \p method on the object as referenced by \p handle and passes
     * the created access object to it.
     * This method can be used to transfer a function call on a C interface to the corresponding method
     * of the wrapper object which takes a unique pointer as parameter.
     *
     * @tparam access_object_type The type of the access object to be created
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam converter_type The type of the converter
     * @tparam method_type The type of the method to be called
     * @tparam result_type The type of the result of the \p method
     * @tparam access_type The type of the C access structure to be filled
     * @tparam argument_types Parameter pack holding all parameters to be passed to \p function
     * @param handle The handle of the object to call the method on
     * @param method Pointer to member method to be called
     * @param converter The result converter
     * @param result Pointer to set the result value
     * @param destruction_manager_access Access structure to the manager destroying the remote object when the local object is destroyed
     * @param access The C access structure for the remote object
     * @param arguments The arguments to be passed to the \p method
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \p method
     */
    template
        <typename access_object_type
        , typename handle_type
        , typename method_type
        , typename converter_type
        , typename result_type
        , typename access_type
        , typename... argument_types
        >
    static fep3_plugin_c_InterfaceError transferUniquePtrWithResultParameter
        (handle_type&& handle
        , method_type&& method
        , converter_type&& converter
        , result_type* result
        , const fep3_plugin_c_arya_SDestructionManager& destruction_manager_access
        , const access_type& access
        , argument_types&&... arguments
        ) noexcept
    {
        try
        {
            if(const auto& wrapped_this = reinterpret_cast<interface_type*>(handle))
            {
                std::unique_ptr<access_object_type> unique_ptr_to_object;
                // create the local object only if the (handle to the) remote object is valid
                if(nullptr != access._handle)
                {
                    std::deque<std::unique_ptr<IDestructor>> destructors;
                    // unique pointer: ownership transfer -> destroy remote object upon destruction of local object
                    destructors.push_back(std::make_unique<access::Destructor<fep3_plugin_c_arya_SDestructionManager>>(destruction_manager_access));
                    unique_ptr_to_object = std::make_unique<access_object_type>
                        (access
                        , std::move(destructors)
                        );
                }
                if(nullptr != result)
                {
                    *result = converter((wrapped_this->*method)(std::move(unique_ptr_to_object), std::forward<argument_types>(arguments)...));
                    return fep3_plugin_c_interface_error_none;
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
     * Creates a shared pointer of type \p access_object_type by using the passed C \p access structure
     * to the remote object and then calls the \p method on the object as referenced by \p handle and passes
     * the created access object to it.
     * This method can be used to transfer a function call on a C interface to the corresponding method
     * of the wrapper object which takes a shared pointer as parameter and returns void.
     *
     * @tparam access_object_type The type of the access object to be created
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam bound_method_type The type of the bound method to be called
     * @tparam access_type The type of the C access structure to be filled
     * @tparam argument_types Parameter pack holding all parameters to be passed to \p function
     * @param handle The handle of the object to call the method on
     * @param bound_method The bound method to be called
     * @param reference_manager_access Access structure to the manager releasing a reference to the remote object when the local object is destroyed
     * @param access The C access structure for the remote object
     * @param arguments Arguments of the methods to forward
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \p method
     */
    template
        <typename access_object_type
        , typename handle_type
        , typename bound_method_type
        , typename access_type
        , typename... argument_types
        >
    static fep3_plugin_c_InterfaceError transferSharedPtr
        (handle_type&& handle
        , bound_method_type&& bound_method
        , const fep3_plugin_c_arya_SDestructionManager& reference_manager_access
        , const access_type& access
        , argument_types&&... arguments
        ) noexcept
    {
        try
        {
            const auto& invoker = [bound_method]
                (interface_type* wrapped_this
                , const std::shared_ptr<access_object_type>& shared_ptr_to_object
                , argument_types&&... arguments
                )
                {
                    bound_method
                        (wrapped_this
                        , std::move(shared_ptr_to_object)
                        , std::forward<argument_types>(arguments)...
                        );
                    return true;
                };
            return detail::Helper<interface_type>::template transferSharedPtr<access_object_type>
                (handle
                , invoker
                , reference_manager_access
                , access
                , std::forward<argument_types>(arguments)...
                );
        }
        catch(...)
        {
            return fep3_plugin_c_interface_error_exception_caught;
        }
    }

    /**
     * Creates a shared pointer of type \p access_object_type by using the passed C \p access structure
     * to the remote object and then calls the \p method on the object as referenced by \p handle and passes
     * the created access object to it.
     * This method can be used to transfer a function call on a C interface to the corresponding method
     * of the wrapper object which takes a shared pointer as parameter and returns non-void.
     *
     * @tparam access_object_type The type of the access object to be created
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam converter_type The type of the converter
     * @tparam bound method_type The type of the bound method to be called
     * @tparam result_type The type of the result of the \p method
     * @tparam access_type The type of the C access structure to be filled
     * @tparam argument_types Parameter pack holding all parameters to be passed to \p function
     * @param handle The handle of the object to call the method on
     * @param bound_method The bound method to be called
     * @param converter The result converter
     * @param result Pointer to set the result value
     * @param reference_manager_access Access structure to the manager releasing a reference to the remote object when the local object is destroyed
     * @param access The C access structure for the remote object
     * @param arguments Arguments of the methods to forward
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \p method
     */
    template
        <typename access_object_type
        , typename handle_type
        , typename bound_method_type
        , typename converter_type
        , typename result_type
        , typename access_type
        , typename... argument_types
        >
    static fep3_plugin_c_InterfaceError transferSharedPtrWithResultParameter
        (handle_type&& handle
        , bound_method_type&& bound_method
        , converter_type&& converter
        , result_type* result
        , const fep3_plugin_c_arya_SDestructionManager& reference_manager_access
        , const access_type& access
        , argument_types&&... arguments
        ) noexcept
    {
        try
        {
            const auto& invoker = [result, bound_method, converter]
                (interface_type* wrapped_this
                , const std::shared_ptr<access_object_type>& shared_ptr_to_object
                , argument_types&&... arguments
                )
                {
                    if(nullptr != result)
                    {
                        *result = converter(bound_method
                            (wrapped_this
                            , std::move(shared_ptr_to_object)
                            , std::forward<argument_types>(arguments)...
                            ));
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                };
            return detail::Helper<interface_type>::template transferSharedPtr<access_object_type>
                (handle
                , invoker
                , reference_manager_access
                , access
                , std::forward<argument_types>(arguments)...
                );
        }
        catch(...)
        {
            return fep3_plugin_c_interface_error_exception_caught;
        }
    }

    /**
     * Creates a shared pointer of type \p access_object_type by using the passed C \p access structure
     * to the remote object and then calls the \p method on the object as referenced by \p handle and passes
     * the created access object to it.
     * This method can be used to transfer a function call on a C interface to the corresponding method
     * of the wrapper object which takes a shared pointer as parameter and returns void.
     *
     * @tparam access_object_type The type of the access object to be created
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam bound_method_type The type of the bound method to be called
     * @tparam access_type The type of the C access structure to be filled
     * @tparam argument_types Parameter pack holding all parameters to be passed to \p function
     * @param handle The handle of the object to call the method on
     * @param bound_method The bound method to be called
     * @param destruction_manager_access_result Pointer to the result of a destruction manager access for the remote object
     * @param access The C access structure for the remote object
     * @param arguments Arguments of the methods to forward
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \p method
     */
    template
        <typename access_object_type
        , typename handle_type
        , typename bound_method_type
        , typename access_type
        , typename... argument_types
        >
    static fep3_plugin_c_InterfaceError transferWeakPtr
        (handle_type&& handle
        , bound_method_type&& bound_method
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_access_result
        , const access_type& access
        , argument_types&&... arguments
        ) noexcept
    {
        try
        {
            const auto& invoker = [bound_method]
                (interface_type* wrapped_this
                , const std::shared_ptr<access_object_type>& shared_ptr_to_object
                , argument_types&&... arguments
                )
                {
                    bound_method
                        (wrapped_this
                        , std::move(shared_ptr_to_object)
                        , std::forward<argument_types>(arguments)...
                        );
                    return true;
                };
            return detail::Helper<interface_type>::template transferWeakPtr<access_object_type>
                (handle
                , invoker
                , destruction_manager_access_result
                , access
                , std::forward<argument_types>(arguments)...
                );
        }
        catch(...)
        {
            return fep3_plugin_c_interface_error_exception_caught;
        }
    }

    /**
     * Creates a shared pointer of type \p access_object_type by using the passed C \p access structure
     * to the remote object and then calls the \p method on the object as referenced by \p handle and passes
     * the created access object to it.
     * This method can be used to transfer a function call on a C interface to the corresponding method
     * of the wrapper object which takes a weak pointer as parameter and returns non-void.
     *
     * @tparam access_object_type The type of the access object to be created
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam converter_type The type of the converter
     * @tparam bound method_type The type of the bound method to be called
     * @tparam result_type The type of the result of the \p method
     * @tparam access_type The type of the C access structure to be filled
     * @tparam argument_types Parameter pack holding all parameters to be passed to \p function
     * @param handle The handle of the object to call the method on
     * @param bound_method The bound method to be called
     * @param converter The result converter
     * @param result Pointer to set the result value
     * @param destruction_manager_access_result Pointer to the result of a destruction manager access for the remote object
     * @param access The C access structure for the remote object
     * @param arguments Arguments of the methods to forward
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \p method
     */
    template
        <typename access_object_type
        , typename handle_type
        , typename bound_method_type
        , typename converter_type
        , typename result_type
        , typename access_type
        , typename... argument_types
        >
    static fep3_plugin_c_InterfaceError transferWeakPtrWithResultParameter
        (handle_type&& handle
        , bound_method_type&& bound_method
        , converter_type&& converter
        , result_type* result
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_access_result
        , const access_type& access
        , argument_types&&... arguments
        ) noexcept
    {
        try
        {
            const auto& invoker = [result, bound_method, converter]
                (interface_type* wrapped_this
                , const std::shared_ptr<access_object_type>& shared_ptr_to_object
                , argument_types&&... arguments
                )
                {
                    if(nullptr != result)
                    {
                        *result = converter(bound_method
                            (wrapped_this
                            , std::move(shared_ptr_to_object)
                            , std::forward<argument_types>(arguments)...
                            ));
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                };
            return detail::Helper<interface_type>::template transferWeakPtr<access_object_type>
                (handle
                , invoker
                , destruction_manager_access_result
                , access
                , std::forward<argument_types>(arguments)...
                );
        }
        catch(...)
        {
            return fep3_plugin_c_interface_error_exception_caught;
        }
    }

    /**
     * Creates an object of type \p access_object_type by using the passed C \p access structure
     * to the remote object and then calls the \p method on the object as referenced by \p handle and passes
     * the created access object to it.
     * This method can be used to transfer a function call on a C interface to the corresponding method
     * of the wrapper object which takes a reference to an object as parameter and returns void.
     * @note The access object is only valid for the time of the method call, thus the callee must not
     *       store a reference to it.
     *
     * @tparam access_object_type The type of the access object to be created
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam converter_type The type of the converter
     * @tparam method_type The type of the method to be called
     * @tparam access_type The type of the C access structure to be filled
     * @tparam argument_types Parameter pack holding all parameters to be passed to \p function
     * @param handle The handle of the object to call the method on
     * @param bound_method Pointer to the bound member method to be called
     * @param access The C access structure for the remote object
     * @param arguments Arguments of the methods to forward
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \p method
     */
    template
        <typename access_object_type
        , typename handle_type
        , typename bound_method_type
        , typename access_type
        , typename... argument_types
        >
    static fep3_plugin_c_InterfaceError passReference
        (handle_type&& handle
        , bound_method_type&& bound_method
        , const access_type& access
        , argument_types&&... arguments
        ) noexcept
    {
        try
        {
            if(const auto& wrapped_this = reinterpret_cast<interface_type*>(handle))
            {
                // note: no ownership transfer -> access object lives only for the time of the method call
                access_object_type access_object
                    (access
                    , {}
                    );
                bound_method(wrapped_this, access_object, std::forward<argument_types>(arguments)...);
                return fep3_plugin_c_interface_error_none;
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
     * Creates an object of type \p access_object_type by using the passed C \p access structure
     * to the remote object and then calls the \p method on the object as referenced by \p handle and passes
     * the created access object to it.
     * This method can be used to transfer a function call on a C interface to the corresponding method
     * of the wrapper object which takes a reference to an object as parameter and returns non-void.
     * @note The access object is only valid for the time of the method call, thus the callee must not
     *       store a reference to it.
     *
     * @tparam access_object_type The type of the access object to be created
     * @tparam handle_type The type of the handle to be passed to the \p function
     * @tparam converter_type The type of the converter
     * @tparam method_type The type of the method to be called
     * @tparam result_type The type of the result of the \p method
     * @tparam access_type The type of the C access structure to be filled
     * @tparam argument_types Parameter pack holding all parameters to be passed to \p function
     * @param handle The handle of the object to call the method on
     * @param bound_method Pointer to the bound member method to be called
     * @param converter The result converter
     * @param result Pointer to set the result value
     * @param access The C access structure for the remote object
     * @param arguments arguments of the methods to forward
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \p method
     */
    template
        <typename access_object_type
        , typename handle_type
        , typename bound_method_type
        , typename converter_type
        , typename result_type
        , typename access_type
        , typename... argument_types
        >
    static fep3_plugin_c_InterfaceError passReferenceWithResultParameter
        (handle_type&& handle
        , bound_method_type&& bound_method
        , converter_type&& converter
        , result_type* result
        , const access_type& access
        , argument_types&&... arguments
        ) noexcept
    {
        try
        {
            if(const auto& wrapped_this = reinterpret_cast<interface_type*>(handle))
            {
                // note: no ownership transfer -> access object lives only for the time of the method call
                access_object_type access_object
                    (access
                    , {}
                    );
                if(nullptr != result)
                {
                    *result = converter(bound_method(wrapped_this, access_object, std::forward<argument_types>(arguments)...));
                    return fep3_plugin_c_interface_error_none;
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
};

} // namespace arya
using arya::create;
using arya::Helper;
} // namespace wrapper
} // namespace c
} // namespace plugin
} // namespace fep3
