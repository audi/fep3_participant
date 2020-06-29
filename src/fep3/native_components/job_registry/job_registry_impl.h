/**
 * Declaration of the native scheduler registry implementation.
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
#include <memory>
#include <string>

#include <fep3/fep3_errors.h>
#include <fep3/components/job_registry/job_registry_intf.h>
#include <fep3/components/scheduler/scheduler_service_intf.h>
#include <fep3/native_components/job_registry/timing_configuration.h>


namespace fep3
{
namespace native
{
    class JobRegistryImpl : public fep3::IJobRegistry
{
public:
    // Inherited via IJobRegistry
    fep3::Result addJob(const std::string & name, const std::shared_ptr<IJob>& job, const JobConfiguration & job_config) override;
    fep3::Result removeJob(const std::string & name) override;    
    std::list<JobInfo> getJobInfos() const override;
    fep3::Jobs getJobs() const override;
    fep3::Result configureJobsByTimingConfiguration(
        const std::string& participant_name,
        const timing_configuration::TimingConfiguration& timing_configuration);

private:
    Jobs _jobs;
};

} // namespace native
} // namespace fep3