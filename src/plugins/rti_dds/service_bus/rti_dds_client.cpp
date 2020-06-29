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
#include "rti_dds_client.h"
#include <../3rdparty/lssdp-cpp/src/url/cxx_url.h>

using namespace fep3;

namespace fep3
{
namespace rti_dds
{
DDSRequester::DDSRequester(const std::string& server_address) : _server_address(server_address)
{
}

DDSRequester::~DDSRequester()
{
}

fep3::Result DDSRequester::sendRequest(const std::string& ,
    const std::string& ,
    IRPCRequester::IRPCResponse& ) const
{
    return {};
}
}
}
