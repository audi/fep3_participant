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

#include <string>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

#include <fep3/native_components/clock_sync/master_on_demand_clock_client.h>
#include <fep3/components/logging/mock/mock_logging_service.h>
#include <fep3/components/clock/mock/mock_clock_service.h>
#include <fep3/components/clock_sync/mock/mock_clock_sync_service.h>
#include <fep3/components/service_bus/mock/mock_service_bus.h>
#include <fep3/core/element_base.h>
#include <fep3/native_components/clock_sync/clock_sync_service.h>
#include <fep3/native_components/service_bus/testing/service_bus_testing.hpp>

using namespace ::testing;
using namespace fep3;
using namespace fep3::arya;
using namespace fep3::rpc::arya;
using namespace fep3::core::arya;

using Logger = NiceMock<mock::Logger>;
using EventSink = NiceMock<mock::EventSink>;
using RPCRequester = StrictMock<mock::RPCRequester>;
using RPCServer = NiceMock<mock::RPCServer>;
using InterpolationTimeMock = StrictMock<mock::InterpolationTime>;

struct NativeClockSyncClientTest : public Test
{
    NativeClockSyncClientTest()
        : _interpolation_time_mock{new InterpolationTimeMock}
        , _interpolation_time_mock_unique_ptr{_interpolation_time_mock}
        , _logger_mock(std::make_shared<Logger>())
        , _event_sink_mock(std::make_shared<EventSink>())
        , _rpc_requester_mock(std::make_shared<RPCRequester>())
        , _rpc_server_mock(std::make_shared<RPCServer>())
    {
    }

    InterpolationTimeMock* _interpolation_time_mock;
    std::unique_ptr<InterpolationTimeMock> _interpolation_time_mock_unique_ptr;
    std::shared_ptr<Logger> _logger_mock;
    std::shared_ptr<EventSink> _event_sink_mock;
    std::shared_ptr<RPCRequester> _rpc_requester_mock;
    std::shared_ptr<RPCServer> _rpc_server_mock;
};

class MyElement : public fep3::core::ElementBase
{
public:
    MyElement() : ElementBase("test", "testversion")
    {
    }
};

std::string createMasterTypeReply(fep3::IClock::ClockType clock_type)
{
   return a_util::strings::format(R"({"id" : 1,"jsonrpc" : "2.0","result" : %i})", static_cast<int>(clock_type));
}

std::string createMasterTimeReply(int master_time)
{
    return a_util::strings::format(R"({"id" : 1,"jsonrpc" : "2.0","result" : "%i"})", master_time);
}

/**
 * @detail Test the clock sync client interpolating clock getNewTime functionality.
 * A time retrieved from the clock sync client clock MasterOnDemandClockInterpolating
 * shall be interpolated and therefore be retrieved from IInterpolationClock, in this
 * test a mock.
 * @req_id FEPSDK-2442
 */
