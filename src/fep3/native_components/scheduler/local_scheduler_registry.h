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

#include "clock_based/local_clock_based_scheduler.h"
#include <fep3/fep3_errors.h>
#include <fep3/components/scheduler/scheduler_service_intf.h>

namespace fep3
{

class IIncidentHandler;

namespace native
{

class LocalSchedulerRegistry : public ISchedulerRegistry
{
public:
    LocalSchedulerRegistry(std::unique_ptr<fep3::IScheduler> default_scheduler);    
    LocalSchedulerRegistry() = delete;

public: 
    // ISchedulerRegistry
    fep3::Result registerScheduler(std::unique_ptr<fep3::IScheduler> scheduler) override;
    fep3::Result unregisterScheduler(const std::string& scheduler_name) override;
    std::list<std::string> getSchedulerNames() const override;    

    fep3::Result setActiveScheduler(const std::string& scheduler_name);        
    std::string getActiveSchedulerName() const;
    std::string getDefaultSchedulerName() const;

    fep3::Result initializeActiveScheduler(fep3::IClockService& clock,
        const fep3::Jobs& jobs) const;
    fep3::Result deinitializeActiveScheduler() const;
    fep3::Result startActiveScheduler() const;
    fep3::Result stopActiveScheduler() const;
    
private:
    fep3::IScheduler* getActiveScheduler() const;
    fep3::IScheduler* findScheduler(const std::string& scheduler_name) const;
    fep3::Result activateDefaultScheduler();

private:
    std::string _default_scheduler_name;
    std::string _active_scheduler;    
    std::list<std::unique_ptr<fep3::IScheduler>> _schedulers;    
};

} // namespace native
} // namespace fep3