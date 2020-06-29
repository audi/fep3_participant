/**
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

#include <fep3/fep3_optional.h>
#include <fep3/fep3_errors.h>
#include <fep3/components/base/component_iid.h>
#include <fep3/components/clock/clock_registry_intf.h>

/**
* @brief The clock service main property tree entry node
*
*/
#define FEP3_CLOCK_SERVICE_CONFIG "clock"

/**
* @brief The mainclock configuration property name
* Use this to set the main clock by configuration.
*
*/
#define FEP3_MAIN_CLOCK_PROPERTY "main_clock"
/**
* @brief The clock service mainclock configuration node
* Use this to set the main clock by configuration.
*
*/
#define FEP3_CLOCK_SERVICE_MAIN_CLOCK FEP3_CLOCK_SERVICE_CONFIG "/" FEP3_MAIN_CLOCK_PROPERTY

/**
* @brief Cycle time of the built-in discrete simulation time clock which defines the length of a discrete time step in nanoseconds.
*
*/
#define FEP3_CLOCK_SIM_TIME_CYCLE_TIME_PROPERTY "cycle_time_ms"

/**
* @brief Cycle time of the built-in discrete simulation time clock which defines the length of a discrete time step in nanoseconds.
*
*/
#define FEP3_CLOCK_SERVICE_CLOCK_SIM_TIME_CYCLE_TIME FEP3_CLOCK_SERVICE_CONFIG "/" FEP3_CLOCK_SIM_TIME_CYCLE_TIME_PROPERTY
/**
* @brief Minimum value of the built-in 'discrete simulation time clock' cycle time property in milliseconds
*
*/
#define FEP3_CLOCK_SIM_TIME_CYCLE_TIME_MIN_VALUE 1
/**
* @brief Default value of the built-in 'discrete simulation time clock' cycle time property in milliseconds
*
*/
#define FEP3_CLOCK_SIM_TIME_CYCLE_TIME_DEFAULT_VALUE 100

/**
* @brief Factor at which discrete time steps of the built-in discrete simulation time clock pass compared to the system time.
* A time factor of 2 means the discrete time step passes twice as fast compared to the system time.
* A time factor of 0,0 means no delay exists between discrete time steps.
*
*/
#define FEP3_CLOCK_SIM_TIME_TIME_FACTOR_PROPERTY "time_factor"

/**
* @brief Factor at which discrete time steps of the built-in discrete simulation time clock pass compared to the system time.
* A time factor of 2 means the discrete time step passes twice as fast compared to the system time.
* A time factor of 0,0 means no delay exists between discrete time steps.
*
*/
#define FEP3_CLOCK_SERVICE_CLOCK_SIM_TIME_TIME_FACTOR FEP3_CLOCK_SERVICE_CONFIG "/" FEP3_CLOCK_SIM_TIME_TIME_FACTOR_PROPERTY
/**
* @brief Minimum value of the built-in 'discrete simulation time clock' time factor property.
*
*/
#define FEP3_CLOCK_SIM_TIME_TIME_FACTOR_MIN_VALUE 0.1
/**
* @brief Value to configure the built-in 'discrete simulation time clock' time factor property to run in
* 'As Fast As Possible' mode.
*
*/
#define FEP3_CLOCK_SIM_TIME_TIME_FACTOR_AFAP_VALUE 0.0
/**
* @brief Default value of the built-in 'discrete simulation time clock' time factor property.
*
*/
#define FEP3_CLOCK_SIM_TIME_TIME_FACTOR_DEFAULT_VALUE 1.0
/**
* @brief Name of the clock service built-in clock to retrieve the current system time (continuous clock).
* @see @ref FEP3_CLOCK_SERVICE_MAIN_CLOCK
*/
#define FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME     "local_system_realtime"

/**
* @brief Name of the clock service built-in clock to retrieve a simulated time (discrete clock).
* The discrete clock may be configured using @ref FEP3_CLOCK_SIM_TIME_CYCLE_TIME_PROPERTY and @ref FEP3_CLOCK_SIM_TIME_TIME_FACTOR_PROPERTY.
* @see @ref FEP3_CLOCK_SERVICE_MAIN_CLOCK
* 
*/
#define FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME      "local_system_simtime"

/**
* @brief Timeout for sending time update events like timeUpdating, timeReset, etc. to timing slaves.
* The timeout is applied per slave and update function.
*
*/
#define FEP3_TIME_UPDATE_TIMEOUT_PROPERTY "time_update_timeout_ms"
/**
* @brief Timeout for sending time update events like timeUpdating, timeReset, etc. to timing slaves.
* The timeout is applied per slave and update function.
*
*/
#define FEP3_CLOCK_SERVICE_TIME_UPDATE_TIMEOUT FEP3_CLOCK_SERVICE_CONFIG "/" FEP3_TIME_UPDATE_TIMEOUT_PROPERTY
/**
* @brief Default value of the timeout for sending time update events in namespace.
*
*/
#define FEP3_TIME_UPDATE_TIMEOUT_DEFAULT_VALUE 5000

namespace fep3
{
namespace arya
{

/**
* @brief Interface of the clock service
* 
* The clock service may be used to register custom clocks and set the active main clock for the participant.
*/
class FEP3_PARTICIPANT_EXPORT IClockService : public IClockRegistry
{
public:
    /**
     * @brief Defintion of the clock service component IID.
     */
    FEP_COMPONENT_IID("clock_service.arya.fep3.iid")

protected:
    /**
     * @brief DTOR
     */
    ~IClockService() override = default;

public:
    /**
     * @brief Return the time of the current main clock.
     * 
     * @return Current time of the main clock
     */
    virtual arya::Timestamp getTime() const = 0;

    /**
     * @brief Return the time of the clock with name @p clock_name.
     * @see @ref IClockRegistry::registerClock, @ref IClock::getName
     * 
     * @param clock_name The name of the clock
     * @return The current time of the clock or no value if no clock with name @p clock_name exists
     */
    virtual arya::Optional<arya::Timestamp> getTime(const std::string& clock_name) const = 0;

    /**
     * @brief Return the clock type of the current main clock.
     * 
     * @return The type of the clock
     */
    virtual IClock::ClockType getType() const = 0;

    /**
     * @brief Return the type of the clock with the name @p clock_name.
     * 
     * @param clock_name The name of the clock
     * @return The type of the clock or no value if no clock with @p clock_name exists
     */
    virtual arya::Optional<IClock::ClockType> getType(const std::string& clock_name) const = 0;

    /**
     * @brief Get the name of the current main clock.
     * 
     * @return Name of the current main clock or "" if no main clock is currently set
     */
    virtual std::string getMainClockName() const= 0;

    /**
     * @brief Register an event sink to receive time events.
     *
     * @param clock_event_sink The event sink to register
     * @return fep3::Result
     * @retval ERR_POINTER        The @p clock_event_sink weak_ptr is expired.
     */
    virtual fep3::Result registerEventSink(const std::weak_ptr<IClock::IEventSink>& clock_event_sink) = 0;

    /**
     * @brief Unregister the event sink.
     * 
     * @param clock_event_sink The event sink to unregister
     * @return fep3::Result
     * @retval ERR_POINTER        The @p clock_event_sink weak_ptr is expired.
     */
    virtual fep3::Result unregisterEventSink(const std::weak_ptr<IClock::IEventSink>& clock_event_sink) = 0;
};

} // namespace arya
using arya::IClockService;
} // namespace fep3
