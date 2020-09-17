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

#include "tests/DawnTest.h"

#include "dawn_native/d3d12/BufferD3D12.h"
#include "dawn_native/d3d12/TextureD3D12.h"

using namespace dawn_native::d3d12;

class D3D12ResourceHeapTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_SKIP_TEST_IF(UsesWire());
    }

    std::vector<const char*> GetRequiredExtensions() override {
        mIsBCFormatSupported = SupportsExtensions({"texture_compression_bc"});
        if (!mIsBCFormatSupported) {
            return {};
        }

        return {"texture_compression_bc"};
    }

    bool IsBCFormatSupported() const {
        return mIsBCFormatSupported;
    }

  private:
    bool mIsBCFormatSupported = false;
};

// Verify that creating a small compressed textures will be 4KB aligned.
TEST_P(D3D12ResourceHeapTests, AlignSmallCompressedTexture) {
    DAWN_SKIP_TEST_IF(!IsBCFormatSupported());

    // TODO(http://crbug.com/dawn/282): Investigate GPU/driver rejections of small alignment.
    DAWN_SKIP_TEST_IF(IsIntel() || IsNvidia() || IsWARP());

    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = 8;
    descriptor.size.height = 8;
    descriptor.size.depth = 1;
    descriptor.sampleCount = 1;
    descriptor.format = wgpu::TextureFormat::BC1RGBAUnorm;
    descriptor.mipLevelCount = 1;
    descriptor.usage = wgpu::TextureUsage::Sampled;

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

    EXPECT_TRUE((d3dBuffer->GetD3D12Resource()->GetDesc().Width %
                 static_cast<uint64_t>(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)) == 0u);

    // Create a larger UBO
    descriptor.size = (4 * 1024 * 1024) + 255;
    descriptor.usage = wgpu::BufferUsage::Uniform;

    buffer = device.CreateBuffer(&descriptor);
    d3dBuffer = reinterpret_cast<Buffer*>(buffer.Get());

    EXPECT_TRUE((d3dBuffer->GetD3D12Resource()->GetDesc().Width %
                 static_cast<uint64_t>(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)) == 0u);
}

DAWN_INSTANTIATE_TEST(D3D12ResourceHeapTests, D3D12Backend());
