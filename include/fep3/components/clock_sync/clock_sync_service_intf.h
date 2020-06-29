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

 /**
  * @brief Name of the clock synchronization entry in configuration service
  *
  */
#define FEP3_CLOCKSYNC_SERVICE_CONFIG "clock_synchronization"

/**
* @brief Name of the property for the set timing master in the clock synchronizattion service
* @see FEP3_CLOCKSYNC_SERVICE_CONFIG
*
*/
#define FEP3_TIMING_MASTER_PROPERTY "timing_master"

/**
 * @brief Full path of the property for the timing master to use
 *
 */
#define FEP3_CLOCKSYNC_SERVICE_CONFIG_TIMING_MASTER FEP3_CLOCKSYNC_SERVICE_CONFIG "/" FEP3_TIMING_MASTER_PROPERTY

 /**
 * @brief Name of the clock of the synchronization service providing as timing slave continuous clock.
 * The clock periodically synchronizes its time with a given timing master.
 * The period after which the synchronization occurs is configurable and defaults to 100 ms.
 * The clock uses the Christian's algorithm to interpolate the time during synchronization steps.
 *
 */
#define FEP3_CLOCK_SLAVE_MASTER_ONDEMAND     "slave_master_on_demand"

/**
* @brief Name of the clock of the synchronization service providing as timing slave discrete clock.
* The clock receives time update events by a timing master which must be a discreate clock. 
* Otherwise it will synchronize in discrete clock steps every given FEP3_CLOCK_SLAVE_SYNC_CYCLE_TIME
* 
*
*/
#define FEP3_CLOCK_SLAVE_MASTER_ONDEMAND_DISCRETE  "slave_master_on_demand_discrete"

/**
* @brief Period at which the clock of the timing slave continuous clock synchronizes with the timing master.
* Only relevant for timing slave configuration if the timing slave's main clock is set to FEP3_CLOCK_SLAVE_MASTER_ONDEMAND.
* The timing slaves's clock cyclically requests the current simulation time from the timing master.
* The duration in ms which has to pass between those time requests is configured by this property.
*
*/
#define FEP3_SLAVE_SYNC_CYCLE_TIME_PROPERTY  "sync_cycle_time_ms"


/**
* @brief Period at which the native timing client continuous clock synchronizes with the timing master. 
* Only relevant for timing client configuration if the timing client's main clock is set to 'slave_master_on_demand'.
* The timing client's slave clock cyclically requests the current simulation time from the timing master.
* The duration which has to pass between those time requests is configured by this property.
*
*/
#define FEP3_CLOCKSYNC_SERVICE_CONFIG_SLAVE_SYNC_CYCLE_TIME FEP3_CLOCKSYNC_SERVICE_CONFIG "/" FEP3_SLAVE_SYNC_CYCLE_TIME_PROPERTY

/**
* @brief Default value of the built-in 'slave_master_on_demand' clock's slave sync cycle time property in millisec
*
*/
#define FEP3_SLAVE_SYNC_CYCLE_TIME_DEFAULT_VALUE 100

namespace fep3
{
namespace arya
{
/**
* @brief Interface of the Clock Sync Service
*
*/
class FEP3_PARTICIPANT_EXPORT IClockSyncService 
{
public:
    /**
    * @brief Defintion of the local clock sync service component ID
    */
    FEP_COMPONENT_IID("clock_sync_service.arya.fep3.iid");

protected:
    /**
    * @brief DTOR
    *
    */
    virtual ~IClockSyncService() = default;
};

} // namespace arya
using arya::IClockSyncService;
} // namespace fep3
