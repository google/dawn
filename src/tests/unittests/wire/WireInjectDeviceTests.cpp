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

#include "tests/unittests/wire/WireTest.h"

#include "dawn_wire/WireClient.h"
#include "dawn_wire/WireServer.h"

using namespace testing;
using namespace dawn_wire;

class WireInjectDeviceTests : public WireTest {
  public:
    WireInjectDeviceTests() {
    }
    ~WireInjectDeviceTests() override = default;
};

// Test that reserving and injecting a device makes calls on the client object forward to the
// server object correctly.
TEST_F(WireInjectDeviceTests, CallAfterReserveInject) {
    ReservedDevice reservation = GetWireClient()->ReserveDevice();

    WGPUDevice serverDevice = api.GetNewDevice();
    EXPECT_CALL(api, DeviceReference(serverDevice));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice, _, _));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice, _, _));
    ASSERT_TRUE(
        GetWireServer()->InjectDevice(serverDevice, reservation.id, reservation.generation));

    WGPUBufferDescriptor bufferDesc = {};
    wgpuDeviceCreateBuffer(reservation.device, &bufferDesc);
    WGPUBuffer serverBuffer = api.GetNewBuffer();
    EXPECT_CALL(api, DeviceCreateBuffer(serverDevice, _)).WillOnce(Return(serverBuffer));
    FlushClient();

    // Called on shutdown.
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice, nullptr, nullptr))
        .Times(Exactly(1));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice, nullptr, nullptr))
        .Times(Exactly(1));
}

// Test that reserve correctly returns different IDs each time.
TEST_F(WireInjectDeviceTests, ReserveDifferentIDs) {
    ReservedDevice reservation1 = GetWireClient()->ReserveDevice();
    ReservedDevice reservation2 = GetWireClient()->ReserveDevice();

    ASSERT_NE(reservation1.id, reservation2.id);
    ASSERT_NE(reservation1.device, reservation2.device);
}

// Test that injecting the same id without a destroy first fails.
TEST_F(WireInjectDeviceTests, InjectExistingID) {
    ReservedDevice reservation = GetWireClient()->ReserveDevice();

    WGPUDevice serverDevice = api.GetNewDevice();
    EXPECT_CALL(api, DeviceReference(serverDevice));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice, _, _));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice, _, _));
    ASSERT_TRUE(
        GetWireServer()->InjectDevice(serverDevice, reservation.id, reservation.generation));

    // ID already in use, call fails.
    ASSERT_FALSE(
        GetWireServer()->InjectDevice(serverDevice, reservation.id, reservation.generation));

    // Called on shutdown.
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice, nullptr, nullptr))
        .Times(Exactly(1));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice, nullptr, nullptr))
        .Times(Exactly(1));
}

// Test that the server only borrows the device and does a single reference-release
TEST_F(WireInjectDeviceTests, InjectedDeviceLifetime) {
    ReservedDevice reservation = GetWireClient()->ReserveDevice();

    // Injecting the device adds a reference
    WGPUDevice serverDevice = api.GetNewDevice();
    EXPECT_CALL(api, DeviceReference(serverDevice));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice, _, _));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice, _, _));
    ASSERT_TRUE(
        GetWireServer()->InjectDevice(serverDevice, reservation.id, reservation.generation));

    // Releasing the device removes a single reference and clears its error callbacks.
    wgpuDeviceRelease(reservation.device);
    EXPECT_CALL(api, DeviceRelease(serverDevice));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice, nullptr, nullptr)).Times(1);
    FlushClient();

    // Deleting the server doesn't release a second reference.
    DeleteServer();
    Mock::VerifyAndClearExpectations(&api);
}

// Test that it is an error to get the default queue of a device before it has been
// injected on the server.
TEST_F(WireInjectDeviceTests, GetQueueBeforeInject) {
    ReservedDevice reservation = GetWireClient()->ReserveDevice();

    wgpuDeviceGetDefaultQueue(reservation.device);
    FlushClient(false);
}

