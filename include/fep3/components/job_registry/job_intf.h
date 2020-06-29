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

#include <memory>
#include <map>

#include <fep3/fep3_errors.h>
#include "job_info.h"
#include "fep3/fep3_timestamp.h"
#include <fep3/fep3_participant_export.h>

namespace fep3
{
namespace arya
{

/**
 * @brief Interface of a job
 */
class FEP3_PARTICIPANT_EXPORT IJob
{
public:
    /**
     * @brief DTOR
     */
    virtual ~IJob() = default;

public:
    /**
     * @brief Read input samples.
     *
     * @param time_of_execution Current simulation time
     * @return fep3::Result
     */
    virtual fep3::Result executeDataIn(Timestamp time_of_execution) = 0;

    /**
     * @brief Process job.
     *
     * @param time_of_execution Current simulation time
     * @return fep3::Result
     */
    virtual fep3::Result execute(Timestamp time_of_execution) = 0;

    /**
     * @brief Write output samples.
     *
     * @param time_of_execution Current simulation time
     * @return fep3::Result
     */
    virtual fep3::Result executeDataOut(Timestamp time_of_execution) = 0;
};

/// Entry of Jobs containing the @ref IJob and the @ref JobInfo
struct JobEntry
{
    /// The job itself
    std::shared_ptr<IJob> job;
    /// Info object about the job
    JobInfo job_info;
};
/// map of job entries
using Jobs = std::map<std::string, JobEntry>;

} // namespace arya
using arya::Jobs;
using arya::JobEntry;
using arya::IJob;
} // namespace fep3
