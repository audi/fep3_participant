/**
 * @file
 * Copyright &copy; Audi AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 */

#include <chrono>

#include <gtest/gtest.h>

#include <fep3/native_components/clock_sync/interpolation_time.h>

using namespace fep3;
using namespace fep3::arya;
using namespace std::chrono;

/**
 * @detail Test whether the interpolation time clock provides 0 if no time has been set yet.
 *
 */
TEST(InterpolationTimeTest, NullIfNoTimeSet)
{
    const InterpolationTime   interpolation_time;

    // actual test case
    {
        ASSERT_EQ(0, interpolation_time.getTime().count());
    }
}

/**
 * @detail Test whether the interpolation time clock provides a valid interpolated time.
 * Validity is checked against an allowed max deviation value.
 * @req_id FEPSDK-2442
 */
TEST(InterpolationTimeTest, ProvideInterpolatedTime)
{
    InterpolationTime   interpolation_time;
    const Timestamp           master_time{ duration_cast<nanoseconds>(10ms) },
        round_trip_time{ duration_cast<nanoseconds>(2ms) },
        allowed_deviation{ duration_cast<nanoseconds>(1ms) };
    const auto expected_interpolated_time = Timestamp{ (master_time + round_trip_time / 2) };

    // actual test case
    {
        interpolation_time.setTime(master_time, round_trip_time);
        const auto interpolated_time = interpolation_time.getTime();

        EXPECT_NEAR(
            static_cast<double>(interpolated_time.count()),
            static_cast<double>(expected_interpolated_time.count()),
            static_cast<double>(allowed_deviation.count()));
    }
}

/**
 * @detail Test whether the interpolation time clock provides a valid time after reset.
 * Validity is checked against an allowed max deviation value.
 *
 */
TEST(InterpolationTimeTest, ProvideNonInterpolatedTimeAfterReset)
{
    InterpolationTime   interpolation_time;
    const Timestamp           reset_time{ duration_cast<nanoseconds>(10ms) },
        allowed_deviation{ duration_cast<nanoseconds>(1ms) };

    // actual test case
    {
        interpolation_time.resetTime(reset_time);
        const auto non_interpolated_time = interpolation_time.getTime();
        
        EXPECT_NEAR(
            static_cast<double>(non_interpolated_time.count()),
            static_cast<double>(reset_time.count()),
            static_cast<double>(allowed_deviation.count()));
    }
}