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
#include <fep3/base/properties/c_access_wrapper/properties_c_access_wrapper.h>
#include <fep3/base/streamtype/c_intf/stream_type_c_intf.h>
#include <fep3/base/properties/properties_intf.h>
#include <fep3/base/streamtype/streamtype_intf.h>
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
 * Class wrapping access to the C interface for @ref fep3::arya::IStreamType.
 * Use this class to access a remote object of a type derived from @ref fep3::arya::IStreamType that resides in another binary (e. g. a shared library).
 */
class StreamType
    : public Properties
    , public IStreamType
{
public:
    /// Type of access structure
    using Access = fep3_arya_SIStreamType;

    /**
     * @brief CTOR
     * @param access Access to the remote object
     * @param destructors List of destructors to be called upon destruction of this
     */
    inline StreamType
        (const Access& access
        , std::deque<std::unique_ptr<IDestructor>> destructors
        );
    inline ~StreamType() override = default;

    // methods implementing fep3::arya::IStreamType
    inline std::string getMetaTypeName() const override;

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
 * Wrapper class for interface \ref fep3::arya::IStreamType
 */
class StreamType : private Helper<fep3::arya::IStreamType>
{
public:
    /**
     * Functor creating an access structure for @ref ::fep3::arya::IStreamType
     */
    struct AccessCreator
    {
        /**
         * Creates an access structure to the stream type as pointed to by @p pointer_to_stream_type
         *
         * @param pointer_to_stream_type Pointer to the stream type to create an access structure for
         * @return Access structure to the stream type
         */
        fep3_arya_SIStreamType operator()(fep3::arya::IStreamType* pointer_to_stream_type)
        {
            return fep3_arya_SIStreamType
                {reinterpret_cast<fep3_arya_HIStreamType>(pointer_to_stream_type)
                , fep3_arya_SIProperties
                    {reinterpret_cast<fep3_arya_HIProperties>(static_cast<fep3::arya::IProperties*>(pointer_to_stream_type))
                    , ::fep3::plugin::c::wrapper::arya::Properties::setProperty
                    , ::fep3::plugin::c::wrapper::arya::Properties::getProperty
                    , ::fep3::plugin::c::wrapper::arya::Properties::getPropertyType
                    , ::fep3::plugin::c::wrapper::arya::Properties::isEqual
                    , ::fep3::plugin::c::wrapper::arya::Properties::copy_to
                    , ::fep3::plugin::c::wrapper::arya::Properties::getPropertyNames
                    }
                , StreamType::getMetaTypeName
                };
        }
    };

    /// Alias for the helper
    using Helper = Helper<fep3::arya::IStreamType>;
    /// Alias for the type of the handle to a wrapped object of type \ref fep3::arya::IStreamType
    using Handle = fep3_arya_HIStreamType;

    /**
     * Calls @ref fep3::arya::IStreamType::getMetaTypeName(...) on the object identified by \p handle
     * @param handle The handle to the object to call @ref fep3::arya::IStreamType::getMetaTypeName on
     * @param callback Pointer to callback function to be called with the meta type name string as
     *                 returned by the call to @ref fep3::arya::IStreamType::getMetaTypeName on the stream type object
     * @param destination Pointer to the destination to be passed to the callback
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within @ref fep3::arya::IStreamType::getMetaTypeName

     */
    static inline fep3_plugin_c_InterfaceError getMetaTypeName(Handle handle, void(*callback)(void*, const char*), void* destination) noexcept
    {
        return Helper::callWithResultCallback
            (handle
            , &fep3::arya::IStreamType::getMetaTypeName
            , callback
            , destination
            , [](const std::string& meta_type_name)
                {
                    return meta_type_name.c_str();
                }
            );
    }
};

} // namespace arya
} // namespace wrapper

namespace access
{
namespace arya
{

StreamType::StreamType
    (const Access& access
    , std::deque<std::unique_ptr<IDestructor>> destructors
    )
    : Properties(access._properties, std::move(destructors))
    , _access(access)
{}

std::string StreamType::getMetaTypeName() const
{
    return callWithResultCallback<std::string>(_access._handle, _access.getMetaTypeName);
}

bool StreamType::setProperty(const std::string& name, const std::string& value, const std::string& type)
{
    return Properties::setProperty(name, value, type);
}

std::string StreamType::getProperty(const std::string& name) const
{
    return Properties::getProperty(name);
}

std::string StreamType::getPropertyType(const std::string& name) const
{
    return Properties::getPropertyType(name);
}

bool StreamType::isEqual(const fep3::arya::IProperties& properties) const
{
    return Properties::isEqual(properties);
}

void StreamType::copy_to(fep3::arya::IProperties& properties) const
{
    return Properties::copy_to(properties);
}

std::vector<std::string> StreamType::getPropertyNames() const
{
    return Properties::getPropertyNames();
}

} // namespace arya
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3
