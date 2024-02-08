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

#include "dawn/tests/unittests/wire/WireTest.h"
#include "dawn/wire/WireClient.h"

namespace dawn::wire {
namespace {

using testing::_;
using testing::DoAll;
using testing::Mock;
using testing::Return;
using testing::SaveArg;
using testing::StrEq;
using testing::StrictMock;

// Mock classes to add expectations on the wire calling callbacks
class MockDeviceErrorCallback {
  public:
    MOCK_METHOD(void, Call, (WGPUErrorType type, const char* message, void* userdata));
};

std::unique_ptr<StrictMock<MockDeviceErrorCallback>> mockDeviceErrorCallback;
void ToMockDeviceErrorCallback(WGPUErrorType type, const char* message, void* userdata) {
    mockDeviceErrorCallback->Call(type, message, userdata);
}

class MockDevicePopErrorScopeCallback {
  public:
    MOCK_METHOD(void, Call, (WGPUErrorType type, const char* message, void* userdata));
};

std::unique_ptr<StrictMock<MockDevicePopErrorScopeCallback>> mockDevicePopErrorScopeCallback;
void ToMockDevicePopErrorScopeCallback(WGPUErrorType type, const char* message, void* userdata) {
    mockDevicePopErrorScopeCallback->Call(type, message, userdata);
}

class MockDeviceLoggingCallback {
  public:
    MOCK_METHOD(void, Call, (WGPULoggingType type, const char* message, void* userdata));
};

std::unique_ptr<StrictMock<MockDeviceLoggingCallback>> mockDeviceLoggingCallback;
void ToMockDeviceLoggingCallback(WGPULoggingType type, const char* message, void* userdata) {
    mockDeviceLoggingCallback->Call(type, message, userdata);
}

class MockDeviceLostCallback {
  public:
    MOCK_METHOD(void, Call, (WGPUDeviceLostReason reason, const char* message, void* userdata));
};

std::unique_ptr<StrictMock<MockDeviceLostCallback>> mockDeviceLostCallback;
void ToMockDeviceLostCallback(WGPUDeviceLostReason reason, const char* message, void* userdata) {
    mockDeviceLostCallback->Call(reason, message, userdata);
}

class WireErrorCallbackTests : public WireTest {
  public:
    WireErrorCallbackTests() {}
    ~WireErrorCallbackTests() override = default;

    void SetUp() override {
        WireTest::SetUp();

        mockDeviceErrorCallback = std::make_unique<StrictMock<MockDeviceErrorCallback>>();
        mockDeviceLoggingCallback = std::make_unique<StrictMock<MockDeviceLoggingCallback>>();
        mockDevicePopErrorScopeCallback =
            std::make_unique<StrictMock<MockDevicePopErrorScopeCallback>>();
        mockDeviceLostCallback = std::make_unique<StrictMock<MockDeviceLostCallback>>();
    }

    void TearDown() override {
        WireTest::TearDown();

        mockDeviceErrorCallback = nullptr;
        mockDeviceLoggingCallback = nullptr;
        mockDevicePopErrorScopeCallback = nullptr;
        mockDeviceLostCallback = nullptr;
    }

