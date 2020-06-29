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
#pragma once

#include <fep3/fep3_participant_types.h>
#include <fep3/plugin/cpp/cpp_plugin_intf.h>
#include <vector>
#include <string>

namespace fep3
{
namespace arya
{
    class CPPPlugin
    {
        public:
            explicit CPPPlugin(const std::string& filepath);
            virtual ~CPPPlugin();
            
            std::unique_ptr<IComponent> createComponent(const std::string& iid) const;
            void load(const std::string& filepath);

        private:        
        #ifdef WIN32
            fcfep3_isDebugPlugin _isDebugPlugin;
        #endif
            fcfep3_getVersion _getVersion;
            fcfep3_getFactory _getFactory;        
    };
}
}
