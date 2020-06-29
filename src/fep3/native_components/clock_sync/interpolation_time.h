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

#pragma once

#include <fep3/fep3_timestamp.h>
#include <fep3/fep3_duration.h>

namespace fep3
{

/**
* Interface for a clock which interpolates time received from a master clock
**/
class IInterpolationTime
{
public:
    /**
     * @brief DTOR
     */
    virtual ~IInterpolationTime() = default;

    /**
     * Calculate and return a currently valid timestamp extrapolated from a reference
     * time set with \c setTime().
     * @return The currently valid extrapolated timestamp.
     */
    virtual Timestamp getTime() const = 0;

    /**
     * Set a new reference time obtained from a request.
     * @param [in] time  the reference time stamp.
     * @param [in] roundtrip_time  The time it took to request the reference time and to get an answer.
     */
    virtual void setTime(Timestamp time, Duration roundtrip_time) = 0;

    /**
     * Set a new reference time obtained without further delay.
     * @param [in] time  the reference time stamp.
     */
    virtual void resetTime(Timestamp time) = 0;
};

/**
 * This class provides the means to extrapolate a timestamp relative to a reference time.
 * The class uses Cristian's Algorithm to extrapolate the current valid timestamp using a reference
 * time and a roundtrip time.
 **/
class InterpolationTime : public IInterpolationTime
{
public:
    /** 
     * CTOR
     */
    InterpolationTime();

    /**
    *\copydoc IInterpolationTime::getTime
    **/
    Timestamp getTime() const override;

    /**
    *\copydoc IInterpolationTime::setTime
    **/
    void setTime(Timestamp time, Duration roundtrip_time) override;

    /**
    *\copydoc IInterpolationTime::resetTime
    **/
    void resetTime(Timestamp time) override;
private:
    // Stores the last value calculated by \c getTime
    mutable Timestamp _last_interpolated_time;
    // Offset of local time to reference time
    Duration _offset;
    // Stores the reference time extrapolated to the moment of reception
    Timestamp _last_time_set;
    // Stores the raw time value of the reference time
    Timestamp _last_raw_time;
};

} // namespace fep3