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

#include "propertynode_intf.h"
#include "propertynode_helper.h"
#include <fep3/fep3_errors.h>

#include <map>
#include <string>
#include <tuple>
#include <vector>
#include <algorithm>
#include <iterator>
#include <shared_mutex>

namespace fep3
{
namespace arya
{

/**
 * @brief Interface for receiving notifications about changes of a property node.
 */
class IPropertyObserver
{
public:
    /**
     * @brief Method to be called whenever a property is updated.
     * 
     * @param updated_property PropertyNode that was updated. 
     *                          It contains the value after the update.
     */
    virtual void onUpdate(IPropertyNode& updated_property) = 0;
};

/**
 * @brief Interface for registration of a @ref fep3::arya::IPropertyObserver. 
 * 
 */
class IPropertyObservable
{
public:
    /**
     * @brief Register the @p observer.
     * 
     * @param observer Observer to register.
     *                  After registration the @p observer will be called whenever 
     *                  the property it registered for is changed.
     */
    virtual void registerObserver(std::weak_ptr<IPropertyObserver> observer) = 0;

    /**
     * @brief Unregister the @p observer
     * 
     * @param observer Observer to unregister.
     */
    virtual void unregisterObserver(std::weak_ptr<IPropertyObserver> observer) = 0;
};

/**
 * @brief Extends @ref fep3::arya::IPropertyNode with additional functionality.
 * 
 */
class IPropertyWithExtendedAccess  
    : public IPropertyNode
    , public IPropertyObservable
{
public:
    /**
     * @brief Sets the child node @p property_to_add. 
     * If a child with the same name as @p property_to_add already exists, it will be overriden.
     * 
     * @param property_to_add Property to be set as child
     * @return The child that was just set
     */
    virtual std::shared_ptr<IPropertyWithExtendedAccess> setChild(std::shared_ptr<IPropertyWithExtendedAccess> property_to_add) = 0;

    /**
     * @brief Get the child with this @p name.
     * In contrary to @ref fep3::arya::IPropertyNode::getChild() this method will return 
     * the interface @ref fep3::arya::IPropertyWithExtendedAccess(), which allows some more modification to the child.
     * 
     * @param name Name of the child
     * @return Child as shared_ptr or 
     * @retval Default constructed shared_ptr, if child was not found. 
     */
    virtual std::shared_ptr<IPropertyWithExtendedAccess> getChildImpl(const std::string& name) = 0;

    /**
     * @brief Update all observers that registered to this property node.
     * Due to synchronization, observers will only be updated if this method is called.
     */
    virtual void updateObservers() = 0;

    /**
     * @brief Deep copies the content of @p other node to this node.
     * The children of @p other are also copied.
     * Observers won't be copied.
     * 
     * @param other Node to copy from
     */
    virtual void copyDeepFrom(const IPropertyNode& other) = 0;

    /**
     * @brief Register a observer to this node.
     * All observers are notified when the method @ref fep3::arya::IPropertyWithExtendedAccess::updateObservers() is called.
     * 
     * @param observer Observer to register
     */
    void registerObserver(std::weak_ptr<IPropertyObserver> observer) override = 0;

     /**
     * @brief Unregister a observer from this node.
     * 
     * @param observer Observer to unregister
     */
    void unregisterObserver(std::weak_ptr<IPropertyObserver> observer) override = 0;

     /**
     * @brief Remove the child with @p name.
     * 
     * @param name Name of the child to remove
     */
    virtual void removeChild(const std::string& name) = 0;   
};


/**
 * @brief PropertyValue that can be registered as an @ref fep3::arya::IPropertyObserver
 * 
 * @tparam T Type of the PropertyValue
 */
template <typename T>
class PropertyValueWithObserver
    : public PropertyValue<T>
    , public IPropertyObserver
{
public:
    /**
     * @brief Forwarder that is used to forward calls to a reference of type @ref fep3::arya::IPropertyObserver
     * Is used if a observer can not be instanciated as shared_ptr directly
     */
    class ObserverForwarder : public IPropertyObserver
    {
    public:
        /**
         * @brief CTOR
         * 
         * @param forward_to Reference to forward calls to
         */
        ObserverForwarder(IPropertyObserver& forward_to)
            : _forward_to(forward_to)
        {

        }

        //! @copydoc fep3::arya::IPropertyObserver::onUpdate() 
        void onUpdate(IPropertyNode& updated_property) override
        {
            _forward_to.onUpdate(updated_property);
        }

    private:
        IPropertyObserver& _forward_to; /// reference to forward calls to
    };

    /**
     * @brief Default CTOR
     *
     */
    PropertyValueWithObserver()
        : PropertyValue<T>()
    {
        _forwarder = std::make_shared<ObserverForwarder>(*this);
    }
    
    /**
     * @brief CTOR
     * 
     * @param value The initial value of the property
     */
    PropertyValueWithObserver(T value)
        : PropertyValue<T>(std::move(value))
    {
        _forwarder = std::make_shared<ObserverForwarder>(*this);
    }        

    /**
     * @brief Update the PropertyValue with @p updated
     * 
     * @param updated 
     */
    void onUpdate(IPropertyNode& updated) override
    {        
        PropertyValue<T>::setValue(DefaultPropertyTypeConversion<T>::fromString(updated.getValue()));
    }

    /**
     * @brief Get the observer of the PropertyValue
     * 
     * @return std::shared_ptr<IPropertyObserver> 
     */
    std::shared_ptr<IPropertyObserver> getObserver()
    {
        return _forwarder;
    }  

    private:
        /// Observer for this property variable
        std::shared_ptr<ObserverForwarder> _forwarder; 
};

/**
 * @brief PropertyVariable as property observer
 * 
 * @tparam T Type of property variable
 */
template <typename T>
using PropertyVariable = PropertyValueWithObserver<T>;


/**
 * @brief Implementation class to represent a property tree node
 * T can either be @ref fep3::arya::IPropertyNode or @ref fep3::arya::IPropertyWithExtendedAccess
 */
template <typename T>
class PropertyNode 
    : public T
{
public:
    /**
     * @brief CTOR
     * 
     * @param name Name of the property node
     * @param value Initial value of the property node
     * @param type Type of the property node
     *                  * See @ref fep3::arya::PropertyType<T>::getTypeName for default types.
     */
    PropertyNode(const std::string& name, const std::string& value, const std::string& type)
        : _name(name)       
        , _value(value)
        , _type(type)
    {
        validatePropertyName(_name);           
    }    

    /**
     * @brief CTOR for a property Node of type @ref fep3::arya::NodePropertyType which represents a node without value.
     * 
     * 
     * @param name Name of the property node
     */
    PropertyNode(std::string  name)
        : _name(std::move(name))        
        , _value("")
        , _type(PropertyType<NodePropertyType>::getTypeName())
    {
        validatePropertyName(_name);
    }   

    /**
     * @brief Copy CTOR
     * 
     * @param other Node to copy from
     */
    PropertyNode(const PropertyNode& other)
    {
        std::shared_lock<std::shared_timed_mutex> children_lock(other._mutex_children);
        std::shared_lock<std::shared_timed_mutex> observer_lock(other._mutex_observer);
        std::shared_lock<std::shared_timed_mutex> member_lock(other._mutex_strings);

        _children = other._children;
        _observers = other._observers;
        _value = other._value;
        _type = other._type;
        _name = other._name;
    }

    /**
     * @brief Copy assignment operator
     * 
     * @param other Node to copy from
     * @return PropertyNode& 
     */
    PropertyNode& operator=(const PropertyNode& other)
    {
        std::shared_lock<std::shared_timed_mutex> other_children_lock(other._mutex_children);
        std::shared_lock<std::shared_timed_mutex> other_observer_lock(other._mutex_observer);
        std::shared_lock<std::shared_timed_mutex> other_member_lock(other._mutex_strings);

        std::unique_lock<std::shared_timed_mutex> children_lock(_mutex_children);
        std::unique_lock<std::shared_timed_mutex> observer_lock(_mutex_observer);
        std::unique_lock<std::shared_timed_mutex> member_lock(_mutex_strings);

        _children = other._children;
        _observers = other._observers;
        _value = other._value;
        _type = other._type;
        _name = other._name;
    }

     //! @copydoc fep3::arya::IPropertyWithExtendedAccess::copyDeepFrom()
    void copyDeepFrom(const IPropertyNode& other)
    {                
        {
            std::unique_lock<std::shared_timed_mutex> member_lock(_mutex_strings);
            _name = other.getName();
            _type = other.getTypeName();
            _value = other.getValue();
        }

        std::unique_lock<std::shared_timed_mutex> children_lock(_mutex_children);
        _children.clear();

        const auto other_childs = other.getChildren();
        for (const auto& other_child : other_childs)
        {
            auto new_child = std::make_shared<PropertyNode>("some_temp_name");
            new_child->copyDeepFrom(*other_child);

            _children.push_back(new_child);
        }
    }

    //! @copydoc fep3::arya::IPropertyNode::getValue()
    std::string getValue() const override
    {       
        std::shared_lock<std::shared_timed_mutex> lock(_mutex_strings);
        return _value;
    }

    //! @copydoc fep3::arya::IPropertyNode::getName()
    std::string getName() const override
    {
        std::shared_lock<std::shared_timed_mutex> lock(_mutex_strings);
        return _name;
    }

    //! @copydoc fep3::arya::IPropertyNode::getTypeName()
    std::string getTypeName() const override
    {
        std::shared_lock<std::shared_timed_mutex> lock(_mutex_strings);
        return _type;
    }

     //! @copydoc fep3::arya::IPropertyNode::setValue()
    fep3::Result setValue(const std::string& value, const std::string type_name = "") override
    {        
        std::unique_lock<std::shared_timed_mutex> lock(_mutex_strings);
        if (!type_name.empty()
            && type_name != _type)
        {
            RETURN_ERROR_DESCRIPTION(ERR_INVALID_TYPE
                , "Type of node and provided type are not matching. Node type = %s; Provided type = %s"
                , _type.c_str()
                , type_name.c_str());
        }

        _value = value;           

        return {};
    }

    //! @copydoc fep3::arya::IPropertyWithExtendedAccess::updateObservers()
    void updateObservers()
    {        
        std::shared_lock<std::shared_timed_mutex> lock(_mutex_observer);
        for (const auto& observer : _observers)
        {
            auto shared = observer.lock();
            if (shared)
            {
                shared->onUpdate(*this);
            }
        }          

        std::vector<std::shared_ptr<T>> children;
        {
            std::shared_lock<std::shared_timed_mutex> children_lock(_mutex_children);
            children = _children;
        }        
       
        for (const auto& child : children)
        {
            child->updateObservers();
        }
    }
    
    //! @copydoc fep3::arya::IPropertyNode::isEqual()
    bool isEqual(const IPropertyNode& other) const override
    {      
       {
            std::shared_lock<std::shared_timed_mutex> children_lock(_mutex_children);
            std::shared_lock<std::shared_timed_mutex> member_lock(_mutex_strings);

            const auto equal_values = 
                _name == other.getName()
                && _value == other.getValue()
                && _type == other.getTypeName();

            const auto equal_child_size = other.getChildren().size() == _children.size();

            if (!equal_values || !equal_child_size)
            {
                return false;
            }      
        }

        const auto other_childs = other.getChildren();        
        for (const auto& other_child : other_childs)
        {
            const auto this_child = getChild(other_child->getName());
            if (!this_child)
            {
                return false;
            }        
            
            if (!(this_child->isEqual(*other_child)))
            {
                return false;
            }            
        }
        return true;
    }

    //! @copydoc fep3::arya::IPropertyNode::reset()
    void reset() override
    {
        throw std::runtime_error("not yet implemented");
    }
 
    //! @copydoc fep3::arya::IPropertyWithExtendedAccess::getChildren()
    std::vector<std::shared_ptr<IPropertyNode>> getChildren() const override
    {       
        std::shared_lock<std::shared_timed_mutex> lock(_mutex_children);

        std::vector<std::shared_ptr<IPropertyNode>> props;
        std::transform(_children.begin(), _children.end(), std::back_inserter(props), [](auto& iter) {
            return iter;
        });

        return props;
    }

    //! @copydoc fep3::arya::IPropertyNode::getChild()
    std::shared_ptr<IPropertyNode> getChild(const std::string& name) const override
    {
        std::shared_lock<std::shared_timed_mutex> lock(_mutex_children);

        auto find_result = std::find_if(_children.begin(), _children.end(), [&name](const std::shared_ptr<T>& iter) {
            return iter->getName() == name;
        });

        if (find_result != _children.end())
        {
            return *find_result;
        }

        return {};
    }       

    //! @copydoc fep3::arya::IPropertyWithExtendedAccess::registerObserver()
    void registerObserver(std::weak_ptr<IPropertyObserver> observer) 
    {
        std::unique_lock<std::shared_timed_mutex> lock(_mutex_observer);

        const auto find_iter = std::find_if(_observers.begin(), _observers.end(), [&observer](auto& iter) {
            return (iter.lock() == observer.lock()) && observer.lock() != nullptr;
        });

        if (find_iter == _observers.end())
        {
            _observers.push_back(std::move(observer));
        }
    }

    //! @copydoc fep3::arya::IPropertyWithExtendedAccess::unregisterObserver()
    void unregisterObserver(std::weak_ptr<IPropertyObserver> observer) 
    {
        std::unique_lock<std::shared_timed_mutex> lock(_mutex_observer);

        const auto find_iter = std::find_if(_observers.begin(), _observers.end(), [&observer](auto& iter) {
            return (iter.lock() == observer.lock()) && observer.lock() != nullptr;
        });

        if (find_iter != _observers.end())
        {
            _observers.erase(find_iter);
        }
    }       

    //! @copydoc fep3::arya::IPropertyNode::getNumberOfChildren()
    size_t getNumberOfChildren() const override
    {
        std::shared_lock<std::shared_timed_mutex> lock(_mutex_children);

        return _children.size();
    }

    //! @copydoc fep3::arya::IPropertyNode::isChild()
    bool isChild(const std::string& name) const override
    {
        return getChild(name) != nullptr;
    }

    //! @copydoc fep3::arya::IPropertyWithExtendedAccess::removeChild()
    void removeChild(const std::string& name)
    {
        std::unique_lock<std::shared_timed_mutex> lock(_mutex_children);

        auto find_result = std::find_if(_children.begin(), _children.end(), [&name](const std::shared_ptr<T>& iter) {
            return iter->getName() == name;
        });

        if (find_result != _children.end())
        {
            _children.erase(find_result);
        }
    }

    //! @copydoc fep3::arya::IPropertyWithExtendedAccess::getChildImpl()
    virtual std::shared_ptr<T> getChildImpl(const std::string& name)
    {
        std::shared_lock<std::shared_timed_mutex> lock(_mutex_children);

        auto find_result = std::find_if(_children.begin(), _children.end(), [&name](const std::shared_ptr<T>& iter) {
            return iter->getName() == name;
        });

        if (find_result != _children.end())
        {
            return *find_result;
        }

        return {};
    }

    //! @copydoc fep3::arya::IPropertyWithExtendedAccess::setChild()
    std::shared_ptr<T> setChild(std::shared_ptr<T> property_to_add)
    {
        std::unique_lock<std::shared_timed_mutex> lock(_mutex_children);

        auto find_result = std::find_if(_children.begin(), _children.end(), [&property_to_add](const std::shared_ptr<T>& iter) {
            return iter->getName() == property_to_add->getName();
        });

        if (find_result != _children.end())
        {
            _children.erase(find_result);
        }

        _children.push_back(std::move(property_to_add));
        return _children.back();
    }

    /**
     * @brief Register a property variable for this property node.
     * The property variable allows typed read access to this property.
     * It is updated on every call of @ref fep3::arya::IPropertyWithExtendedAccess::updateObservers().
     * Changing the property variable does not lead to a change to the actual property,
     * except for the time of registration, when the property will be set to the value of @p property_variable.
	 *
     * @tparam variable_type Type of the property_variable
     * @param property_variable The property variable to register as observer for the property
     * @param name The name of the property to register to.
     *              If empty the property variable will register to this node, otherwise to a child with this @p name.
     *              If a child with @p name does not exist, it will be created
     * @return fep3::Result 
     * @retval ERR_INVALID_TYPE if the property to register to already exist and has a different type than the property variable
     */
    template <typename variable_type>
    fep3::Result registerVariable(PropertyVariable<variable_type>& property_variable, const std::string& name = "")
    {  
       const auto register_to_this = name.empty();

       if (!register_to_this && !isChild(name))
       {
           setChild(std::make_shared<PropertyNode>(name
                , property_variable.toString()
                , property_variable.getTypeName()));
       }

       T* node_to_register = nullptr;
       if (register_to_this)
       {
           node_to_register = this;
       }
       else
       {
            node_to_register = getChildImpl(name).get();
       }
       
       if (node_to_register->getTypeName() != property_variable.getTypeName())
       {
            return CREATE_ERROR_DESCRIPTION(ERR_INVALID_TYPE,
                "Type of node and property variable are not matching. Node = %s; property variable = %s"
                , node_to_register->getTypeName().c_str()
                , property_variable.getTypeName().c_str());
       }      

       FEP3_RETURN_IF_FAILED(node_to_register->setValue(property_variable.toString()));
     
       node_to_register->registerObserver(property_variable.getObserver());
            
       return {};
    }

    /**
     * @brief Unregister a property variable from this property node.
     * 
     * @tparam variable_type Type of the property_variable
     * @param property_variable Property variable to unregister
     * @param name Name of the property variable
     *              If empty the property variable will unregister from this node, otherwise from a child with this @p name.
     * @return fep3::Result 
     * @retval ERR_NOT_FOUND if property with @p name was not found
     */
    template <typename variable_type>
    fep3::Result unregisterVariable(PropertyVariable<variable_type>& property_variable, const std::string& name ="")
    {
        const auto unregister_from_this = name.empty();

        T* node_to_unregister_from = nullptr;
        if (unregister_from_this)
        {
            node_to_unregister_from = this;
        }
        else
        {
            node_to_unregister_from = getChildImpl(name).get();
        }

        if (!node_to_unregister_from)
        {
            return CREATE_ERROR_DESCRIPTION(ERR_NOT_FOUND, "Node with name '%s' to unregister was not found", name);
        }     
       
        node_to_unregister_from->unregisterObserver(property_variable.getObserver());       
        
        return {};      
    }

protected:    
    /// vector of this nodes children
    std::vector<std::shared_ptr<T>> _children; 
    /// mutex to guard children
    mutable std::shared_timed_mutex _mutex_children; 

    /// vector of registered observers
    std::vector<std::weak_ptr<IPropertyObserver>> _observers; 
    /// mutex to guard observers
    mutable std::shared_timed_mutex _mutex_observer; 

    /// name of this node
    std::string _name; 
    /// value of this node
    std::string _value; 
    /// type of this node
    std::string _type; 
    /// mutex to guard name, value and type
    mutable std::shared_timed_mutex _mutex_strings; 
};

/**
 * @brief Native implementation of @ref fep3::arya::IPropertyNode
 * 
 */
using NativePropertyNode = PropertyNode<IPropertyWithExtendedAccess>;

/**
 * @brief Helper function to create a property node from a typed variable.
 * 
 * @tparam T Type of the property node to create
 * @param name Name of the property node
 * @param value Initial value of the property node
 * @return Create property node as shared_ptr
 */
template <typename T>
std::shared_ptr<fep3::arya::NativePropertyNode> makeNativePropertyNode(const std::string& name, T value)
{
    return std::make_shared<fep3::arya::NativePropertyNode>(name,
        fep3::arya::DefaultPropertyTypeConversion<T>::toString(value),
        fep3::arya::PropertyType<T>::getTypeName());
}

/**
* @brief Component configuration base class
*/
class Configuration
{
    public:
    /**
    * @brief CTOR
    *
    * @param root_node_name Name of the configuration root property node
    */
    Configuration(const std::string& root_node_name)
        : _root_property_node(std::make_shared<NativePropertyNode>(root_node_name))
    {
    }
    
