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

using namespace testing;
using namespace dawn_wire;

namespace {

    // Mock classes to add expectations on the wire calling callbacks
    class MockDeviceErrorCallback {
      public:
        MOCK_METHOD2(Call, void(const char* message, DawnCallbackUserdata userdata));
    };

    std::unique_ptr<StrictMock<MockDeviceErrorCallback>> mockDeviceErrorCallback;
    void ToMockDeviceErrorCallback(const char* message, DawnCallbackUserdata userdata) {
        mockDeviceErrorCallback->Call(message, userdata);
    }

    class MockFenceOnCompletionCallback {
      public:
        MOCK_METHOD2(Call, void(DawnFenceCompletionStatus status, DawnCallbackUserdata userdata));
    };

    std::unique_ptr<StrictMock<MockFenceOnCompletionCallback>> mockFenceOnCompletionCallback;
    void ToMockFenceOnCompletionCallback(DawnFenceCompletionStatus status,
                                         DawnCallbackUserdata userdata) {
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

        mockDeviceErrorCallback = std::make_unique<StrictMock<MockDeviceErrorCallback>>();
        mockFenceOnCompletionCallback =
            std::make_unique<StrictMock<MockFenceOnCompletionCallback>>();

        {
            queue = dawnDeviceCreateQueue(device);
            apiQueue = api.GetNewQueue();
            EXPECT_CALL(api, DeviceCreateQueue(apiDevice)).WillOnce(Return(apiQueue));
            FlushClient();
        }
        {
            DawnFenceDescriptor descriptor;
            descriptor.initialValue = 1;
            descriptor.nextInChain = nullptr;

            apiFence = api.GetNewFence();
            fence = dawnQueueCreateFence(queue, &descriptor);

            EXPECT_CALL(api, QueueCreateFence(apiQueue, _)).WillOnce(Return(apiFence));
            FlushClient();
        }
    }

    void TearDown() override {
        WireTest::TearDown();

        mockDeviceErrorCallback = nullptr;
        mockFenceOnCompletionCallback = nullptr;
    }

    void FlushServer() {
        WireTest::FlushServer();

        Mock::VerifyAndClearExpectations(&mockDeviceErrorCallback);
        Mock::VerifyAndClearExpectations(&mockFenceOnCompletionCallback);
    }

  protected:
    void DoQueueSignal(uint64_t signalValue) {
        dawnQueueSignal(queue, fence, signalValue);
        EXPECT_CALL(api, QueueSignal(apiQueue, apiFence, signalValue)).Times(1);

        // This callback is generated to update the completedValue of the fence
        // on the client
        EXPECT_CALL(api, OnFenceOnCompletionCallback(apiFence, signalValue, _, _))
            .WillOnce(InvokeWithoutArgs([&]() {
                api.CallFenceOnCompletionCallback(apiFence, DAWN_FENCE_COMPLETION_STATUS_SUCCESS);
            }));
    }

    // A successfully created fence
    DawnFence fence;
    DawnFence apiFence;

    DawnQueue queue;
    DawnQueue apiQueue;
};

// Check that signaling a fence succeeds
TEST_F(WireFenceTests, QueueSignalSuccess) {
    DoQueueSignal(2u);
    DoQueueSignal(3u);
    FlushClient();
    FlushServer();
}

// Without any flushes, it is valid to signal a value greater than the current
// signaled value
TEST_F(WireFenceTests, QueueSignalSynchronousValidationSuccess) {
    DawnCallbackUserdata userdata = 9157;
    dawnDeviceSetErrorCallback(device, ToMockDeviceErrorCallback, userdata);
    EXPECT_CALL(*mockDeviceErrorCallback, Call(_, userdata)).Times(0);

    dawnQueueSignal(queue, fence, 2u);
    dawnQueueSignal(queue, fence, 4u);
    dawnQueueSignal(queue, fence, 5u);
}

