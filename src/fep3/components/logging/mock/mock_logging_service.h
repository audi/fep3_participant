/**
*
* @file
* Copyright &copy; AUDI AG. All rights reserved.
*
* This Source Code Form is subject to the terms of the
* Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
*/

#pragma once

#include <gmock/gmock.h>
#include <fep3/fep3_errors.h>
#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/components/base/component_base.h>

#include <functional>

namespace fep3
{
namespace mock 
{

struct LoggerMock : fep3::arya::ILoggingService::ILogger
{
    MOCK_CONST_METHOD1(logInfo, fep3::Result(const std::string&));
    MOCK_CONST_METHOD1(logWarning, fep3::Result(const std::string&));
    MOCK_CONST_METHOD1(logError, fep3::Result(const std::string&));
    MOCK_CONST_METHOD1(logFatal, fep3::Result(const std::string&));
    MOCK_CONST_METHOD1(logDebug, fep3::Result(const std::string&));
    MOCK_CONST_METHOD0(isInfoEnabled, bool());
    MOCK_CONST_METHOD0(isWarningEnabled, bool());
    MOCK_CONST_METHOD0(isErrorEnabled, bool());
    MOCK_CONST_METHOD0(isFatalEnabled, bool());
    MOCK_CONST_METHOD0(isDebugEnabled, bool());
};

struct LoggingSink : public fep3::arya::ILoggingService::ILoggingSink
{       
    MOCK_CONST_METHOD1(log, fep3::Result(logging::arya::LogMessage log));

    MOCK_METHOD3(setProperty, bool(const std::string&, const std::string&, const std::string&));
    MOCK_CONST_METHOD1(getProperty, std::string(const std::string&));
    MOCK_CONST_METHOD1(getPropertyType, std::string(const std::string&));
    MOCK_CONST_METHOD1(isEqual, bool(const IProperties&));
    MOCK_CONST_METHOD1(copy_to,void(IProperties&));
    MOCK_CONST_METHOD0(getPropertyNames, std::vector<std::string>());  
};

struct Logger : public LoggerMock
{
    Logger()
    {
        using namespace ::testing;

        ON_CALL(*this, isInfoEnabled())
            .WillByDefault(Return(true));
        ON_CALL(*this, isWarningEnabled())
            .WillByDefault(Return(true));
        ON_CALL(*this, isErrorEnabled())
            .WillByDefault(Return(true));
        ON_CALL(*this, isFatalEnabled())
            .WillByDefault(Return(true));
        ON_CALL(*this, isDebugEnabled())
            .WillByDefault(Return(true));
    }
};

struct LoggerWithDefaultBehaviour : public Logger
{
    LoggerWithDefaultBehaviour()
    {
        using namespace ::testing;

        ON_CALL(*this, logInfo(_))
            .WillByDefault(Return(a_util::result::Result()));
        ON_CALL(*this, logWarning(_))
            .WillByDefault(Return(a_util::result::Result()));
        ON_CALL(*this, logError(_))
            .WillByDefault(Return(a_util::result::Result()));
        ON_CALL(*this, logFatal(_))
            .WillByDefault(Return(a_util::result::Result()));
        ON_CALL(*this, logDebug(_))
            .WillByDefault(Return(a_util::result::Result()));
        ON_CALL(*this, isInfoEnabled())
            .WillByDefault(Return(true));
        ON_CALL(*this, isWarningEnabled())
            .WillByDefault(Return(true));
        ON_CALL(*this, isErrorEnabled())
            .WillByDefault(Return(true));
        ON_CALL(*this, isFatalEnabled())
            .WillByDefault(Return(true));
        ON_CALL(*this, isDebugEnabled())
            .WillByDefault(Return(true));
    }
};

struct LoggingService : public fep3::ComponentBase<ILoggingService>
{
public:
    LoggingService(const std::shared_ptr<Logger>& logger) : _logger(logger)
    {
    }
    ~LoggingService() = default;
    std::shared_ptr<ILogger> createLogger(const std::string&)
    {
        return _logger;
    }
    fep3::Result registerSink(const std::string&,
        const std::shared_ptr<ILoggingSink>&)
    {
        return {};
    }
    fep3::Result unregisterSink(const std::string&)
    {
        return {};
    }
    std::shared_ptr<Logger> _logger;
};

} 
}
