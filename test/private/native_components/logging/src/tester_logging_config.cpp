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

#include "fep3/components/base/component_registry.h"
#include "fep3/components/service_bus/rpc/fep_rpc.h"
#include "fep3/native_components/logging/logging_service.h"
#include "fep3/native_components/service_bus/service_bus.h"
#include "fep3/native_components/service_bus/testing/service_bus_testing.hpp"
#include "fep3/rpc_services/logging/logging_service_rpc_intf_def.h"
#include "fep3/rpc_services/logging/logging_client_stub.h"

typedef fep3::rpc::RPCServiceClient<fep3::rpc_stubs::RPCLoggingClientStub, fep3::rpc::IRPCLoggingServiceDef> LoggingServiceClient;

/**
* Test the Logging Configuration
* @req_id ???
*/
TEST(TestLoggingService, TestLoggingConfiguration)
{
    // Set up
    std::shared_ptr<fep3::native::LoggingService> _logging{ std::make_shared<fep3::native::LoggingService>() };
    std::shared_ptr<fep3::native::ServiceBus> _service_bus{ std::make_shared<fep3::native::ServiceBus>() };
    std::shared_ptr<fep3::ComponentRegistry> _component_registry{ std::make_shared<fep3::ComponentRegistry>() };

    ASSERT_TRUE(fep3::native::testing::prepareServiceBusForTestingDefault(*_service_bus));
    ASSERT_EQ(_component_registry->registerComponent<fep3::IServiceBus>(_service_bus), fep3::ERR_NOERROR);
    ASSERT_EQ(_component_registry->registerComponent<fep3::ILoggingService>(_logging), fep3::ERR_NOERROR);
    ASSERT_EQ(_component_registry->create(), fep3::ERR_NOERROR);

    auto _logging_service_client = std::make_unique<LoggingServiceClient>(fep3::rpc::IRPCLoggingServiceDef::getRPCDefaultName(),
        _service_bus->getRequester(fep3::native::testing::test_participant_name));

    // Actual test
    std::shared_ptr<fep3::ILoggingService::ILogger> logger_tester = _logging->createLogger("Tester");
    std::shared_ptr<fep3::ILoggingService::ILogger> logger_a = _logging->createLogger("LoggerA.Tester");
    std::shared_ptr<fep3::ILoggingService::ILogger> logger_b = _logging->createLogger("LoggerB.Tester");
    std::shared_ptr<fep3::ILoggingService::ILogger> logger_c = _logging->createLogger("LoggerC");

    // set default configuration
    ASSERT_NO_THROW(_logging_service_client->setLoggerFilter("", "", static_cast<int>(fep3::logging::Severity::fatal)));
    // all loggers should use the default if no other configuration exists
    ASSERT_TRUE(logger_a->isFatalEnabled());
    ASSERT_TRUE(logger_b->isFatalEnabled());
    ASSERT_TRUE(logger_c->isFatalEnabled());
    ASSERT_TRUE(logger_tester->isFatalEnabled());
    ASSERT_FALSE(logger_a->isErrorEnabled());
    ASSERT_FALSE(logger_b->isErrorEnabled());
    ASSERT_FALSE(logger_c->isErrorEnabled());
    ASSERT_FALSE(logger_tester->isErrorEnabled());

    ASSERT_NO_THROW(_logging_service_client->setLoggerFilter("", "LoggerA.Tester", static_cast<int>(fep3::logging::Severity::error)));
    // all loggers except for A should still use the default
    ASSERT_TRUE(logger_a->isErrorEnabled());
    ASSERT_FALSE(logger_b->isErrorEnabled());
    ASSERT_FALSE(logger_c->isErrorEnabled());
    ASSERT_FALSE(logger_tester->isErrorEnabled());
    
    ASSERT_NO_THROW(_logging_service_client->setLoggerFilter("", "Tester", static_cast<int>(fep3::logging::Severity::warning)));
    // logger A and B should be set too
    ASSERT_TRUE(logger_a->isWarningEnabled());
    ASSERT_TRUE(logger_b->isWarningEnabled());
    ASSERT_FALSE(logger_c->isWarningEnabled());
    ASSERT_TRUE(logger_tester->isWarningEnabled());
}
