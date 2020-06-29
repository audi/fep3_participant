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
#include <chrono>
#include <common/gtest_asserts.h>

#include <a_util/filesystem/path.h>

#include <fep3/native_components/job_registry/local_job_registry.h>
#include <fep3/components/logging/mock/mock_logging_service.h>
#include <fep3/components/base/component_registry.h>
#include <fep3/components/configuration/mock/mock_configuration_service.h>
#include <fep3/components/service_bus/mock/mock_service_bus.h>

#include <helper/job_registry_helper.h>

using namespace ::testing;
using namespace fep3::test;
using namespace std::chrono_literals;

using LoggerMock = NiceMock<fep3::mock::Logger>;
using LoggingService = fep3::mock::LoggingService;
using ConfigurationServiceComponentMock = StrictMock<fep3::mock::ConfigurationServiceComponent>;
using ServiceBusComponent = StrictMock<fep3::mock::ServiceBusComponent>;
using ServiceBusComponentMock = StrictMock<fep3::mock::ServiceBusComponent>;
using RPCServerMock = StrictMock<fep3::mock::RPCServer>;

struct JobRegistryWithComponentRegistry : Test
{
    JobRegistryWithComponentRegistry()
        : _component_registry(std::make_shared<fep3::ComponentRegistry>())
        , _logger(std::make_shared<LoggerMock>())
        , _configuration_service_mock(std::make_shared<ConfigurationServiceComponentMock>())
        , _service_bus_mock(std::make_shared<ServiceBusComponentMock>())
        , _rpc_server_mock(std::make_shared<RPCServerMock>())
    {
    }

    void SetUp() override
    {
        EXPECT_CALL(*_service_bus_mock, getServer()).Times(1).WillOnce(::testing::Return(_rpc_server_mock));
        EXPECT_CALL(*_rpc_server_mock,
            registerService(fep3::rpc::IRPCJobRegistryDef::getRPCDefaultName(),
                _)).WillOnce(Return(fep3::Result()));

        createComponents();
        setComponents();      

        EXPECT_CALL(*_configuration_service_mock, registerNode(_)).Times(1).WillOnce(
            DoAll(
                WithArg<0>(
                    Invoke([&](
                        const std::shared_ptr<fep3::arya::IPropertyNode>& node)
                        {
                            if (FEP3_JOB_REGISTRY_CONFIG == node->getName())
                            {
                                _job_registry_property_node = node;
                            }
                        })),
                        Return(fep3::Result())));

        ASSERT_FEP3_RESULT(_component_registry->create(), fep3::ERR_NOERROR);
    }

    void createComponents()
    {
        using namespace fep3::native;

        auto job_registry_impl = std::make_unique<JobRegistryImpl>();
        auto job_registry = std::make_unique<JobRegistry>();
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IConfigurationService>(
            _configuration_service_mock));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IServiceBus>(
            _service_bus_mock));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IJobRegistry>(
            std::move(job_registry)));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ILoggingService>(
            std::make_shared<LoggingService>(_logger)));
    }

    void setComponents()
    {
        _job_registry_impl = _component_registry->getComponent<fep3::IJobRegistry>();
        ASSERT_NE(_job_registry_impl, nullptr);
    }

    helper::SimpleJobBuilder _job;
    fep3::IJobRegistry* _job_registry_impl{nullptr};
    std::shared_ptr<fep3::ComponentRegistry> _component_registry;
    std::shared_ptr<LoggerMock> _logger;
    std::shared_ptr<ConfigurationServiceComponentMock> _configuration_service_mock{};
    std::shared_ptr<ServiceBusComponent> _service_bus_mock{ nullptr };
    std::shared_ptr<RPCServerMock> _rpc_server_mock{};
    std::shared_ptr<fep3::IPropertyNode> _job_registry_property_node;
};


