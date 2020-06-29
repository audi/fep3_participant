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

#include "shared_binary_intf.h"

namespace fep3
{
namespace plugin
{
namespace c
{
namespace arya
{

/**
 * @brief The binary manager can hold a strong reference to a shared binary
 */
class SharedBinaryManager
{
public:
    /**
     * DTOR
     */
    virtual ~SharedBinaryManager() = default;
    /**
     * resets the mananged shared binary
     * @param shared_binary the shared binary to manage
     */
    virtual void setSharedBinary(const std::shared_ptr<ISharedBinary>& shared_binary) final
    {
        _shared_binary = shared_binary;
    }
    /**
     * gets the mananged shared binary
     * @return the shared binary currently managing
     */
    virtual std::shared_ptr<ISharedBinary> getSharedBinary() const final
    {
        return _shared_binary;
    }
private:
    /// reference counting for the shared binary
    std::shared_ptr<ISharedBinary> _shared_binary;
};

} // namespace arya
using arya::SharedBinaryManager;
} // namespace c
} // namespace plugin
} // namespace fep3
