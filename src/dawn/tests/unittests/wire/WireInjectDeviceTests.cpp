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
    auto reserved = GetWireClient()->ReserveDevice(instance);

    WGPUDevice serverDevice = api.GetNewDevice();
    EXPECT_CALL(api, DeviceReference(serverDevice));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice, _, _));
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice, _, _));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice, _, _));
    ASSERT_TRUE(GetWireServer()->InjectDevice(serverDevice, reserved.reservation));

    WGPUBufferDescriptor bufferDesc = {};
    wgpuDeviceCreateBuffer(reserved.device, &bufferDesc);
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
    auto reserved1 = GetWireClient()->ReserveDevice(instance);
    auto reserved2 = GetWireClient()->ReserveDevice(instance);

    ASSERT_NE(reserved1.reservation.id, reserved2.reservation.id);
    ASSERT_NE(reserved1.device, reserved2.device);
}

// Test that injecting the same id without a destroy first fails.
TEST_F(WireInjectDeviceTests, InjectExistingID) {
    auto reserved = GetWireClient()->ReserveDevice(instance);

    WGPUDevice serverDevice = api.GetNewDevice();
    EXPECT_CALL(api, DeviceReference(serverDevice));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice, _, _));
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice, _, _));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice, _, _));
    ASSERT_TRUE(GetWireServer()->InjectDevice(serverDevice, reserved.reservation));

    // ID already in use, call fails.
    ASSERT_FALSE(GetWireServer()->InjectDevice(serverDevice, reserved.reservation));

    // Called on shutdown.
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice, nullptr, nullptr))
        .Times(Exactly(1));
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice, nullptr, nullptr)).Times(Exactly(1));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice, nullptr, nullptr))
        .Times(Exactly(1));
}

// Test that the server only borrows the device and does a single reference-release
TEST_F(WireInjectDeviceTests, InjectedDeviceLifetime) {
    auto reserved = GetWireClient()->ReserveDevice(instance);

    // Injecting the device adds a reference
    WGPUDevice serverDevice = api.GetNewDevice();
    EXPECT_CALL(api, DeviceReference(serverDevice));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice, _, _));
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice, _, _));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice, _, _));
    ASSERT_TRUE(GetWireServer()->InjectDevice(serverDevice, reserved.reservation));

    // Releasing the device removes a single reference and clears its error callbacks.
    wgpuDeviceRelease(reserved.device);
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
    auto reserved = GetWireClient()->ReserveDevice(instance);

    wgpuDeviceGetQueue(reserved.device);
    FlushClient(false);
}

// Test that it is valid to get the primary queue of a device after it has been
// injected on the server.
TEST_F(WireInjectDeviceTests, GetQueueAfterInject) {
    auto reserved = GetWireClient()->ReserveDevice(instance);

    WGPUDevice serverDevice = api.GetNewDevice();
    EXPECT_CALL(api, DeviceReference(serverDevice));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice, _, _));
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice, _, _));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice, _, _));
    ASSERT_TRUE(GetWireServer()->InjectDevice(serverDevice, reserved.reservation));

    wgpuDeviceGetQueue(reserved.device);

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
    auto reserved1 = GetWireClient()->ReserveDevice(instance);
    auto reserved2 = GetWireClient()->ReserveDevice(instance);

    // Inject both devices.

    WGPUDevice serverDevice1 = api.GetNewDevice();
    EXPECT_CALL(api, DeviceReference(serverDevice1));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice1, _, _));
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice1, _, _));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice1, _, _));
    ASSERT_TRUE(GetWireServer()->InjectDevice(serverDevice1, reserved1.reservation));

    WGPUDevice serverDevice2 = api.GetNewDevice();
    EXPECT_CALL(api, DeviceReference(serverDevice2));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice2, _, _));
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice2, _, _));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice2, _, _));
    ASSERT_TRUE(GetWireServer()->InjectDevice(serverDevice2, reserved2.reservation));

    // Test that both devices can be reflected.
    ASSERT_EQ(serverDevice1, GetWireServer()->GetDevice(reserved1.reservation.id,
                                                        reserved1.reservation.generation));
    ASSERT_EQ(serverDevice2, GetWireServer()->GetDevice(reserved2.reservation.id,
                                                        reserved2.reservation.generation));

    // Release the first device
    wgpuDeviceRelease(reserved1.device);
    EXPECT_CALL(api, DeviceRelease(serverDevice1));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice1, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice1, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice1, nullptr, nullptr)).Times(1);
    FlushClient();

    // The first device should no longer reflect, but the second should
    ASSERT_EQ(nullptr, GetWireServer()->GetDevice(reserved1.reservation.id,
                                                  reserved1.reservation.generation));
    ASSERT_EQ(serverDevice2, GetWireServer()->GetDevice(reserved2.reservation.id,
                                                        reserved2.reservation.generation));

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
    auto reserved1 = GetWireClient()->ReserveDevice(instance);

    WGPUDevice serverDevice1 = api.GetNewDevice();
    EXPECT_CALL(api, DeviceReference(serverDevice1));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice1, _, _));
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice1, _, _));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice1, _, _));
    ASSERT_TRUE(GetWireServer()->InjectDevice(serverDevice1, reserved1.reservation));

    WGPUCommandEncoder commandEncoder = wgpuDeviceCreateCommandEncoder(reserved1.device, nullptr);

    WGPUCommandEncoder serverCommandEncoder = api.GetNewCommandEncoder();
    EXPECT_CALL(api, DeviceCreateCommandEncoder(serverDevice1, _))
        .WillOnce(Return(serverCommandEncoder));
    FlushClient();

    // Reserve a second device, and inject it.
    auto reserved2 = GetWireClient()->ReserveDevice(instance);

    WGPUDevice serverDevice2 = api.GetNewDevice();
    EXPECT_CALL(api, DeviceReference(serverDevice2));
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(serverDevice2, _, _));
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(serverDevice2, _, _));
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(serverDevice2, _, _));
    ASSERT_TRUE(GetWireServer()->InjectDevice(serverDevice2, reserved2.reservation));

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
        auto reserved = GetWireClient()->ReserveDevice(instance);
        wgpuDeviceRelease(reserved.device);
        FlushClient(false);
    }

    // Test that doing a reservation and then reclaiming it recycles the ID.
    {
        auto reserved1 = GetWireClient()->ReserveDevice(instance);
        GetWireClient()->ReclaimDeviceReservation(reserved1);

        auto reserved2 = GetWireClient()->ReserveDevice(instance);

        // The ID is the same, but the generation is still different.
        ASSERT_EQ(reserved1.reservation.id, reserved2.reservation.id);
        ASSERT_NE(reserved1.reservation.generation, reserved2.reservation.generation);

        // No errors should occur.
        FlushClient();
    }
}

}  // anonymous namespace
}  // namespace dawn::wire
