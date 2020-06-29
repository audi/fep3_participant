/**
 *
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

#include <functional>
#include <vector>
#include <algorithm>

#include <fep3/fep3_timestamp.h>
#include <fep3/components/base/components_intf.h>
#include <fep3/components/job_registry/job_registry_intf.h>

namespace fep3
{
namespace core
{
namespace arya
{

/**
 * @brief Job class implementing @ref fep3::arya::IJob
 */
class Job : public fep3::arya::IJob
{
public:
    /// ExecuteCallback typedef
    typedef std::function<Result(Timestamp)> ExecuteCallback;
    /**
     * @brief CTOR 
     * 
     * @param name Name of the Job
     * @param cycle_time Cycle time of the job (simulation time)
     */
    Job(std::string name, Duration cycle_time)
        : _job_info(std::move(name), cycle_time),
         _execution_cb([](Timestamp) -> Result {return Result(); })
    {
    }

    /**
     * @brief CTOR
     *
     * @param name Name of the Job
     * @param fc Function
     * @param cycle_time Cycle time of the job (simulation time)
     */
    Job(std::string name, Duration cycle_time, ExecuteCallback fc)
        : _job_info(std::move(name), cycle_time),
        _execution_cb(fc)
    {
    }

    /**
     * @brief CTOR
     * 
     * @param name Name of the job
     * @param config Configuration of the job
     */
    Job(std::string name, fep3::arya::JobConfiguration config)
        : _job_info(std::move(name), std::move(config)),
        _execution_cb([](Timestamp) -> Result {return Result(); })
    {
    }

    /**
     * @brief CTOR
     *
     * @param name Name of the job
     * @param fc Function
     * @param config Configuration of the job
     */
    Job(std::string name, fep3::arya::JobConfiguration config, ExecuteCallback fc)
        : _job_info(std::move(name), std::move(config)),
        _execution_cb(fc)
    {
    }

protected:
    /**
     * @brief Reads input samples.
     * 
     * Typically here the samples are read using DataReaders.
     * 
     * @return fep3::Result. Return any FEP result besides ERR_NOERROR to signal an error
     * @retval ERR_NOERROR Everything went fine
     * @retval ERR_UNEXPECTED An unexpected error occurred
     */     

    fep3::Result executeDataIn(Timestamp /*time_of_execution*/) override
    {
        return {};
    }

    /**
     * @brief Actual processing of the job.
     *
     * Typically here the data of input samples is processed and output data is created.
     * The execute method can either be overriden or a callback of type @ref fep3::core::arya::Job::ExecuteCallback
     * can be passed using one of the constructors.
     * 
     * @param time_of_execution The current simulation time
     * @return fep3::Result. Return any FEP result besides ERR_NOERROR to signal an error
     * @retval ERR_NOERROR Everything went fine
     * @retval ERR_UNEXPECTED An unexpected error occurred
     */
    fep3::Result execute(Timestamp time_of_execution) override
    {
        return _execution_cb(time_of_execution);
    }

    /**
     * @brief Writes output samples.
     * 
     * Typically here the samples are published using DataWriter's.
     * 
     * @return fep3::Result. Return any FEP result besides ERR_NOERROR to signal an error
     * @retval ERR_NOERROR Everything went fine
     * @retval ERR_UNEXPECTED An unexpected error occurred
     */
    fep3::Result executeDataOut(Timestamp /*time_of_execution*/) override
    {
        return {};
    }

public:
    /**
     * @brief Gets the @ref fep3::arya::JobInfo for the job.
     *
     * @return Job info for job
     */
    fep3::JobInfo getJobInfo() const
    {
        return _job_info;
    }

    /**
     * @brief Reconfigures the job using the @p configuration.
     * 
     * @param configuration Configuration the job should be reconfigured with
     * @return fep3::Result. Return any FEP result besides ERR_NOERROR to signal an error
     * @retval ERR_NOERROR Everything went fine
     */
    fep3::Result reconfigure(const JobConfiguration& configuration)
    {
        _job_info = fep3::JobInfo(getJobInfo().getName(), configuration);
        return {};
    }

    /**
     * @brief Resets the job.

     * @return fep3::Result. Return any FEP result besides ERR_NOERROR to signal an error
     * @retval ERR_NOERROR Everything went fine
     */
    virtual fep3::Result reset()
    {
        return {};
    }

