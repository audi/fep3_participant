/**
 * Declaration of class ElementBase
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

#include <fep3/core/element_base.h>
#include <fep3/fep3_participant_version.h>
#include "datajob.h"

#include <string>
#include <memory>

namespace fep3
{
namespace cpp
{
namespace arya
{
/**
 * @brief This Simple Element Type will create and add one single DataJob implementation.
 * 
 * @tparam data_job_type Type of the DataJob to add to the Element
 */
template<typename data_job_type>
class DataJobElement : public core::arya::ElementBase
{

public:
    /**
     * CTOR
     *
     */
    DataJobElement()
        : ElementBase("fep3::cpp::DataJobElement",
                      FEP3_PARTICIPANT_LIBRARY_VERSION_STR),
          _job(std::make_shared<data_job_type>()),
          _need_reset(true)
    {
    }

    fep3::Result load() override
    {
        auto config_service = getComponents()->getComponent<fep3::arya::IConfigurationService>();
        if (config_service && _job)
        {
            return _job->initConfiguration(*config_service);
        }
        return {};
    }

    void unload() override
    {
        if (_job)
        {
            _job->deinitConfiguration();
        }
    }

    void stop() override
    {
        if (_job)
        {
            _need_reset = true;
        }
    }

    fep3::Result run() override
    {
        if (_job && _need_reset)
        {
            _need_reset = false;
            return _job->reset();
        }
        return {};
    }

    fep3::Result initialize() override
    {
        if (_job)
        {
            const auto components = getComponents();
            if (components)
            {
                return addToComponents({ _job }, *components);
            }
            else
            {
                RETURN_ERROR_DESCRIPTION(ERR_INVALID_ADDRESS, "components reference invalid");
            }
        }
        else
        {
            RETURN_ERROR_DESCRIPTION(ERR_INVALID_ADDRESS, "job was not initialized in element");
        }
    }
    void deinitialize() override
    {
        if (_job)
        {
            removeFromComponents({ _job }, *getComponents());
        }
    }
private:
    /// data job
    std::shared_ptr<DataJob> _job;
    bool                     _need_reset;
};

}

using arya::DataJobElement;

} // namespace core
} // namespace fep3
