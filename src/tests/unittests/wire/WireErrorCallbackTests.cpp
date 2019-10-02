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
        MOCK_METHOD3(Call, void(DawnErrorType type, const char* message, void* userdata));
    };

    std::unique_ptr<StrictMock<MockDeviceErrorCallback>> mockDeviceErrorCallback;
    void ToMockDeviceErrorCallback(DawnErrorType type, const char* message, void* userdata) {
        mockDeviceErrorCallback->Call(type, message, userdata);
    }

    class MockDevicePopErrorScopeCallback {
      public:
        MOCK_METHOD3(Call, void(DawnErrorType type, const char* message, void* userdata));
    };

    std::unique_ptr<StrictMock<MockDevicePopErrorScopeCallback>> mockDevicePopErrorScopeCallback;
    void ToMockDevicePopErrorScopeCallback(DawnErrorType type, const char* message, void* userdata) {
        mockDevicePopErrorScopeCallback->Call(type, message, userdata);
    }

}  // anonymous namespace

class WireErrorCallbackTests : public WireTest {
  public:
    WireErrorCallbackTests() {
    }
    ~WireErrorCallbackTests() override = default;

    void SetUp() override {
        WireTest::SetUp();

        mockDeviceErrorCallback = std::make_unique<StrictMock<MockDeviceErrorCallback>>();
        mockDevicePopErrorScopeCallback = std::make_unique<StrictMock<MockDevicePopErrorScopeCallback>>();
    }

    void TearDown() override {
        WireTest::TearDown();

        mockDeviceErrorCallback = nullptr;
        mockDevicePopErrorScopeCallback = nullptr;
    }

    void FlushServer() {
        WireTest::FlushServer();

        Mock::VerifyAndClearExpectations(&mockDeviceErrorCallback);
        Mock::VerifyAndClearExpectations(&mockDevicePopErrorScopeCallback);
    }
};

// Test the return wire for device error callbacks
TEST_F(WireErrorCallbackTests, DeviceErrorCallback) {
    dawnDeviceSetUncapturedErrorCallback(device, ToMockDeviceErrorCallback, this);

    // Setting the error callback should stay on the client side and do nothing
    FlushClient();

    // Calling the callback on the server side will result in the callback being called on the
    // client side
    api.CallDeviceErrorCallback(apiDevice, DAWN_ERROR_TYPE_VALIDATION, "Some error message");

    EXPECT_CALL(*mockDeviceErrorCallback, Call(DAWN_ERROR_TYPE_VALIDATION, StrEq("Some error message"), this)).Times(1);

    FlushServer();
}

// Test the return wire for error scopes.
TEST_F(WireErrorCallbackTests, PushPopErrorScopeCallback) {
    dawnDevicePushErrorScope(device, DAWN_ERROR_FILTER_VALIDATION);
    EXPECT_CALL(api, DevicePushErrorScope(apiDevice, DAWN_ERROR_FILTER_VALIDATION)).Times(1);

    FlushClient();

    dawnDevicePopErrorScope(device, ToMockDevicePopErrorScopeCallback, this);

    DawnErrorCallback callback;
    void* userdata;
    EXPECT_CALL(api, OnDevicePopErrorScopeCallback(apiDevice, _, _))
        .WillOnce(DoAll(SaveArg<1>(&callback), SaveArg<2>(&userdata), Return(true)));

    FlushClient();

    callback(DAWN_ERROR_TYPE_VALIDATION, "Some error message", userdata);
    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(DAWN_ERROR_TYPE_VALIDATION, StrEq("Some error message"), this)).Times(1);

    FlushServer();
}

