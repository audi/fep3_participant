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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <testenvs/clock_sync_service_envs.h>

#include <fep3/native_components/service_bus/testing/service_bus_testing.hpp>
#include <fep3/rpc_services/clock_sync/clock_sync_master_client_stub.h>
#include <fep3/cpp/participant.h>
#include <fep3/core/participant_executor.hpp>
#include "fep3/components/scheduler/scheduler_service_intf.h"
#include <fep3/components/clock_sync/mock/mock_clock_sync_service.h>
#include <fep3/components/clock/mock/mock_clock_service.h>
#include <fep3/components/configuration/mock/mock_configuration_service.h>
#include "fep3/components/service_bus/service_bus_intf.h"
#include "fep3/native_components/clock_sync/master_on_demand_clock_client.h"
#include "fep3/rpc_services/clock_sync/clock_sync_service_rpc_intf_def.h"
#include "fep3/rpc_services/clock_sync/clock_sync_slave_client_stub.h"

using namespace ::testing;
using namespace fep3;
using namespace fep3::native;

using LoggerMock = StrictMock<fep3::mock::Logger>;
using LoggingService = fep3::mock::LoggingService;
using EventSinkMock = NiceMock<fep3::mock::EventSink>;
using RPCClockSyncMasterMock = StrictMock<fep3::mock::RPCClockSyncMaster>;
using ClockServiceComponentMock = StrictMock<fep3::mock::ClockService<>>;
using ConfigurationServiceComponentMock = StrictMock<fep3::mock::ConfigurationServiceComponent>;
using PropertyNodeMock = StrictMock<fep3::mock::PropertyNode>;

/**
 * A clock sync master rpc proxy client.
 */
class ClockSyncMasterProxy : public fep3::rpc::RPCServiceClient<fep3::rpc_stubs::RPCClockSyncMasterClientStub, fep3::rpc::IRPCClockSyncMasterDef>
{
private:
    typedef fep3::rpc::RPCServiceClient<fep3::rpc_stubs::RPCClockSyncMasterClientStub, fep3::rpc::IRPCClockSyncMasterDef> base_type;

public:
    using base_type::GetStub;

    ClockSyncMasterProxy(const char* server_object_name,
        const std::shared_ptr<fep3::rpc::IRPCRequester> rpc)
        : base_type(server_object_name, rpc)
    {
    }
};

/**
 * A clock sync slave rpc proxy client.
 */
class ClockSyncSlaveProxy : public fep3::rpc::RPCServiceClient<fep3::rpc_stubs::RPCClockSyncSlaveClientStub, fep3::rpc::IRPCClockSyncSlaveDef>
{
private:
    typedef fep3::rpc::RPCServiceClient<fep3::rpc_stubs::RPCClockSyncSlaveClientStub, fep3::rpc::IRPCClockSyncSlaveDef> base_type;

public:
    using base_type::GetStub;

    ClockSyncSlaveProxy(const char* server_object_name,
        const std::shared_ptr<fep3::rpc::IRPCRequester> rpc)
        : base_type(server_object_name, rpc)
    {
    }
};

/**
 * Clock sync rpc service base.
 */
struct ClockSynchronizationBase : public ::testing::Test
{
    ClockSynchronizationBase()
        : _service_bus{ std::make_shared<ServiceBus>() }
        , _logger(std::make_shared<LoggerMock>())
        , _component_registry{ std::make_shared<ComponentRegistry>() }
        , _configuration_service_mock{std::make_shared<ConfigurationServiceComponentMock>()}
    {
    }

    void SetUp() override
    {
        ASSERT_TRUE(fep3::native::testing::prepareServiceBusForTestingDefault(*_service_bus));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<IServiceBus>(_service_bus));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<ILoggingService>(std::make_shared<LoggingService>(_logger)));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<IConfigurationService>(_configuration_service_mock));
    }

    std::shared_ptr<ServiceBus> _service_bus{};
    std::shared_ptr<LoggerMock> _logger{};
    std::shared_ptr<ComponentRegistry> _component_registry{};
    std::shared_ptr<ConfigurationServiceComponentMock> _configuration_service_mock{};
};

