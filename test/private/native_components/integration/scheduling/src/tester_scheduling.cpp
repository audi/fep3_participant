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

#include <chrono>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fep3/native_components/scheduler/local_scheduler_service.h>
#include <fep3/native_components/clock/local_clock_service.h>
#include <fep3/native_components/configuration/configuration_service.h>
#include <fep3/native_components/job_registry/local_job_registry.h>

#include <fep3/components/logging/mock/mock_logging_service.h>
#include <fep3/components/clock/mock/mock_clock_service.h>
#include <fep3/components/service_bus/mock/mock_service_bus.h>

#include <fep3/components/base/component_registry.h>
#include <fep3/rpc_services/job_registry/job_registry_rpc_intf_def.h>

#include <helper/job_registry_helper.h>
#include <common/gtest_asserts.h>
#include <fep3/core/mock/mock_core.h>
#include <helper/gmock_async_helper.h>

using namespace ::testing;
using namespace fep3::test;
using namespace std::chrono;
using namespace std::chrono_literals;

using LoggerMock = NiceMock<fep3::mock::Logger>;
using ClockEventSinkMock = NiceMock<fep3::mock::EventSink>;
using LoggingService = fep3::mock::LoggingService;
using ServiceBusComponent = NiceMock<fep3::mock::ServiceBusComponent>;
using RPCServer = NiceMock<fep3::mock::RPCServer>;

struct SchedulingWithNativeClock : ::testing::Test
{
    SchedulingWithNativeClock()
        : _component_registry(std::make_shared<fep3::ComponentRegistry>())
        , _logger(std::make_shared<LoggerMock>())
        , _service_bus(std::make_shared<ServiceBusComponent>())
        , _rpc_server(std::make_shared<RPCServer>())
    {
    }

    void SetUp() override 
    {
        using namespace fep3;
        using namespace native;
        using namespace fep3::rpc;

        EXPECT_CALL(*_service_bus, getServer()).Times(6).WillRepeatedly(Return(_rpc_server));
        EXPECT_CALL(*_rpc_server, registerService(IRPCClockSyncMasterDef::getRPCDefaultName(), _)).WillOnce(Return(Result()));
        EXPECT_CALL(*_rpc_server, registerService(IRPCClockServiceDef::getRPCDefaultName(), _)).WillOnce(Return(Result()));
        EXPECT_CALL(*_rpc_server, registerService(IRPCConfigurationDef::getRPCDefaultName(), _)).WillOnce(Return(Result()));
        EXPECT_CALL(*_rpc_server, registerService(IRPCSchedulerServiceDef::getRPCDefaultName(), _)).WillOnce(Return(Result()));
        EXPECT_CALL(*_rpc_server, registerService(IRPCJobRegistryDef::getRPCDefaultName(), _)).WillOnce(Return(Result()));

        EXPECT_CALL(*_rpc_server, unregisterService(_)).WillRepeatedly(Return(fep3::Result()));

        registerComponents();     
        setComponents();

        _clock_event_sink_mock = std::make_shared<ClockEventSinkMock>();
        ASSERT_FEP3_NOERROR(_clock_service->registerEventSink(_clock_event_sink_mock));
      
        ASSERT_FEP3_NOERROR(_component_registry->create());
    }
	
	void registerComponents()
    {
        using namespace fep3::native;
        using namespace fep3::arya::detail;

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IJobRegistry>(
            std::make_unique<JobRegistry>()));

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ISchedulerService>(
            std::make_shared<LocalSchedulerService>()));

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ILoggingService>(
            std::make_shared<LoggingService>(_logger)));

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IServiceBus>(
            _service_bus));

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IClockService>(
            std::make_shared <LocalClockService>()));

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IConfigurationService>(
            std::make_shared <ConfigurationService>()));
    }

    void setComponents()
    {
        _clock_service = _component_registry->getComponent<fep3::IClockService>();
        ASSERT_NE(_clock_service, nullptr);

        _scheduler_service = _component_registry->getComponent<fep3::ISchedulerService>();
        ASSERT_NE(_scheduler_service, nullptr);

        _job_registry_impl = _component_registry->getComponent<fep3::IJobRegistry>();
        ASSERT_NE(_job_registry_impl, nullptr);        
    }  
  
    fep3::ISchedulerService* _scheduler_service = nullptr;
    fep3::IJobRegistry* _job_registry_impl = nullptr;
    fep3::IClockService* _clock_service = nullptr;
        
    std::shared_ptr<fep3::ComponentRegistry> _component_registry{};
    std::shared_ptr<LoggerMock> _logger{};    
    std::shared_ptr<ServiceBusComponent> _service_bus{};
    std::shared_ptr<RPCServer> _rpc_server{};
    std::shared_ptr<ClockEventSinkMock> _clock_event_sink_mock{};

};