    /**
    * @brief DTOR
    */
    virtual ~Configuration()
    {
    }

    /**
     * @brief Update all registered property variables of the root property node.
     */
    void updatePropertyVariables() const
    {
        _root_property_node->updateObservers();
    }

    /**
     * @brief Inittializes the configuration by calling the convenience function @ref registerPropertyVariables
     *        and adding the local property node to the configuration service
     * @remark this function will store a local pointer to the given \p configuration_service to call 
     *         unregistering while @ref deinitConfiguration
     * @param configuration_service [in] the configuration service to register the property node to
     * @return fep3::Result
     */
    fep3::Result initConfiguration(IConfigurationService& configuration_service)
    {
        if (_configuration_service)
        {
            deinitConfiguration();
        }
        auto res = registerPropertyVariables();
        if (fep3::isFailed(res))
        {
            return res;
        }
        res = configuration_service.registerNode(_root_property_node);
        if (fep3::isOk(res))
        {
            _configuration_service = &configuration_service;
        }
        return res;
    }
    /**
     * @brief deinitializes the configuration by unregistering the property node from the 
     *        local pointer given in configuration_service while @ref initConfiguration 
     *        it will also call the unregisterPropertyVariables
     * @return fep3::Result
     */
    void deinitConfiguration()
    {
        if (_configuration_service && _root_property_node)
        {
            _configuration_service->unregisterNode(_root_property_node->getName());
            _configuration_service = nullptr;
            unregisterPropertyVariables();
        }
    }

