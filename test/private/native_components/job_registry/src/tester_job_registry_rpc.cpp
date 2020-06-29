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
#include <gtest/gtest.h>

#include "test_job_registry_client_stub.h"
#include <fep3/rpc_services/job_registry/job_registry_rpc_intf_def.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>
#include "fep3/components/base/component_registry.h"
#include <fep3/native_components/job_registry/local_job_registry.h>
#include <fep3/components/logging/mock/mock_logging_service.h>
#include <fep3/native_components/service_bus/testing/service_bus_testing.hpp>
#include "../../../utils/common/gtest_asserts.h"
#include "fep3/core/job.h"
#include "fep3/native_components/service_bus/service_bus.h"
#include <fep3/components/configuration/mock/mock_configuration_service.h>

namespace fep3
{
namespace test
{
namespace env
{

using namespace ::testing;

using LoggingServiceMock = mock::LoggingService;
using LoggerMock = StrictMock<mock::Logger>;
using ConfigurationServiceComponentMock = StrictMock<fep3::mock::ConfigurationServiceComponent>;

class TestClient : public rpc::RPCServiceClient<::test::rpc_stubs::TestJobRegistryClientStub, rpc::IRPCJobRegistryDef>
{
private:
    typedef RPCServiceClient<TestJobRegistryClientStub, rpc::IRPCJobRegistryDef> base_type;

public:
    using base_type::GetStub;

    TestClient(const std::string& server_object_name,
        const std::shared_ptr<rpc::IRPCRequester>& rpc_requester)
        : base_type(server_object_name, rpc_requester)
    {
    }
};

struct NativeJobRegistryRPC : public Test
{
    NativeJobRegistryRPC()
        : _component_registry(std::make_shared<ComponentRegistry>())
        , _job_registry(std::make_shared<native::JobRegistry>())
        , _logger_mock(std::make_shared<LoggerMock>())
        , _service_bus{ std::make_shared<fep3::native::ServiceBus>() }
        , _configuration_service_mock(std::make_shared<ConfigurationServiceComponentMock>())
    {
    }

    void SetUp() override
    {
        EXPECT_CALL(*_configuration_service_mock, registerNode(_)).Times(1).WillOnce(
                Return(fep3::Result()));

        ASSERT_TRUE(fep3::native::testing::prepareServiceBusForTestingDefault(*_service_bus));

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IJobRegistry>(
            _job_registry));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ILoggingService>(
            std::make_shared<LoggingServiceMock>(_logger_mock)));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IServiceBus>(_service_bus));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IConfigurationService>(
            _configuration_service_mock));

        ASSERT_FEP3_NOERROR(_component_registry->create());
    }

    std::shared_ptr<ComponentRegistry> _component_registry{};
    std::shared_ptr<native::JobRegistry> _job_registry{};
    std::shared_ptr<LoggerMock> _logger_mock{};
    std::shared_ptr<native::ServiceBus> _service_bus{};
    std::shared_ptr<ConfigurationServiceComponentMock> _configuration_service_mock{};
};

TEST_F(NativeJobRegistryRPC, testGetJobNames)
{
    TestClient client(rpc::IRPCJobRegistryDef::getRPCDefaultName(),
        _service_bus->getRequester(native::testing::test_participant_name));

    // actual test
    {
        EXPECT_EQ("", client.getJobNames());

        ASSERT_FEP3_NOERROR(_job_registry->addJob("test_job_1",
            std::make_shared<core::Job>("test_job_1", Duration{ 1 }),
            JobConfiguration{ Duration{1} }));
        EXPECT_EQ("test_job_1", client.getJobNames());

        ASSERT_FEP3_NOERROR(_job_registry->addJob("test_job_2",
            std::make_shared<core::Job>("test_job_2", Duration{ 2 }),
            JobConfiguration{ Duration{2} }));
        EXPECT_EQ("test_job_1,test_job_2", client.getJobNames());

        ASSERT_FEP3_NOERROR(_job_registry->removeJob("test_job_1"));
        EXPECT_EQ("test_job_2", client.getJobNames());

        ASSERT_FEP3_NOERROR(_job_registry->removeJob("test_job_2"));
        EXPECT_EQ("", client.getJobNames());
    }
}

TEST_F(NativeJobRegistryRPC, testGetJobInfoByJobName)
{
    _job_registry->addJob("test_job_1", std::make_shared<core::Job>("test_job_1", Duration{ 1 }),
        JobConfiguration{ Duration{1} });
    _job_registry->addJob("test_job_2", std::make_shared<core::Job>("test_job_2", Duration{ 2 }),
        JobConfiguration{ Duration{2}, Duration{3}, Duration{4},
            JobConfiguration::TimeViolationStrategy::set_stm_to_error,
            std::vector<std::string>{"dependent_job", "another_job"} });

    TestClient client(rpc::IRPCJobRegistryDef::getRPCDefaultName(),
        _service_bus->getRequester(native::testing::test_participant_name));

    // actual test
    {
        const auto job_info = client.getJobInfo("test_job_1");

        EXPECT_EQ("test_job_1", job_info["job_name"].asString());
        EXPECT_EQ("1", job_info["job_configuration"]["cycle_sim_time"].asString());
        EXPECT_EQ("0", job_info["job_configuration"]["delay_sim_time"].asString());
        EXPECT_EQ("", job_info["job_configuration"]["max_runtime_real_time"].asString());
        EXPECT_EQ("ignore_runtime_violation", job_info["job_configuration"]["runtime_violation_strategy"].asString());
        EXPECT_EQ("", job_info["job_configuration"]["jobs_this_depends_on"].asString());

        const auto job_info_2 = client.getJobInfo("test_job_2");

        EXPECT_EQ("test_job_2", job_info_2["job_name"].asString());
        EXPECT_EQ("2", job_info_2["job_configuration"]["cycle_sim_time"].asString());
        EXPECT_EQ("3", job_info_2["job_configuration"]["delay_sim_time"].asString());
        EXPECT_EQ("4", job_info_2["job_configuration"]["max_runtime_real_time"].asString());
        EXPECT_EQ("set_stm_to_error", job_info_2["job_configuration"]["runtime_violation_strategy"].asString());
        EXPECT_EQ("dependent_job,another_job", job_info_2["job_configuration"]["jobs_this_depends_on"].asString());
    }
}

TEST_F(NativeJobRegistryRPC, testGetNonExistentJobInfoByJobName)
{
    TestClient client(rpc::IRPCJobRegistryDef::getRPCDefaultName(),
        _service_bus->getRequester(native::testing::test_participant_name));

    // actual test
    {
        const auto job_info = client.getJobInfo("");

        EXPECT_EQ("", job_info["job_name"].asString());
        EXPECT_EQ("", job_info["job_configuration"].asString());
    }
}

}
}
}
