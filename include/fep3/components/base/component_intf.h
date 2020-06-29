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
#include <fep3/fep3_participant_types.h>
#include <fep3/fep3_errors.h>

namespace fep3
{
namespace arya
{

    /**
     * @brief Get the Component Interface ID for the given interface type T
     * The interface type T must define the interface by the helper macro (@ref FEP_COMPONENT_IID)
     *
     * @tparam T The interface type
     * @return std::string
     */
    template <class T>
    std::string getComponentIID()
    {
        return T::getComponentIID();
    }

    // forward decl
    class IComponents;

    /**
        * @brief base interface of a component as part of a @ref IComponent registry
        *
        */
    class FEP3_PARTICIPANT_EXPORT IComponent
    {
    public:
        /// DTOR
        virtual ~IComponent() = default;
        /**
         * @brief Create a Component object
         *
         * @param components Weak pointer to the components
         * @return fep3::Result
         */
        virtual fep3::Result createComponent(const std::weak_ptr<const IComponents>& components) = 0;
        /**
            * @brief will be called to destroy the component (this will NOT result in a destruction, it is the step before)
            *
            * @return fep3::Result
            */
        virtual fep3::Result destroyComponent() = 0;
        /**
            * @brief Initializing a component
            *
            * @return fep3::Result
            */
        virtual fep3::Result initialize() = 0;
        /**
         * @brief Get Ready for running state
         * make everything ready for the running state, start will be called afterwards
         *
         * @return fep3::Result
         */
        virtual fep3::Result tense() = 0;
        /**
         * @brief fall back into simpla intialized state
         * relax is the antonym of tense
         *
         * @return fep3::Result
         */
        virtual fep3::Result relax() = 0;
        /**
            * @brief start the component
            *
            * @return fep3::Result
            */
        virtual fep3::Result start() = 0;
        /**
            * @brief stops the component
            *
            * @return fep3::Result
            */
        virtual fep3::Result stop() = 0;
        /**
            * @brief stops the component
            *
            * @return fep3::Result
            */
        virtual fep3::Result pause() = 0;
        /**
            * @brief deinitialize the component
            *
            * @return fep3::Result
            */
        virtual fep3::Result deinitialize() = 0;

        /**
            * @brief Get the interface requested by the \p iid and return a void pointer to it
            *
            * @param iid
            * @return Pointer to the interface of the this component if this component has the passed @p idd
            *         , nullptr otherwise
            */
        virtual void* getInterface(const std::string& iid) = 0;
    };

} //arya

using arya::IComponent;

/**
 * @brief extracting @ref fep3::arya::getComponentIID() from version namespace
 * @tparam interface_type the component interface type
 * @return the component interface id of the given @p interface_type
 */
using arya::getComponentIID;

} // namespace fep3