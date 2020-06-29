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

#include <fep3/base/logging/logging_types.h>
#include <fep3/components/service_bus/rpc/fep_rpc.h>
#include <fep3/rpc_services/logging/logging_service_rpc_intf_def.h>
#include <fep3/rpc_services/logging/logging_service_stub.h>

namespace fep3
{
namespace native
{
namespace arya // version namespace is use because we might support more then this Service methods in future
{
class LoggingService;

/// RPC Server of the Logging Service that handles the registration of listeners and configuration of loggers
class LoggingRPCService :
    public rpc::RPCService<rpc_stubs::RPCLoggingServiceStub, rpc::IRPCLoggingServiceDef>
{
public:
    explicit LoggingRPCService(LoggingService& logging_service)
        : _logging_service(logging_service) {}

private:
    /**
    * Sets a filter for a given logger (domain)
    *
    * @param [in] enable_sink       A comma seperated list of all enabled logging sinks
    * @param [in] logger_name       The logger name / domain to be configured. For more details see @ref page_fep_logging_service
    * @param [in] severity          The filter level for this logger. All logs with a higher level will be disregarded.
    *                               To disable a logger use Severity::off.
    *
    * @return Standard Result value.
    *
    * @retval ERR_INVALID_ARG   The @p sink_properties strings have different number of items
    * @retval ERR_POINTER       Failed to create a logging sink
    */
    int setLoggerFilter(const std::string& enable_sinks, const std::string& logger_name, int severity) override;
    Json::Value getLoggerFilter(const std::string& logger_name) override;
    std::string getLoggers() override;
    std::string getSinks() override;
    std::string getSinkProperties(const std::string& sink_name) override;
    Json::Value getSinkProperty(const std::string& property_name, const std::string& sink_name) override;
    int setSinkProperty(const std::string& property_name, const std::string& sink_name, const std::string& type, const std::string& value) override;


private:
    /// Reference to the logging service to access the logging listener map
    LoggingService& _logging_service;
};
} // namespace arya
using arya::LoggingRPCService;
} // namespace native
} // namespace fep