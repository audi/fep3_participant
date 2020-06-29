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

#include <fep3/native_components/scheduler/local_scheduler_service.h>
#include <fep3/native_components/job_registry/local_job_registry.h>
#include <fep3/components/scheduler/mock/mock_scheduler.h>
#include <fep3/components/logging/mock/mock_logging_service.h>
#include <fep3/components/clock/mock/mock_clock_service.h>
#include <fep3/components/configuration/mock/mock_configuration_service.h>
#include <fep3/components/base/component_registry.h>
#include <fep3/base/properties/properties.h>
#include <fep3/components/service_bus/mock/mock_service_bus.h>

using namespace ::testing;

using SchedulerMock = NiceMock<fep3::mock::Scheduler>;
using LoggerMock = NiceMock<fep3::mock::Logger>;
using LoggingService = fep3::mock::LoggingService;
using ClockMockComponent = NiceMock<fep3::mock::ClockServiceComponentWithDefaultBehaviour>;
using ConfigurationServiceComponentMock = StrictMock<fep3::mock::ConfigurationServiceComponent>;
using ServiceBusComponentMock = StrictMock<fep3::mock::ServiceBusComponent>;
using RPCServerMock = StrictMock<fep3::mock::RPCServer>;

struct SchedulerServiceWithSchedulerMock : ::testing::Test
{
    SchedulerServiceWithSchedulerMock()
        : _scheduler_mock{ std::make_unique<SchedulerMock>() }
        , _scheduler_service_impl{ std::make_unique<fep3::native::LocalSchedulerService>() }
        , _component_registry{ std::make_shared<fep3::ComponentRegistry>() }
        , _logger_mock{ std::make_shared<LoggerMock>() }
        , _configuration_service_mock{ std::make_shared<ConfigurationServiceComponentMock>() }
        , _clock_service_mock{ std::make_unique<ClockMockComponent>() }
        , _service_bus(std::make_shared<ServiceBusComponentMock>())
        , _rpc_server(std::make_shared<RPCServerMock>())
    {
    }

    void SetUp() override 
    {
        EXPECT_CALL(*_service_bus, getServer()).Times(2).WillRepeatedly(::testing::Return(_rpc_server));
        EXPECT_CALL(*_rpc_server, registerService(fep3::rpc::IRPCJobRegistryDef::getRPCDefaultName(),
            _)).WillOnce(Return(fep3::Result()));
        EXPECT_CALL(*_rpc_server, registerService(fep3::rpc::IRPCSchedulerServiceDef::getRPCDefaultName(),
            _)).WillOnce(Return(fep3::Result()));

        ON_CALL(*_scheduler_mock, getName())
            .WillByDefault(Return("my_scheduler"));

        ON_CALL(*_clock_service_mock, start()).WillByDefault(Return(fep3::Result{}));
        ON_CALL(*_clock_service_mock, stop()).WillByDefault(Return(fep3::Result{}));

        registerComponents();

        ASSERT_FEP3_NOERROR(_component_registry->create());
    }

