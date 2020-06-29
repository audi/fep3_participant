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

#include <list>

#include "job_intf.h"
#include "job_info.h"
#include <fep3/components/base/component_iid.h>

/**
* @brief Main property entry of the job registry properties
*/
#define FEP3_JOB_REGISTRY_CONFIG "job_registry"
/**
* @brief The job timing configuration property name
* Use this to set the timing configuration by path from inside the job registry configuration node.
*
*/
#define FEP3_TIMING_CONFIGURATION_PROPERTY "timing_configuration"
/**
* @brief The job timing configuration property node
* Use this to set the timing configuration by path.
*/
#define FEP3_JOB_REGISTRY_TIMING_CONFIGURATION FEP3_JOB_REGISTRY_CONFIG "/" FEP3_TIMING_CONFIGURATION_PROPERTY

namespace fep3
{
namespace arya
{

/**
* @brief Interface of the job registry
*
* The job registry may be used to register jobs.
* Registered jobs may be triggered by the active scheduler.
*/
class FEP3_PARTICIPANT_EXPORT IJobRegistry
{
public:
    /// The component interface identifier of IJobRegistry
    FEP_COMPONENT_IID("job_registry.arya.fep3.iid");

protected:
    /**
    * @brief DTOR
    *
    */
    ~IJobRegistry() = default;

public:
    /**
    * @brief Register the given @p job at the job registry.
    * 
    * @param name The name of the job to be registered
    * @param job The job to be registered
    * @param job_config The job configuration of the job to be registered
    * 
    * @return fep3::Result
    * @retval ERR_NOERROR Everything went fine
    * @retval ERR_RESOURCE_IN_USE A job with the given name is already registered
    */
    virtual fep3::Result addJob(const std::string& name, const std::shared_ptr<IJob>& job, const JobConfiguration& job_config) = 0;

    /**
    * @brief Unregister the job with the given @p name from the job registry.
    * 
    * @param name The name of the job to be removed
    * @return fep3::Result
    * @retval ERR_NOERROR Everything went fine
    * @retval ERR_NOT_FOUND A job with the given @p name is not registered
    */
    virtual fep3::Result removeJob(const std::string& name) = 0;

    /**
    * Return the job infos of all registered jobs.
    * @return List containing job infos of all registered jobs
    */
    virtual std::list<JobInfo> getJobInfos() const = 0;

    /**
     * @brief Get all registered jobs.
     * 
     * @return All registered Jobs
     */
    virtual Jobs getJobs() const = 0;    
};

} // namespace arya
using arya::IJobRegistry;
} // namespace fep3
