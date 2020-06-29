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

#include <string>
#include <memory>

#include <fep3/fep3_participant_export.h>
#include <fep3/fep3_timestamp.h>

namespace fep3
{
namespace arya
{

/**
* @brief Interface of a clock
*
*/
class FEP3_PARTICIPANT_EXPORT IClock
{
public:
    /**
     * @brief Event sink to react synchronously on time reset and time update events.
     *
     */
    class IEventSink
    {
    public:
        /**
        * @brief DTOR
        *
        */
        virtual ~IEventSink() = default;

        public:
            /**
             * @brief This event is emitted before the time is updated.
             * The @ref IClock::getTime value is still @p old_time.
             * @remark This event is only emitted by discrete clocks (@ref ClockType::discrete)
             *
             * @param old_time The time before updating
             * @param new_time The future time after updating
             */
            virtual void timeUpdateBegin(arya::Timestamp old_time, arya::Timestamp new_time) = 0;

            /**
             * @brief This event is emitted while the time is beeing updated.
             * @remark This event is only emitted by discrete clocks (@ref ClockType::discrete)
             *
             * @param new_time The future time after updating
             */
            virtual void timeUpdating(arya::Timestamp new_time) = 0;

            /**
             * @brief This event is emitted after the time was updated.
             * The @ref IClock::getTime value was set to the @p new_time.
             * @remark This event is only emitted by discrete clocks (@ref ClockType::discrete)
             *
             * @param new_time The current time after the update
             */
            virtual void timeUpdateEnd(arya::Timestamp new_time) = 0;

            /**
             * @brief This event is emitted before the time will be reset.
             * It is used to inform about time jumps to the future or the past!
             * The IClock::getTime value is still @p old_time.
             *
             * @param old_time The time before reseting
             * @param new_time The future time after resetting
             */
            virtual void timeResetBegin(arya::Timestamp old_time, arya::Timestamp new_time) = 0;

            /**
             * @brief This event is emitted after the time was reset.
             * The @ref IClock::getTime value was already set to @p new_time.
             *
             * @param new_time The current time after the reset
             */
            virtual void timeResetEnd(arya::Timestamp new_time) = 0;
    };

    /**
     * @brief Type of a clock
     *
     */
    enum class ClockType
    {
            /**
            * @brief A continuous clock will steadily raise the time value
            */
            continuous = 0,
            /**
            * @brief A discrete clock will jump configured or calculated time steps.
            * @remark It depends on the implementation which logical time steps are used.
            */
            discrete = 1
    };

protected:
    /**
     * @brief DTOR
     *
     */
    virtual ~IClock() = default;

public:
    /**
     * @brief Get the Name of the clock.
     *
     * @return Name of the clock
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Get the Type of the clock.
     *
     * @return The type of the clock
     * @retval ClockType::continuous The clock implementation is a continuous clock
     * @retval ClockType::discrete   The clock implementation is a discrete clock
     */
    virtual ClockType getType() const = 0;


    /**
     * @brief Get the current time of the clock.
     *
     * @return Current time of the clock
     */
    virtual arya::Timestamp getTime() const = 0;

    /**
     * @brief Reset the clock.
     * Depending on the implementation the offset will be reset.
     * @remark The @ref IEventSink::timeResetBegin and the @ref IEventSink::timeResetEnd will be emitted
     */
    virtual void reset() = 0;

    /**
     * @brief Start the clock.
     * @remark The reset events @ref IEventSink::timeResetBegin and @ref IEventSink::timeResetEnd will be emitted to @p event_sink.
     * @remark If @p event_sink is a @ref IClock::ClockType::discrete also the events
     * @ref IEventSink::timeUpdateBegin, @ref IEventSink::timeUpdating, @ref IEventSink::timeUpdateEnd will be emitted to @p event_sink.
     *
     * @param event_sink The event sink to emit time reset and time updating events to.
     */
    virtual void start(const std::weak_ptr<IEventSink>& event_sink) = 0;

    /**
     * @brief Stop the clock.
     * Usually the clock will reset immediately (still depending on the implementation).
     */
    virtual void stop() = 0;
};

} // namespace arya
using arya::IClock;
} // namespace fep3
