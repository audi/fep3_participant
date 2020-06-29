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
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fep3/components/base/component_base.h>
#include <fep3/components/base/component_registry.h>

class IMyFancyInterface1 
{
    protected:
        //we dont want not be deleted thru the interface!!
        virtual ~IMyFancyInterface1() = default;
    public:
        FEP_COMPONENT_IID("IMyFancyInterface1")

    public:    
        virtual int32_t get1() const = 0;
        virtual void set1(int32_t value) = 0;
};

class IMyFancyInterface2
{
protected:
    //we dont want not be deleted thru the interface!!
    virtual ~IMyFancyInterface2() = default;
public:
    FEP_COMPONENT_IID("IMyFancyInterface2")

public:
    virtual int32_t get2() const = 0;
    virtual void set2(int32_t value) = 0;
};

class IMyFancyInterface3
{
protected:
    //we dont want not be deleted thru the interface!!
    virtual ~IMyFancyInterface3() = default;
public:
    FEP_COMPONENT_IID("IMyFancyInterface3")
};


class MockComponent1 : public fep3::ComponentBase<IMyFancyInterface1>
{

public:
    MOCK_METHOD0(die, void());
    virtual ~MockComponent1()
    {
        die();
    }

    MOCK_CONST_METHOD0(get1, int32_t());
    MOCK_METHOD1(set1, void(int32_t));
    
    // methods of base class ComponentBaseImpl
    MOCK_METHOD0(create, fep3::Result());
    MOCK_METHOD0(destroy, fep3::Result());
    MOCK_METHOD0(initialize, fep3::Result());
    MOCK_METHOD0(tense, fep3::Result());
    MOCK_METHOD0(relax, fep3::Result());
    MOCK_METHOD0(deinitialize, fep3::Result());
    MOCK_METHOD0(start, fep3::Result());
    MOCK_METHOD0(stop, fep3::Result());
    MOCK_METHOD0(pause, fep3::Result());
    
};

class MockComponent2 : public fep3::ComponentBase<IMyFancyInterface2>
{
    
public:
    MOCK_METHOD0(die, void());
    virtual ~MockComponent2()
    {
        die();
    }

    MOCK_CONST_METHOD0(get2, int32_t());
    MOCK_METHOD1(set2, void(int32_t));
    
    // methods of base class ComponentBaseImpl
    MOCK_METHOD0(create, fep3::Result());
    MOCK_METHOD0(destroy, fep3::Result());
    MOCK_METHOD0(initialize, fep3::Result());
    MOCK_METHOD0(tense, fep3::Result());
    MOCK_METHOD0(relax, fep3::Result());
    MOCK_METHOD0(deinitialize, fep3::Result());
    MOCK_METHOD0(start, fep3::Result());
    MOCK_METHOD0(stop, fep3::Result());

    MOCK_METHOD0(pause, fep3::Result());
};

