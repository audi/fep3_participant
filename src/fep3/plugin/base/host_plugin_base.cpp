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

#include <stdexcept>

#include "host_plugin_base.h"
#include "shared_library.h"

namespace fep3
{
namespace plugin
{
namespace arya
{

HostPluginBase::HostPluginBase(const std::string& file_path, bool prevent_unloading)
    //: boost::dll::shared_library(file_path, boost::dll::load_mode::append_decorations)
    : SharedLibrary
        (file_path
        , prevent_unloading
        )
{
    auto get_plugin_version_function = get<void(void(void*, const char*), void*)>(SYMBOL_fep3_plugin_getPluginVersion);
        if(!get_plugin_version_function)
        {
            throw std::runtime_error("The plugin '" + file_path + "' does not provide an appropriate '"
                + SYMBOL_fep3_plugin_getPluginVersion + "' function.");
        }
        get_plugin_version_function
            ([](void* destination, const char* plugin_version)
                {
                    *static_cast<decltype(_plugin_version)*>(destination) = plugin_version;
                }
            , static_cast<void*>(&_plugin_version)
            );
}

HostPluginBase::~HostPluginBase()
{}

std::string HostPluginBase::getPluginVersion() const
{
    return _plugin_version;
}

ParticipantLibraryVersion HostPluginBase::getParticipantLibraryVersion() const
{
    return _participant_library_version;
}

} // namespace arya
} // namespace plugin
} // namespace fep3