/**
 * A clock sync master rpc service.
 */
struct ClockSyncMasterService : public ClockSynchronizationBase
{
    ClockSyncMasterService()
        : ClockSynchronizationBase()
    {
    }

    void SetUp() override
    {
        ClockSynchronizationBase::SetUp();

        EXPECT_CALL(*_configuration_service_mock, registerNode(_)).Times(1).WillOnce(Return(Result()));

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<IClockService>(std::make_shared<LocalClockService>()));

        ASSERT_FEP3_NOERROR(_component_registry->create());
        ASSERT_FEP3_NOERROR(_component_registry->initialize());
        ASSERT_FEP3_NOERROR(_component_registry->tense());
    }
};

/**
 * A base clock sync slave rpc service.
 */
struct ClockSyncSlaveServiceBase : public ClockSynchronizationBase
{
    ClockSyncSlaveServiceBase()
        : ClockSynchronizationBase()
        , _clock_service_mock(std::make_shared<ClockServiceComponentMock>())
        , _rpc_clock_sync_master_mock(std::make_shared<RPCClockSyncMasterMock>())
        , _event_sink_mock(std::make_shared<EventSinkMock>())
        , _sync_service_impl(std::make_shared<ClockSynchronizationService>())
        , _property_node_mock(std::make_shared<PropertyNodeMock>())
    {
    }

    void SetUp() override
    {
        ClockSynchronizationBase::SetUp();

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<IClockService>(_clock_service_mock));
        _sync_service_impl = std::make_shared<ClockSynchronizationService>();
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<IClockSyncService>(
            _sync_service_impl));

        EXPECT_CALL(*_configuration_service_mock, registerNode(_)).Times(1).WillOnce(
            DoAll(
                WithArg<0>(
                    Invoke([&](
                        const std::shared_ptr<IPropertyNode>& node)
                            {
                                _clock_sync_service_property_node = node;
                            })),
                            Return(Result())));

        EXPECT_CALL(*_clock_service_mock, registerClock(_)).WillRepeatedly(
            DoAll(
                WithArg<0>(
                    Invoke([&](
                        const std::shared_ptr<IClock>& clock)
                            {
                                _synchronization_clock = clock;
                            })),
                            Return(Result())));

        EXPECT_CALL(*_clock_service_mock, start()).WillRepeatedly(Invoke([&]()
        {
            _synchronization_clock->start(_event_sink_mock); return Result{};
        }));


        EXPECT_CALL(*_clock_service_mock, stop()).WillRepeatedly(Invoke([&]()
        {
            _synchronization_clock->stop(); return Result{};
        }));

        ASSERT_FEP3_NOERROR(_component_registry->create());

        ASSERT_FEP3_NOERROR(_service_bus->getServer()->registerService(fep3::rpc::IRPCClockSyncMasterDef::getRPCDefaultName(),
            _rpc_clock_sync_master_mock));

        setPropertyValue(*_clock_sync_service_property_node->getChild(
            FEP3_TIMING_MASTER_PROPERTY),
            native::testing::test_participant_name);
    }

    std::shared_ptr<ClockServiceComponentMock> _clock_service_mock;
    std::shared_ptr<RPCClockSyncMasterMock> _rpc_clock_sync_master_mock;
    std::shared_ptr<EventSinkMock> _event_sink_mock;
    std::shared_ptr<ClockSynchronizationService> _sync_service_impl;
    std::shared_ptr<PropertyNodeMock> _property_node_mock;
    std::shared_ptr<IPropertyNode> _clock_sync_service_property_node;
    std::shared_ptr<IClock> _synchronization_clock{};
};

/**
 * A clock sync slave rpc service using a continuous clock.
 */
struct ContinuousClockSyncSlaveService : public ClockSyncSlaveServiceBase
{
    ContinuousClockSyncSlaveService()
        : ClockSyncSlaveServiceBase()
    {
    }

