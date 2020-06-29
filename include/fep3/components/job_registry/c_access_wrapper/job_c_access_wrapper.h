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

#include <fep3/fep3_macros.h>
#include <fep3/components/job_registry/c_intf/job_c_intf.h>
#include <fep3/components/job_registry/job_intf.h>
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
 * Access class for @ref fep3::arya::IJob.
 * Use this class to access a remote object of a type derived from @ref fep3::arya::IJob
 * that resides in another binary (e. g. a shared library).
 */
class Job
    : public ::fep3::arya::IJob
    , private DestructionManager
    , private Helper
{
public:
    /// Type of access structure
    using Access = fep3_arya_SIJob;

    /**
     * @brief CTOR
     * @param access Access to the remote object
     * @param destructors List of destructors to be called upon destruction of this
     */
    inline Job
        (const Access& access
        , std::deque<std::unique_ptr<IDestructor>> destructors
        );
    inline ~Job() override = default;

    // methods implementing fep3::arya::IJob
    /// @cond no_documentation
    inline fep3::Result executeDataIn(Timestamp time_of_execution) override;
    inline fep3::Result execute(Timestamp time_of_execution) override;
    inline fep3::Result executeDataOut(Timestamp time_of_execution) override;
    /// @endcond no_documentation

private:
    Access _access;
};

/**
 * Fetches the job name via @p job_info_access
 *
 * @param job_info_access Job info access to fetch the job name from
 * @return The fetched job name
 */
inline std::string fetchJobName(const fep3_arya_SJobInfo& job_info_access)
{
    std::string job_name;
    job_info_access._name_callback
        (job_info_access._job_info_source
        , [](void* destination, const char* job_name)
            {
                auto name_destination = reinterpret_cast<std::string*>(destination);
                name_destination->assign(job_name);
            }
        , reinterpret_cast<void*>(&job_name)
        );
    return job_name;
}

/**
 * Fetches the @ref fep3_arya_SJobConfiguration via @p job_info_access
 *
 * @param job_info_access Job info access to fetch the job name from
 * @return The fetched job name
 */
inline fep3_arya_SJobConfiguration fetchJobConfigurationAccess(const fep3_arya_SJobInfo& job_info_access)
{
    fep3_arya_SJobConfiguration job_configuration_access;
        job_info_access._config_callback
            (job_info_access._job_info_source
            , [](void* destination, fep3_arya_SJobConfiguration config)
                {
                    auto config_destination = reinterpret_cast<fep3_arya_SJobConfiguration*>(destination);
                    *config_destination = config;
                }
            , reinterpret_cast<void*>(&job_configuration_access)
            );
    return job_configuration_access;
}

/**
 * Creates a @ref fep3::arya::JobConfiguration from @p job_configuration_access
 *
 * @param job_configuration_access Job configuration access to create the @ref fep3::arya::JobConfiguration from
 * @return The created @ref fep3::arya::JobConfiguration
 */
inline fep3::arya::JobConfiguration createJobConfiguration(const fep3_arya_SJobConfiguration& job_configuration_access)
{
    std::vector<std::string> dependee_job_names;
    job_configuration_access._dependee_job_names_callback
        (job_configuration_access._dependee_job_names_source
        , [](void* destination, const char* dependee_job_name)
            {
                auto dependee_job_names_destination = reinterpret_cast<std::vector<std::string>*>(destination);
                dependee_job_names_destination->push_back(dependee_job_name);
            }
        , reinterpret_cast<void*>(&dependee_job_names)
        );
    return fep3::arya::JobConfiguration
        (fep3::Duration(job_configuration_access._cycle_sim_time)
        , fep3::Duration(job_configuration_access._delay_sim_time)
        , job_configuration_access._max_runtime_real_time_validity
            ? fep3::Duration(job_configuration_access._max_runtime_real_time)
            : Optional<Duration>{}
        , static_cast<fep3::arya::JobConfiguration::TimeViolationStrategy>(job_configuration_access._runtime_violation_strategy)
        , dependee_job_names
        );
}

} // namespace arya
} // namespace access

