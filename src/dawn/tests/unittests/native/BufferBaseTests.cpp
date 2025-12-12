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

#include <condition_variable>
#include <mutex>
#include <thread>
#include <utility>

#include "dawn/common/Log.h"
#include "dawn/common/Ref.h"
#include "dawn/utils/TestUtils.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "mocks/BufferMock.h"
#include "mocks/DawnMockTest.h"

using testing::_;
using testing::ByMove;
using testing::Eq;
using testing::Return;

namespace dawn::native {
namespace {

constexpr size_t kBufferSize = 16;
constexpr std::string_view kValidationErrorMessage = "Concurrent buffer operations are not allowed";

class WaitableEvent {
  public:
    void Signal() {
        std::scoped_lock lock(mMutex);
        mSignaled = true;
        mCv.notify_all();
    }
    void Wait() {
        std::unique_lock<std::mutex> lock(mMutex);
        while (!mSignaled) {
            mCv.wait(lock);
        }
    }

  private:
    std::mutex mMutex;
    std::condition_variable mCv;
    bool mSignaled = false;
};

using MockMapAsyncCallback =
    testing::StrictMock<testing::MockCppCallback<void (*)(wgpu::MapAsyncStatus, wgpu::StringView)>>;

class BufferBaseTest : public DawnMockTest {
  protected:
    void SetUp() override {
        DawnMockTest::SetUp();
        BufferDescriptor desc = {};
        desc.size = kBufferSize;
        desc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;

        mBufferMock = AcquireRef(new BufferMock(mDeviceMock, &desc));
        EXPECT_CALL(*mDeviceMock, CreateBufferImpl).WillOnce(Return(mBufferMock));
        mBuffer = device.CreateBuffer(ToCppAPI(&desc));
    }

    void TearDown() override {
        mBuffer = {};
        mBufferMock = {};
        DawnMockTest::TearDown();
    }

