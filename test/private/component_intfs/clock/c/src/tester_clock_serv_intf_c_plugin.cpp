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

#include <fep3/fep3_errors.h>
#include <fep3/plugin/c/c_host_plugin.h>
#include <fep3/participant/component_factories/c/component_creator_c_plugin.h>
#include <fep3/components/clock/mock/mock_transferable_clock_service_with_access_to_clocks.h>
#include <fep3/components/clock/mock/mock_clock_with_access_to_event_sink.h>
#include <fep3/components/clock/mock/mock_clock_service.h>
#include <fep3/components/clock/c_access_wrapper/clock_service_c_access_wrapper.h>
#include <helper/component_c_plugin_helper.h>

const std::string test_plugin_1_path = PLUGIN;

using namespace fep3::plugin::c::arya;
using namespace fep3::plugin::c::access::arya;

ACTION_P(CheckClock, pointer_to_reference_clock)
{
    if(arg0->getName() != pointer_to_reference_clock->getName())
    {
        RETURN_ERROR_DESCRIPTION(::fep3::ERR_INVALID_ARG, "clock does not match");
    }
    return {};
}

ACTION_P(CheckEventSink, pointer_to_reference_event_sink)
{
    // Note: the event sink has no identifier (like e. g. a name), so we cannot check
    // for equality by such identifier. Therefore we call any method of the event sink
    // and check if the reference is called.
    EXPECT_CALL(*pointer_to_reference_event_sink, timeUpdateBegin(::fep3::Timestamp(1), ::fep3::Timestamp(2)))
        .Times(1);
    if(const auto& event_sink = arg0.lock())
    {
        event_sink->timeUpdateBegin(::fep3::Timestamp(1), ::fep3::Timestamp(2));
        return {};
    }
    else
    {
        RETURN_ERROR_DESCRIPTION(::fep3::ERR_INVALID_ARG, "event sink does not match");
    }
}

struct Plugin1PathGetter
{
    std::string operator()() const
    {
        return test_plugin_1_path;
    }
};
struct SetMockComponentFunctionSymbolGetter
{
    std::string operator()() const
    {
        return "setMockClockService";
    }
};

/**
 * Test fixture loading a mocked clock service from within a C plugin
 */
using ClockServiceLoader = MockedComponentCPluginLoader
    <::fep3::IClockService
    , fep3::mock::ClockService<fep3::plugin::c::TransferableComponentBase>
    , ::fep3::plugin::c::access::arya::ClockService
    , Plugin1PathGetter
    , SetMockComponentFunctionSymbolGetter
    >;
using ClockServiceLoaderFixture = MockedComponentCPluginLoaderFixture<ClockServiceLoader>;

/**
 * Test method fep3::IClockService::getTime of a clock service
 * that resides in a C plugin
 * @req_id TODO
 */
TEST_F(ClockServiceLoaderFixture, testMethod_getTime)
{
    const auto& test_clock_name = std::string("test_clock");
    const auto& test_timestamp = ::fep3::Optional<::fep3::Timestamp>(::fep3::Timestamp(2));

    // setting of expectations
    {
        auto& mock_clock_service = getMockComponent();

        EXPECT_CALL(mock_clock_service, getTime())
            .WillOnce(::testing::Return(::fep3::Timestamp(1)));
        EXPECT_CALL(mock_clock_service, getTime(test_clock_name))
            .WillOnce(::testing::Return(test_timestamp));
    }
    ::fep3::arya::IClockService* clock_service = getComponent();
    ASSERT_NE(nullptr, clock_service);
    EXPECT_EQ(::fep3::Timestamp(1), clock_service->getTime());
    EXPECT_EQ(test_timestamp, clock_service->getTime(test_clock_name));
}

/**
 * Test method fep3::IClockService::getType of a clock service
 * that resides in a C plugin
 * @req_id TODO
 */