TEST_F(NativeClockSyncClientTest, InterpolatingClockGetNewTime)
{
    std::mutex mutex;
    std::condition_variable condition_variable;
    const Timestamp new_time{ 0 };
    const Duration sync_cycle_time{ std::chrono::milliseconds{100} },
        timeout{ std::chrono::milliseconds(10) };
    auto master_on_demand_clock_interpolating = std::make_shared<MasterOnDemandClockInterpolating>(
        sync_cycle_time,
        _rpc_server_mock,
        _rpc_requester_mock,
        _logger_mock,
        std::move(_interpolation_time_mock_unique_ptr),
        fep3::native::testing::test_participant_name);
    const auto get_master_type_reply = createMasterTypeReply(IClock::ClockType::continuous);
    const auto get_master_time_reply = createMasterTimeReply(101);

    {
        std::unique_lock<std::mutex> lock(mutex);
        EXPECT_CALL(*_rpc_server_mock, registerService(_, _))
            .WillOnce(Return(ERR_NOERROR));
        EXPECT_CALL(*_rpc_server_mock, unregisterService(_))
            .WillOnce(Return(ERR_NOERROR));

        EXPECT_CALL(*_rpc_requester_mock, sendRequest(_, ContainsRegex("getMasterType"), _))
            .WillRepeatedly(DoAll(
                WithArg<2>(Invoke([get_master_type_reply](IRPCRequester::IRPCResponse& response) {
                    response.set(get_master_type_reply);
                })),
                Return(ERR_NOERROR)));
        EXPECT_CALL(*_rpc_requester_mock, sendRequest(_, ContainsRegex("registerSyncSlave"), _))
            .WillRepeatedly(DoAll(
                WithArg<2>(Invoke([get_master_type_reply](IRPCRequester::IRPCResponse& response) {
                    response.set(get_master_type_reply);
                })),
                Return(ERR_NOERROR)));
        EXPECT_CALL(*_rpc_requester_mock, sendRequest(_, ContainsRegex("getMasterTime"), _)).Times(AtLeast(1))
            .WillRepeatedly(DoAll(
                WithArg<2>(Invoke([get_master_time_reply, &condition_variable](IRPCRequester::IRPCResponse& response) {
                    response.set(get_master_time_reply);
                    condition_variable.notify_one();
                })),
                Return(ERR_NOERROR)));
    	
    	EXPECT_CALL(*_interpolation_time_mock, setTime(Duration{ 101 }, _)).Times(AtLeast(1));

        master_on_demand_clock_interpolating->startRPC();
        EXPECT_EQ(condition_variable.wait_for(lock, timeout), std::cv_status::no_timeout);
        master_on_demand_clock_interpolating->stopRPC();
    }
}

/**
 * @detail Test the clock sync client interpolating clock resetTime functionality.
 * A reset shall call the corresponding IInterpolation time functionaity, in this
 * test a mock.
 */
TEST_F(NativeClockSyncClientTest, InterpolatingClockResetTime)
{
    std::mutex mutex;
    std::condition_variable condition_variable;
    const Timestamp new_time{ 0 };
    const Duration sync_cycle_time{ std::chrono::milliseconds{100} };
    auto master_on_demand_clock_interpolating = std::make_shared<MasterOnDemandClockInterpolating>(
        sync_cycle_time,
        _rpc_server_mock,
        _rpc_requester_mock,
        _logger_mock,
        std::move(_interpolation_time_mock_unique_ptr),
        fep3::native::testing::test_participant_name);
    const auto get_master_type_reply = createMasterTypeReply(IClock::ClockType::continuous);

    {
        std::unique_lock<std::mutex> lock(mutex);

        EXPECT_CALL(*_interpolation_time_mock, resetTime(new_time)).Times(1);

        ASSERT_EQ(master_on_demand_clock_interpolating->resetTime(), new_time);
    }
}

/**
* @detail Test the clock sync interpolating clock exception handling during registration/
* deregistration to/from master.
* Following request exceptions are handled:
* * getMasterType
* * registerSyncSlave
* * unregisterSyncSlave
*
*/
TEST_F(NativeClockSyncClientTest, InterpolatingClockRegisterUnregisterToFromMasterExceptions)
{
    const Duration sync_cycle_time{ std::chrono::milliseconds{100} };
    auto master_on_demand_clock_interpolating = std::make_shared<MasterOnDemandClockInterpolating>(
        sync_cycle_time,
        _rpc_server_mock,
        _rpc_requester_mock,
        _logger_mock,
        std::move(_interpolation_time_mock_unique_ptr),
        fep3::native::testing::test_participant_name);

    {
        EXPECT_CALL(*_logger_mock, logWarning(HasSubstr("get master type exception")))
            .Times(AtLeast(1)).WillRepeatedly(Return(ERR_NOERROR));
        EXPECT_CALL(*_logger_mock, logWarning(HasSubstr("register sync slave exception")))
            .Times(AtLeast(1)).WillRepeatedly(Return(ERR_NOERROR));            
        EXPECT_CALL(*_logger_mock, logWarning(HasSubstr("unregister sync slave exception")))
            .Times(AtLeast(1)).WillRepeatedly(Return(ERR_NOERROR));
        EXPECT_CALL(*_rpc_server_mock, registerService(_, _))
            .Times(AtLeast(1)).WillRepeatedly(Return(ERR_NOERROR));
        EXPECT_CALL(*_rpc_requester_mock, sendRequest(_, ContainsRegex("getMasterType"), _))
            .Times(AtLeast(1)).WillRepeatedly(DoAll(
                WithArg<2>(testing::Invoke([](IRPCRequester::IRPCResponse& /*pResponse*/) {
                    throw std::runtime_error("get master type exception");
                })),
                Return(ERR_NOERROR)));
        EXPECT_CALL(*_rpc_requester_mock, sendRequest(_, ContainsRegex("registerSyncSlave"), _))
            .Times(AtLeast(1)).WillRepeatedly(DoAll(
                WithArg<2>(testing::Invoke([](IRPCRequester::IRPCResponse& /*pResponse*/) {
                    throw std::runtime_error("register sync slave exception");
                })),
                Return(ERR_NOERROR)));
        EXPECT_CALL(*_rpc_requester_mock, sendRequest(_, ContainsRegex("unregisterSyncSlave"), _))
            .Times(AtLeast(1)).WillRepeatedly(DoAll(
                WithArg<2>(testing::Invoke([](IRPCRequester::IRPCResponse& /*pResponse*/) {
                    throw std::runtime_error("unregister sync slave exception");
                })),
                Return(ERR_NOERROR)));
        EXPECT_CALL(*_rpc_server_mock, unregisterService(_))
            .Times(AtLeast(1)).WillRepeatedly(Return(ERR_NOERROR));
       
        master_on_demand_clock_interpolating->startRPC();

        EXPECT_CALL(*_interpolation_time_mock, resetTime(_));
        master_on_demand_clock_interpolating->start(_event_sink_mock);
        master_on_demand_clock_interpolating->stop();
        master_on_demand_clock_interpolating->stopRPC();
    }
}

