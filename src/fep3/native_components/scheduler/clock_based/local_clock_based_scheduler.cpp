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


#include <cassert>

#include "local_clock_based_scheduler.h"

namespace fep3
{
namespace native
{

ServiceThread::ServiceThread(const std::string& name,
                               fep3::IJob& job,
                               fep3::IClockService& clock,
                               uint32_t flags)
    : _flags(flags)
    , _name(name)
    , _runnable(job)
    , _clock(clock)
{
}

ServiceThread::~ServiceThread()
{
    join();
}

fep3::Result ServiceThread::start()
{
    _exited_promise = new  std::promise<void>();
    _exited_future = new  std::future<void>();
    *_exited_future = _exited_promise->get_future();
    {
        std::lock_guard<std::mutex> oLocker(_mutex_thread);
        _system_thread = std::thread(std::ref(*this));
        
    }

    return{};
}

void ServiceThread::operator()()
{
    {
        std::lock_guard<std::mutex> lock(_mutex_thread);

        if (_thread_cancelled)
        {
            _exited_promise->set_value();
            return;
        }
    }  
    auto nResult = execute(_clock.getTime());
    {
        std::lock_guard<std::mutex> lock(_mutex_thread);
        _exited_promise->set_value();
    }
}

fep3::Result ServiceThread::execute(Timestamp wakeup_time)
{
    _runnable.executeDataIn(wakeup_time);
    _runnable.execute(wakeup_time);
    _runnable.executeDataOut(wakeup_time);

    return{};
}

fep3::Result ServiceThread::join(Timestamp timeout)
{
    if (!_system_thread.joinable())
    {
        RETURN_ERROR_DESCRIPTION(fep3::ERR_INVALID_STATE, "thread not joinable");
    }

    if (Timestamp(-1) == timeout)
    {
        _system_thread.join();
    }
    else
    {
        if (_exited_future->wait_for(Timestamp(timeout)) == std::future_status::ready)
        {
            _system_thread.join();
        }
        else
        {
            RETURN_ERROR_DESCRIPTION(fep3::ERR_TIMEOUT, "timeout joining thread");
        }
    }
    if (_exited_promise)
    {
        delete _exited_promise;
        _exited_promise = nullptr;
        delete _exited_future;
        _exited_future = nullptr;
    }
    return{};
}

fep3::Result ServiceThread::detach()
{
    if (!_system_thread.joinable())
    {
        RETURN_ERROR_DESCRIPTION(fep3::ERR_INVALID_STATE, "thread not joinable");
    }

    std::lock_guard<std::mutex> lock(_mutex_thread);
    if (_exited_future->wait_for(Timestamp(0)) == std::future_status::ready)
    {
        _system_thread.join();
    }
    else
    {     
        // TODO implement or remove
		// make sure we live long enough
        //    m_pThis = object_ptr_from_this();
        _system_thread.detach();
    }
    if (_exited_promise)
    {
        delete _exited_promise;
        _exited_promise = nullptr;
        delete _exited_future;
        _exited_future = nullptr;
    }
    return{};
}

bool ServiceThread::isCurrent() const
{
    return std::this_thread::get_id() == _system_thread.get_id();
}

std::string ServiceThread::getName() const
{
    return _name;
}

bool ServiceThread::joinable() const
{
    return _system_thread.joinable();
}

TimerThread::TimerThread(const std::string& name,
                           fep3::IJob& runnable,
                           fep3::IClockService& clock,
                           Duration period,
                           Duration initial_delay,
                           TimerScheduler& scheduler,
                           const fep3::native::JobRunner& job_runner)
    : ServiceThread(name, runnable, clock, 0),
      _period(period),
      _initial_delay(initial_delay),
      _cancelled(false),
      _timer_scheduler(scheduler),
      _job_runner(job_runner)
{
}

TimerThread::~TimerThread()
{
    stop();
}

fep3::Result TimerThread::execute(Timestamp /*first_wakeup_time*/)
{
    fep3::Result result = fep3::ERR_NOERROR;

      while (!_cancelled)
    {              
        std::unique_lock<std::mutex> lock(_mutex_manual_event);

        // we (typically a job) wait here are until we are woken up by TimerScheduler::processSchedulerQueueAsynchron
        _cv_manual_event.wait(lock, [this] { return _manual_event_occured; });
        _manual_event_occured = false;        
        
        const auto wakeup_time = getWakeUpTime();

        if (_cancelled)        
        {            
            break;
        }    

        if(wakeup_time == reset_time)
        {        
            // reset was received after waking us up, so we won't run the job
            continue;
        }          
     
        if (_last_call_time == reset_time.count()
            || wakeup_time.count() > _last_call_time)
        { 
            result |= _job_runner.runJob(wakeup_time, _runnable);
            _last_call_time = _wakeup_time;
        }                 

        if (_finished_promise)
        {
            _finished_promise->set_value();
            _finished_promise = nullptr;
        }    
    }

    return result;
}

fep3::Result TimerThread::wakeUp(Timestamp wakeup_time, std::promise<void>* finished_promise)
{
    std::lock_guard<std::mutex> lock(_mutex_manual_event);

    _finished_promise = finished_promise;
    setWakeUpTime(wakeup_time);
    _manual_event_occured = true;
    _cv_manual_event.notify_all();
    return{};
}

Timestamp TimerThread::getWakeUpTime() const
{
    return Timestamp(_wakeup_time);
}

void TimerThread::setWakeUpTime(Timestamp wakeup_time)
{
    _wakeup_time = wakeup_time.count();
}

fep3::Result TimerThread::start()
{
    _cancelled = false;
    reset();
    return ServiceThread::start();
}

fep3::Result TimerThread::stop()
{
    if (joinable())
    {
        _cancelled = true;
        if (!isCurrent())
        {
            wakeUp(_clock.getTime());
            FEP3_RETURN_IF_FAILED(join());
        }
        else
        {
            return detach();
        }
    }

    return{};
}

fep3::Result TimerThread::remove()
{
    return _timer_scheduler.removeTimer(*this);
}

fep3::Result TimerThread::reset()
{
    _wakeup_time = reset_time.count();
    _last_call_time = reset_time.count();
    return {};
}


LocalClockBasedScheduler::LocalClockBasedScheduler(
    const std::shared_ptr<const fep3::ILoggingService::ILogger>& logger,
    const std::function<fep3::Result()>& set_participant_to_error_state) :
         _logger(logger),
         _set_participant_to_error_state(set_participant_to_error_state)
{
    if (!_logger)
    {
        throw std::runtime_error("Logger not set");
    }
}

std::string LocalClockBasedScheduler::getName() const
{
    return FEP3_SCHEDULER_CLOCK_BASED;
}

fep3::Result LocalClockBasedScheduler::initialize(fep3::IClockService& clock,
                                                 const Jobs& jobs)
{   
    _timer_scheduler= std::make_shared<TimerScheduler>(clock);   
    FEP3_RETURN_IF_FAILED(clock.registerEventSink(_timer_scheduler));

    _clock = &clock;

    _service_thread = std::make_unique<ServiceThread>("__scheduler", *_timer_scheduler, clock, 0);

    for (auto& job : jobs)
    {   
        auto timer_thread = createTimerThread(job.second, clock);

        FEP3_RETURN_IF_FAILED(addTimerThreadToScheduler(job.second, timer_thread));
        _timers.push_back(timer_thread);
    }

    return{};
}

fep3::Result LocalClockBasedScheduler::addTimerThreadToScheduler(
    const fep3::JobEntry& job_entry,
    std::shared_ptr<fep3::native::TimerThread> timer_thread)
{
    FEP3_RETURN_IF_FAILED(_timer_scheduler->addTimer(*timer_thread.get(),
        job_entry.job_info.getConfig()._cycle_sim_time,
        job_entry.job_info.getConfig()._delay_sim_time));
    
    return {};
}


std::shared_ptr<fep3::native::TimerThread> LocalClockBasedScheduler::createTimerThread(
    const fep3::JobEntry& job_entry,
    fep3::IClockService& clock)
{
    const auto job_info = job_entry.job_info;

    fep3::native::JobRunner job_runner(job_info.getName(),
        job_info.getConfig()._runtime_violation_strategy,
        job_info.getConfig()._max_runtime_real_time,
        _logger,
        _set_participant_to_error_state);

    auto timer_thread = std::make_shared<TimerThread>(job_info.getName(),
        *job_entry.job,
        clock,
        job_info.getConfig()._cycle_sim_time,
        job_info.getConfig()._delay_sim_time,      
        *_timer_scheduler,
        job_runner);

    return timer_thread;
}

fep3::Result LocalClockBasedScheduler::start()
{
    for (auto& timer : _timers)
    {
        timer->start();
    }
    FEP3_RETURN_IF_FAILED(_timer_scheduler->start());
    FEP3_RETURN_IF_FAILED(_service_thread->start());
    return{};
}

fep3::Result LocalClockBasedScheduler::stop()
{
    if (_timer_scheduler)
    {
        _timer_scheduler->stop();
    }

    for (auto& timer : _timers)
    {
        timer->stop();
    }

    if (_service_thread)
    {
        _service_thread->join();
    }
    return{};
}

fep3::Result LocalClockBasedScheduler::deinitialize()
{
    stop();
    if(_clock)
    {
        _clock->unregisterEventSink(_timer_scheduler);
        _clock = nullptr;
    }   
    for (auto& timer : _timers)
    {
        timer->remove();
    }
    _timer_scheduler.reset();
    _service_thread.reset();
    _timers.clear();
    return{};
}

} // namespace native
} // namespace fep3