std::unique_ptr<fep3::native::JobRegistry> createJobRegistry()
{
    using namespace fep3::native;

    auto job_registry = std::make_unique<JobRegistry>();

    return job_registry;
}

/**
* @brief All states of the JobRegistry are iterated thru
*/
TEST(JobRegistry, IterateAllStates)
{       
    auto job_registry = createJobRegistry();    

    ASSERT_FEP3_RESULT(job_registry->initialize(), fep3::ERR_NOERROR);
    ASSERT_FEP3_RESULT(job_registry->tense(), fep3::ERR_NOERROR);
    ASSERT_FEP3_RESULT(job_registry->start(), fep3::ERR_NOERROR);
        
    ASSERT_FEP3_RESULT(job_registry->stop(), fep3::ERR_NOERROR);
    ASSERT_FEP3_RESULT(job_registry->relax(), fep3::ERR_NOERROR);
    ASSERT_FEP3_RESULT(job_registry->deinitialize(), fep3::ERR_NOERROR);    
}

/**
* @brief Functional smoke test of the JobRegistry 
* @req_id FEPSDK-2085, FEPSDK-2086, FEPSDK-2087
*/
TEST_F(JobRegistryWithComponentRegistry, FunctionalSmokeTest)
{    
    // actual test
    {
        auto job = _job.makeJob<helper::TestJob>();
        auto job_config = _job.makeJobConfig();
        auto job_name = _job._job_name;

        ASSERT_EQ(_job_registry_impl->getJobInfos().size(), 0);
        ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(job_name, std::move(job), job_config));
        ASSERT_EQ(_job_registry_impl->getJobInfos().size(), 1);
        ASSERT_FEP3_NOERROR(_job_registry_impl->removeJob(job_name));
        ASSERT_EQ(_job_registry_impl->getJobInfos().size(), 0);
    }

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_FEP3_NOERROR(_component_registry->start());
    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}

/**
* @brief Test to validate the configuration of job to be registered
* @req_id FEPSDK-2098, FEPSDK-2166, FEPSDK-2165, FEPSDK-2167, FEPSDK-2284
*/
TEST_F(JobRegistryWithComponentRegistry, JobConfiguration)
{    
    auto job = _job.makeJob<helper::TestJob>();

    {       
        const auto job_name = "my_job";

        const auto cycle_time = 10ms;
        const auto first_delay_sim_time = 1ms;
        const auto max_runtime_real_time = fep3::Optional<fep3::Duration>(100ms);
        const auto runtime_violation_strategy = fep3::JobConfiguration::TimeViolationStrategy::ignore_runtime_violation;
        auto job_config = fep3::JobConfiguration(cycle_time
            , first_delay_sim_time
            , max_runtime_real_time
            , runtime_violation_strategy);    
      
        ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(job_name, std::move(job), job_config));       
    }   

    // use seconds for durations
    {       
        const auto job_name = "my_second_job";

        const auto cycle_time = 1s;
        const auto first_delay_sim_time = 1s;
        const auto max_runtime_real_time = fep3::Optional<fep3::Duration>(1s);
        const auto runtime_violation_strategy = fep3::JobConfiguration::TimeViolationStrategy::set_stm_to_error;
        auto job_config = fep3::JobConfiguration(cycle_time
            , first_delay_sim_time
            , max_runtime_real_time
            , runtime_violation_strategy);    
      
        ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(job_name, std::move(job), job_config));       
    }   
}

/**
* @brief Tests that adding a job while in RUNNING is not possible
* @req_id FEPSDK-2100
*/
TEST_F(JobRegistryWithComponentRegistry, AddFailesInRunning)
{   
    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_FEP3_NOERROR(_component_registry->start());

    // actual test
    {
        auto job = _job.makeJob();
        auto job_config = _job.makeJobConfig();
        auto job_name = _job._job_name;

        EXPECT_CALL((*_logger), logError(_)).WillOnce(::testing::Return(::fep3::ERR_FAILED));
      
        ASSERT_FEP3_RESULT(_job_registry_impl->addJob(job_name, std::move(job), job_config), fep3::ERR_INVALID_STATE);     
    }

    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}