    void SetUp() override
    {
        ClockSyncSlaveServiceBase::SetUp();

        EXPECT_CALL(*_configuration_service_mock, getNode(FEP3_CLOCK_SERVICE_MAIN_CLOCK)).WillRepeatedly(Return(_property_node_mock));
        EXPECT_CALL(*_property_node_mock, getValue()).WillRepeatedly(Return(FEP3_CLOCK_SLAVE_MASTER_ONDEMAND));         

        ASSERT_FEP3_NOERROR(_component_registry->initialize());
        ASSERT_FEP3_NOERROR(_component_registry->tense());
    }
};

/**
 * A clock sync slave rpc service using a discrete clock.
 */
struct DiscreteClockSyncSlaveService : public ClockSyncSlaveServiceBase
{
    DiscreteClockSyncSlaveService()
        : ClockSyncSlaveServiceBase()
    {
    }

    void SetUp() override
    {
        ClockSyncSlaveServiceBase::SetUp();

        EXPECT_CALL(*_configuration_service_mock, getNode(FEP3_CLOCK_SERVICE_MAIN_CLOCK)).WillRepeatedly(Return(_property_node_mock));
        EXPECT_CALL(*_property_node_mock, getValue()).WillRepeatedly(Return(FEP3_CLOCK_SLAVE_MASTER_ONDEMAND_DISCRETE));

        ASSERT_FEP3_NOERROR(_component_registry->initialize());
        ASSERT_FEP3_NOERROR(_component_registry->tense());
    }
};

/**
 * @detail Test whether the clock sync master rpc service successfully registers/unregisters a sync slave.
 *
 * The service bus component provides a requester for the clock sync master proxy while the
 * component registry acts as slave ("test_participant_name").
 */
TEST_F(ClockSyncMasterService, testRegisterUnregisterSyncSlave)
{
    using namespace native::testing;

    const auto result_expected = 0;
    ClockSyncMasterProxy client(fep3::rpc::IRPCClockSyncMasterDef::getRPCDefaultName(),
        _service_bus->getRequester(test_participant_name));

    // actual test
    {
        ASSERT_EQ(client.registerSyncSlave(
            static_cast<int>(fep3::rpc::arya::IRPCClockSyncMasterDef::EventIDFlag::register_for_time_updating),
            test_participant_name),
            result_expected);
        ASSERT_EQ(client.unregisterSyncSlave(test_participant_name), result_expected);
    }
}

/**
 * @detail Test whether the clock sync master rpc service returns an error if a non existent slave is unregistered.
 *
 * The service bus component provides a requester for the clock sync master proxy while the
 * component registry acts as slave ("test_participant_name").
 */
TEST_F(ClockSyncMasterService, testUnregisterNonExistentSyncSlave)
{
    using namespace native::testing;

    const auto result_expected = -1;
    ClockSyncMasterProxy client(fep3::rpc::IRPCClockSyncMasterDef::getRPCDefaultName(),
        _service_bus->getRequester(test_participant_name));

    // actual test
    {
        ASSERT_EQ(client.unregisterSyncSlave(test_participant_name), result_expected);
    }
}

/**
 * @detail Test whether the clock sync master rpc service returns the correct master time.
 *
 * The service bus component provides a requester for the clock sync master proxy while the
 * component registry acts as slave ("test_participant_name").
 */
TEST_F(ClockSyncMasterService, testGetMasterTime)
{
    const auto master_time_expected = "0";
    ClockSyncMasterProxy client(fep3::rpc::IRPCClockSyncMasterDef::getRPCDefaultName(),
        _service_bus->getRequester(fep3::native::testing::test_participant_name));

    // actual test
    {
        ASSERT_EQ(client.getMasterTime(), master_time_expected);
    }
}

/**
 * @detail Test whether the clock sync master rpc service returns the correct master type.
 *
 * The service bus component provides a requester for the clock sync master proxy while the
 * component registry acts as slave ("test_participant_name").
 */
 TEST_F(ClockSyncMasterService, testGetMasterType)
 {
     const auto master_type_expected = 0;
     ClockSyncMasterProxy client(fep3::rpc::IRPCClockSyncMasterDef::getRPCDefaultName(),
         _service_bus->getRequester(fep3::native::testing::test_participant_name));

     // actual test
     {
         ASSERT_EQ(client.getMasterType(), master_type_expected);
     }
 }

