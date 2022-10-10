// Copyright 2020 The Tint Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef TOOLS_SRC_CMD_REMOTE_COMPILE_RWMUTEX_H_
#define TOOLS_SRC_CMD_REMOTE_COMPILE_RWMUTEX_H_

#include <condition_variable>
#include <mutex>

////////////////////////////////////////////////////////////////////////////////
// RWMutex
////////////////////////////////////////////////////////////////////////////////

/// A RWMutex is a reader/writer mutual exclusion lock.
/// The lock can be held by an arbitrary number of readers or a single writer.
/// Also known as a shared mutex.
class RWMutex {
  public:
    inline RWMutex() = default;

    /// LockReader() locks the mutex for reading.
    /// Multiple read locks can be held while there are no writer locks.
    inline void LockReader();

    /// UnlockReader() unlocks the mutex for reading.
    inline void UnlockReader();

    /// LockWriter() locks the mutex for writing.
    /// If the lock is already locked for reading or writing, LockWriter blocks
    /// until the lock is available.
    inline void LockWriter();

    /// UnlockWriter() unlocks the mutex for writing.
    inline void UnlockWriter();

  private:
    RWMutex(const RWMutex&) = delete;
    RWMutex& operator=(const RWMutex&) = delete;

    int read_locks = 0;
    int pending_write_locks = 0;
    std::mutex mutex;
    std::condition_variable cv;
};

void RWMutex::LockReader() {
    std::unique_lock<std::mutex> lock(mutex);
    read_locks++;
}

void RWMutex::UnlockReader() {
    std::unique_lock<std::mutex> lock(mutex);
    read_locks--;
    if (read_locks == 0 && pending_write_locks > 0) {
        cv.notify_one();
    }
}

void RWMutex::LockWriter() {
    std::unique_lock<std::mutex> lock(mutex);
    if (read_locks > 0) {
        pending_write_locks++;
        cv.wait(lock, [&] { return read_locks == 0; });
        pending_write_locks--;
    }
    lock.release();  // Keep lock held
}

void RWMutex::UnlockWriter() {
    if (pending_write_locks > 0) {
        cv.notify_one();
    }
    mutex.unlock();
}

////////////////////////////////////////////////////////////////////////////////
// RLock
////////////////////////////////////////////////////////////////////////////////

/// RLock is a RAII read lock helper for a RWMutex.
class RLock {
  public:
    /// Constructor.
    /// Locks `mutex` with a read-lock for the lifetime of the WLock.
    /// @param mutex the mutex
    explicit inline RLock(RWMutex& mutex);
    /// Destructor.
    /// Unlocks the RWMutex.
    inline ~RLock();

    /// Move constructor
    /// @param other the other RLock to move into this RLock.
    inline RLock(RLock&& other);
    /// Move assignment operator
    /// @param other the other RLock to move into this RLock.
    /// @returns this RLock so calls can be chained
    inline RLock& operator=(RLock&& other);

  private:
    RLock(const RLock&) = delete;
    RLock& operator=(const RLock&) = delete;

    RWMutex* m;
};

RLock::RLock(RWMutex& mutex) : m(&mutex) {
    m->LockReader();
}

RLock::~RLock() {
    if (m != nullptr) {
        m->UnlockReader();
    }
}

RLock::RLock(RLock&& other) {
    m = other.m;
    other.m = nullptr;
}

RLock& RLock::operator=(RLock&& other) {
    m = other.m;
    other.m = nullptr;
    return *this;
}

////////////////////////////////////////////////////////////////////////////////
// WLock
////////////////////////////////////////////////////////////////////////////////

/// WLock is a RAII write lock helper for a RWMutex.
class WLock {
  public:
    /// Constructor.
    /// Locks `mutex` with a write-lock for the lifetime of the WLock.
    /// @param mutex the mutex
    explicit inline WLock(RWMutex& mutex);

    /// Destructor.
    /// Unlocks the RWMutex.
    inline ~WLock();

    /// Move constructor
    /// @param other the other WLock to move into this WLock.
    inline WLock(WLock&& other);
    /// Move assignment operator
    /// @param other the other WLock to move into this WLock.
    /// @returns this WLock so calls can be chained
    inline WLock& operator=(WLock&& other);

  private:
    WLock(const WLock&) = delete;
    WLock& operator=(const WLock&) = delete;

    RWMutex* m;
};

WLock::WLock(RWMutex& mutex) : m(&mutex) {
    m->LockWriter();
}

WLock::~WLock() {
    if (m != nullptr) {
        m->UnlockWriter();
    }
}

WLock::WLock(WLock&& other) {
    m = other.m;
    other.m = nullptr;
}

WLock& WLock::operator=(WLock&& other) {
    m = other.m;
    other.m = nullptr;
    return *this;
}

#endif  // TOOLS_SRC_CMD_REMOTE_COMPILE_RWMUTEX_H_
