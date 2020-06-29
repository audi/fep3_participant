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

#include "local_job_registry.h"

#include "fep3/components/service_bus/service_bus_intf.h"
#include <a_util/strings.h>

namespace fep3 
{
namespace native
{

std::string RPCJobRegistry::getJobNames()
{
    const auto job_infos = _job_registry.getJobInfos();
    std::string job_names;
    auto first = true;
    for (const auto& job_info : job_infos)
    {
        if (first)
        {
            job_names = job_info.getName();
            first = false;
        }
        else
        {
            job_names += "," + job_info.getName();
        }
    }
    return job_names;
}

Json::Value RPCJobRegistry::getJobInfo(const std::string& job_name)
{
    Json::Value json_value;

    try
    {
        const auto job_configuration = _job_registry.getJobs().at(job_name).job_info.getConfig();

        json_value["job_name"] = job_name;
        json_value["job_configuration"]["cycle_sim_time"] = job_configuration._cycle_sim_time.count();
        json_value["job_configuration"]["delay_sim_time"] = job_configuration._delay_sim_time.count();
        json_value["job_configuration"]["max_runtime_real_time"] = job_configuration._max_runtime_real_time.has_value() ?
            std::to_string(job_configuration._max_runtime_real_time.value().count()) : "";
        json_value["job_configuration"]["runtime_violation_strategy"] = job_configuration.timeViolationStrategyAsString();
        json_value["job_configuration"]["jobs_this_depends_on"] = a_util::strings::join(
            job_configuration._jobs_this_depends_on, ",");

        return json_value;
    }
    catch (const std::exception& /*exception*/)
    {
        json_value["job_name"] = "";
        json_value["job_configuration"] = "";
        return json_value;
    }
}

JobRegistryConfiguration::JobRegistryConfiguration()
    : Configuration(FEP3_JOB_REGISTRY_CONFIG)
{
}

fep3::Result JobRegistryConfiguration::registerPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(registerPropertyVariable(
        _timing_configuration_file_path, FEP3_TIMING_CONFIGURATION_PROPERTY));

    return {};
}

fep3::Result JobRegistryConfiguration::unregisterPropertyVariables()
{
    FEP3_RETURN_IF_FAILED(unregisterPropertyVariable(
        _timing_configuration_file_path, FEP3_TIMING_CONFIGURATION_PROPERTY));

    return {};
}

JobRegistry::JobRegistry()
    : _job_registry_impl(std::make_unique<JobRegistryImpl>())
{
}

fep3::Result JobRegistry::create()
{
    const auto components = _components.lock();
    if (!components)
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_STATE, "No IComponents set, can not get logging and configuration interface");
    }

    FEP3_RETURN_IF_FAILED(setupLogger(*components));

    const auto configuration_service = components->getComponent<IConfigurationService>();
    if (!configuration_service)
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Configuration service is not registered");
    }

    FEP3_RETURN_IF_FAILED(_job_registry_configuration.initConfiguration(*configuration_service));

    const auto service_bus = components->getComponent<IServiceBus>();
    if (!service_bus)
    {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Service Bus is not registered");
    }
    const auto rpc_server = service_bus->getServer();
    if (!rpc_server)
    {
        RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "RPC Server not found");
    }

    FEP3_RETURN_IF_FAILED(setupRPCJobRegistry(*rpc_server));

    return {};
}

fep3::Result JobRegistry::destroy()
{
    _logger.reset();
    return {};
}

fep3::Result JobRegistry::initialize()
{
    _job_registry_configuration.updatePropertyVariables();

    if (!static_cast<std::string>(_job_registry_configuration._timing_configuration_file_path).empty())
    {
        FEP3_RETURN_IF_FAILED(readTimingConfigFromFile(
            _job_registry_configuration._timing_configuration_file_path,
            _timing_configuration));

        const auto components = _components.lock();
        if (!components)
        {
            RETURN_ERROR_DESCRIPTION(ERR_INVALID_STATE, "No IComponents set, can not get logging and configuration interface");
        }
        const auto service_bus = components->getComponent<IServiceBus>();
        if (!service_bus)
        {
            RETURN_ERROR_DESCRIPTION(ERR_POINTER, "Service Bus is not registered");
        }
        const auto rpc_server = service_bus->getServer();
        if (!rpc_server)
        {
            RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "RPC Server not found");
        }

        FEP3_RETURN_IF_FAILED(configureJobsByTimingConfiguration(
            rpc_server->getName(),
            _timing_configuration));
    }

    _initialized = true;

    return {};
}

fep3::Result JobRegistry::deinitialize()
{
    _timing_configuration = timing_configuration::TimingConfiguration();
    _initialized = false;

    return {};
}

fep3::Result JobRegistry::addJob(const std::string & name, const std::shared_ptr<fep3::IJob>& job, const JobConfiguration & job_config)
{
    if (_initialized)
    {
        fep3::Result result = CREATE_ERROR_DESCRIPTION(ERR_INVALID_STATE,
            "Registering a job is possible before initialization only");

        result |= _logger->logError(result.getDescription());

        return result;
    }

    auto result = _job_registry_impl->addJob(name, job, job_config);
    if (ERR_RESOURCE_IN_USE == result)
    {
        result |= _logger->logError(result.getDescription());
    }
    else if (isFailed(result))
    {
        result |= _logger->logWarning(result.getDescription());
    }

    return result;
}

fep3::Result JobRegistry::removeJob(const std::string & name)
{
    if (_initialized)
    {
        fep3::Result result = CREATE_ERROR_DESCRIPTION(ERR_INVALID_STATE,
            "Removing a job is possible before initialization only");

        result |= _logger->logError(result.getDescription());

        return result;
    }

    auto result = _job_registry_impl->removeJob(name);
    if (ERR_NOT_FOUND == result)
    {
        result |= _logger->logError(result.getDescription());
    }
    else if (isFailed(result))
    {
        result |= _logger->logWarning(result.getDescription());
    }

    return result;
}

std::list<JobInfo> JobRegistry::getJobInfos() const
{
    return _job_registry_impl->getJobInfos();
}

fep3::Jobs JobRegistry::getJobs() const
{
    return _job_registry_impl->getJobs();
}

fep3::Result JobRegistry::configureJobsByTimingConfiguration(
    const std::string& participant_name,
    const timing_configuration::TimingConfiguration& timing_configuration)
{
    return _job_registry_impl->configureJobsByTimingConfiguration(
        participant_name, timing_configuration);
}

fep3::Result JobRegistry::setupLogger(const IComponents& components)
{
    auto logging_service = components.getComponent<arya::ILoggingService>();
    if (!logging_service)
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Logging service is not registered");
    }

    _logger = logging_service->createLogger("job_registry.component");

    return {};
}

fep3::Result JobRegistry::setupRPCJobRegistry(IServiceBus::IParticipantServer& rpc_server)
{
    if (!_rpc_job_registry)
    {
        _rpc_job_registry = std::make_shared<RPCJobRegistry>(*this);
    }

    FEP3_RETURN_IF_FAILED(rpc_server.registerService(rpc::IRPCJobRegistryDef::getRPCDefaultName(),
        _rpc_job_registry));

    return {};
}

} // namespace native
} // namespace fep3