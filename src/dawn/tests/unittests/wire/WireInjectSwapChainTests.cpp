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
using testing::Return;

class WireInjectSwapChainTests : public WireTest {
  public:
    WireInjectSwapChainTests() {
        swapChainDesc = {};
        swapChainDesc.usage = WGPUTextureUsage_RenderAttachment;
        swapChainDesc.format = WGPUTextureFormat_RGBA8Unorm;
        swapChainDesc.width = 17;
        swapChainDesc.height = 42;
        swapChainDesc.presentMode = WGPUPresentMode_Mailbox;
    }
    ~WireInjectSwapChainTests() override = default;

    WGPUSwapChainDescriptor swapChainDesc;
};

// Test that reserving and injecting a swapchain makes calls on the client object forward to the
// server object correctly.
TEST_F(WireInjectSwapChainTests, CallAfterReserveInject) {
    ReservedSwapChain reservation = GetWireClient()->ReserveSwapChain(device, &swapChainDesc);

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
    ReservedSwapChain reservation1 = GetWireClient()->ReserveSwapChain(device, &swapChainDesc);
    ReservedSwapChain reservation2 = GetWireClient()->ReserveSwapChain(device, &swapChainDesc);

    ASSERT_NE(reservation1.id, reservation2.id);
    ASSERT_NE(reservation1.swapchain, reservation2.swapchain);
}

// Test that injecting the same id without a destroy first fails.
TEST_F(WireInjectSwapChainTests, InjectExistingID) {
    ReservedSwapChain reservation = GetWireClient()->ReserveSwapChain(device, &swapChainDesc);

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
    ReservedSwapChain reservation = GetWireClient()->ReserveSwapChain(device, &swapChainDesc);

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
        ReservedSwapChain reservation = GetWireClient()->ReserveSwapChain(device, &swapChainDesc);
        wgpuSwapChainRelease(reservation.swapchain);
        FlushClient(false);
    }

    // Test that doing a reservation and then reclaiming it recycles the ID.
    {
        ReservedSwapChain reservation1 = GetWireClient()->ReserveSwapChain(device, &swapChainDesc);
        GetWireClient()->ReclaimSwapChainReservation(reservation1);

        ReservedSwapChain reservation2 = GetWireClient()->ReserveSwapChain(device, &swapChainDesc);

        // The ID is the same, but the generation is still different.
        ASSERT_EQ(reservation1.id, reservation2.id);
        ASSERT_NE(reservation1.generation, reservation2.generation);

        // No errors should occur.
        FlushClient();
    }
}

// Test that the texture's reflection is correct for injected swapchains in the wire.
TEST_F(WireInjectSwapChainTests, SwapChainTextureReflection) {
    ReservedSwapChain reservation = GetWireClient()->ReserveSwapChain(device, &swapChainDesc);

    WGPUSwapChain apiSwapchain = api.GetNewSwapChain();
    EXPECT_CALL(api, SwapChainReference(apiSwapchain));
    ASSERT_TRUE(GetWireServer()->InjectSwapChain(apiSwapchain, reservation.id,
                                                 reservation.generation, reservation.deviceId,
                                                 reservation.deviceGeneration));

    WGPUTexture tex = wgpuSwapChainGetCurrentTexture(reservation.swapchain);
    WGPUTexture apiTex = api.GetNewTexture();
    EXPECT_CALL(api, SwapChainGetCurrentTexture(apiSwapchain)).WillOnce(Return(apiTex));
    FlushClient();

    EXPECT_EQ(swapChainDesc.width, wgpuTextureGetWidth(tex));
    EXPECT_EQ(swapChainDesc.height, wgpuTextureGetHeight(tex));
    EXPECT_EQ(swapChainDesc.usage, wgpuTextureGetUsage(tex));
    EXPECT_EQ(swapChainDesc.format, wgpuTextureGetFormat(tex));
    EXPECT_EQ(1u, wgpuTextureGetDepthOrArrayLayers(tex));
    EXPECT_EQ(1u, wgpuTextureGetMipLevelCount(tex));
    EXPECT_EQ(1u, wgpuTextureGetSampleCount(tex));
    EXPECT_EQ(WGPUTextureDimension_2D, wgpuTextureGetDimension(tex));
}

}  // anonymous namespace
}  // namespace dawn::wire
