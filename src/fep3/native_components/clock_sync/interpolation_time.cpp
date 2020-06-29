/**
 * @file
 * Copyright &copy; Audi AG. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 */

#include "interpolation_time.h"

#include <a_util/system/system.h>

namespace fep3
{

InterpolationTime::InterpolationTime() 
    : _last_interpolated_time(0)
    , _offset(0)
    , _last_time_set(0)
    , _last_raw_time(0)
{
}

Timestamp InterpolationTime::getTime() const
{
    using namespace std::chrono;

    if (_last_time_set.count() > 0)
    {
        const auto time = steady_clock::now().time_since_epoch() - _offset;
        if (_last_interpolated_time < time)
        {
            _last_interpolated_time = time;
        }
        return _last_interpolated_time;
    }
    else
    {
        return _last_time_set; //not yet received a time!!
    }
}

void InterpolationTime::setTime(const Timestamp time,const  Duration roundtrip_time)
{
    using namespace std::chrono;

    //autodetection of a reset
    if (time < _last_raw_time)
    {
        resetTime(time);
    }
    _last_raw_time = time;

    // Implementation of https://en.wikipedia.org/wiki/Cristian%27s_algorithm
    _last_time_set = time + roundtrip_time / 2;
    _offset = steady_clock::now().time_since_epoch() - _last_time_set;
}

void InterpolationTime::resetTime(const Timestamp time)
{
    using namespace std::chrono;

    _last_raw_time = time;
    _last_time_set = time;
    _offset = steady_clock::now().time_since_epoch() - time;
    _last_interpolated_time = time;
}

} // namespace fep3