// Test the return wire for error scopes when callbacks return in a various orders.
TEST_F(WireErrorCallbackTests, PopErrorScopeCallbackOrdering) {
    // Two error scopes are popped, and the first one returns first.
    {
        dawnDevicePushErrorScope(device, DAWN_ERROR_FILTER_VALIDATION);
        dawnDevicePushErrorScope(device, DAWN_ERROR_FILTER_VALIDATION);
        EXPECT_CALL(api, DevicePushErrorScope(apiDevice, DAWN_ERROR_FILTER_VALIDATION)).Times(2);

        FlushClient();

        dawnDevicePopErrorScope(device, ToMockDevicePopErrorScopeCallback, this);
        dawnDevicePopErrorScope(device, ToMockDevicePopErrorScopeCallback, this + 1);

        DawnErrorCallback callback1;
        DawnErrorCallback callback2;
        void* userdata1;
        void* userdata2;
        EXPECT_CALL(api, OnDevicePopErrorScopeCallback(apiDevice, _, _))
            .WillOnce(DoAll(SaveArg<1>(&callback1), SaveArg<2>(&userdata1), Return(true)))
            .WillOnce(DoAll(SaveArg<1>(&callback2), SaveArg<2>(&userdata2), Return(true)));

        FlushClient();

        callback1(DAWN_ERROR_TYPE_VALIDATION, "First error message", userdata1);
        EXPECT_CALL(*mockDevicePopErrorScopeCallback,
                    Call(DAWN_ERROR_TYPE_VALIDATION,
                    StrEq("First error message"), this)).Times(1);
        FlushServer();

        callback2(DAWN_ERROR_TYPE_VALIDATION, "Second error message", userdata2);
        EXPECT_CALL(*mockDevicePopErrorScopeCallback,
                    Call(DAWN_ERROR_TYPE_VALIDATION,
                    StrEq("Second error message"), this + 1)).Times(1);
        FlushServer();
    }

    // Two error scopes are popped, and the second one returns first.
    {
        dawnDevicePushErrorScope(device, DAWN_ERROR_FILTER_VALIDATION);
        dawnDevicePushErrorScope(device, DAWN_ERROR_FILTER_VALIDATION);
        EXPECT_CALL(api, DevicePushErrorScope(apiDevice, DAWN_ERROR_FILTER_VALIDATION)).Times(2);

        FlushClient();

        dawnDevicePopErrorScope(device, ToMockDevicePopErrorScopeCallback, this);
        dawnDevicePopErrorScope(device, ToMockDevicePopErrorScopeCallback, this + 1);

        DawnErrorCallback callback1;
        DawnErrorCallback callback2;
        void* userdata1;
        void* userdata2;
        EXPECT_CALL(api, OnDevicePopErrorScopeCallback(apiDevice, _, _))
            .WillOnce(DoAll(SaveArg<1>(&callback1), SaveArg<2>(&userdata1), Return(true)))
            .WillOnce(DoAll(SaveArg<1>(&callback2), SaveArg<2>(&userdata2), Return(true)));

        FlushClient();

        callback2(DAWN_ERROR_TYPE_VALIDATION, "Second error message", userdata2);
        EXPECT_CALL(*mockDevicePopErrorScopeCallback,
                    Call(DAWN_ERROR_TYPE_VALIDATION,
                    StrEq("Second error message"), this + 1)).Times(1);
        FlushServer();

        callback1(DAWN_ERROR_TYPE_VALIDATION, "First error message", userdata1);
        EXPECT_CALL(*mockDevicePopErrorScopeCallback,
                    Call(DAWN_ERROR_TYPE_VALIDATION,
                    StrEq("First error message"), this)).Times(1);
        FlushServer();
    }
}

// Test the return wire for error scopes in flight when the device is destroyed.
TEST_F(WireErrorCallbackTests, PopErrorScopeDeviceDestroyed) {
    dawnDevicePushErrorScope(device, DAWN_ERROR_FILTER_VALIDATION);
    EXPECT_CALL(api, DevicePushErrorScope(apiDevice, DAWN_ERROR_FILTER_VALIDATION)).Times(1);

    FlushClient();

    EXPECT_TRUE(dawnDevicePopErrorScope(device, ToMockDevicePopErrorScopeCallback, this));

    EXPECT_CALL(api, OnDevicePopErrorScopeCallback(apiDevice, _, _))
        .WillOnce(Return(true));
    FlushClient();

    // Incomplete callback called in Device destructor.
    EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(DAWN_ERROR_TYPE_UNKNOWN, ValidStringMessage(), this)).Times(1);
}

// Test that PopErrorScope returns false if there are no error scopes.
TEST_F(WireErrorCallbackTests, PopErrorScopeEmptyStack) {
    // Empty stack
    {
        EXPECT_FALSE(dawnDevicePopErrorScope(device, ToMockDevicePopErrorScopeCallback, this));
    }

    // Pop too many times
    {
        dawnDevicePushErrorScope(device, DAWN_ERROR_FILTER_VALIDATION);
        EXPECT_CALL(api, DevicePushErrorScope(apiDevice, DAWN_ERROR_FILTER_VALIDATION)).Times(1);

        EXPECT_TRUE(dawnDevicePopErrorScope(device, ToMockDevicePopErrorScopeCallback, this));
        EXPECT_FALSE(dawnDevicePopErrorScope(device, ToMockDevicePopErrorScopeCallback, this + 1));

        DawnErrorCallback callback;
        void* userdata;
        EXPECT_CALL(api, OnDevicePopErrorScopeCallback(apiDevice, _, _))
            .WillOnce(DoAll(SaveArg<1>(&callback), SaveArg<2>(&userdata), Return(true)));

        FlushClient();

        callback(DAWN_ERROR_TYPE_VALIDATION, "Some error message", userdata);
        EXPECT_CALL(*mockDevicePopErrorScopeCallback, Call(DAWN_ERROR_TYPE_VALIDATION, StrEq("Some error message"), this)).Times(1);

        FlushServer();
    }
}