/**
 * @detail Test the clock sync client discrete clock.
 *
 */
TEST_F(NativeClockSyncClientTest, DiscreteClockBase)
{
    const Timestamp new_time{ 100 }, old_time{ 0 };
    const Duration sync_cycle_time{ std::chrono::milliseconds{100} };
    auto master_on_demand_clock_discrete = std::make_shared<MasterOnDemandClockDiscrete>(
        sync_cycle_time,
        _rpc_server_mock,
        _rpc_requester_mock,
        true,
        _logger_mock,
        "test_participant_name");

    const auto master_type_reply = createMasterTypeReply(IClock::ClockType::discrete);
    const auto register_slave_reply = R"({"id" : 1,"jsonrpc" : "2.0","result" : 0})";
    const auto unregister_slave_reply = R"({"id" : 1,"jsonrpc" : "2.0","result" : 0})";

    {
        EXPECT_CALL(*_rpc_server_mock, registerService(_, _))
            .WillOnce(Return(ERR_NOERROR));
        EXPECT_CALL(*_rpc_requester_mock, sendRequest(_, ContainsRegex("getMasterType"), _))
            .WillOnce(DoAll(
                WithArg<2>(Invoke([master_type_reply](IRPCRequester::IRPCResponse& response) {
                    response.set(master_type_reply);
                })),
                Return(ERR_NOERROR)));
        EXPECT_CALL(*_rpc_requester_mock, sendRequest(_, ContainsRegex("registerSyncSlave"), _))
            .WillOnce(DoAll(
                WithArg<2>(Invoke([register_slave_reply](IRPCRequester::IRPCResponse& response) {
                    response.set(register_slave_reply);
                })),
                Return(ERR_NOERROR)));        
        
        // start of clock and explicit reset
        EXPECT_CALL(*_event_sink_mock, timeResetBegin(old_time, old_time)).Times(2);
        EXPECT_CALL(*_event_sink_mock, timeResetEnd(old_time)).Times(2);
  
        EXPECT_CALL(*_event_sink_mock, timeUpdateBegin(old_time, new_time)).Times(1);
        EXPECT_CALL(*_event_sink_mock, timeUpdating(new_time)).Times(1);
        EXPECT_CALL(*_event_sink_mock, timeUpdateEnd(new_time)).Times(1);
        EXPECT_CALL(*_rpc_requester_mock, sendRequest(_, ContainsRegex("unregisterSyncSlave"), _))
            .WillOnce(DoAll(
                WithArg<2>(Invoke([unregister_slave_reply](IRPCRequester::IRPCResponse& response) {
                    response.set(unregister_slave_reply);
                })),
                Return(ERR_NOERROR)));
        EXPECT_CALL(*_rpc_server_mock, unregisterService(_))
            .WillOnce(Return(ERR_NOERROR));

        master_on_demand_clock_discrete->startRPC();
        master_on_demand_clock_discrete->start(_event_sink_mock);
        master_on_demand_clock_discrete->masterTimeEvent(IRPCClockSyncMasterDef::EventID::time_reset, new_time, old_time);
        master_on_demand_clock_discrete->updateTime(new_time, Duration{ 0 });
        master_on_demand_clock_discrete->stop();
        master_on_demand_clock_discrete->stopRPC();
    }
}