TEST_F(ClockServiceLoaderFixture, testMethod_getType)
{
    const auto& test_clock_name = std::string("test_clock");
    const auto& test_clock_type = ::fep3::Optional<::fep3::IClock::ClockType>(::fep3::IClock::ClockType::discrete);

    // setting of expectations
    {
        auto& mock_clock_service = getMockComponent();

        EXPECT_CALL(mock_clock_service, getType())
            .WillOnce(::testing::Return(::fep3::IClock::ClockType::continuous));
        EXPECT_CALL(mock_clock_service, getType(test_clock_name))
            .WillOnce(::testing::Return(test_clock_type));
    }
    ::fep3::arya::IClockService* clock_service = getComponent();
    ASSERT_NE(nullptr, clock_service);
    EXPECT_EQ(::fep3::IClock::ClockType::continuous, clock_service->getType());
    EXPECT_EQ(test_clock_type, clock_service->getType(test_clock_name));
}

/**
 * Test method fep3::IClockService::getMainClockName of a clock service
 * that resides in a C plugin
 * @req_id TODO
 */
TEST_F(ClockServiceLoaderFixture, testMethod_getMainClockName)
{
    const auto& test_clock_name = std::string("test_clock");

    // setting of expectations
    {
        auto& mock_clock_service = getMockComponent();

        EXPECT_CALL(mock_clock_service, getMainClockName())
            .WillOnce(::testing::Return(test_clock_name));
    }
    ::fep3::arya::IClockService* clock_service = getComponent();
    ASSERT_NE(nullptr, clock_service);
    EXPECT_EQ(test_clock_name, clock_service->getMainClockName());
}

/**
 * Test method fep3::IClockService::registerEventSink of a clock service
 * that resides in a C plugin
 * @req_id TODO
 */
TEST_F(ClockServiceLoaderFixture, testMethod_registerEventSink)
{
    auto mock_event_sink = std::make_shared<::testing::StrictMock<::fep3::mock::EventSink>>();
    // setting of expectations
    {
        auto& mock_clock_service = getMockComponent();

        // Note: Using a gMock matcher would suit better here than an action, but a matcher 
        // must not have any side effects and invoking a mock method has a side effect.
        EXPECT_CALL(mock_clock_service, registerEventSink(::testing::_))
            .WillOnce(CheckEventSink(mock_event_sink.get()));
    }
    ::fep3::arya::IClockService* clock_service = getComponent();
    ASSERT_NE(nullptr, clock_service);
    EXPECT_EQ(::fep3::Result{}, clock_service->registerEventSink(std::move(mock_event_sink)));
}

/**
 * Test method fep3::IClockService::unregisterEventSink of a clock service
 * that resides in a C plugin
 * @req_id TODO
 */
TEST_F(ClockServiceLoaderFixture, testMethod_unregisterEventSink)
{
    auto mock_event_sink = std::make_shared<::testing::StrictMock<::fep3::mock::EventSink>>();
    // setting of expectations
    {
        auto& mock_clock_service = getMockComponent();

        // Note: Using a gMock matcher would suit better here than an action, but a matcher 
        // must not have any side effects and invoking a mock method has a side effect.
        EXPECT_CALL(mock_clock_service, unregisterEventSink(::testing::_))
            .WillOnce(CheckEventSink(mock_event_sink.get()));
    }
    ::fep3::arya::IClockService* clock_service = getComponent();
    ASSERT_NE(nullptr, clock_service);
    EXPECT_EQ(::fep3::Result{}, clock_service->unregisterEventSink(std::move(mock_event_sink)));
}

/**
 * Test method fep3::IClockRegistry::registerClock of a clock service
 * that resides in a C plugin
 * @req_id TODO
 */
TEST_F(ClockServiceLoaderFixture, testMethod_registerClock)
{
    // Note: We are not testing the IClock interface in this test, so we use NiceMock and don't set expectations.
    auto mock_clock = std::make_shared<::testing::NiceMock<::fep3::mock::Clock>>();
    // setting of expectations
    {
        auto& mock_clock_service = getMockComponent();

        // Note: Using a gMock matcher would suit better here than an action, but a matcher 
        // must not have any side effects and invoking a mock method has a side effect.
        EXPECT_CALL(mock_clock_service, registerClock(::testing::_))
            .WillOnce(CheckClock(mock_clock.get()));
    }
    ::fep3::arya::IClockService* clock_service = getComponent();
    ASSERT_NE(nullptr, clock_service);
    EXPECT_EQ(::fep3::Result{}, clock_service->registerClock(std::move(mock_clock)));
}

