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
 
#include "job_registry_impl.h"

#include <algorithm>
#include <iterator>

namespace fep3
{
namespace native
{

fep3::Result JobRegistryImpl::addJob(const std::string& name, const std::shared_ptr<fep3::IJob>& job, const JobConfiguration& job_config)
{
    if (_jobs.count(name) > 0)
    {
        RETURN_ERROR_DESCRIPTION(ERR_RESOURCE_IN_USE,
            a_util::strings::format(
                "Adding job to job registry failed. A job with the name '%s' already exists.",
                name.c_str())
            .c_str());
    }
    _jobs.emplace(name, fep3::JobEntry{job, fep3::JobInfo(name, job_config)});
            
    return {};
}

fep3::Result JobRegistryImpl::removeJob(const std::string& name)
{
    if (_jobs.count(name) == 0)
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND,
            a_util::strings::format(
                "Removing job from job registry failed. A job with the name '%s' does not exist.",
                name.c_str())
            .c_str());
    }
    _jobs.erase(name);

    return {};
}

std::list<JobInfo> JobRegistryImpl::getJobInfos() const
{
    std::list<fep3::JobInfo> job_list;
    std::transform(_jobs.begin(), _jobs.end(), 
        std::back_inserter(job_list), 
        [](auto map_entry){return map_entry.second.job_info;});    
 
    return job_list;
}

fep3::Jobs JobRegistryImpl::getJobs() const
{
    return _jobs;    
}

fep3::Result JobRegistryImpl::configureJobsByTimingConfiguration(
    const std::string& participant_name,
    const timing_configuration::TimingConfiguration& timing_configuration)
{
    return timing_configuration::configureJobsByTimingConfiguration(_jobs,
        participant_name,
        timing_configuration);
}

} // namespace native
} // namespace fep3