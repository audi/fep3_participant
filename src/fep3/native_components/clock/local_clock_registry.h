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

#include <fep3/components/clock/clock_registry_intf.h>
#include <fep3/components/logging/logging_service_intf.h>
#include "local_system_clock_discrete.h"

namespace fep3
{
namespace native
{

/**
* @brief Native implementation of a clock registry.
*/
class LocalClockRegistry : public IClockRegistry
{
public:  
    LocalClockRegistry();
    ~LocalClockRegistry() = default;

public: // inherited via IClockRegistry  
    fep3::Result registerClock(const std::shared_ptr<IClock>& clock) override;
    fep3::Result unregisterClock(const std::string& clock_name) override;
    std::list<std::string> getClockNames() const override;
    std::shared_ptr<IClock> findClock(const std::string& clock_name) const override;

public:
    void setLogger(const std::shared_ptr<const ILoggingService::ILogger>& logger);

private: 
    fep3::Result unregisterClockUnlocked(const std::string& clock_name);
    fep3::Result logError(const fep3::Result error) const;

private:
    mutable std::mutex                              _mutex;
    Clocks                                          _clocks;
    std::shared_ptr<const ILoggingService::ILogger> _logger;
};

} // namespace native
} // namespace fep3