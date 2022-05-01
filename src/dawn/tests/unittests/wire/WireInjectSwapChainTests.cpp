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

using testing::Mock;

class WireInjectSwapChainTests : public WireTest {
  public:
    WireInjectSwapChainTests() {}
    ~WireInjectSwapChainTests() override = default;
};

// Test that reserving and injecting a swapchain makes calls on the client object forward to the
// server object correctly.
TEST_F(WireInjectSwapChainTests, CallAfterReserveInject) {
    ReservedSwapChain reservation = GetWireClient()->ReserveSwapChain(device);

    WGPUSwapChain apiSwapchain = api.GetNewSwapChain();
    EXPECT_CALL(api, SwapChainReference(apiSwapchain));
    ASSERT_TRUE(GetWireServer()->InjectSwapChain(apiSwapchain, reservation.id,
                                                 reservation.generation, reservation.deviceId,
                                                 reservation.deviceGeneration));

    wgpuSwapChainPresent(reservation.swapchain);
    EXPECT_CALL(api, SwapChainPresent(apiSwapchain));
    FlushClient();
}

// Test that reserve correctly returns different IDs each time.
TEST_F(WireInjectSwapChainTests, ReserveDifferentIDs) {
    ReservedSwapChain reservation1 = GetWireClient()->ReserveSwapChain(device);
    ReservedSwapChain reservation2 = GetWireClient()->ReserveSwapChain(device);

    ASSERT_NE(reservation1.id, reservation2.id);
    ASSERT_NE(reservation1.swapchain, reservation2.swapchain);
}

// Test that injecting the same id without a destroy first fails.
TEST_F(WireInjectSwapChainTests, InjectExistingID) {
    ReservedSwapChain reservation = GetWireClient()->ReserveSwapChain(device);

    WGPUSwapChain apiSwapchain = api.GetNewSwapChain();
    EXPECT_CALL(api, SwapChainReference(apiSwapchain));
    ASSERT_TRUE(GetWireServer()->InjectSwapChain(apiSwapchain, reservation.id,
                                                 reservation.generation, reservation.deviceId,
                                                 reservation.deviceGeneration));

    // ID already in use, call fails.
    ASSERT_FALSE(GetWireServer()->InjectSwapChain(apiSwapchain, reservation.id,
                                                  reservation.generation, reservation.deviceId,
                                                  reservation.deviceGeneration));
}

// Test that the server only borrows the swapchain and does a single reference-release
TEST_F(WireInjectSwapChainTests, InjectedSwapChainLifetime) {
    ReservedSwapChain reservation = GetWireClient()->ReserveSwapChain(device);

    // Injecting the swapchain adds a reference
    WGPUSwapChain apiSwapchain = api.GetNewSwapChain();
    EXPECT_CALL(api, SwapChainReference(apiSwapchain));
    ASSERT_TRUE(GetWireServer()->InjectSwapChain(apiSwapchain, reservation.id,
                                                 reservation.generation, reservation.deviceId,
                                                 reservation.deviceGeneration));

    // Releasing the swapchain removes a single reference.
    wgpuSwapChainRelease(reservation.swapchain);
    EXPECT_CALL(api, SwapChainRelease(apiSwapchain));
    FlushClient();

    // Deleting the server doesn't release a second reference.
    DeleteServer();
    Mock::VerifyAndClearExpectations(&api);
}

// Test that a swapchain reservation can be reclaimed. This is necessary to
// avoid leaking ObjectIDs for reservations that are never injected.
TEST_F(WireInjectSwapChainTests, ReclaimSwapChainReservation) {
    // Test that doing a reservation and full release is an error.
    {
        ReservedSwapChain reservation = GetWireClient()->ReserveSwapChain(device);
        wgpuSwapChainRelease(reservation.swapchain);
        FlushClient(false);
    }

    // Test that doing a reservation and then reclaiming it recycles the ID.
    {
        ReservedSwapChain reservation1 = GetWireClient()->ReserveSwapChain(device);
        GetWireClient()->ReclaimSwapChainReservation(reservation1);

        ReservedSwapChain reservation2 = GetWireClient()->ReserveSwapChain(device);

        // The ID is the same, but the generation is still different.
        ASSERT_EQ(reservation1.id, reservation2.id);
        ASSERT_NE(reservation1.generation, reservation2.generation);

        // No errors should occur.
        FlushClient();
    }
}

}  // namespace dawn::wire