    private:
        fep3::JobInfo _job_info;
        ExecuteCallback _execution_cb;
};


/**
* @brief Jobs will be added to the @p job_registry.
* If one job can not be added, the function returns not adding the following ones.
*
* @param jobs List of jobs to be added
* @param job_registry The job registry to add to
* @return fep3::Result
* @retval ERR_NOERROR Everything went fine
* @retval ERR_RESOURCE_IN_USE A job with the given name is already registered
*/
inline fep3::Result addJobsToJobRegistry(const std::vector<std::shared_ptr<Job>>& jobs, IJobRegistry& job_registry)
{
    for (const auto& job : jobs)
    {
        FEP3_RETURN_IF_FAILED(job_registry.addJob(job->getJobInfo().getName(), job, job->getJobInfo().getConfig()));
    }

    return{};
}

/**
* @brief Jobs will be added to the job registry of the @p component.
* If one job can not be added, the function returns not adding the following ones.
*
* @param jobs List of jobs to be added
* @param components The component container in which the job registry will be looked up
* @return fep3::Result
* @retval ERR_NOERROR Everything went fine
* @retval ERR_RESOURCE_IN_USE A job with the given name is already registered
* @retval ERR_NO_INTERFACE The @ref fep3::arya::IJobRegistry was not found within @p components
*/
inline fep3::Result addToComponents(const std::vector<std::shared_ptr<Job>>& jobs, const IComponents& components)
{
    //do not lock here... this is task of the job registry
    auto job_registry = fep3::getComponent<IJobRegistry>(components);
    if (!job_registry)
    {
        RETURN_ERROR_DESCRIPTION(ERR_NO_INTERFACE,
            a_util::strings::format("could not find '%s' in components", IJobRegistry::FEP3_COMP_IID).c_str());
    }

    return addJobsToJobRegistry(jobs, *job_registry);
}

/**
* @brief Jobs will be removed from the @p job_registry.
* If one job can not be removed, the function will still try to remove the following ones within @p job_names.
*
* @param job_names List of jobs to be removed
* @param job_registry The job registry to remove from
* @return fep3::Result.
*           If it's an actual error it is the error code for the job that failed last.
*           If more than one remove fails, the error description contains a list of error descriptions separated by ';'.
* @retval ERR_NOERROR Everything went fine
* @retval ERR_NOT_FOUND A job with the given name is not registered
*/
inline fep3::Result removeJobsFromJobRegistry(const std::vector<std::string>& job_names, IJobRegistry& job_registry)
{
    auto result = fep3::Result{};
    std::string message = "";
    for (const auto& job_name : job_names)
    {
        const auto current_result = job_registry.removeJob(job_name);
        result |= current_result;

        if (isFailed(current_result))
        {
            message = message + "; " + current_result.getDescription();
        }
    }
    if (isFailed(result))
    {
        return CREATE_ERROR_DESCRIPTION(result,
            message.size() < 2 ? message.c_str() : std::string(message.begin() + 2, message.end()).c_str());
    }

    return result;
}

/**
* @brief Jobs will be removed from the job registry of the @p component.
* If one job can not be removed, the function will still try to remove the following ones within @p job_names.
*
* @param job_names List of jobs to be removed
* @param components The component container in which the job registry will be looked up
* @return fep3::Result.
*           If it's an actual error it is the error code for the job that failed last.
*           If more than one remove fails, the error description contains a list of error descriptions separated by ';'.
* @retval ERR_NOERROR Everything went fine
* @retval ERR_NOT_FOUND A job with the given name is not registered
* @retval ERR_NO_INTERFACE The @ref fep3::arya::IJobRegistry was not found within @p components
*/
inline fep3::Result removeFromComponents(const std::vector<std::string>& job_names, const IComponents& components)
{
    //do not lock here... this is task of the job registry 
    auto job_registry = components.getComponent<IJobRegistry>();
    if (!job_registry)
    {
        RETURN_ERROR_DESCRIPTION(ERR_NO_INTERFACE,
            a_util::strings::format("could not find '%s' in components", IJobRegistry::FEP3_COMP_IID).c_str());
    }

    return removeJobsFromJobRegistry(job_names, *job_registry);
}

