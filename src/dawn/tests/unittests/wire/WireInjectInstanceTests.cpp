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

#include "dawn/tests/unittests/wire/WireTest.h"

#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireServer.h"

namespace dawn::wire {
namespace {

using testing::Mock;
using testing::NotNull;
using testing::Return;

class WireInjectInstanceTests : public WireTest {
  public:
    WireInjectInstanceTests() {}
    ~WireInjectInstanceTests() override = default;
};

// Test that reserving and injecting an instance makes calls on the client object forward to the
// server object correctly.
TEST_F(WireInjectInstanceTests, CallAfterReserveInject) {
    ReservedInstance reservation = GetWireClient()->ReserveInstance();

    WGPUInstance serverInstance = api.GetNewInstance();
    EXPECT_CALL(api, InstanceReference(serverInstance));
    ASSERT_TRUE(
        GetWireServer()->InjectInstance(serverInstance, reservation.id, reservation.generation));

    WGPUSurfaceDescriptor surfaceDesc = {};
    wgpuInstanceCreateSurface(reservation.instance, &surfaceDesc);
    WGPUSurface serverSurface = api.GetNewSurface();
    EXPECT_CALL(api, InstanceCreateSurface(serverInstance, NotNull()))
        .WillOnce(Return(serverSurface));
    FlushClient();
}

// Test that reserve correctly returns different IDs each time.
TEST_F(WireInjectInstanceTests, ReserveDifferentIDs) {
    ReservedInstance reservation1 = GetWireClient()->ReserveInstance();
    ReservedInstance reservation2 = GetWireClient()->ReserveInstance();

    ASSERT_NE(reservation1.id, reservation2.id);
    ASSERT_NE(reservation1.instance, reservation2.instance);
}

// Test that injecting the same id fails.
TEST_F(WireInjectInstanceTests, InjectExistingID) {
    ReservedInstance reservation = GetWireClient()->ReserveInstance();

    WGPUInstance serverInstance = api.GetNewInstance();
    EXPECT_CALL(api, InstanceReference(serverInstance));
    ASSERT_TRUE(
        GetWireServer()->InjectInstance(serverInstance, reservation.id, reservation.generation));

    // ID already in use, call fails.
    ASSERT_FALSE(
        GetWireServer()->InjectInstance(serverInstance, reservation.id, reservation.generation));
}

// Test that the server only borrows the instance and does a single reference-release
TEST_F(WireInjectInstanceTests, InjectedInstanceLifetime) {
    ReservedInstance reservation = GetWireClient()->ReserveInstance();

    // Injecting the instance adds a reference
    WGPUInstance serverInstance = api.GetNewInstance();
    EXPECT_CALL(api, InstanceReference(serverInstance));
    ASSERT_TRUE(
        GetWireServer()->InjectInstance(serverInstance, reservation.id, reservation.generation));

    // Releasing the instance removes a single reference.
    wgpuInstanceRelease(reservation.instance);
    EXPECT_CALL(api, InstanceRelease(serverInstance));
    FlushClient();

    // Deleting the server doesn't release a second reference.
    DeleteServer();
    Mock::VerifyAndClearExpectations(&api);
}

// Test that a device reservation can be reclaimed. This is necessary to
// avoid leaking ObjectIDs for reservations that are never injected.
TEST_F(WireInjectInstanceTests, ReclaimInstanceReservation) {
    // Test that doing a reservation and full release is an error.
    {
        ReservedInstance reservation = GetWireClient()->ReserveInstance();
        wgpuInstanceRelease(reservation.instance);
        FlushClient(false);
    }

    // Test that doing a reservation and then reclaiming it recycles the ID.
    {
        ReservedInstance reservation1 = GetWireClient()->ReserveInstance();
        GetWireClient()->ReclaimInstanceReservation(reservation1);

        ReservedInstance reservation2 = GetWireClient()->ReserveInstance();

        // The ID is the same, but the generation is still different.
        ASSERT_EQ(reservation1.id, reservation2.id);
        ASSERT_NE(reservation1.generation, reservation2.generation);

        // No errors should occur.
        FlushClient();
    }
}

// TODO(https://crbug.com/dawn/1381) Remove when namespaces are not indented.
// NOLINTNEXTLINE(readability/namespace)
}  // namespace
}  // namespace dawn::wire
