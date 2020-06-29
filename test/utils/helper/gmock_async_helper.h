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

#include <condition_variable>

namespace test
{
namespace helper
{

/**
 * Class providing facilities to wait for a notification asynchronously
 * This class implements what the gMock cook book
 * https://github.com/google/googletest/blob/master/googlemock/docs/cook_book.md
 * suggests on "Testing Asynchronous Behavior".
 *
 * Usage:
 * First define a gMock action like so:
 *
 * ACTION_P(Notify, notification)
 * {
 *     notification->notify();
 * }
 *
 * Then in your test do:
 *
 * Notification done;
 * EXPECT_CALL(mock_event_dispatcher, dispatchEvent(kEventId))
 *     .WillOnce(Notify(&done));
 * // Call stuff that causes the method "dispatchEvent" of the mock object "mock_event_dispatcher" to be invoked asynchronously
 * // ...
 * // Then wait for the notification to be notified.
 * done.waitForNotification();
 * // or done.waitForNotificationWithTimeout(std::chrono::milliseconds(0));
 *
 */
class Notification
{
public:
    /**
     * CTOR
     */
    Notification()
    {}
    /**
     * DTOR
     */
    virtual ~Notification() = default;

    /**
     * Waits for the \ref Notify method to be called
     */
    void waitForNotification()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _cv.wait
            (lock
            , [this]()
                {
                    return _notified;
                }
            );
        // reset _notified to be able to wait again on this Notification
        _notified = false;
    }

    /**
     * Waits for the \ref notify method to be called with \p timeout
     *
     * @tparam rep_type Arithmethic type representing the number of ticks (see std::chrono::duration for further information)
     * @tparam period_type Period representing the tick period (see std::chrono::duration for further information)
     * @param timeout The duration how long to wait for the \ref Notify method to be called
     * @return true if the notification has arrived in time, false otherwise
     */
    template<typename rep_type, typename period_type>
    bool waitForNotificationWithTimeout(const std::chrono::duration<rep_type, period_type>& timeout)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        auto wait_flag = _cv.wait_for
            (lock
            , timeout
            , [this]()
                {
                    return _notified;
                }
            );
        // reset _notified to be able to wait again on this Notification
        _notified = false;
        
        return wait_flag;
    }

    /**
     * @brief Notifies the notification
     * This method causes any threads currently waiting for a notification
     * in \ref WaitForNotification or \ref WaitForNotificationWithTimeout
     * to return.
     */
    void notify()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _notified = true;
        _cv.notify_all();
    }

private:
    std::mutex _mutex;
    std::condition_variable _cv;
    bool _notified{false};
};


} // namespace helper
} // namespace test