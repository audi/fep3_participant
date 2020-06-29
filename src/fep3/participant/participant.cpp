/**
 * Implementation of participant
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

#include <mutex>
#include <condition_variable>

#include <a_util/result/error_def.h>
#include "state_machine/participant_state_machine.h"

#include <fep3/participant/participant.h>
#include "component_registry_factory/component_registry_factory.h"
#include "element_manager/element_manager.h"

#include <fep3/components/service_bus/service_bus_intf.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_service.h>

#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/rpc_services/participant_statemachine/participant_statemachine_rpc_intf_def.h>
#include <fep3/rpc_services/participant_statemachine/participant_statemachine_service_stub.h>

namespace fep3
{
namespace arya
{

class Participant::Impl : public rpc::RPCService<fep3::rpc::arya::ParticipantStateMachineServiceStub,
                                                 fep3::rpc::arya::IRPCParticipantStateMachineDef>
{
public:
    /**
     * @brief A runner functor to encapsulate the multithread access.
     * Note: Encapsulation is necessary to support moving of the participant.
    */
    class Runner
    {
    public:
        void operator()(const std::weak_ptr<ParticipantStateMachine>& weak_ptr_to_participant_state_machine);
        void notify();
    private:
        std::mutex _mutex;
        std::condition_variable _cv;
    };


    Impl(const std::string& name,
         const std::string& version_info,
         const std::string& system_name,
         const std::shared_ptr<ComponentRegistry>& comp_registry,
         const std::shared_ptr<const IElementFactory>& elem_factory)
        : _name(name),
          _version_info(version_info),
          _system_name(system_name),
          _component_registry(comp_registry),
          _element_factory(elem_factory)
    {
    }

    Runner _runner;
    std::shared_ptr<ParticipantStateMachine> _participant_state_machine;

    bool load()
    {
        return _participant_state_machine && _participant_state_machine->load();
    }

    bool unload()
    {
        return _participant_state_machine && _participant_state_machine->unload();
    }

    bool initialize()
    {
        return _participant_state_machine && _participant_state_machine->initialize();
    }

    bool deinitialize()
    {
        return _participant_state_machine && _participant_state_machine->deinitialize();
    }


    bool stop()
    {
        return _participant_state_machine && _participant_state_machine->stop();
    }

    bool start()
    {
        return _participant_state_machine && _participant_state_machine->start();
    }

    bool pause()
    {
        return _participant_state_machine && _participant_state_machine->pause();
    }

    std::string getCurrentStateName()
    {
        return _participant_state_machine ? _participant_state_machine->getCurrentStateName() : "";
    }
    bool exit()
    {
        bool success = false;

        if (_participant_state_machine)
        {
            if (_participant_state_machine->exit())
            {
                _runner.notify();
                success = true;
            }
            else
            {
                success = false;
            }
        }
        else
        {
            // if participant state machine is invalid, exitting is considered successful
            success = true;
        }

        return success;
    }

    std::shared_ptr<ComponentRegistry> _component_registry;
    std::shared_ptr<const IElementFactory> _element_factory;
    std::string getName() const
    {
        return _name;
    }
    std::string getSystemName() const
    {
        return _system_name;
    }
    std::string getVersionInfo() const
    {
        return _version_info;
    }

private:
    std::string _name;
    std::string _system_name;
    std::string _version_info;
};

void Participant::Impl::Runner::operator()(const std::weak_ptr<ParticipantStateMachine>& weak_ptr_to_participant_state_machine)
{
    std::unique_lock<std::mutex> lock(_mutex);
    // wait until participant state machine is finalized
    _cv.wait
        (lock
        , [weak_ptr_to_participant_state_machine]
            {
                if (const auto& participant_state_machine = weak_ptr_to_participant_state_machine.lock())
                {
                    return participant_state_machine->isFinalized();
                }
                else
                {
                    // if the participant state machine is invalid, it is considered to be finalized
                    return true;
                }
            }
        );
}

void Participant::Impl::Runner::notify()
{
    _cv.notify_one();
}


/********************************************************************
 *
 *
 *
 ********************************************************************/

Participant::Participant(const std::string& name,
    const std::string& version_info,
    const std::string& system_name,
    const std::shared_ptr<ComponentRegistry>& component_registry,
    const std::shared_ptr<const IElementFactory>& factory)
    : _impl(std::make_shared<Impl>(name,
                                   version_info,
                                   system_name,
                                   component_registry,
                                   factory)),
      _component_registry(component_registry)
{}

Participant::~Participant() = default;

Participant::Participant(Participant&&) = default;

Participant& Participant::operator=(Participant&& other) = default;


int Participant::exec(const std::function<void()>& start_up_callback)
{
    const auto& component_registry = _impl->_component_registry;
    if(isOk(component_registry->create()))
    {
        //try logging
        auto logging_service = _impl->_component_registry->getComponent<ILoggingService>();
        std::shared_ptr<ILoggingService::ILogger> participant_logger;
        if (logging_service)
        {
            participant_logger = logging_service->createLogger("participant");
        }
        //the most important part
        //we do not work without a servicebus ??? 
        auto service_bus = _impl->_component_registry->getComponent<IServiceBus>();
        
        if (service_bus)
        {
            //the default access must be created while createParticipant!
            ElementManager man(_impl->_element_factory);
            _impl->_participant_state_machine = std::make_shared<ParticipantStateMachine>(std::move(man),
                _impl->_component_registry,
                participant_logger);

            auto server = service_bus->getServer();
            // here we need to think about throwing or return error
            if (server)
            {
                server->registerService(rpc::IRPCParticipantStateMachineDef::getRPCDefaultName(),
                                        _impl);
                if (start_up_callback)
                {
                    start_up_callback();
                }
                _impl->_runner.operator()(_impl->_participant_state_machine);

                server->unregisterService(rpc::IRPCParticipantStateMachineDef::getRPCDefaultName());

                //we release the logger
                participant_logger.reset();

                if (isOk(component_registry->destroy()))
                {
                    return 0;
                }
            }
        }
    }
    return 1;
}

Participant createParticipant(const std::string& name,
    const std::string& version_info,
    const std::string& system_name,
    const std::shared_ptr<const IElementFactory>& factory,
    const std::string& server_address_url)
{
    std::shared_ptr<ComponentRegistry> components = ComponentRegistryFactory::createRegistry();
    //initialize the service bus here!! 
    auto service_bus = components->getComponent<IServiceBus>();
    if (service_bus)
    {
        auto result_system_access_creation = service_bus->createSystemAccess(system_name,
            IServiceBus::ISystemAccess::_use_default_url,
            true);
        if (isOk(result_system_access_creation))
        {
            auto system_access = service_bus->getSystemAccess(system_name);
            std::string use_url = server_address_url;
            if (use_url.empty())
            {
                use_url = IServiceBus::ISystemAccess::_use_default_url;
            }
            auto result_server_creation = system_access->createServer(name,
                                                                      use_url);
            if (fep3::isFailed(result_server_creation))
            { 
                throw std::runtime_error(std::string("can not create participant ")
                    + name + " Error:" + a_util::strings::toString(result_server_creation.getErrorCode())
                    + " Description: " + result_server_creation.getDescription());
            }
        }
    }
    //maybe we throw here if initializing of servicebus failed!
    return Participant(name, version_info, system_name, components, factory);
}

std::string Participant::getName() const
{
    return _impl->getName();
}

std::string Participant::getSystemName() const
{
    return _impl->getSystemName();
}

std::string Participant::getVersionInfo() const
{
    return _impl->getVersionInfo();
}

} // namespace arya
} // namespace fep3