/**
 * @detail Test the clock sync client discrete clock update time functionality.
 *
 */
TEST_F(NativeClockSyncClientTest, DiscreteClockMasterTimeEvent)
{
    const Timestamp new_time{ 100 }, old_time{ 0 };
    const Duration sync_cycle_time{ std::chrono::milliseconds{100} };
    auto master_on_demand_clock_discrete = std::make_shared<MasterOnDemandClockDiscrete>(
        sync_cycle_time,
        _rpc_server_mock,
        _rpc_requester_mock,
        true,
        _logger_mock,
        "test_participant_name");

    const auto master_type_reply = createMasterTypeReply(IClock::ClockType::discrete);
    const auto register_slave_reply = R"({"id" : 1,"jsonrpc" : "2.0","result" : 0})";
    const auto unregister_slave_reply = R"({"id" : 1,"jsonrpc" : "2.0","result" : 0})";

    {
        EXPECT_CALL(*_rpc_server_mock, registerService(_, _))
            .WillOnce(Return(ERR_NOERROR));
        EXPECT_CALL(*_rpc_requester_mock, sendRequest(_, ContainsRegex("getMasterType"), _))
            .WillOnce(DoAll(
                WithArg<2>(Invoke([master_type_reply](IRPCRequester::IRPCResponse& response) {
                    response.set(master_type_reply);
                })),
                Return(ERR_NOERROR)));
        EXPECT_CALL(*_rpc_requester_mock, sendRequest(_, ContainsRegex("registerSyncSlave"), _))
            .WillOnce(DoAll(
                WithArg<2>(Invoke([register_slave_reply](IRPCRequester::IRPCResponse& response) {
                    response.set(register_slave_reply);
                })),
                Return(ERR_NOERROR)));

        EXPECT_CALL(*_event_sink_mock, timeResetBegin(old_time, old_time)).Times(2);
        EXPECT_CALL(*_event_sink_mock, timeResetEnd(old_time)).Times(2);
        EXPECT_CALL(*_event_sink_mock, timeUpdateBegin(old_time, new_time)).Times(2);
        EXPECT_CALL(*_event_sink_mock, timeUpdating(new_time)).Times(1);
        EXPECT_CALL(*_event_sink_mock, timeUpdateEnd(new_time)).Times(2);
        EXPECT_CALL(*_rpc_requester_mock, sendRequest(_, ContainsRegex("unregisterSyncSlave"), _))
            .WillOnce(DoAll(
                WithArg<2>(Invoke([unregister_slave_reply](IRPCRequester::IRPCResponse& response) {
                    response.set(unregister_slave_reply);
                })),
                Return(ERR_NOERROR)));
        EXPECT_CALL(*_rpc_server_mock, unregisterService(_))
            .WillOnce(Return(ERR_NOERROR));

        master_on_demand_clock_discrete->startRPC();
        master_on_demand_clock_discrete->start(_event_sink_mock);
        master_on_demand_clock_discrete->masterTimeEvent(IRPCClockSyncMasterDef::EventID::time_reset, new_time, old_time);
        master_on_demand_clock_discrete->masterTimeEvent(IRPCClockSyncMasterDef::EventID::time_update_before, new_time, old_time);
        master_on_demand_clock_discrete->masterTimeEvent(IRPCClockSyncMasterDef::EventID::time_updating, new_time, old_time);
        master_on_demand_clock_discrete->masterTimeEvent(IRPCClockSyncMasterDef::EventID::time_update_after, new_time, old_time);
        master_on_demand_clock_discrete->stop();
        master_on_demand_clock_discrete->stopRPC();
    }
}
