/**
 *
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

#include <fep3/fep3_participant_types.h>
#include <fep3/fep3_errors.h>
#include <fep3/core/participant.h>

namespace fep3
{
namespace core
{
namespace arya
{

/**
* @brief helper class to change a participants state from system side
* Since the participant does not have another possibility to change the state than from service bus 
* this testing helper will add state machine change functionality for you.
*/
class ParticipantStateChanger
{
public:
    /**
     * @brief CTOR
     * 
     * @param part participant to manage
     * @remark the participants servicebus will be used to obtain the participant requester 
     */
    ParticipantStateChanger(Participant& part);
    /**
     * @brief DTOR
     * 
     */
    virtual ~ParticipantStateChanger();

    /**
     * @brief Copy CTOR
     * 
     * @param other the other to copy from
     */
    ParticipantStateChanger(const ParticipantStateChanger& other) = default;
    /**
     * @brief Copy operator
     * 
     * @param other the other to copy from
     * @return the copied ParticipantStateChanger
     */
    ParticipantStateChanger& operator=(const ParticipantStateChanger& other) = default;
    /**
     * @brief move CTOR
     * 
     * @param other the other to move from
     */
    ParticipantStateChanger(ParticipantStateChanger&& other) = default;
    /**
     * @brief move operator
     * 
     * @param other the other to move from
     * @return the moved ParticipantStateChanger
     */
    ParticipantStateChanger& operator=(ParticipantStateChanger&& other) = default;

    /**
     * @brief sends a load request
     * 
     * @return true request reached participant
     * @return false request did not reach participant
     */
    bool load();
    /**
     * @brief sends a unload request
     * 
     * @return true request reached participant
     * @return false request did not reach participant
     */
    bool unload();
    /**
     * @brief sends a initialize request
     * 
     * @return true request reached participant
     * @return false request did not reach participant
     */
    bool initialize();
    /**
     * @brief sends a deinitialize request
     * 
     * @return true request reached participant
     * @return false request did not reach participant
     */
    bool deinitialize();
    /**
     * @brief sends a start request
     * 
     * @return true request reached participant
     * @return false request did not reach participant
     */
    bool start();
    /**
     * @brief sends a pause request
     * 
     * @return true request reached participant
     * @return false request did not reach participant
     */
    bool pause();
    /**
     * @brief sends a stop request
     * 
     * @return true request reached participant
     * @return false request did not reach participant
     */
    bool stop();
    /**
     * @brief sends a exit request
     * 
     * @return true request reached participant
     * @return false request did not reach participant
     */
    bool shutdown();

private:
    /**
     * @cond no_documentation
     * 
     */
    class Impl;
    std::unique_ptr<Impl> _impl;
    /**
     * @endcond no_documentation
     * 
     */
};

} // namespace arya
} // namespace core
} // namespace fep3