/**
* @brief Tests that removing a job after initialization is not possible
* @req_id FEPSDK-2101
*/
TEST_F(JobRegistryWithComponentRegistry, RemoveFailesInRunning)
{
    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_FEP3_NOERROR(_component_registry->start());

    // actual test
    {
        EXPECT_CALL((*_logger), logError(_)).WillOnce(::testing::Return(::fep3::ERR_FAILED));

        ASSERT_FEP3_RESULT(_job_registry_impl->removeJob("not_existing"), fep3::ERR_INVALID_STATE);
    }

    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}

/**
* @brief Tests that a log is issued if an error ocurrs on add or remove
* @req_id FEPSDK-2085, FEPSDK-2086
*/
TEST_F(JobRegistryWithComponentRegistry, LogOnAddOrRemoveError)
{
    // add
    {
        auto job = _job.makeJob();
        auto job_config = _job.makeJobConfig();
        auto job_name = _job._job_name;

        EXPECT_CALL((*_logger), logError(_)).WillOnce(::testing::Return(::fep3::ERR_FAILED));

        ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(job_name, job, job_config));
        ASSERT_FEP3_RESULT(_job_registry_impl->addJob(job_name, job, job_config), fep3::ERR_RESOURCE_IN_USE);
    }

    // remove 
    {
        EXPECT_CALL((*_logger), logError(_)).WillOnce(::testing::Return(::fep3::ERR_FAILED));

        ASSERT_FEP3_RESULT(_job_registry_impl->removeJob("not_existing"), fep3::ERR_NOT_FOUND);
    }

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());
    ASSERT_FEP3_NOERROR(_component_registry->start());
    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->relax());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}

/**
* @brief Test whether registered jobs are reconfigured successfully on initialization
* if a valid timing configuration is configured.
*
* @req_id TODO
*/
TEST_F(JobRegistryWithComponentRegistry, ReconfigureJobsByTimingConfiguration)
{
    auto job = _job.makeJob();
    auto job_config = _job.makeJobConfig();
    auto job_name = _job._job_name;
    const auto participant_name = "Participant";

    ASSERT_FEP3_NOERROR(_job_registry_impl->addJob(job_name, job, job_config));

    const auto path = a_util::filesystem::Path(TEST_FILES_DIRECTORY).append("valid_timing_configuration.xml");
    _job_registry_property_node->getChild(FEP3_TIMING_CONFIGURATION_PROPERTY)->setValue(path);

    EXPECT_CALL(*_rpc_server_mock,
        getName()).WillOnce(Return(participant_name));

    EXPECT_CALL(*_service_bus_mock, getServer()).Times(1).WillOnce(Return(_rpc_server_mock));

    ASSERT_FEP3_NOERROR(_component_registry->initialize());
    ASSERT_FEP3_NOERROR(_component_registry->tense());

    // verify job reconfiguration success
    {
        const auto configured_job_info = _job_registry_impl->getJobs().at(_job._job_name).job_info.getConfig();

        EXPECT_EQ(100000000, configured_job_info._cycle_sim_time.count());
        EXPECT_EQ(200000000, configured_job_info._delay_sim_time.count());
        EXPECT_EQ(300000000, configured_job_info._max_runtime_real_time.value().count());
        EXPECT_EQ(fep3::arya::JobConfiguration::TimeViolationStrategy::set_stm_to_error,
            configured_job_info._runtime_violation_strategy);
    }

    ASSERT_FEP3_NOERROR(_component_registry->start());
    ASSERT_FEP3_NOERROR(_component_registry->stop());
    ASSERT_FEP3_NOERROR(_component_registry->deinitialize());
    ASSERT_FEP3_NOERROR(_component_registry->destroy());
}
