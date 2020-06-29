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

#include <string>
#include <vector>
#include <memory>

#include <fep3/fep3_result_decl.h>
namespace fep3
{
namespace arya
{


/**
 * @brief Interface for a property node to store configuration information.
 * The node can be registered at the @ref fep3::arya::IConfigurationService.
 * The node consists of a value, a name and a vector of children.
 */
class IPropertyNode
{
protected:  
    /**
     * @brief DTOR
     * 
     */
    virtual ~IPropertyNode() = default;

public:
    /**
     * @brief Get the name of the node.
     * 
     * @return Name of node
     */
    virtual std::string getName() const = 0;

     /**
     * @brief Get the current value of the node as string.
     * 
     * @return Current value of node as std::string
     */
    virtual std::string getValue() const = 0;  

    /**
     * @brief Get the type name of the node.
     * 
     * See @ref fep3::arya::PropertyType<T>::getTypeName for default types.
     * 
     * @return Type name of node
     */
    virtual std::string getTypeName() const = 0;

    /**
     * @brief Set the value of the node to @p value.
     * 
     * If a @p type_name is provided it has to be either equal to the type name of this node,
     * or the implementation has to provide a internal conversion. If no conversion is provided
     * an error will be returned.
     * 
     * @param value The value to set for the node as std::string
     * @param type_name The type, the @p value represents.
     *                      Default value is "".
     * 
     * @return fep3::Result
     * @retval ERR_INVALID_TYPE if @p type_name is different than the type name of this node
     *                              and no conversion is provided.
     */
    virtual fep3::Result setValue(const std::string& value, const std::string type_name = "") = 0;  

    /**
     * @brief Compare the node with @p other for equality.
     * 
     * Comparison will be done for the nodes as well as all the children.
     * Value, type and name have to be equal.
     * 
     * @param other Node to compare this node with
     * @retval true if equal
     * @retval false if not equal
     */
    virtual bool isEqual(const IPropertyNode& other) const = 0;

    /**
     * @brief Reset this node itself and all its children.
     * 
     * The reset will change the value to the value at creation.
     */
    virtual void reset() = 0;

    /**
     * @brief Get all children of the node
     * 
     * @return Vector of children
     */
    virtual std::vector<std::shared_ptr<IPropertyNode>> getChildren() const = 0;

    /**
     * @brief Get the number of children of this node.
     * 
     * @return Number of children 
     */
    virtual size_t getNumberOfChildren() const = 0;

    /**
     * @brief Get the child with the name @p name.
     * 
     * @param name Name of the child to look for
     * @return Shared pointer to child with name @p name
     * @retval Empty std::shared_ptr<IPropertyNode> if child was not found
     */
    virtual std::shared_ptr<IPropertyNode> getChild(const std::string& name) const = 0;

    /**
     * @brief Check whether @p name is a child of this node.
     * 
     * @param name Name of the child
     * @return true if @p name is a child of this node
     * @return false if @p name is not a child of this node
     */
    virtual bool isChild(const std::string& name) const = 0;
};

}  // namespace arya
using arya::IPropertyNode;
} // namespace fep3

