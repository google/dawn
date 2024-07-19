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
#include <utility>

#include "dawn/common/FutureUtils.h"
#include "dawn/tests/unittests/wire/WireFutureTest.h"
#include "dawn/tests/unittests/wire/WireTest.h"
#include "dawn/wire/WireClient.h"

namespace dawn::wire {
namespace {

using testing::_;
using testing::DoAll;
using testing::InvokeWithoutArgs;
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
        mockDeviceLostCallback = std::make_unique<StrictMock<MockDeviceLostCallback>>();
    }

    void TearDown() override {
        WireTest::TearDown();

        mockDeviceErrorCallback = nullptr;
        mockDeviceLoggingCallback = nullptr;
        mockDeviceLostCallback = nullptr;
    }

    void FlushServer() {
        WireTest::FlushServer();

        Mock::VerifyAndClearExpectations(&mockDeviceErrorCallback);
    }
};

// Test the return wire for device validation error callbacks
TEST_F(WireErrorCallbackTests, DeviceValidationErrorCallback) {
    device.SetUncapturedErrorCallback(ToMockDeviceErrorCallback, this);

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
    device.SetUncapturedErrorCallback(ToMockDeviceErrorCallback, this);

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
    device.SetUncapturedErrorCallback(ToMockDeviceErrorCallback, this);

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
    device.SetLoggingCallback(ToMockDeviceLoggingCallback, this);

    // Setting the injected warning callback should stay on the client side and do nothing
    FlushClient();

    // Calling the callback on the server side will result in the callback being called on the
    // client side
    api.CallDeviceSetLoggingCallbackCallback(apiDevice, WGPULoggingType_Info, "Some message");

    EXPECT_CALL(*mockDeviceLoggingCallback, Call(WGPULoggingType_Info, StrEq("Some message"), this))
        .Times(1);

    FlushServer();
}

// Test the return wire for device lost callback
TEST_F(WireErrorCallbackTests, DeviceLostCallback) {
    wgpuDeviceSetDeviceLostCallback(cDevice, ToMockDeviceLostCallback, this);

    // Setting the error callback should stay on the client side and do nothing
    FlushClient();

    // Calling the callback on the server side will result in the callback being called on the
    // client side
    api.CallDeviceSetDeviceLostCallbackCallback(apiDevice, WGPUDeviceLostReason_Unknown,
                                                "Some error message");

    EXPECT_CALL(*mockDeviceLostCallback,
                Call(WGPUDeviceLostReason_Unknown, StrEq("Some error message"), this))
        .Times(1);

    FlushServer();
}

class WirePopErrorScopeCallbackTests : public WireFutureTestWithParamsBase<> {
  protected:
    // Overridden version of wgpuDevicePopErrorScope that defers to the API call based on the test
    // callback mode.
    void DevicePopErrorScope(const wgpu::Device& d, void* userdata = nullptr) {
        if (IsAsync()) {
            d.PopErrorScope(mMockOldCb.Callback(), mMockOldCb.MakeUserdata(userdata));
        } else {
            wgpu::PopErrorScopeCallbackInfo callbackInfo = {};
            callbackInfo.mode =
                static_cast<wgpu::CallbackMode>(ToWGPUCallbackMode(GetParam().mCallbackMode));
            callbackInfo.callback = mMockCb.Callback();
            callbackInfo.userdata = mMockCb.MakeUserdata(userdata);
            this->mFutureIDs.push_back(d.PopErrorScope(callbackInfo).id);
        }
    }

    void PushErrorScope(wgpu::ErrorFilter filter) {
        EXPECT_CALL(api, DevicePushErrorScope(apiDevice, static_cast<WGPUErrorFilter>(filter)))
            .Times(1);
        device.PushErrorScope(filter);
        FlushClient();
    }

    void ExpectWireCallbacksWhen(
        std::function<void(testing::MockCallback<WGPUErrorCallback>&)> oldExp,
        std::function<void(testing::MockCallback<WGPUPopErrorScopeCallback>&)> exp) {
        if (IsAsync()) {
            oldExp(mMockOldCb);
            ASSERT_TRUE(testing::Mock::VerifyAndClearExpectations(&mMockOldCb));
        } else {
            exp(mMockCb);
            ASSERT_TRUE(testing::Mock::VerifyAndClearExpectations(&mMockCb));
        }
    }

  private:
    testing::MockCallback<WGPUPopErrorScopeCallback> mMockCb;
    testing::MockCallback<WGPUErrorCallback> mMockOldCb;
};
DAWN_INSTANTIATE_WIRE_FUTURE_TEST_P(WirePopErrorScopeCallbackTests);

