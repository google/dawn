// Copyright 2026 The Dawn & Tint Authors
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

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/native/Device.h"
#include "src/dawn/native/metal/DeviceMTL.h"
#include "src/dawn/native/metal/FencedReferencer.h"
#include "src/dawn/native/metal/TextureMTL.h"
#include "src/dawn/tests/DawnTest.h"

namespace dawn::native::metal {
namespace {

class MetalFencedReferencerTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    }

    Device* GetMetalDevice() { return ToBackend(FromAPI(device.Get())); }
};

// Test that if a TextureView has its own MTLTexture, then destroying it enqueues its underlying
// Metal texture to the FencedReferencer, so that it gets freed only once GPU execution completes.
TEST_P(MetalFencedReferencerTests, DestroyTextureViewWithOwnMTLTexture) {
    Device* metalDevice = GetMetalDevice();

    wgpu::TextureDescriptor texDesc;
    texDesc.size = {4, 4, 1};
    texDesc.mipLevelCount = 2;
    texDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    texDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
    wgpu::Texture texture = device.CreateTexture(&texDesc);

    // Create a view that restricts to mip level 1, which requires an actual Metal texture view.
    wgpu::TextureViewDescriptor viewDesc = {};
    viewDesc.baseMipLevel = 1;
    viewDesc.mipLevelCount = 1;
    wgpu::TextureView view = texture.CreateView(&viewDesc);

    ASSERT_NE(ToBackend(FromAPI(texture.Get()))->GetMTLTexture(Aspect::None),
              ToBackend(FromAPI(view.Get()))->GetMTLTexture());

    EXPECT_FALSE(metalDevice->GetFencedReferencer().HasPendingObjectsForTesting());

    // Drop the view (and destroy the texture to drop cached views) to check that, upon destruction,
    // the MTLTexture got transferred to the current pending serial in the FencedReferencer.
    view = nullptr;
    texture.Destroy();

    EXPECT_TRUE(metalDevice->GetFencedReferencer().HasPendingObjectsForTesting());

    // Advance the execution queue serial by submitting a command buffer.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    WaitForAllOperations();

    EXPECT_FALSE(metalDevice->GetFencedReferencer().HasPendingObjectsForTesting());
}

DAWN_INSTANTIATE_TEST(MetalFencedReferencerTests, MetalBackend());

}  // anonymous namespace
}  // namespace dawn::native::metal