/**
 * @detail Test whether the continuous clock sync slave rpc service successfully synchronizes with a clock sync master.
 * This involves:
 * * requesting the master type
 * * registering as slave at the master
 * * requesting the master time
 * * unregistering from the master
 * @req_id FEPSDK-2437, FEPSDK-2436
 */
TEST_F(ContinuousClockSyncSlaveService, testSyncSlave)
{
    EXPECT_CALL(*_rpc_clock_sync_master_mock, getMasterType()).WillOnce(Return(static_cast<int>(fep3::IClock::ClockType::continuous)));
    EXPECT_CALL(*_rpc_clock_sync_master_mock, registerSyncSlave(_, _)).WillOnce(Return(1));
    EXPECT_CALL(*_rpc_clock_sync_master_mock, getMasterTime()).WillRepeatedly(Return("100"));
    _component_registry->start();

    EXPECT_CALL(*_rpc_clock_sync_master_mock, unregisterSyncSlave(_)).WillOnce(Return(1));
    _component_registry->stop();
}

/**
 * @detail Test whether the frequency of synchronization between continuous clock sync slave and a clock sync master
 * may be configured.
 * The configuration entry FEP3_SLAVE_SYNC_CYCLE_TIME_PROPERTY is used to configure the sync frequency.
 * @req_id FEPSDK-2441
 */
