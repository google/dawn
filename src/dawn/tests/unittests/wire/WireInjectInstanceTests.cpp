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

}  // anonymous namespace
}  // namespace dawn::wire
