/**
 * @file
 * Copyright &copy; AUDI AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * @note All methods are defined inline to provide the functionality as header only.
 */

#pragma once

#include <fep3/base/sample/c_intf/data_sample_c_intf.h>
#include <fep3/base/sample/data_sample_intf.h>
#include <fep3/plugin/c/c_access/c_access_helper.h>
#include <fep3/plugin/c/c_wrapper/c_wrapper_helper.h>

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
 * Class wrapping access to the C interface for @ref fep3::arya::IRawMemory.
 * Use this class to access a remote object of a type derived from @ref fep3::arya::IRawMemory that resides in another binary (e. g. a shared library).
 */
class RawMemory
    : public IRawMemory
    , private DestructionManager
    , private Helper
{
public:
    /// Type of access structure
    using Access = fep3_arya_SIRawMemory;

    /**
     * @brief CTOR
     * @param access Access to the remote object
     * @param destructors List of destructors to be called upon destruction of this
     */
    inline RawMemory
        (const Access& access
        , std::deque<std::unique_ptr<IDestructor>> destructors
        );
    inline ~RawMemory() override = default;

    // methods implementing fep3::arya::IRawMemory
    /**
     * Calls @ref fep3::arya::IRawMemory::capacity(...) on the remote object
     * @return The capacity as returned by call of @ref fep3::arya::IRawMemory::capacity on the remote object
     */
    inline size_t capacity() const override;
    /**
     * Calls @ref fep3::arya::IRawMemory::cdata(...) on the remote object
     * @return The pointer to the contained data as returned by call of @ref fep3::arya::IRawMemory::cdata on the remote object
     */
    inline const void* cdata() const override;
    /**
     * Calls @ref fep3::arya::IRawMemory::size(...) on the remote object
     * @return The capacity as returned by call of @ref fep3::arya::IRawMemory::size on the remote object
     */
    inline size_t size() const override;
    /**
     * Calls @ref fep3::arya::IRawMemory::set(...) on the remote object
     * @param data Pointer to the data to be set
     * @param data_size The size of the data to be set
     * @return The size as returned by call of @ref fep3::arya::IRawMemory::set on the remote object
     */
    inline size_t set(const void* data, size_t data_size) override;
    /**
     * Calls @ref fep3::arya::IRawMemory::resize(...) on the remote object
     * @param data_size The size of the data to be set
     * @return The size as returned by call of @ref fep3::arya::IRawMemory::resize on the remote object
     */
    inline size_t resize(size_t data_size) override;

private:
    Access _access;
};

} // namespace arya
} // namespace access

namespace wrapper
{
namespace arya
{

/**
 * Wrapper class for interface \ref fep3::arya::IRawMemory
 */
class RawMemory : private Helper<fep3::arya::IRawMemory>
{
public:
    /// Alias for the helper
    using Helper = Helper<fep3::arya::IRawMemory>;
    /// Alias for the type of the handle to a wrapped object of type \ref fep3::arya::IRawMemory
    using Handle = fep3_arya_HIRawMemory;

    /**
     * Calls @ref fep3::arya::IRawMemory::capacity(...) on the object identified by \p handle
     * @param handle The handle to the object to call @ref fep3::arya::IRawMemory::capacity on
     * @param result Pointer to the result of the call of @ref fep3::arya::IRawMemory::capacity
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IRawMemory::capacity
     */
    static inline fep3_plugin_c_InterfaceError capacity(Handle handle, size_t* result) noexcept
    {
        return Helper::callWithResultParameter
            (handle
            , &fep3::arya::IRawMemory::capacity
            , [](size_t capacity)
                {
                    return capacity;
                }
            , result
            );
    }
    /**
     * Calls @ref fep3::arya::IRawMemory::cdata(...) on the object identified by \p handle
     * @param handle The handle to the object to call @ref fep3::arya::IRawMemory::cdata on
     * @param result Pointer to the result of the call of @ref fep3::arya::IRawMemory::cdata
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IRawMemory::cdata
     */
    static inline fep3_plugin_c_InterfaceError cdata(Handle handle, const void** result) noexcept
    {
        return Helper::callWithResultParameter
            (handle
            , &fep3::arya::IRawMemory::cdata
            , [](const void* cdata)
                {
                    return cdata;
                }
            , result
            );
    }
    /**
     * Calls @ref fep3::arya::IRawMemory::size(...) on the object identified by \p handle
     * @param handle The handle to the object to call @ref fep3::arya::IRawMemory::size on
     * @param result Pointer to the result of the call of @ref fep3::arya::IRawMemory::size
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IRawMemory::size
     */
    static inline fep3_plugin_c_InterfaceError size(Handle handle, size_t* result) noexcept
    {
        return Helper::callWithResultParameter
            (handle
            , &fep3::arya::IRawMemory::size
            , [](size_t size)
                {
                    return size;
                }
            , result
            );
    }
    /**
     * Calls \ref fep3::arya::IRawMemory::set(...) on the object identified by \p handle
     * @param handle The handle to the object to call \ref fep3::arya::IRawMemory::set on
     * @param result Pointer to the result of the call of \ref fep3::arya::IRawMemory::set
     * @param data Pointer to the raw data to be set
     * @param data_size Size of the data to be set
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IRawMemory::set
     */
    static inline fep3_plugin_c_InterfaceError set
        (Handle handle
        , size_t* result
        , const void* data
        , size_t data_size
        ) noexcept
    {
        return Helper::callWithResultParameter
            (handle
            , &fep3::arya::IRawMemory::set
            , [](const size_t& size)
                {
                    return size;
                }
            , result
            , data
            , data_size
            );
    }
    /**
     * Calls \ref fep3::arya::IRawMemory::resize(...) on the object identified by \p handle
     * @param handle The handle to the object to call \ref fep3::arya::IRawMemory::resize on
     * @param result Pointer to the result of the call of \ref fep3::arya::IRawMemory::resize
     * @param data_size The new size to be passed
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IRawMemory::resize
     */
    static inline fep3_plugin_c_InterfaceError resize(Handle handle, size_t* result, size_t data_size) noexcept
    {
        return Helper::callWithResultParameter
            (handle
            , &fep3::arya::IRawMemory::resize
            , [](const size_t& size)
                {
                    return size;
                }
            , result
            , data_size
            );
    }
};

} // namespace arya
} // namespace wrapper

namespace access
{
namespace arya
{

RawMemory::RawMemory
    (const Access& access
    , std::deque<std::unique_ptr<IDestructor>> destructors
    )
    : _access(access)
{
    addDestructors(std::move(destructors));
}

size_t RawMemory::capacity() const
{
    return callWithResultParameter(_access._handle, _access.capacity);
}

const void* RawMemory::cdata() const
{
    return callWithResultParameter(_access._handle, _access.cdata);
}

size_t RawMemory::size() const
{
    return callWithResultParameter(_access._handle, _access.size);
}

size_t RawMemory::set(const void* data, size_t data_size)
{
    return callWithResultParameter(_access._handle, _access.set, data, data_size);
}

size_t RawMemory::resize(size_t data_size)
{
    return callWithResultParameter(_access._handle, _access.resize, data_size);
}

} // namespace arya
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3
