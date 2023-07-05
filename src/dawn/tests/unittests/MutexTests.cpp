// Copyright 2023 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "dawn/common/Mutex.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

#if defined(DAWN_ENABLE_ASSERTS)
constexpr bool kAssertEnabled = true;
#else
constexpr bool kAssertEnabled = false;
#endif

class MutexTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // IsLockedByCurrentThread() requires DAWN_ENABLE_ASSERTS flag enabled.
        if (!kAssertEnabled) {
            GTEST_SKIP() << "DAWN_ENABLE_ASSERTS is not enabled";
        }
    }

    Mutex mMutex;
};

// Simple Lock() then Unlock() test.
TEST_F(MutexTest, SimpleLockUnlock) {
    mMutex.Lock();
    EXPECT_TRUE(mMutex.IsLockedByCurrentThread());
    mMutex.Unlock();
    EXPECT_FALSE(mMutex.IsLockedByCurrentThread());
}

// Test AutoLock automatically locks the mutex and unlocks it when out of scope.
TEST_F(MutexTest, AutoLock) {
    {
        Mutex::AutoLock autoLock(&mMutex);
        EXPECT_TRUE(mMutex.IsLockedByCurrentThread());
    }
    EXPECT_FALSE(mMutex.IsLockedByCurrentThread());
}

// Test AutoLockAndHoldRef will keep the mutex alive
TEST_F(MutexTest, AutoLockAndHoldRef) {
    auto* mutex = new Mutex();
    EXPECT_EQ(mutex->GetRefCountForTesting(), 1u);
    {
        Mutex::AutoLockAndHoldRef autoLock(mutex);
        EXPECT_TRUE(mutex->IsLockedByCurrentThread());
        EXPECT_EQ(mutex->GetRefCountForTesting(), 2u);

        mutex->Release();
        EXPECT_EQ(mutex->GetRefCountForTesting(), 1u);
    }
}

using MutexDeathTest = MutexTest;

// Test that Unlock() call on unlocked mutex will cause assertion failure.
TEST_F(MutexDeathTest, UnlockWhenNotLocked) {
    ASSERT_DEATH_IF_SUPPORTED({ mMutex.Unlock(); }, "");
}

// Double Lock() calls should be cause assertion failure
TEST_F(MutexDeathTest, DoubleLockCalls) {
    mMutex.Lock();
    EXPECT_TRUE(mMutex.IsLockedByCurrentThread());
    ASSERT_DEATH_IF_SUPPORTED({ mMutex.Lock(); }, "");
    mMutex.Unlock();
}

// Lock() then use AutoLock should cause assertion failure.
TEST_F(MutexDeathTest, LockThenAutoLock) {
    mMutex.Lock();
    EXPECT_TRUE(mMutex.IsLockedByCurrentThread());
    ASSERT_DEATH_IF_SUPPORTED({ Mutex::AutoLock autoLock(&mMutex); }, "");
    mMutex.Unlock();
}

// Use AutoLock then call Lock() should cause assertion failure.
TEST_F(MutexDeathTest, AutoLockThenLock) {
    Mutex::AutoLock autoLock(&mMutex);
    EXPECT_TRUE(mMutex.IsLockedByCurrentThread());
    ASSERT_DEATH_IF_SUPPORTED({ mMutex.Lock(); }, "");
}

}  // anonymous namespace
}  // namespace dawn
