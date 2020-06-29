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

#include <scenario/scenario_fixtures.h>
#include <helper/gmock_async_helper.h>

using namespace fep3;
using namespace cpp;
using namespace std::literals::chrono_literals;
using namespace ::testing;
using namespace fep3::test::scenario;

struct MyCoreJob_50ms : NiceMock<mock::core::Job>
{
    MyCoreJob_50ms() : NiceMock<Job>("core_job_50ms", Duration{50ms})
    {
        setDefaultBehaviour();
    }
};

template <class T>
struct MyMasterSlaveSystem
    : public T
{    
    void SetUp() override
    {   
        T::SetUp();

        auto slave_clock = T::getWrapper("test_timing_slave")->clock_event_sink;
        auto master_clock = T::getWrapper("test_timing_master")->clock_event_sink;

        EXPECT_CALL(*slave_clock, timeResetBegin(_,_)).Times(1);
        EXPECT_CALL(*slave_clock, timeResetEnd(_)).Times(1);

        EXPECT_CALL(*master_clock, timeResetBegin(_,_)).Times(1);
        EXPECT_CALL(*master_clock, timeResetEnd(_)).Times(1);
    }    

    virtual std::vector<std::shared_ptr<Participant>> createParticipants() const override
    {
        const std::string master_name {"test_timing_master"};
        const std::string slave_name {"test_timing_slave"};

        auto master = std::make_shared<Participant>(cpp::createParticipant<MyElement<MyCoreJob_50ms>>(
            master_name, T::_system_name));  

        auto slave = std::make_shared<Participant>(cpp::createParticipant<MyElement<MyCoreJob_50ms>>(
            slave_name, T::_system_name));

        return  { slave, master };
    }   
};

ACTION_P(Notify, notification)
{
     notification->notify();
}

using MyDiscreteSystem = MyMasterSlaveSystem<MasterSlaveSystemDiscrete>;
using MyContinuousSysem = MyMasterSlaveSystem<MasterSlaveSystemContinuous>;

TEST_F(MyDiscreteSystem, twoParticipantsSynchronizedDiscrete)
{   
    Initialized();

    const auto core_job = getWrapper("test_timing_slave")->getJob("core_job_50ms");
    const auto mock_job = reinterpret_cast<MyCoreJob_50ms*>(core_job);
    
    ::test::helper::Notification done;

    EXPECT_CALL(*mock_job, execute(_)).Times(AnyNumber());
    EXPECT_CALL(*mock_job, execute(fep3::Timestamp(0))).Times(1);
    EXPECT_CALL(*mock_job, execute(fep3::Timestamp(50ms))).Times(1);
    EXPECT_CALL(*mock_job, execute(fep3::Timestamp(100ms))).Times(1);
    EXPECT_CALL(*mock_job, execute(fep3::Timestamp(150ms))).WillOnce(DoAll(
        Notify(&done), 
        Return(ERR_NOERROR)));

    Running();    

    ASSERT_TRUE(done.waitForNotificationWithTimeout(std::chrono::seconds(5)));

    Initialized();  
}