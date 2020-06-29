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

#include "components_intf.h"
#include "component_iid.h"

namespace fep3
{
namespace arya
{
    /**
     * @brief default helper implementation for component
     *
     */
    class ComponentBaseImpl : public IComponent
    {
    protected:
        /**
         * @brief Construct a new Component Base object
         *
         */
        ComponentBaseImpl() = default;
        /**
         * @brief Destroy the Component Base object
         *
         */
        ~ComponentBaseImpl() override = default;

    public:
        /// @copydoc fep3::IComponent::createComponent
        fep3::Result createComponent(const std::weak_ptr<const arya::IComponents>& components) override
        {
            _components = components;
            return create();
        }
        /// @copydoc fep3::IComponent::destroyComponent
        fep3::Result destroyComponent() override
        {
            auto res = destroy();
            _components.reset();
            return res;
        }
        fep3::Result initialize() override
        {
            return Result();
        }
        fep3::Result tense() override
        {
            return Result();
        }
        fep3::Result relax() override
        {
            return Result();
        }
        Result deinitialize() override
        {
            return Result();
        }
        fep3::Result start() override
        {
            return Result();
        }
        fep3::Result stop() override
        {
            return Result();
        }
        fep3::Result pause() override
        {
            return Result();
        }
    protected:
        /**
         * @brief create the base component.
         * if this create method is called the _components pointer is valid already
         *
         * @return Result
         */
        virtual fep3::Result create()
        {
            return Result();
        }
        /**
         * @brief destroy the base component.
         * if this destroy method is called the _components pointer is still valid
         *
         * @return Result
         */
        virtual fep3::Result destroy()
        {
            return Result();
        }

        /**
         * @brief Weak pointer to the components
         * @note The component (this) must not take permanent ownership of the components
         */
        std::weak_ptr<const IComponents> _components;
    };
    /**
     * @brief default helper implementation for component
     * 
     */
    template<typename... component_interface_types>
    class ComponentBase : public ComponentBaseImpl,
                          public component_interface_types...
    {
        protected:
            /**
             * @brief Construct a new Component Base object
             * 
             */
            ComponentBase() : ComponentBaseImpl()
            {
            }

        protected:
            /// @copydoc fep3::arya::IComponent::getInterface
            void* getInterface(const std::string& iid) override
            {
                return InterfaceGetter<component_interface_types...>()(this, iid);
            }
            
        private:
            /**
             * Functor getting a pointer to an interface by IID
             */
            template<typename... interface_types>
            struct InterfaceGetter
            {
                // end of compile time recursion
                void* operator()(ComponentBase*, const std::string&)
                {
                    return {};
                }
            };
            /**
             * Specialization of above functor for more than zero interface types
             */
            template<typename interface_type, typename... remaining_interface_types>
            struct InterfaceGetter<interface_type, remaining_interface_types...>
            {
                void* operator()(ComponentBase* component, const std::string& iid)
                {
                    if(interface_type::FEP3_COMP_IID  == iid)
                    {
                        return static_cast<interface_type*>(component);
                    }
                    else
                    {
                        // compile time recursion: go on with remaining component access object types
                        return InterfaceGetter<remaining_interface_types...>()(component, iid);
                    }
                }
            };
    };
}

/**
* @brief extracting \ref fep3::arya::ComponentBase from version namespace 
*
*/
using arya::ComponentBase;
}