/**
 * Test method fep3::IClockRegistry::unregisterClock of a clock service
 * that resides in a C plugin
 * @req_id TODO
 */
TEST_F(ClockServiceLoaderFixture, testMethod_unregisterClock)
{
    const auto& test_clock_name = std::string("test_clock");
    
    // setting of expectations
    {
        auto& mock_clock_service = getMockComponent();

        EXPECT_CALL(mock_clock_service, unregisterClock(test_clock_name))
            .WillOnce(::testing::Return(::fep3::Result{}));
    }
    ::fep3::arya::IClockService* clock_service = getComponent();
    ASSERT_NE(nullptr, clock_service);
    EXPECT_EQ(::fep3::Result{}, clock_service->unregisterClock(test_clock_name));
}

/**
 * Test method fep3::IClockRegistry::getClockNames of a clock service
 * that resides in a C plugin
 * @req_id TODO
 */
TEST_F(ClockServiceLoaderFixture, testMethod_getClockNames)
{
    const auto& test_clock_names = std::list<std::string>
        {"test_clock_1"
        , "test_clock_2"
        , "test_clock_3"
        };

    // setting of expectations
    {
        auto& mock_clock_service = getMockComponent();

        EXPECT_CALL(mock_clock_service, getClockNames())
            .WillOnce(::testing::Return(test_clock_names));
    }
    ::fep3::arya::IClockService* clock_service = getComponent();
    ASSERT_NE(nullptr, clock_service);
    EXPECT_EQ(test_clock_names, clock_service->getClockNames());
}

/**
 * Test fixture loading a mocked clock service from within a C plugin
 * with additional access to registered clocks
 */
using ClockServiceLoaderWithAccessToClocks = MockedComponentCPluginLoader
    <::fep3::IClockService
    , ::fep3::mock::TransferableClockServiceWithAccessToClocks
    , ::fep3::plugin::c::access::arya::ClockService
    , Plugin1PathGetter
    , SetMockComponentFunctionSymbolGetter
    >;
    
/**
 * Test fixture class loading a single mocked component from within a C plugin
 */
class ClockServiceWithAccessToClocksLoaderFixture 
    : public ::testing::Test
    , public ClockServiceLoaderWithAccessToClocks
{
protected:
    void SetUp() override
    {
        ClockServiceLoaderWithAccessToClocks::SetUp();
        _get_clocks_function = getPlugin()->get<fep3::IClock*(size_t)>("getClock");
        ASSERT_NE(nullptr, _get_clocks_function);
    }
    
    ::fep3::IClock* getClock(size_t index) const
    {
        return _get_clocks_function(index);
    }
private:
    ::fep3::IClock*(*_get_clocks_function)(size_t);
};

/**
 * Test the interface fep3::IClock of a clock that resides in a C plugin
 * @req_id TODO
 */
