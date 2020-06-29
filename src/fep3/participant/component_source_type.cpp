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

#include "component_source_type.h"

namespace fep3
{
namespace arya
{

ComponentSourceType getComponentSourceType(const std::string& string)
{
    if("built-in" == string)
    {
        return ComponentSourceType::built_in;
    }
    else if("cpp-plugin" == string)
    {
        return ComponentSourceType::cpp_plugin;
    }
    else if("c-plugin" == string)
    {
        return ComponentSourceType::c_plugin;
    }
    else
    {
        return ComponentSourceType::unknown;
    }
}

std::string getString(ComponentSourceType component_origin)
{
    switch(component_origin)
    {
        case ComponentSourceType::built_in: return "built_in";
        case ComponentSourceType::cpp_plugin: return "cpp-plugin";
        case ComponentSourceType::c_plugin: return "c-plugin";
        default: return "unknown";
    }
}

}
using arya::ComponentSourceType;
}