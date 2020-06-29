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

#ifdef WIN32
#include <Windows.h>
#include <vector>
#else
#include <stdlib.h>
#endif

#include <fep3/fep3_errors.h>

#include "environment_variable.h"

namespace fep3
{
namespace environment_variable
{

Optional<std::string> get(const std::string& name)
{
#ifdef WIN32
    std::wstring wide_string_name(name.cbegin(), name.cend());
    const DWORD length = GetEnvironmentVariableW(wide_string_name.c_str(), nullptr, 0);
    if (0 < length)
    {
        std::vector<wchar_t> value_buffer(length);
        GetEnvironmentVariableW(wide_string_name.c_str(), &value_buffer[0], static_cast<DWORD>(value_buffer.size()));
        return std::string(value_buffer.cbegin(), value_buffer.cend());
    }
#else //WIN32
    char* const value = std::getenv(name.c_str());
    if(nullptr != value)
    {
        return value;
    }
#endif
    return {};
}

Result set(const std::string& name, const std::string& value)
{
#ifdef WIN32
    if (FALSE == ::SetEnvironmentVariable(name.c_str(), value.c_str()))
    {
        RETURN_ERROR_DESCRIPTION(ERR_FAILED, "failed to set environment variable %s", name.c_str());
    }
#else //WIN32
    if (setenv(name.c_str(), value.c_str(), 1) != 0)
    {
        RETURN_ERROR_DESCRIPTION(ERR_FAILED, "failed to set environment variable %s", name.c_str());
    }
#endif
    return {};
}

} // namespace environment_variable
} // namespace fep3