/**
 *
 * Declaration of class Participant as redirect
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

#include <fep3/participant/participant.h>

namespace fep3
{
namespace core
{
namespace arya
{
using fep3::arya::Participant;
using fep3::arya::createParticipant;

} // namespace arya
using arya::Participant;
using arya::createParticipant;

} // namespace core
} // namespace fep3
