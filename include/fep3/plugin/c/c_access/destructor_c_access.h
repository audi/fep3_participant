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

#include <memory>

#include <fep3/plugin/c/c_intf/destruction_manager_c_intf.h>
#include <fep3/plugin/c/destructor_intf.h>

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
 * @brief Class calling destroy(...) on a C access structure of \p access_type
 * Use this class to automatically destroy a remote object when the instance of
 * this class is destroyed.
 * @pre \p access_type must have fields "destroy" and "_handle"
 *
 * @tparam access_type Type of the C access structure
 */
template<typename access_type>
class Destructor : public IDestructor
{
public:
    /**
     * CTOR
     * @param access C access structure to call destroy on upon destruction
     */
    Destructor(const access_type& access)
        : _access(access)
    {}
    /**
     * DTOR calling destroy(...) on the C access structure
     */
    virtual ~Destructor()
    {
        if(nullptr != _access.destroy)
        {
            _access.destroy(_access._handle);
        }
    }

private:
    access_type _access;
};

} // namespace arya
using arya::Destructor;
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3