struct SchedulingWithDiscreteClock 
    : public SchedulingWithNativeClock 
{
    void SetUp() override
    {
        SchedulingWithNativeClock::SetUp();

        ASSERT_FEP3_NOERROR(fep3::setPropertyValue(*(_component_registry->getComponent<fep3::IConfigurationService>())
            , FEP3_CLOCK_SERVICE_MAIN_CLOCK
            , FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME));

        ASSERT_FEP3_NOERROR(fep3::setPropertyValue(*(_component_registry->getComponent<fep3::IConfigurationService>())
            , FEP3_CLOCK_SERVICE_CLOCK_SIM_TIME_CYCLE_TIME
            , static_cast<int>((10ms).count())));
    }
};

struct SchedulingWithContinuousClock 
    : public SchedulingWithNativeClock 
{
    void SetUp() override
    {
        SchedulingWithNativeClock::SetUp();

         ASSERT_FEP3_NOERROR(fep3::setPropertyValue(*(_component_registry->getComponent<fep3::IConfigurationService>())
            , FEP3_CLOCK_SERVICE_MAIN_CLOCK
            , FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME));        
    }
};

ACTION_P(Notify, notification)
{
     notification->notify();
}


/**
* @brief Tests job execution by the continuous clock
*
* @detail The default scheduler (clock_based_scheduler) is driven by the continous clock
* One job with cycle time 100 ms will be executed until job is executed with >= 400ms
* After that the the components are restarted
* And another 400ms will be simulated.
* @req_id FEPSDK-2088, FEPSDK-2080, FEPSDK-2468, FEPSDK-2467, FEPSDK-2472
*/
TEST_F(SchedulingWithContinuousClock, ExecuteOneJobWithReset)
{    
    const auto job_cycle_time = 100ms;  

    const helper::SimpleJobBuilder builder("my_job", duration_cast<fep3::Duration > (job_cycle_time));
    auto job = builder.makeJob<NiceMock<fep3::mock::core::Job>>();
	job->setDefaultBehaviour();
    auto configuration_service = _component_registry->getComponent<fep3::IConfigurationService>();
    ASSERT_TRUE(configuration_service != nullptr);
    ASSERT_FEP3_NOERROR(fep3::setPropertyValue(*configuration_service, FEP3_CLOCK_SERVICE_MAIN_CLOCK, FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME));

    ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(builder._job_name, job, builder.makeJobConfig()));

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());

    // start - stop
    {
        ::test::helper::Notification done;

        EXPECT_CALL(*_clock_event_sink_mock, timeResetBegin(Ge(fep3::Duration{ 0 }), Ge(fep3::Duration{ 0 }))).Times(1);
        EXPECT_CALL(*_clock_event_sink_mock, timeResetEnd(Ge(fep3::Duration{ 0 }))).Times(1);

        // we expect something like 2ms, 102ms, 201ms ... 310ms and will stop on >= 400ms
        EXPECT_CALL(*job, execute(_)).Times(AnyNumber()); /// this is for a second execute after >= 400ms (if not stopped early enough)
        EXPECT_CALL(*job, execute(Lt(fep3::Timestamp(350ms)))).Times(4);
        EXPECT_CALL(*job, execute(Ge(fep3::Timestamp(400ms)))).Times(1).WillOnce(DoAll(Notify(&done), Return(fep3::ERR_NOERROR)));        

        ASSERT_FEP3_NOERROR(_component_registry->start());
        ASSERT_TRUE(done.waitForNotificationWithTimeout(std::chrono::seconds(1)));
        ASSERT_FEP3_NOERROR(_component_registry->stop());

        Mock::VerifyAndClearExpectations(job.get());
        Mock::VerifyAndClearExpectations(_clock_event_sink_mock.get());
    }

    // restart
    {
        ::test::helper::Notification called_400ms;
        
        EXPECT_CALL(*_clock_event_sink_mock, timeResetBegin(Ge(fep3::Duration{ 0 }), Ge(fep3::Duration{ 0 }))).Times(1);
        EXPECT_CALL(*_clock_event_sink_mock, timeResetEnd(Ge(fep3::Duration{ 0 }))).Times(1);

        // FYI in contrast to start the 0ms is skipped here
        // we expect something like 102ms, 201ms ... 310ms and will stop on >= 400ms
        EXPECT_CALL(*job, execute(_)).Times(AnyNumber()); /// this is for a second execute after >= 400ms (if not stopped early enough)
        EXPECT_CALL(*job, execute(Le(fep3::Timestamp(350ms)))).Times(3);
        EXPECT_CALL(*job, execute(Ge(fep3::Timestamp(400ms)))).WillRepeatedly(DoAll(Notify(&called_400ms), Return(fep3::ERR_NOERROR)));        

        ASSERT_FEP3_NOERROR(_component_registry->start());
        ASSERT_TRUE(called_400ms.waitForNotificationWithTimeout(std::chrono::seconds(1)));
        ASSERT_FEP3_NOERROR(_component_registry->stop());
    }    
   
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}


