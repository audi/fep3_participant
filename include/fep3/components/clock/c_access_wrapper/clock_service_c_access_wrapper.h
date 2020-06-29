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

#include <cstring> // for strcmp
#include <functional>

#include <fep3/fep3_macros.h>
#include <fep3/components/clock/c_intf/clock_service_c_intf.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/components/base/c_access_wrapper/component_base_c_access.h>
#include <fep3/components/base/c_access_wrapper/component_c_wrapper.h>
#include "clock_c_access_wrapper.h"

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
 * @brief Access class for @ref fep3::arya::IClockService.
 * Use this class to access a remote object of a type derived from IClockService that resides in another binary (e. g. a shared library).
 */
class ClockService
    : public ::fep3::plugin::c::access::arya::ComponentBase<fep3::arya::IClockService>
{
public:
    /// Symbol name of the create function that is capable to create a clock service
    static constexpr const char* const create_function_name = FEP3_EXPAND_TO_STRING(SYMBOL_fep3_plugin_c_arya_createClockService);
    /// Gets the function to get an instance of a clock service that resides in a C plugin
    static decltype(&fep3_plugin_c_arya_getClockService) getGetterFunction()
    {
         return fep3_plugin_c_arya_getClockService;
    }
    /// Type of access object
    using Access = fep3_arya_SIClockService;

    /**
     * CTOR
     *
     * @param access Access to the remote object
     * @param shared_binary Shared pointer to the binary this resides in
     */
    inline ClockService
        (const Access& access
        , const std::shared_ptr<ISharedBinary>& shared_binary
        );
    /**
     * DTOR destroying the corresponding remote object
     */
    inline ~ClockService() override = default;

    // methods implementing fep3::arya::IClockService
    /// @cond no_documentation
    inline fep3::arya::Timestamp getTime() const override;
    inline Optional<fep3::arya::Timestamp> getTime(const std::string& clock_name) const override;
    inline IClock::ClockType getType() const override;
    inline Optional<IClock::ClockType> getType(const std::string& clock_name) const override;
    inline std::string getMainClockName() const override;
    inline fep3::Result registerEventSink(const std::weak_ptr<::fep3::arya::IClock::IEventSink>& clock_event_sink) override;
    inline fep3::Result unregisterEventSink(const std::weak_ptr<::fep3::arya::IClock::IEventSink>& clock_event_sink) override;
    // methods implementing fep3::arya::ISchedulerRegistry
    inline fep3::Result registerClock(const std::shared_ptr<fep3::arya::IClock>& clock) override;
    inline fep3::Result unregisterClock(const std::string& clock_name) override;
    inline std::list<std::string> getClockNames() const override;
    inline std::shared_ptr<fep3::arya::IClock> findClock(const std::string& clock_name) const override;
    /// @endcond no_documentation

private:
    Access _access;
    std::deque
        <std::pair<std::weak_ptr<::fep3::arya::IClock::IEventSink>
        , ::fep3::plugin::c::access::arya::Destructor<fep3_plugin_c_arya_SDestructionManager>>
        > _remote_event_sink_object_destructors;
};

} // namespace arya
} // namespace access

