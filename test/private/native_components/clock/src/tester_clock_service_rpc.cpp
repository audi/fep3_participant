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

#include <fep3/base/properties/properties.h>
#include <fep3/components/configuration/mock/mock_configuration_service.h>
#include <fep3/components/logging/mock/mock_logging_service.h>
#include "../../../utils/common/gtest_asserts.h"
#include "test_clock_service_proxy_stub.h"
#include "fep3/components/base/component_registry.h"
#include "fep3/components/clock/mock/mock_clock_service.h"
#include "fep3/components/service_bus/rpc/fep_rpc.h"
#include "fep3/native_components/clock/local_clock_service.h"
#include "fep3/native_components/service_bus/service_bus.h"
#include "fep3/native_components/service_bus/testing/service_bus_testing.hpp"
#include "fep3/rpc_services/clock/clock_service_rpc_intf_def.h"

namespace fep3
{
namespace test
{
namespace env
{

using namespace ::testing;

using Logger = NiceMock<fep3::mock::Logger>;
using ClockMock = NiceMock<mock::Clock>;
using LoggingService = fep3::mock::LoggingService;
using ConfigurationServiceComponentMock = StrictMock<fep3::mock::ConfigurationServiceComponent>;

class TestClient : public fep3::rpc::RPCServiceClient<::test::rpc_stubs::TestClockServiceProxy, fep3::rpc::IRPCClockServiceDef>
{
private:
    typedef fep3::rpc::RPCServiceClient<::test::rpc_stubs::TestClockServiceProxy, fep3::rpc::IRPCClockServiceDef> base_type;

public:
    using base_type::GetStub;

    TestClient(const char* server_object_name,
        const std::shared_ptr<fep3::rpc::IRPCRequester> rpc)
    : base_type(server_object_name, rpc)
    {
    }
};

struct NativeClockServiceRPC : public ::testing::Test
{
    NativeClockServiceRPC()
        : _service_bus{ std::make_shared<fep3::native::ServiceBus>() }
        , _component_registry{ std::make_shared<fep3::ComponentRegistry>() }
        , _logger(std::make_shared<Logger>())
        , _configuration_service_mock{ std::make_shared<ConfigurationServiceComponentMock>() }
        , _clock_service{ std::make_shared<native::LocalClockService>() }
    {}

    void SetUp() override
    {
        EXPECT_CALL(*_configuration_service_mock, registerNode(_)).Times(1).WillOnce(Return(Result()));

        ASSERT_TRUE(fep3::native::testing::prepareServiceBusForTestingDefault(*_service_bus));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IConfigurationService>(
            _configuration_service_mock));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ILoggingService>(
            std::make_shared<LoggingService>(_logger)));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IClockService>(
            _clock_service));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IServiceBus>(_service_bus));
        ASSERT_FEP3_NOERROR(_component_registry->create());
    }

    std::shared_ptr<fep3::native::ServiceBus> _service_bus{};
    std::shared_ptr<fep3::ComponentRegistry> _component_registry{};
    std::shared_ptr<Logger> _logger{};
    std::shared_ptr<ConfigurationServiceComponentMock> _configuration_service_mock{};
    std::shared_ptr<native::LocalClockService> _clock_service{};
};

TEST_F(NativeClockServiceRPC, testGetClockNames)
{
    TestClient client(fep3::rpc::IRPCClockServiceDef::getRPCDefaultName(),
        _service_bus->getRequester(fep3::native::testing::test_participant_name));

    // actual test
    {
        ASSERT_EQ(client.getClockNames(), "local_system_realtime,local_system_simtime");

        std::unique_ptr<ClockMock> clock_mock{ std::make_unique<ClockMock>() };

        ON_CALL(*clock_mock, getName())
            .WillByDefault(Return("my_custom_clock"));
        ASSERT_FEP3_NOERROR(_clock_service->registerClock(std::move(clock_mock)));

        ASSERT_EQ(client.getClockNames(), "local_system_realtime,local_system_simtime,my_custom_clock");

        ASSERT_FEP3_NOERROR(_clock_service->unregisterClock("my_custom_clock"));

        ASSERT_EQ(client.getClockNames(), "local_system_realtime,local_system_simtime");
    }
}

TEST_F(NativeClockServiceRPC, testGetCurrentClock)
{
    const auto current_clock_expected = "local_system_realtime";
    TestClient client(fep3::rpc::IRPCClockServiceDef::getRPCDefaultName(),
        _service_bus->getRequester(fep3::native::testing::test_participant_name));

    // actual test
    {
        ASSERT_EQ(client.getMainClockName(), current_clock_expected);
    }
}

TEST_F(NativeClockServiceRPC, testGetClockTime)
{
    const auto current_clock_expected = "0"
        , invalid_time_expected = "-1";
    TestClient client(fep3::rpc::IRPCClockServiceDef::getRPCDefaultName(),
        _service_bus->getRequester(fep3::native::testing::test_participant_name));

    // actual test
    {
        EXPECT_CALL((*_logger), logWarning(_)).Times(1).WillOnce(::testing::Return(::fep3::Result{}));

        ASSERT_EQ(client.getTime(""), current_clock_expected);
        ASSERT_EQ(client.getTime("non_existent_clock"), invalid_time_expected);
    }
}

TEST_F(NativeClockServiceRPC, testGetClockType)
{
    const auto current_type_expected = 0
    , invalid_type_expected = -1;
    TestClient client(fep3::rpc::IRPCClockServiceDef::getRPCDefaultName(),
        _service_bus->getRequester(fep3::native::testing::test_participant_name));

    // actual test
    {
        EXPECT_CALL((*_logger), logWarning(_)).Times(1).WillOnce(::testing::Return(::fep3::Result{}));

        ASSERT_EQ(client.getType(""), current_type_expected);
        ASSERT_EQ(client.getType("non_existent_clock"), invalid_type_expected);
    }
}

}
}
}