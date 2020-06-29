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

#include <functional>
#include <list>
#include <string>

#include <a_util/result/result_type.h>
#include <a_util/result/error_def.h>

#include "fep3/components/base/component_registry.h"
#include "fep3/components/base/component_intf.h"
#include "fep3/fep3_errors.h"

namespace fep3
{
namespace arya
{

    ComponentRegistry::ComponentRegistry()
    {
    }

    ComponentRegistry::~ComponentRegistry()
    {
        clear();
    }

    fep3::Result ComponentRegistry::registerComponent(const std::string& fep_iid, const std::shared_ptr<IComponent>& component)
    {
        auto supported_interface = component->getInterface(fep_iid);
        if (supported_interface == nullptr)
        {
            RETURN_ERROR_DESCRIPTION(fep3::ERR_INVALID_TYPE, "given component does not support the interface %s", fep_iid.c_str());
        }
        IComponent* component_found = findComponent(fep_iid);
        if (component_found == nullptr)
        {
            _components.emplace_back(fep_iid, component);
            return fep3::Result();
        }
        RETURN_ERROR_DESCRIPTION(fep3::ERR_INVALID_ARG, "component %s already exists", fep_iid.c_str());
    }

    IComponent* ComponentRegistry::findComponent(const std::string& fep_iid) const
    {
        for (const auto& comp : _components)
        {
            if (comp.first == fep_iid)
            {
                return comp.second.get();
            }
        }
        return nullptr;
    }
    
    std::shared_ptr<IComponent> ComponentRegistry::findComponentByPtr(IComponent* component) const
    {
        for (const auto& comp : _components)
        {
            if (comp.second.get() == component)
            {
                return comp.second;
            }
        }
        return std::shared_ptr<IComponent>();
    }

    fep3::Result ComponentRegistry::unregisterComponent(const std::string& fep_iid)
    {
        for (decltype(_components)::iterator comp_iterator = _components.begin();
            comp_iterator != _components.end();
            comp_iterator++)
        {
            if (std::get<0>(*comp_iterator) == fep_iid)
            {
                _components.erase(comp_iterator);
                return fep3::Result();
            }
        }
        RETURN_ERROR_DESCRIPTION(fep3::ERR_INVALID_ARG, "component %s does not exist", fep_iid.c_str());
    }

    fep3::Result raiseWithFallback(std::vector<std::pair<std::string, std::shared_ptr<IComponent>>>& components,
        std::function<fep3::Result(IComponent&)> raise_func,
        std::function<fep3::Result(IComponent&)> fallback_func,
        const std::string& func_call_name)
    {
        std::list<IComponent*> succeeded_list;
        fep3::Result res;
        for (auto& current_comp : components)
        {
            //we need to catch here becase that might be user code in the plugins
            try
            {
                res = raise_func(*current_comp.second.get());
            }
            catch (const std::exception& ex)
            {
                res = fep3::Result(ERR_UNEXPECTED,
                    std::string("Exception occured while " + func_call_name + ": " + ex.what()).c_str(),
                    __LINE__, __FILE__, std::string("ComponentRegistry::" + func_call_name).c_str());
            }
            if (fep3::isOk(res))
            {
                //remember the components where raise_function succeded
                succeeded_list.push_back(current_comp.second.get());
            }
            else
            {
                //on error fallback to the previous "state" of the component (reverse remember list)
                for (decltype(succeeded_list)::reverse_iterator comp_fallback = succeeded_list.rbegin();
                    comp_fallback != succeeded_list.rend();
                    comp_fallback++)
                {
                    try
                    {
                        fallback_func(**comp_fallback);
                    }
                    catch (const std::exception& ex)
                    {
                        res |= fep3::Result(ERR_UNEXPECTED,
                            std::string("Exception occured while " + func_call_name + ": " + ex.what()).c_str(),
                            __LINE__, __FILE__, std::string("ComponentRegistry::" + func_call_name).c_str());
                    }
                }
                return res;
            }
        }
        return fep3::Result();
    }

    fep3::Result ComponentRegistry::create()
    {
        //create or fallback if one of it failed
        return raiseWithFallback(_components,
            [&](IComponent& comp)-> fep3::Result
        {
            return comp.createComponent(static_cast<std::weak_ptr<const IComponents>>
                (std::static_pointer_cast<const IComponents>(shared_from_this())
                ));
        },
            [&](IComponent& comp)-> fep3::Result
        {
            return comp.destroyComponent();
        }, "create");
    }

