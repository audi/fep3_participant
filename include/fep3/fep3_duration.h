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
 
#pragma once

#include <chrono>

/**
 * FEP 3 SDK Participant Library namespace
 */
namespace fep3
{
/**
 * FEP 3 SDK Participant namespace for all interfaces and public implementations introduced in version 3.0.
 */
namespace arya
{

/**
 * Alias for duration in nanoseconds
 */
using Duration = std::chrono::nanoseconds;

} // namespace arya
using arya::Duration;
} // namespace fep3
