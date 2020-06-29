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

#include <gmock/gmock.h>

#include <fep3/components/base/component_base.h>
#include <fep3/components/job_registry/job_registry_intf.h>

namespace fep3
{

// for "argument dependent lookup" in gtest macros the comparison operator has to be in the
// same namespace as the type of the arguments to be compared
namespace arya
{
bool operator==(const JobConfiguration& lhs, const JobConfiguration& rhs)
{
    return
        (lhs._cycle_sim_time == rhs._cycle_sim_time
        && lhs._delay_sim_time == rhs._delay_sim_time
        && lhs._max_runtime_real_time == rhs._max_runtime_real_time
        && lhs._runtime_violation_strategy == rhs._runtime_violation_strategy
        && lhs._jobs_this_depends_on == rhs._jobs_this_depends_on
        );
}
bool operator==(const JobInfo& lhs, const JobInfo& rhs)
{
    return
        (lhs.getName() == rhs.getName()
        && lhs.getConfig() == rhs.getConfig()
        );
}
} // namespace arya

namespace mock 
{

struct Job : public IJob
{
    ~Job() override = default;
    MOCK_METHOD1(executeDataIn, fep3::Result(Timestamp));
    MOCK_METHOD1(execute, fep3::Result(Timestamp));
    MOCK_METHOD1(executeDataOut, fep3::Result(Timestamp));
};

MATCHER_P(JobsMatcher, other, "Equality matcher for fep3::Jobs")
{
    if(arg.size() != other.size())
    {
        return false;
    }
    else
    {
        auto arg_iter = arg.begin();
        auto other_iter = other.begin();
        for(; arg_iter != arg.end() && other_iter != other.end(); ++arg_iter, ++other_iter)
        {
            const JobEntry& arg_job_entry = arg_iter->second;
            const JobEntry& other_job_entry = other_iter->second;
            const JobInfo& arg_job_info = arg_job_entry.job_info;
            const JobInfo& other_job_info = other_job_entry.job_info;
            if
                (arg_iter->first != other_iter->first
                // note: the pointers to the job are actually differing, because one side is the pointer to the wrapped job
                //|| arg_job_entry.job != other_job_entry.job
                || !(arg_job_info == other_job_info)
                )
            {
                break;
            }
        }
        return (arg_iter == arg.end());
    }
}

} 
}
