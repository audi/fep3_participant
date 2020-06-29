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

#include "test_scheduler_service_client_stub.h"
#include <fep3/rpc_services/scheduler_service/scheduler_service_rpc_intf_def.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>
#include "fep3/components/base/component_registry.h"
#include "fep3/native_components/scheduler/local_scheduler_service.h"
#include <fep3/components/logging/mock/mock_logging_service.h>
#include <fep3/components/configuration/mock/mock_configuration_service.h>
#include <fep3/native_components/service_bus/testing/service_bus_testing.hpp>
#include "../../../utils/common/gtest_asserts.h"
#include "fep3/native_components/service_bus/service_bus.h"
#include <fep3/components/scheduler/mock/mock_scheduler.h>

namespace fep3
{
namespace test
{
namespace env
{

using namespace ::testing;

using LoggingServiceMock = mock::LoggingService;
using LoggerMock = StrictMock<mock::Logger>;
using ConfigurationServiceComponentMock = StrictMock<mock::ConfigurationServiceComponent>;
using SchedulerMock = NiceMock<mock::Scheduler>;

class TestClient : public rpc::RPCServiceClient<::test::rpc_stubs::TestSchedulerServiceClientStub, rpc::IRPCSchedulerServiceDef>
{
private:
    typedef RPCServiceClient<TestSchedulerServiceClientStub, rpc::IRPCSchedulerServiceDef> base_type;

public:
    using base_type::GetStub;

    TestClient(const std::string& server_object_name,
        const std::shared_ptr<rpc::IRPCRequester>& rpc_requester)
        : base_type(server_object_name, rpc_requester)
    {
    }
};

struct NativeSchedulerServiceRPC : public Test
{
    NativeSchedulerServiceRPC()
        : _component_registry(std::make_shared<ComponentRegistry>())
        , _scheduler_service(std::make_shared<native::LocalSchedulerService>())
        , _logger_mock(std::make_shared<LoggerMock>())
        , _configuration_service_mock{ std::make_shared<ConfigurationServiceComponentMock>() }
        , _service_bus{ std::make_shared<fep3::native::ServiceBus>() }
    {
    }

    void SetUp() override
    {
        EXPECT_CALL(*_configuration_service_mock, registerNode(_)).Times(1).WillOnce(Return(Result()));

        ASSERT_TRUE(fep3::native::testing::prepareServiceBusForTestingDefault(*_service_bus));

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ISchedulerService>(
            _scheduler_service));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ILoggingService>(
            std::make_shared<LoggingServiceMock>(_logger_mock)));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IConfigurationService>(
            _configuration_service_mock));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IServiceBus>(_service_bus));

        ASSERT_FEP3_NOERROR(_component_registry->create());
    }

    std::shared_ptr<ComponentRegistry> _component_registry{};
    std::shared_ptr<native::LocalSchedulerService> _scheduler_service{};
    std::shared_ptr<LoggerMock> _logger_mock{};
    std::shared_ptr<ConfigurationServiceComponentMock> _configuration_service_mock{};
    std::shared_ptr<native::ServiceBus> _service_bus{};
};

TEST_F(NativeSchedulerServiceRPC, testGetSchedulerNames)
{
    TestClient client(rpc::IRPCSchedulerServiceDef::getRPCDefaultName(),
        _service_bus->getRequester(native::testing::test_participant_name));

    // actual test
    {
        ASSERT_EQ("clock_based_scheduler", client.getSchedulerNames());

        std::unique_ptr<SchedulerMock> scheduler_mock{ std::make_unique<SchedulerMock>() };

        ON_CALL(*scheduler_mock, getName())
            .WillByDefault(Return("my_custom_scheduler"));
        _scheduler_service->registerScheduler(std::move(scheduler_mock));

        ASSERT_EQ("clock_based_scheduler,my_custom_scheduler", client.getSchedulerNames());

        _scheduler_service->unregisterScheduler("my_custom_scheduler");

        ASSERT_EQ("clock_based_scheduler", client.getSchedulerNames());
    }
}

TEST_F(NativeSchedulerServiceRPC, testGetActiveSchedulerName)
{
    const auto scheduler_name_expected = "clock_based_scheduler";
    TestClient client(rpc::IRPCSchedulerServiceDef::getRPCDefaultName(),
        _service_bus->getRequester(native::testing::test_participant_name));

    // actual test
    {
        ASSERT_EQ(scheduler_name_expected, client.getActiveSchedulerName());
    }
}

}
}
}
