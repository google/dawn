// Copyright 2025 The Dawn & Tint Authors
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

#include <vector>

#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class TexelBufferValidationTest : public ValidationTest {
  protected:
    wgpu::Buffer CreateTexelBuffer(uint64_t size, wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor desc;
        desc.size = size;
        desc.usage = usage;
        return device.CreateBuffer(&desc);
    }

    struct TexelBufferLayoutDescriptor {
        wgpu::TexelBufferBindingLayout layout;
        wgpu::BindGroupLayoutEntry entry;
        wgpu::BindGroupLayoutDescriptor desc;

        TexelBufferLayoutDescriptor(wgpu::TexelBufferAccess access,
                                    wgpu::ShaderStage visibility,
                                    wgpu::TextureFormat format,
                                    uint32_t bindingArraySize = 0) {
            layout = {};
            layout.access = access;
            layout.format = format;

            entry = {};
            entry.binding = 0;
            entry.visibility = visibility;
            entry.bindingArraySize = bindingArraySize;
            entry.nextInChain = &layout;

            desc = {};
            desc.entryCount = 1;
            desc.entries = &entry;
        }
    };
};

// Creation succeeds when the feature is enabled.
TEST_F(TexelBufferValidationTest, CreationSuccess) {
    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::TexelBuffer;
    device.CreateBuffer(&desc);
}

// Mappable usages require BufferMapExtendedUsages when combined with TexelBuffer.
TEST_F(TexelBufferValidationTest, MappableUsageRequiresExtendedFeature) {
    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::TexelBuffer | wgpu::BufferUsage::MapRead;
    ASSERT_DEVICE_ERROR(device.CreateBuffer(&desc));

    desc.usage = wgpu::BufferUsage::TexelBuffer | wgpu::BufferUsage::MapWrite;
    ASSERT_DEVICE_ERROR(device.CreateBuffer(&desc));
}

// bindingArraySize > 1 is invalid.
TEST_F(TexelBufferValidationTest, BindingArraySize) {
    TexelBufferLayoutDescriptor helper(wgpu::TexelBufferAccess::ReadOnly,
                                       wgpu::ShaderStage::Fragment, wgpu::TextureFormat::RGBA8Uint,
                                       2);
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&helper.desc));
}

// Vertex visibility requires read-only access.
TEST_F(TexelBufferValidationTest, VertexVisibilityRequiresReadOnly) {
    TexelBufferLayoutDescriptor helper(wgpu::TexelBufferAccess::ReadWrite,
                                       wgpu::ShaderStage::Vertex, wgpu::TextureFormat::RGBA8Uint);
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&helper.desc));
}

// Access must not be undefined.
TEST_F(TexelBufferValidationTest, UndefinedLayoutAccess) {
    TexelBufferLayoutDescriptor helper(wgpu::TexelBufferAccess::Undefined,
                                       wgpu::ShaderStage::Fragment, wgpu::TextureFormat::RGBA8Uint);
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&helper.desc));
}

// Format must not be undefined.
TEST_F(TexelBufferValidationTest, UndefinedLayoutFormat) {
    TexelBufferLayoutDescriptor helper(wgpu::TexelBufferAccess::ReadOnly,
                                       wgpu::ShaderStage::Fragment, wgpu::TextureFormat::Undefined);
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&helper.desc));
}

// BindingInitializationHelper should chain a TexelBufferBindingEntry when initialized
// with a TexelBufferView.
TEST_F(TexelBufferValidationTest, BindingHelperChainsTexelBufferBindingEntry) {
    constexpr uint64_t kSize = 4 * 4;
    wgpu::Buffer buffer = CreateTexelBuffer(kSize, wgpu::BufferUsage::TexelBuffer);

    wgpu::TexelBufferViewDescriptor viewDesc;
    viewDesc.format = wgpu::TextureFormat::RGBA8Uint;
    viewDesc.offset = 0;
    viewDesc.size = kSize;
    wgpu::TexelBufferView view = buffer.CreateTexelView(&viewDesc);

    utils::BindingInitializationHelper helper(0, view);
    wgpu::BindGroupEntry entry = helper.GetAsBinding();

    EXPECT_EQ(entry.buffer, nullptr);
    EXPECT_EQ(entry.textureView, nullptr);

    ASSERT_NE(entry.nextInChain, nullptr);
    EXPECT_EQ(entry.nextInChain->sType, wgpu::SType::TexelBufferBindingEntry);

    const auto* texelEntry =
        reinterpret_cast<const wgpu::TexelBufferBindingEntry*>(entry.nextInChain);
    EXPECT_EQ(texelEntry->texelBufferView.Get(), view.Get());
}

// The format of a bound texel buffer view must match the layout.
TEST_F(TexelBufferValidationTest, ViewFormatMustMatchLayout) {
    wgpu::BufferDescriptor desc;
    desc.size = 256;
    desc.usage = wgpu::BufferUsage::TexelBuffer;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    wgpu::TexelBufferViewDescriptor viewDesc = {};
    viewDesc.format = wgpu::TextureFormat::RGBA8Uint;
    viewDesc.offset = 0;
    viewDesc.size = 256;
    wgpu::TexelBufferView view = buffer.CreateTexelView(&viewDesc);

    wgpu::TexelBufferBindingLayout layout = {};
    layout.access = wgpu::TexelBufferAccess::ReadOnly;
    layout.format = wgpu::TextureFormat::R32Uint;

    wgpu::BindGroupLayout bgl =
        utils::MakeBindGroupLayout(device, {{0, wgpu::ShaderStage::Compute, &layout}});

    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl, {{0, view}}));
}

