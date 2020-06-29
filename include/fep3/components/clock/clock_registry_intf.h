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

#include <list>
#include <string>
#include <memory>
#include <map>

#include <fep3/fep3_errors.h>
#include <fep3/fep3_participant_export.h>
#include <fep3/components/clock/clock_intf.h>

namespace fep3
{
namespace arya
{

/**
* @brief Interface of the clock registry
* The clock registry may be used to register clocks.
*/
class FEP3_PARTICIPANT_EXPORT IClockRegistry
{
public:
    /**
     * Map of clock entries.
     */
    using Clocks = std::map<std::string, std::shared_ptr<arya::IClock>>;

public:
    /**
     * @brief DTOR
     */
    virtual ~IClockRegistry() = default;

public:

    /**
    * @brief Register a clock.
    * The name of the clock must be unique within this registry.
    *
    * @param clock The clock to register
    * @return fep3::Result
    * @retval ERR_INVALID_ARG       A clock with the same name as @p clock is already registered.
    * @retval ERR_INVALID_STATE     Clock service is in state running in which registration of clocks is not allowed.
    */
    virtual fep3::Result registerClock(const std::shared_ptr<arya::IClock>& clock) = 0;

    /**
    * @brief Unregister a clock by name.
    *
    * @param clock_name The name of the clock
    * @return fep3::Result
    * @retval ERR_INVALID_ARG       The clock with name @p clock_name is a default clock or
    *                               no clock with name @p clock_name is registered.
    * @retval ERR_INVALID_STATE     Clock service is in state running in which deregistration of clocks is not allowed.
    */
    virtual fep3::Result unregisterClock(const std::string& clock_name) = 0;

    /**
    * @brief Get a list of all clock names.
    *
    * @return The list of clocks that are registered
    */
    virtual std::list<std::string> getClockNames() const = 0;

    /**
    * @brief Get a clock by name.
    *
    * @param clock_name The name of the clock
    * @return The clock with name @p clock_name
    */
    virtual std::shared_ptr<arya::IClock> findClock(const std::string& clock_name) const = 0;
};

} // namespace arya
using arya::IClockRegistry;
} // namespace fep3
