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
#include "cpp_plugin.h"
#include <fep3/fep3_participant_version.h>
#include <a_util/filesystem.h>
#include <a_util/strings.h>

#include <sstream>
#ifdef WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#include <string.h>
#endif

namespace fep3
{
namespace arya
{
    CPPPlugin::CPPPlugin(const std::string& filepath)
    {
        load(filepath);
    }

    CPPPlugin::~CPPPlugin()
    {

    }

    std::unique_ptr<IComponent> CPPPlugin::createComponent(const std::string& iid) const
    {
        auto factory = _getFactory();
        if (factory)
        {
            return factory->createComponent(iid);
        }
        else
        {
            return {};
        }
    }

    void CPPPlugin::load(const std::string& filepath)
    {
        std::string so_file = filepath;
        a_util::strings::trim(so_file);
        a_util::filesystem::Path full_file_path{so_file};
        auto file_name = full_file_path.getLastElement().toString();
        full_file_path.removeLastElement();

#ifndef WIN32
        // add prefix if not there yet
        if(0 != file_name.find("lib"))
        {
            file_name = "lib" + file_name;
        }
#endif
        full_file_path.append(file_name);
        if (full_file_path.getExtension().empty())
        {
            // add extension
#ifdef WIN32
            full_file_path.replaceExtension("dll");
#else
            full_file_path.replaceExtension("so");
#endif
        }

        const auto& full_file_path_string = full_file_path.toString();
#ifdef WIN32
        auto library_handle = ::LoadLibrary(full_file_path_string.c_str());
        if (!library_handle)
        {
            throw std::runtime_error("unable to load shared library '" + so_file + "'");
        }
#else
        auto library_handle = ::dlopen(full_file_path_string.c_str(), RTLD_LAZY);
        if (!library_handle)
        {
            throw std::runtime_error("unable to load shared library '" + so_file + "': " + dlerror());
        }
#endif

#ifdef WIN32
        _isDebugPlugin = reinterpret_cast<bool(*)()>(::GetProcAddress(library_handle, SYMBOL_fep3_isDebugPlugin));
        if (!_isDebugPlugin)
        {
            throw std::runtime_error("the shared library '" + so_file + "' does not provide the required fep3_isDebugPlugin method");
        }

#ifdef _DEBUG
        if (!_isDebugPlugin())
        {
            throw std::runtime_error("the shared library '" + so_file + "' is not compiled in debug mode which this executable is");
        }
#else
        if (_isDebugPlugin())
        {
            throw std::runtime_error("the shared library '" + so_file + "' is compiled in debug mode which this executable is not");
        }
#endif
#endif

#ifdef WIN32
        _getFactory = reinterpret_cast<ICPPPluginComponentFactory*(*)()>(::GetProcAddress(library_handle, SYMBOL_fep3_getFactory));
#else
        _getFactory = reinterpret_cast<ICPPPluginComponentFactory*(*)()>(dlsym(library_handle, SYMBOL_fep3_getFactory));
#endif
#ifdef WIN32
        _getVersion = reinterpret_cast<const char*(*)()>(::GetProcAddress(library_handle, SYMBOL_fep3_getVersion));
#else
        _getVersion = reinterpret_cast<const char*(*)()>(dlsym(library_handle, SYMBOL_fep3_getVersion));
#endif
        if (!_getFactory)
        {
            const char* plugin_version = FEP3_PARTICIPANT_LIBRARY_VERSION_STR;

            if (_getVersion)
            {
                plugin_version = _getVersion();
            }

            std::ostringstream error;
            error << "The shared library '" << filepath << "' does not provide the required version of the fep3_getFactory method."
                                << "This participant uses version " << FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR
                                << "." << FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR 
                                << " of the fep_participant library, the plugin was built with version " << plugin_version << ".";
            throw std::runtime_error(error.str());
        }
    }


}
}
