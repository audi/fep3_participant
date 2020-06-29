/**
* @file
* Copyright &copy; Audi AG. All rights reserved.
*
* This Source Code Form is subject to the terms of the
* Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "logging_rpc_service.h"
#include "logging_service.h"


namespace fep3
{
namespace native
{
namespace arya
{

int LoggingRPCService::setLoggerFilter(const std::string& enable_sinks, const std::string& logger_name, int severity)
{
    std::vector<std::string> enabled_logging_sinks = a_util::strings::split(enable_sinks, ",");

    logging::LoggerFilter config{ static_cast<logging::Severity>(severity), enabled_logging_sinks };

    return _logging_service.setFilter(logger_name, config).getErrorCode();
}

Json::Value LoggingRPCService::getLoggerFilter(const std::string& logger_name)
{
    auto filter = _logging_service.getFilter(logger_name);
    Json::Value ret;
    ret["severity"] = static_cast<int>(filter._severity);
    ret["enable_sinks"] = a_util::strings::join(filter._enabled_logging_sinks, ",");
    return ret;
}

std::string LoggingRPCService::getLoggers()
{
    return a_util::strings::join(_logging_service.getLoggers(), ",");
}

std::string LoggingRPCService::getSinks()
{
    return a_util::strings::join(_logging_service.getSinks(), ",");
}

std::string LoggingRPCService::getSinkProperties(const std::string& sink_name)
{
    auto sink = _logging_service.getSink(sink_name);
    if (sink)
    {
        return a_util::strings::join(sink->getPropertyNames(), ",");
    }
    return {};
}

Json::Value LoggingRPCService::getSinkProperty(const std::string& property_name, const std::string& sink_name)
{
    auto sink = _logging_service.getSink(sink_name);
    if (sink)
    {
        Json::Value prop;
        prop["value"] = sink->getProperty(property_name);
        prop["type"] = sink->getPropertyType(property_name);
        return prop;
    }
    return {};
}

int LoggingRPCService::setSinkProperty(const std::string& property_name,
    const std::string& sink_name,
    const std::string& type,
    const std::string& value)
{
    auto sink = _logging_service.getSink(sink_name);
    if (sink)
    {
        if (sink->setProperty(property_name, type, value))
        {
            return 0;
        }
        else
        {
            return ERR_ACCESS_DENIED.getCode();
        }
    }
    return ERR_NOT_FOUND.getCode();
}

}
}
}
