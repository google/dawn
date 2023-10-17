// Copyright 2021 The Dawn & Tint Authors
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

#include "dawn/tests/unittests/wire/WireTest.h"
#include "dawn/wire/WireClient.h"

namespace dawn::wire {
namespace {

using testing::_;
using testing::InvokeWithoutArgs;
using testing::Mock;

class MockQueueWorkDoneCallback {
  public:
    MOCK_METHOD(void, Call, (WGPUQueueWorkDoneStatus status, void* userdata));
};

static std::unique_ptr<MockQueueWorkDoneCallback> mockQueueWorkDoneCallback;
static void ToMockQueueWorkDone(WGPUQueueWorkDoneStatus status, void* userdata) {
    mockQueueWorkDoneCallback->Call(status, userdata);
}

class WireQueueTests : public WireTest {
  protected:
    void SetUp() override {
        WireTest::SetUp();
        mockQueueWorkDoneCallback = std::make_unique<MockQueueWorkDoneCallback>();
    }

    void TearDown() override {
        WireTest::TearDown();
        mockQueueWorkDoneCallback = nullptr;
    }

    void FlushServer() {
        WireTest::FlushServer();
        Mock::VerifyAndClearExpectations(&mockQueueWorkDoneCallback);
    }
};

// Test that a successful OnSubmittedWorkDone call is forwarded to the client.
TEST_F(WireQueueTests, OnSubmittedWorkDoneSuccess) {
    wgpuQueueOnSubmittedWorkDone(queue, ToMockQueueWorkDone, this);
    EXPECT_CALL(api, OnQueueOnSubmittedWorkDone(apiQueue, _, _)).WillOnce(InvokeWithoutArgs([&] {
        api.CallQueueOnSubmittedWorkDoneCallback(apiQueue, WGPUQueueWorkDoneStatus_Success);
    }));
    FlushClient();

    EXPECT_CALL(*mockQueueWorkDoneCallback, Call(WGPUQueueWorkDoneStatus_Success, this)).Times(1);
    FlushServer();
}

// Test that an error OnSubmittedWorkDone call is forwarded as an error to the client.
TEST_F(WireQueueTests, OnSubmittedWorkDoneError) {
    wgpuQueueOnSubmittedWorkDone(queue, ToMockQueueWorkDone, this);
    EXPECT_CALL(api, OnQueueOnSubmittedWorkDone(apiQueue, _, _)).WillOnce(InvokeWithoutArgs([&] {
        api.CallQueueOnSubmittedWorkDoneCallback(apiQueue, WGPUQueueWorkDoneStatus_Error);
    }));
    FlushClient();

    EXPECT_CALL(*mockQueueWorkDoneCallback, Call(WGPUQueueWorkDoneStatus_Error, this)).Times(1);
    FlushServer();
}

// Test registering an OnSubmittedWorkDone then disconnecting the wire calls the callback with
// device loss
TEST_F(WireQueueTests, OnSubmittedWorkDoneBeforeDisconnect) {
    wgpuQueueOnSubmittedWorkDone(queue, ToMockQueueWorkDone, this);
    EXPECT_CALL(api, OnQueueOnSubmittedWorkDone(apiQueue, _, _)).WillOnce(InvokeWithoutArgs([&] {
        api.CallQueueOnSubmittedWorkDoneCallback(apiQueue, WGPUQueueWorkDoneStatus_Error);
    }));
    FlushClient();

    EXPECT_CALL(*mockQueueWorkDoneCallback, Call(WGPUQueueWorkDoneStatus_DeviceLost, this))
        .Times(1);
    GetWireClient()->Disconnect();
}

// Test registering an OnSubmittedWorkDone after disconnecting the wire calls the callback with
// device loss
TEST_F(WireQueueTests, OnSubmittedWorkDoneAfterDisconnect) {
    GetWireClient()->Disconnect();

    EXPECT_CALL(*mockQueueWorkDoneCallback, Call(WGPUQueueWorkDoneStatus_DeviceLost, this))
        .Times(1);
    wgpuQueueOnSubmittedWorkDone(queue, ToMockQueueWorkDone, this);
}

// Hack to pass in test context into user callback
struct TestData {
    WireQueueTests* pTest;
    WGPUQueue* pTestQueue;
    size_t numRequests;
};

static void ToMockQueueWorkDoneWithNewRequests(WGPUQueueWorkDoneStatus status, void* userdata) {
    TestData* testData = reinterpret_cast<TestData*>(userdata);
    // Mimic the user callback is sending new requests
    ASSERT_NE(testData, nullptr);
    ASSERT_NE(testData->pTest, nullptr);
    ASSERT_NE(testData->pTestQueue, nullptr);
    mockQueueWorkDoneCallback->Call(status, testData->pTest);

    // Send the requests a number of times
    for (size_t i = 0; i < testData->numRequests; i++) {
        wgpuQueueOnSubmittedWorkDone(*(testData->pTestQueue), ToMockQueueWorkDone, testData->pTest);
    }
}

// Test that requests inside user callbacks before disconnect are called
TEST_F(WireQueueTests, OnSubmittedWorkDoneInsideCallbackBeforeDisconnect) {
    TestData testData = {this, &queue, 10};
    wgpuQueueOnSubmittedWorkDone(queue, ToMockQueueWorkDoneWithNewRequests, &testData);
    EXPECT_CALL(api, OnQueueOnSubmittedWorkDone(apiQueue, _, _)).WillOnce(InvokeWithoutArgs([&] {
        api.CallQueueOnSubmittedWorkDoneCallback(apiQueue, WGPUQueueWorkDoneStatus_Error);
    }));
    FlushClient();

    EXPECT_CALL(*mockQueueWorkDoneCallback, Call(WGPUQueueWorkDoneStatus_DeviceLost, this))
        .Times(1 + testData.numRequests);
    GetWireClient()->Disconnect();
}

// Test releasing the default queue, then its device. Both should be
// released when the device is released since the device holds a reference
// to the queue. Regresssion test for crbug.com/1332926.
TEST_F(WireQueueTests, DefaultQueueThenDeviceReleased) {
    // Note: The test fixture gets the default queue.

    // Release the queue which is the last external client reference.
    // The device still holds a reference.
    wgpuQueueRelease(queue);
    FlushClient();

    // Release the device which holds an internal reference to the queue.
    // Now, the queue and device should be released on the server.
    wgpuDeviceRelease(device);

    EXPECT_CALL(api, QueueRelease(apiQueue));
    EXPECT_CALL(api, DeviceRelease(apiDevice));
    // These set X callback methods are called before the device is released.
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(apiDevice, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(apiDevice, nullptr, nullptr)).Times(1);
    FlushClient();

    // Indicate to the fixture that the device was already released.
    DefaultApiDeviceWasReleased();
}

// Test the device, then its default queue. The default queue should be
// released when its external reference is dropped since releasing the device
// drops the internal reference. Regresssion test for crbug.com/1332926.
TEST_F(WireQueueTests, DeviceThenDefaultQueueReleased) {
    // Note: The test fixture gets the default queue.

    // Release the device which holds an internal reference to the queue.
    // Now, the should be released on the server, but not the queue since
    // the default queue still has one external reference.
    wgpuDeviceRelease(device);

    EXPECT_CALL(api, DeviceRelease(apiDevice));
    // These set X callback methods are called before the device is released.
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(apiDevice, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(apiDevice, nullptr, nullptr)).Times(1);
    FlushClient();

    // Release the external queue reference. The queue should be released.
    wgpuQueueRelease(queue);
    EXPECT_CALL(api, QueueRelease(apiQueue));
    FlushClient();

    // Indicate to the fixture that the device was already released.
    DefaultApiDeviceWasReleased();
}

// Only one default queue is supported now so we cannot test ~Queue triggering ClearAllCallbacks
// since it is always destructed after the test TearDown, and we cannot create a new queue obj
// with wgpuDeviceGetQueue

}  // anonymous namespace
}  // namespace dawn::wire