TEST_F(ClockServiceWithAccessToClocksLoaderFixture, testClockInterface)
{
    const auto& test_clock_name = std::string("test_clock");
    const auto& test_clock_type = ::fep3::IClock::ClockType(::fep3::IClock::ClockType::continuous);
    const auto& test_time = ::fep3::Timestamp{1};
    auto mock_clock = std::make_unique<::testing::StrictMock<::fep3::mock::Clock>>();
    // setting of expectations
    {
        auto& mock_clock_service = getMockComponent();
        ::testing::InSequence call_sequence;

        EXPECT_CALL(mock_clock_service, registerClock(::testing::_))
            .WillOnce(::testing::Return(::fep3::Result{}));

        EXPECT_CALL(*mock_clock.get(), getName())
            .WillOnce(::testing::Return(test_clock_name));
        EXPECT_CALL(*mock_clock.get(), getType())
            .WillOnce(::testing::Return(test_clock_type));
        EXPECT_CALL(*mock_clock.get(), getTime())
            .WillOnce(::testing::Return(test_time));
        EXPECT_CALL(*mock_clock.get(), reset())
            .WillOnce(::testing::Return());
        EXPECT_CALL(*mock_clock.get(), start(::testing::_))
            .WillOnce(::testing::Return());
        EXPECT_CALL(*mock_clock.get(), stop())
            .WillOnce(::testing::Return());
    }
    ::fep3::arya::IClockService* clock_service = getComponent();
    ASSERT_NE(nullptr, clock_service);
    EXPECT_EQ(::fep3::Result{}, clock_service->registerClock(std::move(mock_clock)));

    const auto& clock = getClock(0);
    // we registered one clock before
    ASSERT_NE(nullptr, clock);
    
    // now test the interface IClock of a clock that resides in a C plugin
    EXPECT_EQ(test_clock_name, clock->getName());
    EXPECT_EQ(test_clock_type, clock->getType());
    EXPECT_EQ(test_time, clock->getTime());
    clock->reset();
    // Note: We are not testing the IClock::IEventSink interface in this test, so we use NiceMock and don't set expectations.
    clock->start(std::make_shared<::testing::NiceMock<::fep3::mock::EventSink>>());
    clock->stop();
}

/**
 * Test the interface fep3::IClock::IEventSink of an event sink that resides in a C plugin
 * @req_id TODO
 */
TEST_F(ClockServiceWithAccessToClocksLoaderFixture, testEventSinkInterface)
{
    const auto& mock_event_sink = std::make_shared<::testing::StrictMock<::fep3::mock::EventSink>>();
    // Note: We are not testing the IClock interface in this test, so we use NiceMock and don't set expectations.
    auto mock_clock = std::make_unique<::testing::NiceMock<::fep3::mock::ClockWithAccessToEventSink>>();
    const auto& pointer_to_mock_clock = mock_clock.get();
    // setting of expectations
    {
        auto& mock_clock_service = getMockComponent();
        ::testing::InSequence call_sequence;

        EXPECT_CALL(mock_clock_service, registerClock(::testing::_))
            .WillOnce(::testing::Return(::fep3::Result{}));
            
        EXPECT_CALL(*mock_event_sink.get(), timeUpdateBegin(::fep3::Timestamp(1), ::fep3::Timestamp(2)))
            .WillOnce(::testing::Return());
        EXPECT_CALL(*mock_event_sink.get(), timeUpdating(::fep3::Timestamp(3)))
            .WillOnce(::testing::Return());
        EXPECT_CALL(*mock_event_sink.get(), timeUpdateEnd(::fep3::Timestamp(4)))
            .WillOnce(::testing::Return());
        EXPECT_CALL(*mock_event_sink.get(), timeResetBegin(::fep3::Timestamp(5), ::fep3::Timestamp(6)))
            .WillOnce(::testing::Return());
        EXPECT_CALL(*mock_event_sink.get(), timeResetEnd(::fep3::Timestamp(7)))
            .WillOnce(::testing::Return());
    }
    ::fep3::arya::IClockService* clock_service = getComponent();
    ASSERT_NE(nullptr, clock_service);
    EXPECT_EQ(::fep3::Result{}, clock_service->registerClock(std::move(mock_clock)));

    const auto& clock = getClock(0);
    // we registered one clock before
    ASSERT_NE(nullptr, clock);
    
    clock->start(mock_event_sink);
    const auto& event_sink_of_clock = pointer_to_mock_clock->getEventSink().lock();
    ASSERT_TRUE(event_sink_of_clock);
    
    // now test the interface IClock::IEventSink of an event sink that resides in a C plugin
    event_sink_of_clock->timeUpdateBegin(::fep3::Timestamp(1), ::fep3::Timestamp(2));
    event_sink_of_clock->timeUpdating(::fep3::Timestamp(3));
    event_sink_of_clock->timeUpdateEnd(::fep3::Timestamp(4));
    event_sink_of_clock->timeResetBegin(::fep3::Timestamp(5), ::fep3::Timestamp(6));
    event_sink_of_clock->timeResetEnd(::fep3::Timestamp(7));
}