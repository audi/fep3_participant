/**
* @file
* Copyright &copy; Audi AG. All rights reserved.
*
* This Source Code Form is subject to the terms of the
* Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#ifndef _FEP_LOCKED_QUEUE_
#define _FEP_LOCKED_QUEUE_

#include <queue>
#include <a_util/concurrency/fast_mutex.h>

namespace fep3
{
namespace base
{
namespace detail
{
/// Template class of a locked queue.
/// The specialization using a std::queue is the class LockedQueue
template <typename T, class QUEUE, class GUARD_MUTEX> class LockedQueueAdaptor
{
private:
    QUEUE       _queue; ///< The internal queue
    GUARD_MUTEX _lock;  ///< Guarding mutex

public:
    /// CTOR
    LockedQueueAdaptor() : _queue(), _lock() { }
    /// DTOR
    ~LockedQueueAdaptor() { }

public:
    /// Push an element at end of queue and notify consumer
    /// @param [in] t Element to add to the queue
    void enqueue(const T& t)
    {
        _lock.lock();
        _queue.push(t);
        _lock.unlock();
    }

    /// Try to copy the first element and remove it from the queue
    /// @param [out] t The first element of the queue, if present
    /// @retval true Element found
    /// @retval false Queue is empty
    bool tryDequeue(T& t)
    {
        _lock.lock();
        if (_queue.empty())
        {
            _lock.unlock();
            return false;
        }
        t = _queue.front();
        _queue.pop();
        _lock.unlock();
        return true;
    }

    /// Try to copy the first element and remove it from the queue.
    /// Unlock guard, when queue is empty.
    /// @param [out] t The first element of the queue, if present
    /// @param [in] guard Mutex to guard the queue
    /// @retval true Element found
    /// @retval false Queue is empty
    template <class GUARD> bool tryDequeueAndUnlockGuardIfEmpty(T& t, GUARD& guard)
    {
        _lock.lock();
        if (_queue.empty())
        {
            _lock.unlock();
            guard.unlock();
            return false;
        }
        t = _queue.front();
        _queue.pop();
        _lock.unlock();
        return true;
    }
};

/// Template class of a unlocked queue.
/// The specialization using a std::queue is the class UnlockedQueue
template <typename T, class QUEUE> class UnlockedQueueAdaptor
{
private:
    QUEUE _queue; ///< The internal queue

public:
    /// CTOR
    UnlockedQueueAdaptor() : _queue() { }
    /// DTOR
    ~UnlockedQueueAdaptor() { }

public:
    /// Push an element at end of queue and notify consumer
    /// @param [in] t Element to add to the queue
    void enqueue(const T& t)
    {
        _queue.push(t);
    }

    /// Try to copy the first element and remove it from the queue
    /// @param [out] t The first element of the queue, if present
    /// @retval true Element found
    /// @retval false Queue is empty
    bool tryDequeue(T& t)
    {
        if (_queue.empty())
        {
            return false;
        }
        t = _queue.front();
        _queue.pop();
        return true;
    }
};
} // namespace detail

/// Locked Queue Template
template <typename T, class Alloc = std::allocator<T> >
class LockedQueue : public detail::LockedQueueAdaptor<T, std::queue<T, std::deque<T, Alloc> >, a_util::concurrency::fast_mutex > { };
/// Unlocked Queue Template
template <typename T, class Alloc = std::allocator<T> >
class UnlockedQueue : public detail::UnlockedQueueAdaptor<T, std::queue<T, std::deque<T, Alloc> > > { };

} // namespace base
} // namespace fep3

#endif // _FEP_LOCKED_QUEUE_
