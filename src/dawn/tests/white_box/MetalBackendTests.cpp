// Copyright 2024 The Dawn & Tint Authors
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

#include <atomic>
#include <chrono>
#include <cstdint>
#include <limits>
#include <thread>

#include "dawn/native/DawnNative.h"
#include "dawn/native/MetalBackend.h"
#include "src/dawn/tests/DawnTest.h"

namespace dawn::native {
namespace {

using ::testing::HasSubstr;

class MetalBackendTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    }

    void TearDown() override {
#if defined(DAWN_ENABLE_ERROR_INJECTION)
        // Ensure error injector is reset in TearDown so it doesn't matter if the test exits early.
        DisableErrorInjector();
        ClearErrorInjector();
#endif  // defined(DAWN_ENABLE_ERROR_INJECTION)

        DawnTest::TearDown();
    }
};

// Test WaitForCommandsToBeScheduled() call without any crash.
TEST_P(MetalBackendTests, WaitForCommandsToBeScheduledOK) {
    metal::WaitForCommandsToBeScheduled(device.Get());
}

// Test WaitForCommandsToBeScheduled() won't crash even if the device is destroyed.
TEST_P(MetalBackendTests, WaitForCommandsToBeScheduledOnDestroyedDevice) {
    DestroyDevice(device);
    metal::WaitForCommandsToBeScheduled(device.Get());
}

class MetalSuspendedSubmissionTests : public MetalBackendTests {
  protected:
    static constexpr uint32_t kValue = 0x12345678u;
    static constexpr uint64_t kShortWaitNs = 50'000'000;  // 50 ms
    static constexpr uint64_t kForeverNs = std::numeric_limits<uint64_t>::max();

    void TearDown() override {
        // Never leave the queue held: the device waits for its work to be destroyed.
        metal::SetSubmissionSuspended(device.Get(), false);
        MetalBackendTests::TearDown();
    }

    // A command buffer copying kValue (written to a source buffer and already on the GPU) into
    // `dst`, a buffer of the given usage.
    wgpu::CommandBuffer MakeCopyOfValue(wgpu::Buffer* dst, wgpu::BufferUsage dstUsage) {
        wgpu::BufferDescriptor srcDesc;
        srcDesc.size = 4;
        srcDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        wgpu::Buffer src = device.CreateBuffer(&srcDesc);
        queue.WriteBuffer(src, 0, &kValue, sizeof(kValue));
        WaitForAllOperations();

        wgpu::BufferDescriptor dstDesc;
        dstDesc.size = 4;
        dstDesc.usage = dstUsage;
        *dst = device.CreateBuffer(&dstDesc);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(src, 0, *dst, 0, 4);
        return encoder.Finish();
    }

    wgpu::Future WorkDone() {
        return queue.OnSubmittedWorkDone(
            wgpu::CallbackMode::WaitAnyOnly,
            [](wgpu::QueueWorkDoneStatus status, wgpu::StringView) {
                EXPECT_EQ(status, wgpu::QueueWorkDoneStatus::Success);
            });
    }
};

// Work submitted while the queue is suspended is held, and completes once the queue is resumed.
TEST_P(MetalSuspendedSubmissionTests, SubmissionCompletesOnceResumed) {
    wgpu::Buffer dst;
    wgpu::CommandBuffer commands =
        MakeCopyOfValue(&dst, wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);

    metal::SetSubmissionSuspended(device.Get(), true);
    EXPECT_TRUE(metal::IsSubmissionSuspended(device.Get()));
    queue.Submit(1, &commands);
    wgpu::Future done = WorkDone();
    // Held: nothing completes, and the wait does not deadlock on the flush it asks for.
    EXPECT_EQ(GetInstance().WaitAny(done, kShortWaitNs), wgpu::WaitStatus::TimedOut);

    metal::SetSubmissionSuspended(device.Get(), false);
    EXPECT_FALSE(metal::IsSubmissionSuspended(device.Get()));
    EXPECT_EQ(GetInstance().WaitAny(done, kForeverNs), wgpu::WaitStatus::Success);
    EXPECT_BUFFER_U32_EQ(kValue, dst, 0);
}

// Suspending commits the work pending at that point: it completes while the queue is suspended.
TEST_P(MetalSuspendedSubmissionTests, PendingWorkIsCommittedOnSuspend) {
    wgpu::Buffer dst;
    wgpu::CommandBuffer commands =
        MakeCopyOfValue(&dst, wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);
    queue.Submit(1, &commands);
    wgpu::Future done = WorkDone();

    metal::SetSubmissionSuspended(device.Get(), true);
    EXPECT_EQ(GetInstance().WaitAny(done, kForeverNs), wgpu::WaitStatus::Success);
    metal::SetSubmissionSuspended(device.Get(), false);
    EXPECT_BUFFER_U32_EQ(kValue, dst, 0);
}

