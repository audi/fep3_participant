/**
* Kernel timer scheduler
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
#include <list>
#include <mutex>
#include <future>

#include <fep3/fep3_duration.h>
#include <fep3/components/scheduler/scheduler_service_intf.h>
#include <fep3/components/clock/clock_intf.h>

namespace fep3
{
namespace native
{


class ITimer
{
public:
    virtual fep3::Result wakeUp(Timestamp wakeup_time, std::promise<void>* finished = nullptr) = 0;
    virtual fep3::Result reset() = 0;
};

class TimerScheduler : 
    public fep3::IClock::IEventSink,
    public fep3::IJob,
    public std::enable_shared_from_this<TimerScheduler>
{
private:
    struct TimerInfo
    {
        ITimer*   _timer;
        Timestamp _next_instant;
        Duration _period;

        bool operator<(const TimerInfo& other) const
        {
            return _next_instant < other._next_instant;
        }
    };           

public:       
    explicit TimerScheduler(fep3::IClockService& clock);
    TimerScheduler() = delete;

    virtual ~TimerScheduler();      

    fep3::Result addTimer(ITimer& timer, Duration period, Duration initial_delay);
    fep3::Result removeTimer(ITimer& timer);
    fep3::Result start();
    fep3::Result stop();   

private:        
    void processSchedulerQueueSynchron(Timestamp current_time, fep3::Optional<Duration>& time_to_wait);
    void processSchedulerQueueAsynchron(Timestamp current_time, fep3::Optional<Duration>& time_to_wait);
    Timestamp getTime() const;
    IClock::ClockType getClockType() const;
    void initBlockSchedulingStart();

    // IClock::IEventSink
    void timeUpdateBegin(Timestamp old_time, Timestamp new_time) override;
    void timeUpdating(Timestamp  new_time) override;
    void timeUpdateEnd(Timestamp new_time) override;
    void timeResetBegin(Timestamp old_time, Timestamp new_time) override;
    void timeResetEnd(Timestamp new_time) override;

    // IJob
    fep3::Result executeDataIn(Timestamp /*time_of_execution*/) override { return {}; }
    fep3::Result execute(Timestamp  time_of_execution) override;
    fep3::Result executeDataOut(Timestamp  /*time_of_execution*/) override { return {}; }

private:
    std::list<TimerInfo> _timers;
    std::mutex _mutex_timer;
    std::mutex _mutex_processing_lock;
	std::mutex _mutex_processing_trigger;
	std::mutex _mutex_start_stop_update;
	std::condition_variable _cv_trigger_event;
    fep3::IClockService* _clock;
    fep3::Optional<Timestamp> _startup_reset_time;

#ifndef __QNX__
    std::atomic<bool> _cancelled;
    std::atomic<bool> _started;
    std::atomic<bool> _block_scheduling_start;
#else
    std::atomic_bool  _cancelled;
    std::atomic_bool _started;
    std::atomic_bool _block_scheduling_start;
#endif
};

}
}