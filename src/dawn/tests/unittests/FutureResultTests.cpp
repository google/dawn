// Copyright 2025 The Dawn & Tint Authors
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

#include <memory>
#include <thread>
#include <vector>

#include "dawn/common/Ref.h"
#include "dawn/common/RefCounted.h"
#include "dawn/native/Error.h"
#include "dawn/native/FutureResult.h"
#include "dawn/utils/TestUtils.h"
#include "gtest/gtest.h"

namespace dawn::native {
namespace {

class FutureResultTest : public ::testing::Test {};

// Test basic success case where TryGet waits for Set.
TEST(FutureResultTest, BasicSuccess) {
    Ref<FutureResult<int>> future = AcquireRef(new FutureResult<int>());

    std::thread t([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        future->Set(42);
    });

    ResultOrError<int> result = future->TryGet();
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_EQ(result.AcquireSuccess(), 42);
    t.join();
}

// Test basic error case where TryGet waits for Set.
TEST(FutureResultTest, BasicError) {
    Ref<FutureResult<int>> future = AcquireRef(new FutureResult<int>());

    std::thread t([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        future->Set(DAWN_INTERNAL_ERROR("Test Error"));
    });

    ResultOrError<int> result = future->TryGet();
    ASSERT_TRUE(result.IsError());
    std::unique_ptr<ErrorData> err = result.AcquireError();
    ASSERT_NE(err, nullptr);
    ASSERT_EQ(err->GetMessage(), "Test Error");
    t.join();
}

// Test getting the result multiple times on success.
TEST(FutureResultTest, MultipleGetsSuccess) {
    Ref<FutureResult<int>> future = AcquireRef(new FutureResult<int>());
    future->Set(55);

    // First get should succeed.
    ResultOrError<int> result1 = future->TryGet();
    ASSERT_TRUE(result1.IsSuccess());
    ASSERT_EQ(result1.AcquireSuccess(), 55);

    // Second get should also succeed with the same value.
    ResultOrError<int> result2 = future->TryGet();
    ASSERT_TRUE(result2.IsSuccess());
    ASSERT_EQ(result2.AcquireSuccess(), 55);
}

// Test getting the result multiple times on error.
TEST(FutureResultTest, MultipleGetsError) {
    Ref<FutureResult<int>> future = AcquireRef(new FutureResult<int>());
    future->Set(DAWN_INTERNAL_ERROR("Original Error"));

    // First get should get the original error.
    ResultOrError<int> result1 = future->TryGet();
    ASSERT_TRUE(result1.IsError());
    std::unique_ptr<ErrorData> err1 = result1.AcquireError();
    ASSERT_NE(err1, nullptr);
    ASSERT_EQ(err1->GetMessage(), "Original Error");

    // Second get should get a placeholder error because the first moved the error.
    ResultOrError<int> result2 = future->TryGet();
    ASSERT_TRUE(result2.IsError());
    std::unique_ptr<ErrorData> err2 = result2.AcquireError();
    ASSERT_NE(err2, nullptr);
    ASSERT_EQ(err2->GetMessage(), "Error was already moved.");
}

// Stress test setting the result on one thread and getting it on multiple other threads.
TEST(FutureResultTest, MultiThreadedGet) {
    Ref<FutureResult<int>> future = AcquireRef(new FutureResult<int>());
    constexpr int kNumThreads = 4;
    constexpr int kExpectedValue = 12345;

    auto getter = [&](uint32_t) {
        ResultOrError<int> result = future->TryGet();
        ASSERT_TRUE(result.IsSuccess());
        ASSERT_EQ(result.AcquireSuccess(), kExpectedValue);
    };

    auto setter = [&]() {
        // Sleep to give getters a chance to wait.
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        future->Set(kExpectedValue);
    };

    utils::RunInParallel(kNumThreads, getter, setter);
}

// Test that multiple threads calling TryGet on an error future all receive an error.
TEST(FutureResultTest, MultipleThreadsGetError) {
    Ref<FutureResult<int>> future = AcquireRef(new FutureResult<int>());
    constexpr int kNumThreads = 8;

    auto getter = [&](uint32_t) {
        ResultOrError<int> result = future->TryGet();
        ASSERT_TRUE(result.IsError());
        result.AcquireError();
    };

    auto setter = [&]() {
        // Sleep to give getters a chance to wait.
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        future->Set(DAWN_INTERNAL_ERROR("Test error"));
    };

    utils::RunInParallel(kNumThreads, getter, setter);
}

#ifdef DAWN_ENABLE_ASSERTS
// Name "*DeathTest" per https://google.github.io/googletest/advanced.html#death-test-naming
using FutureResultDeathTest = FutureResultTest;

// Test that calling Set twice causes an assertion failure.
TEST_F(FutureResultDeathTest, DoubleSet) {
    Ref<FutureResult<int>> future = AcquireRef(new FutureResult<int>());
    future->Set(1);
    ASSERT_DEATH_IF_SUPPORTED({ future->Set(2); }, "");
}
#endif

}  // namespace
}  // namespace dawn::native