    void FlushServer() {
        WireTest::FlushServer();

        Mock::VerifyAndClearExpectations(&mockDeviceErrorCallback);
        Mock::VerifyAndClearExpectations(&mockDevicePopErrorScopeCallback);
    }
};

// Test the return wire for device validation error callbacks
TEST_F(WireErrorCallbackTests, DeviceValidationErrorCallback) {
    wgpuDeviceSetUncapturedErrorCallback(device, ToMockDeviceErrorCallback, this);

    // Setting the error callback should stay on the client side and do nothing
    FlushClient();

    // Calling the callback on the server side will result in the callback being called on the
    // client side
    api.CallDeviceSetUncapturedErrorCallbackCallback(apiDevice, WGPUErrorType_Validation,
                                                     "Some error message");

    EXPECT_CALL(*mockDeviceErrorCallback,
                Call(WGPUErrorType_Validation, StrEq("Some error message"), this))
        .Times(1);

    FlushServer();
}

// Test the return wire for device OOM error callbacks
TEST_F(WireErrorCallbackTests, DeviceOutOfMemoryErrorCallback) {
    wgpuDeviceSetUncapturedErrorCallback(device, ToMockDeviceErrorCallback, this);

    // Setting the error callback should stay on the client side and do nothing
    FlushClient();

    // Calling the callback on the server side will result in the callback being called on the
    // client side
    api.CallDeviceSetUncapturedErrorCallbackCallback(apiDevice, WGPUErrorType_OutOfMemory,
                                                     "Some error message");

    EXPECT_CALL(*mockDeviceErrorCallback,
                Call(WGPUErrorType_OutOfMemory, StrEq("Some error message"), this))
        .Times(1);

    FlushServer();
}

// Test the return wire for device internal error callbacks
TEST_F(WireErrorCallbackTests, DeviceInternalErrorCallback) {
    wgpuDeviceSetUncapturedErrorCallback(device, ToMockDeviceErrorCallback, this);

    // Setting the error callback should stay on the client side and do nothing
    FlushClient();

    // Calling the callback on the server side will result in the callback being called on the
    // client side
    api.CallDeviceSetUncapturedErrorCallbackCallback(apiDevice, WGPUErrorType_Internal,
                                                     "Some error message");

    EXPECT_CALL(*mockDeviceErrorCallback,
                Call(WGPUErrorType_Internal, StrEq("Some error message"), this))
        .Times(1);

    FlushServer();
}

// Test the return wire for device user warning callbacks
TEST_F(WireErrorCallbackTests, DeviceLoggingCallback) {
    wgpuDeviceSetLoggingCallback(device, ToMockDeviceLoggingCallback, this);

    // Setting the injected warning callback should stay on the client side and do nothing
    FlushClient();

    // Calling the callback on the server side will result in the callback being called on the
    // client side
    api.CallDeviceSetLoggingCallbackCallback(apiDevice, WGPULoggingType_Info, "Some message");

    EXPECT_CALL(*mockDeviceLoggingCallback, Call(WGPULoggingType_Info, StrEq("Some message"), this))
        .Times(1);

    FlushServer();
}

// Test the return wire for validation error scopes.
TEST_F(WireErrorCallbackTests, PushPopValidationErrorScopeCallback) {
    EXPECT_CALL(api, DevicePushErrorScope(apiDevice, WGPUErrorFilter_Validation)).Times(1);
    wgpuDevicePushErrorScope(device, WGPUErrorFilter_Validation);
    FlushClient();

    WGPUErrorCallback callback;
    void* userdata;
    EXPECT_CALL(api, OnDevicePopErrorScope(apiDevice, _, _))
        .WillOnce(DoAll(SaveArg<1>(&callback), SaveArg<2>(&userdata)));
    wgpuDevicePopErrorScope(device, ToMockDevicePopErrorScopeCallback, this);
    FlushClient();

    EXPECT_CALL(*mockDevicePopErrorScopeCallback,
                Call(WGPUErrorType_Validation, StrEq("Some error message"), this))
        .Times(1);
    callback(WGPUErrorType_Validation, "Some error message", userdata);
    FlushServer();
}

// Test the return wire for OOM error scopes.
TEST_F(WireErrorCallbackTests, PushPopOOMErrorScopeCallback) {
    EXPECT_CALL(api, DevicePushErrorScope(apiDevice, WGPUErrorFilter_OutOfMemory)).Times(1);
    wgpuDevicePushErrorScope(device, WGPUErrorFilter_OutOfMemory);
    FlushClient();

    WGPUErrorCallback callback;
    void* userdata;
    EXPECT_CALL(api, OnDevicePopErrorScope(apiDevice, _, _))
        .WillOnce(DoAll(SaveArg<1>(&callback), SaveArg<2>(&userdata)));
    wgpuDevicePopErrorScope(device, ToMockDevicePopErrorScopeCallback, this);
    FlushClient();

    EXPECT_CALL(*mockDevicePopErrorScopeCallback,
                Call(WGPUErrorType_OutOfMemory, StrEq("Some error message"), this))
        .Times(1);
    callback(WGPUErrorType_OutOfMemory, "Some error message", userdata);
    FlushServer();
}

// Test the return wire for internal error scopes.
TEST_F(WireErrorCallbackTests, PushPopInternalErrorScopeCallback) {
    EXPECT_CALL(api, DevicePushErrorScope(apiDevice, WGPUErrorFilter_Internal)).Times(1);
    wgpuDevicePushErrorScope(device, WGPUErrorFilter_Internal);
    FlushClient();

    WGPUErrorCallback callback;
    void* userdata;
    EXPECT_CALL(api, OnDevicePopErrorScope(apiDevice, _, _))
        .WillOnce(DoAll(SaveArg<1>(&callback), SaveArg<2>(&userdata)));
    wgpuDevicePopErrorScope(device, ToMockDevicePopErrorScopeCallback, this);
    FlushClient();

    EXPECT_CALL(*mockDevicePopErrorScopeCallback,
                Call(WGPUErrorType_Internal, StrEq("Some error message"), this))
        .Times(1);
    callback(WGPUErrorType_Internal, "Some error message", userdata);
    FlushServer();
}

// Test the return wire for error scopes when callbacks return in a various orders.
TEST_F(WireErrorCallbackTests, PopErrorScopeCallbackOrdering) {
    // Two error scopes are popped, and the first one returns first.
    {
        EXPECT_CALL(api, DevicePushErrorScope(apiDevice, WGPUErrorFilter_Validation)).Times(2);
        wgpuDevicePushErrorScope(device, WGPUErrorFilter_Validation);
        wgpuDevicePushErrorScope(device, WGPUErrorFilter_Validation);
        FlushClient();

        WGPUErrorCallback callback1;
        WGPUErrorCallback callback2;
        void* userdata1;
        void* userdata2;
        EXPECT_CALL(api, OnDevicePopErrorScope(apiDevice, _, _))
            .WillOnce(DoAll(SaveArg<1>(&callback1), SaveArg<2>(&userdata1)))
            .WillOnce(DoAll(SaveArg<1>(&callback2), SaveArg<2>(&userdata2)));
        wgpuDevicePopErrorScope(device, ToMockDevicePopErrorScopeCallback, this);
        wgpuDevicePopErrorScope(device, ToMockDevicePopErrorScopeCallback, this + 1);
        FlushClient();

        EXPECT_CALL(*mockDevicePopErrorScopeCallback,
                    Call(WGPUErrorType_Validation, StrEq("First error message"), this))
            .Times(1);
        callback1(WGPUErrorType_Validation, "First error message", userdata1);
        FlushServer();

        EXPECT_CALL(*mockDevicePopErrorScopeCallback,
                    Call(WGPUErrorType_Validation, StrEq("Second error message"), this + 1))
            .Times(1);
        callback2(WGPUErrorType_Validation, "Second error message", userdata2);
        FlushServer();
    }

    // Two error scopes are popped, and the second one returns first.
    {
        EXPECT_CALL(api, DevicePushErrorScope(apiDevice, WGPUErrorFilter_Validation)).Times(2);
        wgpuDevicePushErrorScope(device, WGPUErrorFilter_Validation);
        wgpuDevicePushErrorScope(device, WGPUErrorFilter_Validation);
        FlushClient();

        WGPUErrorCallback callback1;
        WGPUErrorCallback callback2;
        void* userdata1;
        void* userdata2;
        EXPECT_CALL(api, OnDevicePopErrorScope(apiDevice, _, _))
            .WillOnce(DoAll(SaveArg<1>(&callback1), SaveArg<2>(&userdata1)))
            .WillOnce(DoAll(SaveArg<1>(&callback2), SaveArg<2>(&userdata2)));
        wgpuDevicePopErrorScope(device, ToMockDevicePopErrorScopeCallback, this);
        wgpuDevicePopErrorScope(device, ToMockDevicePopErrorScopeCallback, this + 1);
        FlushClient();

        EXPECT_CALL(*mockDevicePopErrorScopeCallback,
                    Call(WGPUErrorType_Validation, StrEq("Second error message"), this + 1))
            .Times(1);
        callback2(WGPUErrorType_Validation, "Second error message", userdata2);
        FlushServer();

        EXPECT_CALL(*mockDevicePopErrorScopeCallback,
                    Call(WGPUErrorType_Validation, StrEq("First error message"), this))
            .Times(1);
        callback1(WGPUErrorType_Validation, "First error message", userdata1);
        FlushServer();
    }
}

// Test the return wire for error scopes in flight when the device is destroyed.
TEST_F(WireErrorCallbackTests, PopErrorScopeDeviceInFlightDestroy) {
    EXPECT_CALL(api, DevicePushErrorScope(apiDevice, WGPUErrorFilter_Validation)).Times(1);
    wgpuDevicePushErrorScope(device, WGPUErrorFilter_Validation);
    FlushClient();

    EXPECT_CALL(api, OnDevicePopErrorScope(apiDevice, _, _)).Times(1);
    wgpuDevicePopErrorScope(device, ToMockDevicePopErrorScopeCallback, this);
    FlushClient();

    // Incomplete callback called in Device destructor. This is resolved after the end of this
    // test.
    EXPECT_CALL(*mockDevicePopErrorScopeCallback,
                Call(WGPUErrorType_Unknown, ValidStringMessage(), this))
        .Times(1);
}

// Test that registering a callback then wire disconnect calls the callback with
// DeviceLost.
TEST_F(WireErrorCallbackTests, PopErrorScopeThenDisconnect) {
    EXPECT_CALL(api, DevicePushErrorScope(apiDevice, WGPUErrorFilter_Validation)).Times(1);
    wgpuDevicePushErrorScope(device, WGPUErrorFilter_Validation);

    EXPECT_CALL(api, OnDevicePopErrorScope(apiDevice, _, _)).Times(1);
    wgpuDevicePopErrorScope(device, ToMockDevicePopErrorScopeCallback, this);
    FlushClient();

    EXPECT_CALL(*mockDevicePopErrorScopeCallback,
                Call(WGPUErrorType_DeviceLost, ValidStringMessage(), this))
        .Times(1);
    GetWireClient()->Disconnect();
}

// Test that registering a callback after wire disconnect calls the callback with
// DeviceLost.
TEST_F(WireErrorCallbackTests, PopErrorScopeAfterDisconnect) {
    EXPECT_CALL(api, DevicePushErrorScope(apiDevice, WGPUErrorFilter_Validation)).Times(1);
    wgpuDevicePushErrorScope(device, WGPUErrorFilter_Validation);
    FlushClient();

    GetWireClient()->Disconnect();

    EXPECT_CALL(*mockDevicePopErrorScopeCallback,
                Call(WGPUErrorType_DeviceLost, ValidStringMessage(), this))
        .Times(1);
    wgpuDevicePopErrorScope(device, ToMockDevicePopErrorScopeCallback, this);
}

// Empty stack (We are emulating the errors that would be callback-ed from native).
TEST_F(WireErrorCallbackTests, PopErrorScopeEmptyStack) {
    WGPUErrorCallback callback;
    void* userdata;
    EXPECT_CALL(api, OnDevicePopErrorScope(apiDevice, _, _))
        .WillOnce(DoAll(SaveArg<1>(&callback), SaveArg<2>(&userdata)));
    wgpuDevicePopErrorScope(device, ToMockDevicePopErrorScopeCallback, this);
    FlushClient();

    EXPECT_CALL(*mockDevicePopErrorScopeCallback,
                Call(WGPUErrorType_Validation, StrEq("No error scopes to pop"), this))
        .Times(1);
    callback(WGPUErrorType_Validation, "No error scopes to pop", userdata);
    FlushServer();
}

// Test the return wire for device lost callback
TEST_F(WireErrorCallbackTests, DeviceLostCallback) {
    wgpuDeviceSetDeviceLostCallback(device, ToMockDeviceLostCallback, this);

    // Setting the error callback should stay on the client side and do nothing
    FlushClient();

    // Calling the callback on the server side will result in the callback being called on the
    // client side
    api.CallDeviceSetDeviceLostCallbackCallback(apiDevice, WGPUDeviceLostReason_Undefined,
                                                "Some error message");

    EXPECT_CALL(*mockDeviceLostCallback,
                Call(WGPUDeviceLostReason_Undefined, StrEq("Some error message"), this))
        .Times(1);

    FlushServer();
}

}  // anonymous namespace
}  // namespace dawn::wire
