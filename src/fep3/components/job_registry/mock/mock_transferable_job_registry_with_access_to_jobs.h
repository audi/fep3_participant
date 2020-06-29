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

#include "mock_job_registry.h"
#include <fep3/components/base/c_access_wrapper/transferable_component_base.h>

namespace fep3
{
namespace mock
{

struct TransferableJobRegistryWithAccessToJobs
    : public JobRegistryComponent<fep3::plugin::c::TransferableComponentBase>
{
private:
    fep3::Result addJob(const std::string& name, const std::shared_ptr<IJob>& job, const JobConfiguration& job_config) override
    {
        _jobs.emplace(name, fep3::JobEntry{job, fep3::JobInfo(name, job_config)});
        // call the mocked method to enable setting of expectations
        return JobRegistryComponent<fep3::plugin::c::TransferableComponentBase>::addJob(name, job, job_config);
    }
    
public:
    std::list<::fep3::IJob*> getJobPointers() const
    {
        std::list<::fep3::IJob*> jobs;
        for(const auto& job_entry : _jobs)
        {
            jobs.push_back(job_entry.second.job.get());
        }
        return jobs;
    }
private:
    fep3::Jobs _jobs;
};

} 
}