namespace wrapper
{
namespace arya
{

/**
 * Wrapper class for interface @ref fep3::arya::IJob
 */
class Job : private Helper<fep3::arya::IJob>
{
public:
    /**
     * Functor creating a structure for @ref fep3::arya::JobConfiguration
     */
    struct JobConfigurationAccessCreator
    {
        /**
         * Creates a structure containing job configuration from @p job_configuration
         *
         * @param job_configuration Job configuration to create the structure from
         * @return Structure containing the job configuration
         */
        fep3_arya_SJobConfiguration operator()(const fep3::arya::JobConfiguration& job_configuration)
        {
            return fep3_arya_SJobConfiguration
                {job_configuration._cycle_sim_time.count()
                , job_configuration._delay_sim_time.count()
                , job_configuration._max_runtime_real_time.has_value()
                , job_configuration._max_runtime_real_time.value_or(0).count()
                , static_cast<int32_t>(job_configuration._runtime_violation_strategy)
                , [](void* source, void(*dependee_job_name_callback)(void*, const char*), void* dependee_job_name_destination)
                    {
                        const auto& dependee_job_names_source
                            = reinterpret_cast<decltype(job_configuration._jobs_this_depends_on)*>(source);
                        for(const auto& dependee_job_name_source : *dependee_job_names_source)
                        {
                            dependee_job_name_callback(dependee_job_name_destination, dependee_job_name_source.c_str());
                        }
                    }
                , reinterpret_cast<void*>(const_cast<decltype(job_configuration._jobs_this_depends_on)*>(&job_configuration._jobs_this_depends_on))
                };
        }
        
        /**
         * Creates a structure containing job configuration from @p job_info
         *
         * @param job_info Job info to create the structure from
         * @return Structure containing the job configuration
         */
        fep3_arya_SJobConfiguration operator()(const fep3::arya::JobInfo& job_info)
        {
            const auto& job_configuration = job_info.getConfig();
            return fep3_arya_SJobConfiguration
                {job_configuration._cycle_sim_time.count()
                , job_configuration._delay_sim_time.count()
                , job_configuration._max_runtime_real_time.has_value()
                , job_configuration._max_runtime_real_time.value_or(0).count()
                , static_cast<int32_t>(job_configuration._runtime_violation_strategy)
                , [](void* source, void(*dependee_job_name_callback)(void*, const char*), void* dependee_job_name_destination)
                    {
                        const auto& job_info_source
                            = reinterpret_cast<std::decay<decltype(job_info)>::type*>(source);
                        for(const auto& dependee_job_name_source : job_info_source->getConfig()._jobs_this_depends_on)
                        {
                            dependee_job_name_callback(dependee_job_name_destination, dependee_job_name_source.c_str());
                        }
                    }
                , reinterpret_cast<void*>(const_cast<std::decay<decltype(job_info)>::type*>(&job_info))
                };
        }
    };
    
    /// Alias for the helper
    using Helper = Helper<fep3::arya::IJob>;
    /// Alias for the type of the handle to a wrapped object of type @ref fep3::arya::IJob
    using Handle = fep3_arya_HIJob;

    // static methods transferring calls from the C interface to an object of fep3::arya::IJob
    /// @cond no_documentation
    static inline fep3_plugin_c_InterfaceError executeDataIn
        (Handle handle
        , int32_t* result
        , int64_t time_of_execution
        ) noexcept
    {
        return Helper::callWithResultParameter
            (handle
            , &fep3::arya::IJob::executeDataIn
            , [](const Result& fep_result)
                {
                    return fep_result.getErrorCode();
                }
            , result
            , ::fep3::arya::Timestamp(time_of_execution)
            );
    }
    static inline fep3_plugin_c_InterfaceError execute
        (Handle handle
        , int32_t* result
        , int64_t time_of_execution
        ) noexcept
    {
        return Helper::callWithResultParameter
            (handle
            , &fep3::arya::IJob::execute
            , [](const Result& fep_result)
                {
                    return fep_result.getErrorCode();
                }
            , result
            , ::fep3::arya::Timestamp(time_of_execution)
            );
    }
    static inline fep3_plugin_c_InterfaceError executeDataOut
        (Handle handle
        , int32_t* result
        , int64_t time_of_execution
        ) noexcept
    {
        return Helper::callWithResultParameter
            (handle
            , &fep3::arya::IJob::executeDataOut
            , [](const Result& fep_result)
                {
                    return fep_result.getErrorCode();
                }
            , result
            , ::fep3::arya::Timestamp(time_of_execution)
            );
    }
    /// @endcond no_documentation
};

} // namespace arya
} // namespace wrapper

namespace access
{
namespace arya
{

/// @cond no_documentation
Job::Job
    (const Access& access
    , std::deque<std::unique_ptr<IDestructor>> destructors
    )
    : _access(access)
{
    addDestructors(std::move(destructors));
}

fep3::Result Job::executeDataIn(Timestamp time_of_execution)
{
    return Helper::callWithResultParameter
        (_access._handle
        , _access.executeDataIn
        , time_of_execution.count()
        );
}

fep3::Result Job::execute(Timestamp time_of_execution)
{
    return Helper::callWithResultParameter
        (_access._handle
        , _access.execute
        , time_of_execution.count()
        );
}

fep3::Result Job::executeDataOut(Timestamp time_of_execution)
{
    return Helper::callWithResultParameter
        (_access._handle
        , _access.executeDataOut
        , time_of_execution.count()
        );
}
/// @endcond no_documentation

} // namespace arya
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3
