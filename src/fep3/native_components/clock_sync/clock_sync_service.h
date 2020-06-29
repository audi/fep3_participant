/**
 * @file
 * Copyright &copy; Audi AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <memory>

#include <fep3/components/base/component_base.h>
#include <fep3/components/clock/clock_base.h>
#include <fep3/components/clock_sync/clock_sync_service_intf.h>
#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/components/configuration/propertynode.h>

#include "fep3/fep3_duration.h"
#include "master_on_demand_clock_client.h"

namespace fep3
{
namespace native
{

/**
* @brief Configuration for the LocalClockService
*/
struct ClockSyncServiceConfiguration : public Configuration
{
    ClockSyncServiceConfiguration();
    ~ClockSyncServiceConfiguration() = default;

    fep3::Result registerPropertyVariables() override;
    fep3::Result unregisterPropertyVariables() override;
    std::pair<bool, fep3::Result>  validateConfiguration(const std::string& main_clock_name,
        const ILoggingService::ILogger& logger) const;

    PropertyVariable<std::string> _timing_master_name{ "" };
    PropertyVariable<int32_t>     _slave_sync_cycle_time{ FEP3_SLAVE_SYNC_CYCLE_TIME_DEFAULT_VALUE };
};

/**
* @brief Native implementation of a clock sync service.
*/
class ClockSynchronizationService
    : public ComponentBase<IClockSyncService>
{
public:
    /**
    * CTOR
    */
    ClockSynchronizationService() = default;

    /**
    * DTOR
    */
    ~ClockSynchronizationService() = default;

public:// inherited via ComponentBase
    fep3::Result create() override;
    fep3::Result destroy() override;
    fep3::Result initialize() override;   
    fep3::Result deinitialize() override;
    fep3::Result tense() override;
    fep3::Result start() override;
    fep3::Result stop() override;


private:
    fep3::Result setupLogger(const IComponents& components);
    fep3::Result setupSlaveClock(
        const IComponents& components,
        const std::string& main_clock_name);

    fep3::Result logError(const fep3::Result& error) const;
    fep3::Result logError(const std::string& message) const;

private:  
    std::shared_ptr<const ILoggingService::ILogger>                                 _logger;
    std::pair<std::shared_ptr<fep3::base::ClockBase>, fep3::rpc::FarClockUpdater*> _slave_clock;

    ClockSyncServiceConfiguration _configuration;
};

} // namespace native
} // namespace fep3