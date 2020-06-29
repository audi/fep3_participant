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

#include <string>

#include <fep3/fep3_errors.h>
#include <fep3/components/job_registry/job_configuration.h>
#include <fep3/fep3_participant_export.h>

namespace fep3
{
namespace arya
{
/**
* @brief JobInfo contains the name of a job and the corresponding job configuration.
*
* Usually JobInfos are used to retrieve information regarding jobs registered at the job registry.
*/
class FEP3_PARTICIPANT_EXPORT JobInfo final
{
public:
    /**
    * @brief CTOR
    *
    * @param name Name of the job
    * @param cycle_time Cycle time of the job
    */
    inline JobInfo(const std::string& name, Duration cycle_time);

    /**
    * @brief CTOR
    *
    * @param name Name of the job
    * @param configuration Configuration of the job
    */
    inline JobInfo(const std::string& name, const fep3::JobConfiguration& configuration);

    /**
    * @brief DTOR
    */
    ~JobInfo() = default;

    /**
    * @brief Get the Name of the Job
    *
    * @return The name of the Job
    */
    inline std::string getName() const;

    /**
    * @brief Get the JobConfiguration 
    *
    * @return @ref fep3::arya::JobConfiguration
    */
    inline JobConfiguration getConfig() const;

    /**
    * @brief Reconfigure JobInfo by replacing its JobConfiguration.
    * @param configuration The new JobConfiguration to replace the existing one.
    *
    * @return fep3::Result
    */
    Result reconfigure(const JobConfiguration& configuration)
    {
        _configuration = configuration;

        return {};
    }

private:
    std::string _name;
    JobConfiguration _configuration;
};

JobInfo::JobInfo(const std::string& name, Duration cycle_time) 
    : _name(name)
    , _configuration(cycle_time)
{
}

JobInfo::JobInfo(const std::string& name, const JobConfiguration& configuration) 
    : _name(name)
    , _configuration(std::move(configuration))
{
}

std::string JobInfo::getName() const
{
    return _name;
}

JobConfiguration JobInfo::getConfig() const
{
    return _configuration;
}

} // namespace arya	
using arya::JobInfo;
} // namespace fep3