/**
 * Tests the registration and unregistration of a component to the component registry
 * @req_id FEPSDK-1911 FEPSDK-1912 
*/
TEST(BaseComponentRegistryTester, testRegistration)
{  
const auto& registry = std::make_shared<fep3::ComponentRegistry>();
    
    auto mock_component_1 = std::make_unique<::testing::StrictMock<MockComponent1>>();
    const auto& pointer_to_mock_component_1 = mock_component_1.get();
    auto mock_component_2 = std::make_shared<::testing::StrictMock<MockComponent2>>();
    const auto& pointer_to_mock_component_2 = mock_component_2.get();
    
    {
        ::testing::InSequence call_sequence;
        // note: component 2 will be unregistered before destroying the ComponentRegistry 
        // so it must die before component 1
        EXPECT_CALL(*mock_component_2.get(), die()).WillOnce(::testing::Return());
        EXPECT_CALL(*mock_component_1.get(), die()).WillOnce(::testing::Return());
    }

    //we check if registration is okay
    ASSERT_EQ(fep3::Result(), registry->registerComponent<IMyFancyInterface1>(std::move(mock_component_1)));
    ASSERT_EQ(fep3::Result(), registry->registerComponent<IMyFancyInterface2>(mock_component_2));

    //we check if registration is not okay if we want to register the pointer with the same COMP IID again
    // Note: In the following we are not testing the destruction the mock objects
    //, so we can use the NiceMock of MockComponent2 and don't set expectations ...
    auto mock_component_1b = std::make_unique<::testing::NiceMock<MockComponent1>>();
    auto mock_component_2b = std::make_unique<::testing::NiceMock<MockComponent2>>();
    // ... well, we wouldn't need to set expectations if NiceMock would work correctly, but somehow it has no effect
    // (i. e. uninteresting mock function call warnings are emitted anyway; maybe a bug in gmock)
    // , so we have to set expectations anyway.
    EXPECT_CALL(*mock_component_1b.get(), die()).WillOnce(::testing::Return());
    EXPECT_CALL(*mock_component_2b.get(), die()).WillOnce(::testing::Return());
    ASSERT_NE(fep3::Result(), registry->registerComponent<IMyFancyInterface1>(std::move(mock_component_1b)));
    ASSERT_NE(fep3::Result(), registry->registerComponent<IMyFancyInterface2>(std::move(mock_component_2b)));
    
    // test getting pointers to the components from the registry
    EXPECT_EQ(pointer_to_mock_component_1, registry->getComponent<IMyFancyInterface1>());
    EXPECT_EQ(pointer_to_mock_component_2, registry->getComponent<IMyFancyInterface2>());

    //two references: one local and one in ComponentRegistry
    EXPECT_EQ(2, mock_component_2.use_count());
    //we can unregister
    ASSERT_EQ(fep3::Result(), registry->unregisterComponent<IMyFancyInterface2>());
    //the ComponentRegistry must have released its reference to component 2
    EXPECT_EQ(1, mock_component_2.use_count());
    mock_component_2.reset();

    // Note: In the following we are not testing the destruction the mock object
    //, so we can use the NiceMock of MockComponent2 and don't set expectations ...
    auto mock_component_2c = std::make_unique<::testing::NiceMock<MockComponent2>>();
    // ... well, we wouldn't need to set expectations if NiceMock would work correctly, but somehow it has no effect
    // (i. e. uninteresting mock function call warnings are emitted anyway; maybe a bug in gmock)
    // , so we have to set expectations anyway.
    EXPECT_CALL(*mock_component_2c.get(), die()).WillOnce(::testing::Return());

    //check if we only can register IMyFancyInterface1 if the class really supports it
    ASSERT_NE(fep3::Result(), registry->registerComponent<IMyFancyInterface3>(std::move(mock_component_2c)));
}

/**
 * Tests the registration and unregistration of a component to the component registry
 * @req_id FEPSDK-1911 FEPSDK-1912 
*/
TEST(BaseComponentRegistryTester, testCallsToComponents)
{  
const auto& registry = std::make_shared<fep3::ComponentRegistry>();
    
    auto mock_component_1 = std::make_unique<::testing::StrictMock<MockComponent1>>();
    auto mock_component_2 = std::make_unique<::testing::StrictMock<MockComponent2>>();
    
    {
        ::testing::InSequence call_sequence;
        // Component 1 will be registered first, so its methods that put it deeper into the level
        // machine must be called first. In contrast, methods that put it to a higher level, must
        // be called after those of Component 2.
        EXPECT_CALL(*mock_component_1.get(), create()).WillOnce(::testing::Return(fep3::Result{}));
        EXPECT_CALL(*mock_component_2.get(), create()).WillOnce(::testing::Return(fep3::Result{}));

        EXPECT_CALL(*mock_component_2.get(), destroy()).WillOnce(::testing::Return(fep3::Result{}));
        EXPECT_CALL(*mock_component_1.get(), destroy()).WillOnce(::testing::Return(fep3::Result{}));

        EXPECT_CALL(*mock_component_1.get(), initialize()).WillOnce(::testing::Return(fep3::Result{}));
        EXPECT_CALL(*mock_component_2.get(), initialize()).WillOnce(::testing::Return(fep3::Result{}));

        EXPECT_CALL(*mock_component_1.get(), tense()).WillOnce(::testing::Return(fep3::Result{}));
        EXPECT_CALL(*mock_component_2.get(), tense()).WillOnce(::testing::Return(fep3::Result{}));

        EXPECT_CALL(*mock_component_2.get(), relax()).WillOnce(::testing::Return(fep3::Result{}));
        EXPECT_CALL(*mock_component_1.get(), relax()).WillOnce(::testing::Return(fep3::Result{}));

        EXPECT_CALL(*mock_component_2.get(), deinitialize()).WillOnce(::testing::Return(fep3::Result{}));
        EXPECT_CALL(*mock_component_1.get(), deinitialize()).WillOnce(::testing::Return(fep3::Result{}));

        EXPECT_CALL(*mock_component_1.get(), start()).WillOnce(::testing::Return(fep3::Result{}));
        EXPECT_CALL(*mock_component_2.get(), start()).WillOnce(::testing::Return(fep3::Result{}));

        EXPECT_CALL(*mock_component_2.get(), stop()).WillOnce(::testing::Return(fep3::Result{}));
        EXPECT_CALL(*mock_component_1.get(), stop()).WillOnce(::testing::Return(fep3::Result{}));

        EXPECT_CALL(*mock_component_1.get(), pause()).WillOnce(::testing::Return(fep3::Result{}));
        EXPECT_CALL(*mock_component_2.get(), pause()).WillOnce(::testing::Return(fep3::Result{}));
        // note: component 2 will be unregistered before destroying the ComponentRegistry 
        // so it must die before component 1
        EXPECT_CALL(*mock_component_2.get(), die()).WillOnce(::testing::Return());
        EXPECT_CALL(*mock_component_1.get(), die()).WillOnce(::testing::Return());
    }

    //we check if registration is okay
    ASSERT_EQ(fep3::Result(), registry->registerComponent<IMyFancyInterface1>(std::move(mock_component_1)));
    ASSERT_EQ(fep3::Result(), registry->registerComponent<IMyFancyInterface2>(std::move(mock_component_2)));

    // test that all function calls lead to the corresponding function call of the components
    EXPECT_EQ(fep3::Result{}, registry->create());
    EXPECT_EQ(fep3::Result{}, registry->destroy());
    EXPECT_EQ(fep3::Result{}, registry->initialize());
    EXPECT_EQ(fep3::Result{}, registry->tense());
    EXPECT_EQ(fep3::Result{}, registry->relax());
    EXPECT_EQ(fep3::Result{}, registry->deinitialize());
    EXPECT_EQ(fep3::Result{}, registry->start());
    EXPECT_EQ(fep3::Result{}, registry->stop());
    EXPECT_EQ(fep3::Result{}, registry->pause());
}

