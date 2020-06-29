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

#pragma once

#include <gmock/gmock.h>

#include <fep3/base/properties/properties.h>
#include <fep3/base/properties/property_type.h>
#include <fep3/components/clock/clock_base.h>
#include <fep3/native_components/clock/local_clock_service.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/fep3_errors.h>
#include <fep3/components/base/component_registry.h>
#include <fep3/components/logging/mock/mock_logging_service.h>
#include <fep3/components/clock/mock/mock_clock_service.h>
#include <fep3/components/service_bus/mock/mock_service_bus.h>
#include <fep3/components/configuration/mock/mock_configuration_service.h>

#include <common/gtest_asserts.h>

namespace fep3
{
namespace test
{
namespace env
{

using namespace ::testing;

using LoggingService = fep3::mock::LoggingService;
using Logger = NiceMock<fep3::mock::Logger>;
using Clock = NiceMock<fep3::mock::Clock>;
using ServiceBusComponent = NiceMock<fep3::mock::ServiceBusComponent>;
using RPCServer = NiceMock<fep3::mock::RPCServer>;
using ConfigurationServiceComponentMock = StrictMock<fep3::mock::ConfigurationServiceComponent>;

struct NativeClockService : public ::testing::Test
{
    NativeClockService()
        : _component_registry(std::make_shared<fep3::ComponentRegistry>())
        , _service_bus(std::make_shared<ServiceBusComponent>())
        , _rpc_server(std::make_shared<RPCServer>())
        , _logger(std::make_shared<Logger>())
        , _configuration_service_mock{ std::make_shared<ConfigurationServiceComponentMock>() }
    {
    }

    void SetUp() override
    {
        EXPECT_CALL(*_service_bus, getServer()).Times(1).WillOnce(::testing::Return(_rpc_server));
        EXPECT_CALL(*_rpc_server,
            registerService(rpc::IRPCClockSyncMasterDef::getRPCDefaultName(),
                _)).WillOnce(::testing::Return(fep3::Result()));
        EXPECT_CALL(*_rpc_server,
            registerService(rpc::IRPCClockServiceDef::getRPCDefaultName(),
                _)).WillOnce(::testing::Return(fep3::Result()));
        EXPECT_CALL(*_configuration_service_mock, registerNode(_)).Times(1).WillOnce(
            DoAll(
                WithArg<0>(
                    Invoke([&](
                        const std::shared_ptr<IPropertyNode>& node)
                        {
                            _clock_service_property_node = node;
                        })),
                        ::testing::Return(Result())));
                        
         EXPECT_CALL(*_configuration_service_mock, getNode(FEP3_CLOCK_SERVICE_MAIN_CLOCK))
            .WillRepeatedly(InvokeWithoutArgs([this](){ return _clock_service_property_node->getChild(FEP3_MAIN_CLOCK_PROPERTY); }));

        registerComponents();
        setComponents();

        ASSERT_FEP3_NOERROR(_component_registry->create());
    }

    virtual void registerComponents()
    {
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IServiceBus>(
            _service_bus));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::ILoggingService>(
            std::make_shared<LoggingService>(_logger)));
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IConfigurationService>(
            _configuration_service_mock));

        _clock_service_impl = std::make_shared<fep3::native::LocalClockService>();
        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IClockService>(
            _clock_service_impl));
    }

    virtual void setComponents()
    {
        _clock_service_intf = _component_registry->getComponent<fep3::IClockService>();
        ASSERT_NE(_clock_service_intf, nullptr);
    }

    void TearDown() override
    {
        if (_service_bus)
        {
            testing::Mock::VerifyAndClearExpectations(_service_bus.get());
        }
        if (_rpc_server)
        {
            testing::Mock::VerifyAndClearExpectations(_rpc_server.get());
        }
    }

    std::shared_ptr<fep3::ComponentRegistry> _component_registry{};
    std::shared_ptr<ServiceBusComponent> _service_bus{ nullptr };
    std::shared_ptr<RPCServer> _rpc_server{};
    std::shared_ptr<Logger> _logger{};
    fep3::IClockService* _clock_service_intf{nullptr};
    std::shared_ptr<fep3::native::LocalClockService> _clock_service_impl{ nullptr };
    std::shared_ptr<ConfigurationServiceComponentMock> _configuration_service_mock{};
    std::shared_ptr<IPropertyNode> _clock_service_property_node;
};

struct NativeClockServiceWithClockMocks : NativeClockService
{
    NativeClockServiceWithClockMocks()
        : _clock_mock(std::make_shared<Clock>())
        , _clock_mock_2(std::make_shared<Clock>())
    {
    }

    void SetUp() override {
        NativeClockService::SetUp();

        ON_CALL(*_clock_mock, getName())
            .WillByDefault(Return("my_clock"));
        ON_CALL(*_clock_mock_2, getName())
            .WillByDefault(Return("my_clock_2"));
    }

    std::shared_ptr<Clock> _clock_mock{};
    std::shared_ptr<Clock> _clock_mock_2{};
};

}
}
}
