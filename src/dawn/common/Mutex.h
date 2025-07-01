// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_COMMON_MUTEX_H_
#define SRC_DAWN_COMMON_MUTEX_H_

#include <atomic>
#include <mutex>
#include <thread>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/NonCopyable.h"
#include "dawn/common/NonMovable.h"
#include "dawn/common/Ref.h"
#include "dawn/common/RefCounted.h"

namespace dawn {

template <typename MutexT>
class MutexBase : public RefCounted, NonCopyable {
  public:
    template <typename MutexRef>
    struct AutoLockBase : NonMovable {
        AutoLockBase() : mMutex(nullptr) {}
        explicit AutoLockBase(MutexRef mutex) : mMutex(std::move(mutex)) {
            if (mMutex != nullptr) {
#if defined(DAWN_ENABLE_ASSERTS)
                auto currentThread = std::this_thread::get_id();
                if constexpr (!std::is_same_v<MutexT, std::recursive_mutex>) {
                    DAWN_ASSERT(mMutex->mOwner.load(std::memory_order_acquire) != currentThread);
                }
#endif  // DAWN_ENABLE_ASSERTS

                mMutex->Lock();

#if defined(DAWN_ENABLE_ASSERTS)
                if (mMutex->mOwner.exchange(currentThread, std::memory_order_release) ==
                    std::thread::id()) {
                    // This is the first time that the lock is being acquired in the stack.
                    mThreadId = &mMutex->mOwner;
                }
#endif  // DAWN_ENABLE_ASSERTS
            }
        }

        ~AutoLockBase() {
            if (mMutex != nullptr) {
#if defined(DAWN_ENABLE_ASSERTS)
                DAWN_ASSERT(mMutex->IsLockedByCurrentThread());
                if (mThreadId) {
                    mThreadId->store(std::thread::id(), std::memory_order_release);
                }
#endif  // DAWN_ENABLE_ASSERTS
                mMutex->Unlock();
            }
        }

      private:
        MutexRef mMutex;

#if defined(DAWN_ENABLE_ASSERTS)
        // Additional variables to help track the thread ids and to ensure that it works for
        // recursive type locks.
        std::atomic<std::thread::id>* mThreadId = nullptr;
#endif  // DAWN_ENABLE_ASSERTS
    };

    // This scoped lock won't keep the mutex alive.
    using AutoLock = AutoLockBase<MutexBase<MutexT>*>;
    // This scoped lock will keep the mutex alive until out of scope.
    using AutoLockAndHoldRef = AutoLockBase<Ref<MutexBase<MutexT>>>;

    ~MutexBase() override = default;

    // This method is only enabled when DAWN_ENABLE_ASSERTS is turned on.
    // Thus it should only be wrapped inside DAWN_ASSERT() macro.
    // i.e. DAWN_ASSERT(mutex.IsLockedByCurrentThread())
    bool IsLockedByCurrentThread() {
#if defined(DAWN_ENABLE_ASSERTS)
        return mOwner.load(std::memory_order_acquire) == std::this_thread::get_id();
#else
        // This is not supported.
        DAWN_CHECK(false);
#endif
    }

  private:
    void Lock() { mNativeMutex.lock(); }
    void Unlock() { mNativeMutex.unlock(); }

  private:
#if defined(DAWN_ENABLE_ASSERTS)
    std::atomic<std::thread::id> mOwner;
#endif  // DAWN_ENABLE_ASSERTS
    MutexT mNativeMutex;
};

using Mutex = MutexBase<std::mutex>;
using RecursiveMutex = MutexBase<std::recursive_mutex>;

}  // namespace dawn

#endif
