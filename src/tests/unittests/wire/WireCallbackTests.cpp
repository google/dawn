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

    class MockBuilderErrorCallback {
      public:
        MOCK_METHOD4(Call,
                     void(dawnBuilderErrorStatus status,
                          const char* message,
                          dawnCallbackUserdata userdata1,
                          dawnCallbackUserdata userdata2));
    };

    std::unique_ptr<MockBuilderErrorCallback> mockBuilderErrorCallback;
    void ToMockBuilderErrorCallback(dawnBuilderErrorStatus status,
                                    const char* message,
                                    dawnCallbackUserdata userdata1,
                                    dawnCallbackUserdata userdata2) {
        mockBuilderErrorCallback->Call(status, message, userdata1, userdata2);
    }

}  // anonymous namespace

class WireCallbackTests : public WireTest {
  public:
    WireCallbackTests() : WireTest(true) {
    }
    ~WireCallbackTests() override = default;

    void SetUp() override {
        WireTest::SetUp();

        mockDeviceErrorCallback = std::make_unique<MockDeviceErrorCallback>();
        mockBuilderErrorCallback = std::make_unique<MockBuilderErrorCallback>();
    }

    void TearDown() override {
        WireTest::TearDown();

        // Delete mocks so that expectations are checked
        mockDeviceErrorCallback = nullptr;
        mockBuilderErrorCallback = nullptr;
    }
};

// Test that we get a success builder error status when no error happens
TEST_F(WireCallbackTests, SuccessCallbackOnBuilderSuccess) {
    dawnBufferBuilder bufferBuilder = dawnDeviceCreateBufferBuilderForTesting(device);
    dawnBufferBuilderSetErrorCallback(bufferBuilder, ToMockBuilderErrorCallback, 1, 2);
    dawnBufferBuilderGetResult(bufferBuilder);

    dawnBufferBuilder apiBufferBuilder = api.GetNewBufferBuilder();
    EXPECT_CALL(api, DeviceCreateBufferBuilderForTesting(apiDevice))
        .WillOnce(Return(apiBufferBuilder));

    dawnBuffer apiBuffer = api.GetNewBuffer();
    EXPECT_CALL(api, BufferBuilderGetResult(apiBufferBuilder))
        .WillOnce(InvokeWithoutArgs([&]() -> dawnBuffer {
            api.CallBuilderErrorCallback(apiBufferBuilder, DAWN_BUILDER_ERROR_STATUS_SUCCESS,
                                         "I like cheese");
            return apiBuffer;
        }));

    EXPECT_CALL(api, BufferBuilderRelease(apiBufferBuilder));
    EXPECT_CALL(api, BufferRelease(apiBuffer));
    FlushClient();

    EXPECT_CALL(*mockBuilderErrorCallback, Call(DAWN_BUILDER_ERROR_STATUS_SUCCESS, _, 1, 2));

    FlushServer();
}

// Test that the client calls the builder callback with unknown when it HAS to fire the callback but
// can't know the status yet.
TEST_F(WireCallbackTests, UnknownBuilderErrorStatusCallback) {
    // The builder is destroyed before the object is built
    {
        dawnBufferBuilder bufferBuilder = dawnDeviceCreateBufferBuilderForTesting(device);
        dawnBufferBuilderSetErrorCallback(bufferBuilder, ToMockBuilderErrorCallback, 1, 2);

        EXPECT_CALL(*mockBuilderErrorCallback, Call(DAWN_BUILDER_ERROR_STATUS_UNKNOWN, _, 1, 2))
            .Times(1);

        dawnBufferBuilderRelease(bufferBuilder);
    }

    // If the builder has been consumed, it doesn't fire the callback with unknown
    {
        dawnBufferBuilder bufferBuilder = dawnDeviceCreateBufferBuilderForTesting(device);
        dawnBufferBuilderSetErrorCallback(bufferBuilder, ToMockBuilderErrorCallback, 3, 4);
        dawnBufferBuilderGetResult(bufferBuilder);

        EXPECT_CALL(*mockBuilderErrorCallback, Call(DAWN_BUILDER_ERROR_STATUS_UNKNOWN, _, 3, 4))
            .Times(0);

        dawnBufferBuilderRelease(bufferBuilder);
    }

    // If the builder has been consumed, and the object is destroyed before the result comes from
    // the server, then the callback is fired with unknown
    {
        dawnBufferBuilder bufferBuilder = dawnDeviceCreateBufferBuilderForTesting(device);
        dawnBufferBuilderSetErrorCallback(bufferBuilder, ToMockBuilderErrorCallback, 5, 6);
        dawnBuffer buffer = dawnBufferBuilderGetResult(bufferBuilder);

        EXPECT_CALL(*mockBuilderErrorCallback, Call(DAWN_BUILDER_ERROR_STATUS_UNKNOWN, _, 5, 6))
            .Times(1);

        dawnBufferRelease(buffer);
    }
}

