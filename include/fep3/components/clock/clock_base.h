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

#include <mutex>
#include <atomic>

#include <fep3/components/clock/clock_service_intf.h>

namespace fep3
{
namespace base
{
namespace arya
{
/**
* @brief Base implementation of a Clock 
* 
*/
class ClockBase : public fep3::arya::IClock
{
public:
    /**
    * @brief CTOR.
    * A @ref ClockBase is initialized with a current time of 0.
    *
    * @param clock_name Name of the clock
    */
    explicit ClockBase(const std::string& clock_name)
        : _clock_name(clock_name)
        , _current_time(fep3::arya::Timestamp(0))
        , _updated(false)
        , _started(false)
    {
    }

    /**
    * @brief Delete copy CTOR
    * 
    * @param other ClockBase to copy from
    */
    ClockBase(const ClockBase& other) = delete;

    /**
    * @brief Delete move CTOR
    * 
    * @param other ClockBase to move from
    */
    ClockBase(ClockBase&& other) = delete;

    /**
    * @brief Delete copy assignment operator
    * 
    * @param other ClockBase to copy from
    * @return ClockBase 
    */
    ClockBase& operator=(const ClockBase& other) = delete;

    /**
    * @brief Delete move assignment operator
    * 
    * @param other ClockBase to move from
    * @return ClockBase 
    */
    ClockBase& operator=(ClockBase&& other) = delete;

    /**
    * @brief DTOR
    */
    ~ClockBase() override = default;

public:
   /**
    * @copydoc IClock::getName
    */
    std::string getName() const override
    {
        return _clock_name;
    }

    /**
    * @copydoc IClock::start
    */
    void start(const std::weak_ptr<fep3::arya::IClock::IEventSink>& event_sink) override
    {
        _updated = false;        
        {
            std::lock_guard<std::recursive_mutex> lock_guard(_mutex_sink_and_time);
            _event_sink = event_sink;
        }
        _started = true;
        reset();
    }

    /**
    * @copydoc IClock::stop
    */
    void stop() override
    {
        _started = false;
        {
            std::lock_guard<std::recursive_mutex> lock_guard(_mutex_sink_and_time);
            _event_sink.reset();
        }
        _updated = false;
    }


protected:
    ///members have to be secured versus multi threaded access in derived classes
    ///event sink given on start call which receives time events
    std::weak_ptr<fep3::arya::IClock::IEventSink> _event_sink;
    ///the name of this clock
    std::string _clock_name;
    ///the current time of this clock
    mutable fep3::arya::Timestamp _current_time;
    ///determine if the clock was set by any setNewTime call 
    mutable std::atomic_bool _updated;
    ///determine wether the clock is started or not
    mutable std::atomic_bool _started;
    ///recursive mutex
    mutable std::recursive_mutex _mutex_sink_and_time;
};

/**
* @brief Base implementation for a continuous clock which will automatically 
* 	call the @ref arya::IClock::IEventSink for you.
* You have to implement following functionality: 
* @li CTOR
* @li @ref ContinuousClock::getNewTime
* @li @ref ContinuousClock::resetTime
*/
class ContinuousClock : public ClockBase
{ 
public:
    /**
    * @brief CTOR
    * 
    * @param name Name of the clock
    */
    ContinuousClock(const std::string& name)
        : ClockBase(name)
    {
    }

protected:
    /**
    * @brief Receive a new Timestamp from the continuous clock
    *
    * @remark @p Override this function to implement a custom continuous clock
    *
    * @return The new time
    */    
    virtual fep3::arya::Timestamp getNewTime() const = 0;

    /**
    * @brief Reset the clock
    *
    * @remark @p Override this function to implement a custom continuous clock
    *
    * @return The timestamp to which the clock has been reset to
    */
    virtual fep3::arya::Timestamp resetTime() = 0;

protected:
    /**
    * @brief Get the current clock time
    *
    * @return The current time of the clock
    */
    fep3::arya::Timestamp getTime() const override
    {
        setNewTime(getNewTime());
        
        std::lock_guard<std::recursive_mutex> lock_guard(_mutex_sink_and_time);
        return _current_time;
    }

    /**
    * @brief Get the clock type
    *
    * @return The clock type
    */
    fep3::arya::IClock::ClockType getType() const override
    {
        return fep3::arya::IClock::ClockType::continuous;
    }

    /**
    * @brief Reset the clock time
    */
    void reset() override
    {
         setResetTime(resetTime());
    }

private:
    /**
    * @brief Set a new time for the clock.
    * Reset the clock if 'setNewTime' has been called for the first time
    * or if @p new_time is smaller than the old time
    *
    * @param new_time The new time of the clock
    */
    void setNewTime(fep3::arya::Timestamp new_time) const
    {
        std::unique_lock<std::recursive_mutex> lock_guard(_mutex_sink_and_time);
        const auto old_time = _current_time;
        lock_guard.unlock();

        if (!_updated)
        {
            _updated = true;
            setResetTime(new_time);
        }
        if (new_time < old_time)
        {
            setResetTime(new_time);
        }

        _updated = true;

        lock_guard.lock();        
        _current_time = new_time;
        lock_guard.unlock();
    }
        
