// Copyright 2020 The Dawn Authors
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

#include "common/Assert.h"
#include "dawn_wire/WireClient.h"
#include "tests/MockCallback.h"

using namespace testing;
using namespace dawn_wire;

namespace {

    class WireDisconnectTests : public WireTest {};

}  // anonymous namespace

// Test that commands are not received if the client disconnects.
TEST_F(WireDisconnectTests, CommandsAfterDisconnect) {
    // Sanity check that commands work at all.
    wgpuDeviceCreateCommandEncoder(device, nullptr);

    WGPUCommandEncoder apiCmdBufEncoder = api.GetNewCommandEncoder();
    EXPECT_CALL(api, DeviceCreateCommandEncoder(apiDevice, nullptr))
        .WillOnce(Return(apiCmdBufEncoder));
    FlushClient();

    // Disconnect.
    GetWireClient()->Disconnect();

    // Command is not received because client disconnected.
    wgpuDeviceCreateCommandEncoder(device, nullptr);
    EXPECT_CALL(api, DeviceCreateCommandEncoder(_, _)).Times(Exactly(0));
    FlushClient();
}

// Test that commands that are serialized before a disconnect but flushed
// after are received.
TEST_F(WireDisconnectTests, FlushAfterDisconnect) {
    // Sanity check that commands work at all.
    wgpuDeviceCreateCommandEncoder(device, nullptr);

    // Disconnect.
    GetWireClient()->Disconnect();

    // Already-serialized commmands are still received.
    WGPUCommandEncoder apiCmdBufEncoder = api.GetNewCommandEncoder();
    EXPECT_CALL(api, DeviceCreateCommandEncoder(apiDevice, nullptr))
        .WillOnce(Return(apiCmdBufEncoder));
    FlushClient();
}

// Check that disconnecting the wire client calls the device lost callback exacty once.
TEST_F(WireDisconnectTests, CallsDeviceLostCallback) {
    MockCallback<WGPUDeviceLostCallback> mockDeviceLostCallback;
    wgpuDeviceSetDeviceLostCallback(device, mockDeviceLostCallback.Callback(),
                                    mockDeviceLostCallback.MakeUserdata(this));

    // Disconnect the wire client. We should receive device lost only once.
    EXPECT_CALL(mockDeviceLostCallback, Call(_, this)).Times(Exactly(1));
    GetWireClient()->Disconnect();
    GetWireClient()->Disconnect();
}

// Check that disconnecting the wire client after a device loss does not trigger the callback again.
TEST_F(WireDisconnectTests, ServerLostThenDisconnect) {
    MockCallback<WGPUDeviceLostCallback> mockDeviceLostCallback;
    wgpuDeviceSetDeviceLostCallback(device, mockDeviceLostCallback.Callback(),
                                    mockDeviceLostCallback.MakeUserdata(this));

    api.CallDeviceSetDeviceLostCallbackCallback(apiDevice, "some reason");

    // Flush the device lost return command.
    EXPECT_CALL(mockDeviceLostCallback, Call(StrEq("some reason"), this)).Times(Exactly(1));
    FlushServer();

    // Disconnect the client. We shouldn't see the lost callback again.
    EXPECT_CALL(mockDeviceLostCallback, Call(_, _)).Times(Exactly(0));
    GetWireClient()->Disconnect();
}

// Check that disconnecting the wire client inside the device loss callback does not trigger the
// callback again.
TEST_F(WireDisconnectTests, ServerLostThenDisconnectInCallback) {
    MockCallback<WGPUDeviceLostCallback> mockDeviceLostCallback;
    wgpuDeviceSetDeviceLostCallback(device, mockDeviceLostCallback.Callback(),
                                    mockDeviceLostCallback.MakeUserdata(this));

    api.CallDeviceSetDeviceLostCallbackCallback(apiDevice, "lost reason");

    // Disconnect the client inside the lost callback. We should see the callback
    // only once.
    EXPECT_CALL(mockDeviceLostCallback, Call(StrEq("lost reason"), this))
        .WillOnce(InvokeWithoutArgs([&]() {
            EXPECT_CALL(mockDeviceLostCallback, Call(_, _)).Times(Exactly(0));
            GetWireClient()->Disconnect();
        }));
    FlushServer();
}

// Check that a device loss after a disconnect does not trigger the callback again.
TEST_F(WireDisconnectTests, DisconnectThenServerLost) {
    MockCallback<WGPUDeviceLostCallback> mockDeviceLostCallback;
    wgpuDeviceSetDeviceLostCallback(device, mockDeviceLostCallback.Callback(),
                                    mockDeviceLostCallback.MakeUserdata(this));

    // Disconnect the client. We should see the callback once.
    EXPECT_CALL(mockDeviceLostCallback, Call(_, this)).Times(Exactly(1));
    GetWireClient()->Disconnect();

    // Lose the device on the server. The client callback shouldn't be
    // called again.
    api.CallDeviceSetDeviceLostCallbackCallback(apiDevice, "lost reason");
    EXPECT_CALL(mockDeviceLostCallback, Call(_, _)).Times(Exactly(0));
    FlushServer();
}

// Test that client objects are all destroyed if the WireClient is destroyed.
TEST_F(WireDisconnectTests, DeleteClientDestroysObjects) {
    WGPUSamplerDescriptor desc = {};
    wgpuDeviceCreateCommandEncoder(device, nullptr);
    wgpuDeviceCreateSampler(device, &desc);

    WGPUCommandEncoder apiCommandEncoder = api.GetNewCommandEncoder();
    EXPECT_CALL(api, DeviceCreateCommandEncoder(apiDevice, nullptr))
        .WillOnce(Return(apiCommandEncoder));

    WGPUSampler apiSampler = api.GetNewSampler();
    EXPECT_CALL(api, DeviceCreateSampler(apiDevice, _)).WillOnce(Return(apiSampler));

    FlushClient();

    DeleteClient();

    // Expect release on all objects created by the client.
    Sequence s1, s2, s3;
    EXPECT_CALL(api, QueueRelease(apiQueue)).Times(1).InSequence(s1);
    EXPECT_CALL(api, CommandEncoderRelease(apiCommandEncoder)).Times(1).InSequence(s2);
    EXPECT_CALL(api, SamplerRelease(apiSampler)).Times(1).InSequence(s3);
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(apiDevice, nullptr, nullptr))
        .Times(1)
        .InSequence(s1, s2);
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(apiDevice, nullptr, nullptr))
        .Times(1)
        .InSequence(s1, s2);
    EXPECT_CALL(api, DeviceRelease(apiDevice)).Times(1).InSequence(s1, s2, s3);
    FlushClient();

    // Signal that we already released and cleared callbacks for |apiDevice|
    DefaultApiDeviceWasReleased();
}
