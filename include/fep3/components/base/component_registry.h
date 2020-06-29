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
#include <string>
#include <utility>
#include <vector>

#include <fep3/fep3_result_decl.h>
#include <a_util/result/error_def.h>
#include "components_intf.h"

namespace fep3
{
namespace arya
{
    /**
        * @brief default component registry implementation
        *
        * @note Objects of this class must be held in an std::shared_ptr, because this class is derived from std::enable_shared_from_this
        */
    class FEP3_PARTICIPANT_EXPORT ComponentRegistry 
        : public IComponents,
          public std::enable_shared_from_this<ComponentRegistry>
    {
    public:
        /**
            * @brief Construct a new Component Registry object
            *
            */
        ComponentRegistry();
        /**
            * @brief Destroy the Component Registry object
            *
            */
        ~ComponentRegistry() override;
        /**
          * @brief would construct a new Component Registry object, but is deleted
          *
          */
        ComponentRegistry(const ComponentRegistry&) = delete;
        /**
          * @brief would assign the Component Registry object, but is deleted
          * @return this ComponentRegistry
          */
        ComponentRegistry& operator=(const ComponentRegistry&) = delete;

        /**
         * @brief Construct a new Component Registry by moving from another
         *
         */
        ComponentRegistry(ComponentRegistry&&) = default;
        /**
          * @brief move the Component Registry object
          * @return this ComponentRegistry
          */
        ComponentRegistry& operator=( ComponentRegistry&&) = default;

        /**
         * @brief This will register a instance of a component to the registry.
         * @pre @p component_type must implement access to the interface id (see @ref FEP3_COMPONENT_IID)
         * @remark The ownership of the IComponent pointer goes to the registry.
         *         Only one component of the interface id is possible.
         *
         * @tparam component_type The component class which must be provide the component interface id.
         * @param component Shared pointer to the component to be registered
         * @retval fep3::ERR_INVALID_TYPE given component does not support the interface
         * @return fep3::Result
         */
        template<class component_type>
        fep3::Result registerComponent(const std::shared_ptr<IComponent>& component)
        {
            const std::string fep_iid = getComponentIID<component_type>();
            return registerComponent(fep_iid, component);
        }
        /**
         * @brief unregister the component with the interface id of T
         * @pre @p component_type must implement access to the interface id (see @ref FEP3_COMPONENT_IID)
         *
         * @tparam component_type The component interface to unregister.
         * @return fep3::Result
         */
        template<class component_type>
        fep3::Result unregisterComponent()
        {
            const std::string fep_iid = getComponentIID<component_type>();
            return unregisterComponent(fep_iid);
        }

        /**
            * @brief call the IComponent::create method of the registered components
            *
            * @return fep3::Result
            */
        fep3::Result create();
        /**
            * @brief call the IComponent::destroy method of the registered components
            *
            * @return fep3::Result
            */
        fep3::Result destroy();

        /**
            * @brief call the IComponent::initializing method of the registered components
            *
            * @return fep3::Result
            */
        fep3::Result initialize();
        /**
            * @brief call the IComponent::deinitializing method of the registered components
            *
            * @return fep3::Result
            */
        fep3::Result deinitialize();

        /**
         * @brief call the IComponent::tense method of the registered components
         *
         * @return fep3::Result
         */
        fep3::Result tense();

        /**
         * @brief call the IComponent::relax method of the registered components
         *
         * @return fep3::Result
         */
        fep3::Result relax();

        /**
            * @brief call the IComponent::start method of the registered components
            *
            * @return fep3::Result
            */
        fep3::Result start();
        /**
            * @brief call the IComponent::stop method of the registered components
            *
            * @return fep3::Result
            */
        fep3::Result stop();
        
        /**
        * @brief call the IComponent::pause method of the registered components
        *
        * @return fep3::Result
        */
        fep3::Result pause();
        /**
        * @brief call the IComponent::contains method of the registered components
        *
        * @return bool
        */
        template<class component_type>
        bool contains() const
        {
            const std::string fep_iid = getComponentIID<component_type>();
            if (findComponent(fep_iid) == nullptr)
            {
                return false;
            }
            return true;
        }

        /**
            * @brief empties the list of components and call there DTOR
            *
            */
        void clear();
        /**
          * @brief Registers an instance of a component to the registry.
          * The given component must implement an access to an interface identifier \p fep_iid (see @ref FEP3_COMPONENT_IID)
          * @remark The component registry takes shared ownership of the \p component if and only if registration is successful.
          *         Only one component of the interface \p fep_iid can be registered.
          *
          * see access function  @ref registerComponent<T>
          *
          * @param fep_iid the component interface identifier of the given \p component
          * @param component Shared pointer to the component to be registered
          * @return fep3::Result
          */
        fep3::Result registerComponent(const std::string& fep_iid,
                                       const std::shared_ptr<IComponent>& component);
        /**
          * @brief unregister the component with the interface id of (see @ref FEP3_COMPONENT_IID)
          *
          * see access function  @ref unregisterComponent<T>
          *
          * @param fep_iid the component interface identifier to unregister.
          * @return fep3::Result
          */
        fep3::Result unregisterComponent(const std::string& fep_iid);

    private:
        IComponent* findComponent(const std::string& fep_iid) const override;

        /**
        * @brief searches for a component in the component registry by raw pointer
        *
        * @param component component to be looked for
        * @return shared_ptr to the component or empty shared_ptr (if not found)
        */
        std::shared_ptr<IComponent> findComponentByPtr(IComponent* component) const;
        
        /// the components container
        std::vector<std::pair<std::string, std::shared_ptr<IComponent>>> _components;
    };
}
using arya::ComponentRegistry;
}