/**
* @brief Tests job execution by the discrete clock
*
* @detail The default scheduler (clock_based_scheduler) is driven by the discrete clock.
* One job with cycle time 10 ms will be executed for a period of 50ms.
* After that the the components are restarted
* And another 50ms will be simulated.
* @req_id FEPSDK-2088, FEPSDK-2080, FEPSDK-2469, FEPSDK-2467, FEPSDK-2472
*/
TEST_F(SchedulingWithDiscreteClock, ExecuteOneJobWithReset)
{ 
    const auto job_cycle_time = 10ms;   
        
    const helper::SimpleJobBuilder builder("my_job", duration_cast<fep3::Duration > (job_cycle_time));
    auto job = builder.makeJob<NiceMock<fep3::mock::core::Job>>();
	job->setDefaultBehaviour();

    auto configuration_service = _component_registry->getComponent<fep3::IConfigurationService>();
    ASSERT_TRUE(configuration_service != nullptr);
    ASSERT_FEP3_NOERROR(fep3::setPropertyValue(*configuration_service, FEP3_CLOCK_SERVICE_MAIN_CLOCK, FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME));

    ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(builder._job_name, job, builder.makeJobConfig()));

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());

    // start - stop
    {
        ::test::helper::Notification called_50ms;

        EXPECT_CALL(*_clock_event_sink_mock, timeResetBegin(fep3::Duration{ 0 }, fep3::Duration{ 0 })).Times(1);
        EXPECT_CALL(*_clock_event_sink_mock, timeResetEnd(fep3::Duration{ 0 })).Times(1);

        EXPECT_CALL(*job, execute(_)).Times(AnyNumber());
        EXPECT_CALL(*job, execute(fep3::Timestamp(0ms))).Times(1);
        EXPECT_CALL(*job, execute(fep3::Timestamp(10ms))).Times(1);
        EXPECT_CALL(*job, execute(fep3::Timestamp(20ms))).Times(1);
        EXPECT_CALL(*job, execute(fep3::Timestamp(30ms))).Times(1);
        EXPECT_CALL(*job, execute(fep3::Timestamp(40ms))).Times(1);  
        EXPECT_CALL(*job, execute(fep3::Timestamp(50ms))).WillOnce(DoAll(Notify(&called_50ms), Return(fep3::ERR_NOERROR)));

        ASSERT_FEP3_NOERROR(_component_registry->start());       
        ASSERT_TRUE(called_50ms.waitForNotificationWithTimeout(std::chrono::seconds(1)));
        ASSERT_FEP3_NOERROR(_component_registry->stop()); 

        Mock::VerifyAndClearExpectations(job.get());
        Mock::VerifyAndClearExpectations(_clock_event_sink_mock.get());
    }   

    // restart
    {     
        ::test::helper::Notification done;
        
        EXPECT_CALL(*_clock_event_sink_mock, timeResetBegin(Ge(fep3::Duration{ 0 }), fep3::Duration{ 0 })).Times(1);
        EXPECT_CALL(*_clock_event_sink_mock, timeResetEnd(Ge(fep3::Duration{ 0 }))).Times(1);      

        EXPECT_CALL(*job, execute(_)).Times(AnyNumber());
        // FYI in contrast to start the 0ms is skipped here => EXPECT_CALL(*job, execute(fep3::Timestamp(0ms))).Times(1);
        EXPECT_CALL(*job, execute(fep3::Timestamp(10ms))).Times(1);
        EXPECT_CALL(*job, execute(fep3::Timestamp(20ms))).Times(1);
        EXPECT_CALL(*job, execute(fep3::Timestamp(30ms))).Times(1);
        EXPECT_CALL(*job, execute(fep3::Timestamp(40ms))).Times(1);  
        EXPECT_CALL(*job, execute(fep3::Timestamp(50ms))).WillOnce(DoAll(Notify(&done), Return(fep3::ERR_NOERROR)));
        
        ASSERT_FEP3_NOERROR(_component_registry->start());       
        ASSERT_TRUE(done.waitForNotificationWithTimeout(std::chrono::seconds(1)));
        ASSERT_FEP3_NOERROR(_component_registry->stop()); 
    }

    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}


