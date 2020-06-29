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

#include <fep3/fep3_errors.h>

#include <fep3/components/base/component_iid.h>
#include "scheduler_intf.h"
#include "scheduler_registry_intf.h"

/**
* @brief Main property entry of the scheduling properties 
*/
#define FEP3_SCHEDULER_SERVICE_CONFIG "scheduling"
/**
* @brief The scheduler configuration property name
* Use this to set the scheduler by configuration.
*
*/
#define FEP3_SCHEDULER_PROPERTY "scheduler"
/**
* @brief The scheduler configuration property path to set up the scheduler to use
*/
#define FEP3_SCHEDULER_SERVICE_SCHEDULER FEP3_SCHEDULER_SERVICE_CONFIG "/" FEP3_SCHEDULER_PROPERTY

namespace fep3
{
namespace arya
{

/**
* @brief Interface for the SchedulerService
*
*/
class FEP3_PARTICIPANT_EXPORT ISchedulerService : public arya::ISchedulerRegistry
{
public:
    /// The component interface identifier of ISchedulerService
    FEP_COMPONENT_IID("scheduler_service.arya.fep3.iid");

protected:
    /**
    * @brief DTOR
    *
    */
    ~ISchedulerService() override = default;

public:

    /**
    * @brief Returns the name of the active scheduler.
    * 
    * @return Name of active scheduler
    */
    virtual std::string getActiveSchedulerName() const = 0;
};

} // namespace arya
using arya::ISchedulerService;
} // namespace fep3