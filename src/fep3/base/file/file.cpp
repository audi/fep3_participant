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

#include "file.h"

namespace fep3
{
namespace file
{

a_util::filesystem::Path find
    (const a_util::filesystem::Path& file_path
    , const std::vector<a_util::filesystem::Path>& hints
    )
{
    // if path is given absolute, there is no need to evaluate the hints
    if(file_path.isAbsolute())
    {
        if(a_util::filesystem::exists(file_path))
        {
            return file_path;
        }
    }
    else
    {
        for(const auto& hint : hints)
        {
            const auto& search_file_path = hint + file_path;
            if(a_util::filesystem::exists(search_file_path))
            {
                return search_file_path;
            }
        }
    }
    return {};
}

} // namespace file
} // namespace fep3