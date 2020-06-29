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
#include <thread>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fep3/native_components/clock/local_system_clock_discrete.h>
#include <fep3/components/clock/mock/mock_clock_service.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <common/gtest_asserts.h>

using namespace ::testing;
using namespace fep3::native;
using namespace fep3::arya;
using namespace std::chrono;
using namespace fep3;

using EventSinkTimeEventValues = NiceMock<fep3::mock::EventSinkTimeEventValues>;
using EventSinkTimeEventFrequency = NiceMock<fep3::mock::EventSinkTimeEventFrequency>;

struct DiscreteClockTest : public Test
{
    DiscreteClockTest() = default;

    LocalSystemSimClock local_system_sim_clock;
};

/**
 * @detail Test whether the clock omits discrete time steps as configured to an event sink
 * @req_id FEPSDK-2110
 */
TEST_F(DiscreteClockTest, testClockProvidesDiscreteTimeSteps)
{
    using namespace std::chrono_literals;

    const size_t cycle_time = 100, clock_cycles = 5;
    const double time_factor = 1.0;
    std::shared_ptr<EventSinkTimeEventValues> _event_sink_mock = std::make_shared<EventSinkTimeEventValues>(clock_cycles);

    std::vector<Timestamp>expected_timestamps{
        Timestamp{ cycle_time * 0 } };  // First timestamp originating from a reset event once the clock is started

    for (size_t i = 1; i < clock_cycles; i++)
    {
        expected_timestamps.emplace_back(Timestamp( i * cycle_time ));  // Timestamps originating from time update events
    }

    local_system_sim_clock.updateConfiguration(Duration{ cycle_time }, time_factor);

    // actual test
    {
        local_system_sim_clock.start(_event_sink_mock);
        std::mutex mutex;
        {
            std::unique_lock<std::mutex> lock(mutex);
            _event_sink_mock->_expected_calls_reached.wait_for(lock, 1s);
        }
        local_system_sim_clock.stop();

        EXPECT_EQ(expected_timestamps, _event_sink_mock->_calls);
    }
}

/**
 * @detail Test whether the clock can reset its time
 *
 */
TEST_F(DiscreteClockTest, testClockReset)
{
    using namespace std::chrono_literals;

    const size_t cycle_time = 100;
    const double time_factor = 1.0;
    std::shared_ptr<EventSinkTimeEventValues> _event_sink_mock = std::make_shared<EventSinkTimeEventValues>(2);
    std::vector<Timestamp>expected_timestamps{ Timestamp{0}, Timestamp{100}, Timestamp{0}, Timestamp{100} };

    // Update the clock configuraiton to reduce test duration
    local_system_sim_clock.updateConfiguration(Duration{ cycle_time }, time_factor);

    // actual test
    {
        local_system_sim_clock.start(_event_sink_mock);
        std::mutex mutex;
        {
            std::unique_lock<std::mutex> lock(mutex);
            ASSERT_NE(_event_sink_mock->_expected_calls_reached.wait_for(lock, 1s), std::cv_status::timeout);
        }
        local_system_sim_clock.stop();

        _event_sink_mock->_expected_calls_count = 4;

        // start the clock again to trigger a time reset event
        local_system_sim_clock.start(_event_sink_mock);
        {
            std::unique_lock<std::mutex> lock(mutex);
            ASSERT_NE(_event_sink_mock->_expected_calls_reached.wait_for(lock, 1s), std::cv_status::timeout);
        }
        local_system_sim_clock.stop();

        EXPECT_EQ(expected_timestamps, _event_sink_mock->_calls);
    }
}

/**
 * @detail Test whether the frequency of a discrete clock's time update events meets the configured expectations
 *
 * @testType performanceTest
 * This test may fail if the system is under heavy load as the test depends on the system performance.
 */
TEST_F(DiscreteClockTest, testClockTimeEventFrequency)
{
    using namespace std::chrono_literals;

    const Duration cycle_time{ duration_cast<Duration>(50ms) },
        expected_event_duration{ duration_cast<Duration>(50ms) },
        allowed_deviation{ duration_cast<Duration>(50ms) };
    const size_t clock_cycles = 3;
    const double time_factor = 1;
    std::shared_ptr<EventSinkTimeEventFrequency> _event_sink_mock = std::make_shared<EventSinkTimeEventFrequency>(clock_cycles);

    local_system_sim_clock.updateConfiguration(cycle_time, time_factor);

    // actual test
    {
        local_system_sim_clock.start(_event_sink_mock);
        std::mutex mutex;
        {
            std::unique_lock<std::mutex> lock(mutex);
            _event_sink_mock->_expected_calls_reached.wait_for(lock, 1000ms);
        }
        local_system_sim_clock.stop();

        EXPECT_EQ(clock_cycles, _event_sink_mock->_call_durations.size());
        _event_sink_mock->assertTimeEventDeviation(expected_event_duration, allowed_deviation);
    }
}