    Ref<BufferMock> mBufferMock;
    wgpu::Buffer mBuffer;
};

// Test that MapAsyncImpl() throwing an error causes map async callback to return an error status.
TEST_F(BufferBaseTest, MapAsyncImplError) {
    constexpr std::string_view kErrorText = "Platform error";

    EXPECT_CALL(*mBufferMock.Get(), MapAsyncImpl).WillOnce([&]() -> MaybeError {
        return DAWN_FORMAT_INTERNAL_ERROR(kErrorText);
    });

    // Internal error will cause device loss as well.
    EXPECT_CALL(mDeviceLostCallback,
                Call(_, wgpu::DeviceLostReason::Unknown, testing::HasSubstr(kErrorText)))
        .Times(1);

    MockMapAsyncCallback cb;
    EXPECT_CALL(cb, Call(Eq(wgpu::MapAsyncStatus::Error), Eq(kErrorText)));
    mBuffer.MapAsync(wgpu::MapMode::Write, 0, kBufferSize, wgpu::CallbackMode::AllowSpontaneous,
                     cb.Callback());
}

class BufferBaseThreadedTest : public BufferBaseTest {
  public:
    BufferBaseThreadedTest() : BufferBaseTest() {
        mRequiredFeatures.push_back(wgpu::FeatureName::ImplicitDeviceSynchronization);
    }
};

// Tests calling MapAsync() on one thread and then Unmap() on another thread works.
TEST_F(BufferBaseThreadedTest, MapAsyncThenUnmap) {
    WaitableEvent event;
    EXPECT_CALL(*mBufferMock.Get(), MapAsyncImpl).Times(1);
    EXPECT_CALL(*mBufferMock.Get(), UnmapImpl).Times(1);

    utils::RunInParallel(2, [&](uint32_t index) {
        if (index == 0) {
            MockMapAsyncCallback cb;
            EXPECT_CALL(cb, Call(Eq(wgpu::MapAsyncStatus::Success), _))
                .WillOnce([&event](wgpu::MapAsyncStatus, wgpu::StringView) { event.Signal(); });
            mBuffer.MapAsync(wgpu::MapMode::Write, 0, kBufferSize,
                             wgpu::CallbackMode::AllowSpontaneous, cb.Callback());
        } else {
            event.Wait();
            mBuffer.Unmap();
        }
    });
}

// Tests calling Unmap() on one thread and then Unmap() concurrently on another thread.
// The second Unmap() should throw a validation error and do nothing.
TEST_F(BufferBaseThreadedTest, ConcurrentUnmap) {
    WaitableEvent event;
    EXPECT_CALL(*mBufferMock.Get(), MapAsyncImpl);
    EXPECT_CALL(*mBufferMock.Get(), UnmapImpl).WillOnce([&]() { event.Wait(); });

    EXPECT_CALL(mDeviceErrorCallback,
                Call(_, wgpu::ErrorType::Validation, testing::HasSubstr(kValidationErrorMessage)))
        .Times(1);

    MockMapAsyncCallback cb;
    EXPECT_CALL(cb, Call(Eq(wgpu::MapAsyncStatus::Success), _));
    mBuffer.MapAsync(wgpu::MapMode::Write, 0, kBufferSize, wgpu::CallbackMode::AllowSpontaneous,
                     cb.Callback());

    utils::RunInParallel(2, [&](uint32_t) {
        mBuffer.Unmap();
        event.Signal();
    });
}

// Tests calling Unmap() on one thread and then GetMappedRange() concurrently on another thread.
// GetMappedRange() will return a null since the buffer has already started being unmapped.
TEST_F(BufferBaseThreadedTest, ConcurrentUnmapGetMappedRange) {
    WaitableEvent mappedRangeEvent;
    WaitableEvent unmapEvent;
    EXPECT_CALL(*mBufferMock.Get(), UnmapImpl).WillOnce([&]() {
        mappedRangeEvent.Signal();
        unmapEvent.Wait();
    });

    MockMapAsyncCallback cb;
    EXPECT_CALL(cb, Call(Eq(wgpu::MapAsyncStatus::Success), _));
    mBuffer.MapAsync(wgpu::MapMode::Write, 0, kBufferSize, wgpu::CallbackMode::AllowSpontaneous,
                     cb.Callback());

    utils::RunInParallel(2, [&](uint32_t index) {
        if (index == 0) {
            mBuffer.Unmap();
        } else {
            mappedRangeEvent.Wait();
            EXPECT_EQ(mBuffer.GetMappedRange(), nullptr);
            unmapEvent.Signal();
        }
    });
    ProcessEvents();
}

// Tests calling Unmap() on one thread and then Destroy() concurrently on another thread. Destroy()
// should throw a validation error and wait for unmap to finish before destroying the object.
TEST_F(BufferBaseThreadedTest, ConcurrentUnmapDestroy) {
    WaitableEvent destroyEvent;
    WaitableEvent errorEvent;
    EXPECT_CALL(*mBufferMock.Get(), UnmapImpl).WillOnce([&]() {
        destroyEvent.Signal();
        errorEvent.Wait();
    });
    EXPECT_CALL(mDeviceErrorCallback,
                Call(_, wgpu::ErrorType::Validation, testing::HasSubstr(kValidationErrorMessage)))
        .WillOnce([&]() { errorEvent.Signal(); });

    MockMapAsyncCallback cb;
    EXPECT_CALL(cb, Call(Eq(wgpu::MapAsyncStatus::Success), _));
    mBuffer.MapAsync(wgpu::MapMode::Write, 0, kBufferSize, wgpu::CallbackMode::AllowSpontaneous,
                     cb.Callback());

    utils::RunInParallel(2, [&](uint32_t index) {
        if (index == 0) {
            mBuffer.Unmap();
        } else {
            destroyEvent.Wait();
            mBuffer.Destroy();
        }
    });
}

}  // namespace
}  // namespace dawn::native
