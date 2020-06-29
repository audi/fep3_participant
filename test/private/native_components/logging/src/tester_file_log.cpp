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

#include <a_util/filesystem.h>

#include <thread>

typedef fep3::rpc::RPCServiceClient<fep3::rpc_stubs::RPCLoggingClientStub, fep3::rpc::IRPCLoggingServiceDef> LoggingServiceClient;

struct TestLoggingServiceFile : public ::testing::Test
{
    std::shared_ptr<fep3::native::LoggingService> _logging{ std::make_shared<fep3::native::LoggingService>() };
    std::shared_ptr<fep3::native::ServiceBus> _service_bus{ std::make_shared<fep3::native::ServiceBus>() };
    std::shared_ptr<fep3::ComponentRegistry> _component_registry{ std::make_shared<fep3::ComponentRegistry>() };
    std::unique_ptr<LoggingServiceClient> _logging_service_client;

    void SetUp()
    {
        ASSERT_TRUE(fep3::native::testing::prepareServiceBusForTestingDefault(*_service_bus));
        ASSERT_EQ(_component_registry->registerComponent<fep3::IServiceBus>(_service_bus), fep3::ERR_NOERROR);
        ASSERT_EQ(_component_registry->registerComponent<fep3::ILoggingService>(_logging), fep3::ERR_NOERROR);
        ASSERT_EQ(_component_registry->create(), fep3::ERR_NOERROR);

        _logging_service_client = std::make_unique<LoggingServiceClient>(fep3::rpc::IRPCLoggingServiceDef::getRPCDefaultName(),
            _service_bus->getRequester(fep3::native::testing::test_participant_name));
    }

    void TearDown()
    {
       
     
    }
};

/**
* The file logger must create a file if it doesn't exist during configuration and write the correct messages into it during logging
* @req_id ???
*/
TEST_F(TestLoggingServiceFile, TestFileLog)
{
    std::string test_log_file = "./../files/some_logfile.txt";
    ASSERT_TRUE(a_util::filesystem::createDirectory("./../files/"));

    // deleting leftover files...
    a_util::filesystem::remove(test_log_file);

    std::shared_ptr<fep3::ILoggingService::ILogger> logger = _logging->createLogger("FileLogger.LoggingService.Tester");
    ASSERT_TRUE(logger);

    ASSERT_NO_THROW(_logging_service_client->setLoggerFilter(
        "file",
        "FileLogger.LoggingService.Tester",
        static_cast<int>(fep3::logging::Severity::warning)));
    ASSERT_NO_THROW(_logging_service_client->setSinkProperty(
        "file_path",
        "file",
        "string",
        test_log_file));

    ASSERT_TRUE(a_util::filesystem::exists(test_log_file));

    ASSERT_EQ(logger->logError("First message"), fep3::ERR_NOERROR);
    ASSERT_EQ(logger->logWarning("Second message"), fep3::ERR_NOERROR);
    // severity == info should not appear because it's not configured
    ASSERT_EQ(logger->logInfo("Test log: must not appear in file"), fep3::ERR_NOERROR);

    // wait until the logs are executed from queue
    a_util::system::sleepMilliseconds(300);

    // validate file content
    std::string content;
    a_util::filesystem::readTextFile(test_log_file, content);

    ASSERT_TRUE(content.find("FileLogger.LoggingService.Tester") != std::string::npos);
    ASSERT_TRUE(content.find("Error") != std::string::npos);
    ASSERT_TRUE(content.find("First message") != std::string::npos);
    ASSERT_TRUE(content.find("Warning") != std::string::npos);
    ASSERT_TRUE(content.find("Second message") != std::string::npos);

    ASSERT_FALSE(content.find("Info") != std::string::npos);
    ASSERT_FALSE(content.find("must not appear in file") != std::string::npos);

    // deleting leftover files...
    a_util::filesystem::remove(test_log_file);
}

/**
* Two or more loggers using the same log file must not interfere with each other, but not loose any logs either
* @req_id ???
*/
TEST_F(TestLoggingServiceFile, TestFileStress)
{
    std::string test_log_file = "./../files/some_logfile.txt";
    ASSERT_TRUE(a_util::filesystem::createDirectory("./../files/"));

    // deleting leftover files...
    a_util::filesystem::remove(test_log_file);

    std::shared_ptr<fep3::ILoggingService::ILogger> logger_first = _logging->createLogger("FileLogger.LoggingService.Tester");
    std::shared_ptr<fep3::ILoggingService::ILogger> logger_second = _logging->createLogger("FileLogger.LoggingService.Tester");
    ASSERT_NO_THROW(_logging_service_client->setLoggerFilter(
        "file",
        "FileLogger.LoggingService.Tester",
        static_cast<int>(fep3::logging::Severity::warning)));
    ASSERT_NO_THROW(_logging_service_client->setSinkProperty(
        "file_path",
        "file",
        "string",
        test_log_file));

    for (int i = 0; i < 100; ++i)
    {
        std::thread([logger_first, i]() {
            ASSERT_EQ(logger_first->logWarning("First:  " + a_util::strings::toString(i)), fep3::ERR_NOERROR);
        }).detach();
        std::thread([logger_second, i]() {
            ASSERT_EQ(logger_second->logWarning("Second: " + a_util::strings::toString(i)), fep3::ERR_NOERROR);
        }).detach();
    }

    // make sure all threads have returned
    a_util::system::sleepMilliseconds(1000);

    // validate file content
    std::string content;
    a_util::filesystem::readTextFile(test_log_file, content);

    ASSERT_TRUE(content.find("FileLogger.LoggingService.Tester") != std::string::npos);
    ASSERT_TRUE(content.find("Warning") != std::string::npos);

    for (int i = 0; i < 100; ++i)
    {
        ASSERT_TRUE(content.find("First:  " + a_util::strings::toString(i)) != std::string::npos);
        ASSERT_TRUE(content.find("Second: " + a_util::strings::toString(i)) != std::string::npos);
    }

    // deleting leftover files...
    a_util::filesystem::remove(test_log_file);
}