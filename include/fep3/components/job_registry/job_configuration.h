/**
 * Declaration of the class JobConfiguration.
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
#include <vector>

#include <fep3/fep3_duration.h>
#include <fep3/fep3_optional.h>


namespace fep3
{
namespace arya
{

/**
* @brief Configuration of a job
*
*/
class JobConfiguration
{
public:
    /// Strategy enum configuring behaviour in case of an operational time violation
    enum class TimeViolationStrategy
    {
        /// dummy value
        unknown = 0,
        /// Time violations are ignored
        ignore_runtime_violation,
        /// A warning incident will be published when an operational time violation is detected
        warn_about_runtime_violation,
        /// Configured output samples will not be published when an operational time violation is detected
        skip_output_publish,
        /// The job will abort and set the participant to error state
        set_stm_to_error
    };

public:
    /**
    * @brief CTOR
    * 
    * @param cycle_sim_time The cycle time to be used for the job (simulation time)
    * @param first_delay_sim_time The cycle delay time to the 0 point of the time base (simulation time)
    * @param max_runtime_real_time The maximum duration that a single job execution is expected to need for computation (real time).
    *                                   Provide no value if you have no expectations on the jobs runtime.
    * @param runtime_violation_strategy The violation strategy
    * @param jobs_this_depends_on The jobs (by name), this job depends on
    */
    JobConfiguration(Duration cycle_sim_time,
                     Duration first_delay_sim_time = Duration(0),
                     Optional<Duration> max_runtime_real_time = {},
                     TimeViolationStrategy runtime_violation_strategy = TimeViolationStrategy::ignore_runtime_violation,
                     std::vector<std::string> jobs_this_depends_on = {})
        : _cycle_sim_time(cycle_sim_time)
        , _delay_sim_time(first_delay_sim_time)
        , _max_runtime_real_time(std::move(max_runtime_real_time))
        , _runtime_violation_strategy(runtime_violation_strategy)
        , _jobs_this_depends_on(std::move(jobs_this_depends_on))
    {
    }

    /**
    * @brief Return a time violation strategy for a given string.
    *
    * The string parameter must match one of the time violation strategy names.
    * In case of no match, the unknown strategy is returned.
    *
    * @param strategy_string The string to derive a time violation strategy from.
    *
    * @return Tiolation strategy matching the provided string.
    */
    static TimeViolationStrategy timeViolationStrategyFromString(const std::string& strategy_string)
    {
        if ("ignore_runtime_violation" == strategy_string)
        {
            return TimeViolationStrategy::ignore_runtime_violation;
        }
        else if ("warn_about_runtime_violation" == strategy_string)
        {
            return TimeViolationStrategy::warn_about_runtime_violation;
        }
        else if ("skip_output_publish" == strategy_string)
        {
            return TimeViolationStrategy::skip_output_publish;
        }
        else if ("set_stm_to_error" == strategy_string)
        {
            return TimeViolationStrategy::set_stm_to_error;
        }
        else
        {
            return TimeViolationStrategy::unknown;
        }
    }

public:
    /**
     * @brief Return the configured time violation strategy as string.
     *
     * @return The configured time violation strategy as std::string.
     */
    std::string timeViolationStrategyAsString() const
    {
        switch (_runtime_violation_strategy)
        {
        case TimeViolationStrategy::ignore_runtime_violation:
            return "ignore_runtime_violation";
        case TimeViolationStrategy::warn_about_runtime_violation:
            return "warn_about_runtime_violation";
        case TimeViolationStrategy::skip_output_publish:
            return "skip_output_publish";
        case TimeViolationStrategy::set_stm_to_error:
            return "set_stm_to_error";
        default:
            return "unknown";
        }
    }

public:
    /// The cycle time to be used for the job (simulation time)
    Duration                         _cycle_sim_time;
    /// The cycle delay time to the 0 point of the time base (simulation time)
    Duration                         _delay_sim_time;
    /// The maximum duration that a single job execution is expected to need for computation (real time)
    Optional<Duration>               _max_runtime_real_time;
    /// The strategy that will be applied in case of a longer computation time than expected
    TimeViolationStrategy	         _runtime_violation_strategy;
    /// list of jobs (by name), this job depends on
    std::vector<std::string>         _jobs_this_depends_on;
};

} // namespace arya
using arya::JobConfiguration;
} // namespace fep3