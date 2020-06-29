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

#include <fep3/plugin/base/shared_library.h>
#include <fep3/plugin/base/fep3_calling_convention.h>
#include <fep3/plugin/c/shared_binary_intf.h>
#include <fep3/plugin/c/c_intf/shared_binary_c_intf.h>
#include <fep3/plugin/c/c_wrapper/shared_binary_c_wrapper.h>
#include <fep3/plugin/c/c_intf/c_intf_errors.h>
#include <fep3/plugin/base/host_plugin_base.h>

namespace fep3
{
namespace plugin
{
namespace c
{
namespace arya
{

/**
 * Class representing a plugin
 */
class HostPlugin
    : public HostPluginBase
    , public ISharedBinary
    , public std::enable_shared_from_this<HostPlugin>
{
public:
    /**
     * CTOR
     * @param file_path The file path of the plugin to be loaded;
     *                  prefix and extension are automatically added if not present, i. e.:
     *                  * on non-windows /lib/my_stuff results in loading of /lib/libmy_stuff.so
     *                  * on windows C:\lib\my_stuff results in loading of C:\lib\my_stuff.dll
     */
    HostPlugin(const std::string& file_path);
    /**
     * DTOR
     */
    virtual ~HostPlugin();


    /**
     * Creates an object of type @p t encapsulating access to an object residing in the plugin
     * by calling the factory function symbol \p factory_function_symbol_name in the plugin.
     *
     * @tparam t The type of the object to be created
     * @tparam argument_types Types of arguments to be forwarded to the factory function
     * @pre @p t defines type "Access" that provides access to the object to be created in the plugin
     * @param factory_function_symbol_name The name of the factory function symbol
     * @param arguments Arguments to be forwarded to the factory function
     * @return Unique pointer to the created object if the @p factory_function_symbol_name
     *          was found, empty unique pointer otherwise
     * @return Unique pointer to the created object if one was created, empty unique pointer otherwise
     */
    template<typename t, typename... argument_types>
    std::unique_ptr<t> create
        (const std::string& factory_function_symbol_name
        , argument_types&&... arguments
        )
    {
        // create new object managing the lifetime of the binary (the plugin)
        typename t::Access access{};
        typedef fep3_plugin_c_InterfaceError (FEP3_PLUGIN_CALL FactoryFunction)
            (typename t::Access*, fep3_plugin_c_arya_SISharedBinary, argument_types...);
        const auto& factory_function = get<FactoryFunction>(factory_function_symbol_name);
        if (nullptr == factory_function)
        {
            throw std::runtime_error("couldn't get factory function '" + factory_function_symbol_name
                + "' from library '" + getFilePath() + "'");
        }
        fep3_plugin_c_InterfaceError error = factory_function
            (&access
            , fep3_plugin_c_arya_SISharedBinary
                {reinterpret_cast<fep3_plugin_c_arya_HISharedBinary>(new std::shared_ptr<ISharedBinary>(shared_from_this()))
                , wrapper::SharedBinary::destroy
                }
            , arguments...
            );
        if(fep3_plugin_c_interface_error_none != error)
        {
            throw std::runtime_error("calling factory function '" + factory_function_symbol_name
                + "' of library '" + getFilePath() + "' returned error '" + std::to_string(error) + "'");
        }
        if(nullptr != access._handle)
        {
            return std::make_unique<t>(typename t::Access(std::move(access)), shared_from_this());
        }
        return {};
    }
};

} // namespace arya
using arya::HostPlugin;
} // namespace c
} // namespace plugin
} // namespace fep3