// Without any flushes, errors should be generated when signaling a value less
// than or equal to the current signaled value
TEST_F(WireFenceTests, QueueSignalSynchronousValidationError) {
    DawnCallbackUserdata userdata = 3157;
    dawnDeviceSetErrorCallback(device, ToMockDeviceErrorCallback, userdata);

    EXPECT_CALL(*mockDeviceErrorCallback, Call(_, userdata)).Times(1);
    dawnQueueSignal(queue, fence, 0u);  // Error
    EXPECT_TRUE(Mock::VerifyAndClear(mockDeviceErrorCallback.get()));

    EXPECT_CALL(*mockDeviceErrorCallback, Call(_, userdata)).Times(1);
    dawnQueueSignal(queue, fence, 1u);  // Error
    EXPECT_TRUE(Mock::VerifyAndClear(mockDeviceErrorCallback.get()));

    EXPECT_CALL(*mockDeviceErrorCallback, Call(_, userdata)).Times(0);
    dawnQueueSignal(queue, fence, 4u);  // Success
    EXPECT_TRUE(Mock::VerifyAndClear(mockDeviceErrorCallback.get()));

    EXPECT_CALL(*mockDeviceErrorCallback, Call(_, userdata)).Times(1);
    dawnQueueSignal(queue, fence, 3u);  // Error
    EXPECT_TRUE(Mock::VerifyAndClear(mockDeviceErrorCallback.get()));
}

// Check that callbacks are immediately called if the fence is already finished
TEST_F(WireFenceTests, OnCompletionImmediate) {
    // Can call on value < (initial) signaled value happens immediately
    {
        DawnCallbackUserdata userdata = 9847;
        EXPECT_CALL(*mockFenceOnCompletionCallback,
                    Call(DAWN_FENCE_COMPLETION_STATUS_SUCCESS, userdata))
            .Times(1);
        dawnFenceOnCompletion(fence, 0, ToMockFenceOnCompletionCallback, userdata);
    }

    // Can call on value == (initial) signaled value happens immediately
    {
        DawnCallbackUserdata userdata = 4347;
        EXPECT_CALL(*mockFenceOnCompletionCallback,
                    Call(DAWN_FENCE_COMPLETION_STATUS_SUCCESS, userdata))
            .Times(1);
        dawnFenceOnCompletion(fence, 1, ToMockFenceOnCompletionCallback, userdata);
    }
}

// Check that all passed client completion callbacks are called
TEST_F(WireFenceTests, OnCompletionMultiple) {
    DoQueueSignal(3u);
    DoQueueSignal(6u);

    DawnCallbackUserdata userdata0 = 2134;
    DawnCallbackUserdata userdata1 = 7134;
    DawnCallbackUserdata userdata2 = 3144;
    DawnCallbackUserdata userdata3 = 1130;

    // Add callbacks in a non-monotonic order. They should still be called
    // in order of increasing fence value.
    // Add multiple callbacks for the same value.
    dawnFenceOnCompletion(fence, 6, ToMockFenceOnCompletionCallback, userdata0);
    dawnFenceOnCompletion(fence, 2, ToMockFenceOnCompletionCallback, userdata1);
    dawnFenceOnCompletion(fence, 3, ToMockFenceOnCompletionCallback, userdata2);
    dawnFenceOnCompletion(fence, 2, ToMockFenceOnCompletionCallback, userdata3);

    Sequence s1, s2;
    EXPECT_CALL(*mockFenceOnCompletionCallback,
                Call(DAWN_FENCE_COMPLETION_STATUS_SUCCESS, userdata1))
        .Times(1)
        .InSequence(s1);
    EXPECT_CALL(*mockFenceOnCompletionCallback,
                Call(DAWN_FENCE_COMPLETION_STATUS_SUCCESS, userdata3))
        .Times(1)
        .InSequence(s2);
    EXPECT_CALL(*mockFenceOnCompletionCallback,
                Call(DAWN_FENCE_COMPLETION_STATUS_SUCCESS, userdata2))
        .Times(1)
        .InSequence(s1, s2);
    EXPECT_CALL(*mockFenceOnCompletionCallback,
                Call(DAWN_FENCE_COMPLETION_STATUS_SUCCESS, userdata0))
        .Times(1)
        .InSequence(s1, s2);

    FlushClient();
    FlushServer();
}

// Without any flushes, it is valid to wait on a value less than or equal to
// the last signaled value
TEST_F(WireFenceTests, OnCompletionSynchronousValidationSuccess) {
    dawnQueueSignal(queue, fence, 4u);
    dawnFenceOnCompletion(fence, 2u, ToMockFenceOnCompletionCallback, 0);
    dawnFenceOnCompletion(fence, 3u, ToMockFenceOnCompletionCallback, 0);
    dawnFenceOnCompletion(fence, 4u, ToMockFenceOnCompletionCallback, 0);

    EXPECT_CALL(*mockFenceOnCompletionCallback, Call(DAWN_FENCE_COMPLETION_STATUS_UNKNOWN, _))
        .Times(3);
}

