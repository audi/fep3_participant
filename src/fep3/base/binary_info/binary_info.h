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

#include <a_util/filesystem.h>

namespace fep3
{
// Namespace providing information on the binary
namespace binary_info
{

a_util::filesystem::Path getFilePath();

} // namespace binary_info
} // namespace fep3