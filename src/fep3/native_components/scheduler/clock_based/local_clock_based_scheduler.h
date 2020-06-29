/**
* Scheduler based on local clock
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

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <list>
#include <memory>
#include <mutex>
#include <stdint.h>
#include <string>
#include <chrono>
#include <thread>

#include "timer_scheduler_impl.h"
#include <fep3/native_components/scheduler/job_runner.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/components/job_registry/job_configuration.h>

namespace fep3
{
namespace native
{

static fep3::Timestamp reset_time = fep3::Timestamp(-1);

class ServiceThread
{   
public:
    ServiceThread(const std::string& name,
        fep3::IJob& job,
        fep3::IClockService& clock,
        uint32_t flags);

    ServiceThread(ServiceThread&&) = delete;
    ServiceThread(const ServiceThread&) = delete;
    virtual ~ServiceThread();
    void operator()();

    virtual fep3::Result execute(Timestamp first_wakeup_time);
    fep3::Result join(Timestamp timeout = Timestamp(-1));
    fep3::Result start();
    fep3::Result detach();

    bool isCurrent() const;
    std::string getName() const;

protected:
    bool joinable() const;

protected:
    std::string _name;
    fep3::IJob& _runnable;
    fep3::IClockService& _clock;

private:
    std::thread _system_thread;
    std::future<void>* _exited_future = nullptr;
    std::promise<void>* _exited_promise = nullptr;
    uint32_t _flags;
    std::mutex _mutex_thread;
    bool _thread_cancelled = false;
};


class TimerThread : public ServiceThread, public ITimer
{
public:
    TimerThread(const std::string& name,
        fep3::IJob& runnable,
        fep3::IClockService& clock,
        Duration period,
        Duration initial_delay,
        TimerScheduler& timer_scheduler,
        const fep3::native::JobRunner& job_runner);

    ~TimerThread();
    /**
     * @brief This method will reapeaditly wait until woken up by method wakeUp 
     */
    fep3::Result execute(Timestamp wakeup_time) override;
    /**
     * @brief Wakes up our timer.Will be called by TimerScheduler::processSchedulerQueueAsynchron.
     * Whenever it's woken up we execute the job using the _job_runner    
     */
    fep3::Result wakeUp(Timestamp wakeup_time, std::promise<void>* finished = nullptr) override;
    fep3::Result start();
    fep3::Result stop();
    fep3::Result remove();
    fep3::Result reset() override;

private:
    // accessors are needed because of volatile (see https://stackoverflow.com/questions/52616666/volatile-objects-of-stdchronoduration-type)
    Timestamp getWakeUpTime() const;
    void setWakeUpTime(Timestamp wakeup_time);

private:
    Duration _period;
    Duration _initial_delay;
    std::mutex _mutex_manual_event;
    std::condition_variable _cv_manual_event;
    volatile bool _manual_event_occured = false;
#ifndef __QNX__
    std::atomic<bool> _cancelled;
#else
    std::atomic_bool _cancelled;
#endif
    std::promise<void>* _finished_promise = nullptr;
    volatile Timestamp::rep _wakeup_time{ -1 };
    volatile Timestamp::rep _last_call_time{ -1 };  
    TimerScheduler& _timer_scheduler;
    fep3::native::JobRunner _job_runner;
};


class LocalClockBasedScheduler : public fep3::IScheduler
{
public:
    LocalClockBasedScheduler(
        const std::shared_ptr<const fep3::ILoggingService::ILogger>& logger,
        const std::function<fep3::Result()>& set_participant_to_error_state);
    ~LocalClockBasedScheduler() = default;

public:
    std::string getName() const override;

    fep3::Result initialize(fep3::IClockService& clock, const fep3::Jobs& jobs) override;
    fep3::Result start() override;
    fep3::Result stop() override;
    fep3::Result deinitialize() override; 

private:
    std::shared_ptr<fep3::native::TimerThread> createTimerThread(
        const fep3::JobEntry& job_info,
        fep3::IClockService& clock);

    fep3::Result addTimerThreadToScheduler(
        const fep3::JobEntry & job_entry,
         std::shared_ptr<fep3::native::TimerThread>);

private:
    std::unique_ptr<ServiceThread> _service_thread;
    std::shared_ptr<TimerScheduler> _timer_scheduler;
    std::list<std::shared_ptr<TimerThread>> _timers;
    std::shared_ptr<const fep3::ILoggingService::ILogger> _logger;
    std::function<fep3::Result()> _set_participant_to_error_state;
    fep3::IClockService* _clock = nullptr;
};

} // namespace native
} // namespace fep3

