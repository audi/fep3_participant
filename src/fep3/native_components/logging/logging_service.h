/**
* @file
* Copyright &copy; Audi AG. All rights reserved.
*
* This Source Code Form is subject to the terms of the
* Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#pragma once

#include <fep3/components/base/component_base.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/components/logging/logging_service_intf.h>
#include "logging_config.h"
#include "logging_rpc_service.h"
#include <fep3/components/configuration/propertynode.h>
#include <fep3/components/configuration/configuration_service_intf.h>
#include "sinks/logging_sink_rpc.hpp"

namespace fep3
{
namespace native
{
namespace arya
{
class LoggingServer;
class LoggingQueue;
struct LoggingConfigDescription;

class LoggingService : public ComponentBase<ILoggingService>,
                              Configuration
{
public:
    class Logger : public ILogger
    {
        friend class LoggingService;
    public:
        explicit Logger(LoggingService& logging_service, const std::string& logger_name);

        fep3::Result logInfo(const std::string& message) const override;
        fep3::Result logWarning(const std::string& message) const override;
        fep3::Result logError(const std::string& message) const override;
        fep3::Result logFatal(const std::string& message) const override;
        fep3::Result logDebug(const std::string& message) const override;
        bool isInfoEnabled() const override;
        bool isWarningEnabled() const override;
        bool isErrorEnabled() const override;
        bool isFatalEnabled() const override;
        bool isDebugEnabled() const override;

    private:
        fep3::Result log(const std::string& message, logging::Severity severity) const;
        void releaseLogService();

    private:
        std::string _logger_name;
        LoggingService* _logging_service;
        mutable std::recursive_mutex _sync_service_access;
    };

    LoggingService();
    ~LoggingService();

    // Methods inherited from ComponentBase
    fep3::Result create() override;
    fep3::Result destroy() override;

    // Methods inherited from ILoggingService
    std::shared_ptr<ILogger> createLogger(const std::string& logger_name) override;
    fep3::Result registerSink(const std::string& name,
                              const std::shared_ptr<ILoggingSink>& sink) override;
    fep3::Result unregisterSink(const std::string& name) override;

public:
    // Methods of LoggingService
    fep3::Result setFilter(const std::string& logger_filter_name,
                           const logging::LoggerFilter& config);

    logging::LoggerFilter getFilter(const std::string& logger_filter_name) const;

    std::shared_ptr<ILoggingSink> getSink(const std::string& name) const;
    std::vector<std::string> getLoggers() const;
    std::vector<std::string> getSinks() const;

private:
    /// RPC server object to set the logging configurations for this participant
    std::shared_ptr<LoggingRPCService> _logging_rpc_service;
    /// Queue object so that loggers don't halt the main program
    std::unique_ptr<LoggingQueue> _queue;
    mutable a_util::concurrency::mutex _lock_queue;
    /// Configuration which logs should be filtered
    LoggingConfigTree _configuration;
    /// Pointer to the clock service to get the current timestamp for the log
    IClockService* _clock_service;
    std::string    _participant_name;

    std::vector<std::shared_ptr<Logger>> _loggers;
    mutable std::recursive_mutex _sync_loggers;

    std::shared_ptr<LoggingSinkRPC> _rpc_sink;
    std::map<std::string, std::shared_ptr<ILoggingSink>> _sinks;
    mutable std::recursive_mutex _sync_sinks;

    PropertyVariable<std::string> _default_sinks;
    PropertyVariable<std::string> _default_file_sink_file;
    PropertyVariable<int32_t> _default_severity;
};
} // namespace arya
using arya::LoggingService;
} // namespace native
} // namespace fep3
