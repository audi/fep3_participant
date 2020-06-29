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

#include <map>

#include "component_registry_factory.h"
#include "components_file.h"
#include <fep3/base/environment_variable/environment_variable.h>
#include <fep3/participant/component_factories/built_in/component_factory_built_in.h>
#include <fep3/participant/component_factories/cpp/component_factory_cpp_plugins.h>
#include <fep3/participant/component_factories/c/component_factory_c_plugins.h>
#include <fep3/base/file/file.h>
#include <fep3/base/binary_info/binary_info.h>

#define FEP3_PARTICIPANT_COMPONENTS_FILE_PATH_ENVIRONMENT_VARIABLE "FEP3_PARTICIPANT_COMPONENTS_FILE_PATH"

namespace fep3
{
    namespace arya
    {
        std::map<ComponentSourceType, std::shared_ptr<ComponentFactoryBase>> getFactories(const ComponentsFile& found_file)
        {
            std::map<ComponentSourceType, std::shared_ptr<ComponentFactoryBase>> factories;
            //create the native component factory
            factories[ComponentFactoryBuiltIn::getType()]
                = std::make_shared<ComponentFactoryBuiltIn>();
            //create the CPP Plugin component factory
            factories[ComponentFactoryCPPPlugin::getType()]
                = std::make_shared<ComponentFactoryCPPPlugin>(found_file.getFiles(ComponentFactoryCPPPlugin::getType()));
            //create the C Plugin component factory
            factories[ComponentFactoryCPlugin::getType()]
                = std::make_shared<ComponentFactoryCPlugin>(found_file.getFiles(ComponentFactoryCPlugin::getType()));
            return factories;
        }

        
        std::shared_ptr<ComponentRegistry> ComponentRegistryFactory::createRegistry()
        {
            a_util::filesystem::Path component_config_file_path;
            const std::vector<a_util::filesystem::Path> search_hints
                {::a_util::filesystem::getWorkingDirectory()
                , binary_info::getFilePath()
                };
            // Note: if the environment variable is set, the user tells us to load a specific components configuration file, so 
            // - we do not search for the default file name (see below)
            // - we do not silently load native components only
            // , but rather throw an exception if such file is not found.
            const auto& environment_variable_file_path = environment_variable::get(FEP3_PARTICIPANT_COMPONENTS_FILE_PATH_ENVIRONMENT_VARIABLE);
            if(environment_variable_file_path)
            {
                component_config_file_path = file::find
                    (environment_variable_file_path.value()
                    , search_hints
                    );
                if(component_config_file_path.isEmpty())
                {
                    throw std::runtime_error(std::string() + "Couldn't find components configuration file '" + *environment_variable_file_path + "'");
                }
            }
            else
            {
                // search for the default file name
                component_config_file_path = file::find
                    ("./fep3_participant.fep_components"
                    , search_hints
                    );
            }
            if(component_config_file_path.isEmpty())
            {
                return createRegistryDefault();
            }
            else
            {
                return createRegistryByFile(component_config_file_path);
            }
        }

        std::shared_ptr<ComponentRegistry> ComponentRegistryFactory::createRegistryDefault()
        {
            ComponentFactoryBuiltIn native_factory;
            auto registry = std::make_shared<ComponentRegistry>();
            native_factory.createDefaults(*registry.get());
            return registry;
        }
        std::shared_ptr<ComponentRegistry> ComponentRegistryFactory::createRegistryByFile(const a_util::filesystem::Path& file_path)
        {
            auto registry = std::make_shared<ComponentRegistry>();
            //everything must be clearly defined in a file !
            ComponentsFile comp_file;
            comp_file.load(file_path);

            //create the factories 
            std::map<ComponentSourceType, std::shared_ptr<ComponentFactoryBase>> factories = getFactories(comp_file);
            
            for (const auto& item_to_create : comp_file.getItems())
            {
                auto factory_iterator = factories.find(item_to_create._comp_source_type);
                if (factory_iterator != factories.end())
                {
                    auto created_comp = factory_iterator->second->createComponent(item_to_create._comp_iid);
                    if (created_comp)
                    {
                        registry->registerComponent
                            (item_to_create._comp_iid
                            , std::move(created_comp)
                            );
                    }
                    else
                    {
                         throw std::runtime_error("The factory for type "
                            + getString(item_to_create._comp_source_type)
                            + " can not create a component with iid "
                            + item_to_create._comp_iid
                            + "within the plugin "
                            + item_to_create._comp_source_file_reference);
                    }
                }
                else
                {
                    throw std::runtime_error("There is no factory "
                        + getString(item_to_create._comp_source_type)
                        + " to create a component. Check your fep_components file : " + comp_file.getCurrentPath());
                }
            }
            return registry;
        }
    }
}
