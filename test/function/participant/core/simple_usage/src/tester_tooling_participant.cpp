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

#include <fep3/core.h>
#include <fep3/core/participant_executor.hpp>

class MyElement : public fep3::core::ElementBase
{
public:
    MyElement() : ElementBase("test", "testversion")
    {
    }
};

/**
 * @detail Test the registration, unregistration and memorymanagment of the ComponentRegistry
 * @req_id FEPSDK-Sample
 */
TEST(ParticipantUsageTest, testUseTheFactory)
{
    using namespace fep3::core;
    Participant part = createParticipant<ElementFactory<MyElement>>("test", "testversion", "testsystem");
    ParticipantExecutor executor(part); 
    executor.exec();//this will not block
    executor.load();
    executor.initialize();
    executor.start();
}
