/**  
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

#include <list>
#include <memory>
#include <string>

#include <fep3/components/base/component_base.h>
#include <fep3/components/job_registry/job_configuration.h>
#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/native_components/job_registry/job_registry_impl.h>
#include <fep3/native_components/job_registry/timing_configuration.h>
#include <fep3/components/configuration/propertynode.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_service.h>
#include <fep3/rpc_services/job_registry/job_registry_rpc_intf_def.h>
#include <fep3/rpc_services/job_registry/job_registry_service_stub.h>
#include <fep3/components/service_bus/service_bus_intf.h>

namespace fep3
{
namespace native
{

/**
* @brief Configuration for the Job Registry
*/
struct JobRegistryConfiguration : public Configuration
{
public:
    JobRegistryConfiguration();
    ~JobRegistryConfiguration() = default;

public:
    fep3::Result registerPropertyVariables() override;
    fep3::Result unregisterPropertyVariables() override;

public:
    PropertyVariable<std::string> _timing_configuration_file_path{ "" };
};

class JobRegistry;

class RPCJobRegistry : public rpc::RPCService<rpc_stubs::RPCJobRegistryServiceStub, rpc::IRPCJobRegistryDef>
{
public:
    explicit RPCJobRegistry(JobRegistry& job_registry)
        : _job_registry(job_registry)
    {
    }

protected:
    std::string getJobNames() override;
    Json::Value getJobInfo(const std::string& job_name) override;

private:
    JobRegistry& _job_registry;
};

class JobRegistry : public fep3::ComponentBase<fep3::IJobRegistry>
{
public:
    explicit JobRegistry();
    ~JobRegistry() = default;

public:
    // Inherited via IJobRegistry
    fep3::Result addJob(const std::string & name, const std::shared_ptr<fep3::IJob>& job, const JobConfiguration & job_config) override;
    fep3::Result removeJob(const std::string & name) override;
    std::list<JobInfo> getJobInfos() const override; 
    fep3::Jobs getJobs() const override;

    fep3::Result create() override;
    fep3::Result destroy() override;
    fep3::Result initialize() override;
    fep3::Result deinitialize() override;

private:
    fep3::Result setupLogger(const IComponents& components);
    fep3::Result setupRPCJobRegistry(IServiceBus::IParticipantServer& rpc_server);
    fep3::Result configureJobsByTimingConfiguration(
        const std::string& participant_name,
        const timing_configuration::TimingConfiguration& timing_configuration);

private:  
    std::unique_ptr<JobRegistryImpl> _job_registry_impl;
    bool _initialized{ false };
    std::shared_ptr<const ILoggingService::ILogger> _logger;

    std::shared_ptr<RPCJobRegistry> _rpc_job_registry{};

    JobRegistryConfiguration _job_registry_configuration;
    
    timing_configuration::TimingConfiguration _timing_configuration;
};

} // namespace native
} // namespace fep3