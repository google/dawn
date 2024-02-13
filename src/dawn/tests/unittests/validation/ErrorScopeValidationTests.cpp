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
#include <vector>

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
    void FlushWireAndProcessEvents() {
        FlushWire();
        instance.ProcessEvents();
    }

    // Generate new void* pointer for use as `userdata` in callback. The pointer
    // is valid for the whole duration of the test. This avoids dangling
    // pointers checks to trigger.
    void* CreateUserData() {
        mUserData.push_back(std::make_unique<bool>());
        return reinterpret_cast<void*>(mUserData.back().get());
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

    std::vector<std::unique_ptr<bool>> mUserData;
};

// Test the simple success case.
TEST_F(ErrorScopeValidationTest, Success) {
    device.PushErrorScope(wgpu::ErrorFilter::Validation);

    void* userdata = CreateUserData();
    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_NoError, _, userdata))
        .Times(1);
    device.PopErrorScope(ToMockDevicePopErrorScopeCallback, userdata);
    FlushWireAndProcessEvents();
}

// Test the simple case where the error scope catches an error.
TEST_F(ErrorScopeValidationTest, CatchesError) {
    device.PushErrorScope(wgpu::ErrorFilter::Validation);

    wgpu::BufferDescriptor desc = {};
    desc.usage = static_cast<wgpu::BufferUsage>(WGPUBufferUsage_Force32);
    device.CreateBuffer(&desc);

    void* userdata = CreateUserData();
    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_Validation, _, userdata))
        .Times(1);
    device.PopErrorScope(ToMockDevicePopErrorScopeCallback, userdata);
    FlushWireAndProcessEvents();
}

// Test that errors bubble to the parent scope if not handled by the current scope.
TEST_F(ErrorScopeValidationTest, ErrorBubbles) {
    device.PushErrorScope(wgpu::ErrorFilter::Validation);
    device.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);

    wgpu::BufferDescriptor desc = {};
    desc.usage = static_cast<wgpu::BufferUsage>(WGPUBufferUsage_Force32);
    device.CreateBuffer(&desc);

    // OutOfMemory does not match Validation error.
    void* userdata_1 = CreateUserData();
    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_NoError, _, userdata_1))
        .Times(1);
    device.PopErrorScope(ToMockDevicePopErrorScopeCallback, userdata_1);
    FlushWireAndProcessEvents();

    // Parent validation error scope captures the error.
    void* userdata_2 = CreateUserData();
    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_Validation, _, userdata_2))
        .Times(1);
    device.PopErrorScope(ToMockDevicePopErrorScopeCallback, userdata_2);
    FlushWireAndProcessEvents();
}

// Test that if an error scope matches an error, it does not bubble to the parent scope.
TEST_F(ErrorScopeValidationTest, HandledErrorsStopBubbling) {
    device.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);
    device.PushErrorScope(wgpu::ErrorFilter::Validation);

    wgpu::BufferDescriptor desc = {};
    desc.usage = static_cast<wgpu::BufferUsage>(WGPUBufferUsage_Force32);
    device.CreateBuffer(&desc);

    // Inner scope catches the error.
    void* userdata_1 = CreateUserData();
    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_Validation, _, userdata_1))
        .Times(1);
    device.PopErrorScope(ToMockDevicePopErrorScopeCallback, userdata_1);
    FlushWireAndProcessEvents();

    // Parent scope does not see the error.
    void* userdata_2 = CreateUserData();
    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_NoError, _, userdata_2))
        .Times(1);
    device.PopErrorScope(ToMockDevicePopErrorScopeCallback, userdata_2);
    FlushWireAndProcessEvents();
}