/**
* @brief the job Jobs will be added to the job registry of the @p component.
*
* @param job_name the job name 
* @param job shared pointer to a jobpointer
* @param job_config configuration of the job
* @param components The component container in which the job registry will be looked up
* @return fep3::Result
* @retval ERR_NOERROR Everything went fine
* @retval ERR_RESOURCE_IN_USE A job with the given name is already registered
* @retval ERR_NO_INTERFACE The @ref fep3::arya::IJobRegistry was not found within @p components
*/
inline fep3::Result addToComponents(
    const std::string& job_name,
    const std::shared_ptr<fep3::arya::IJob>& job,
    const fep3::arya::JobConfiguration& job_config,
    const IComponents& components)
{
    //do not lock here... this is task of the job registry
    auto job_registry = fep3::getComponent<IJobRegistry>(components);
    if (!job_registry)
    {
        RETURN_ERROR_DESCRIPTION(ERR_NO_INTERFACE,
            a_util::strings::format("could not find '%s' in components", IJobRegistry::FEP3_COMP_IID).c_str());
    }

    return job_registry->addJob(job_name, job, job_config);
}

} // namespace arya

using arya::Job;

/**
 * @brief the job Jobs will be added to the job registry of the @p component.
 *
 * @param job_name the job name
 * @param job shared pointer to a jobpointer
 * @param job_config configuration of the job
 * @param components The component container in which the job registry will be looked up
 * @return fep3::Result
 * @retval ERR_NOERROR Everything went fine
 * @retval ERR_RESOURCE_IN_USE A job with the given name is already registered
 * @retval ERR_NO_INTERFACE The @ref fep3::arya::IJobRegistry was not found within @p components
 */
inline fep3::Result addToComponents(
    const std::string& job_name,
    const std::shared_ptr<fep3::arya::IJob>& job,
    const fep3::arya::JobConfiguration& job_config,
    const IComponents& components)
{
    return arya::addToComponents(job_name, job, job_config, components);
}

/**
 * @brief The job with the given name will be removed from the job registry of the @p components.
 *
 * @param job_name The name of the job to remove
 * @param components The component container in which the job registry will be looked up
 * @return fep3::Result.
 *           If it's an actual error it is the error code for the job that failed last.
 * @retval ERR_NOERROR Everything went fine
 * @retval ERR_NOT_FOUND The job with the given name is not registered
 * @retval ERR_NO_INTERFACE The @ref fep3::arya::IJobRegistry was not found within @p components
 */
inline fep3::Result removeFromComponents(
    const std::string& job_name,
    const IComponents& components)
{
    return arya::removeFromComponents({ job_name }, components);
}

/**
 * @brief Jobs will be added to the job registry of the @p component.
 * If one job can not be added, the function returns not adding the following ones.
 *
 * @param jobs List of jobs to be added
 * @param components The component container in which the job registry will be looked up
 * @return fep3::Result
 * @retval ERR_NOERROR Everything went fine
 * @retval ERR_RESOURCE_IN_USE A job with the given name is already registered
 * @retval ERR_NO_INTERFACE The @ref fep3::arya::IJobRegistry was not found within @p components
 */
inline fep3::Result addToComponents(
    const std::vector<std::shared_ptr<Job>>& jobs,
    const IComponents& components)
{
    return arya::addToComponents(jobs, components);
}

/**
* @brief Jobs will be removed from the job registry of the @p component.
* If one job can not be removed, the function will still try to remove the following ones within @p job_names.
*
* @param job_names List of jobs to be removed
* @param components The component container in which the job registry will be looked up
* @return fep3::Result.
*           If it's an actual error it is the error code for the job that failed last.
*           If more than one remove fails, the error description contains a list of error descriptions separated by ';'.
* @retval ERR_NOERROR Everything went fine
* @retval ERR_NOT_FOUND A job with the given name is not registered
* @retval ERR_NO_INTERFACE The @ref fep3::arya::IJobRegistry was not found within @p components
*/
inline fep3::Result removeFromComponents(
    const std::vector<std::string>& job_names,
    const IComponents& components)
{
    return arya::removeFromComponents(job_names, components);
}

} // namespace core
} // namespace fep3
