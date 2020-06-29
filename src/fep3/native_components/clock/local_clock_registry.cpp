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

#include "local_clock_registry.h"

#include <algorithm>
#include <iterator>

#include <a_util/strings/strings_format.h>
#include <fep3/fep3_errors.h>

namespace fep3
{
namespace native
{

LocalClockRegistry::LocalClockRegistry()
{
}

void LocalClockRegistry::setLogger(const std::shared_ptr<const ILoggingService::ILogger>& logger)
{
    _logger = logger;
}

std::list<std::string> LocalClockRegistry::getClockNames() const
{
    std::lock_guard<std::mutex> lock_guard(_mutex);
    std::list<std::string> clock_list;

    std::transform(
        _clocks.begin(),
        _clocks.end(),
        std::back_inserter(clock_list),
        [](auto map_entry)
    {
        return map_entry.second->getName();
    });

    return clock_list;
}

std::shared_ptr<IClock> LocalClockRegistry::findClock(const std::string& clock_name) const
{
    std::lock_guard<std::mutex> lock_guard(_mutex);

    const auto clocks_iterator = _clocks.find(clock_name);
    if (clocks_iterator != _clocks.end())
    {
        return clocks_iterator->second;
    }
    return nullptr;
}

fep3::Result LocalClockRegistry::registerClock(const std::shared_ptr<IClock>& clock)
{
    std::lock_guard<std::mutex> lock_guard(_mutex);

    if (!clock)
    {
        auto result = CREATE_ERROR_DESCRIPTION(ERR_POINTER,
            a_util::strings::format(
                "Registering clock failed. The clock to be registered is invalid.")
            .c_str());

        result |= _logger->logError(result.getDescription());

        return result;
    }
    else if (_clocks.count(clock->getName()) > 0)
    {
        auto result = CREATE_ERROR_DESCRIPTION(ERR_INVALID_ARG,
            a_util::strings::format(
                "Registering clock failed. A clock with the name %s is already registered.",
                clock->getName().c_str())
            .c_str());

        result |= _logger->logError(result.getDescription());

        return result;
    }
    else
    {
        _clocks.emplace(clock->getName(), clock);

        return {};
    }
}

fep3::Result LocalClockRegistry::unregisterClock(const std::string& clock_name)
{    
    std::lock_guard<std::mutex> lock_guard(_mutex);

    if ((FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME != clock_name) &&
        (FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME != clock_name))
    {
        return unregisterClockUnlocked(clock_name);
    }    

    auto result = CREATE_ERROR_DESCRIPTION(ERR_INVALID_ARG,
        a_util::strings::format(
            "Unregistering clock failed. The clocks %s or %s can not be unregistered.",
            FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME,
            FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME)
        .c_str());

    result |= _logger->logError(result.getDescription());

    return result;
}

fep3::Result LocalClockRegistry::unregisterClockUnlocked(const std::string& clock_name)
{
    if (_clocks.count(clock_name) == 0)
    {
        auto result = CREATE_ERROR_DESCRIPTION(ERR_INVALID_ARG,
            a_util::strings::format(
                "Unregistering clock failed. A clock with the name '%s' is not registered.",
                clock_name.c_str())
            .c_str());

        result |= _logger->logError(result.getDescription());

        return result;
    }
    
    _clocks.erase(clock_name);
    
    return {};
}

fep3::Result LocalClockRegistry::logError(const fep3::Result error) const
{
    if (_logger && _logger->isErrorEnabled())
    {
        return _logger->logError(error.getDescription());
    }
    else
    {
        return {};
    }
}

} // namespace native
} // namespace fep3
