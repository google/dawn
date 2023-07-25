// Copyright 2019 The Dawn Authors
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

#include <vector>

#include "dawn/native/d3d12/BufferD3D12.h"
#include "dawn/native/d3d12/TextureD3D12.h"
#include "dawn/tests/DawnTest.h"

namespace dawn::native::d3d12 {
namespace {

class D3D12ResourceHeapTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        mIsBCFormatSupported = SupportsFeatures({wgpu::FeatureName::TextureCompressionBC});
        if (!mIsBCFormatSupported) {
            return {};
        }

        return {wgpu::FeatureName::TextureCompressionBC};
    }

    bool IsBCFormatSupported() const { return mIsBCFormatSupported; }

  private:
    bool mIsBCFormatSupported = false;
};

// Verify that creating a small compressed texture will be 4KB aligned.
TEST_P(D3D12ResourceHeapTests, AlignSmallCompressedTexture) {
    DAWN_TEST_UNSUPPORTED_IF(!IsBCFormatSupported());

    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = 8;
    descriptor.size.height = 8;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.sampleCount = 1;
    descriptor.format = wgpu::TextureFormat::BC1RGBAUnorm;
    descriptor.mipLevelCount = 1;
    descriptor.usage = wgpu::TextureUsage::TextureBinding;

    // Create a smaller one that allows use of the smaller alignment.
    wgpu::Texture texture = device.CreateTexture(&descriptor);
    Texture* d3dTexture = reinterpret_cast<Texture*>(texture.Get());

    EXPECT_EQ(d3dTexture->GetD3D12Resource()->GetDesc().Alignment,
              static_cast<uint64_t>(D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT));

    // Create a larger one (>64KB) that forbids use the smaller alignment.
    descriptor.size.width = 4096;
    descriptor.size.height = 4096;

    texture = device.CreateTexture(&descriptor);
    d3dTexture = reinterpret_cast<Texture*>(texture.Get());

    EXPECT_EQ(d3dTexture->GetD3D12Resource()->GetDesc().Alignment,
              static_cast<uint64_t>(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT));
}

// Verify creating a UBO will always be 256B aligned.
TEST_P(D3D12ResourceHeapTests, AlignUBO) {
    // Create a small UBO
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4 * 1024;
    descriptor.usage = wgpu::BufferUsage::Uniform;

    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
    Buffer* d3dBuffer = reinterpret_cast<Buffer*>(buffer.Get());

    EXPECT_EQ((d3dBuffer->GetD3D12Resource()->GetDesc().Width %
               static_cast<uint64_t>(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)),
              0u);

    // Create a larger UBO
    descriptor.size = (4 * 1024 * 1024) + 255;
    descriptor.usage = wgpu::BufferUsage::Uniform;

    buffer = device.CreateBuffer(&descriptor);
    d3dBuffer = reinterpret_cast<Buffer*>(buffer.Get());

    EXPECT_EQ((d3dBuffer->GetD3D12Resource()->GetDesc().Width %
               static_cast<uint64_t>(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)),
              0u);
}

// Verify the MSAA textures are 64KB alignment if the alignment is supported for MSAA textures on
// the current platform. Otherwise it will be 4MB.
TEST_P(D3D12ResourceHeapTests, MSAATexture) {
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = 8;
    descriptor.size.height = 8;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.sampleCount = 4;
    descriptor.format = wgpu::TextureFormat::RGBA16Float;
    descriptor.mipLevelCount = 1;
    descriptor.usage = wgpu::TextureUsage::RenderAttachment;

    // Create a smaller MSAA texture
    wgpu::Texture texture = device.CreateTexture(&descriptor);
    Texture* d3dTexture = reinterpret_cast<Texture*>(texture.Get());

    uint64_t expectedAlignment = HasToggleEnabled("d3d12_use_64kb_alignment_msaa_texture")
                                     ? D3D12_SMALL_MSAA_RESOURCE_PLACEMENT_ALIGNMENT
                                     : D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT;
    EXPECT_EQ(d3dTexture->GetD3D12Resource()->GetDesc().Alignment, expectedAlignment);

    // Create a larger MSAA texture. Currently it will be created as a committed resource in Dawn.
    descriptor.size.width = 4096;
    descriptor.size.height = 4096;

    texture = device.CreateTexture(&descriptor);
    d3dTexture = reinterpret_cast<Texture*>(texture.Get());

    EXPECT_EQ(d3dTexture->GetD3D12Resource()->GetDesc().Alignment, expectedAlignment);
}

DAWN_INSTANTIATE_TEST(D3D12ResourceHeapTests,
                      D3D12Backend(),
                      D3D12Backend({}, {"d3d12_use_64kb_alignment_msaa_texture"}));

}  // anonymous namespace
}  // namespace dawn::native::d3d12