/**
* @brief Tests that for a job the runtime checks are performed
*
* @detail Only the integration is tested here. The detailed tests are executed as unit tests.
* A sleeping job is used. This job runs longer than the configured max runtime. 
* Therefore a logging is executed. Test passes if the log is detected.
* @req_id FEPSDK-2089
*/
TEST_F(SchedulingWithNativeClock, VerifyJobRuntimeCheckIsExecuted)
{
    using namespace std::chrono_literals;
    using Strategy = fep3::JobConfiguration::TimeViolationStrategy;

    const auto job_cycle_time = 10ms;
    const auto job_sleeping_time = 2us;
    const auto job_max_runtime = 1us;
    const auto job_name = "my_job";

    ASSERT_GT(job_sleeping_time, job_max_runtime);
    
    const auto job = std::make_shared<helper::SleepingJob>(job_name
        , duration_cast<fep3::Duration>(job_cycle_time)
        , duration_cast<fep3::Duration>(job_sleeping_time)
        , duration_cast<fep3::Duration>(20ms));

    const auto job_configuration = fep3::JobConfiguration(duration_cast<fep3::Duration>(job_cycle_time)
        , duration_cast<fep3::Duration>(0ms)
        , duration_cast<fep3::Duration>(job_max_runtime)
        , Strategy::warn_about_runtime_violation);
        
    ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(job_name, job, job_configuration));

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());

    // start (expect runtime violation warning)
    {
        // log is emitted for every job call
        EXPECT_CALL((*_logger), logWarning(_)).Times(AtLeast(1)).WillRepeatedly(::testing::Return(::fep3::Result{}));
      
        ASSERT_FEP3_NOERROR(_component_registry->start());
                
        job->waitForExpectedCallTime(1s);    

        ASSERT_FEP3_NOERROR(_component_registry->stop());
        ASSERT_FEP3_NOERROR(_component_registry->relax());
        ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
        ASSERT_FEP3_NOERROR(_component_registry->destroy());
    }
}

/**
 * @brief Two jobs will be scheduled discrete
 * @req_id FEPSDK-2088, FEPSDK-2080, FEPSDK-2469
 */
