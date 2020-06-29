/**
 * Declaration of class Participant
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

#include <fep3/fep3_participant_version.h>
#include <fep3/fep3_participant_export.h>
#include <fep3/fep3_errors.h>

#include "../core/participant.h"
#include "../core/element_factory.h"

namespace fep3
{
namespace cpp
{
namespace arya
{
using fep3::core::arya::Participant;

/**
 * @brief Creates a special particpant without a named system access (it will be part of the "default_system")
 * @param name Name of the participant to be created
 * @tparam element_factory type of the fatory which is able to create the element
 * @return The created participant
*/
template<typename element_type>
Participant createParticipant(const std::string& name)
{
    auto elem_factory = std::make_shared<fep3::core::arya::ElementFactory<element_type>>();
    return fep3::core::arya::createParticipant(name,
        FEP3_PARTICIPANT_LIBRARY_VERSION_STR,
        std::string(),
        elem_factory);
}
/**
 * @brief Creates a special particpant 
 * @param name Name of the participant to be created
 * @param system_name Name of the system the participant belongs to 
 * @tparam element_factory type of the fatory which is able to create the element
 * @return The created participant
*/
template<typename element_type>
Participant createParticipant(const std::string& name,
                              const std::string& system_name)
{
    auto elem_factory = std::make_shared<fep3::core::arya::ElementFactory<element_type>>();
    return fep3::core::arya::createParticipant(name,
        FEP3_PARTICIPANT_LIBRARY_VERSION_STR,
        system_name,
        elem_factory);
}



}
using arya::Participant;
using arya::createParticipant;
} // namespace core
} // namespace fep3
