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

#include <string>
#include <vector>
#include <fep3/fep3_participant_types.h>
#include <fep3/participant/component_source_type.h>

namespace fep3
{
namespace arya
{
    class ComponentsFile
    {
        public:
            struct ComponentItem
            {
                std::string _comp_iid;
                ComponentSourceType _comp_source_type;
                std::string _comp_source_file_reference;
            };
        public:
            ComponentsFile();
            void load(const std::string& file);

            const std::vector<ComponentItem> getItems() const;
            std::string getCurrentPath() const;
            std::vector<std::string> getFiles(ComponentSourceType source_type) const;

        private:                          
            std::vector<ComponentItem> _items;
            std::string _current_path;
    };
}
}