// Without any flushes, errors should be generated when waiting on a value greater
// than the last signaled value
TEST_F(WireFenceTests, OnCompletionSynchronousValidationError) {
    DawnCallbackUserdata userdata1 = 3817;
    DawnCallbackUserdata userdata2 = 3857;
    dawnDeviceSetErrorCallback(device, ToMockDeviceErrorCallback, userdata2);

    EXPECT_CALL(*mockFenceOnCompletionCallback, Call(DAWN_FENCE_COMPLETION_STATUS_ERROR, userdata1))
        .Times(1);
    EXPECT_CALL(*mockDeviceErrorCallback, Call(_, userdata2)).Times(1);

    dawnFenceOnCompletion(fence, 2u, ToMockFenceOnCompletionCallback, userdata1);
}

// Check that the fence completed value is initialized
TEST_F(WireFenceTests, GetCompletedValueInitialization) {
    EXPECT_EQ(dawnFenceGetCompletedValue(fence), 1u);
}

// Check that the fence completed value updates after signaling the fence
TEST_F(WireFenceTests, GetCompletedValueUpdate) {
    DoQueueSignal(3u);
    FlushClient();
    FlushServer();

    EXPECT_EQ(dawnFenceGetCompletedValue(fence), 3u);
}

// Check that the fence completed value does not update without a flush
TEST_F(WireFenceTests, GetCompletedValueNoUpdate) {
    dawnQueueSignal(queue, fence, 3u);
    EXPECT_EQ(dawnFenceGetCompletedValue(fence), 1u);
}

// Check that the callback is called with UNKNOWN when the fence is destroyed
// before the completed value is updated
TEST_F(WireFenceTests, DestroyBeforeOnCompletionEnd) {
    DawnCallbackUserdata userdata = 8616;
    dawnQueueSignal(queue, fence, 3u);
    dawnFenceOnCompletion(fence, 2u, ToMockFenceOnCompletionCallback, userdata);
    EXPECT_CALL(*mockFenceOnCompletionCallback,
                Call(DAWN_FENCE_COMPLETION_STATUS_UNKNOWN, userdata))
        .Times(1);
}

// Test that signaling a fence on a wrong queue is invalid
TEST_F(WireFenceTests, SignalWrongQueue) {
    DawnQueue queue2 = dawnDeviceCreateQueue(device);
    DawnQueue apiQueue2 = api.GetNewQueue();
    EXPECT_CALL(api, DeviceCreateQueue(apiDevice)).WillOnce(Return(apiQueue2));
    FlushClient();

    DawnCallbackUserdata userdata = 1520;
    dawnDeviceSetErrorCallback(device, ToMockDeviceErrorCallback, userdata);

    EXPECT_CALL(*mockDeviceErrorCallback, Call(_, userdata)).Times(1);
    dawnQueueSignal(queue2, fence, 2u);  // error
}

// Test that signaling a fence on a wrong queue does not update fence signaled value
TEST_F(WireFenceTests, SignalWrongQueueDoesNotUpdateValue) {
    DawnQueue queue2 = dawnDeviceCreateQueue(device);
    DawnQueue apiQueue2 = api.GetNewQueue();
    EXPECT_CALL(api, DeviceCreateQueue(apiDevice)).WillOnce(Return(apiQueue2));
    FlushClient();

    DawnCallbackUserdata userdata = 1024;
    dawnDeviceSetErrorCallback(device, ToMockDeviceErrorCallback, userdata);

    EXPECT_CALL(*mockDeviceErrorCallback, Call(_, userdata)).Times(1);
    dawnQueueSignal(queue2, fence, 2u);  // error

    // Fence value should be unchanged.
    FlushClient();
    FlushServer();
    EXPECT_EQ(dawnFenceGetCompletedValue(fence), 1u);

    // Signaling with 2 on the correct queue should succeed
    DoQueueSignal(2u);  // success
    FlushClient();
    FlushServer();
    EXPECT_EQ(dawnFenceGetCompletedValue(fence), 2u);
}