TEST_F(ContinuousClockSyncSlaveService, testSyncSlaveFrequencyConfiguration)
{
    using namespace std::chrono;
    using namespace std::chrono_literals;

    std::mutex mutex;
    std::condition_variable time_updates_received;
    const int time_update_repetitions = 5;
    const Duration timeout{ 1s };

    EXPECT_CALL(*_rpc_clock_sync_master_mock, getMasterType()).WillRepeatedly(Return(static_cast<int>(IClock::ClockType::continuous)));
    EXPECT_CALL(*_rpc_clock_sync_master_mock, registerSyncSlave(_, _)).WillRepeatedly(Return(1));
    EXPECT_CALL(*_rpc_clock_sync_master_mock, unregisterSyncSlave(_)).WillRepeatedly(Return(1));

    std::vector<time_point<steady_clock>> time_updates_default_freq{};

    // synchronization frequency of default value
    {
        std::unique_lock<std::mutex> lock(mutex);

        EXPECT_CALL(*_rpc_clock_sync_master_mock, getMasterTime()).WillRepeatedly(
            DoAll(
                Invoke([&]()
                    {
                        if (time_update_repetitions <= time_updates_default_freq.size())
                        {
                            time_updates_received.notify_one();
                        }
                        time_updates_default_freq.emplace_back(steady_clock::now());
                    }),
                Return("100"))
        );

        _component_registry->start();

        EXPECT_EQ(time_updates_received.wait_for(lock, timeout), std::cv_status::no_timeout);

        _component_registry->stop();
    }

    EXPECT_CALL(*_clock_service_mock, unregisterClock(_)).Times(1).WillOnce(
            Return(Result()));

    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());

    setPropertyValue(*_clock_sync_service_property_node->getChild(
        FEP3_SLAVE_SYNC_CYCLE_TIME_PROPERTY), 10);

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());

    std::vector<time_point<steady_clock>> time_updates_low_freq{};

    // synchronization frequency is 10ms
    {
        std::unique_lock<std::mutex> lock(mutex);

        EXPECT_CALL(*_rpc_clock_sync_master_mock, getMasterTime()).WillRepeatedly(
            DoAll(
                Invoke([&]()
                {
                    if (time_update_repetitions <= time_updates_low_freq.size())
                    {
                        time_updates_received.notify_one();
                    }
                    time_updates_low_freq.emplace_back(steady_clock::now());
                }),
                Return("100"))
        );

        _component_registry->start();

        EXPECT_EQ(time_updates_received.wait_for(lock, timeout), std::cv_status::no_timeout);

        _component_registry->stop();
    }

    // verify difference in sync frequency
    {
        const auto duration_syncs_default_freq = duration_cast<milliseconds>(time_updates_default_freq.back() - time_updates_default_freq.front());
        const auto duration_syncs_low_freq = duration_cast<milliseconds>(time_updates_low_freq.back() - time_updates_low_freq.front());
        ASSERT_TRUE((duration_syncs_low_freq * 2) < duration_syncs_default_freq);
    }
}

 /**
 * @detail Test whether the discrete clock sync slave rpc service correctly synchronizes with a clock sync master.
 * This involves:
 * * requesting the master type
 * * registering as slave at the master
 * * receiving time events:
 * * * reset begin
 * * * reset end
 * * * updating
 * * unregistering from the master
 * @req_id FEPSDK-2438, FEPSDK-2436
 */
 TEST_F(DiscreteClockSyncSlaveService, testSyncSlave)
 {
     ClockSyncSlaveProxy client(fep3::rpc::IRPCClockSyncSlaveDef::getRPCDefaultName(),
         _service_bus->getRequester(native::testing::test_participant_name));

     // actual test
     {
         EXPECT_CALL(*_rpc_clock_sync_master_mock, getMasterType()).WillOnce(
             Return(static_cast<int>(fep3::IClock::ClockType::discrete)));
         EXPECT_CALL(*_rpc_clock_sync_master_mock, registerSyncSlave(_, _)).WillOnce(Return(1));
		 EXPECT_CALL(*_event_sink_mock, timeResetBegin(Duration{ 0 }, Duration{ 0 }));
		 EXPECT_CALL(*_event_sink_mock, timeResetEnd(Duration{ 0 }));
         _component_registry->start();                  

		 EXPECT_CALL(*_event_sink_mock, timeUpdating(Duration{ 100 }));
		 ASSERT_EQ(client.syncTimeEvent(static_cast<int>(fep3::rpc::arya::IRPCClockSyncMasterDef::EventID::time_updating), "100", "0"), "100");

         EXPECT_CALL(*_event_sink_mock, timeUpdating(Duration{ 200 }));
         ASSERT_EQ(client.syncTimeEvent(static_cast<int>(fep3::rpc::arya::IRPCClockSyncMasterDef::EventID::time_updating), "200", "100"), "200");

         EXPECT_CALL(*_rpc_clock_sync_master_mock, unregisterSyncSlave(_)).WillOnce(Return(1));
         _component_registry->stop(); 
     }
 }

 /**
* @detail Test whether the discrete clock sync slave rpc service correctly receives
* and reacts to all sync time update events.
* This comprises following events:
* -time_update_before
* -time_updating
* -time_update_after
*/
 TEST_F(DiscreteClockSyncSlaveService, testSyncSlaveUpdateEvents)
 {
     ClockSyncSlaveProxy client(fep3::rpc::IRPCClockSyncSlaveDef::getRPCDefaultName(),
         _service_bus->getRequester(native::testing::test_participant_name));

     // actual test
      {
          EXPECT_CALL(*_rpc_clock_sync_master_mock, getMasterType()).WillOnce(Return(static_cast<int>(fep3::IClock::ClockType::discrete)));
          EXPECT_CALL(*_rpc_clock_sync_master_mock, registerSyncSlave(_, _)).WillOnce(Return(1));
		  EXPECT_CALL(*_event_sink_mock, timeResetBegin(Duration{ 0 }, Duration{ 0 }));
		  EXPECT_CALL(*_event_sink_mock, timeResetEnd(Duration{ 0 }));
		  _component_registry->start();         

         {
             EXPECT_CALL(*_event_sink_mock, timeUpdateBegin(Duration{ 0 }, Duration{ 100 })).Times(1);
             ASSERT_EQ(client.syncTimeEvent(static_cast<int>(fep3::rpc::arya::IRPCClockSyncMasterDef::EventID::time_update_before), "100", "0"), "0");
			 EXPECT_CALL(*_event_sink_mock, timeUpdating(Duration{ 100 })).Times(1);
             ASSERT_EQ(client.syncTimeEvent(static_cast<int>(fep3::rpc::arya::IRPCClockSyncMasterDef::EventID::time_updating), "100", "0"), "100");
			 EXPECT_CALL(*_event_sink_mock, timeUpdateEnd(Duration{ 100 })).Times(1);
             ASSERT_EQ(client.syncTimeEvent(static_cast<int>(fep3::rpc::arya::IRPCClockSyncMasterDef::EventID::time_update_after), "100", "0"), "100");
         }

         {
             EXPECT_CALL(*_event_sink_mock, timeUpdateBegin(Duration{ 100 }, Duration{ 200 })).Times(1);
             ASSERT_EQ(client.syncTimeEvent(static_cast<int>(fep3::rpc::arya::IRPCClockSyncMasterDef::EventID::time_update_before), "200", "100"), "100");
			 EXPECT_CALL(*_event_sink_mock, timeUpdating(Duration{ 200 })).Times(1);
             ASSERT_EQ(client.syncTimeEvent(static_cast<int>(fep3::rpc::arya::IRPCClockSyncMasterDef::EventID::time_updating), "200", "100"), "200");
			 EXPECT_CALL(*_event_sink_mock, timeUpdateEnd(Duration{ 200 })).Times(1);
             ASSERT_EQ(client.syncTimeEvent(static_cast<int>(fep3::rpc::arya::IRPCClockSyncMasterDef::EventID::time_update_after), "200", "100"), "200");
         }

         EXPECT_CALL(*_rpc_clock_sync_master_mock, unregisterSyncSlave(_)).WillOnce(Return(1));
         _component_registry->stop();
     }
 }
 
 /**
* @detail Test whether the discrete clock sync slave rpc service correctly receives and reacts to sync time reset events.
* This comprises following events:
* - timeResetBegin
* - timeResetEnd
*/
 TEST_F(DiscreteClockSyncSlaveService, testSyncSlaveResetEvents)
 {
     ClockSyncSlaveProxy client(fep3::rpc::IRPCClockSyncSlaveDef::getRPCDefaultName(),
         _service_bus->getRequester(native::testing::test_participant_name));

     // actual test
     {
         EXPECT_CALL(*_rpc_clock_sync_master_mock, getMasterType()).WillOnce(Return(static_cast<int>(fep3::IClock::ClockType::discrete)));
         EXPECT_CALL(*_rpc_clock_sync_master_mock, registerSyncSlave(_, _)).WillOnce(Return(1));       
		 EXPECT_CALL(*_event_sink_mock, timeResetBegin(Duration{ 0 }, Duration{ 0 }));
		 EXPECT_CALL(*_event_sink_mock, timeResetEnd(Duration{ 0 }));
		 _component_registry->start();

     	 // normal update
		 {
			 EXPECT_CALL(*_event_sink_mock, timeUpdating(Duration{ 100 })).Times(1);
			 ASSERT_EQ(client.syncTimeEvent(static_cast<int>(fep3::rpc::arya::IRPCClockSyncMasterDef::EventID::time_updating), "100", "0"), "100");
		 }

         // normal update
		 {
			 EXPECT_CALL(*_event_sink_mock, timeUpdating(Duration{ 200 })).Times(1);
			 ASSERT_EQ(client.syncTimeEvent(static_cast<int>(fep3::rpc::arya::IRPCClockSyncMasterDef::EventID::time_updating), "200", "100"), "200");
		 }

         // reset
         {
            EXPECT_CALL(*_event_sink_mock, timeResetBegin(Duration{ 200 }, Duration{ 0 })).Times(1);
            EXPECT_CALL(*_event_sink_mock, timeResetEnd(Duration{ 0 })).Times(1);
         	// time reset events do not propagate the "old time" parameter to the clock, therefore "100" is not used for the following reset begin/end events
            ASSERT_EQ(client.syncTimeEvent(static_cast<int>(fep3::rpc::arya::IRPCClockSyncMasterDef::EventID::time_reset), "0", "100"), "0");
         }    

         EXPECT_CALL(*_rpc_clock_sync_master_mock, unregisterSyncSlave(_)).WillOnce(Return(1));
         _component_registry->stop();
     }
 }
