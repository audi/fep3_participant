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
#else
#include <dlfcn.h>
#endif

#include "binary_info.h"

namespace fep3
{
namespace binary_info
{

a_util::filesystem::Path getFilePath()
{
    a_util::filesystem::Path current_binary_file_path;
#ifdef WIN32
    HMODULE hModule = nullptr;
    if(GetModuleHandleEx
        (GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
        , (LPCTSTR)getFilePath
        , &hModule
        ))
    {
        std::vector<wchar_t> file_path_buffer;
        DWORD number_of_copied_characters = 0;
        // note: to support paths with length > MAX_PATH we have do trial-and-error
        // because GetModuleFileName does not indicate if the path was truncated
        while(number_of_copied_characters >= file_path_buffer.size())
        {
            file_path_buffer.resize(file_path_buffer.size() + MAX_PATH);
            number_of_copied_characters = GetModuleFileNameW(hModule, &file_path_buffer[0], static_cast<DWORD>(file_path_buffer.size()));
        }
        file_path_buffer.resize(number_of_copied_characters);
        current_binary_file_path = std::string(file_path_buffer.cbegin(), file_path_buffer.cend());
    }
#else   // WIN32
    Dl_info dl_info;
    dladdr(reinterpret_cast<void*>(getFilePath), &dl_info);
    current_binary_file_path = dl_info.dli_fname;
#endif
    return current_binary_file_path.getParent();
}

} // namespace binary_info
} // namespace fep3