    /**
     * @copydoc fep3::arya::PropertyNode::registerVariable
     */
    template<typename variable_type>
    fep3::Result registerPropertyVariable(PropertyVariable<variable_type>& property_variable,
        const std::string& name = "")
    {
        return getNode()->registerVariable(property_variable, name);
    }
    /**
     * @copydoc fep3::arya::PropertyNode::unregisterVariable
     */
    template<typename variable_type>
    fep3::Result unregisterPropertyVariable(PropertyVariable<variable_type>& property_variable,
        const std::string& name = "")
    {
        return getNode()->unregisterVariable(property_variable, name);
    }

    /**
     * @brief Register property variables at the root property node.
     * Override this add properties and property variables to the root node by calling registerPropertyVariable
     *
     * @return fep3::Result
     */
    virtual fep3::Result registerPropertyVariables()
    {
        //return no error 
        return {};
    }
    /**
     * @brief Unregister property variables at the root property node.
     * Override this to unregister property variables from the root node by calling  registerPropertyVariable
     *
     * @return fep3::Result
     */
    virtual fep3::Result unregisterPropertyVariables()
    {
        return {};
    }
    
    /**
     * @brief gets the property node which will also be registered at the configuration service
     * @return std::shared_ptr<NativePropertyNode> the node
     */
    std::shared_ptr<NativePropertyNode> getNode() const
    {
        return _root_property_node;
    }

private:
    /// The root property node which will be registered at the configuration service
    std::shared_ptr<NativePropertyNode> _root_property_node{};
    IConfigurationService* _configuration_service{ nullptr };
};

}

using arya::IPropertyObservable;
using arya::IPropertyObserver;
using arya::PropertyNode;
using arya::NativePropertyNode;
using arya::PropertyVariable;
using arya::makeNativePropertyNode;
using arya::Configuration;

} //end of fep3 namespace
