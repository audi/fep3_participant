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

#include <a_util/filesystem.h>

#include <fep3/fep3_participant_types.h>
#include <fep3/components/base/component_registry.h>
#include <fep3/participant/component_factories/component_factory_base.h>

namespace fep3
{
namespace arya
{

class ComponentRegistryFactory
{
public:
    /**
     * @brief Creates a component registry
     * The components are created according to the rules in a components configuration file if such file
     * is found as follows:
     * - If the environment variable "FEP3_PARTICIPANT_COMPONENTS_FILE_PATH" is set and it 
     *    contains an absolute file path and such file is found at that path.
     * - If the environment variable "FEP3_PARTICIPANT_COMPONENTS_FILE_PATH" is set and it
     *    contains a relative file path and such file is found in the current working directory 
     *    or in the directory where the fep3_participant shared library file resides in 
     * - If the environment variable "FEP3_PARTICIPANT_COMPONENTS_FILE_PATH" is not set and 
     *    the file "./fep3_participant.fep_components" is found in the current working directory 
     *    or in the directory where the fep3_participant shared library file resides in 
     * Otherwise, i. e. if no components configuration file is found, the a default set of native
     * components is created.
     * 
     * @throw Throws an exception of type std::runtime_error if
     *  * the environment variable "FEP3_PARTICIPANT_COMPONENTS_FILE_PATH" is set but no file can be found at the that location
     *  * parsing the components configuration file fails, e. g. if its content does not conform to the expected schema
     *  * opening one of the shared libraries as referred to by the components configuration file
     * @return Shared pointer to the created component registry
     */
    static std::shared_ptr<ComponentRegistry> createRegistry();
private:
    static std::shared_ptr<ComponentRegistry> createRegistryByFile(const a_util::filesystem::Path& file_path);
    static std::shared_ptr<ComponentRegistry> createRegistryDefault();
};

}
using arya::ComponentRegistryFactory;
}