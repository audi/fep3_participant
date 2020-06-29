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

#include "participant.h"
#include "participant_state_changer.h"
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <future>

namespace fep3
{
namespace core
{
namespace arya
{
/**
 * This template makes sure to execute an instance of a participant in a separate thread.
 * It will call the the exec function asynchronously, so it is possible to continue testing.
 * Additionally you can send state machine change requests.
 */
class ParticipantExecutor : public ParticipantStateChanger
{
public:
    /**
     * @brief CTOR
     * Do not forget to call \c exec afterwards!
     * 
     * @param participant the participant to execute
     */
    explicit ParticipantExecutor(fep3::core::arya::Participant& participant)
        : _participant(participant), ParticipantStateChanger(participant)
    {
    }
    /**
     * @brief executor function which will call the \c Participant::exec function in a separated thread.
     * @throws std::runtime_error this function throws if something went wrong
     */
    void exec()
    {
        if (_exec_wait_thread.joinable())
        {
            throw std::runtime_error("invalid state of executor");
        }

        std::promise<bool> exec_loop_started;
        auto started = exec_loop_started.get_future();
        _exec_wait_thread = std::thread([&]
        {
            auto ret = _participant.exec(
                [&] {
                     exec_loop_started.set_value(true);
                 });
            if (ret != 0)
            {
                exec_loop_started.set_value(false);
            }
        });

        auto started_value = started.get();
        if (started_value)
        {
            //everything went fine
        }
        else
        {
            throw std::runtime_error("executor start error");
        }
    }
    /**
     * @brief DTOR
     * 
     */
    ~ParticipantExecutor()
    {
        stop();
        deinitialize();
        unload();
        shutdown();
        if (_exec_wait_thread.joinable())
        {
            _exec_wait_thread.join();
        }
    }
    
private:
    /**
     * @cond no_documentation
     * 
     */
    fep3::core::arya::Participant& _participant;
    std::thread _exec_wait_thread;
    /**
     * @endcond no_documentation
     * 
     */
};

}
using arya::ParticipantExecutor;
}
}