// Test that a builder success status doesn't get forwarded to the device
TEST_F(WireCallbackTests, SuccessCallbackNotForwardedToDevice) {
    dawnDeviceSetErrorCallback(device, ToMockDeviceErrorCallback, 0);

    dawnBufferBuilder bufferBuilder = dawnDeviceCreateBufferBuilderForTesting(device);
    dawnBufferBuilderGetResult(bufferBuilder);

    dawnBufferBuilder apiBufferBuilder = api.GetNewBufferBuilder();
    EXPECT_CALL(api, DeviceCreateBufferBuilderForTesting(apiDevice))
        .WillOnce(Return(apiBufferBuilder));

    dawnBuffer apiBuffer = api.GetNewBuffer();
    EXPECT_CALL(api, BufferBuilderGetResult(apiBufferBuilder))
        .WillOnce(InvokeWithoutArgs([&]() -> dawnBuffer {
            api.CallBuilderErrorCallback(apiBufferBuilder, DAWN_BUILDER_ERROR_STATUS_SUCCESS,
                                         "I like cheese");
            return apiBuffer;
        }));

    EXPECT_CALL(api, BufferBuilderRelease(apiBufferBuilder));
    EXPECT_CALL(api, BufferRelease(apiBuffer));
    FlushClient();
    FlushServer();
}

// Test that a builder error status gets forwarded to the device
TEST_F(WireCallbackTests, ErrorCallbackForwardedToDevice) {
    uint64_t userdata = 30495;
    dawnDeviceSetErrorCallback(device, ToMockDeviceErrorCallback, userdata);

    dawnBufferBuilder bufferBuilder = dawnDeviceCreateBufferBuilderForTesting(device);
    dawnBufferBuilderGetResult(bufferBuilder);

    dawnBufferBuilder apiBufferBuilder = api.GetNewBufferBuilder();
    EXPECT_CALL(api, DeviceCreateBufferBuilderForTesting(apiDevice))
        .WillOnce(Return(apiBufferBuilder));

    EXPECT_CALL(api, BufferBuilderGetResult(apiBufferBuilder))
        .WillOnce(InvokeWithoutArgs([&]() -> dawnBuffer {
            api.CallBuilderErrorCallback(apiBufferBuilder, DAWN_BUILDER_ERROR_STATUS_ERROR,
                                         "Error :(");
            return nullptr;
        }));

    EXPECT_CALL(api, BufferBuilderRelease(apiBufferBuilder));
    FlushClient();

    EXPECT_CALL(*mockDeviceErrorCallback, Call(_, userdata)).Times(1);

    FlushServer();
}

// Test the return wire for device error callbacks
TEST_F(WireCallbackTests, DeviceErrorCallback) {
    uint64_t userdata = 3049785;
    dawnDeviceSetErrorCallback(device, ToMockDeviceErrorCallback, userdata);

    // Setting the error callback should stay on the client side and do nothing
    FlushClient();

    // Calling the callback on the server side will result in the callback being called on the
    // client side
    api.CallDeviceErrorCallback(apiDevice, "Some error message");

    EXPECT_CALL(*mockDeviceErrorCallback, Call(StrEq("Some error message"), userdata)).Times(1);

    FlushServer();
}

