// Copyright 2019 The Dawn & Tint Authors
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

class WireInjectTextureTests : public WireTest {
  public:
    WireInjectTextureTests() {}
    ~WireInjectTextureTests() override = default;

    // A placeholder texture format for ReserveTexture. The data in it doesn't matter as long as
    // we don't call texture reflection methods.
    WGPUTextureDescriptor placeholderDesc = {};
};

// Test that reserving and injecting a texture makes calls on the client object forward to the
// server object correctly.
TEST_F(WireInjectTextureTests, CallAfterReserveInject) {
    ReservedTexture reservation = GetWireClient()->ReserveTexture(device, &placeholderDesc);

    WGPUTexture apiTexture = api.GetNewTexture();
    EXPECT_CALL(api, TextureReference(apiTexture));
    ASSERT_TRUE(GetWireServer()->InjectTexture(apiTexture, reservation.id, reservation.generation,
                                               reservation.deviceId, reservation.deviceGeneration));

    wgpuTextureCreateView(reservation.texture, nullptr);
    WGPUTextureView apiPlaceholderView = api.GetNewTextureView();
    EXPECT_CALL(api, TextureCreateView(apiTexture, nullptr)).WillOnce(Return(apiPlaceholderView));
    FlushClient();
}

// Test that reserve correctly returns different IDs each time.
TEST_F(WireInjectTextureTests, ReserveDifferentIDs) {
    ReservedTexture reservation1 = GetWireClient()->ReserveTexture(device, &placeholderDesc);
    ReservedTexture reservation2 = GetWireClient()->ReserveTexture(device, &placeholderDesc);

    ASSERT_NE(reservation1.id, reservation2.id);
    ASSERT_NE(reservation1.texture, reservation2.texture);
}

// Test that injecting the same id without a destroy first fails.
TEST_F(WireInjectTextureTests, InjectExistingID) {
    ReservedTexture reservation = GetWireClient()->ReserveTexture(device, &placeholderDesc);

    WGPUTexture apiTexture = api.GetNewTexture();
    EXPECT_CALL(api, TextureReference(apiTexture));
    ASSERT_TRUE(GetWireServer()->InjectTexture(apiTexture, reservation.id, reservation.generation,
                                               reservation.deviceId, reservation.deviceGeneration));

    // ID already in use, call fails.
    ASSERT_FALSE(GetWireServer()->InjectTexture(apiTexture, reservation.id, reservation.generation,
                                                reservation.deviceId,
                                                reservation.deviceGeneration));
}

// Test that injecting the same id without a destroy first fails.
TEST_F(WireInjectTextureTests, ReuseIDAndGeneration) {
    // Do this loop multiple times since the first time, we can't test `generation - 1` since
    // generation == 0.
    ReservedTexture reservation;
    WGPUTexture apiTexture = nullptr;
    for (int i = 0; i < 2; ++i) {
        reservation = GetWireClient()->ReserveTexture(device, &placeholderDesc);

        apiTexture = api.GetNewTexture();
        EXPECT_CALL(api, TextureReference(apiTexture));
        ASSERT_TRUE(GetWireServer()->InjectTexture(apiTexture, reservation.id,
                                                   reservation.generation, reservation.deviceId,
                                                   reservation.deviceGeneration));

        // Release the texture. It should be possible to reuse the ID now, but not the generation
        wgpuTextureRelease(reservation.texture);
        EXPECT_CALL(api, TextureRelease(apiTexture));
        FlushClient();

        // Invalid to inject with the same ID and generation.
        ASSERT_FALSE(GetWireServer()->InjectTexture(apiTexture, reservation.id,
                                                    reservation.generation, reservation.deviceId,
                                                    reservation.deviceGeneration));
        if (i > 0) {
            EXPECT_GE(reservation.generation, 1u);

            // Invalid to inject with the same ID and lesser generation.
            ASSERT_FALSE(GetWireServer()->InjectTexture(
                apiTexture, reservation.id, reservation.generation - 1, reservation.deviceId,
                reservation.deviceGeneration));
        }
    }

    // Valid to inject with the same ID and greater generation.
    EXPECT_CALL(api, TextureReference(apiTexture));
    ASSERT_TRUE(GetWireServer()->InjectTexture(apiTexture, reservation.id,
                                               reservation.generation + 1, reservation.deviceId,
                                               reservation.deviceGeneration));
}

// Test that the server only borrows the texture and does a single reference-release
TEST_F(WireInjectTextureTests, InjectedTextureLifetime) {
    ReservedTexture reservation = GetWireClient()->ReserveTexture(device, &placeholderDesc);

    // Injecting the texture adds a reference
    WGPUTexture apiTexture = api.GetNewTexture();
    EXPECT_CALL(api, TextureReference(apiTexture));
    ASSERT_TRUE(GetWireServer()->InjectTexture(apiTexture, reservation.id, reservation.generation,
                                               reservation.deviceId, reservation.deviceGeneration));

    // Releasing the texture removes a single reference.
    wgpuTextureRelease(reservation.texture);
    EXPECT_CALL(api, TextureRelease(apiTexture));
    FlushClient();

    // Deleting the server doesn't release a second reference.
    DeleteServer();
    Mock::VerifyAndClearExpectations(&api);
}

// Test that a texture reservation can be reclaimed. This is necessary to
// avoid leaking ObjectIDs for reservations that are never injected.
TEST_F(WireInjectTextureTests, ReclaimTextureReservation) {
    // Test that doing a reservation and full release is an error.
    {
        ReservedTexture reservation = GetWireClient()->ReserveTexture(device, &placeholderDesc);
        wgpuTextureRelease(reservation.texture);
        FlushClient(false);
    }

    // Test that doing a reservation and then reclaiming it recycles the ID.
    {
        ReservedTexture reservation1 = GetWireClient()->ReserveTexture(device, &placeholderDesc);
        GetWireClient()->ReclaimTextureReservation(reservation1);

        ReservedTexture reservation2 = GetWireClient()->ReserveTexture(device, &placeholderDesc);

        // The ID is the same, but the generation is still different.
        ASSERT_EQ(reservation1.id, reservation2.id);
        ASSERT_NE(reservation1.generation, reservation2.generation);

        // No errors should occur.
        FlushClient();
    }
}

// Test the reflection of texture creation parameters for reserved textures.
TEST_F(WireInjectTextureTests, ReservedTextureReflection) {
    WGPUTextureDescriptor desc = {};
    desc.size = {10, 11, 12};
    desc.format = WGPUTextureFormat_R32Float;
    desc.dimension = WGPUTextureDimension_3D;
    desc.mipLevelCount = 1000;
    desc.sampleCount = 3;
    desc.usage = WGPUTextureUsage_RenderAttachment;

    ReservedTexture reservation = GetWireClient()->ReserveTexture(device, &desc);
    WGPUTexture texture = reservation.texture;

    ASSERT_EQ(desc.size.width, wgpuTextureGetWidth(texture));
    ASSERT_EQ(desc.size.height, wgpuTextureGetHeight(texture));
    ASSERT_EQ(desc.size.depthOrArrayLayers, wgpuTextureGetDepthOrArrayLayers(texture));
    ASSERT_EQ(desc.format, wgpuTextureGetFormat(texture));
    ASSERT_EQ(desc.dimension, wgpuTextureGetDimension(texture));
    ASSERT_EQ(desc.mipLevelCount, wgpuTextureGetMipLevelCount(texture));
    ASSERT_EQ(desc.sampleCount, wgpuTextureGetSampleCount(texture));
}

}  // anonymous namespace
}  // namespace dawn::wire
