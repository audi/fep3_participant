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

#include <deque>
#include <memory>
#include <string>
#include <tuple>

#include <fep3/components/base/c_intf/component_c_intf.h>
#include <fep3/components/base/component_intf.h>

namespace fep3
{
namespace plugin
{
namespace c
{
namespace arya
{

/**
 * Interface for getting a component from within a plugin
 */
class IComponentGetter
{
public:
    /**
     * DTOR
     */
    virtual ~IComponentGetter() = default;
    /**
     * @brief Parenthesis operator getting a transferable component as identified by @p handle_to_component
     * @param component_getter_function The function to be used to get a component
     * @param iid The interface ID of the component to get
     * @param handle_to_component Handle to the component to get
     * @return Pointer to the component, nullptr if the component cannot be get
     */
    virtual fep3::arya::IComponent* operator()
        (void* component_getter_function
        , const std::string& iid
        , const fep3_arya_HIComponent& handle_to_component
        ) = 0;
};

/**
 * Functor for getting a component from within a plugin
 */
template<typename... component_access_types>
class ComponentGetter : public IComponentGetter
{
private:
    template<typename component_access_type>
    class Getter
    {
    public:
        inline Getter()
            : get_function_()
        {}
        
        inline ::fep3::arya::IComponent* operator()
            (void* component_getter_function
            , const std::string& iid
            , const fep3_arya_HIComponent& handle_to_component
            )
        {
            if(iid == component_access_type::getComponentIID())
            {
                typename component_access_type::Access access_result;
                if(nullptr != component_getter_function)
                {
                    const auto& specific_component_getter_function 
                        = reinterpret_cast<fep3_plugin_c_InterfaceError(*)(typename component_access_type::Access* access, const char*, fep3_arya_HIComponent)>
                        (component_getter_function);
                    specific_component_getter_function(&access_result, iid.c_str(), handle_to_component);
                    if(!_exposed_component)
                    {
                        _exposed_component.reset(new component_access_type
                            (access_result
                            , {} // the getter returns a plain pointer so we cannot provide shared binary management
                            ));
                    }
                    return _exposed_component.get();
                }
            }
            return nullptr;
        }
    private:
        fep3_plugin_c_InterfaceError (*get_function_)
            (typename component_access_type::Access* access
            , const char* iid
            , fep3_arya_HIComponent handle_to_component
            ){nullptr};
        // note: as the getter exposes a plain pointer to the component, we cannot 
        // predict how long it will be in use, thus we need to keep the object a pointer pointing to 
        // was exposed forever (i. e. until the getter is destroyed).
        std::unique_ptr<component_access_type> _exposed_component;
    };

public:
    /**
     * Default CTOR
     */
    inline ComponentGetter()
        : _getters(Getter<component_access_types>()...)
    {}

    /// @copydoc ::fep3::plugin::c::arya::IComponentGetter::operator()(void*, const std::string&, const fep3_arya_HIComponent&)
    inline ::fep3::arya::IComponent* operator()
        (void* component_getter_function
        , const std::string& iid
        , const fep3_arya_HIComponent& handle_to_component
        )
    {
        return GetterCaller<std::tuple_size<decltype(_getters)>::value, false>()
            (_getters, component_getter_function, iid, handle_to_component);
    }

private:
    // dummy template parameter needed for template specialization in headerk
    template<int remaining_number_of_entries, bool dummy>
    class GetterCaller
    {
    public:
        template<typename getters_tuple_type>
        ::fep3::arya::IComponent* operator()
            (getters_tuple_type& getters_tuple
            , void* component_getter_function
            , const std::string& iid
            , const fep3_arya_HIComponent& handle_to_component
            ) const
        {
            auto& getter = std::get<remaining_number_of_entries - 1>(getters_tuple);
            if(const auto& component = getter(component_getter_function, iid, handle_to_component))
            {
                return component;
            }
            // compile time recursion: go on with next tuple index
            return GetterCaller<remaining_number_of_entries - 1, dummy>()(getters_tuple, component_getter_function, iid, handle_to_component);
        }
    };
    // end of compile time recursion
    template<bool dummy>
    class GetterCaller<0, dummy>
    {
    public:
        template<typename getters_tuple_type>
        ::fep3::arya::IComponent* operator()
        (getters_tuple_type&
            , void*
            , const std::string&
            , const fep3_arya_HIComponent&
            ) const
        {
            return nullptr;
        }
    };

private:
    std::tuple<Getter<component_access_types>...> _getters;
};

/**
 * Makes a component getter
 *
 * @tparam component_access_types Types of the component access classes to make the component getter for
 * @return Shared pointer to the created component getter
 */
template<typename... component_access_types>
static std::shared_ptr<ComponentGetter<component_access_types...>> makeComponentGetter()
{
    return std::make_shared<ComponentGetter<component_access_types...>>();
}

} // namespace arya

} // namespace c
} // namespace plugin
} // namespace fep3