namespace wrapper
{
namespace arya
{

/**
 * Wrapper class for interface @ref fep3::arya::IClockService
 */
class ClockService : private Helper<fep3::arya::IClockService>
{
private:
    using Helper = Helper<fep3::arya::IClockService>;
    using Handle = fep3_arya_HIClockService;

public:
    /// @cond no_documentation
    static inline fep3_plugin_c_InterfaceError getTime
        (Handle handle
        , int64_t* result
        )
    {
        return Helper::callWithResultParameter
            (handle
            , static_cast<::fep3::arya::Timestamp(fep3::arya::IClockService::*)()const>(&fep3::arya::IClockService::getTime)
            , [](fep3::arya::Timestamp timestamp)
                {
                    return timestamp.count();
                }
            , result
            );
    }
    static inline fep3_plugin_c_InterfaceError getTimeByClockName
        (Handle handle
        , int64_t* result
        , bool* validity
        , const char* clock_name
        )
    {
        try
        {
            if(const auto& pointer_to_object = reinterpret_cast<fep3::arya::IClockService*>(handle))
            {
                if((nullptr != validity) && (nullptr != result))
                {
                    const Optional<fep3::arya::Timestamp>& optional_timestamp = pointer_to_object->getTime(clock_name);
                    *validity = optional_timestamp.operator bool();
                    if(*validity)
                    {
                        *result = optional_timestamp.value().count();
                    }
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
    static inline fep3_plugin_c_InterfaceError getType
        (Handle handle
        , int32_t* result
        )
    {
        return Helper::callWithResultParameter
            (handle
            , static_cast<::fep3::arya::IClock::ClockType(fep3::arya::IClockService::*)()const>(&fep3::arya::IClockService::getType)
            , [](IClock::ClockType clock_type)
                {
                    return static_cast<int32_t>(clock_type);
                }
            , result
            );
    }
    static inline fep3_plugin_c_InterfaceError getTypeByClockName
        (Handle handle
        , int32_t* result
        , bool* validity
        , const char* clock_name
        )
    {
        try
        {
            if(const auto& pointer_to_object = reinterpret_cast<fep3::arya::IClockService*>(handle))
            {
                if((nullptr != validity) && (nullptr != result))
                {
                    const Optional<fep3::arya::IClock::ClockType>& optional_clock_type = pointer_to_object->getType(clock_name);
                    *validity = optional_clock_type.operator bool();
                    if(*validity)
                    {
                        *result = static_cast<int32_t>(optional_clock_type.value());
                    }
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
    static inline fep3_plugin_c_InterfaceError getMainClockName
        (Handle handle
        , void(*callback)(void*, const char*)
        , void* destination
        )
    {
        return Helper::callWithResultCallback
            (handle
            , &fep3::arya::IClockService::getMainClockName
            , callback
            , destination
            , [](const std::string& type)
                {
                    return type.c_str();
                }
            );
    }
    static inline fep3_plugin_c_InterfaceError registerEventSink
        (Handle handle
        , int32_t* result
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_access_result
        , fep3_arya_IClock_SIEventSink event_sink_access
        ) noexcept
    {
        return Helper::transferWeakPtrWithResultParameter<::fep3::plugin::c::access::arya::Clock::EventSink>
            (handle
            , std::bind
                (&fep3::arya::IClockService::registerEventSink
                , std::placeholders::_1
                , std::placeholders::_2
                )
            , [](const Result& fep_result)
                {
                    return fep_result.getErrorCode();
                }
            , result
            , destruction_manager_access_result
            , event_sink_access
            );
    }
    static inline fep3_plugin_c_InterfaceError unregisterEventSink
        (Handle handle
        , int32_t* result
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_access_result
        , fep3_arya_IClock_SIEventSink event_sink_access
        ) noexcept
    {
        return Helper::transferWeakPtrWithResultParameter<access::arya::Clock::EventSink>
            (handle
            , std::bind
                (&fep3::arya::IClockService::unregisterEventSink
                , std::placeholders::_1
                , std::placeholders::_2
                )
            , [](const Result& fep_result)
                {
                    return fep_result.getErrorCode();
                }
            , result
            , destruction_manager_access_result
            , event_sink_access
            );
    }
    static inline fep3_plugin_c_InterfaceError registerClock
        (Handle handle
        , int32_t* result
        , fep3_plugin_c_arya_SDestructionManager reference_manager_access
        , fep3_arya_SIClock clock_access
        ) noexcept
    {
        return Helper::transferSharedPtrWithResultParameter<::fep3::plugin::c::access::arya::Clock>
            (handle
            , std::bind
                // using static_cast to disambiguate the address of the overload
                (&fep3::arya::IClockRegistry::registerClock
                , std::placeholders::_1
                , std::placeholders::_2
                )
            , [](const Result& fep_result)
                {
                    return fep_result.getErrorCode();
                }
            , result
            , reference_manager_access
            , clock_access
            );
    }

    static inline fep3_plugin_c_InterfaceError unregisterClock
        (Handle handle
        , int32_t* result
        , const char* clock_name
        ) noexcept
    {
        return Helper::callWithResultParameter
            (handle
            , &fep3::arya::IClockRegistry::unregisterClock
            , [](const Result& fep_result)
                {
                    return fep_result.getErrorCode();
                }
            , result
            , clock_name
            );
    }

    static inline fep3_plugin_c_InterfaceError getClockNames
        (Handle handle
        , void (*callback)(void*, const char*)
        , void* destination
        ) noexcept
    {
        return Helper::callWithRecurringResultCallback
            (handle
            , &fep3::arya::IClockRegistry::getClockNames
            , callback
            , destination
            , [](const std::string& clock_name)
                {
                    return clock_name.c_str();
                }
            );
    }
    static inline fep3_plugin_c_InterfaceError findClock
        (Handle handle
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_result
        , fep3_arya_SIClock* clock_access_result
        , const char* clock_name
        ) noexcept
    {
        return Helper::getSharedPtr
            (handle
            , &fep3::arya::IClockRegistry::findClock
            , destruction_manager_result
            , clock_access_result
            , [](const auto& pointer_to_clock)
                {
                    return ::fep3::plugin::c::wrapper::arya::Clock::AccessCreator()(pointer_to_clock);
                }
            , clock_name
            );
    }
    /// @endcond no_documentation

public:
    /// Type of access structure
    using Access = fep3_arya_SIClockService;
};

namespace detail
{

/// @cond no_documentation
inline fep3_plugin_c_InterfaceError getClockService
    (fep3_arya_SIClockService* access_result
    , const char* iid
    , fep3_arya_HIComponent handle_to_component
    )
{
    if(0 == strcmp(::fep3::arya::IClockService::getComponentIID(), iid))
    {
        return ::fep3::plugin::c::wrapper::arya::get<::fep3::arya::IComponent, ::fep3::arya::IClockService>
            (access_result
            , handle_to_component
            , [](::fep3::arya::IClockService* pointer_to_object)
                {
                    return fep3_arya_SIClockService
                        {reinterpret_cast<fep3_arya_HIClockService>(pointer_to_object)
                        , {} // don't provide access to IComponent interface
                        , wrapper::arya::ClockService::getTime
                        , wrapper::arya::ClockService::getTimeByClockName
                        , wrapper::arya::ClockService::getType
                        , wrapper::arya::ClockService::getTypeByClockName
                        , wrapper::arya::ClockService::getMainClockName
                        , wrapper::arya::ClockService::registerEventSink
                        , wrapper::arya::ClockService::unregisterEventSink
                        , wrapper::arya::ClockService::registerClock
                        , wrapper::arya::ClockService::unregisterClock
                        , wrapper::arya::ClockService::getClockNames
                        , wrapper::arya::ClockService::findClock
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

template<typename factory_type>
inline fep3_plugin_c_InterfaceError createClockService
    (factory_type&& factory
    , fep3_arya_SIClockService* result
    , const fep3_plugin_c_arya_SISharedBinary& shared_binary_access
    , const char* iid
    ) noexcept
{
    using clock_service_type = typename std::remove_pointer<decltype(std::declval<factory_type>()())>::type;
    if(0 == strcmp(clock_service_type::getComponentIID(), iid))
    {
        return create
            (factory
            , result
            , shared_binary_access
            , [](clock_service_type* pointer_to_object)
                {
                    return fep3_arya_SIClockService
                        {reinterpret_cast<fep3_arya_HIClockService>(static_cast<::fep3::arya::IClockService*>(pointer_to_object))
                        , wrapper::arya::Component::AccessCreator()(pointer_to_object)
                        , wrapper::arya::ClockService::getTime
                        , wrapper::arya::ClockService::getTimeByClockName
                        , wrapper::arya::ClockService::getType
                        , wrapper::arya::ClockService::getTypeByClockName
                        , wrapper::arya::ClockService::getMainClockName
                        , wrapper::arya::ClockService::registerEventSink
                        , wrapper::arya::ClockService::unregisterEventSink
                        , wrapper::arya::ClockService::registerClock
                        , wrapper::arya::ClockService::unregisterClock
                        , wrapper::arya::ClockService::getClockNames
                        , wrapper::arya::ClockService::findClock
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
/// @endcond no_documentation

} // namespace detail

/**
 * Creates a clock service object of type \p clock_service_type
 * @tparam clock_service_type The type of the clock service object to be created
 * @param result Pointer to the access structure to the created clock service object
 * @param shared_binary_access Access strcuture to the shared binary the clock service object resides in
 * @param iid The interface ID of the clock service interface of the created object
 * @return Interface error code
 * @retval fep3_plugin_c_interface_error_none No error occurred
 * @retval fep3_plugin_c_interface_error_invalid_result_pointer The @p result is null
 * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown while creating the component
 */
template<typename clock_service_type>
inline fep3_plugin_c_InterfaceError createClockService
    (fep3_arya_SIClockService* result
    , const fep3_plugin_c_arya_SISharedBinary& shared_binary_access
    , const char* iid
    ) noexcept
{
    return detail::createClockService
        ([]()
            {
                return new clock_service_type;
            }
        , result
        , shared_binary_access
        , iid
        );
}

} // namespace arya
} // namespace wrapper

namespace access
{
namespace arya
{

ClockService::ClockService
    (const Access& access
    , const std::shared_ptr<ISharedBinary>& shared_binary
    )
    : ComponentBase<fep3::arya::IClockService>
        (access._component
        , shared_binary
        )
    , _access(access)
{}

/// @cond no_documentation
fep3::arya::Timestamp ClockService::getTime() const
{
    return fep3::arya::Timestamp(Helper::callWithResultParameter
        (_access._handle
        , _access.getTime
        ));
}

fep3::arya::Optional<fep3::arya::Timestamp> ClockService::getTime(const std::string& clock_name) const
{
    using function_type = decltype(_access.getTimeByClockName);
    typename std::remove_pointer<typename FunctionParameters<function_type>::Parameter2Type>::type result{};
    bool validity{};
    fep3_plugin_c_InterfaceError error = _access.getTimeByClockName
        (_access._handle
        , &result
        , &validity
        , clock_name.c_str()
        );
    if(fep3_plugin_c_interface_error_none != error)
    {
        throw Exception(error);
    }
    return validity ? fep3::arya::Timestamp(result) : fep3::arya::Optional<fep3::arya::Timestamp>{};
}

fep3::arya::IClock::ClockType ClockService::getType() const
{
    return static_cast<fep3::arya::IClock::ClockType>(Helper::callWithResultParameter
        (_access._handle
        , _access.getType
        ));
}

fep3::arya::Optional<fep3::arya::IClock::ClockType> ClockService::getType(const std::string& clock_name) const
{
    using function_type = decltype(_access.getTypeByClockName);
    typename std::remove_pointer<typename FunctionParameters<function_type>::Parameter2Type>::type result{};
    bool validity{};
    fep3_plugin_c_InterfaceError error = _access.getTypeByClockName
        (_access._handle
        , &result
        , &validity
        , clock_name.c_str()
        );
    if(fep3_plugin_c_interface_error_none != error)
    {
        throw Exception(error);
    }
    return validity ? static_cast<fep3::arya::IClock::ClockType>(result) : fep3::arya::Optional<fep3::arya::IClock::ClockType>{};
}

std::string ClockService::getMainClockName() const
{
    return Helper::callWithResultCallback<std::string>
        (_access._handle
        , _access.getMainClockName
        );
}

fep3::Result ClockService::registerEventSink(const std::weak_ptr<::fep3::arya::IClock::IEventSink>& event_sink)
{
    return Helper::transferWeakPtrWithResultParameter<fep3::Result>
        (event_sink
        , _remote_event_sink_object_destructors
        , _access._handle
        , _access.registerEventSink
        , [](const auto& pointer_to_event_sink)
            {
                return ::fep3::plugin::c::wrapper::arya::Clock::EventSink::AccessCreator()(pointer_to_event_sink);
            }
        );
}

fep3::Result ClockService::unregisterEventSink(const std::weak_ptr<::fep3::arya::IClock::IEventSink>& event_sink)
{
    return Helper::transferWeakPtrWithResultParameter<fep3::Result>
        (event_sink
        , _remote_event_sink_object_destructors
        , _access._handle
        , _access.unregisterEventSink
        , [](const auto& pointer_to_event_sink)
            {
                return ::fep3::plugin::c::wrapper::arya::Clock::EventSink::AccessCreator()(pointer_to_event_sink);
            }
        );
}

fep3::Result ClockService::registerClock(const std::shared_ptr<fep3::arya::IClock>& clock)
{
    return Helper::transferSharedPtrWithResultParameter<fep3::Result>
        (clock
        , _access._handle
        , _access.registerClock
        , [](const auto& pointer_to_clock)
            {
                return fep3_arya_SIClock
                    {reinterpret_cast<fep3_arya_HIClock>(pointer_to_clock)
                    , ::fep3::plugin::c::wrapper::arya::Clock::getName
                    , ::fep3::plugin::c::wrapper::arya::Clock::getType
                    , ::fep3::plugin::c::wrapper::arya::Clock::getTime
                    , ::fep3::plugin::c::wrapper::arya::Clock::reset
                    , ::fep3::plugin::c::wrapper::arya::Clock::start
                    , ::fep3::plugin::c::wrapper::arya::Clock::stop
                    };
            }
        );
}

fep3::Result ClockService::unregisterClock(const std::string& clock_name)
{
    return Helper::callWithResultParameter
        (_access._handle
        , _access.unregisterClock
        , clock_name.c_str()
        );
}

std::list<std::string> ClockService::getClockNames() const
{
    return Helper::callWithRecurringResultCallback<std::list<std::string>, const char*>
        (_access._handle
        , _access.getClockNames
        , [](const char* clock_name)
            {
                return clock_name;
            }
        , &std::list<std::string>::push_back
        );
}

std::shared_ptr<fep3::arya::IClock> ClockService::findClock(const std::string& clock_name) const
{
    return Helper::getSharedPtr<::fep3::plugin::c::access::arya::Clock, fep3_arya_SIClock>
        (_access._handle
        , _access.findClock
        , clock_name.c_str()
        );

}
/// @endcond no_documentation

} // namespace arya
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3

/**
 * Gets access to a clock service object as identified by @p handle_to_component
 * @param access_result Pointer to the access structure to the clock service object
 * @param iid The interface ID of the clock service interface to get
 * @param handle_to_component Handle to the interface of the object to get
 * @return Interface error code
 * @retval fep3_plugin_c_interface_error_none No error occurred
 * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle_to_component is null
 * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p access_result is null
 * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown while getting the component
 */
inline fep3_plugin_c_InterfaceError fep3_plugin_c_arya_getClockService
    (fep3_arya_SIClockService* access_result
    , const char* iid
    , fep3_arya_HIComponent handle_to_component
    )
{
    return ::fep3::plugin::c::wrapper::arya::detail::getClockService
        (access_result
        , iid
        , handle_to_component
        );
}