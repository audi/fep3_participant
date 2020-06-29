/**
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

#include <gmock/gmock.h>

#include <fep3/components/configuration/configuration_service_intf.h>
#include <fep3/base/properties/properties_intf.h>
#include <fep3/components/base/component_base.h>

namespace fep3
{
namespace mock 
{

struct PropertyNode : public arya::IPropertyNode
{
    MOCK_CONST_METHOD0(getName, std::string());
    MOCK_CONST_METHOD0(getValue, std::string());
    MOCK_CONST_METHOD0(getTypeName, std::string());
    MOCK_METHOD2(setValueImpl, Result(const std::string&, const std::string));
    virtual Result setValue(const std::string& value, const std::string type_name = "")
    {
        setValueImpl(value, type_name);

        return Result{};
    }
    MOCK_CONST_METHOD1(isEqual, bool(const IPropertyNode&));
    MOCK_METHOD0(reset, void());
    MOCK_CONST_METHOD0(getChildren, std::vector<std::shared_ptr<IPropertyNode>>());
    MOCK_CONST_METHOD0(getNumberOfChildren, size_t());
    MOCK_CONST_METHOD1(getChild, std::shared_ptr<IPropertyNode>(const std::string&));
    MOCK_CONST_METHOD1(isChild, bool(const std::string&));
};

struct ConfigurationServiceComponent : public ComponentBase<IConfigurationService>
{
    MOCK_METHOD1(registerNode, Result(std::shared_ptr<arya::IPropertyNode>));
    MOCK_METHOD1(unregisterNode, Result(const std::string&));
    MOCK_CONST_METHOD1(isNodeRegistered, bool(const std::string&));
    MOCK_CONST_METHOD1(getNode, std::shared_ptr<arya::IPropertyNode>(const std::string&));
    MOCK_CONST_METHOD1(getConstNode, std::shared_ptr<const arya::IPropertyNode>(const std::string&));
    MOCK_CONST_METHOD3(createSystemProperty, fep3::Result(const std::string&, const std::string&, const std::string&));
};

}
} 
