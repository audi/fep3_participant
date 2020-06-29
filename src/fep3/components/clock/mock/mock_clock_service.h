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

#pragma once

#include <gmock/gmock.h>
#include <chrono>
#include <mutex>
#include <list>
#include <condition_variable>
#include <vector>
#include <memory>

#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/components/base/component_base.h>

namespace fep3
{
namespace mock 
{

struct Clock : public fep3::IClock
{
    MOCK_CONST_METHOD0(getName, std::string());
    MOCK_CONST_METHOD0(getType, ClockType());
    MOCK_CONST_METHOD0(getTime, Timestamp());
    MOCK_METHOD0(reset, void());
    MOCK_METHOD1(start, void(const std::weak_ptr<IEventSink>&));
    MOCK_METHOD0(stop, void());
};

struct EventSink : IClock::IEventSink
{
    MOCK_METHOD2(timeUpdateBegin, void(Timestamp, Timestamp));
    MOCK_METHOD1(timeUpdating, void(Timestamp));
    MOCK_METHOD1(timeUpdateEnd, void(Timestamp));
    MOCK_METHOD2(timeResetBegin, void(Timestamp, Timestamp));
    MOCK_METHOD1(timeResetEnd, void(Timestamp));
};

// Event sink which stores values of time update and reset events
struct EventSinkTimeEventValues : EventSink
{
    EventSinkTimeEventValues(const size_t expected_calls_count)
        : _expected_calls_count(expected_calls_count)
    {
        using namespace ::testing;

        ON_CALL(*this, timeUpdating(_))
            .WillByDefault(Invoke([this](Timestamp new_time)
        {
            if (_calls.size() < _expected_calls_count)
            {
                _calls.push_back(new_time);
            }
            else if (_calls.size() >= _expected_calls_count)
            {
               _expected_calls_reached.notify_all();
            }
        }
        ));
        ON_CALL(*this, timeResetEnd(_))
            .WillByDefault(Invoke([this](Timestamp new_time)
        {
            if (_calls.size() < _expected_calls_count)
            {
               _calls.push_back(new_time);
            }
            else if (_calls.size() >= _expected_calls_count)
            {
              _expected_calls_reached.notify_all();
            }
        }
        ));
    }

    size_t _expected_calls_count;
    std::condition_variable _expected_calls_reached;
    std::vector<Timestamp> _calls{};
};

// Event sink which stores frequency of (the duration between) time update events
struct EventSinkTimeEventFrequency : fep3::mock::EventSink
{
    EventSinkTimeEventFrequency(const size_t expected_calls_count)
        : _expected_calls_count(expected_calls_count)
    {
        using namespace ::testing;
        using namespace std::chrono;

        ON_CALL(*this, timeUpdating(_))
            .WillByDefault(Invoke([this](Timestamp /**unused_parameter*/)
        {
            if (_call_durations.size() == 0)
            {
                _last_time_event_occured = steady_clock::now();
                _call_durations.push_back(Timestamp{ 0 });
            }
            else if (_call_durations.size() < _expected_calls_count)
            {
                _call_durations.push_back(steady_clock::now() - _last_time_event_occured);
                _last_time_event_occured = steady_clock::now();
            }
            else if (_call_durations.size() >= _expected_calls_count)
            {
                _expected_calls_reached.notify_all();
            }
        }
        ));
    }

    void assertTimeEventDeviation(Timestamp expected_event_duration, Timestamp allowed_deviation = Timestamp{ 0 })
    {
        for (const auto& time_actual : _call_durations)
        {
            if (0 != time_actual.count())
            {
                EXPECT_NEAR(
                    static_cast<double>(time_actual.count()),
                    static_cast<double>(expected_event_duration.count()),
                    static_cast<double>(allowed_deviation.count()));
            }
        }
    }

    std::chrono::time_point<std::chrono::steady_clock> _last_time_event_occured;
    size_t _expected_calls_count;
    std::condition_variable _expected_calls_reached;
    std::vector<Timestamp> _call_durations{};
};

template<template<typename...> class component_base_type = fep3::ComponentBase>
struct ClockService
    : public component_base_type<IClockService>
{
    MOCK_CONST_METHOD0(getTime, Timestamp());
    MOCK_CONST_METHOD1(getTime, Optional<Timestamp>(const std::string&));

    MOCK_CONST_METHOD0(getType, IClock::ClockType());
    MOCK_CONST_METHOD1(getType, Optional<IClock::ClockType>(const std::string&));

    MOCK_CONST_METHOD0(getMainClockName, std::string());

    MOCK_METHOD1(registerEventSink, fep3::Result(const std::weak_ptr<fep3::IClock::IEventSink>&));
    MOCK_METHOD1(unregisterEventSink, fep3::Result(const std::weak_ptr<fep3::IClock::IEventSink>&));

    MOCK_METHOD1(registerClock, fep3::Result(const std::shared_ptr<IClock>&));
    MOCK_METHOD1(unregisterClock, fep3::Result(const std::string&));

    MOCK_CONST_METHOD0(getClockNames, std::list<std::string>());
    MOCK_CONST_METHOD1(findClock, std::shared_ptr<IClock>(const std::string&));

    MOCK_METHOD0(start, fep3::Result());
    MOCK_METHOD0(stop, fep3::Result());
};

struct ClockServiceComponentWithDefaultBehaviour : public ClockService<>
{
    ClockServiceComponentWithDefaultBehaviour()
    {
        using namespace ::testing;

        ON_CALL(*this, getTime())
            .WillByDefault(Invoke([]() {return Timestamp(0); }));
        ON_CALL(*this, getTime(_))
            .WillByDefault(Invoke([](std::string /*clock_name*/) {return Optional<Timestamp>{Timestamp(0)}; }));

        ON_CALL(*this, registerEventSink(_))
            .WillByDefault(Invoke([this](std::weak_ptr<fep3::IClock::IEventSink>) {return fep3::Result{}; }));

        ON_CALL(*this, unregisterEventSink(_))
            .WillByDefault(Invoke([this](std::weak_ptr<fep3::IClock::IEventSink>) {return fep3::Result{}; }));
    }
};

struct DiscreteSteppingClockService : public fep3::ComponentBase<fep3::IClockService>
{
    MOCK_CONST_METHOD0(getTime, Timestamp());
    MOCK_CONST_METHOD1(getTime, Optional<Timestamp>(const std::string&));

