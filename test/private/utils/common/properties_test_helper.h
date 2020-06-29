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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fep3/components/configuration/propertynode.h>


inline std::shared_ptr<fep3::IPropertyNode> createTestProperties(const std::string& node_name = std::string("Clock"))
{
    const std::string default_type = fep3::PropertyType<std::string>::getTypeName();

    auto properties_clock = std::make_shared<fep3::NativePropertyNode>(node_name);

    auto node_clocks = std::make_shared<fep3::NativePropertyNode>("Clocks", "2", fep3::PropertyType<int32_t>::getTypeName());

    auto node_clocks1 = std::make_shared<fep3::NativePropertyNode>("Clock1", "my name", default_type);  
    node_clocks1->setChild(std::make_shared<fep3::NativePropertyNode>("CycleTime", "1", fep3::PropertyType<int32_t>::getTypeName()));

    auto node_clocks2 = std::make_shared<fep3::NativePropertyNode>("Clock2");
    node_clocks2->setChild(std::make_shared<fep3::NativePropertyNode>("CycleTime", "2", fep3::PropertyType<int32_t>::getTypeName()));

    properties_clock->setChild(node_clocks);
    node_clocks->setChild(node_clocks1);
    node_clocks->setChild(node_clocks2);

    return properties_clock;
}



inline std::shared_ptr<fep3::IPropertyNode> createTypeTestProperties()
{
    const std::string default_type = fep3::PropertyType<std::string>::getTypeName();
   
    auto node_types = std::make_shared<fep3::NativePropertyNode>("types");
    node_types->setChild(fep3::makeNativePropertyNode<int32_t>("int", 1));
    node_types->setChild(fep3::makeNativePropertyNode<double>("double", 1.0));
    node_types->setChild(fep3::makeNativePropertyNode<bool>("bool", true));
    node_types->setChild(fep3::makeNativePropertyNode<std::string>("bool", "some value"));

    return node_types;
}


inline std::shared_ptr<fep3::NativePropertyNode> setChildImpl(std::shared_ptr<fep3::NativePropertyNode> node, std::shared_ptr<fep3::NativePropertyNode> to_add)
{
    node->setChild(to_add);
    return to_add;
}
