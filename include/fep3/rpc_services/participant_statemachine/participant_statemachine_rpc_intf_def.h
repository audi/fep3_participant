/**
 * Declaration of the Class IRPCParticipantStateMachine. (can be reached from over rpc)
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

#ifndef FEP3_RPC_IID_STATEMACHINE
#define FEP3_RPC_IID_STATEMACHINE

#include "./../base/fep_rpc_iid.h"

namespace fep3
{
namespace rpc
{
namespace arya
{

/**
 * @brief definition of the external service interface of the participant service
 * see also the delivered participant.json file
 */
class IRPCParticipantStateMachineDef
{
public:
    /// definiton of the FEP rpc service iid for the state machine
    FEP_RPC_IID("participant_statemachine.arya.rpc.fep3.iid", "participant_statemachine");
};

} // namespace arya
using arya::IRPCParticipantStateMachineDef;

} // namespace rpc
} // namespace fep3

#endif //FEP3_RPC_IID_STATEMACHINE
