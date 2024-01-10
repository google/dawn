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

#include "dawn/tests/unittests/wire/WireTest.h"

#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireServer.h"

namespace dawn::wire {
namespace {

using testing::_;
using testing::Exactly;
using testing::Mock;
using testing::Return;

class WireInjectDeviceTests : public WireTest {
  public:
    WireInjectDeviceTests() {}
    ~WireInjectDeviceTests() override = default;
};

// Test that reserving and injecting a device makes calls on the client object forward to the
// server object correctly.
TEST_F(WireInjectDeviceTests, CallAfterReserveInject) {
    ReservedDevice reservation = GetWireClient()->ReserveDevice();

    WGPUDevice serverDevice = api.GetNewDevice();
    EXPECT_CALL(api, DeviceReference(serverDevice));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice, _, _));
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice, _, _));
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
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice, nullptr, nullptr)).Times(Exactly(1));
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
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice, _, _));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice, _, _));
    ASSERT_TRUE(
        GetWireServer()->InjectDevice(serverDevice, reservation.id, reservation.generation));

    // ID already in use, call fails.
    ASSERT_FALSE(
        GetWireServer()->InjectDevice(serverDevice, reservation.id, reservation.generation));

    // Called on shutdown.
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice, nullptr, nullptr))
        .Times(Exactly(1));
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice, nullptr, nullptr)).Times(Exactly(1));
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
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice, _, _));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice, _, _));
    ASSERT_TRUE(
        GetWireServer()->InjectDevice(serverDevice, reservation.id, reservation.generation));

    // Releasing the device removes a single reference and clears its error callbacks.
    wgpuDeviceRelease(reservation.device);
    EXPECT_CALL(api, DeviceRelease(serverDevice));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice, nullptr, nullptr)).Times(1);
    FlushClient();

    // Deleting the server doesn't release a second reference.
    DeleteServer();
    Mock::VerifyAndClearExpectations(&api);
}

// Test that it is an error to get the primary queue of a device before it has been
// injected on the server.
TEST_F(WireInjectDeviceTests, GetQueueBeforeInject) {
    ReservedDevice reservation = GetWireClient()->ReserveDevice();

    wgpuDeviceGetQueue(reservation.device);
    FlushClient(false);
}

// Test that it is valid to get the primary queue of a device after it has been
// injected on the server.
TEST_F(WireInjectDeviceTests, GetQueueAfterInject) {
    ReservedDevice reservation = GetWireClient()->ReserveDevice();

    WGPUDevice serverDevice = api.GetNewDevice();
    EXPECT_CALL(api, DeviceReference(serverDevice));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice, _, _));
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice, _, _));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice, _, _));
    ASSERT_TRUE(
        GetWireServer()->InjectDevice(serverDevice, reservation.id, reservation.generation));

    wgpuDeviceGetQueue(reservation.device);

    WGPUQueue apiQueue = api.GetNewQueue();
    EXPECT_CALL(api, DeviceGetQueue(serverDevice)).WillOnce(Return(apiQueue));
    FlushClient();

    // Called on shutdown.
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice, nullptr, nullptr))
        .Times(Exactly(1));
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice, nullptr, nullptr)).Times(Exactly(1));
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
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice1, _, _));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice1, _, _));
    ASSERT_TRUE(
        GetWireServer()->InjectDevice(serverDevice1, reservation1.id, reservation1.generation));

    WGPUDevice serverDevice2 = api.GetNewDevice();
    EXPECT_CALL(api, DeviceReference(serverDevice2));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice2, _, _));
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice2, _, _));
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
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice1, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice1, nullptr, nullptr)).Times(1);
    FlushClient();

    // The first device should no longer reflect, but the second should
    ASSERT_EQ(nullptr, GetWireServer()->GetDevice(reservation1.id, reservation1.generation));
    ASSERT_EQ(serverDevice2, GetWireServer()->GetDevice(reservation2.id, reservation2.generation));

    // Called on shutdown.
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice2, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice2, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice2, nullptr, nullptr)).Times(1);
}

// This is a regression test where a second device reservation invalidated pointers into the
// KnownObjects std::vector of devices. The fix was to store pointers to heap allocated
// objects instead.
TEST_F(WireInjectDeviceTests, TrackChildObjectsWithTwoReservedDevices) {
    // Reserve one device, inject it, and get the primary queue.
    ReservedDevice reservation1 = GetWireClient()->ReserveDevice();

    WGPUDevice serverDevice1 = api.GetNewDevice();
    EXPECT_CALL(api, DeviceReference(serverDevice1));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice1, _, _));
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice1, _, _));
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
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice2, _, _));
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
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice1, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice1, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice2, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice2, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice2, nullptr, nullptr)).Times(1);
}

// Test that a device reservation can be reclaimed. This is necessary to
// avoid leaking ObjectIDs for reservations that are never injected.
TEST_F(WireInjectDeviceTests, ReclaimDeviceReservation) {
    // Test that doing a reservation and full release is an error.
    {
        ReservedDevice reservation = GetWireClient()->ReserveDevice();
        wgpuDeviceRelease(reservation.device);
        FlushClient(false);
    }

    // Test that doing a reservation and then reclaiming it recycles the ID.
    {
        ReservedDevice reservation1 = GetWireClient()->ReserveDevice();
        GetWireClient()->ReclaimDeviceReservation(reservation1);

        ReservedDevice reservation2 = GetWireClient()->ReserveDevice();

        // The ID is the same, but the generation is still different.
        ASSERT_EQ(reservation1.id, reservation2.id);
        ASSERT_NE(reservation1.generation, reservation2.generation);

        // No errors should occur.
        FlushClient();
    }
}

}  // anonymous namespace
}  // namespace dawn::wire
