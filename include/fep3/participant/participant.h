/**
 *
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
#include <functional>
#include <fep3/fep3_participant_types.h>
#include <fep3/fep3_errors.h>

#include "element_factory_intf.h"
#include "element_intf.h"
#include <fep3/components/base/component_registry.h>


namespace fep3
{
namespace arya
{

/**
 * @brief class declaring a Participant
 * 
 * The participant embeds an Element into the FEP context and will provide a access point to the service bus and simulation bus.
 */
class FEP3_PARTICIPANT_EXPORT Participant final
{
private:
    /**
     * CTOR
     * @param name The name of the participant
     * @param version_info The version information of the participant
     * @param system_name The name of the fep system this participant belongs to. Leave it empty for default or set it later.
     * @param component_registry the component registry which is managed by this participant
     * @param element_factory the factory which is able to create
     */
    Participant(const std::string& name,
                const std::string& version_info,
                const std::string& system_name,
                const std::shared_ptr<ComponentRegistry>& component_registry,
                const std::shared_ptr<const IElementFactory>& element_factory);

    /**
     * @brief friend class declaration
     * 
     */
    friend Participant FEP3_PARTICIPANT_EXPORT createParticipant(const std::string& name,
                                         const std::string& version_info,
                                         const std::string& system_name,
                                         const std::shared_ptr<const IElementFactory>& factory,
                                         const std::string& server_address_url);

public:
    /**
     * Deleted copy CTOR
    */
    Participant(const Participant&) = delete;
    /**
     * Deleted copy assignment operator
     *
     * @return Reference to this
    */
    Participant& operator=(const Participant&) = delete;
    /**
     * Move CTOR
     */
    Participant(Participant&&);
    /**
     * move assignment operator
     *
     * @return Reference to this
    */
    Participant& operator=(Participant&&);
    /**
     * DTOR
    */
    virtual ~Participant();

    // access to components
    /**
     * Gets a component from a participant
     *
     * @tparam T Type of the component to get
     * @return Result 'NO_ERROR' if succeded, error code otherwise
    */
    template <class T>
    T* getComponent() const
    {
        return _component_registry->getComponent<T>();
    }

    /**
     * Blocking call to execute this participant.
     *
     * @param start_up_callback optional parameter if the startup is done 
     * @return int value to return to stdout if wanted 
     * @retval 0 successful and peaceful shutdown of participant reached
     */
    int exec(const std::function<void()>& start_up_callback = nullptr);

    /**
     * Gets the name of the participant
     * @return The name of the participant
     */
    std::string getName() const;

    /**
     * Gets the system name of the participant
     * @return The system name of the participant
     */
    std::string getSystemName() const;

    /**
     * Gets the version information of the participant
     * @return The version information of the participant
     */
    std::string getVersionInfo() const;

private:
    class Impl;
    std::shared_ptr<Impl> _impl;
    std::shared_ptr<ComponentRegistry> _component_registry;
};

/**
 * @brief Creates a participant
 * @param name Name of the participant to be created
 * @param system_name Name of the system this participant belongs to
 * @param version_info Version information of the participant to be created
 * @param factory factory instance which is able to create the element while load command
 * @param server_address_url forced server URL for the participant where it is reachable
 * @return Participant The created participant
*/
Participant FEP3_PARTICIPANT_EXPORT createParticipant(const std::string& name,
    const std::string& version_info,
    const std::string& system_name,
    const std::shared_ptr<const IElementFactory>& factory,
    const std::string& server_address_url = std::string() );
/**
 * @brief Creates a participant
 * @tparam element_factory type of the fatory which is able to create the element
 * @param name Name of the participant to be created
 * @param system_name Name of the system this participant belongs to
 * @param version_info Version information of the participant to be created
 * @param server_address_url forced server URL for the participant where it is reachable
 * @return Participant The created participant
*/
template<typename element_factory>
Participant createParticipant(const std::string& name,
    const std::string& version_info,
    const std::string& system_name,
    const std::string& server_address_url = std::string())
{
    const std::shared_ptr<const IElementFactory> factory = std::make_shared<element_factory>();
    return createParticipant(name, version_info, system_name, factory, server_address_url);
}
} // namespace arya
using arya::Participant;
using arya::createParticipant;

} // namespace fep3
