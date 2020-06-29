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

#include <stdexcept>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fep3/participant/state_machine/participant_state_machine.h>
#include <fep3/participant/mock/mock_element_base.h>

/**
 * Test the state machine, when it has no element manager set
 *         -> must be unable to load
 * @req_id // TODO reference the REQUIREMENT as soon it has been created
 */
TEST(BaseParticipantStateMachineTester, testNoElementManager)
{
    // dont't set the element manager / component registry in this test
    ::fep3::arya::ParticipantStateMachine state_machine({}, {}, {});
    // the state machine must immediately enter the state "Unloaded"
    EXPECT_FALSE(state_machine.isFinalized());
    EXPECT_EQ("Unloaded", state_machine.getCurrentStateName());

    // loading must fail because no element manager was set
    EXPECT_FALSE(state_machine.load());
    // after failed loading, the state must still be "Unloaded"
    EXPECT_EQ("Unloaded", state_machine.getCurrentStateName());

    // all other events (except exit) must fail
    EXPECT_FALSE(state_machine.unload());
    EXPECT_FALSE(state_machine.initialize());
    EXPECT_FALSE(state_machine.deinitialize());
    EXPECT_FALSE(state_machine.start());
    EXPECT_FALSE(state_machine.stop());
    EXPECT_FALSE(state_machine.pause());

    // exit must succeed
    EXPECT_TRUE(state_machine.exit());
    ASSERT_TRUE(state_machine.isFinalized());
    // state machine has finalized, so the current state name is unknown
    EXPECT_EQ("Unknown", state_machine.getCurrentStateName());
}

/**
 * Test the state machine in normal operation (i. e. no error)
 *  * step through all states
 *  * in each state check that
 *  * all events, that don't trigger a transition (uncommented below), are not handled
 * @req_id // TODO reference the REQUIREMENT as soon it has been created
 */
