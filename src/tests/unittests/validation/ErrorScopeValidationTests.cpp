// Copyright 2019 The Dawn Authors
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

#include "tests/MockCallback.h"
#include "tests/unittests/validation/ValidationTest.h"

#include <gmock/gmock.h>

using namespace testing;

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

class ErrorScopeValidationTest : public ValidationTest {
  private:
    void SetUp() override {
        ValidationTest::SetUp();
        mockDevicePopErrorScopeCallback = std::make_unique<MockDevicePopErrorScopeCallback>();
    }

    void TearDown() override {
        ValidationTest::TearDown();

        // Delete mocks so that expectations are checked
        mockDevicePopErrorScopeCallback = nullptr;
    }
};

// Test the simple success case.
TEST_F(ErrorScopeValidationTest, Success) {
    device.PushErrorScope(wgpu::ErrorFilter::Validation);

    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_NoError, _, this)).Times(1);
    device.PopErrorScope(ToMockDevicePopErrorScopeCallback, this);
    FlushWire();
}

// Test the simple case where the error scope catches an error.
TEST_F(ErrorScopeValidationTest, CatchesError) {
    device.PushErrorScope(wgpu::ErrorFilter::Validation);

    wgpu::BufferDescriptor desc = {};
    desc.usage = static_cast<wgpu::BufferUsage>(WGPUBufferUsage_Force32);
    device.CreateBuffer(&desc);

    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_Validation, _, this)).Times(1);
    device.PopErrorScope(ToMockDevicePopErrorScopeCallback, this);
    FlushWire();
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
    FlushWire();

    // Parent validation error scope captures the error.
    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_Validation, _, this + 1))
        .Times(1);
    device.PopErrorScope(ToMockDevicePopErrorScopeCallback, this + 1);
    FlushWire();
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
    FlushWire();

    // Parent scope does not see the error.
    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_NoError, _, this + 1))
        .Times(1);
    device.PopErrorScope(ToMockDevicePopErrorScopeCallback, this + 1);
    FlushWire();
}

// Test that if no error scope handles an error, it goes to the device UncapturedError callback
TEST_F(ErrorScopeValidationTest, UnhandledErrorsMatchUncapturedErrorCallback) {
    device.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);

    wgpu::BufferDescriptor desc = {};
    desc.usage = static_cast<wgpu::BufferUsage>(WGPUBufferUsage_Force32);
    ASSERT_DEVICE_ERROR(device.CreateBuffer(&desc));

    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_NoError, _, this)).Times(1);
    device.PopErrorScope(ToMockDevicePopErrorScopeCallback, this);
    FlushWire();
}

// Check that push/popping error scopes must be balanced.
TEST_F(ErrorScopeValidationTest, PushPopBalanced) {
    // No error scopes to pop.
    { EXPECT_FALSE(device.PopErrorScope(ToMockDevicePopErrorScopeCallback, this)); }

    // Too many pops
    {
        device.PushErrorScope(wgpu::ErrorFilter::Validation);

        EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_NoError, _, this + 1))
            .Times(1);
        device.PopErrorScope(ToMockDevicePopErrorScopeCallback, this + 1);
        FlushWire();

        EXPECT_FALSE(device.PopErrorScope(ToMockDevicePopErrorScopeCallback, this + 2));
    }
}

// Test that error scopes call their callbacks before an enclosed Queue::Submit
// completes
TEST_F(ErrorScopeValidationTest, EnclosedQueueSubmit) {
    wgpu::Queue queue = device.GetQueue();

    device.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);

    queue.Submit(0, nullptr);
    wgpu::Fence fence = queue.CreateFence();
    queue.Signal(fence, 1);

    testing::Sequence seq;

    MockCallback<WGPUFenceOnCompletionCallback> fenceCallback;
    fence.OnCompletion(1, fenceCallback.Callback(), fenceCallback.MakeUserdata(this));

    MockCallback<WGPUErrorCallback> errorScopeCallback;
    EXPECT_CALL(errorScopeCallback, Call(WGPUErrorType_NoError, _, this + 1)).InSequence(seq);
    device.PopErrorScope(errorScopeCallback.Callback(), errorScopeCallback.MakeUserdata(this + 1));

    EXPECT_CALL(fenceCallback, Call(WGPUFenceCompletionStatus_Success, this)).InSequence(seq);
    WaitForAllOperations(device);
}

// Test that parent error scopes also call their callbacks before an enclosed Queue::Submit
// completes
TEST_F(ErrorScopeValidationTest, EnclosedQueueSubmitNested) {
    wgpu::Queue queue = device.GetQueue();

    device.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);
    device.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);

    queue.Submit(0, nullptr);
    wgpu::Fence fence = queue.CreateFence();
    queue.Signal(fence, 1);

    testing::Sequence seq;

    MockCallback<WGPUFenceOnCompletionCallback> fenceCallback;
    fence.OnCompletion(1, fenceCallback.Callback(), fenceCallback.MakeUserdata(this));

    MockCallback<WGPUErrorCallback> errorScopeCallback2;
    EXPECT_CALL(errorScopeCallback2, Call(WGPUErrorType_NoError, _, this + 1)).InSequence(seq);
    device.PopErrorScope(errorScopeCallback2.Callback(),
                         errorScopeCallback2.MakeUserdata(this + 1));

    MockCallback<WGPUErrorCallback> errorScopeCallback1;
    EXPECT_CALL(errorScopeCallback1, Call(WGPUErrorType_NoError, _, this + 2)).InSequence(seq);
    device.PopErrorScope(errorScopeCallback1.Callback(),
                         errorScopeCallback1.MakeUserdata(this + 2));

    EXPECT_CALL(fenceCallback, Call(WGPUFenceCompletionStatus_Success, this)).InSequence(seq);
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
        device = nullptr;
    } else {
        EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_NoError, _, this))
            .Times(1);
        device.PopErrorScope(ToMockDevicePopErrorScopeCallback, this);

        device = nullptr;
    }
}

// Regression test that on device shutdown, we don't get a recursion in O(pushed error scope) that
// would lead to a stack overflow
TEST_F(ErrorScopeValidationTest, ShutdownStackOverflow) {
    for (size_t i = 0; i < 1'000'000; i++) {
        device.PushErrorScope(wgpu::ErrorFilter::Validation);
    }
}