    void registerComponents()
    {
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IServiceBus>(
            _service_bus));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IJobRegistry>(
            std::make_unique<fep3::native::JobRegistry>()));      
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ISchedulerService>(
            _scheduler_service_impl));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IConfigurationService>(
            _configuration_service_mock));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ILoggingService>(
            std::make_unique<LoggingService>(_logger_mock)));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IClockService>(
            _clock_service_mock));

        EXPECT_CALL(*_clock_service_mock, getType()).WillRepeatedly(Return(fep3::IClock::ClockType::continuous));

        EXPECT_CALL(*_configuration_service_mock, registerNode(_)).Times(2).WillRepeatedly(
            DoAll(
                WithArg<0>(
                    Invoke([&](
                        const std::shared_ptr<fep3::arya::IPropertyNode>& node)
                        {
                            if (FEP3_SCHEDULER_SERVICE_CONFIG == node->getName())
                            {
                                _scheduler_service_property_node = node;
                            }
                        })),
                ::testing::Return(fep3::Result())));
    }

    fep3::ISchedulerService* getSchedulerService() const
    {
        const auto scheduler_service = _component_registry->getComponent<fep3::ISchedulerService>();    
        return scheduler_service;
    }

    std::unique_ptr<SchedulerMock> _scheduler_mock{};   
    std::shared_ptr<fep3::native::LocalSchedulerService> _scheduler_service_impl{};
    std::shared_ptr<fep3::ComponentRegistry> _component_registry{};
    std::shared_ptr<LoggerMock> _logger_mock{};
    std::shared_ptr<ConfigurationServiceComponentMock> _configuration_service_mock{};
    std::shared_ptr<fep3::IPropertyNode> _scheduler_service_property_node;
    std::shared_ptr<ClockMockComponent> _clock_service_mock{};
    std::shared_ptr<ServiceBusComponentMock> _service_bus{};
    std::shared_ptr<RPCServerMock> _rpc_server{};
};

/**
* @brief A custom scheduler is registered. It will be tested that the schedulers interface
* is called correctly
* @req_id FEPSDK-2079
*/
TEST_F(SchedulerServiceWithSchedulerMock, RegisterCustomScheduler)
{
    ON_CALL(*_scheduler_mock, getName())
        .WillByDefault(Return("my_custom_scheduler"));

    EXPECT_CALL(*_scheduler_mock, initialize(_, _)).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*_scheduler_mock, deinitialize()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*_scheduler_mock, start()).WillOnce(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*_scheduler_mock, stop()).Times(2).WillRepeatedly(::testing::Return(::fep3::Result{}));
    EXPECT_CALL(*_configuration_service_mock, unregisterNode(_)).Times(1).WillOnce(Return(fep3::Result{}));
    const auto scheduler_name = _scheduler_mock->getName();
    // register scheduler
    {
        fep3::setPropertyValue<std::string>(*_scheduler_service_property_node->getChild(FEP3_SCHEDULER_PROPERTY), "my_custom_scheduler");
        
        ASSERT_FEP3_NOERROR(getSchedulerService()->registerScheduler(std::move(_scheduler_mock)));          
    }

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_STREQ(getSchedulerService()->getActiveSchedulerName().c_str(), "clock_based_scheduler");
    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_STREQ(getSchedulerService()->getActiveSchedulerName().c_str(), scheduler_name.c_str());
    ASSERT_FEP3_NOERROR(_component_registry->start());

    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}

/**
* @brief Iterates the component thru all states
*
*/
TEST_F(SchedulerServiceWithSchedulerMock, IterateAllStates)
{
    EXPECT_CALL(*_configuration_service_mock, unregisterNode(_)).Times(1).WillOnce(Return(fep3::Result{}));

    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_FEP3_NOERROR(_component_registry->start());

    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}

/**
* @detail While running the following actions are performed:
* register, unregister, setActiveScheduler
* An error has to be returned and logged
* @req_id FEPSDK-2102, FEPSDK-2103, FEPSDK-2104
*/
TEST_F(SchedulerServiceWithSchedulerMock, AddRemoveSetInRunningFails)
{
    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_FEP3_NOERROR(_component_registry->start());

    EXPECT_CALL(*_configuration_service_mock, unregisterNode(_)).Times(1).WillOnce(Return(fep3::Result{}));

    // registerScheduler
    {
        EXPECT_CALL((*_logger_mock), logError(_)).Times(1).WillOnce(::testing::Return(::fep3::Result{}));

        auto scheduler_name = _scheduler_mock->getName();
        ASSERT_FEP3_RESULT(getSchedulerService()->registerScheduler(std::move(_scheduler_mock)), fep3::ERR_INVALID_STATE);
    }

    // unregisterScheduler
    {
        EXPECT_CALL((*_logger_mock), logError(_)).Times(1).WillOnce(::testing::Return(::fep3::Result{}));

        ASSERT_FEP3_RESULT(getSchedulerService()->unregisterScheduler("some_scheduler"), fep3::ERR_INVALID_STATE);
    }
    
    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}

