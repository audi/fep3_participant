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
#pragma warning(push)
#pragma warning(disable : 4996 4290)

#include "rpc_intf.h"   //public FEP RPC Interface

//include FEP RPC on JSON
#include <rpc_pkg.h>
#include "fep_json_rpc.h"   //FEP to JSON 
#include "fep_rpc_stubs_client.h"  //default templates
#include "fep_rpc_stubs_service.h"  //default templates
#pragma warning(pop)

