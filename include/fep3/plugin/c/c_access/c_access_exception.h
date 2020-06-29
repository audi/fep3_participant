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

#include <exception>
#include <string>

#include <fep3/plugin/c/c_intf/c_intf_errors.h>

namespace fep3
{
namespace plugin
{
namespace c
{
namespace access
{
namespace arya
{

/**
 * Exception class representing exceptions in the C plugin access
 */
class Exception : public std::exception
{
public:
    /**
     * CTOR
     * @param error Error information to be passed with the exception
     */
    Exception(fep3_plugin_c_InterfaceError error)
        : _what(std::to_string(error))
    {}
    /**
     * @brief Gets the error information as string
     * @return Error information as string
     */
    virtual const char* what() const noexcept override
    {
        return _what.c_str();
    }
private:
    //fep3_plugin_c_InterfaceError _error;
    std::string _what;
};

} // namespace arya
using arya::Exception;
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3
