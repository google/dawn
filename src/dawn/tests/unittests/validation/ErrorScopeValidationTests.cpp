// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/tests/MockCallback.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "gmock/gmock.h"

using testing::_;
using testing::MockCallback;
using testing::Sequence;

class MockDevicePopErrorScopeCallback {
  public:
    MOCK_METHOD(void, Call, (WGPUErrorType type, const char* message, void* userdata));
};

static std::unique_ptr<MockDevicePopErrorScopeCallback> mockDevicePopErrorScopeCallback;
static void ToMockDevicePopErrorScopeCallback(WGPUErrorType type,
                                              const char* message,
                                              void* userdata) {
    mockDevicePopErrorScopeCallback->Call(type, message, userdata);
}

class MockQueueWorkDoneCallback {
  public:
    MOCK_METHOD(void, Call, (WGPUQueueWorkDoneStatus status, void* userdata));
};

static std::unique_ptr<MockQueueWorkDoneCallback> mockQueueWorkDoneCallback;
static void ToMockQueueWorkDone(WGPUQueueWorkDoneStatus status, void* userdata) {
    mockQueueWorkDoneCallback->Call(status, userdata);
}

class ErrorScopeValidationTest : public ValidationTest {
  protected:
    void FlushWireAndTick() {
        FlushWire();
        device.Tick();
    }

  private:
    void SetUp() override {
        ValidationTest::SetUp();
        mockDevicePopErrorScopeCallback = std::make_unique<MockDevicePopErrorScopeCallback>();
        mockQueueWorkDoneCallback = std::make_unique<MockQueueWorkDoneCallback>();
    }

    void TearDown() override {
        ValidationTest::TearDown();

        // Delete mocks so that expectations are checked
        mockDevicePopErrorScopeCallback = nullptr;
        mockQueueWorkDoneCallback = nullptr;
    }
};

// Test the simple success case.
TEST_F(ErrorScopeValidationTest, Success) {
    device.PushErrorScope(wgpu::ErrorFilter::Validation);

    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_NoError, _, this)).Times(1);
    device.PopErrorScope(ToMockDevicePopErrorScopeCallback, this);
    FlushWireAndTick();
}

// Test the simple case where the error scope catches an error.
TEST_F(ErrorScopeValidationTest, CatchesError) {
    device.PushErrorScope(wgpu::ErrorFilter::Validation);

    wgpu::BufferDescriptor desc = {};
    desc.usage = static_cast<wgpu::BufferUsage>(WGPUBufferUsage_Force32);
    device.CreateBuffer(&desc);

    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_Validation, _, this)).Times(1);
    device.PopErrorScope(ToMockDevicePopErrorScopeCallback, this);
    FlushWireAndTick();
}

// Test that errors bubble to the parent scope if not handled by the current scope.
TEST_F(ErrorScopeValidationTest, ErrorBubbles) {
    device.PushErrorScope(wgpu::ErrorFilter::Validation);
    device.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);

    wgpu::BufferDescriptor desc = {};
    desc.usage = static_cast<wgpu::BufferUsage>(WGPUBufferUsage_Force32);
    device.CreateBuffer(&desc);

    // OutOfMemory does not match Validation error.
    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_NoError, _, this)).Times(1);
    device.PopErrorScope(ToMockDevicePopErrorScopeCallback, this);
    FlushWireAndTick();

    // Parent validation error scope captures the error.
    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_Validation, _, this + 1))
        .Times(1);
    device.PopErrorScope(ToMockDevicePopErrorScopeCallback, this + 1);
    FlushWireAndTick();
}

// Test that if an error scope matches an error, it does not bubble to the parent scope.
TEST_F(ErrorScopeValidationTest, HandledErrorsStopBubbling) {
    device.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);
    device.PushErrorScope(wgpu::ErrorFilter::Validation);

    wgpu::BufferDescriptor desc = {};
    desc.usage = static_cast<wgpu::BufferUsage>(WGPUBufferUsage_Force32);
    device.CreateBuffer(&desc);

    // Inner scope catches the error.
    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_Validation, _, this)).Times(1);
    device.PopErrorScope(ToMockDevicePopErrorScopeCallback, this);
    FlushWireAndTick();

    // Parent scope does not see the error.
    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_NoError, _, this + 1))
        .Times(1);
    device.PopErrorScope(ToMockDevicePopErrorScopeCallback, this + 1);
    FlushWireAndTick();
}

