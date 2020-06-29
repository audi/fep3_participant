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
#include <mutex>

#include <fep3/fep3_participant_export.h>
#include <fep3/components/base/component_base.h>
#include <fep3/participant/element_manager/element_manager.h>
#include <fep3/participant/element_intf.h>
#include <fep3/components/logging/logging_service_intf.h>

namespace fep3
{
namespace arya
{

/**
 * @brief Class implementing the participant state machine
 *
*/
class ParticipantStateMachine
{
public:
    /**
     * CTOR
     *
    */
    ParticipantStateMachine
        (ElementManager element_manager
        , const std::shared_ptr<ComponentRegistry>& component_registry
        , const std::shared_ptr<ILoggingService::ILogger>& participant_logger);
    /**
     * DTOR
     *
    */
    virtual ~ParticipantStateMachine();

    /**
     * Gets the finalized state of the participant state machine
     *
     * @return true if the participant state machine has finalized (i. e. is in its final state), false otherwise
    */
    virtual bool isFinalized() const final;
    /**
     * Gets the name of the state the participant current is in
     *
    */
    virtual std::string getCurrentStateName() const final;

    /**
     * Triggers the event "exit"
     *
     * @return true if event was handled, false otherwise
    */
    virtual bool exit() final;
    /**
     * Triggers the event "load"
     *
     * @return true if event was handled, false otherwise
    */
    virtual bool load() final;
    /**
     * Triggers the event "unload"
     *
     * @return true if event was handled, false otherwise
    */
    virtual bool unload() final;
    /**
     * Triggers the event "initialize"
     *
     * @return true if event was handled, false otherwise
    */
    virtual bool initialize() final;
    /**
     * Triggers the event "deinitialize"
     *
     * @return true if event was handled, false otherwise
    */
    virtual bool deinitialize() final;

    /**
     * Triggers the event "reinitialize"
     *
     * @return true if event was handled, false otherwise
    */
    virtual bool stop() final;
    /**
     * Triggers the event "run"
     *
     * @return true if event was handled, false otherwise
    */
    virtual bool start() final;
    /**
     * Triggers the event "hold"
     *
     * @return true if event was handled, false otherwise
    */
    virtual bool pause() final;

private:
    mutable std::mutex _mutex;
    bool _finalized{false};
    class Impl;
    std::unique_ptr<Impl> _impl;
};

} // namespace arya
} // namespace fep3
