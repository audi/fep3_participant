/**
 * @file
 * @copyright AUDI AG
 *            All right reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 */

#ifndef __FEP_RPC_PARTICIPANT_INFO_INTF_DEF_H
#define __FEP_RPC_PARTICIPANT_INFO_INTF_DEF_H

#include <vector>
#include <string>
//very important to have this relative! system library!
#include "../base/fep_rpc_iid.h"

namespace fep3
{
namespace rpc
{
namespace arya
{
/**
* @brief definition of the external service interface of the participant itself
* @see participant_info.json file
*/
class IRPCParticipantInfoDef
{
protected:
    /**
     * @brief Destroy the IRPCParticipantInfo object
     *
     */
    virtual ~IRPCParticipantInfoDef() = default;

public:
    ///definition of the FEP rpc service iid as clock service
    FEP_RPC_IID("participant_info.arya.rpc.fep3.iid", "participant_info");
};
}
}
}

#endif // __FEP_RPC_PARTICIPANT_INFO_INTF_DEF_H