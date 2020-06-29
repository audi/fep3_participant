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

#include <string>

namespace fep3
{
namespace arya
{

enum class ComponentSourceType
{
    built_in = 0,
    cpp_plugin,
    c_plugin,
    unknown
};

ComponentSourceType getComponentSourceType(const std::string& string);
std::string getString(ComponentSourceType component_origin);

}
using arya::ComponentSourceType;
using arya::getComponentSourceType;
using arya::getString;
}