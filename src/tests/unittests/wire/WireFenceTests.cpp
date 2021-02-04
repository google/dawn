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

#include "tests/unittests/wire/WireTest.h"

#include "dawn_wire/WireClient.h"

using namespace testing;
using namespace dawn_wire;

namespace {

    class MockFenceOnCompletionCallback {
      public:
        MOCK_METHOD(void, Call, (WGPUFenceCompletionStatus status, void* userdata));
    };

    std::unique_ptr<StrictMock<MockFenceOnCompletionCallback>> mockFenceOnCompletionCallback;
    void ToMockFenceOnCompletion(WGPUFenceCompletionStatus status, void* userdata) {
        mockFenceOnCompletionCallback->Call(status, userdata);
    }

}  // anonymous namespace

class WireFenceTests : public WireTest {
  public:
    WireFenceTests() {
    }
    ~WireFenceTests() override = default;

    void SetUp() override {
        WireTest::SetUp();

        mockFenceOnCompletionCallback =
            std::make_unique<StrictMock<MockFenceOnCompletionCallback>>();

        {
            WGPUFenceDescriptor descriptor = {};
            descriptor.initialValue = 1;

            apiFence = api.GetNewFence();
            fence = wgpuQueueCreateFence(queue, &descriptor);

            EXPECT_CALL(api, QueueCreateFence(apiQueue, _)).WillOnce(Return(apiFence));
            FlushClient();
        }
    }

    void TearDown() override {
        WireTest::TearDown();

        mockFenceOnCompletionCallback = nullptr;
    }

    void FlushServer() {
        WireTest::FlushServer();

        Mock::VerifyAndClearExpectations(&mockFenceOnCompletionCallback);
    }

  protected:
    void DoQueueSignal(uint64_t signalValue,
                       WGPUFenceCompletionStatus status = WGPUFenceCompletionStatus_Success) {
        wgpuQueueSignal(queue, fence, signalValue);
        EXPECT_CALL(api, QueueSignal(apiQueue, apiFence, signalValue)).Times(1);

        // This callback is generated to update the completedValue of the fence
        // on the client
        EXPECT_CALL(api, OnFenceOnCompletion(apiFence, signalValue, _, _))
            .WillOnce(
                InvokeWithoutArgs([=]() { api.CallFenceOnCompletionCallback(apiFence, status); }))
            .RetiresOnSaturation();
    }

    // A successfully created fence
    WGPUFence fence;
    WGPUFence apiFence;
};

// Check that signaling a fence succeeds
TEST_F(WireFenceTests, QueueSignalSuccess) {
    DoQueueSignal(2u);
    FlushClient();
    FlushServer();

    EXPECT_EQ(wgpuFenceGetCompletedValue(fence), 2u);
}

// Check that signaling a fence twice succeeds
TEST_F(WireFenceTests, QueueSignalIncreasing) {
    DoQueueSignal(2u);
    DoQueueSignal(3u);
    FlushClient();
    FlushServer();

    EXPECT_EQ(wgpuFenceGetCompletedValue(fence), 3u);
}

// Check that an error in queue signal does not update the completed value.
TEST_F(WireFenceTests, QueueSignalValidationError) {
    DoQueueSignal(2u);
    DoQueueSignal(1u, WGPUFenceCompletionStatus_Error);
    FlushClient();
    FlushServer();

    // Value should stay at 2 and not be updated to 1.
    EXPECT_EQ(wgpuFenceGetCompletedValue(fence), 2u);
}

// Check that a success in the on completion callback is forwarded to the client.
TEST_F(WireFenceTests, OnCompletionSuccess) {
    wgpuFenceOnCompletion(fence, 0, ToMockFenceOnCompletion, nullptr);
    EXPECT_CALL(api, OnFenceOnCompletion(apiFence, 0u, _, _)).WillOnce(InvokeWithoutArgs([&]() {
        api.CallFenceOnCompletionCallback(apiFence, WGPUFenceCompletionStatus_Success);
    }));
    FlushClient();

    EXPECT_CALL(*mockFenceOnCompletionCallback, Call(WGPUFenceCompletionStatus_Success, _))
        .Times(1);
    FlushServer();
}

// Check that an error in the on completion callback is forwarded to the client.
TEST_F(WireFenceTests, OnCompletionError) {
    wgpuFenceOnCompletion(fence, 0, ToMockFenceOnCompletion, nullptr);
    EXPECT_CALL(api, OnFenceOnCompletion(apiFence, 0u, _, _)).WillOnce(InvokeWithoutArgs([&]() {
        api.CallFenceOnCompletionCallback(apiFence, WGPUFenceCompletionStatus_Error);
    }));
    FlushClient();

    EXPECT_CALL(*mockFenceOnCompletionCallback, Call(WGPUFenceCompletionStatus_Error, _)).Times(1);
    FlushServer();
}

// Test that registering a callback then wire disconnect calls the callback with
// DeviceLost.
TEST_F(WireFenceTests, OnCompletionThenDisconnect) {
    wgpuFenceOnCompletion(fence, 0, ToMockFenceOnCompletion, this);
    EXPECT_CALL(api, OnFenceOnCompletion(apiFence, 0u, _, _)).WillOnce(InvokeWithoutArgs([&]() {
        api.CallFenceOnCompletionCallback(apiFence, WGPUFenceCompletionStatus_Success);
    }));
    FlushClient();

    EXPECT_CALL(*mockFenceOnCompletionCallback, Call(WGPUFenceCompletionStatus_DeviceLost, this))
        .Times(1);
    GetWireClient()->Disconnect();
}