    MOCK_CONST_METHOD0(getType, IClock::ClockType());
    MOCK_CONST_METHOD1(getType, Optional<IClock::ClockType>(const std::string&));

    MOCK_CONST_METHOD0(getMainClockName, std::string());

    MOCK_METHOD1(registerEventSink, fep3::Result(const std::weak_ptr<fep3::IClock::IEventSink>&));
    MOCK_METHOD1(unregisterEventSink, fep3::Result(const std::weak_ptr<fep3::IClock::IEventSink>&));

    MOCK_METHOD1(registerClock, fep3::Result(const std::shared_ptr<IClock>&));
    MOCK_METHOD1(unregisterClock, fep3::Result(const std::string&));

    MOCK_CONST_METHOD0(getClockNames, std::list<std::string>());
    MOCK_CONST_METHOD1(findClock, std::shared_ptr<IClock>(const std::string&));

    inline DiscreteSteppingClockService()
    {
        using namespace std::chrono;
        using namespace ::testing;

        ON_CALL(*this, getTime())
            .WillByDefault(Invoke([this]() {return Timestamp(_current_time); }));

        ON_CALL(*this, getTime(_))
            .WillByDefault(Invoke([this](std::string /*dont_care_for_name*/) {return Optional<Timestamp>{Timestamp(_current_time)}; }));

        ON_CALL(*this, registerEventSink(_))
            .WillByDefault(Invoke([this](std::weak_ptr<fep3::IClock::IEventSink>) {return fep3::Result{}; }));

        ON_CALL(*this, unregisterEventSink(_))
            .WillByDefault(Invoke([this](std::weak_ptr<fep3::IClock::IEventSink>) {return fep3::Result{}; }));

        ON_CALL(*this, getType())
            .WillByDefault(Return(fep3::IClock::ClockType::continuous));    
    }
    
    void inline setCurrentTime(Timestamp current_time)
    {
        std::lock_guard<std::mutex> lock(_time_mutex);
        _current_time = current_time;
    }

    void inline incrementTime(Timestamp time_increment)
    {
        std::lock_guard<std::mutex> lock(_time_mutex);
        _current_time += time_increment;
    }

private:
    std::mutex _time_mutex;
    Timestamp _current_time{0};
};


struct ChronoDrivenClockService : public fep3::ComponentBase<fep3::IClockService>
{
    MOCK_CONST_METHOD0(getTime, Timestamp());
    MOCK_CONST_METHOD1(getTime, Optional<Timestamp>(const std::string&));

    MOCK_CONST_METHOD0(getType, IClock::ClockType());
    MOCK_CONST_METHOD1(getType, Optional<IClock::ClockType>(const std::string&));

    MOCK_CONST_METHOD0(getMainClockName, std::string());

    MOCK_METHOD1(registerEventSink, fep3::Result(const std::weak_ptr<fep3::IClock::IEventSink>&));
    MOCK_METHOD1(unregisterEventSink, fep3::Result(const std::weak_ptr<fep3::IClock::IEventSink>&));

    MOCK_METHOD1(registerClock, fep3::Result(const std::shared_ptr<IClock>&));
    MOCK_METHOD1(unregisterClock, fep3::Result(const std::string&));

    MOCK_CONST_METHOD0(getClockNames, std::list<std::string>());
    MOCK_CONST_METHOD1(findClock, std::shared_ptr<IClock>(const std::string&));

    fep3::Result start() override
    {
        _current_offset = std::chrono::steady_clock::now();
        _started = true;
        return {};
    }  

    fep3::Result stop() override
    {        
        _started = false;
        return {};
    }  

    inline ChronoDrivenClockService()
    {
        using namespace std::chrono;
        using namespace ::testing;

        ON_CALL(*this, getTime())
            .WillByDefault(Invoke([this]() 
                {
                   return getChronoTime();
                }));

        ON_CALL(*this, getTime(_))
            .WillByDefault(Invoke([this](std::string /*dont_care_for_name*/) 
                {
                    return getChronoTime();
                }));

        ON_CALL(*this, registerEventSink(_))
            .WillByDefault(Invoke([this](std::weak_ptr<fep3::IClock::IEventSink>) {return fep3::Result{}; }));

        ON_CALL(*this, unregisterEventSink(_))
            .WillByDefault(Invoke([this](std::weak_ptr<fep3::IClock::IEventSink>) {return fep3::Result{}; }));
    }

private:
    fep3::Timestamp getChronoTime()
    {
        if(!_started)
        {
            return fep3::Timestamp{0};
        }
        return Timestamp{std::chrono::steady_clock::now() - _current_offset};
    }

private:
    std::mutex _time_mutex;
    bool _started {false};
    mutable std::chrono::time_point<std::chrono::steady_clock> _current_offset;
};

}
} 
