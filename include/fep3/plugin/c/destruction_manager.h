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
#include <deque>

#include "destructor_intf.h"

namespace fep3
{
namespace plugin
{
namespace c
{
namespace arya
{

/**
 * @brief Class managing the destruction of objects
 */
class DestructionManager
{
public:
    /**
     * Default CTOR
     */
    DestructionManager() = default;
    /**
     * Copy CTOR
     * @note Does not copy the queue of destructors of the object to copy from
     *       , because it is strictly bound to the latter
     */
    DestructionManager(const DestructionManager&)
    {}
    /**
     * Move CTOR
     * @note Does not move the queue of destructors of the object to copy from
     *       , because it is strictly bound to the latter
     */
    DestructionManager(DestructionManager&&)
    {}
    /**
     * Copy assignment
     * @note Does not copy the queue of destructors of the object to copy from
     *       , because it is strictly bound to the latter
     * @return Reference to this
     */
    DestructionManager& operator=(const DestructionManager&)
    {
        return *this;
    }
    /**
     * Move assignment
     * @note Does not copy the queue of destructors of the object to copy from
     *       , because it is strictly bound to the latter
     * @return Reference to this
     */
    DestructionManager& operator=(DestructionManager&&)
    {
        return *this;
    }
    /**
     * DTOR
     */
    virtual ~DestructionManager() = default;
    /**
     * Adds a destructor object to the destruction manager
     * @param destructor Unique pointer to the destruction object
     */
    virtual void addDestructor(std::unique_ptr<IDestructor> destructor) final
    {
        _destructors.push_back(std::move(destructor));
    }
    /**
     * Adds destructor objects to the destruction manager; this is an overload
     * @param destructors Queue of unique pointers to the destructor objects
     */
    virtual void addDestructors(std::deque<std::unique_ptr<IDestructor>> destructors) final
    {
        _destructors.insert
            (_destructors.end()
            , std::make_move_iterator(destructors.begin())
            , std::make_move_iterator(destructors.end())
            );
    }
private:
    std::deque<std::unique_ptr<IDestructor>> _destructors;
};

/**
 * Class deleting another object upon destruction
 * @tparam other_type The type of the other object
 */
template<typename other_type>
class OtherDestructor : public IDestructor
{
public:
    /**
     * CTOR Constructs an object that will delete the object
     * behind \p pointer_to_other upon destruction of this
     * @param pointer_to_other The pointer to the other object to
     *                         be deleted
     */
    OtherDestructor(other_type* pointer_to_other)
        : _pointer_to_other(pointer_to_other)
    {}
    /**
     * DTOR Deletes the object behind the pointer as passed to the constructor
     */
    ~OtherDestructor() override
    {
        delete _pointer_to_other;
    }
private:
    other_type* _pointer_to_other{nullptr};
};

} // namespace arya
using arya::DestructionManager;
using arya::OtherDestructor;
} // namespace c
} // namespace plugin
} // namespace fep3