/**
 * @brief Test component class that implements multiple component interfaces
 */
class MockSuperComponent
    : public fep3::ComponentBase<IMyFancyInterface1, IMyFancyInterface2>
{
public:
    MOCK_METHOD0(die, void());
    virtual ~MockSuperComponent()
    {
        die();
    }

    MOCK_CONST_METHOD0(get1, int32_t());
    MOCK_METHOD1(set1, void(int32_t));
    MOCK_CONST_METHOD0(get2, int32_t());
    MOCK_METHOD1(set2, void(int32_t));
    
    // methods of base class ComponentBaseImpl
    MOCK_METHOD0(create, fep3::Result());
    MOCK_METHOD0(destroy, fep3::Result());
    MOCK_METHOD0(initialize, fep3::Result());
    MOCK_METHOD0(tense, fep3::Result());
    MOCK_METHOD0(relax, fep3::Result());
    MOCK_METHOD0(deinitialize, fep3::Result());
    MOCK_METHOD0(start, fep3::Result());
    MOCK_METHOD0(stop, fep3::Result());
    MOCK_METHOD0(pause, fep3::Result());
};

/**
 * Tests the registration and unregistration of a FEP Component that implements multiple FEP Component interfaces
 * @req_id FEPSDK-2209
*/
TEST(BaseComponentRegistryTester, testRegistrationOfSuperComponent)
{
    const auto& registry = std::make_shared<fep3::ComponentRegistry>();
    
    const auto& mock_super_component = std::make_shared<::testing::StrictMock<MockSuperComponent>>();
    
    EXPECT_CALL(*mock_super_component.get(), die()).WillOnce(::testing::Return());

    // test registration of one component by multiple component iids
    ASSERT_EQ(fep3::Result(), registry->registerComponent<IMyFancyInterface1>(mock_super_component));
    ASSERT_EQ(fep3::Result(), registry->registerComponent<IMyFancyInterface2>(mock_super_component));
    
    // test getting pointers to the components from the registry
    EXPECT_EQ(mock_super_component.get(), registry->getComponent<IMyFancyInterface1>());
    EXPECT_EQ(mock_super_component.get(), registry->getComponent<IMyFancyInterface2>());

    // three references: one local, two in ComponentRegistry
    EXPECT_EQ(3, mock_super_component.use_count());
    // unregistration by one of the registered interfaces must not lead to destruction of the component
    ASSERT_EQ(fep3::Result(), registry->unregisterComponent<IMyFancyInterface1>());
    //the ComponentRegistry must have released one reference
    EXPECT_EQ(2, mock_super_component.use_count());
}