// Test the return wire for validation error scopes.
TEST_P(WirePopErrorScopeCallbackTests, TypeAndFilters) {
    static constexpr std::array<std::pair<WGPUErrorType, wgpu::ErrorFilter>, 3>
        kErrorTypeAndFilters = {{{WGPUErrorType_Validation, wgpu::ErrorFilter::Validation},
                                 {WGPUErrorType_OutOfMemory, wgpu::ErrorFilter::OutOfMemory},
                                 {WGPUErrorType_Internal, wgpu::ErrorFilter::Internal}}};

    for (const auto& [type, filter] : kErrorTypeAndFilters) {
        PushErrorScope(filter);

        DevicePopErrorScope(device, this);
        EXPECT_CALL(api, OnDevicePopErrorScope2(apiDevice, _)).WillOnce([&] {
            api.CallDevicePopErrorScope2Callback(apiDevice, WGPUPopErrorScopeStatus_Success, type,
                                                 "Some error message");
        });

        FlushClient();
        FlushFutures();
        ExpectWireCallbacksWhen(
            [&](auto& oldMockCb) {
                EXPECT_CALL(oldMockCb, Call(type, StrEq("Some error message"), this)).Times(1);

                FlushCallbacks();
            },
            [&](auto& mockCb) {
                EXPECT_CALL(mockCb, Call(WGPUPopErrorScopeStatus_Success, type,
                                         StrEq("Some error message"), this))
                    .Times(1);

                FlushCallbacks();
            });
    }
}

// Wire disconnect before server response calls the callback with Unknown error type.
// TODO(crbug.com/dawn/2021) When using new callback signature, check for InstanceDropped status.
TEST_P(WirePopErrorScopeCallbackTests, DisconnectBeforeServerReply) {
    PushErrorScope(wgpu::ErrorFilter::Validation);

    DevicePopErrorScope(device, this);
    EXPECT_CALL(api, OnDevicePopErrorScope2(apiDevice, _)).Times(1);

    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen(
        [&](auto& oldMockCb) {
            EXPECT_CALL(oldMockCb, Call(WGPUErrorType_Unknown, nullptr, this)).Times(1);

            GetWireClient()->Disconnect();
        },
        [&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(WGPUPopErrorScopeStatus_InstanceDropped, WGPUErrorType_Unknown,
                                     nullptr, this))
                .Times(1);

            GetWireClient()->Disconnect();
        });
}

// Wire disconnect after server response calls the callback with returned error type.
TEST_P(WirePopErrorScopeCallbackTests, DisconnectAfterServerReply) {
    // On Async and Spontaneous mode, it is not possible to simulate this because on the server
    // reponse, the callback would also be fired.
    DAWN_SKIP_TEST_IF(IsSpontaneous());

    PushErrorScope(wgpu::ErrorFilter::Validation);

    DevicePopErrorScope(device, this);
    EXPECT_CALL(api, OnDevicePopErrorScope2(apiDevice, _)).WillOnce(InvokeWithoutArgs([&] {
        api.CallDevicePopErrorScope2Callback(apiDevice, WGPUPopErrorScopeStatus_Success,
                                             WGPUErrorType_Validation, "Some error message");
    }));

    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen(
        [&](auto& oldMockCb) {
            EXPECT_CALL(oldMockCb, Call(WGPUErrorType_Validation, nullptr, this)).Times(1);

            GetWireClient()->Disconnect();
        },
        [&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(WGPUPopErrorScopeStatus_InstanceDropped,
                                     WGPUErrorType_Validation, nullptr, this))
                .Times(1);

            GetWireClient()->Disconnect();
        });
}

// Empty stack (We are emulating the errors that would be callback-ed from native).
TEST_P(WirePopErrorScopeCallbackTests, EmptyStack) {
    DevicePopErrorScope(cDevice, this);
    EXPECT_CALL(api, OnDevicePopErrorScope2(apiDevice, _)).WillOnce(InvokeWithoutArgs([&] {
        api.CallDevicePopErrorScope2Callback(apiDevice, WGPUPopErrorScopeStatus_Success,
                                             WGPUErrorType_Validation, "No error scopes to pop");
    }));

    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen(
        [&](auto& oldMockCb) {
            EXPECT_CALL(oldMockCb,
                        Call(WGPUErrorType_Validation, StrEq("No error scopes to pop"), this))
                .Times(1);

            FlushCallbacks();
        },
        [&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call(WGPUPopErrorScopeStatus_Success, WGPUErrorType_Validation,
                                     StrEq("No error scopes to pop"), this))
                .Times(1);

            FlushCallbacks();
        });
}

}  // anonymous namespace
}  // namespace dawn::wire
