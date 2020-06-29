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

#include <fep3/base/properties/c_intf/properties_c_intf.h>
#include <fep3/base/properties/properties_intf.h>
#include <fep3/plugin/c/c_access/c_access_helper.h>
#include <fep3/plugin/c/c_wrapper/c_wrapper_helper.h>
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
 * Class wrapping access to the C interface for @ref fep3::arya::IProperties.
 * Use this class to access a remote object of a type derived from @ref fep3::arya::IProperties that resides in another binary (e. g. a shared library).
 */
class Properties
    : public fep3::arya::IProperties
    , private DestructionManager
    , protected Helper
{
public:
    /// Type of access structure
    using Access = fep3_arya_SIProperties;

    /**
     * @brief CTOR
     * @param access Access to the remote object
     * @param destructors List of destructors to be called upon destruction of this
     */
    inline Properties
        (const Access& access
        , std::deque<std::unique_ptr<IDestructor>> destructors
        );
    inline ~Properties() override = default;

    // methods implementing fep3::arya::IProperties
    inline bool setProperty(const std::string& name, const std::string& value, const std::string& type) override;
    inline std::string getProperty(const std::string& name) const override;
    inline std::string getPropertyType(const std::string& name) const override;
    inline bool isEqual(const fep3::arya::IProperties& properties) const override;
    inline void copy_to(fep3::arya::IProperties& properties) const override;
    inline std::vector<std::string> getPropertyNames() const override;

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
 * Wrapper class for interface \ref fep3::arya::IProperties
 */
class Properties : private Helper<fep3::arya::IProperties>
{
private:
    using Helper = Helper<fep3::arya::IProperties>;
    using Handle = fep3_arya_HIProperties;

public:
    /**
     * Calls \ref fep3::arya::IProperties::setProperty on the object identified by \p handle
     * @param handle The handle to the properties object to call \ref fep3::arya::IProperties::setProperty on
     * @param result Pointer to the result of the call of @ref fep3::arya::IProperties::setProperty on the property object
     * @param name The name of the property to set
     * @param value The value of the property to set
     * @param type The type of the property to set
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IProperties::setProperty
     */
    static inline fep3_plugin_c_InterfaceError setProperty
        (Handle handle
        , bool* result
        , const char* name
        , const char* value
        , const char* type
        ) noexcept
    {
        return Helper::callWithResultParameter
            (handle
            , &fep3::arya::IProperties::setProperty
            , [](bool result)
                {
                    return result;
                }
            , result
            , name
            , value
            , type
            );
    }
    /**
     * Calls \ref fep3::arya::IProperties::getProperty(...) on the object identified by \p handle
     * @param handle The handle to the properties object to call \ref fep3::arya::IProperties::getProperty on
     * @param callback Pointer to callback function to be called with the property value string as
     *                 returned by the call to \ref fep3::arya::IProperties::getProperty on the property object
     * @param destination Pointer to the destination to be passed to the callback
     * @param name The name of the property to get the value of
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IProperties::getProperty
     */
    static inline fep3_plugin_c_InterfaceError getProperty
        (Handle handle
        , void(*callback)(void*, const char*)
        , void* destination
        , const char* name
        ) noexcept
    {
        return Helper::callWithResultCallback
            (handle
            , &fep3::arya::IProperties::getProperty
            , callback
            , destination
            , [](const std::string& name)
                {
                    return name.c_str();
                }
            , name
            );
    }
    /**
     * Calls \ref fep3::arya::IProperties::getPropertyType(...) on the object identified by \p handle
     * @param handle The handle to the properties object to call \ref fep3::arya::IProperties::getPropertyType on
     * @param callback Pointer to callback function to be called with the property value string as
     *                 returned by the call to \ref fep3::arya::IProperties::getPropertyType on the property object
     * @param destination Pointer to the destination to be passed to the callback
     * @param name The name of the property to get the type of
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IProperties::getPropertyType
     */
    static inline fep3_plugin_c_InterfaceError getPropertyType
        (Handle handle
        , void(*callback)(void*, const char*)
        , void* destination
        , const char* name
        ) noexcept
    {
        return Helper::callWithResultCallback
            (handle
            , &fep3::arya::IProperties::getPropertyType
            , callback
            , destination
            , [](const std::string& type)
                {
                    return type.c_str();
                }
            , name
            );
    }
    /**
     * Calls \ref fep3::arya::IProperties::isEqual(...) on the object identified by \p handle
     * @param handle The handle to the properties object to call \ref fep3::arya::IProperties::isEqual on
     * @param result Pointer to the result of the call of @ref fep3::arya::IProperties::isEqual on the property object
     * @param properties_access The access structure to the properties to be checked for equality
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IProperties::isEqual
     */
    static inline fep3_plugin_c_InterfaceError isEqual
        (Handle handle
        , bool* result
        , fep3_arya_SIProperties properties_access
        ) noexcept
    {
        return Helper::callWithResultParameter
            (handle
            , &fep3::arya::IProperties::isEqual
            , [](bool result)
                {
                    return result;
                }
            , result
            , access::arya::Properties(properties_access, {})
            );
    }
    /**
     * Calls \ref fep3::arya::IProperties::getPropertyNames(...) on the object identified by \p handle
     * @param handle The handle to the properties object to call \ref fep3::arya::IProperties::getPropertyNames on
     * @param callback Pointer to callback function to be called with the property name as
     *                 returned by the call to \ref fep3::arya::IProperties::getPropertyNames on the property object
     * @param destination Pointer to the destination to be passed to the callback
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IProperties::getPropertyNames
     */
    static inline fep3_plugin_c_InterfaceError getPropertyNames
        (fep3_arya_HIProperties handle, void(*callback)(void*, const char*), void* destination) noexcept
    {
        return Helper::callWithRecurringResultCallback
            (handle
            , &fep3::arya::IProperties::getPropertyNames
            , callback
            , destination
            , [](const std::string& name)
                {
                    return name.c_str();
                }
            );
    }
    /**
     * Calls \ref fep3::arya::IProperties::copy_to(...) on the object identified by \p handle
     * @param handle The handle to the properties object to call \ref fep3::arya::IProperties::copy_to on
     * @param properties_access The access structure to the properties to be checked for equality
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IProperties::copy_to
     */
    static inline fep3_plugin_c_InterfaceError copy_to
        (Handle handle
        , fep3_arya_SIProperties properties_access
        ) noexcept
    {
        access::arya::Properties properties(properties_access, {});
        return Helper::call
            (handle
            , &fep3::arya::IProperties::copy_to
            , properties
            );
    }
};

} // namespace arya
} // namespace wrapper

