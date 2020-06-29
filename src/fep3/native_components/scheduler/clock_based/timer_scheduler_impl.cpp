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

#include "timer_scheduler_impl.h"

#include <cassert>
#include <stddef.h>
#include <chrono>
#include <mutex>
#include <thread>
#include <cassert>

namespace fep3
{
namespace native
{

TimerScheduler::TimerScheduler(fep3::IClockService& clock)
    : _clock(&clock)
    , _cancelled(false)
    , _started(false)
   
    , _startup_reset_time(fep3::Optional<Timestamp>())    
{       
    initBlockSchedulingStart();
}



TimerScheduler::~TimerScheduler()
{
    stop();   
}

void TimerScheduler::initBlockSchedulingStart()
{
	_block_scheduling_start = IClock::ClockType::continuous == getClockType();
}

fep3::Result TimerScheduler::addTimer(ITimer& timer, Duration period, Duration initial_delay)
{
    std::unique_lock<std::mutex> lock(_mutex_timer);

    _timers.push_back({&timer, getTime() + initial_delay, period});
    _cv_trigger_event.notify_all();
    return{};
}

fep3::Result TimerScheduler::removeTimer(ITimer& oTimer)
{
    std::unique_lock<std::mutex> lock(_mutex_timer);
    for (auto it = _timers.begin();
         it != _timers.end(); ++it)
    {
        if (it->_timer == &oTimer)
        {
            _timers.erase(it);
            return{};
        }
    }

    RETURN_ERROR_DESCRIPTION(fep3::ERR_NOT_FOUND, "Timer not found");
}


fep3::Result TimerScheduler::start()
{   
    _cancelled = false;

     if (_startup_reset_time.has_value() && IClock::ClockType::discrete == getClockType())
    {
        fep3::Optional<Duration> time_to_wait;     
        processSchedulerQueueSynchron(_startup_reset_time.value(), time_to_wait);
    }

    if(IClock::ClockType::continuous !=  getClockType())
    {
        // continuous clock will be started later on first reset event
        _started = true;
    }
   
    return {};
}

fep3::Result TimerScheduler::stop()
{
	std::lock_guard<std::mutex> lock_guard_stop(_mutex_start_stop_update);
	
    initBlockSchedulingStart();    
    _cancelled = true;
    _started = false;
    _startup_reset_time = fep3::Optional<Timestamp>();
    _cv_trigger_event.notify_all();

    return{};
}

void TimerScheduler::processSchedulerQueueSynchron(Timestamp current_time, fep3::Optional<Duration>& time_to_wait)
{
    assert(current_time >= Timestamp(0));
    assert(!time_to_wait.has_value()
        || (time_to_wait.has_value() && time_to_wait.value() > Duration(0)));

    // ATTENTION: This is the old implementation of ProcessSchedulerQueue for the synchronous case.
    // If you have to change anything in this method have also a look at the asynchron version!!!

    time_to_wait = Optional<Duration>();

    // the scheduler thread or the OnTimeUpdate method must process the queue exclusively.
    std::lock_guard<std::mutex> processing_lock(_mutex_processing_lock);

    // because the asynchronous version does not sort the list we must do this first
    {
        std::lock_guard<std::mutex> timers_lock(_mutex_timer);
        // have a look at the operator implementation in the header
        _timers.sort();
    }

    while (true)
    {
        std::unique_lock<std::mutex> timers_lock(_mutex_timer);

        auto timer_it = _timers.begin();
        if (timer_it == _timers.end())
        {
            break; // while
        }

        if (timer_it->_next_instant != Timestamp(0) &&
            timer_it->_next_instant > current_time)
        {
            time_to_wait = timer_it->_next_instant - current_time;
            break; // while
        }

        TimerInfo timer_info = *timer_it;   // copy timer info
        //we remember the simulated time step 
        const auto current_time_for_call = timer_info._next_instant;

        if (timer_it->_period != Duration(0))
        {
            // if the scheduler item has a period time, we have to
            // reinsert with a new timestamp
            timer_it->_next_instant += timer_it->_period;
            // don't resynchronize with the clock because
            // WE MUST CALL ALL THREADLOOPS of the item
            // maybe the item will resynchronize it self

            // iterate over the other items to find the next execution slot
            auto other_timer_it = _timers.begin();
            for (; other_timer_it != _timers.end(); ++other_timer_it)
            {
                if(other_timer_it->_next_instant <= current_time)
                {
                    // skip tasks which are delayed on the planned execution time
                    // to give them a chance to work (e.g. OneShotTimer). See #22389
                    // for more information
                    continue;
                }
                if (timer_it->_next_instant < other_timer_it->_next_instant)
                {
                    // break if the eventtime is smaller than the eventtime of the next item
                    break;
                }
            }

            if (other_timer_it != timer_it)
            {
                // insert the scheduleritem at the found place
                _timers.splice(other_timer_it, _timers, timer_it);
            }

        }
        else
        {
            // erase the scheduler item from list (OneShotTimer)
            _timers.erase(timer_it);
        }

        std::promise<void> finished_promise;
        timer_info._timer->wakeUp(current_time_for_call, &finished_promise);

        timers_lock.unlock();
        // in this case we have to wait until the timer has finished processing
        finished_promise.get_future().wait();
    } 
   
    // no negative duration
    assert(!time_to_wait.has_value()
        || (time_to_wait.has_value() && time_to_wait.value() > Duration(0)));
}

void TimerScheduler::processSchedulerQueueAsynchron(Timestamp current_time, Optional<Duration>& time_to_wait)
{
    assert(current_time >= Timestamp(0));
    assert(!time_to_wait.has_value()
        || (time_to_wait.has_value() && time_to_wait.value() > Duration(0)));

    // ATTENTION: This is the new implementation of ProcessSchedulerQueue for the asynchronous case.
    // If you have to change anything in this method have also a look at the synchron version!!!

    time_to_wait = Optional<Duration>();

    // the scheduler thread or the OnTimeUpdate method must process the queue exclusively.
    std::lock_guard<std::mutex> processing_lock(_mutex_processing_lock);

    bool loop_again = true;
    auto time_to_wait_min = Optional<Duration>();
    // the value for max loop count is 1000 because assuming that one loop takes at least
    // 1µs (I think it takes longer time), than the resync with the current time will be
    // performed every 1ms which is needed to guarantee that other (new) timers will be scheduled.
    // see #23270 for more information
    const size_t max_loop_count = 1000;
    size_t loop_count = 0;
    while (loop_again && loop_count < max_loop_count)
    {
        std::lock_guard<std::mutex> lock(_mutex_timer);

        loop_again = false;
        for (auto timer_it = _timers.begin(); timer_it != _timers.end();
            /*increment on iterator will be done within the loop*/)
        {
            // variable to detect necessary deletion
            bool delete_timer = false;

            // check the eventtimes for timeout calculation
            if (timer_it->_next_instant != Timestamp(0) &&
                timer_it->_next_instant > current_time)
            {
                // diff time is always greater than 0
                // remember the shortest time to wait
                const auto diff_time = timer_it->_next_instant - current_time;
                if (!time_to_wait_min.has_value() || diff_time < time_to_wait_min.value())
                {
                    time_to_wait_min = diff_time;
                }
                else
                {
                    // do nothing. the last remembered time was smaller.
                }

            }
            // check if the eventtime is smaller than the current time
            else if(timer_it->_next_instant <= current_time)
            {
                // the item must be triggered

                // wakeup the thread
                assert(current_time >= Timestamp(0));
                timer_it->_timer->wakeUp(current_time);

                if (timer_it->_period <= Duration(0))
                {
                    // OneShotTimer: delete from list
                    delete_timer = true;
                }
                else
                {
                    // periodic timer: increment the event time by period time
                    timer_it->_next_instant += timer_it->_period;
                    // if at least one period timer was found in the list, we have to loop again
                    // because it is possible that the timer has to be triggered again
                    loop_again = true;
                }
            }

            if (delete_timer)
            {
                // erase the scheduler item from list
                // and get the next valid iterator
                timer_it = _timers.erase(timer_it);
            }
            else
            {
                // increment iterator
                ++timer_it;
            }

        }

        time_to_wait = time_to_wait_min;
        ++loop_count;
    }

    // check if the loop ends because the max loop count was reached
    if (loop_count >= max_loop_count)
    {
        // in that case there are already some loops which have to be
        // executed. So we set the wait time to 0 (causes just execution yield)
        // and get the new current time from outside.
        time_to_wait = Duration(0);
    }

    // no negative duration
    assert(!time_to_wait.has_value() 
        || (time_to_wait.has_value() && time_to_wait.value() >= Duration(0)));
}

Timestamp TimerScheduler::getTime() const
{
    return _clock->getTime();
}

IClock::ClockType TimerScheduler::getClockType() const
{
    return _clock->getType();
}

fep3::Result TimerScheduler::execute(Timestamp /*time_of_execution*/)
{
    using namespace std::chrono_literals;

    const auto clock_type = getClockType();
       
    while (!_cancelled) 
    {     
        while(_block_scheduling_start 
            && !_cancelled) 
        {
            // waiting for start (will be set by call to timeResetBegin)
            std::this_thread::sleep_for(std::chrono::microseconds(300));
        }
     
        auto time_to_wait = Optional<Duration>();
        if (clock_type == IClock::ClockType::continuous)
        {
            processSchedulerQueueAsynchron(getTime(), time_to_wait);
        }               

        if (!time_to_wait.has_value())
        {
            // wait for the event. The loop within ProcessSchedulerQueue
            // was not executed. Otherwise time_to_wait is greater 0.
            while (!_cancelled)
            {
                std::unique_lock<std::mutex> lock(_mutex_processing_trigger);

                auto timeout = _cv_trigger_event.wait_for(lock, std::chrono::milliseconds(300));
                if (timeout == std::cv_status::no_timeout)
                {
                    break;
                }
            }
        }
        else
        {        
            if (time_to_wait.value() < 1ms)
            {
                // timespan is to short for wait. so just yield the execution.
                std::this_thread::yield();
            }
            else
            {
                // the next execution is more or equal 1ms. Just wait that time.
                std::unique_lock<std::mutex> lock(_mutex_processing_trigger);

                _cv_trigger_event.wait_for(lock, time_to_wait.value());
            }
        }
    }

    return{};
}

void TimerScheduler::timeResetBegin(Timestamp old_time, Timestamp new_time)
{
    _mutex_processing_lock.lock();    
 
    const auto do_forward = old_time < new_time;
    const Timestamp time_diff = (do_forward) ? new_time - old_time : old_time - new_time;  
    {
        std::lock_guard<std::mutex> lock(_mutex_timer);

        for (auto it = _timers.begin();
             it != _timers.end(); ++it)
        {
            if (do_forward)
            {
                it->_next_instant += time_diff;
            }
            else
            {
                it->_timer->reset();
                it->_next_instant -= time_diff;
            }
        }
    }

    if (IClock::ClockType::continuous == getClockType())
    {
        _block_scheduling_start = false;
    }

    if(!_started)
    {
       _startup_reset_time = new_time;     
    }

    // make sure any ongoing waiting is cancelled
    _cv_trigger_event.notify_all();
}

void TimerScheduler::timeResetEnd(Timestamp new_time)
{
    _mutex_processing_lock.unlock();

	std::lock_guard<std::mutex> lock_guard_reset_end(_mutex_start_stop_update);

    if (_started && getClockType() == IClock::ClockType::discrete)
    {
        fep3::Optional<Timestamp> time_to_wait;      
        processSchedulerQueueSynchron(new_time, time_to_wait);
    }

#ifdef __QNX__
    sched_yield();      // without this test cTimingLegacy::interfaceSystemTime fails on QNX
#endif
}

void TimerScheduler::timeUpdateBegin(Timestamp /*old_time*/, Timestamp /*new_time*/)
{
    //Nothing to do here
}

void TimerScheduler::timeUpdating(Timestamp new_time)
{
	std::lock_guard<std::mutex> lock_guard_updating(_mutex_start_stop_update);

    if (_started)
    {
        fep3::Optional<Duration> time_to_wait;
        processSchedulerQueueSynchron(new_time, time_to_wait);
    }
}

void TimerScheduler::timeUpdateEnd(Timestamp /*new_time*/)
{
    //Nothing to do here
}

} //namespace native
} //namespace fep3
