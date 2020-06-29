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
#include <chrono>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fep3/components/clock/clock_base.h>
#include <fep3/native_components/clock/local_system_clock.h>
#include <fep3/components/clock/mock/mock_clock_service.h>
#include <fep3/components/clock/clock_service_intf.h>


using namespace ::testing;
using namespace fep3::arya;
using namespace fep3::native;
using namespace std::literals;

using EventSinkTimeEventValues = NiceMock<fep3::mock::EventSinkTimeEventValues>;

struct ContinuousClockTest : public Test
{
    ContinuousClockTest()
        : _event_sink_mock(std::make_shared<EventSinkTimeEventValues>(10))
    {

    }

    std::weak_ptr<EventSinkTimeEventValues> _event_sink_mock;
};

/**
 * @detail Test whether the clock provides a steadily rising time
 * @req_id FEPSDK-2108
 */
TEST_F(ContinuousClockTest, ClockProvidesSteadyTime)
{
    LocalSystemRealClock local_system_real_clock;
    Timestamp last_time(0), current_time(0);

    ASSERT_EQ(local_system_real_clock.getNewTime(), Timestamp(0));

    local_system_real_clock.resetTime();

    local_system_real_clock.start(_event_sink_mock);

    auto testidx = 0;
    //check if the clock is steady
    while (testidx < 10)
    {
        std::this_thread::sleep_for(1ms);
        current_time = local_system_real_clock.getNewTime();
        ASSERT_GT(current_time, last_time);
        last_time = current_time;
        testidx++;
    }

    local_system_real_clock.stop();
}

/**
 * @detail Test whether the clock can reset it's time
 *
 */
TEST_F(ContinuousClockTest, ClockReset)
{
    LocalSystemRealClock local_system_real_clock;
    Timestamp reset_time(0), reference_time(0);

    local_system_real_clock.resetTime();

    local_system_real_clock.start(_event_sink_mock);

    auto testidx = 0;
    //check if the clock correctly resets it's time
    while (testidx < 10)
    {
        std::this_thread::sleep_for(1ms);
        reference_time = local_system_real_clock.getNewTime();
        reset_time = local_system_real_clock.resetTime();
        ASSERT_GT(reference_time, reset_time);
        testidx++;
    }

    local_system_real_clock.stop();
}