// Test that if no error scope handles an error, it goes to the device UncapturedError callback
TEST_F(ErrorScopeValidationTest, UnhandledErrorsMatchUncapturedErrorCallback) {
    device.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);

    wgpu::BufferDescriptor desc = {};
    desc.usage = static_cast<wgpu::BufferUsage>(WGPUBufferUsage_Force32);
    ASSERT_DEVICE_ERROR(device.CreateBuffer(&desc));

    void* userdata = CreateUserData();
    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_NoError, _, userdata))
        .Times(1);
    device.PopErrorScope(ToMockDevicePopErrorScopeCallback, userdata);
    FlushWireAndProcessEvents();
}

// Check that push/popping error scopes must be balanced.
TEST_F(ErrorScopeValidationTest, PushPopBalanced) {
    // No error scopes to pop.
    {
        void* userdata = CreateUserData();
        EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_Unknown, _, userdata))
            .Times(1);
        device.PopErrorScope(ToMockDevicePopErrorScopeCallback, userdata);
        FlushWireAndProcessEvents();
    }
    // Too many pops
    {
        device.PushErrorScope(wgpu::ErrorFilter::Validation);

        void* userdata_1 = CreateUserData();
        EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_NoError, _, userdata_1))
            .Times(1);
        device.PopErrorScope(ToMockDevicePopErrorScopeCallback, userdata_1);
        FlushWireAndProcessEvents();

        void* userdata_2 = CreateUserData();
        EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_Unknown, _, userdata_2))
            .Times(1);
        device.PopErrorScope(ToMockDevicePopErrorScopeCallback, userdata_2);
        FlushWireAndProcessEvents();
    }
}

// Test that parent error scopes also call their callbacks before an enclosed Queue::Submit
// completes
TEST_F(ErrorScopeValidationTest, EnclosedQueueSubmitNested) {
    wgpu::Queue queue = device.GetQueue();

    device.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);
    device.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);

    void* userdata_1 = CreateUserData();
    queue.Submit(0, nullptr);
    queue.OnSubmittedWorkDone(ToMockQueueWorkDone, userdata_1);

    Sequence seq;

    MockCallback<WGPUErrorCallback> errorScopeCallback2;
    void* userdata_2 = CreateUserData();
    EXPECT_CALL(errorScopeCallback2, Call(WGPUErrorType_NoError, _, userdata_2)).InSequence(seq);
    device.PopErrorScope(errorScopeCallback2.Callback(),
                         errorScopeCallback2.MakeUserdata(userdata_2));

    MockCallback<WGPUErrorCallback> errorScopeCallback1;
    void* userdata_3 = CreateUserData();
    EXPECT_CALL(errorScopeCallback1, Call(WGPUErrorType_NoError, _, userdata_3)).InSequence(seq);
    device.PopErrorScope(errorScopeCallback1.Callback(),
                         errorScopeCallback1.MakeUserdata(userdata_3));

    EXPECT_CALL(*mockQueueWorkDoneCallback, Call(WGPUQueueWorkDoneStatus_Success, userdata_1));
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

    void* userdata = CreateUserData();
    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_NoError, _, userdata))
        .Times(1);
    device.PopErrorScope(ToMockDevicePopErrorScopeCallback, userdata);
    ExpectDeviceDestruction();
    device = nullptr;

    FlushWireAndProcessEvents();
}

// If the device is destroyed, pop error scope should callback with device lost.
TEST_F(ErrorScopeValidationTest, DeviceDestroyedBeforePop) {
    device.PushErrorScope(wgpu::ErrorFilter::Validation);
    ExpectDeviceDestruction();
    device.Destroy();
    FlushWireAndProcessEvents();

    void* userdata = CreateUserData();
    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(WGPUErrorType_DeviceLost, _, userdata))
        .Times(1);
    device.PopErrorScope(ToMockDevicePopErrorScopeCallback, userdata);
    FlushWireAndProcessEvents();
}

// Regression test that on device shutdown, we don't get a recursion in O(pushed error scope) that
// would lead to a stack overflow
TEST_F(ErrorScopeValidationTest, ShutdownStackOverflow) {
    for (size_t i = 0; i < 1'000'000; i++) {
        device.PushErrorScope(wgpu::ErrorFilter::Validation);
    }
}
