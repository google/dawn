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
        MOCK_METHOD2(Call, void(const char* message, dawnCallbackUserdata userdata));
    };

    std::unique_ptr<MockDeviceErrorCallback> mockDeviceErrorCallback;
    void ToMockDeviceErrorCallback(const char* message, dawnCallbackUserdata userdata) {
        mockDeviceErrorCallback->Call(message, userdata);
    }

    class MockFenceOnCompletionCallback {
      public:
        MOCK_METHOD2(Call, void(dawnFenceCompletionStatus status, dawnCallbackUserdata userdata));
    };

    std::unique_ptr<MockFenceOnCompletionCallback> mockFenceOnCompletionCallback;
    void ToMockFenceOnCompletionCallback(dawnFenceCompletionStatus status,
                                         dawnCallbackUserdata userdata) {
        mockFenceOnCompletionCallback->Call(status, userdata);
    }

}  // anonymous namespace

class WireFenceTests : public WireTest {
  public:
    WireFenceTests() : WireTest(true) {
    }
    ~WireFenceTests() override = default;

    void SetUp() override {
        WireTest::SetUp();

        mockDeviceErrorCallback = std::make_unique<MockDeviceErrorCallback>();
        mockFenceOnCompletionCallback = std::make_unique<MockFenceOnCompletionCallback>();

        {
            dawnFenceDescriptor descriptor;
            descriptor.initialValue = 1;
            descriptor.nextInChain = nullptr;

            apiFence = api.GetNewFence();
            fence = dawnDeviceCreateFence(device, &descriptor);

            EXPECT_CALL(api, DeviceCreateFence(apiDevice, _)).WillOnce(Return(apiFence));
            EXPECT_CALL(api, FenceRelease(apiFence));
            FlushClient();
        }
        {
            queue = dawnDeviceCreateQueue(device);
            apiQueue = api.GetNewQueue();
            EXPECT_CALL(api, DeviceCreateQueue(apiDevice)).WillOnce(Return(apiQueue));
            EXPECT_CALL(api, QueueRelease(apiQueue));
            FlushClient();
        }
    }

    void TearDown() override {
        WireTest::TearDown();

        // Delete mocks so that expectations are checked
        mockDeviceErrorCallback = nullptr;
        mockFenceOnCompletionCallback = nullptr;
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
    dawnFence fence;
    dawnFence apiFence;

    dawnQueue queue;
    dawnQueue apiQueue;
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
    dawnCallbackUserdata userdata = 9157;
    dawnDeviceSetErrorCallback(device, ToMockDeviceErrorCallback, userdata);
    EXPECT_CALL(*mockDeviceErrorCallback, Call(_, userdata)).Times(0);

    dawnQueueSignal(queue, fence, 2u);
    dawnQueueSignal(queue, fence, 4u);
    dawnQueueSignal(queue, fence, 5u);
}

// Without any flushes, errors should be generated when signaling a value less
// than or equal to the current signaled value
TEST_F(WireFenceTests, QueueSignalSynchronousValidationError) {
    dawnCallbackUserdata userdata = 3157;
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
        dawnCallbackUserdata userdata = 9847;
        EXPECT_CALL(*mockFenceOnCompletionCallback,
                    Call(DAWN_FENCE_COMPLETION_STATUS_SUCCESS, userdata))
            .Times(1);
        dawnFenceOnCompletion(fence, 0, ToMockFenceOnCompletionCallback, userdata);
    }

    // Can call on value == (initial) signaled value happens immediately
    {
        dawnCallbackUserdata userdata = 4347;
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

    dawnCallbackUserdata userdata0 = 2134;
    dawnCallbackUserdata userdata1 = 7134;
    dawnCallbackUserdata userdata2 = 3144;
    dawnCallbackUserdata userdata3 = 1130;

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
}

// Without any flushes, errors should be generated when waiting on a value greater
// than the last signaled value
TEST_F(WireFenceTests, OnCompletionSynchronousValidationError) {
    dawnCallbackUserdata userdata1 = 3817;
    dawnCallbackUserdata userdata2 = 3857;
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
    dawnCallbackUserdata userdata = 8616;
    dawnQueueSignal(queue, fence, 3u);
    dawnFenceOnCompletion(fence, 2u, ToMockFenceOnCompletionCallback, userdata);
    EXPECT_CALL(*mockFenceOnCompletionCallback,
                Call(DAWN_FENCE_COMPLETION_STATUS_UNKNOWN, userdata))
        .Times(1);

    dawnFenceRelease(fence);
}