// Test that it is valid to get the default queue of a device after it has been
// injected on the server.
TEST_F(WireInjectDeviceTests, GetQueueAfterInject) {
    ReservedDevice reservation = GetWireClient()->ReserveDevice();

    WGPUDevice serverDevice = api.GetNewDevice();
    EXPECT_CALL(api, DeviceReference(serverDevice));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice, _, _));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice, _, _));
    ASSERT_TRUE(
        GetWireServer()->InjectDevice(serverDevice, reservation.id, reservation.generation));

    wgpuDeviceGetDefaultQueue(reservation.device);

    WGPUQueue apiQueue = api.GetNewQueue();
    EXPECT_CALL(api, DeviceGetDefaultQueue(serverDevice)).WillOnce(Return(apiQueue));
    FlushClient();

    // Called on shutdown.
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice, nullptr, nullptr))
        .Times(Exactly(1));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice, nullptr, nullptr))
        .Times(Exactly(1));
}

// Test that the list of live devices can be reflected using GetDevice.
TEST_F(WireInjectDeviceTests, ReflectLiveDevices) {
    // Reserve two devices.
    ReservedDevice reservation1 = GetWireClient()->ReserveDevice();
    ReservedDevice reservation2 = GetWireClient()->ReserveDevice();

    // Inject both devices.

    WGPUDevice serverDevice1 = api.GetNewDevice();
    EXPECT_CALL(api, DeviceReference(serverDevice1));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice1, _, _));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice1, _, _));
    ASSERT_TRUE(
        GetWireServer()->InjectDevice(serverDevice1, reservation1.id, reservation1.generation));

    WGPUDevice serverDevice2 = api.GetNewDevice();
    EXPECT_CALL(api, DeviceReference(serverDevice2));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice2, _, _));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice2, _, _));
    ASSERT_TRUE(
        GetWireServer()->InjectDevice(serverDevice2, reservation2.id, reservation2.generation));

    // Test that both devices can be reflected.
    ASSERT_EQ(serverDevice1, GetWireServer()->GetDevice(reservation1.id, reservation1.generation));
    ASSERT_EQ(serverDevice2, GetWireServer()->GetDevice(reservation2.id, reservation2.generation));

    // Release the first device
    wgpuDeviceRelease(reservation1.device);
    EXPECT_CALL(api, DeviceRelease(serverDevice1));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice1, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice1, nullptr, nullptr)).Times(1);
    FlushClient();

    // The first device should no longer reflect, but the second should
    ASSERT_EQ(nullptr, GetWireServer()->GetDevice(reservation1.id, reservation1.generation));
    ASSERT_EQ(serverDevice2, GetWireServer()->GetDevice(reservation2.id, reservation2.generation));

    // Called on shutdown.
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice2, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice2, nullptr, nullptr)).Times(1);
}

// This is a regression test where a second device reservation invalidated pointers into the
// KnownObjects std::vector of devices. The fix was to store pointers to heap allocated
// objects instead.
TEST_F(WireInjectDeviceTests, TrackChildObjectsWithTwoReservedDevices) {
    // Reserve one device, inject it, and get the default queue.
    ReservedDevice reservation1 = GetWireClient()->ReserveDevice();

    WGPUDevice serverDevice1 = api.GetNewDevice();
    EXPECT_CALL(api, DeviceReference(serverDevice1));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice1, _, _));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice1, _, _));
    ASSERT_TRUE(
        GetWireServer()->InjectDevice(serverDevice1, reservation1.id, reservation1.generation));

    WGPUCommandEncoder commandEncoder =
        wgpuDeviceCreateCommandEncoder(reservation1.device, nullptr);

    WGPUCommandEncoder serverCommandEncoder = api.GetNewCommandEncoder();
    EXPECT_CALL(api, DeviceCreateCommandEncoder(serverDevice1, _))
        .WillOnce(Return(serverCommandEncoder));
    FlushClient();

    // Reserve a second device, and inject it.
    ReservedDevice reservation2 = GetWireClient()->ReserveDevice();

    WGPUDevice serverDevice2 = api.GetNewDevice();
    EXPECT_CALL(api, DeviceReference(serverDevice2));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice2, _, _));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice2, _, _));
    ASSERT_TRUE(
        GetWireServer()->InjectDevice(serverDevice2, reservation2.id, reservation2.generation));

    // Release the encoder. This should work without error because it stores a stable
    // pointer to its device's list of child objects. On destruction, it removes itself from the
    // list.
    wgpuCommandEncoderRelease(commandEncoder);
    EXPECT_CALL(api, CommandEncoderRelease(serverCommandEncoder));
    FlushClient();

    // Called on shutdown.
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice1, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice1, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice2, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice2, nullptr, nullptr)).Times(1);
}