TEST_F(SchedulingWithDiscreteClock, ExecuteTwoJobs)
{        
    const helper::SimpleJobBuilder builder_10ms("my_job_10ms", duration_cast<fep3::Duration > (10ms));
    auto job_10ms = builder_10ms.makeJob<NiceMock<fep3::mock::core::Job>>();
	job_10ms->setDefaultBehaviour();

    const helper::SimpleJobBuilder builder_20ms("my_job_20ms", duration_cast<fep3::Duration > (20ms));
    auto job_20ms = builder_20ms.makeJob<NiceMock<fep3::mock::core::Job>>();
	job_20ms->setDefaultBehaviour();

    ASSERT_FEP3_NOERROR(fep3::setPropertyValue(*(_component_registry->getComponent<fep3::IConfigurationService>())
        , FEP3_CLOCK_SERVICE_CLOCK_SIM_TIME_CYCLE_TIME
        , static_cast<int>((10ms).count())));

    ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(builder_10ms._job_name, job_10ms, builder_10ms.makeJobConfig()));
    ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(builder_20ms._job_name, job_20ms, builder_20ms.makeJobConfig()));

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());

    // start - stop
    {
        ::test::helper::Notification called_10ms;
        ::test::helper::Notification called_20ms;

        EXPECT_CALL(*job_10ms, execute(_)).Times(AnyNumber());
        EXPECT_CALL(*job_10ms, execute(fep3::Timestamp(0ms))).Times(1);
        EXPECT_CALL(*job_10ms, execute(fep3::Timestamp(10ms))).Times(1);
        EXPECT_CALL(*job_10ms, execute(fep3::Timestamp(20ms))).Times(1);
        EXPECT_CALL(*job_10ms, execute(fep3::Timestamp(30ms))).WillOnce(DoAll(Notify(&called_10ms), Return(fep3::ERR_NOERROR)));

        EXPECT_CALL(*job_20ms, execute(_)).Times(AnyNumber());
        EXPECT_CALL(*job_20ms, execute(fep3::Timestamp(0ms))).Times(1);
        EXPECT_CALL(*job_20ms, execute(fep3::Timestamp(20ms))).Times(1);
        EXPECT_CALL(*job_20ms, execute(fep3::Timestamp(40ms))).Times(1);      
        EXPECT_CALL(*job_20ms, execute(fep3::Timestamp(60ms))).WillOnce(DoAll(Notify(&called_20ms), Return(fep3::ERR_NOERROR)));

        ASSERT_FEP3_NOERROR(_component_registry->start());

        ASSERT_TRUE(called_10ms.waitForNotificationWithTimeout(std::chrono::seconds(1)));
        ASSERT_TRUE(called_20ms.waitForNotificationWithTimeout(std::chrono::seconds(1)));

        ASSERT_FEP3_NOERROR(_component_registry->stop());      
    }     

    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}

/**
 * @brief Two jobs will be scheduled continuous
 * @req_id FEPSDK-2088, FEPSDK-2080, FEPSDK-2468
 */
TEST_F(SchedulingWithContinuousClock, ExecuteTwoJobs)
{    
    const helper::SimpleJobBuilder builder_100ms("my_job_100ms", duration_cast<fep3::Duration > (100ms));
    auto job_100ms = builder_100ms.makeJob<NiceMock<fep3::mock::core::Job>>();
	job_100ms->setDefaultBehaviour();

    const helper::SimpleJobBuilder builder_200ms("my_job_200ms", duration_cast<fep3::Duration > (200ms));
    auto job_200ms = builder_200ms.makeJob<NiceMock<fep3::mock::core::Job>>();
	job_200ms->setDefaultBehaviour();    

    ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(builder_100ms._job_name, job_100ms, builder_100ms.makeJobConfig()));
    ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(builder_200ms._job_name, job_200ms, builder_200ms.makeJobConfig()));

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());

    // start - stop
    {
        ::test::helper::Notification called_500ms_job1;
        ::test::helper::Notification called_500ms_job2;

        // we expect something like 2ms, 102ms, 201ms ... 410ms and will stop on >= 500ms
        EXPECT_CALL(*job_100ms, execute(_)).Times(AnyNumber()); /// this is for a second execute after >= 500ms (if not stopped early enough)
        EXPECT_CALL(*job_100ms, execute(Lt(fep3::Timestamp(450ms)))).Times(5);
        EXPECT_CALL(*job_100ms, execute(Ge(fep3::Timestamp(500ms)))).WillRepeatedly(DoAll(Notify(&called_500ms_job1), Return(fep3::ERR_NOERROR)));

        // we expect something like 2ms, 202ms, 401ms ... 510ms and will stop on >= 500ms
        EXPECT_CALL(*job_200ms, execute(_)).Times(AnyNumber()); /// this is for a second execute after >= 500ms (if not stopped early enough)
        EXPECT_CALL(*job_200ms, execute(Lt(fep3::Timestamp(450ms)))).Times(3);
        EXPECT_CALL(*job_200ms, execute(Ge(fep3::Timestamp(500ms)))).WillRepeatedly(DoAll(Notify(&called_500ms_job2), Return(fep3::ERR_NOERROR)));

        ASSERT_FEP3_NOERROR(_component_registry->start());

        ASSERT_TRUE(called_500ms_job1.waitForNotificationWithTimeout(std::chrono::seconds(1)));
        ASSERT_TRUE(called_500ms_job2.waitForNotificationWithTimeout(std::chrono::seconds(1)));

        ASSERT_FEP3_NOERROR(_component_registry->stop());        
    }
   
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}