// A thread waiting for held work, like a readback or a promise waiter, wakes once the queue is
// resumed from another thread. The wait flushes the queue before sleeping, which only the device
// lock keeps apart from the resume.
TEST_P(MetalSuspendedSubmissionTests, WaiterWakesOnceResumed) {
    DAWN_TEST_UNSUPPORTED_IF(!IsImplicitDeviceSyncEnabled());

    wgpu::Buffer dst;
    wgpu::CommandBuffer commands =
        MakeCopyOfValue(&dst, wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);

    metal::SetSubmissionSuspended(device.Get(), true);
    queue.Submit(1, &commands);
    wgpu::Future done = WorkDone();

    std::atomic<bool> woke = false;
    std::thread waiter([&] {
        EXPECT_EQ(GetInstance().WaitAny(done, kForeverNs), wgpu::WaitStatus::Success);
        woke.store(true);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(woke.load());

    metal::SetSubmissionSuspended(device.Get(), false);
    waiter.join();
    EXPECT_TRUE(woke.load());
    EXPECT_BUFFER_U32_EQ(kValue, dst, 0);
}

// A mapping requested while the queue is suspended resolves, with the held work's result, once
// the queue is resumed.
TEST_P(MetalSuspendedSubmissionTests, MapAsyncResolvesOnceResumed) {
    wgpu::Buffer dst;
    wgpu::CommandBuffer commands =
        MakeCopyOfValue(&dst, wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst);

    metal::SetSubmissionSuspended(device.Get(), true);
    queue.Submit(1, &commands);

    bool mapped = false;
    dst.MapAsync(wgpu::MapMode::Read, 0, 4, wgpu::CallbackMode::AllowProcessEvents,
                 [&](wgpu::MapAsyncStatus status, wgpu::StringView) {
                     EXPECT_EQ(status, wgpu::MapAsyncStatus::Success);
                     mapped = true;
                 });
    for (int i = 0; i < 5 && !mapped; ++i) {
        WaitABit();
    }
    EXPECT_FALSE(mapped);

    metal::SetSubmissionSuspended(device.Get(), false);
    while (!mapped) {
        WaitABit();
    }
    EXPECT_EQ(*static_cast<const uint32_t*>(dst.GetConstMappedRange(0, 4)), kValue);
    dst.Unmap();
}

// Suspending or resuming twice changes nothing, and neither crashes on a destroyed device.
TEST_P(MetalSuspendedSubmissionTests, RepeatedAndOnDestroyedDevice) {
    metal::SetSubmissionSuspended(device.Get(), true);
    metal::SetSubmissionSuspended(device.Get(), true);
    EXPECT_TRUE(metal::IsSubmissionSuspended(device.Get()));
    metal::SetSubmissionSuspended(device.Get(), false);
    metal::SetSubmissionSuspended(device.Get(), false);
    EXPECT_FALSE(metal::IsSubmissionSuspended(device.Get()));

    DestroyDevice(device);
    metal::SetSubmissionSuspended(device.Get(), true);
    EXPECT_TRUE(metal::IsSubmissionSuspended(device.Get()));
    metal::SetSubmissionSuspended(device.Get(), false);
}

#if defined(DAWN_ENABLE_ERROR_INJECTION)

// Test that a command buffer execution failure causes the device to be lost cleanly and
// does not deadlock on destruction.
TEST_P(MetalBackendTests, CommandBufferExecutionErrorTriggersDeviceLost) {
    bool deviceLostCalled = false;
    EXPECT_CALL(mDeviceLostCallback, Call(CHandleIs(device.Get()), wgpu::DeviceLostReason::Unknown,
                                          HasSubstr("Metal command buffer failed")))
        .WillOnce([&](auto, auto, auto) { deviceLostCalled = true; });

    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer src = device.CreateBuffer(&desc);
    wgpu::Buffer dst = device.CreateBuffer(&desc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(src, 0, dst, 0, 4);
    wgpu::CommandBuffer commands = encoder.Finish();

    EnableErrorInjector();
    InjectErrorAt(0u);

    queue.Submit(1, &commands);

    while (!deviceLostCalled) {
        WaitABit();
    }
}

// Test that destroying a device while a failing command buffer is in-flight does not deadlock.
TEST_P(MetalBackendTests, DestroyDeviceWithInFlightExecutionError) {
    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer src = device.CreateBuffer(&desc);
    wgpu::Buffer dst = device.CreateBuffer(&desc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(src, 0, dst, 0, 4);
    wgpu::CommandBuffer commands = encoder.Finish();

    EnableErrorInjector();
    InjectErrorAt(0u);

    queue.Submit(1, &commands);

    // Immediately destroy the device while the command buffer is completing with an error.
    // This verifies WaitForIdleForDestructionImpl / WaitForQueueSerial does not deadlock.
    DestroyDevice(device);
}

// Test that OnSubmittedWorkDone does not hang when a command buffer execution error occurs.
TEST_P(MetalBackendTests, OnSubmittedWorkDoneWithExecutionError) {
    bool deviceLostCalled = false;
    EXPECT_CALL(mDeviceLostCallback, Call(CHandleIs(device.Get()), wgpu::DeviceLostReason::Unknown,
                                          HasSubstr("Metal command buffer failed")))
        .WillOnce([&](auto, auto, auto) { deviceLostCalled = true; });

    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer src = device.CreateBuffer(&desc);
    wgpu::Buffer dst = device.CreateBuffer(&desc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(src, 0, dst, 0, 4);
    wgpu::CommandBuffer commands = encoder.Finish();

    EnableErrorInjector();
    InjectErrorAt(0u);

    queue.Submit(1, &commands);

    bool workDoneCalled = false;
    queue.OnSubmittedWorkDone(
        wgpu::CallbackMode::AllowProcessEvents,
        [](wgpu::QueueWorkDoneStatus, wgpu::StringView, bool* called) { *called = true; },
        &workDoneCalled);

    while (!deviceLostCalled || !workDoneCalled) {
        WaitABit();
    }
    EXPECT_TRUE(deviceLostCalled);
    EXPECT_TRUE(workDoneCalled);
}

// Test that if a command buffer fails, MapAsync succeeds and the mapping is valid.
TEST_P(MetalBackendTests, MapAsyncOnFailedCommandBuffer) {
    bool deviceLostCalled = false;
    EXPECT_CALL(mDeviceLostCallback, Call(CHandleIs(device.Get()), wgpu::DeviceLostReason::Unknown,
                                          HasSubstr("Metal command buffer failed")))
        .WillOnce([&](auto, auto, auto) { deviceLostCalled = true; });

    wgpu::BufferDescriptor dstDesc;
    dstDesc.size = 4;
    dstDesc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer dst = device.CreateBuffer(&dstDesc);

    wgpu::BufferDescriptor srcDesc;
    srcDesc.size = 4;
    srcDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer srcB = device.CreateBuffer(&srcDesc);
    wgpu::Buffer srcC = device.CreateBuffer(&srcDesc);

    uint32_t valueA = 1010;
    queue.WriteBuffer(dst, 0, &valueA, sizeof(valueA));

    uint32_t valueB = 2020;
    queue.WriteBuffer(srcB, 0, &valueB, sizeof(valueB));

    uint32_t valueC = 3030;
    queue.WriteBuffer(srcC, 0, &valueC, sizeof(valueC));

    // Ensure the initial writes complete before injecting the error.
    WaitForAllOperations();

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(srcB, 0, dst, 0, 4);
        wgpu::CommandBuffer commands = encoder.Finish();

        EnableErrorInjector();
        InjectErrorAt(0u);

        // This should set dst to valueB because despite the fake injected error, the Metal command
        // buffer actually succeeded. However, valueB should never be visible to the application
        // because this causes a device loss due to DAWN_UNRECOVERABLE_ERROR.
        queue.Submit(1, &commands);
    }

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(srcC, 0, dst, 0, 4);
        wgpu::CommandBuffer commands = encoder.Finish();

        // This submit should silently no-op because we don't submit new work after device loss.
        queue.Submit(1, &commands);
    }

    bool mapCompleted = false;
    dst.MapAsync(wgpu::MapMode::Read, 0, 4, wgpu::CallbackMode::AllowProcessEvents,
                 [&](wgpu::MapAsyncStatus status, wgpu::StringView) {
                     EXPECT_EQ(status, wgpu::MapAsyncStatus::Aborted);
                     EXPECT_EQ(dst.GetConstMappedRange(0, 4), nullptr);
                     mapCompleted = true;
                 });

    while (!deviceLostCalled || !mapCompleted) {
        WaitABit();
    }
}

#endif  // defined(DAWN_ENABLE_ERROR_INJECTION)

DAWN_INSTANTIATE_TEST(MetalBackendTests, MetalBackend());
DAWN_INSTANTIATE_TEST(MetalSuspendedSubmissionTests, MetalBackend());

}  // anonymous namespace
}  // namespace dawn::native