/**
 * @detail Test whether a small time factor decreases the frequency of a discrete clock's time update events
 * @req_id FEPSDK-2111
 *
 * @testType performanceTest
 * This test may fail if the system is under heavy load as the test depends on the system performance.
 */
TEST_F(DiscreteClockTest, testClockTimeEventFrequencySmallTimeFactor)
{
    using namespace std::chrono_literals;

    const Duration cycle_time{ duration_cast<Duration>(50ms) },
        expected_event_duration{ duration_cast<Duration>(100ms) },
        allowed_deviation{ duration_cast<Duration>(50ms) };
    const size_t clock_cycles = 3;
    const double time_factor = 0.5;
    std::shared_ptr<EventSinkTimeEventFrequency> _event_sink_mock = std::make_shared<EventSinkTimeEventFrequency>(clock_cycles);

    local_system_sim_clock.updateConfiguration(cycle_time, time_factor);

    // actual test
    {
        local_system_sim_clock.start(_event_sink_mock);
        std::mutex mutex;
        {
            std::unique_lock<std::mutex> lock(mutex);
            _event_sink_mock->_expected_calls_reached.wait_for(lock, 1000ms);
        }
        local_system_sim_clock.stop();

        EXPECT_EQ(clock_cycles, _event_sink_mock->_call_durations.size());
        _event_sink_mock->assertTimeEventDeviation(expected_event_duration, allowed_deviation);
    }
}

/**
 * @detail Test whether a big time factor increases the frequency of a discrete clock's time update events
 * @req_id FEPSDK-2111
 *
 * @testType performanceTest
 * This test may fail if the system is under heavy load as the test depends on the system performance.
 */
TEST_F(DiscreteClockTest, testClockTimeEventFrequencyBigTimeFactor)
{
    using namespace std::chrono_literals;

    // we have to use 10 ms as cycle time due to performance limitations of the discrete clock
    const Duration cycle_time{ duration_cast<Duration>(50ms) },
        expected_event_duration{ duration_cast<Duration>(25ms) },
        allowed_deviation{ duration_cast<Duration>(50ms) };
    const size_t clock_cycles = 3;
    const double time_factor = 2;
    std::shared_ptr<EventSinkTimeEventFrequency> _event_sink_mock = std::make_shared<EventSinkTimeEventFrequency>(clock_cycles);

    local_system_sim_clock.updateConfiguration(cycle_time, time_factor);

    // actual test
    {
        local_system_sim_clock.start(_event_sink_mock);
        std::mutex mutex;
        {
            std::unique_lock<std::mutex> lock(mutex);
            _event_sink_mock->_expected_calls_reached.wait_for(lock, 1000ms);
        }
        local_system_sim_clock.stop();

        EXPECT_EQ(clock_cycles, _event_sink_mock->_call_durations.size());
        _event_sink_mock->assertTimeEventDeviation(expected_event_duration, allowed_deviation);
    }
}

/**
 * @detail Test whether a time factor of 0 (which means AFAP mode) increases the frequency of a discrete clock's time update events
 * @req_id FEPSDK-2122
 *
 * @testType performanceTest
 * This test may fail if the system is under heavy load as the test depends on the system performance.
 */
TEST_F(DiscreteClockTest, testClockTimeEventAFAP)
{
    using namespace std::chrono_literals;

    // Cycle time value does not matter in case of this test due to time factor being 0.
    // This test succeeds if event durations are below 2 ms as this durations can not be
    // reached when using a time factor != 0.
    const Duration cycle_time{ duration_cast<Duration>(20ms) },
        expected_event_duration{ duration_cast<Duration>(20ms) },
        allowed_deviation{ duration_cast<Duration>(20ms) };
    const size_t clock_cycles = 3;
    const double time_factor = 0;
    std::shared_ptr<EventSinkTimeEventFrequency> _event_sink_mock = std::make_shared<EventSinkTimeEventFrequency>(clock_cycles);

    local_system_sim_clock.updateConfiguration(cycle_time, time_factor);

    // actual test
    {
        local_system_sim_clock.start(_event_sink_mock);
        std::mutex mutex;
        {
            std::unique_lock<std::mutex> lock(mutex);
            _event_sink_mock->_expected_calls_reached.wait_for(lock, 1000ms);
        }
        local_system_sim_clock.stop();

        EXPECT_EQ(clock_cycles, _event_sink_mock->_call_durations.size());
        _event_sink_mock->assertTimeEventDeviation(expected_event_duration, allowed_deviation);
    }
}