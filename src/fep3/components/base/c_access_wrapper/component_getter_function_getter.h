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

#include <map>
#include <memory>

#include <fep3/components/base/c_access_wrapper/component_getter_function_getter_intf.h>

namespace fep3
{
namespace plugin
{
namespace c
{
namespace arya
{

template<typename... component_access_object_types>
class ComponentGetterFunctionGetter : public arya::IComponentGetterFunctionGetter
{
private:
    class IGetterFunctionGetter
    {
    public:
        virtual ~IGetterFunctionGetter() = default;
        virtual void* operator()() const = 0;
    };
    template<typename getter_function_type>
    class GetterFunctionGetter : public IGetterFunctionGetter
    {
    public:
        GetterFunctionGetter(const getter_function_type& getter_function)
            : _getter_function(getter_function)
        {}
        void* operator()() const override
        {
            return reinterpret_cast<void*>(_getter_function);
        }
    private:
        getter_function_type _getter_function;
    };
    
    template<typename getter_function_type>
    static std::unique_ptr<GetterFunctionGetter<getter_function_type>> makeGetterFunctionGetter
        (getter_function_type&& getter_function)
    {
        return std::make_unique<GetterFunctionGetter<getter_function_type>>
            (std::forward<getter_function_type>(getter_function));
    }
    
    using GetterFunctionGetters = std::map<std::string, std::unique_ptr<IGetterFunctionGetter>>;
    
    template<typename... access_object_types>
    struct Factory
    {
        GetterFunctionGetters operator()()
        {
            return {};
        }
    };
    // Specialization of above class for more than zero specific access object types
    template<typename access_object_type, typename... remaining_access_object_types>
    struct Factory<access_object_type, remaining_access_object_types...>
    {
        GetterFunctionGetters operator()()
        {
            // compile time recursion: go on with remaining access object types
            auto getter_function_getters = Factory<remaining_access_object_types...>()();
            getter_function_getters.emplace
                (access_object_type::getComponentIID()
                , makeGetterFunctionGetter(access_object_type::getGetterFunction())
                );
            return getter_function_getters;
        }
    };
public:
    ComponentGetterFunctionGetter()
        : _getter_function_getters(Factory<component_access_object_types...>()())
    {}
    void* operator()(const std::string& iid) const
    {
        const auto& iter_getter_function_getter = _getter_function_getters.find(iid);
        if(iter_getter_function_getter != _getter_function_getters.cend())
        {
            if(const auto& getter_function_getter = iter_getter_function_getter->second)
            {
                return getter_function_getter->operator()();
            }
        }
        return nullptr;
    }
private:
    GetterFunctionGetters _getter_function_getters;
};

} // namespace arya
} // namespace c
} // namespace plugin
} // namespace fep3