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

#include <fep3/fep3_errors.h>
#include <fep3/fep3_duration.h>
#include <fep3/fep3_optional.h>
#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/components/job_registry/job_configuration.h>
#include <fep3/components/job_registry/job_registry_intf.h>

namespace fep3
{  
namespace native
{

class JobRunner
{
public:
    JobRunner(const std::string& name,
                    const fep3::JobConfiguration::TimeViolationStrategy& time_violation_strategy,
                    const fep3::Optional<fep3::Duration>& max_runtime,
                    const std::shared_ptr<const fep3::ILoggingService::ILogger>& logger,
                    const std::function<fep3::Result()>& set_participant_to_error_state);

    fep3::Result runJob(const Timestamp trigger_time, fep3::IJob& job);

private:
    fep3::Result applyTimeViolationStrategy(const Timestamp process_duration);
    fep3::Result emitErrorStateChange();

private:
    const std::string _name;
    fep3::JobConfiguration::TimeViolationStrategy _time_violation_strategy;
    fep3::Optional<fep3::Duration> _max_runtime;
    std::shared_ptr<const fep3::ILoggingService::ILogger> _logger;
    std::function<fep3::Result()> _set_participant_to_error_state;
    
    bool _cancelled;
    bool _skip_output;
};

} // namespace native
} // namespace fep3


