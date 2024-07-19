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

#include <utility>

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
        swapChainDesc.usage = wgpu::TextureUsage::RenderAttachment;
        swapChainDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        swapChainDesc.width = 17;
        swapChainDesc.height = 42;
        swapChainDesc.presentMode = wgpu::PresentMode::Mailbox;
    }
    ~WireInjectSwapChainTests() override = default;

    std::pair<ReservedSwapChain, wgpu::SwapChain> ReserveSwapChain(
        const wgpu::SwapChainDescriptor* desc = nullptr) {
        if (desc == nullptr) {
            desc = &swapChainDesc;
        }

        auto reservation = GetWireClient()->ReserveSwapChain(
            device.Get(), reinterpret_cast<const WGPUSwapChainDescriptor*>(desc));
        return {reservation, wgpu::SwapChain::Acquire(reservation.swapchain)};
    }

    wgpu::SwapChainDescriptor swapChainDesc;
};

// Test that reserving and injecting a swapchain makes calls on the client object forward to the
// server object correctly.
TEST_F(WireInjectSwapChainTests, CallAfterReserveInject) {
    auto [reservation, swapchain] = ReserveSwapChain();

    WGPUSwapChain apiSwapchain = api.GetNewSwapChain();
    EXPECT_CALL(api, SwapChainAddRef(apiSwapchain));
    ASSERT_TRUE(GetWireServer()->InjectSwapChain(apiSwapchain, reservation.handle,
                                                 reservation.deviceHandle));

    swapchain.Present();
    EXPECT_CALL(api, SwapChainPresent(apiSwapchain));
    FlushClient();
}

// Test that reserve correctly returns different IDs each time.
TEST_F(WireInjectSwapChainTests, ReserveDifferentIDs) {
    auto [reservation1, swapchain1] = ReserveSwapChain();
    auto [reservation2, swapchain2] = ReserveSwapChain();

    ASSERT_NE(reservation1.handle.id, reservation2.handle.id);
    ASSERT_NE(swapchain1.Get(), swapchain2.Get());
}

// Test that injecting the same id without a destroy first fails.
TEST_F(WireInjectSwapChainTests, InjectExistingID) {
    auto [reservation, swapchain] = ReserveSwapChain();

    WGPUSwapChain apiSwapchain = api.GetNewSwapChain();
    EXPECT_CALL(api, SwapChainAddRef(apiSwapchain));
    ASSERT_TRUE(GetWireServer()->InjectSwapChain(apiSwapchain, reservation.handle,
                                                 reservation.deviceHandle));

    // ID already in use, call fails.
    ASSERT_FALSE(GetWireServer()->InjectSwapChain(apiSwapchain, reservation.handle,
                                                  reservation.deviceHandle));
}

// Test that the server only borrows the swapchain and does a single addref-release
TEST_F(WireInjectSwapChainTests, InjectedSwapChainLifetime) {
    auto [reservation, swapchain] = ReserveSwapChain();

    // Injecting the swapchain adds a reference
    WGPUSwapChain apiSwapchain = api.GetNewSwapChain();
    EXPECT_CALL(api, SwapChainAddRef(apiSwapchain));
    ASSERT_TRUE(GetWireServer()->InjectSwapChain(apiSwapchain, reservation.handle,
                                                 reservation.deviceHandle));

    // Releasing the swapchain removes a single reference.
    swapchain = nullptr;
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
        auto [reservation, swapchain] = ReserveSwapChain();
        swapchain = nullptr;
        FlushClient(false);
    }

    // Test that doing a reservation and then reclaiming it recycles the ID.
    {
        auto [reservation1, swapchain1] = ReserveSwapChain();
        GetWireClient()->ReclaimSwapChainReservation(reservation1);

        auto [reservation2, swapchain2] = ReserveSwapChain();

        // The ID is the same, but the generation is still different.
        ASSERT_EQ(reservation1.handle.id, reservation2.handle.id);
        ASSERT_NE(reservation1.handle.generation, reservation2.handle.generation);

        // No errors should occur.
        FlushClient();
    }
}

// Test that the texture's reflection is correct for injected swapchains in the wire.
TEST_F(WireInjectSwapChainTests, SwapChainTextureReflection) {
    auto [reservation, swapchain] = ReserveSwapChain();

    WGPUSwapChain apiSwapchain = api.GetNewSwapChain();
    EXPECT_CALL(api, SwapChainAddRef(apiSwapchain));
    ASSERT_TRUE(GetWireServer()->InjectSwapChain(apiSwapchain, reservation.handle,
                                                 reservation.deviceHandle));

    wgpu::Texture tex = swapchain.GetCurrentTexture();
    WGPUTexture apiTex = api.GetNewTexture();
    EXPECT_CALL(api, SwapChainGetCurrentTexture(apiSwapchain)).WillOnce(Return(apiTex));
    FlushClient();

    EXPECT_EQ(swapChainDesc.width, tex.GetWidth());
    EXPECT_EQ(swapChainDesc.height, tex.GetHeight());
    EXPECT_EQ(swapChainDesc.usage, tex.GetUsage());
    EXPECT_EQ(swapChainDesc.format, tex.GetFormat());
    EXPECT_EQ(1u, tex.GetDepthOrArrayLayers());
    EXPECT_EQ(1u, tex.GetMipLevelCount());
    EXPECT_EQ(1u, tex.GetSampleCount());
    EXPECT_EQ(wgpu::TextureDimension::e2D, tex.GetDimension());
}

}  // anonymous namespace
}  // namespace dawn::wire