TEST(BaseParticipantStateMachineTester, testNormalOperation)
{
    // wrapping unique_ptr into shared_ptr to be able to capture it in the lambda for the element factory (see below)
    auto test_element_wrapper = std::make_shared<std::unique_ptr<::testing::StrictMock<::fep3::mock::MockElementBase>>>
        (std::make_unique<::testing::StrictMock<::fep3::mock::MockElementBase>>());
    {
        auto& test_element = *(test_element_wrapper.get()->get());
        ::testing::InSequence call_sequence;
        EXPECT_CALL(test_element, initialize()).WillOnce(::testing::Return(::fep3::Result{}));
        EXPECT_CALL(test_element, run()).WillOnce(::testing::Return(::fep3::Result{}));
        EXPECT_CALL(test_element, stop()).WillOnce(::testing::Return());
        EXPECT_CALL(test_element, deinitialize()).WillOnce(::testing::Return());
        EXPECT_CALL(test_element, die()).WillOnce(::testing::Return());
    }

    const auto& component_registry = std::make_shared<::fep3::ComponentRegistry>();
    std::shared_ptr<fep3::ILoggingService::ILogger> logger;

    class TestElementFactory : public ::fep3::IElementFactory
    {
    public:
        TestElementFactory(const std::shared_ptr<std::unique_ptr<::testing::StrictMock<::fep3::mock::MockElementBase>>>& test_element_wrapper)
            : _test_element_wrapper(test_element_wrapper)
        {}
        std::unique_ptr<::fep3::IElement> createElement(const ::fep3::IComponents&) const override
        {
            if(_test_element_wrapper)
            {
                auto& unique_ptr_to_element = *(_test_element_wrapper.get());
                return std::move(unique_ptr_to_element);
            }
            else
            {
                return {};
            }
        }
    private:
        std::shared_ptr<std::unique_ptr<::testing::StrictMock<::fep3::mock::MockElementBase>>> _test_element_wrapper;
    };

    ::fep3::arya::ParticipantStateMachine state_machine
        (::fep3::arya::ElementManager(std::make_shared<TestElementFactory>(test_element_wrapper))
        , component_registry
        , logger
        );
    // the state machine must immediately enter the state "Unloaded"
    EXPECT_EQ("Unloaded", state_machine.getCurrentStateName());
    //EXPECT_FALSE(state_machine.load());
    EXPECT_FALSE(state_machine.unload());
    EXPECT_FALSE(state_machine.initialize());
    EXPECT_FALSE(state_machine.deinitialize());
    EXPECT_FALSE(state_machine.start());
    EXPECT_FALSE(state_machine.pause());
    EXPECT_FALSE(state_machine.stop());
    //EXPECT_FALSE(state_machine.exit());

    // switch to Loaded
    EXPECT_TRUE(state_machine.load());
    EXPECT_EQ("Loaded", state_machine.getCurrentStateName());
    EXPECT_FALSE(state_machine.load());
    //EXPECT_FALSE(state_machine.unload());
    //EXPECT_FALSE(state_machine.initialize());
    EXPECT_FALSE(state_machine.deinitialize());
    EXPECT_FALSE(state_machine.start());
    EXPECT_FALSE(state_machine.pause());
    EXPECT_FALSE(state_machine.stop());
    EXPECT_FALSE(state_machine.exit());

    // switch to Initialized
    EXPECT_TRUE(state_machine.initialize());
    EXPECT_EQ("Initialized", state_machine.getCurrentStateName());
    EXPECT_FALSE(state_machine.load());
    EXPECT_FALSE(state_machine.unload());
    EXPECT_FALSE(state_machine.initialize());
    //EXPECT_FALSE(state_machine.deinitialize());
    //EXPECT_FALSE(state_machine.start());
    //EXPECT_FALSE(state_machine.pause());
    EXPECT_FALSE(state_machine.stop());
    EXPECT_FALSE(state_machine.exit());

    // switch to Running
    EXPECT_TRUE(state_machine.start());
    EXPECT_EQ("Running", state_machine.getCurrentStateName());
    EXPECT_FALSE(state_machine.load());
    EXPECT_FALSE(state_machine.unload());
    EXPECT_FALSE(state_machine.initialize());
    EXPECT_FALSE(state_machine.deinitialize());
    EXPECT_FALSE(state_machine.start());
    //EXPECT_FALSE(state_machine.pause());
    //EXPECT_FALSE(state_machine.stop());
    EXPECT_FALSE(state_machine.exit());

    // switch to Paused
    EXPECT_TRUE(state_machine.pause());
    EXPECT_EQ("Paused", state_machine.getCurrentStateName());
    EXPECT_FALSE(state_machine.load());
    EXPECT_FALSE(state_machine.unload());
    EXPECT_FALSE(state_machine.initialize());
    EXPECT_FALSE(state_machine.deinitialize());
    //EXPECT_FALSE(state_machine.start());
    EXPECT_FALSE(state_machine.pause());
    //EXPECT_FALSE(state_machine.stop());
    EXPECT_FALSE(state_machine.exit());

    // switch back to Initialized
    EXPECT_TRUE(state_machine.stop());
    EXPECT_EQ("Initialized", state_machine.getCurrentStateName());
    EXPECT_FALSE(state_machine.load());
    EXPECT_FALSE(state_machine.unload());
    EXPECT_FALSE(state_machine.initialize());
    //EXPECT_FALSE(state_machine.deinitialize());
    //EXPECT_FALSE(state_machine.start());
    //EXPECT_FALSE(state_machine.pause());
    EXPECT_FALSE(state_machine.stop());
    EXPECT_FALSE(state_machine.exit());

    // switch back to Loaded
    EXPECT_TRUE(state_machine.deinitialize());
    EXPECT_EQ("Loaded", state_machine.getCurrentStateName());
    EXPECT_FALSE(state_machine.load());
    //EXPECT_FALSE(state_machine.unload());
    //EXPECT_FALSE(state_machine.initialize());
    EXPECT_FALSE(state_machine.deinitialize());
    EXPECT_FALSE(state_machine.start());
    EXPECT_FALSE(state_machine.pause());
    EXPECT_FALSE(state_machine.stop());
    EXPECT_FALSE(state_machine.exit());

    // switch back to Unloaded
    EXPECT_TRUE(state_machine.unload());
    EXPECT_EQ("Unloaded", state_machine.getCurrentStateName());
    //EXPECT_FALSE(state_machine.load());
    EXPECT_FALSE(state_machine.unload());
    EXPECT_FALSE(state_machine.initialize());
    EXPECT_FALSE(state_machine.deinitialize());
    EXPECT_FALSE(state_machine.start());
    EXPECT_FALSE(state_machine.pause());
    EXPECT_FALSE(state_machine.stop());
    //EXPECT_FALSE(state_machine.exit());

    // exit
    EXPECT_TRUE(state_machine.exit());
    ASSERT_TRUE(state_machine.isFinalized());
    // state machine has finalized, so the current state name is unknown
    EXPECT_EQ("Unknown", state_machine.getCurrentStateName());
}
