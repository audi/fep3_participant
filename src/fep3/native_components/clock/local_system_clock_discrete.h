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

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#include <fep3/components/clock/clock_base.h>
#include <fep3/fep3_duration.h>

namespace fep3
{
namespace native
{

/**
* @brief Base class for the native implementation of a discrete clock.
*/
class DiscreteClockUpdater
{
protected:
    /**
    * CTOR
    */
    DiscreteClockUpdater();

    /**
    * DTOR
    */
    ~DiscreteClockUpdater();

    /**
    * @brief Start the clock updater.
    * Start the thread which updates the clock time and omits time update events to the event sink.
    */
    void startWorking();

    /**
    * @brief Stop the clock updater.
    */
    void stopWorking();

    /**
    * @brief Update the clock time.
    * Has to be implemented by the clock.
    */
    virtual void updateTime(Timestamp new_time) = 0;

private:
    /**
    * @brief Cyclically wait for the configured clock cycle time and update the clock time.
    */
    void work();

    /// Current simulation time
    Timestamp                                           _simulation_time;
    /// Time point of the next discrete time step
    std::chrono::time_point<std::chrono::steady_clock>  _next_request_gettime;
    /// Duration of a single discrete time step in nanoseconds
    Duration                                           _cycle_time;
    /// Factor to control the relation between simulated time and system time
    double                                              _time_factor;

    /// Thread to update the clock time
    std::thread                                         _worker;
    std::atomic_bool                                    _stop;

    std::mutex                                          _clock_updater_mutex;
    std::condition_variable                             _cycle_wait_condition;

public:
    /**
    * @brief Update the clock configuration.
    *
    * @param cycle_time new clock cycle time in nanoseconds
    * @param time_factor new clock time factor
    */
    void updateConfiguration(Duration cycle_time,
                             double time_factor);
};

/**
* @brief Native implementation of a discrete clock.
*/
class LocalSystemSimClock : public DiscreteClockUpdater,
                            public base::DiscreteClock
{
public:
    /**
    * CTOR
    */
    LocalSystemSimClock();

    /**
    * DTOR
    */
    ~LocalSystemSimClock() = default;

    /**
    * @brief Start the clock.
    */
    void start(const std::weak_ptr<IEventSink>& _sink) override;

    /**
    * @brief Stop the clock.
    */
    void stop() override;

    /**
    * @brief Update the clock time.
    *
    * @param new_time new clock time in nanoseconds
    */
    void updateTime(Timestamp new_time) override;
};

} // namespace native
} // namespace fep3
