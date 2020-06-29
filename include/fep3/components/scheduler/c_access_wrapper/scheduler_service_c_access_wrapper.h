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
#include <fep3/components/scheduler/c_intf/scheduler_service_c_intf.h>
#include <fep3/components/scheduler/scheduler_service_intf.h>
#include <fep3/components/base/c_access_wrapper/component_base_c_access.h>
#include <fep3/components/base/c_access_wrapper/component_c_wrapper.h>
#include "scheduler_c_access_wrapper.h"

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
 * @brief Access class for @ref fep3::arya::ISchedulerService.
 * Use this class to access a remote object of a type derived from ISchedulerService that resides in another binary (e. g. a shared library).
 */
class SchedulerService
    : public ::fep3::plugin::c::access::arya::ComponentBase<::fep3::arya::ISchedulerService>
{
public:
    /// Symbol name of the create function that is capable to create a scheduler service
    static constexpr const char* const create_function_name = FEP3_EXPAND_TO_STRING(SYMBOL_fep3_plugin_c_arya_createSchedulerService);
    /// Gets the function to get an instance of a scheduler service that resides in a C plugin
    static decltype(&fep3_plugin_c_arya_getSchedulerService) getGetterFunction()
    {
         return fep3_plugin_c_arya_getSchedulerService;
    }
    /// Type of access object
    using Access = fep3_arya_SISchedulerService;
    
    /**
     * CTOR
     *
     * @param access Access to the remote object
     * @param shared_binary Shared pointer to the binary this resides in
     */
    inline SchedulerService
        (const Access& access
        , const std::shared_ptr<ISharedBinary>& shared_binary
        );
    /**
     * DTOR destroying the corresponding remote object
     */
    inline ~SchedulerService() override = default;

    /// @cond no_documentation
    // methods implementing fep3::arya::ISchedulerService
    inline std::string getActiveSchedulerName() const override;
    // methods implementing fep3::arya::ISchedulerRegistry
    inline fep3::Result registerScheduler(std::unique_ptr<fep3::arya::IScheduler> scheduler) override;
    inline fep3::Result unregisterScheduler(const std::string& scheduler_name) override;
    inline std::list<std::string> getSchedulerNames() const override;
    /// @endcond no_documentation

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
 * Wrapper class for interface @ref fep3::arya::ISchedulerService
 */
class SchedulerService : private Helper<fep3::arya::ISchedulerService>
{
private:
    using Helper = Helper<fep3::arya::ISchedulerService>;
    using Handle = fep3_arya_HISchedulerService;

public:
    /// @cond no_documentation
    static inline fep3_plugin_c_InterfaceError getActiveSchedulerName
        (Handle handle
        , void(*callback)(void*, const char*)
        , void* destination
        ) noexcept
    {
        return Helper::callWithResultCallback
            (handle
            , &fep3::arya::ISchedulerService::getActiveSchedulerName
            , callback
            , destination
            , [](const std::string& active_scheduler_name)
                {
                    return active_scheduler_name.c_str();
                }
            );
    }
    
    static inline fep3_plugin_c_InterfaceError registerScheduler
        (Handle handle
        , int32_t* result
        , fep3_plugin_c_arya_SDestructionManager destruction_manager_access
        , fep3_arya_SIScheduler scheduler_access
        ) noexcept
    {
        return Helper::transferUniquePtrWithResultParameter<::fep3::plugin::c::access::arya::Scheduler>
            (handle
            , &fep3::arya::ISchedulerRegistry::registerScheduler
            , [](const Result& fep_result)
                {
                    return fep_result.getErrorCode();
                }
            , result
            , destruction_manager_access
            , scheduler_access
            );
    }

    static inline fep3_plugin_c_InterfaceError unregisterScheduler
        (Handle handle
        , int32_t* result
        , const char* scheduler_name
        ) noexcept
    {
        return Helper::callWithResultParameter
            (handle
            , &fep3::arya::ISchedulerRegistry::unregisterScheduler
            , [](const Result& fep_result)
                {
                    return fep_result.getErrorCode();
                }
            , result
            , scheduler_name
            );
    }

    static inline fep3_plugin_c_InterfaceError getSchedulerNames
        (Handle handle
        , void (*callback)(void*, const char*)
        , void* destination
        ) noexcept
    {
        return Helper::callWithRecurringResultCallback
            (handle
            , &fep3::arya::ISchedulerRegistry::getSchedulerNames
            , callback
            , destination
            , [](const std::string& scheduler_name)
                {
                    return scheduler_name.c_str();
                }
            );
    }
    /// @endcond no_documentation

private:
    /// Type of access structure
    using Access = fep3_arya_SISchedulerService;
};

namespace detail
{

/// @cond no_documentation
inline fep3_plugin_c_InterfaceError getSchedulerService
    (fep3_arya_SISchedulerService* access_result
    , const char* iid
    , fep3_arya_HIComponent handle_to_component
    )
{
    if(0 == strcmp(::fep3::arya::ISchedulerService::getComponentIID(), iid))
    {
        return ::fep3::plugin::c::wrapper::arya::get<::fep3::arya::IComponent, ::fep3::arya::ISchedulerService>
            (access_result
            , handle_to_component
            , [](::fep3::arya::ISchedulerService* pointer_to_object)
                {
                    return fep3_arya_SISchedulerService
                        {reinterpret_cast<fep3_arya_HISchedulerService>(pointer_to_object)
                        , {} // don't provide access to IComponent interface
                        , wrapper::arya::SchedulerService::getActiveSchedulerName
                        , wrapper::arya::SchedulerService::registerScheduler
                        , wrapper::arya::SchedulerService::unregisterScheduler
                        , wrapper::arya::SchedulerService::getSchedulerNames
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
inline fep3_plugin_c_InterfaceError createSchedulerService
    (factory_type&& factory
    , fep3_arya_SISchedulerService* result
    , const fep3_plugin_c_arya_SISharedBinary& shared_binary_access
    , const char* iid
    ) noexcept
{
    using scheduler_service_type = typename std::remove_pointer<decltype(std::declval<factory_type>()())>::type;
    if(0 == strcmp(scheduler_service_type::getComponentIID(), iid))
    {
        return create
            (factory
            , result
            , shared_binary_access
            , [](scheduler_service_type* pointer_to_object)
                {
                    return fep3_arya_SISchedulerService
                        {reinterpret_cast<fep3_arya_HISchedulerService>(static_cast<ISchedulerService*>(pointer_to_object))
                        , wrapper::arya::Component::AccessCreator()(pointer_to_object)
                        , wrapper::arya::SchedulerService::getActiveSchedulerName
                        , wrapper::arya::SchedulerService::registerScheduler
                        , wrapper::arya::SchedulerService::unregisterScheduler
                        , wrapper::arya::SchedulerService::getSchedulerNames
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
 * Creates a scheduler service object of type \p scheduler_service_type
 * @tparam scheduler_service_type The type of the scheduler service object to be created
 * @param access_result Pointer to the access structure to the created scheduler service object
 * @param shared_binary_access Access strcuture to the shared binary the scheduler service object resides in
 * @param iid The interface ID of the scheduler service interface of the created object
 * @return Interface error code
 * @retval fep3_plugin_c_interface_error_none No error occurred
 * @retval fep3_plugin_c_interface_error_invalid_result_pointer The @p result is null
 * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown while creating the component
 */
template<typename scheduler_service_type>
inline fep3_plugin_c_InterfaceError createSchedulerService
    (fep3_arya_SISchedulerService* access_result
    , const fep3_plugin_c_arya_SISharedBinary& shared_binary_access
    , const char* iid
    ) noexcept
{
    return detail::createSchedulerService
        ([]()
            {
                return new scheduler_service_type;
            }
        , access_result
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

SchedulerService::SchedulerService
    (const Access& access
    , const std::shared_ptr<ISharedBinary>& shared_binary
    )
    : ::fep3::plugin::c::access::arya::ComponentBase<fep3::arya::ISchedulerService>
        (access._component
        , shared_binary
        )
    , _access(access)
{}

/// @cond no_documentation
std::string SchedulerService::getActiveSchedulerName() const
{
    return Helper::callWithResultCallback<std::string>
        (_access._handle
        , _access.getActiveSchedulerName
        );
}

fep3::Result SchedulerService::registerScheduler(std::unique_ptr<fep3::arya::IScheduler> scheduler)
{
    return Helper::transferUniquePtrWithResultParameter<fep3::Result>
        (std::move(scheduler)
        , _access._handle
        , _access.registerScheduler
        , [](const auto& pointer_to_scheduler)
            {
                return fep3_arya_SIScheduler
                    {reinterpret_cast<fep3_arya_HIScheduler>(pointer_to_scheduler)
                    , ::fep3::plugin::c::wrapper::arya::Scheduler::getName
                    , ::fep3::plugin::c::wrapper::arya::Scheduler::initialize
                    , ::fep3::plugin::c::wrapper::arya::Scheduler::start
                    , ::fep3::plugin::c::wrapper::arya::Scheduler::stop
                    , ::fep3::plugin::c::wrapper::arya::Scheduler::deinitialize
                    };
            }
        );
}

fep3::Result SchedulerService::unregisterScheduler(const std::string& scheduler_name)
{
    return Helper::callWithResultParameter
        (_access._handle
        , _access.unregisterScheduler
        , scheduler_name.c_str()
        );
}

std::list<std::string> SchedulerService::getSchedulerNames() const
{
    return Helper::callWithRecurringResultCallback<std::list<std::string>, const char*>
        (_access._handle
        , _access.getSchedulerNames
        , [](const char* scheduler_name)
            {
                return scheduler_name;
            }
        , &std::list<std::string>::push_back
        );
}
/// @endcond no_documentation

} // namespace arya
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3

/**
 * Gets access to a scheduler service object as identified by @p handle_to_component
 * @param access_result Pointer to the access structure to the scheduler service object
 * @param iid The interface ID of the scheduler service interface to get
 * @param handle_to_component Handle to the interface of the object to get
 * @return Interface error code
 * @retval fep3_plugin_c_interface_error_none No error occurred
 * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle_to_component is null
 * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p access_result is null
 * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown while getting the component
 */
inline fep3_plugin_c_InterfaceError fep3_plugin_c_arya_getSchedulerService
    (fep3_arya_SISchedulerService* access_result
    , const char* iid
    , fep3_arya_HIComponent handle_to_component
    )
{
    return ::fep3::plugin::c::wrapper::arya::detail::getSchedulerService
        (access_result
        , iid
        , handle_to_component
        );
}