// Read-write texel buffer bindings require the buffer to also have STORAGE usage.
TEST_F(TexelBufferValidationTest, ReadWriteBindingRequiresStorageUsage) {
    wgpu::BufferDescriptor desc;
    desc.size = 256;
    desc.usage = wgpu::BufferUsage::TexelBuffer;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    wgpu::TexelBufferViewDescriptor viewDesc = {};
    viewDesc.format = wgpu::TextureFormat::R32Uint;
    viewDesc.offset = 0;
    viewDesc.size = 4;
    wgpu::TexelBufferView view = buffer.CreateTexelView(&viewDesc);

    wgpu::TexelBufferBindingLayout layout = {};
    layout.access = wgpu::TexelBufferAccess::ReadWrite;
    layout.format = wgpu::TextureFormat::R32Uint;

    wgpu::BindGroupLayout bgl =
        utils::MakeBindGroupLayout(device, {{0, wgpu::ShaderStage::Compute, &layout}});

    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl, {{0, view}}));
}

// Binding a TexelBuffer to a texture binding slot fails.
TEST_F(TexelBufferValidationTest, TexelBufferCannotBindToTextureSlot) {
    wgpu::BindGroupLayoutEntry textureEntry = {};
    textureEntry.binding = 0;
    textureEntry.visibility = wgpu::ShaderStage::Compute;
    textureEntry.texture.sampleType = wgpu::TextureSampleType::Float;
    textureEntry.texture.viewDimension = wgpu::TextureViewDimension::e2D;

    wgpu::BindGroupLayoutDescriptor bglDesc = {};
    bglDesc.entryCount = 1;
    bglDesc.entries = &textureEntry;
    wgpu::BindGroupLayout bgl = device.CreateBindGroupLayout(&bglDesc);

    wgpu::Buffer buffer = CreateTexelBuffer(4, wgpu::BufferUsage::TexelBuffer);
    wgpu::TexelBufferViewDescriptor viewDesc = {};
    viewDesc.format = wgpu::TextureFormat::R32Uint;
    wgpu::TexelBufferView view = buffer.CreateTexelView(&viewDesc);

    wgpu::TexelBufferBindingEntry texelEntry = {};
    texelEntry.texelBufferView = view;

    wgpu::BindGroupEntry bgEntry = {};
    bgEntry.binding = 0;
    bgEntry.nextInChain = &texelEntry;

    wgpu::BindGroupDescriptor bgDesc = {};
    bgDesc.layout = bgl;
    bgDesc.entryCount = 1;
    bgDesc.entries = &bgEntry;

    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&bgDesc));
}

class TexelBufferValidationWithExtendedMapTest : public TexelBufferValidationTest {
  protected:
    void SetUp() override {
        DAWN_SKIP_TEST_IF(UsesWire());
        TexelBufferValidationTest::SetUp();
        DAWN_SKIP_TEST_IF(!adapter.HasFeature(wgpu::FeatureName::BufferMapExtendedUsages));
    }
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::BufferMapExtendedUsages};
    }
};

// When BufferMapExtendedUsages is enabled, MapRead and MapWrite can be combined
// with TexelBuffer usage.
TEST_F(TexelBufferValidationWithExtendedMapTest, MappableUsageWithFeatureEnabled) {
    wgpu::BufferDescriptor desc;
    desc.size = 4;

    desc.usage = wgpu::BufferUsage::TexelBuffer | wgpu::BufferUsage::MapRead;
    device.CreateBuffer(&desc);

    desc.usage = wgpu::BufferUsage::TexelBuffer | wgpu::BufferUsage::MapWrite;
    device.CreateBuffer(&desc);
}

class TexelBufferFeatureDisabledTest : public ValidationTest {
  protected:
    void SetUp() override {
        std::vector<const char*> features = {"texel_buffers"};
        wgpu::DawnWGSLBlocklist blocklist;
        blocklist.blocklistedFeatureCount = features.size();
        blocklist.blocklistedFeatures = features.data();

        wgpu::DawnTogglesDescriptor togglesDesc;
        togglesDesc.nextInChain = &blocklist;

        wgpu::InstanceDescriptor nativeDesc;
        nativeDesc.nextInChain = &togglesDesc;

        wgpu::DawnWireWGSLControl wgslControl;
        wgslControl.nextInChain = &blocklist;
        wgslControl.enableExperimental = false;
        wgslControl.enableUnsafe = false;
        wgslControl.enableTesting = true;

        wgpu::InstanceDescriptor wireDesc;
        wireDesc.nextInChain = &wgslControl;

        ValidationTest::SetUp(&nativeDesc, &wireDesc);
    }
};

// Creating a buffer with texel buffer bit without enabling the feature fails.
TEST_F(TexelBufferFeatureDisabledTest, BufferRequiresFeature) {
    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::TexelBuffer;
    ASSERT_DEVICE_ERROR(device.CreateBuffer(&desc));
}

// Creating a texel buffer layout without enabling the feature fails.
TEST_F(TexelBufferFeatureDisabledTest, LayoutRequiresFeature) {
    wgpu::TexelBufferBindingLayout layout = {};
    layout.access = wgpu::TexelBufferAccess::ReadOnly;
    layout.format = wgpu::TextureFormat::RGBA8Uint;

    wgpu::BindGroupLayoutEntry entry = {};
    entry.binding = 0;
    entry.visibility = wgpu::ShaderStage::Fragment;
    entry.nextInChain = &layout;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &entry;

    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
}

}  // anonymous namespace
}  // namespace dawn