// Test that registering a callback after wire disconnect calls the callback with
// DeviceLost.
TEST_F(WireFenceTests, OnCompletionAfterDisconnect) {
    GetWireClient()->Disconnect();

    EXPECT_CALL(*mockFenceOnCompletionCallback, Call(WGPUFenceCompletionStatus_DeviceLost, this))
        .Times(1);
    wgpuFenceOnCompletion(fence, 0, ToMockFenceOnCompletion, this);
}

// Without any flushes, it is valid to wait on a value less than or equal to
// the last signaled value
TEST_F(WireFenceTests, OnCompletionSynchronousValidationSuccess) {
    wgpuQueueSignal(queue, fence, 4u);
    wgpuFenceOnCompletion(fence, 2u, ToMockFenceOnCompletion, 0);
    wgpuFenceOnCompletion(fence, 3u, ToMockFenceOnCompletion, 0);
    wgpuFenceOnCompletion(fence, 4u, ToMockFenceOnCompletion, 0);

    EXPECT_CALL(*mockFenceOnCompletionCallback, Call(WGPUFenceCompletionStatus_Unknown, _))
        .Times(3);
}

// Check that the fence completed value is initialized
TEST_F(WireFenceTests, GetCompletedValueInitialization) {
    EXPECT_EQ(wgpuFenceGetCompletedValue(fence), 1u);
}

// Check that the fence completed value updates after signaling the fence
TEST_F(WireFenceTests, GetCompletedValueUpdate) {
    DoQueueSignal(3u);
    FlushClient();
    FlushServer();

    EXPECT_EQ(wgpuFenceGetCompletedValue(fence), 3u);
}

// Check that the fence completed value updates after signaling the fence
TEST_F(WireFenceTests, GetCompletedValueUpdateInCallback) {
    // Signal the fence
    DoQueueSignal(3u);

    // Register the callback
    wgpuFenceOnCompletion(fence, 3u, ToMockFenceOnCompletion, this);
    EXPECT_CALL(api, OnFenceOnCompletion(apiFence, 3u, _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallFenceOnCompletionCallback(apiFence, WGPUFenceCompletionStatus_Success);
        }))
        .RetiresOnSaturation();

    FlushClient();

    EXPECT_CALL(*mockFenceOnCompletionCallback, Call(WGPUFenceCompletionStatus_Success, this))
        .WillOnce(InvokeWithoutArgs([&]() { EXPECT_EQ(wgpuFenceGetCompletedValue(fence), 3u); }));
    FlushServer();
}

// Check that the fence completed value does not update without a flush
TEST_F(WireFenceTests, GetCompletedValueNoUpdate) {
    wgpuQueueSignal(queue, fence, 3u);
    EXPECT_EQ(wgpuFenceGetCompletedValue(fence), 1u);
}

// Check that the callback is called with UNKNOWN when the fence is destroyed
// before the completed value is updated
TEST_F(WireFenceTests, DestroyBeforeOnCompletionEnd) {
    wgpuQueueSignal(queue, fence, 3u);
    wgpuFenceOnCompletion(fence, 2u, ToMockFenceOnCompletion, nullptr);
    EXPECT_CALL(*mockFenceOnCompletionCallback, Call(WGPUFenceCompletionStatus_Unknown, _))
        .Times(1);
}

// Test that signaling a fence on a wrong queue is invalid
// DISABLED until we have support for multiple queues.
TEST_F(WireFenceTests, DISABLED_SignalWrongQueue) {
    WGPUQueue queue2 = wgpuDeviceGetQueue(device);
    WGPUQueue apiQueue2 = api.GetNewQueue();
    EXPECT_CALL(api, DeviceGetQueue(apiDevice)).WillOnce(Return(apiQueue2));
    FlushClient();

    wgpuQueueSignal(queue2, fence, 2u);  // error
    EXPECT_CALL(api, DeviceInjectError(apiDevice, WGPUErrorType_Validation, ValidStringMessage()))
        .Times(1);
    FlushClient();
}

// Test that signaling a fence on a wrong queue does not update fence signaled value
// DISABLED until we have support for multiple queues.
TEST_F(WireFenceTests, DISABLED_SignalWrongQueueDoesNotUpdateValue) {
    WGPUQueue queue2 = wgpuDeviceGetQueue(device);
    WGPUQueue apiQueue2 = api.GetNewQueue();
    EXPECT_CALL(api, DeviceGetQueue(apiDevice)).WillOnce(Return(apiQueue2));
    FlushClient();

    wgpuQueueSignal(queue2, fence, 2u);  // error
    EXPECT_CALL(api, DeviceInjectError(apiDevice, WGPUErrorType_Validation, ValidStringMessage()))
        .Times(1);
    FlushClient();

    // Fence value should be unchanged.
    FlushClient();
    FlushServer();
    EXPECT_EQ(wgpuFenceGetCompletedValue(fence), 1u);

    // Signaling with 2 on the correct queue should succeed
    DoQueueSignal(2u);  // success
    FlushClient();
    FlushServer();
    EXPECT_EQ(wgpuFenceGetCompletedValue(fence), 2u);
}
