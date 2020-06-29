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
#include <fep3/core/participant_state_changer.h>
#include <fep3/rpc_services/participant_statemachine/participant_statemachine_client_stub.h>
#include <fep3/rpc_services/participant_statemachine/participant_statemachine_rpc_intf_def.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>
#include <fep3/components/service_bus/service_bus_intf.h>

namespace fep3
{
namespace core
{
namespace arya
{
class StateMachineClient 
    : public fep3::rpc::RPCServiceClient<fep3::rpc::arya::ParticipantStateMachineClientStub,
                                         fep3::rpc::arya::IRPCParticipantStateMachineDef>
{
    typedef fep3::rpc::RPCServiceClient<fep3::rpc::arya::ParticipantStateMachineClientStub,
                                        fep3::rpc::arya::IRPCParticipantStateMachineDef> super;
public:
    StateMachineClient(const std::string& service_name,
                       const std::shared_ptr<fep3::rpc::IRPCRequester>& rpc_requester) 
        : super(service_name, rpc_requester)
    {
    }
};

class ParticipantStateChanger::Impl
{
public:
    Impl(fep3::core::arya::Participant& part) 
        : _part(part),
          _sm_client(fep3::rpc::arya::IRPCParticipantStateMachineDef::DEFAULT_NAME, 
                     part.getComponent<IServiceBus>()->getRequester(part.getName()))
    {
        
    }
    virtual ~Impl()
    {
    }

    fep3::core::arya::Participant& _part;
    StateMachineClient             _sm_client;
};

ParticipantStateChanger::ParticipantStateChanger(Participant& part)
    : _impl(std::make_unique<Impl>(part))
{
}

ParticipantStateChanger::~ParticipantStateChanger()
{
}

bool ParticipantStateChanger::load()
{
    try
    {
        return _impl->_sm_client.load();
    }
    catch (...)
    {
        return false;
    }
}
bool ParticipantStateChanger::unload()
{
    try
    {
        return _impl->_sm_client.unload();
    }
    catch (...)
    {
        return false;
    }
}
bool ParticipantStateChanger::initialize()
{
    try
    {
        return _impl->_sm_client.initialize();
    }
    catch (...)
    {
        return false;
    }
}

bool ParticipantStateChanger::deinitialize()
{
    try
    {
        return _impl->_sm_client.deinitialize();
    }
    catch (...)
    {
        return false;
    }
}
bool ParticipantStateChanger::start()
{
    try
    {
        return _impl->_sm_client.start();
    }
    catch (...)
    {
        return false;
    }
}
bool ParticipantStateChanger::pause()
{
    try
    {
        return _impl->_sm_client.pause();
    }
    catch (...)
    {
        return false;
    }
}
bool ParticipantStateChanger::stop()
{
    try
    {
        return _impl->_sm_client.stop();
    }
    catch (...)
    {
        return false;
    }
}

bool ParticipantStateChanger::shutdown()
{
    try
    {
        return _impl->_sm_client.exit();
    }
    catch (...)
    {
        return false;
    }
}

} //arya
using arya::ParticipantStateChanger;
} // namespace core
} // namespace fep3
