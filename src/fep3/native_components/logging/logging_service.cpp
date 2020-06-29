/**
* @file
* Copyright &copy; Audi AG. All rights reserved.
*
* This Source Code Form is subject to the terms of the
* Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "logging_service.h"

#include "logging_queue.h"
#include "logging_rpc_service.h"

#include "sinks/logging_sink_common.hpp"
#include "sinks/logging_sink_file.hpp"
#include "sinks/logging_sink_console.hpp"
#include <fep3/components/service_bus/service_bus_intf.h>

#include <a_util/datetime.h>
#include <a_util/strings.h>

using namespace fep3;
using namespace fep3::native;

LoggingService::Logger::Logger(LoggingService& logging_service, const std::string& logger_name)
    : _logging_service(&logging_service), _logger_name(logger_name)
{
}

void LoggingService::Logger::releaseLogService()
{
    std::lock_guard<std::recursive_mutex> lock(_sync_service_access);
    _logging_service = nullptr;
}

fep3::Result LoggingService::Logger::logInfo(const std::string& message) const
{
    return log(message, logging::Severity::info);
}

fep3::Result LoggingService::Logger::logWarning(const std::string& message) const
{
    return log(message, logging::Severity::warning);
}

fep3::Result LoggingService::Logger::logError(const std::string& message) const
{
    return log(message, logging::Severity::error);
}

fep3::Result LoggingService::Logger::logFatal(const std::string& message) const
{
    return log(message, logging::Severity::fatal);
}

fep3::Result LoggingService::Logger::logDebug(const std::string& message) const
{
    return log(message, logging::Severity::debug);
}

bool LoggingService::Logger::isInfoEnabled() const
{
    std::lock_guard<std::recursive_mutex> lock(_sync_service_access);
    return (_logging_service 
        && (logging::Severity::info <= _logging_service->_configuration.getLoggerConfig(_logger_name)._severity));
}

bool LoggingService::Logger::isWarningEnabled() const
{
    std::lock_guard<std::recursive_mutex> lock(_sync_service_access);
    return (_logging_service 
        && (logging::Severity::warning <= _logging_service->_configuration.getLoggerConfig(_logger_name)._severity));
}

bool LoggingService::Logger::isErrorEnabled() const
{
    std::lock_guard<std::recursive_mutex> lock(_sync_service_access);
    return (_logging_service 
        && (logging::Severity::error <= _logging_service->_configuration.getLoggerConfig(_logger_name)._severity));
}

bool LoggingService::Logger::isFatalEnabled() const
{
    std::lock_guard<std::recursive_mutex> lock(_sync_service_access);
    return (_logging_service
        && (logging::Severity::fatal <= _logging_service->_configuration.getLoggerConfig(_logger_name)._severity));
}

bool LoggingService::Logger::isDebugEnabled() const
{
    std::lock_guard<std::recursive_mutex> lock(_sync_service_access);
    return (_logging_service
        && (logging::Severity::debug <= _logging_service->_configuration.getLoggerConfig(_logger_name)._severity));
}

fep3::Result LoggingService::Logger::log(const std::string& message, logging::Severity severity) const
{
    std::lock_guard<std::recursive_mutex> lock(_sync_service_access);
    fep3::Result result = ERR_NOERROR;

    // Get Metadata and create Log Message
    std::string timestamp{ "0" };
    std::string part_name{ };
    if (_logging_service)
    {
        if (_logging_service->_clock_service)
        {
            timestamp = a_util::strings::toString(_logging_service->_clock_service->getTime().count());
        }

        logging::LogMessage log_message = {
            timestamp,
            severity,
            _logging_service->_participant_name,
            _logger_name,
            message };

        // Get Configuration and log to all configured sinks
        const LoggerFilterConfig& config = _logging_service->_configuration.getLoggerConfig(_logger_name);
        if (severity <= config._severity)
        {
            for (const auto& logging_sink : config._logging_sinks)
            {
                std::unique_lock<std::mutex> guard(_logging_service->_lock_queue);
                auto fcn = [log_message, logging_sink]()
                {
                    //potentiell an data race here
                    //use an reference to an member that might be destroyed before
                    //function is called
                    logging_sink.second->log(log_message);
                };
                result |= _logging_service->_queue->add(fcn);
            }
        }
    }

    return result;
}

LoggingService::LoggingService() : Configuration(FEP3_LOGGING_SERVICE_CONFIG)
{
    _queue = std::make_unique<LoggingQueue>();
    _default_sinks = std::string("console");
    _default_severity = static_cast<int32_t>(logging::Severity::info);

    registerPropertyVariable(_default_sinks, FEP3_LOGGING_DEFAULT_SINKS_PROPERTY);
    registerPropertyVariable(_default_severity, FEP3_LOGGING_DEFAULT_SEVERITY_PROPERTY);
    registerPropertyVariable(_default_file_sink_file, FEP3_LOGGING_DEFAULT_FILE_SINK_PROPERTY);

    //init the default sinks
    registerSink("console", std::make_shared<LoggingSinkConsole>());
    registerSink("file", std::make_shared<LoggingSinkFile>());
 
    _configuration.setLoggerConfig("",
        { logging::Severity::info, { {"console", getSink("console") } } });
}

LoggingService::~LoggingService()
{
    std::lock_guard<std::recursive_mutex> lock(_sync_loggers);
    //this is to make sure there is no logger in the world anymore which logs 
    //on the reference of this loggingservice
    //this would make boom! 
    for (auto& logger : _loggers)
    {
        logger->releaseLogService();
    }
    _loggers.clear();
}

fep3::Result LoggingService::create()
{
    auto components = _components.lock();
    if (components)
    {
        //clockservice is optional
        _clock_service = components->getComponent<IClockService>();
        //service bus is not optional at the moment
        auto service_bus = components->getComponent<IServiceBus>();
        if (service_bus)
        {
            auto rpc_server = service_bus->getServer();
            if (rpc_server)
            {
                _participant_name = rpc_server->getName();
                _logging_rpc_service = std::make_shared<LoggingRPCService>(*this);
                FEP3_RETURN_IF_FAILED(
                    rpc_server->registerService(::fep3::rpc::IRPCLoggingServiceDef::getRPCDefaultName(),
                        _logging_rpc_service));

                _rpc_sink = std::make_shared<LoggingSinkRPC>(*service_bus);
                //now we cann add the logging sink for the rpc
                registerSink("rpc", _rpc_sink);
                //we change the default properties to log every thing on console AND RPC
                _default_sinks = std::string("rpc,console");
                _configuration.setLoggerConfig("",
                    { logging::Severity::info, { {"rpc", getSink("rpc") },
                                                     {"console", getSink("console") } } });
            }
            else
            {
                RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "RPC Server not found");
            }
        }
        else
        {
            //no rpc possible
            //and no participant name can be obtain!!
            //but i think this is okay
        }
        auto config_serv = components->getComponent<IConfigurationService>();
        if (config_serv)
        {
            auto ret_val = initConfiguration(*config_serv);
            if (isFailed(ret_val))
            {
                return ret_val;
            }
        }
    }
    return {};
}

fep3::Result LoggingService::destroy()
{
    unregisterSink("rpc");
    _rpc_sink->releaseServiceBus();
    deinitConfiguration();
    return {};
}

std::shared_ptr<ILoggingService::ILogger> LoggingService::createLogger(const std::string& logger_name)
{
    std::lock_guard<std::recursive_mutex> lock(_sync_loggers);
    auto new_logger = std::make_shared<Logger>(*this, logger_name);
    _loggers.push_back(new_logger);
    return new_logger;
}

fep3::Result LoggingService::registerSink(const std::string& name,
    const std::shared_ptr<ILoggingSink>& sink)
{
    std::lock_guard<std::recursive_mutex> lock(_sync_sinks);
    const auto& found = _sinks.find(name);
    if (found != _sinks.cend())
    {
        RETURN_ERROR_DESCRIPTION(ERR_RESOURCE_IN_USE,
            "A logging sink with the name %s already exists", name.c_str());
    }
    else
    {
        _sinks[name] = sink;
        return{};
    }
}

fep3::Result LoggingService::unregisterSink(const std::string& name)
{
    std::lock_guard<std::recursive_mutex> lock(_sync_sinks);
    const auto& found = _sinks.find(name);
    if (found != _sinks.cend())
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND,
            "A logging sink with the name %s does not exist", name.c_str());
    }
    else
    {
        _sinks.erase(name);
        return {};
    }
}

fep3::Result LoggingService::setFilter(const std::string& logger_name,
                                       const logging::LoggerFilter& config)
{
    LoggerFilterConfig new_config{ config._severity, {} };

    for (const auto& sink_name : config._enabled_logging_sinks)
    {
        auto sink_found = getSink(sink_name);
        if (sink_found)
        {
            new_config._logging_sinks[sink_name] = sink_found;
        }
        else
        {
            return ERR_NOT_FOUND;
        }
    }

    _configuration.setLoggerConfig(logger_name, new_config);
    return {};
}

logging::LoggerFilter LoggingService::getFilter(const std::string& logger_name) const
{
    auto config = _configuration.getLoggerConfig(logger_name);
    logging::LoggerFilter filter = { config._severity, {} };
    for (const auto& current_sink : config._logging_sinks)
    {
        filter._enabled_logging_sinks.push_back(current_sink.first);
    }
    return filter;
}

std::vector<std::string> LoggingService::getLoggers() const
{
    std::lock_guard<std::recursive_mutex> lock(_sync_loggers);
    std::vector<std::string> ret;
    for (const auto& logger : _loggers)
    {
        ret.push_back(logger->_logger_name);
    }
    return ret;
}

std::vector<std::string> LoggingService::getSinks() const
{
    std::lock_guard<std::recursive_mutex> lock(_sync_sinks);
    std::vector<std::string> ret;
    for (const auto& sink : _sinks)
    {
        ret.push_back(sink.first);
    }
    return ret;
}

std::shared_ptr<ILoggingService::ILoggingSink> LoggingService::getSink(const std::string& name) const
{
    std::lock_guard<std::recursive_mutex> lock(_sync_sinks);
    const auto& it = _sinks.find(name);
    if (it != _sinks.cend())
    {
        return it->second;
    }
    return {};
}
