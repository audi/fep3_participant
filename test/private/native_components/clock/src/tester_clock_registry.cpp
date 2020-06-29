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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <common/gtest_asserts.h>

#include <fep3/native_components/clock/local_clock_registry.h>
#include <fep3/components/logging/mock/mock_logging_service.h>
#include <fep3/components/clock/mock/mock_clock_service.h>

using namespace ::testing;
using namespace fep3::arya;

using Clock = NiceMock<fep3::mock::Clock>;
using Logger = NiceMock<fep3::mock::Logger>;
using LoggingService = fep3::mock::LoggingService;

struct ClockRegistryTest : public ::testing::Test
{
    ClockRegistryTest()
        : _clock_mock(new Clock())
        , _default_clock(std::make_shared<Clock>())
        , _logger(std::make_shared<Logger>())
    {
        ON_CALL(*_clock_mock.get(), getName())
            .WillByDefault(Return("my_clock"));

        ON_CALL(*_default_clock.get(), getName())
            .WillByDefault(Return("default_clock"));

    }

    std::shared_ptr<Clock> _clock_mock{};
    std::shared_ptr<Clock> _default_clock{};
    std::shared_ptr<Logger> _logger{};
};

/**
 * @detail Test whether the clock registry correctly registers and unregisters clock
 * @req_id FEPSDK-2114, FEPSDK-2115
 */
TEST_F(ClockRegistryTest, RegisterUnregisterClocks)
{
    fep3::native::LocalClockRegistry local_clock_registry;
    
    // actual test
    {
        ASSERT_FEP3_RESULT(local_clock_registry.registerClock(_default_clock), fep3::Result());
        ASSERT_EQ(local_clock_registry.getClockNames().size(), 1);

        ASSERT_FEP3_RESULT(local_clock_registry.registerClock(_clock_mock), fep3::Result());
        ASSERT_EQ(local_clock_registry.getClockNames().size(), 2);

        ASSERT_FEP3_RESULT(local_clock_registry.unregisterClock(_default_clock->getName()), fep3::Result());
        ASSERT_EQ(local_clock_registry.getClockNames().size(), 1);

        ASSERT_FEP3_RESULT(local_clock_registry.unregisterClock(_clock_mock->getName()), fep3::Result());
        ASSERT_EQ(local_clock_registry.getClockNames().size(), 0);
    }
}

/**
 * @detail Test whether the clock registry declines unregistering native clocks
 * @req_id FEPSDK-2434
 */
TEST_F(ClockRegistryTest, UnregisterNativeClocks)
{
    fep3::native::LocalClockRegistry local_clock_registry;
    local_clock_registry.setLogger(_logger);

    // actual test
    {
        EXPECT_CALL(*_logger, logError(_)).Times(2).WillRepeatedly(Return(::fep3::Result{}));

        ASSERT_FEP3_RESULT(local_clock_registry.unregisterClock(FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME), fep3::ERR_INVALID_ARG);
        ASSERT_FEP3_RESULT(local_clock_registry.unregisterClock(FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME), fep3::ERR_INVALID_ARG);
        
    }
}

/**
 * @detail Test whether the clock registry declines registering an invalid clock shared_ptr
 *
 */
TEST_F(ClockRegistryTest, RegisterInvalidClockPtr)
{
    fep3::native::LocalClockRegistry local_clock_registry;
    local_clock_registry.setLogger(_logger);
    std::shared_ptr<Clock> _clock_invalid_ptr;

    // actual test
    {
        EXPECT_CALL(*_logger, logError(_)).Times(1).WillOnce(::testing::Return(::fep3::Result{}));

        ASSERT_FEP3_RESULT(local_clock_registry.registerClock(_clock_invalid_ptr), fep3::ERR_POINTER);
        ASSERT_EQ(local_clock_registry.getClockNames().size(), 0);
    }
}

/**
 * @detail Test whether the clock registry declines registering a clock with the same name twice
 * @req_id FEPSDK-2135
 *
 */
TEST_F(ClockRegistryTest, RegisterClocksSameName)
{
    fep3::native::LocalClockRegistry local_clock_registry;
    local_clock_registry.setLogger(_logger);

    std::shared_ptr<Clock> _clock_same_name{ std::make_shared<Clock>() };
    ON_CALL(*_clock_same_name.get(), getName())
        .WillByDefault(Return(_clock_mock->getName()));

    // actual test
    {
        EXPECT_CALL(*_logger, logError(_)).Times(1).WillOnce(Return(::fep3::Result{}));

        ASSERT_FEP3_RESULT(local_clock_registry.registerClock(_clock_mock), fep3::Result());
        ASSERT_FEP3_RESULT(local_clock_registry.registerClock(_clock_same_name), fep3::ERR_INVALID_ARG);
        ASSERT_EQ(local_clock_registry.getClockNames().size(), 1);
    }
}

/**
 * @detail Test whether the clock registry declines unregistering a non existent clock
 *
 */
TEST_F(ClockRegistryTest, UnregisterNonExistentClock)
{
    fep3::native::LocalClockRegistry local_clock_registry;
    local_clock_registry.setLogger(_logger);

    // actual test
    {
        EXPECT_CALL(*_logger, logError(_)).Times(1).WillOnce(::testing::Return(::fep3::Result{}));

        ASSERT_FEP3_RESULT(local_clock_registry.unregisterClock("non-existent-clock"), fep3::ERR_INVALID_ARG);
    }
}

/**
 * @detail Test whether the clock registry returns a correct list of registered clocks
 * @req_id FEPSDK-2117
 */
TEST_F(ClockRegistryTest, GetClockList)
{
    fep3::native::LocalClockRegistry local_clock_registry;
    local_clock_registry.setLogger(_logger);
    const std::list<std::string> clock_list = { "default_clock", "my_clock" };

    // actual test
    {
        ASSERT_FEP3_RESULT(local_clock_registry.registerClock(_default_clock), fep3::Result());
        ASSERT_EQ(local_clock_registry.getClockNames().size(), 1);

        ASSERT_FEP3_RESULT(local_clock_registry.registerClock(_clock_mock), fep3::Result());
        ASSERT_EQ(local_clock_registry.getClockNames().size(), 2);

        ASSERT_EQ(local_clock_registry.getClockNames(), clock_list);
    }
}

/**
 * @detail Test whether the clock registry returns clocks by name
 * @req_id FEPSDK-2435
 */
TEST_F(ClockRegistryTest, GetClocksByName)
{
    fep3::native::LocalClockRegistry local_clock_registry;
    local_clock_registry.setLogger(_logger);

    // actual test
    {
        ASSERT_FEP3_RESULT(local_clock_registry.registerClock(_default_clock), fep3::Result());
        ASSERT_FEP3_RESULT(local_clock_registry.registerClock(_clock_mock), fep3::Result());

        ASSERT_EQ(local_clock_registry.findClock(_default_clock->getName()), _default_clock);
        ASSERT_EQ(local_clock_registry.findClock(_clock_mock->getName()), _clock_mock);
    }
}