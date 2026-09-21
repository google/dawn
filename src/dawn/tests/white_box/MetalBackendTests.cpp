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

#include "dawn/native/DawnNative.h"
#include "dawn/native/MetalBackend.h"
#include "src/dawn/tests/DawnTest.h"

namespace dawn::native {
namespace {

class MetalBackendTests : public DawnTest {
  private:
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

#if defined(DAWN_ENABLE_ERROR_INJECTION)

// Test that a command buffer execution failure causes the device to be lost cleanly and
// does not deadlock on destruction.
TEST_P(MetalBackendTests, CommandBufferExecutionErrorTriggersDeviceLost) {
    bool deviceLostCalled = false;
    EXPECT_CALL(mDeviceLostCallback, Call(CHandleIs(device.Get()), wgpu::DeviceLostReason::Unknown,
                                          testing::HasSubstr("Metal command buffer failed")))
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
                                          testing::HasSubstr("Metal command buffer failed")))
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
                                          testing::HasSubstr("Metal command buffer failed")))
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
        // because this causes a device loss due to DAWN_INTERNAL_ERROR.
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

}  // anonymous namespace
}  // namespace dawn::native