// Test the return wire for device error callbacks
TEST_F(WireCallbackTests, BuilderErrorCallback) {
    uint64_t userdata1 = 982734;
    uint64_t userdata2 = 982734239028;

    // Create the buffer builder, the callback is set immediately on the server side
    dawnBufferBuilder bufferBuilder = dawnDeviceCreateBufferBuilderForTesting(device);

    dawnBufferBuilder apiBufferBuilder = api.GetNewBufferBuilder();
    EXPECT_CALL(api, DeviceCreateBufferBuilderForTesting(apiDevice))
        .WillOnce(Return(apiBufferBuilder));

    EXPECT_CALL(api, OnBuilderSetErrorCallback(apiBufferBuilder, _, _, _)).Times(1);

    FlushClient();

    // Setting the callback on the client side doesn't do anything on the server side
    dawnBufferBuilderSetErrorCallback(bufferBuilder, ToMockBuilderErrorCallback, userdata1,
                                      userdata2);
    FlushClient();

    // Create an object so that it is a valid case to call the error callback
    dawnBufferBuilderGetResult(bufferBuilder);

    dawnBuffer apiBuffer = api.GetNewBuffer();
    EXPECT_CALL(api, BufferBuilderGetResult(apiBufferBuilder))
        .WillOnce(InvokeWithoutArgs([&]() -> dawnBuffer {
            api.CallBuilderErrorCallback(apiBufferBuilder, DAWN_BUILDER_ERROR_STATUS_SUCCESS,
                                         "Success!");
            return apiBuffer;
        }));

    EXPECT_CALL(api, BufferBuilderRelease(apiBufferBuilder));
    EXPECT_CALL(api, BufferRelease(apiBuffer));
    FlushClient();

    // The error callback gets called on the client side
    EXPECT_CALL(*mockBuilderErrorCallback,
                Call(DAWN_BUILDER_ERROR_STATUS_SUCCESS, StrEq("Success!"), userdata1, userdata2))
        .Times(1);

    FlushServer();
}

// Test that the server doesn't forward calls to error objects or with error objects
// Also test that when GetResult is called on an error builder, the error callback is fired
// TODO(cwallez@chromium.org): This test is disabled because the introduction of encoders breaks
// the assumptions of the "builder error" handling that a builder is self-contained. We need to
// revisit this once the new error handling is in place.
TEST_F(WireCallbackTests, DISABLED_CallsSkippedAfterBuilderError) {
    dawnCommandBufferBuilder cmdBufBuilder = dawnDeviceCreateCommandBufferBuilder(device);
    dawnCommandBufferBuilderSetErrorCallback(cmdBufBuilder, ToMockBuilderErrorCallback, 1, 2);

    dawnRenderPassEncoder pass = dawnCommandBufferBuilderBeginRenderPass(cmdBufBuilder, nullptr);

    dawnBufferBuilder bufferBuilder = dawnDeviceCreateBufferBuilderForTesting(device);
    dawnBufferBuilderSetErrorCallback(bufferBuilder, ToMockBuilderErrorCallback, 3, 4);
    dawnBuffer buffer = dawnBufferBuilderGetResult(bufferBuilder);  // Hey look an error!

    // These calls will be skipped because of the error
    dawnBufferSetSubData(buffer, 0, 0, nullptr);
    dawnRenderPassEncoderSetIndexBuffer(pass, buffer, 0);
    dawnRenderPassEncoderEndPass(pass);
    dawnCommandBufferBuilderGetResult(cmdBufBuilder);

    dawnCommandBufferBuilder apiCmdBufBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiCmdBufBuilder));

    dawnRenderPassEncoder apiPass = api.GetNewRenderPassEncoder();
    EXPECT_CALL(api, CommandBufferBuilderBeginRenderPass(apiCmdBufBuilder, _))
        .WillOnce(Return(apiPass));

    dawnBufferBuilder apiBufferBuilder = api.GetNewBufferBuilder();
    EXPECT_CALL(api, DeviceCreateBufferBuilderForTesting(apiDevice))
        .WillOnce(Return(apiBufferBuilder));

    // Hey look an error!
    EXPECT_CALL(api, BufferBuilderGetResult(apiBufferBuilder))
        .WillOnce(InvokeWithoutArgs([&]() -> dawnBuffer {
            api.CallBuilderErrorCallback(apiBufferBuilder, DAWN_BUILDER_ERROR_STATUS_ERROR,
                                         "Error");
            return nullptr;
        }));

    EXPECT_CALL(api, BufferSetSubData(_, _, _, _)).Times(0);
    EXPECT_CALL(api, RenderPassEncoderSetIndexBuffer(_, _, _)).Times(0);
    EXPECT_CALL(api, CommandBufferBuilderGetResult(_)).Times(0);

    FlushClient();

    EXPECT_CALL(*mockBuilderErrorCallback, Call(DAWN_BUILDER_ERROR_STATUS_ERROR, _, 1, 2)).Times(1);
    EXPECT_CALL(*mockBuilderErrorCallback, Call(DAWN_BUILDER_ERROR_STATUS_ERROR, _, 3, 4)).Times(1);

    FlushServer();
}
