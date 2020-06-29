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

#include "c_host_plugin.h"
#include <fep3/plugin/base/plugin_base_intf.h>

namespace fep3
{
namespace plugin
{
namespace c
{
namespace arya
{

HostPlugin::HostPlugin(const std::string& file_path)
    : HostPluginBase(file_path)
{
    const auto& get_participant_library_version_function
        = get<void(void(void*, fep3_plugin_base_ParticipantLibraryVersion), void*)>(SYMBOL_fep3_plugin_getParticipantLibraryVersion);
    if(!get_participant_library_version_function)
    {
        throw std::runtime_error("The plugin '" + file_path + "' does not provide an appropriate '"
            + SYMBOL_fep3_plugin_getParticipantLibraryVersion + "' function.");
    }
    get_participant_library_version_function
        ([](void* destination, fep3_plugin_base_ParticipantLibraryVersion participant_library_version)
            {
                *static_cast<decltype(_participant_library_version)*>(destination) = participant_library_version;
            }
        , static_cast<void*>(&_participant_library_version)
        );
}

HostPlugin::~HostPlugin() = default;

} // namespace arya
} // namespace c
} // namespace plugin
} // namespace fep3
