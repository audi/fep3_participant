/**
* @file
* Copyright &copy; Audi AG. All rights reserved.
*
* This Source Code Form is subject to the terms of the
* Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*
*/

#ifndef _FEP3_RPC_CLOCK_SERVICE_INTF_DEF_H_
#define _FEP3_RPC_CLOCK_SERVICE_INTF_DEF_H_

//very important to have this relative! system library!
#include "../base/fep_rpc_iid.h"

namespace fep3
{
namespace rpc
{
namespace arya
{

/**
 * @brief definition of the external service interface of the clock service
 * @see delivered clock_service.json file
 */
class IRPCClockServiceDef
{
protected:
    virtual ~IRPCClockServiceDef() = default;

public:
    ///definiton of the FEP rpc service iid for the clock service
    FEP_RPC_IID("clock_service.arya.fep3.iid", "clock_service");
};

} // namespace arya
using arya::IRPCClockServiceDef;
} // namespace rpc
} // namespace fep3

#endif // _FEP3_RPC_CLOCK_SERVICE_INTF_H_