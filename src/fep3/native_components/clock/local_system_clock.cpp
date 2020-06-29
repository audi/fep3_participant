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

#include "local_system_clock.h"

#include <a_util/system/system.h>

#include <fep3/components/clock/clock_service_intf.h>

namespace fep3
{
namespace native
{

LocalSystemRealClock::LocalSystemRealClock()
    : ContinuousClock(FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME)
    , _current_offset(std::chrono::steady_clock::now())
{
}

Timestamp LocalSystemRealClock::getNewTime() const
{
    using namespace std::chrono;

    if (_started)
    {           
        return Timestamp{steady_clock::now() - _current_offset};
    }
    else
    {      
        return Timestamp{ 0 };
    }
}

Timestamp LocalSystemRealClock::resetTime()
{
    using namespace std::chrono;

    _current_offset = steady_clock::now();

    return getNewTime();
}

} // namespace native
} // namespace fep3
