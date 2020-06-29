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

namespace fep3
{
namespace plugin
{
namespace c
{
namespace arya
{

/**
 * @brief Interface for a destructor object
 */
class IDestructor
{
public:
    virtual ~IDestructor() = default;
};

} // namespace arya
using arya::IDestructor;
} // namespace c
} // namespace plugin
} // namespace fep3