    fep3::Result ComponentRegistry::destroy()
    {
        fep3::Result res;
        for (decltype(_components)::reverse_iterator current_comp_it = _components.rbegin();
            current_comp_it != _components.rend();
            current_comp_it++)
        {
            try
            {
                res |= current_comp_it->second->destroyComponent();
            }
            catch (const std::exception& ex)
            {
                res |= fep3::Result(ERR_UNEXPECTED,
                    std::string(std::string("Exception occured while destroy: ") + ex.what()).c_str(),
                    __LINE__, __FILE__, "ComponentRegistry::destroy");
            }
        }
        return res;
    }

    fep3::Result ComponentRegistry::initialize()
    {
        //initializing or fallback if one of it failed
        return raiseWithFallback(_components,
            [&](IComponent& comp)-> fep3::Result
        {
            return comp.initialize();
        },
            [&](IComponent& comp)-> fep3::Result
        {
            return comp.deinitialize();
        }, "initialize");
    }

    fep3::Result ComponentRegistry::tense()
    {
        //getready or fallback if one of it failed
        return raiseWithFallback(_components,
            [&](IComponent& comp)-> fep3::Result
        {
            return comp.tense();
        },
            [&](IComponent& comp)-> fep3::Result
        {
            return comp.relax();
        }, "tense");
    }

    fep3::Result ComponentRegistry::relax()
    {
        fep3::Result res;
        for (decltype(_components)::reverse_iterator current_comp_it = _components.rbegin();
            current_comp_it != _components.rend();
            current_comp_it++)
        {
            //we need to catch here becase that might be user code in the plugins
            try
            {
                res |= current_comp_it->second->relax();
            }
            catch (const std::exception& ex)
            {
                res |= fep3::Result(ERR_UNEXPECTED, 
                    std::string(std::string("Exception occured while relax: ") + ex.what()).c_str(),
                    __LINE__, __FILE__, "ComponentRegistry::relax");
            }
        }
        return res;
    }

    fep3::Result ComponentRegistry::deinitialize()
    {
        fep3::Result res;
        for (decltype(_components)::reverse_iterator current_comp_it = _components.rbegin();
            current_comp_it != _components.rend();
            current_comp_it++)
        {
            //we need to catch here becase that might be user code in the plugins
            try
            {
                res |= current_comp_it->second->deinitialize();
            }
            catch (const std::exception& ex)
            {
                res |= fep3::Result(ERR_UNEXPECTED,
                    std::string(std::string("Exception occured while deinitialize: ") + ex.what()).c_str(),
                    __LINE__, __FILE__, "ComponentRegistry::deinitialize");
            }
        }
        return res;
    }

    fep3::Result ComponentRegistry::start()
    {
        //start or fallback if one of it failed
        return raiseWithFallback(_components,
            [&](IComponent& comp)-> fep3::Result
        {
            return comp.start();
        },
            [&](IComponent& comp)-> fep3::Result
        {
            return comp.stop();
        }, "start");
    }

    fep3::Result ComponentRegistry::stop()
    {
        //stop in reverse order
        fep3::Result res;
        for (decltype(_components)::reverse_iterator current_comp_it = _components.rbegin();
            current_comp_it != _components.rend();
            current_comp_it++)
        {
            //we need to catch here becase that might be user code in the plugins
            try
            {
                res |= current_comp_it->second->stop();
            }
            catch (const std::exception& ex)
            {
                res |= fep3::Result(ERR_UNEXPECTED, 
                    std::string(std::string("Exception occured while stop: ") + ex.what()).c_str(),
                    __LINE__, __FILE__, "ComponentRegistry::stop");
            }
        }
        return res;
    }

    fep3::Result ComponentRegistry::pause()
    {
        //pause or fallback if one of it failed
        return raiseWithFallback(_components,
            [&](IComponent& comp)-> fep3::Result
        {
            return comp.pause();
        },
            [&](IComponent& comp)-> fep3::Result
        {
            return comp.stop();
        }, "pause");
    }

    void ComponentRegistry::clear()
    {
        //destroy in reverse order
        decltype(_components)::iterator current_comp_it = _components.end();
        while (!_components.empty())
        {
            _components.erase(--current_comp_it);
            current_comp_it = _components.end();
        }
    }
}
}