    /**
    * @brief Set a new time for the clock and emit time reset events via the event sink
    *
    * @param new_time The new time of the clock
    */
    void setResetTime(const fep3::arya::Timestamp new_time) const
    {
        std::unique_lock<std::recursive_mutex> lock_guard(_mutex_sink_and_time);
        
        const auto old_time = _current_time;
        auto event_sink_pointer = _event_sink.lock();
        lock_guard.unlock();

        if (event_sink_pointer)
        {
            event_sink_pointer->timeResetBegin(old_time, new_time);
        }

        _updated = true;

        lock_guard.lock();
        _current_time = new_time;
        lock_guard.unlock();   

        if (event_sink_pointer)
        {
            event_sink_pointer->timeResetEnd(new_time);
        }
    }
};
/**
* @brief Base implementation for a discrete clock which will automatically 
*        call the IClock::IEventSink for you.
* You have to implement following functionality:
* @li CTOR
* 
* While using you only call DiscreteClock::setNewTime and DiscreteClock::setResetTime
*/
class DiscreteClock : public ClockBase
{
public:
    /**
    * @brief CTOR
    * 
    * @param name Name of the clock
    */
    DiscreteClock(const std::string& name) : ClockBase(name)
    {
    }

protected:
    /**
    * @brief Get the current clock time
    *
    * @return The current clock time
    */
    fep3::arya::Timestamp getTime() const override
    {
        std::lock_guard<std::recursive_mutex> lock_guard(_mutex_sink_and_time);
        return _current_time;
    }

    /**
    * @brief Reset the clock time
    */
    void reset() override
    {              
        _updated = true;

        setResetTime(fep3::arya::Timestamp(0));
    }

    /**
    * @brief Get the clock type
    *
    * @return The clock type
    */
    fep3::arya::IClock::ClockType getType() const override
    {
        return fep3::arya::IClock::ClockType::discrete;
    }

public:
    /**
    * @brief Set a new time for the clock. 
    * Emit time update events via the event sink.
    * Reset the clock if @ref setNewTime has been called for the first time
    * or if @p new_time is smaller than the current time
    *
    * @param new_time The new time of the clock
    * @param send_update_before_after Flag indicates whether @ref IClock::IEventSink::timeUpdateBegin 
    *                                 and @ref IClock::IEventSink::timeUpdateEnd events should be emitted.
    *                                 The event @ref IClock::IEventSink::timeUpdating is always emitted.
    */
    void setNewTime(const fep3::arya::Timestamp new_time, const bool send_update_before_after) const
    {       
        std::unique_lock<std::recursive_mutex> lock_guard(_mutex_sink_and_time); 

        const auto old_time = _current_time;
        lock_guard.unlock();

        if (!_updated)
        {            
            _updated = true;
            setResetTime(new_time);
        }
        else if (new_time < old_time)
        {           
            setResetTime(new_time);
        }
        else
        {
            lock_guard.lock();
            auto event_sink_pointer = _event_sink.lock();
            lock_guard.unlock();

            if (event_sink_pointer && send_update_before_after)
            {
                event_sink_pointer->timeUpdateBegin(old_time, new_time);
            }
            
            lock_guard.lock();
            _current_time = new_time;
            lock_guard.unlock();
            
            if (event_sink_pointer)
            {
                event_sink_pointer->timeUpdating(new_time);
            }
            if (event_sink_pointer && send_update_before_after)
            {
                event_sink_pointer->timeUpdateEnd(new_time);
            }
        }
    }

    /**
    * @brief Set a new time for the clock and emit time reset events via the event sink.
    *
    * @param new_time The new time of the clock
    */
    void setResetTime(const fep3::arya::Timestamp new_time) const
    {      
        std::unique_lock<std::recursive_mutex> lock_guard(_mutex_sink_and_time);

        const auto old_time = _current_time;
        auto _event_sink_pointer = _event_sink.lock();
        lock_guard.unlock();

        if (_event_sink_pointer)
        {
            _event_sink_pointer->timeResetBegin(old_time, new_time);
        }

        _updated = true;

        lock_guard.lock();        
        _current_time = new_time;
        lock_guard.unlock();
        
        if (_event_sink_pointer)
        {
            _event_sink_pointer->timeResetEnd(new_time);
        }
    }
};
} // namespace arya
using arya::ClockBase;
using arya::ContinuousClock;
using arya::DiscreteClock;
} // namespace base
} // namespace fep3