// Test that if no error scope handles an error, it goes to the device UncapturedError callback
TEST_F(ErrorScopeValidationTest, UnhandledErrorsMatchUncapturedErrorCallback) {
    device.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);

    wgpu::BufferDescriptor desc = {};
    desc.usage = static_cast<wgpu::BufferUsage>(WGPUBufferUsage_Force32);
    ASSERT_DEVICE_ERROR(device.CreateBuffer(&desc));

    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_NoError, _, this)).Times(1);
    device.PopErrorScope(ToMockDevicePopErrorScopeCallback, this);
    FlushWireAndTick();
}

// Check that push/popping error scopes must be balanced.
TEST_F(ErrorScopeValidationTest, PushPopBalanced) {
    // No error scopes to pop.
    {
        EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_Unknown, _, this))
            .Times(1);
        device.PopErrorScope(ToMockDevicePopErrorScopeCallback, this);
        FlushWireAndTick();
    }
    // Too many pops
    {
        device.PushErrorScope(wgpu::ErrorFilter::Validation);

        EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_NoError, _, this + 1))
            .Times(1);
        device.PopErrorScope(ToMockDevicePopErrorScopeCallback, this + 1);
        FlushWireAndTick();

        EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_Unknown, _, this + 2))
            .Times(1);
        device.PopErrorScope(ToMockDevicePopErrorScopeCallback, this + 2);
        FlushWireAndTick();
    }
}

// Test that parent error scopes also call their callbacks before an enclosed Queue::Submit
// completes
TEST_F(ErrorScopeValidationTest, EnclosedQueueSubmitNested) {
    wgpu::Queue queue = device.GetQueue();

    device.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);
    device.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);

    queue.Submit(0, nullptr);
    queue.OnSubmittedWorkDone(ToMockQueueWorkDone, this);

    Sequence seq;

    MockCallback<WGPUErrorCallback> errorScopeCallback2;
    EXPECT_CALL(errorScopeCallback2, Call(WGPUErrorType_NoError, _, this + 1)).InSequence(seq);
    device.PopErrorScope(errorScopeCallback2.Callback(),
                         errorScopeCallback2.MakeUserdata(this + 1));

    MockCallback<WGPUErrorCallback> errorScopeCallback1;
    EXPECT_CALL(errorScopeCallback1, Call(WGPUErrorType_NoError, _, this + 2)).InSequence(seq);
    device.PopErrorScope(errorScopeCallback1.Callback(),
                         errorScopeCallback1.MakeUserdata(this + 2));

    EXPECT_CALL(*mockQueueWorkDoneCallback, Call(WGPUQueueWorkDoneStatus_Success, this))
        .InSequence(seq);
    WaitForAllOperations(device);
}

// Test that if the device is destroyed before the callback occurs, it is called with NoError
// in dawn_native, but Unknown in dawn_wire because the device is destroyed before the callback
// message happens.
TEST_F(ErrorScopeValidationTest, DeviceDestroyedBeforeCallback) {
    device.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);
    {
        // Note: this is in its own scope to be clear the queue does not outlive the device.
        wgpu::Queue queue = device.GetQueue();
        queue.Submit(0, nullptr);
    }

    if (UsesWire()) {
        device.PopErrorScope(ToMockDevicePopErrorScopeCallback, this);

        EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_Unknown, _, this))
            .Times(1);
        ExpectDeviceDestruction();
        device = nullptr;
    } else {
        EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_NoError, _, this))
            .Times(1);
        device.PopErrorScope(ToMockDevicePopErrorScopeCallback, this);
        ExpectDeviceDestruction();
        device = nullptr;
    }
}

// If the device is destroyed, pop error scope should callback with device lost.
TEST_F(ErrorScopeValidationTest, DeviceDestroyedBeforePop) {
    device.PushErrorScope(wgpu::ErrorFilter::Validation);
    ExpectDeviceDestruction();
    device.Destroy();
    FlushWireAndTick();

    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_DeviceLost, _, this)).Times(1);
    device.PopErrorScope(ToMockDevicePopErrorScopeCallback, this);
    FlushWireAndTick();
}

// Regression test that on device shutdown, we don't get a recursion in O(pushed error scope) that
// would lead to a stack overflow
TEST_F(ErrorScopeValidationTest, ShutdownStackOverflow) {
    for (size_t i = 0; i < 1'000'000; i++) {
        device.PushErrorScope(wgpu::ErrorFilter::Validation);
    }
}
