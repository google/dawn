// Copyright 2021 The Dawn Authors
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

#include <memory>

#include "dawn/tests/unittests/wire/WireTest.h"
#include "dawn/wire/WireClient.h"

namespace dawn::wire {

using testing::_;
using testing::InvokeWithoutArgs;
using testing::Mock;
using testing::Return;
using testing::StrictMock;

namespace {

// Mock class to add expectations on the wire calling callbacks
class MockCompilationInfoCallback {
  public:
    MOCK_METHOD(void,
                Call,
                (WGPUCompilationInfoRequestStatus status,
                 const WGPUCompilationInfo* info,
                 void* userdata));
};

std::unique_ptr<StrictMock<MockCompilationInfoCallback>> mockCompilationInfoCallback;
void ToMockGetCompilationInfoCallback(WGPUCompilationInfoRequestStatus status,
                                      const WGPUCompilationInfo* info,
                                      void* userdata) {
    mockCompilationInfoCallback->Call(status, info, userdata);
}

}  // anonymous namespace

class WireShaderModuleTests : public WireTest {
  public:
    WireShaderModuleTests() {}
    ~WireShaderModuleTests() override = default;

    void SetUp() override {
        WireTest::SetUp();

        mockCompilationInfoCallback = std::make_unique<StrictMock<MockCompilationInfoCallback>>();
        apiShaderModule = api.GetNewShaderModule();

        WGPUShaderModuleDescriptor descriptor = {};
        shaderModule = wgpuDeviceCreateShaderModule(device, &descriptor);

        EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _))
            .WillOnce(Return(apiShaderModule))
            .RetiresOnSaturation();
        FlushClient();
    }

    void TearDown() override {
        WireTest::TearDown();

        // Delete mock so that expectations are checked
        mockCompilationInfoCallback = nullptr;
    }

    void FlushClient() {
        WireTest::FlushClient();
        Mock::VerifyAndClearExpectations(&mockCompilationInfoCallback);
    }

    void FlushServer() {
        WireTest::FlushServer();
        Mock::VerifyAndClearExpectations(&mockCompilationInfoCallback);
    }

  protected:
    WGPUShaderModule shaderModule;
    WGPUShaderModule apiShaderModule;
};

// Check getting CompilationInfo for a successfully created shader module
TEST_F(WireShaderModuleTests, GetCompilationInfo) {
    wgpuShaderModuleGetCompilationInfo(shaderModule, ToMockGetCompilationInfoCallback, nullptr);

    WGPUCompilationMessage message = {
        nullptr, "Test Message", WGPUCompilationMessageType_Info, 2, 4, 6, 8};
    WGPUCompilationInfo compilationInfo;
    compilationInfo.nextInChain = nullptr;
    compilationInfo.messageCount = 1;
    compilationInfo.messages = &message;

    EXPECT_CALL(api, OnShaderModuleGetCompilationInfo(apiShaderModule, _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallShaderModuleGetCompilationInfoCallback(
                apiShaderModule, WGPUCompilationInfoRequestStatus_Success, &compilationInfo);
        }));

    FlushClient();

    EXPECT_CALL(*mockCompilationInfoCallback,
                Call(WGPUCompilationInfoRequestStatus_Success,
                     MatchesLambda([&](const WGPUCompilationInfo* info) -> bool {
                         if (info->messageCount != compilationInfo.messageCount) {
                             return false;
                         }
                         const WGPUCompilationMessage* infoMessage = &info->messages[0];
                         return strcmp(infoMessage->message, message.message) == 0 &&
                                infoMessage->nextInChain == message.nextInChain &&
                                infoMessage->type == message.type &&
                                infoMessage->lineNum == message.lineNum &&
                                infoMessage->linePos == message.linePos &&
                                infoMessage->offset == message.offset &&
                                infoMessage->length == message.length;
                     }),
                     _))
        .Times(1);
    FlushServer();
}

// Test that calling GetCompilationInfo then disconnecting the wire calls the callback with a
// device loss.
TEST_F(WireShaderModuleTests, GetCompilationInfoBeforeDisconnect) {
    wgpuShaderModuleGetCompilationInfo(shaderModule, ToMockGetCompilationInfoCallback, nullptr);

    WGPUCompilationMessage message = {
        nullptr, "Test Message", WGPUCompilationMessageType_Info, 2, 4, 6, 8};
    WGPUCompilationInfo compilationInfo;
    compilationInfo.nextInChain = nullptr;
    compilationInfo.messageCount = 1;
    compilationInfo.messages = &message;

    EXPECT_CALL(api, OnShaderModuleGetCompilationInfo(apiShaderModule, _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallShaderModuleGetCompilationInfoCallback(
                apiShaderModule, WGPUCompilationInfoRequestStatus_Success, &compilationInfo);
        }));
    FlushClient();

    EXPECT_CALL(*mockCompilationInfoCallback,
                Call(WGPUCompilationInfoRequestStatus_DeviceLost, nullptr, _));
    GetWireClient()->Disconnect();
}

