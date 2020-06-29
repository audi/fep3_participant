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
#include <common/gtest_asserts.h>
#include "clock_service_envs.h"

#include <fep3/native_components/clock_sync/clock_sync_service.h>
#include <fep3/base/properties/properties.h>
#include <fep3/fep3_errors.h>
#include <fep3/components/base/component_registry.h>
#include <fep3/components/logging/mock/mock_logging_service.h>

namespace fep3
{
namespace test
{
namespace env
{

using namespace ::testing;

using ClockServiceComponent = NiceMock<fep3::mock::ClockService<>>;
using PropertyNodeMock = StrictMock<fep3::mock::PropertyNode>;

struct NativeClockSyncService : NativeClockService
{
    NativeClockSyncService()
        : _clock_service(std::make_shared<ClockServiceComponent>())
        , _property_node_mock(std::make_shared<PropertyNodeMock>())
    {
    }

    void SetUp() override
    {
        EXPECT_CALL(*_service_bus, getServer()).Times(1).WillOnce(::testing::Return(_rpc_server));
        EXPECT_CALL(*_rpc_server,
            registerService(rpc::IRPCClockSyncMasterDef::getRPCDefaultName(),
                _)).Times(1).WillOnce(::testing::Return(fep3::Result()));
        EXPECT_CALL(*_rpc_server,
            registerService(rpc::IRPCClockServiceDef::getRPCDefaultName(),
                _)).Times(1).WillOnce(::testing::Return(fep3::Result()));
        EXPECT_CALL(*_configuration_service_mock, registerNode(_)).Times(2).WillRepeatedly(
            DoAll(
                WithArg<0>(
                    Invoke([&](
                        const std::shared_ptr<IPropertyNode>& node)
        {
            _clock_sync_service_property_node = node;
        })),
                ::testing::Return(Result())));
        registerComponents();
        setComponents();

        ASSERT_FEP3_NOERROR(_component_registry->create());
    }

    void registerComponents() override
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

        _clock_sync_service_impl = std::make_shared<fep3::native::ClockSynchronizationService>();

        ASSERT_FEP3_NOERROR(_component_registry->registerComponent<fep3::IClockSyncService>(
            _clock_sync_service_impl));
    }

    void setComponents() override
    {
        _clock_sync_service_intf = _component_registry->getComponent<fep3::IClockSyncService>();
        ASSERT_NE(_clock_sync_service_intf, nullptr);
    }

    std::shared_ptr<ClockServiceComponent> _clock_service{ nullptr };
    std::shared_ptr<fep3::native::ClockSynchronizationService> _clock_sync_service_impl{};
    fep3::IClockSyncService* _clock_sync_service_intf{ nullptr };
    std::shared_ptr<PropertyNodeMock> _property_node_mock;
    std::shared_ptr<IPropertyNode> _clock_sync_service_property_node;
};

}
}
}