/**
* @brief Two schedulers with same name are registered. An error has to be returned and logged
* @req_id FEPSDK-2081
*/
TEST_F(SchedulerServiceWithSchedulerMock, TwoSchedulerSameName)
{
    auto scheduler_mock2 = std::make_unique<SchedulerMock>();
    ON_CALL(*scheduler_mock2, getName())
        .WillByDefault(Return("my_scheduler"));    

    ASSERT_STREQ(scheduler_mock2->getName().c_str(), _scheduler_mock->getName().c_str());

    // actual test
    {
        EXPECT_CALL((*_logger_mock), logError(_)).Times(1).WillOnce(::testing::Return(::fep3::Result{}));
        
        ASSERT_FEP3_NOERROR(getSchedulerService()->registerScheduler(std::move(_scheduler_mock)));
        ASSERT_FEP3_RESULT(getSchedulerService()->registerScheduler(std::move(scheduler_mock2)), fep3::ERR_RESOURCE_IN_USE);
    }   
}

/**
* @brief A scheduler that is not existing is tried to be set as active scheduler. An error has to be returned and logged
* @req_id FEPSDK-2083
*/
TEST_F(SchedulerServiceWithSchedulerMock, ActivateNonExistingScheduler)
{
    // actual test
    {
        fep3::setPropertyValue<std::string>(*_scheduler_service_property_node->getChild(FEP3_SCHEDULER_PROPERTY), "not_existing");
        ASSERT_FEP3_NOERROR(_component_registry->initialize());
        ASSERT_FEP3_RESULT_WITH_MESSAGE(_component_registry->tense(), fep3::ERR_NOT_FOUND,
            "Setting scheduler failed\\. A scheduler with the name 'not_existing' is not registered\\.");
    } 
}

/**
* @brief If the property FEP3_SCHEDULER_SERVICE_SCHEDULER is not set 
* it should be set to FEP3_SCHEDULER_CLOCK_BASED
* @req_id FEPSDK-2099
*/
TEST_F(SchedulerServiceWithSchedulerMock, SchedulerPropertyIsInitializedToDefault)
{
    ASSERT_EQ(fep3::arya::getPropertyValue<std::string>(*_scheduler_service_property_node->getChild(FEP3_SCHEDULER_PROPERTY)), FEP3_SCHEDULER_CLOCK_BASED);
}

/**
* @detail The integration between scheduler service and the scheduler registry is tested.
* Every by the user callabe function of the scheduler service tht will call the scheduler registry is called.
*/
TEST_F(SchedulerServiceWithSchedulerMock, IntegrationWithJobRegistryTest)
{
    // registerScheduler
    {
        EXPECT_CALL((*_logger_mock), logError(_)).Times(1).WillOnce(::testing::Return(::fep3::Result{}));

        auto scheduler_mock2 = std::make_unique<SchedulerMock>();
        ON_CALL(*scheduler_mock2, getName())
            .WillByDefault(Return("clock_based_scheduler"));       
      
        ASSERT_FEP3_RESULT(getSchedulerService()->registerScheduler(std::move(scheduler_mock2)), fep3::ERR_RESOURCE_IN_USE);
    }

    // unregisterScheduler
    {
        EXPECT_CALL((*_logger_mock), logError(_)).Times(1).WillOnce(::testing::Return(::fep3::Result{}));

        ASSERT_FEP3_RESULT(getSchedulerService()->unregisterScheduler("not_existing_scheduler"), fep3::ERR_NOT_FOUND);
    }   

    // getActiveSchedulerName
    {
        ASSERT_STREQ(getSchedulerService()->getActiveSchedulerName().c_str(), "clock_based_scheduler");       
    }

    // getSchedulerNames
    {
        ASSERT_EQ(getSchedulerService()->getSchedulerNames(), std::list<std::string>{"clock_based_scheduler"});
    }    
}