// Test that calling GetCompilationInfo after disconnecting the wire calls the callback with a
// device loss.
TEST_F(WireShaderModuleTests, GetCompilationInfoAfterDisconnect) {
    GetWireClient()->Disconnect();
    EXPECT_CALL(*mockCompilationInfoCallback,
                Call(WGPUCompilationInfoRequestStatus_DeviceLost, nullptr, _));
    wgpuShaderModuleGetCompilationInfo(shaderModule, ToMockGetCompilationInfoCallback, nullptr);
}

// Hack to pass in test context into user callback
struct TestData {
    WireShaderModuleTests* pTest;
    WGPUShaderModule* pTestShaderModule;
    size_t numRequests;
};

static void ToMockBufferMapCallbackWithNewRequests(WGPUCompilationInfoRequestStatus status,
                                                   const WGPUCompilationInfo* info,
                                                   void* userdata) {
    TestData* testData = reinterpret_cast<TestData*>(userdata);
    // Mimic the user callback is sending new requests
    ASSERT_NE(testData, nullptr);
    ASSERT_NE(testData->pTest, nullptr);
    ASSERT_NE(testData->pTestShaderModule, nullptr);

    mockCompilationInfoCallback->Call(status, info, testData->pTest);

    // Send the requests a number of times
    for (size_t i = 0; i < testData->numRequests; i++) {
        wgpuShaderModuleGetCompilationInfo(*(testData->pTestShaderModule),
                                           ToMockGetCompilationInfoCallback, nullptr);
    }
}

// Test that requests inside user callbacks before disconnect are called
TEST_F(WireShaderModuleTests, GetCompilationInfoInsideCallbackBeforeDisconnect) {
    TestData testData = {this, &shaderModule, 10};

    wgpuShaderModuleGetCompilationInfo(shaderModule, ToMockBufferMapCallbackWithNewRequests,
                                       &testData);

    WGPUCompilationMessage message = {
        nullptr, "Test Message", WGPUCompilationMessageType_Info, 2, 4, 6, 8};
    WGPUCompilationInfo compilationInfo;
    compilationInfo.nextInChain = nullptr;
    compilationInfo.messageCount = 1;
    compilationInfo.messages = &message;

    EXPECT_CALL(api, OnShaderModuleGetCompilationInfo(apiShaderModule, _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallShaderModuleGetCompilationInfoCallback(
                apiShaderModule, WGPUCompilationInfoRequestStatus_Success, &compilationInfo);
        }));
    FlushClient();

    EXPECT_CALL(*mockCompilationInfoCallback,
                Call(WGPUCompilationInfoRequestStatus_DeviceLost, nullptr, _))
        .Times(1 + testData.numRequests);
    GetWireClient()->Disconnect();
}

// Test that requests inside user callbacks before object destruction are called
TEST_F(WireShaderModuleTests, GetCompilationInfoInsideCallbackBeforeDestruction) {
    TestData testData = {this, &shaderModule, 10};

    wgpuShaderModuleGetCompilationInfo(shaderModule, ToMockBufferMapCallbackWithNewRequests,
                                       &testData);

    WGPUCompilationMessage message = {
        nullptr, "Test Message", WGPUCompilationMessageType_Info, 2, 4, 6, 8};
    WGPUCompilationInfo compilationInfo;
    compilationInfo.nextInChain = nullptr;
    compilationInfo.messageCount = 1;
    compilationInfo.messages = &message;

    EXPECT_CALL(api, OnShaderModuleGetCompilationInfo(apiShaderModule, _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallShaderModuleGetCompilationInfoCallback(
                apiShaderModule, WGPUCompilationInfoRequestStatus_Success, &compilationInfo);
        }));
    FlushClient();

    EXPECT_CALL(*mockCompilationInfoCallback,
                Call(WGPUCompilationInfoRequestStatus_Unknown, nullptr, _))
        .Times(1 + testData.numRequests);
    wgpuShaderModuleRelease(shaderModule);
}

}  // namespace dawn::wire
