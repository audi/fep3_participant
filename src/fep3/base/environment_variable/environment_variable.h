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

#include <fep3/fep3_result_decl.h>
#include <fep3/fep3_optional.h>

namespace fep3
{
// Note: this namespace provides functionality similar to a subset of a_util version 4.x.x "cSystem" class.
// As such funcionality is not yet available in a_util 5.x.x, it was added here
namespace environment_variable
{

Optional<std::string> get(const std::string& strName);
Result set(const std::string& name, const std::string& value);

} // namespace environment_variable
} // namespace fep3