namespace access
{
namespace arya
{

Properties::Properties
    (const Access& access
    , std::deque<std::unique_ptr<IDestructor>> destructors
    )
    : _access(access)
{
    addDestructors(std::move(destructors));
}

bool Properties::setProperty(const std::string& name, const std::string& value, const std::string& type)
{
    return callWithResultParameter
        (_access._handle
        , _access.setProperty
        , name.c_str()
        , value.c_str()
        , type.c_str()
        );
}

std::string Properties::getProperty(const std::string& name) const
{
    return callWithResultCallback<std::string>(_access._handle, _access.getProperty, name.c_str());
}

std::string Properties::getPropertyType(const std::string& name) const
{
    return callWithResultCallback<std::string>(_access._handle, _access.getPropertyType, name.c_str());
}

bool Properties::isEqual(const fep3::arya::IProperties& properties) const
{
    return callWithResultParameter
        (_access._handle
        , _access.isEqual
        , fep3_arya_SIProperties
            {reinterpret_cast<fep3_arya_HIProperties>(const_cast<fep3::arya::IProperties*>(&properties))
            , wrapper::arya::Properties::setProperty
            , wrapper::arya::Properties::getProperty
            , wrapper::arya::Properties::getPropertyType
            , wrapper::arya::Properties::isEqual
            , wrapper::arya::Properties::copy_to
            , wrapper::arya::Properties::getPropertyNames
            }
        );
}

void Properties::copy_to(fep3::arya::IProperties& properties) const
{
    return call
        (_access._handle
        , _access.copy_to
        , fep3_arya_SIProperties
            {reinterpret_cast<fep3_arya_HIProperties>(const_cast<fep3::arya::IProperties*>(&properties))
            , wrapper::arya::Properties::setProperty
            , wrapper::arya::Properties::getProperty
            , wrapper::arya::Properties::getPropertyType
            , wrapper::arya::Properties::isEqual
            , wrapper::arya::Properties::copy_to
            , wrapper::arya::Properties::getPropertyNames
            }
        );
}

std::vector<std::string> Properties::getPropertyNames() const
{
    return callWithRecurringResultCallback<std::vector<std::string>, const char*>
        (_access._handle
        , _access.getPropertyNames
        , [](const char* name)
            {
                return std::string(name);
            }
        , &std::vector<std::string>::push_back
        );
}

} // namespace arya
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3
