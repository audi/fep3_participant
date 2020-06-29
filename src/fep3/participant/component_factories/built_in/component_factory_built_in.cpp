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
#include "component_factory_built_in.h"
#include <fep3/native_components/clock/local_clock_service.h>
#include <fep3/native_components/configuration/configuration_service.h>
#include <fep3/native_components/data_registry/data_registry.h>
#include <fep3/native_components/scheduler/local_scheduler_service.h>
#include <fep3/native_components/service_bus/service_bus.h>
#include <fep3/native_components/simulation_bus/simulation_bus.h>
#include <fep3/native_components/clock_sync/clock_sync_service.h>
#include <fep3/native_components/job_registry/local_job_registry.h>
#include <fep3/native_components/logging/logging_service.h>
#include <fep3/native_components/configuration/configuration_service.h>

namespace fep3
{
namespace arya
{    
    template<typename interface_type>
    void createAndRegisterComponent(ComponentRegistry& components, const ComponentFactoryBuiltIn& factory)
    {
        components.registerComponent<interface_type>(factory.createComponent(getComponentIID<interface_type>()));
    }

    ComponentFactoryBuiltIn::ComponentFactoryBuiltIn()
    {
    }

    ComponentFactoryBuiltIn::~ComponentFactoryBuiltIn()
    {
    }

    std::unique_ptr<fep3::arya::IComponent> ComponentFactoryBuiltIn::createComponent(const std::string& iid) const
    {
        // note: if/else-if/else in alphabetic order by component interface type
        if (iid == getComponentIID<IClockService>())
        {
            return std::unique_ptr<fep3::arya::IComponent>(new fep3::native::LocalClockService());
        }
        else if (iid == getComponentIID<IClockSyncService>())
        {
            return std::unique_ptr<fep3::arya::IComponent>(new fep3::native::ClockSynchronizationService());
        }
        else if (iid == getComponentIID<IConfigurationService>())
        {
            return std::unique_ptr<fep3::arya::IComponent>(new fep3::native::ConfigurationService());
        }
        else if (iid == getComponentIID<IDataRegistry>())
        {
            return std::unique_ptr<fep3::arya::IComponent>(new fep3::native::DataRegistry());
        }
        else if (iid == getComponentIID<IJobRegistry>())
        {
            return std::unique_ptr<fep3::arya::IComponent>(new fep3::native::JobRegistry());
        }
        else if (iid == getComponentIID<ILoggingService>())
        {
            return std::unique_ptr<fep3::arya::IComponent>(new fep3::native::LoggingService());
        }
        else if (iid == getComponentIID<ISchedulerService>())
        {
            return std::unique_ptr<fep3::arya::IComponent>(new fep3::native::LocalSchedulerService());
        }
        else if (iid == getComponentIID<IServiceBus>())
        {
            return std::unique_ptr<fep3::arya::IComponent>(new fep3::native::ServiceBus());
        }
        else if (iid == getComponentIID<ISimulationBus>())
        {
            return std::unique_ptr<fep3::arya::IComponent>(new fep3::native::SimulationBus());
        }
        return std::unique_ptr<fep3::arya::IComponent>();
    }

    void ComponentFactoryBuiltIn::createDefaults(ComponentRegistry& components) const
    {
        createAndRegisterComponent<ILoggingService>(components, *this);
        createAndRegisterComponent<IConfigurationService>(components, *this);
        createAndRegisterComponent<IServiceBus>(components, *this);
        createAndRegisterComponent<IClockService>(components, *this);
        createAndRegisterComponent<IClockSyncService>(components, *this);
        createAndRegisterComponent<IDataRegistry>(components, *this);
        createAndRegisterComponent<IJobRegistry>(components, *this);
        createAndRegisterComponent<ISchedulerService>(components, *this);
        createAndRegisterComponent<ISimulationBus>(components, *this);
    }
}
}
