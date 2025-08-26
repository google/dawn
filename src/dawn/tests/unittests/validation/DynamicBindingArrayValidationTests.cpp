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

#include "dawn/common/Constants.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class DynamicBindingArrayTests : public ValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::ChromiumExperimentalBindless};
    }

    // Helper similar to utils::MakeBindGroupLayout but that adds a dynamic array.
    wgpu::BindGroupLayout MakeBindGroupLayout(
        wgpu::DynamicBindingKind kind,
        uint32_t dynamicArrayStart = 0,
        std::initializer_list<utils::BindingLayoutEntryInitializationHelper> entriesInitializer =
            {}) {
        std::vector<wgpu::BindGroupLayoutEntry> entries;
        for (const utils::BindingLayoutEntryInitializationHelper& entry : entriesInitializer) {
            entries.push_back(entry);
        }

        wgpu::BindGroupLayoutDynamicBindingArray dynamic;
        dynamic.dynamicArray.kind = kind;
        dynamic.dynamicArray.start = dynamicArrayStart;

        wgpu::BindGroupLayoutDescriptor descriptor;
        descriptor.nextInChain = &dynamic;
        descriptor.entryCount = entries.size();
        descriptor.entries = entries.data();
        return device.CreateBindGroupLayout(&descriptor);
    }

    // Helper similar to utils::MakeBindGroup but that adds a dynamic array.
    wgpu::BindGroup MakeBindGroup(
        const wgpu::BindGroupLayout& layout,
        uint32_t dynamicArraySize,
        std::initializer_list<utils::BindingInitializationHelper> entriesInitializer) {
        std::vector<wgpu::BindGroupEntry> entries;
        for (const utils::BindingInitializationHelper& helper : entriesInitializer) {
            entries.push_back(helper.GetAsBinding());
        }

        wgpu::BindGroupDynamicBindingArray dynamic;
        dynamic.dynamicArraySize = dynamicArraySize;

        wgpu::BindGroupDescriptor descriptor;
        descriptor.nextInChain = &dynamic;
        descriptor.layout = layout;
        descriptor.entryCount = entries.size();
        descriptor.entries = entries.data();

        return device.CreateBindGroup(&descriptor);
    }
};

class DynamicBindingArrayTests_FeatureDisabled : public ValidationTest {};

// Control case where creating a dynamic binding array layout with the feature enabled is valid.
TEST_F(DynamicBindingArrayTests, LayoutSuccessWithFeatureEnabled) {
    wgpu::BindGroupLayoutDynamicBindingArray dynamic;
    dynamic.dynamicArray.kind = wgpu::DynamicBindingKind::SampledTexture;

    wgpu::BindGroupLayoutDescriptor desc;
    desc.nextInChain = &dynamic;

    // No error is produced.
    device.CreateBindGroupLayout(&desc);
}

// Error case where creating a dynamic binding array layout with the feature disabled is an error.
TEST_F(DynamicBindingArrayTests_FeatureDisabled, LayoutErrorWithFeatureDisabled) {
    wgpu::BindGroupLayoutDynamicBindingArray dynamic;
    dynamic.dynamicArray.kind = wgpu::DynamicBindingKind::SampledTexture;

    wgpu::BindGroupLayoutDescriptor desc;
    desc.nextInChain = &dynamic;

    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
}

// Control case where creating a dynamic binding array bind group with the feature enabled is valid.
TEST_F(DynamicBindingArrayTests, GroupSuccessWithFeatureEnabled) {
    wgpu::BindGroupDynamicBindingArray dynamic;
    dynamic.dynamicArraySize = 1;

    wgpu::BindGroupDescriptor desc;
    desc.nextInChain = &dynamic;
    desc.layout = MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture);

    // No error is produced.
    device.CreateBindGroup(&desc);
}

// Error case where creating a dynamic binding array bind group with the feature disabled is an
// error.
TEST_F(DynamicBindingArrayTests_FeatureDisabled, GroupErrorWithFeatureDisabled) {
    wgpu::BindGroupDynamicBindingArray dynamic;
    dynamic.dynamicArraySize = 1;

    wgpu::BindGroupDescriptor desc;
    desc.nextInChain = &dynamic;
    desc.layout = utils::MakeBindGroupLayout(device, {});

    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&desc));
}

// Check that using DynamicArrayKind::Undefined is an error.
TEST_F(DynamicBindingArrayTests, UndefinedArrayKind) {
    wgpu::BindGroupLayoutDynamicBindingArray dynamic;

    wgpu::BindGroupLayoutDescriptor desc;
    desc.nextInChain = &dynamic;

    // Control case: SampledTexture is a valid kind.
    dynamic.dynamicArray.kind = wgpu::DynamicBindingKind::SampledTexture;
    device.CreateBindGroupLayout(&desc);

    // Error case: Undefined is invalid.
    dynamic.dynamicArray.kind = wgpu::DynamicBindingKind::Undefined;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
}

// Check that the start of the binding array must be less than maxBindingsPerBindGroup
TEST_F(DynamicBindingArrayTests, DynamicArrayStartLimit) {
    wgpu::BindGroupLayoutDynamicBindingArray dynamic;
    dynamic.dynamicArray.kind = wgpu::DynamicBindingKind::SampledTexture;

    wgpu::BindGroupLayoutDescriptor desc;
    desc.nextInChain = &dynamic;

    // No error is produced if we are under the limit.
    dynamic.dynamicArray.start = kMaxBindingsPerBindGroup - 1;
    device.CreateBindGroupLayout(&desc);

    // Error case if we are at the limit.
    dynamic.dynamicArray.start = kMaxBindingsPerBindGroup;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));

    // Error case if we are above the limit.
    dynamic.dynamicArray.start = kMaxBindingsPerBindGroup + 1;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
}

// Check that conflicts of binding number are not allowed between dynamic and static bindings.
TEST_F(DynamicBindingArrayTests, ConflictWithStaticBindings) {
    wgpu::BindGroupLayoutEntry entry;
    entry.binding = 0;
    entry.bindingArraySize = 0;
    entry.texture.sampleType = wgpu::TextureSampleType::Float;

    wgpu::BindGroupLayoutDynamicBindingArray dynamic;
    dynamic.dynamicArray.kind = wgpu::DynamicBindingKind::SampledTexture;
    dynamic.dynamicArray.start = 3;

    wgpu::BindGroupLayoutDescriptor desc;
    desc.nextInChain = &dynamic;
    desc.entryCount = 1;
    desc.entries = &entry;

    // Control case: the non-arrayed static binding is before the dynamic array.
    entry.binding = 2;
    entry.bindingArraySize = 1;
    device.CreateBindGroupLayout(&desc);

    // Error case: the non-arrayed static binding is after the dynamic array.
    entry.binding = 3;
    entry.bindingArraySize = 1;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));

    // Control case: the arrayed static binding is before the dynamic array.
    entry.binding = 0;
    entry.bindingArraySize = 3;
    device.CreateBindGroupLayout(&desc);

    // Error case: the arrayed static binding is after the dynamic array.
    entry.binding = 0;
    entry.bindingArraySize = 4;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&desc));
}

// Check that the layout must have a dynamic array, even if the dynamicArraySize is 0.
TEST_F(DynamicBindingArrayTests, LayoutNoDynamicArray) {
    wgpu::BindGroupDynamicBindingArray dynamic;

    wgpu::BindGroupDescriptor desc;
    desc.nextInChain = &dynamic;

    // Control case: dynamicArraySize = 1 is valid with a layout with a dynamic binding array.
    dynamic.dynamicArraySize = 1;
    desc.layout = MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture);
    device.CreateBindGroup(&desc);

    // Control case: dynamicArraySize = 0 is valid with a layout with a dynamic binding array.
    dynamic.dynamicArraySize = 0;
    device.CreateBindGroup(&desc);

    // Error case: dynamicArraySize > 0 requires the layout to have a dynamic binding array.
    dynamic.dynamicArraySize = 1;
    desc.layout = utils::MakeBindGroupLayout(device, {});
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&desc));

    // Error case: dynamicArraySize = 0 requires the layout to have a dynamic binding array.
    dynamic.dynamicArraySize = 1;
    desc.layout = utils::MakeBindGroupLayout(device, {});
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&desc));
}

// Check that the dynamic array size must be below the limit.
TEST_F(DynamicBindingArrayTests, DynamicArraySizeLimit) {
    wgpu::BindGroupLayoutDynamicBindingArray layoutDynamic;
    layoutDynamic.dynamicArray.kind = wgpu::DynamicBindingKind::SampledTexture;
    wgpu::BindGroupLayoutDescriptor layoutDesc;
    layoutDesc.nextInChain = &layoutDynamic;

    wgpu::BindGroupDynamicBindingArray dynamic;
    wgpu::BindGroupDescriptor desc;
    desc.nextInChain = &dynamic;
    desc.layout = device.CreateBindGroupLayout(&layoutDesc);

    // Control case: reaching the limit is valid.
    dynamic.dynamicArraySize = kMaxDynamicBindingArraySize;
    device.CreateBindGroup(&desc);

    // Error case: going above the limit isn't.
    dynamic.dynamicArraySize = kMaxDynamicBindingArraySize + 1;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&desc));
}

// Check that a dynamic array counts as one additional storage buffer when checking against limits.
TEST_F(DynamicBindingArrayTests, UsesOneStorageBufferTowardsLimit) {
    const uint32_t maxStorageBuffers = deviceLimits.maxStorageBuffersPerShaderStage;
    std::vector<wgpu::BindGroupLayoutEntry> storageBufferEntries(maxStorageBuffers);
    for (size_t i = 0; i < storageBufferEntries.size(); i++) {
        storageBufferEntries[i].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
        storageBufferEntries[i].visibility =
            wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Compute;
        storageBufferEntries[i].binding = i;
    }

    wgpu::BindGroupLayoutDynamicBindingArray layoutDynamic;
    layoutDynamic.dynamicArray.kind = wgpu::DynamicBindingKind::SampledTexture;
    layoutDynamic.dynamicArray.start = maxStorageBuffers + 3;
    wgpu::BindGroupLayoutDescriptor layoutDesc;
    layoutDesc.nextInChain = &layoutDynamic;
    layoutDesc.entries = storageBufferEntries.data();

    // Success case: exactly maxStorageBuffers are used (1 for the dynamic array, max - 1 for the
    // the static bindings).
    layoutDesc.entryCount = maxStorageBuffers - 1;
    device.CreateBindGroupLayout(&layoutDesc);

    // Error case: the dynamic binding array storage buffer make the layout go over the limit.
    layoutDesc.entryCount = maxStorageBuffers;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&layoutDesc));
}

// Check that the dynamic array size must be below the limit.
TEST_F(DynamicBindingArrayTests, DynamicArrayRequiresASize) {
    wgpu::BindGroupLayoutDynamicBindingArray layoutDynamic;
    layoutDynamic.dynamicArray.kind = wgpu::DynamicBindingKind::SampledTexture;
    wgpu::BindGroupLayoutDescriptor layoutDesc;
    layoutDesc.nextInChain = &layoutDynamic;

    wgpu::BindGroupDynamicBindingArray dynamic;
    dynamic.dynamicArraySize = 0;
    wgpu::BindGroupDescriptor desc;
    desc.layout = device.CreateBindGroupLayout(&layoutDesc);

    // Control case: a size, even of 0 is valid.
    desc.nextInChain = &dynamic;
    device.CreateBindGroup(&desc);

    // Error case: an unspecified size is invalid.
    desc.nextInChain = nullptr;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&desc));
}

// Check that specifying an entry that's neither a known static one or a dynamic one is an error.
TEST_F(DynamicBindingArrayTests, EntryNeitherStaticNorDynamic) {
    // Create a layout with a static entry at 0 and a dynamic binding array starting at 2.
    wgpu::BindGroupLayoutDynamicBindingArray layoutDynamic;
    layoutDynamic.dynamicArray.kind = wgpu::DynamicBindingKind::SampledTexture;
    layoutDynamic.dynamicArray.start = 2;
    wgpu::BindGroupLayoutEntry layoutEntry;
    layoutEntry.binding = 0;
    layoutEntry.texture.sampleType = wgpu::TextureSampleType::Float;
    wgpu::BindGroupLayoutDescriptor layoutDesc;
    layoutDesc.nextInChain = &layoutDynamic;
    layoutDesc.entryCount = 1;
    layoutDesc.entries = &layoutEntry;

    // Create the texture to put in the bind group.
    wgpu::TextureDescriptor tDesc;
    tDesc.size = {1, 1};
    tDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    tDesc.usage = wgpu::TextureUsage::TextureBinding;
    wgpu::Texture texture = device.CreateTexture(&tDesc);

    // Create a bind group on that layout with one or two static entries for 0/1.
    wgpu::BindGroupDynamicBindingArray dynamic;
    dynamic.dynamicArraySize = 10;
    wgpu::BindGroupEntry entries[2];
    entries[0].binding = 0;
    entries[0].textureView = texture.CreateView();
    entries[1].binding = 1;
    entries[1].textureView = texture.CreateView();
    wgpu::BindGroupDescriptor desc;
    desc.nextInChain = &dynamic;
    desc.layout = device.CreateBindGroupLayout(&layoutDesc);
    desc.entries = entries;

    // Control case: specifying only entry 0 which is in the layout is valid.
    desc.entryCount = 1;
    device.CreateBindGroup(&desc);

    // Error case: specifying entry 1 which is not in the layout is an error.
    desc.entryCount = 2;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&desc));
}

// Check that even when the bind group has a dynamic binding array, forgetting a static entry is an
// error.
TEST_F(DynamicBindingArrayTests, MissingStaticEntry) {
    // Create a layout with a static entry at 0 and a dynamic binding array starting at 1.
    wgpu::BindGroupLayoutDynamicBindingArray layoutDynamic;
    layoutDynamic.dynamicArray.kind = wgpu::DynamicBindingKind::SampledTexture;
    layoutDynamic.dynamicArray.start = 1;
    wgpu::BindGroupLayoutEntry layoutEntry;
    layoutEntry.binding = 0;
    layoutEntry.texture.sampleType = wgpu::TextureSampleType::Float;
    wgpu::BindGroupLayoutDescriptor layoutDesc;
    layoutDesc.nextInChain = &layoutDynamic;
    layoutDesc.entryCount = 1;
    layoutDesc.entries = &layoutEntry;

    // Create the texture to put in the bind group.
    wgpu::TextureDescriptor tDesc;
    tDesc.size = {1, 1};
    tDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    tDesc.usage = wgpu::TextureUsage::TextureBinding;
    wgpu::Texture texture = device.CreateTexture(&tDesc);

    // Create a bind group on that layout with or without a static entry at binding 0.
    wgpu::BindGroupDynamicBindingArray dynamic;
    dynamic.dynamicArraySize = 10;
    wgpu::BindGroupEntry entry;
    entry.binding = 0;
    entry.textureView = texture.CreateView();
    wgpu::BindGroupDescriptor desc;
    desc.nextInChain = &dynamic;
    desc.layout = device.CreateBindGroupLayout(&layoutDesc);
    desc.entries = &entry;

    // Control case: static entry 0 is specified.
    desc.entryCount = 1;
    device.CreateBindGroup(&desc);

    // Error case: static entry 0 is missing, which is an error.
    desc.entryCount = 0;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&desc));
}

// Check that dynamic entries must be in bounds of the dynamic binding array.
TEST_F(DynamicBindingArrayTests, BindingPastDynamicArray) {
    // Create a layout with a static entry at 0 and a dynamic binding array starting at 1.
    wgpu::BindGroupLayoutDynamicBindingArray layoutDynamic;
    layoutDynamic.dynamicArray.kind = wgpu::DynamicBindingKind::SampledTexture;
    layoutDynamic.dynamicArray.start = 2;
    wgpu::BindGroupLayoutDescriptor layoutDesc;
    layoutDesc.nextInChain = &layoutDynamic;

    // Create the texture to put in the bind group.
    wgpu::TextureDescriptor tDesc;
    tDesc.size = {1, 1};
    tDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    tDesc.usage = wgpu::TextureUsage::TextureBinding;
    wgpu::Texture texture = device.CreateTexture(&tDesc);

    // Create a bind group on that layout with an entry at binding 11 or 12.
    wgpu::BindGroupDynamicBindingArray dynamic;
    dynamic.dynamicArraySize = 10;
    wgpu::BindGroupEntry entry;
    entry.textureView = texture.CreateView();
    wgpu::BindGroupDescriptor desc;
    desc.nextInChain = &dynamic;
    desc.layout = device.CreateBindGroupLayout(&layoutDesc);
    desc.entries = &entry;
    desc.entryCount = 1;

    // Control case: entry is exactly at the end of the dynamic binding array.
    entry.binding = 11;
    device.CreateBindGroup(&desc);

    // Error case: entry is past the end of the dynamic binding array, which is an error.
    entry.binding = 12;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&desc));
}

// Check that dynamic entries must not conflict.
TEST_F(DynamicBindingArrayTests, DynamicEntryConflict) {
    wgpu::BindGroupLayoutDynamicBindingArray layoutDynamic;
    layoutDynamic.dynamicArray.kind = wgpu::DynamicBindingKind::SampledTexture;
    wgpu::BindGroupLayoutDescriptor layoutDesc;
    layoutDesc.nextInChain = &layoutDynamic;

    // Create the texture to put in the bind group.
    wgpu::TextureDescriptor tDesc;
    tDesc.size = {1, 1};
    tDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    tDesc.usage = wgpu::TextureUsage::TextureBinding;
    wgpu::Texture texture = device.CreateTexture(&tDesc);

    // Create a bind group on that layout with one or two static entries for 0/1.
    wgpu::BindGroupDynamicBindingArray dynamic;
    dynamic.dynamicArraySize = 10;
    wgpu::BindGroupEntry entries[2];
    entries[0].textureView = texture.CreateView();
    entries[1].textureView = texture.CreateView();
    wgpu::BindGroupDescriptor desc;
    desc.nextInChain = &dynamic;
    desc.layout = device.CreateBindGroupLayout(&layoutDesc);
    desc.entryCount = 2;
    desc.entries = entries;

    // Control case: dynamic entries have different binding indices.
    entries[0].binding = 0;
    entries[1].binding = 1;
    device.CreateBindGroup(&desc);

    // Error case: dynamic entries have the same binding index, which is an error.
    entries[0].binding = 0;
    entries[1].binding = 0;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&desc));
}

// Check that DynamicBindingKind::SampledTexture must be a texture entry.
// TODO(https://issues.chromium.org/435251399): Figure out the additional validation rules for the
// texture kind.
TEST_F(DynamicBindingArrayTests, SampledTextureKindRequiresTexture) {
    wgpu::BindGroupLayoutDynamicBindingArray layoutDynamic;
    layoutDynamic.dynamicArray.kind = wgpu::DynamicBindingKind::SampledTexture;
    wgpu::BindGroupLayoutDescriptor layoutDesc;
    layoutDesc.nextInChain = &layoutDynamic;

    // Create the texture to put in the bind group.
    wgpu::TextureDescriptor tDesc;
    tDesc.size = {1, 1};
    tDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    tDesc.usage = wgpu::TextureUsage::TextureBinding;
    wgpu::Texture texture = device.CreateTexture(&tDesc);

    // Create the buffer to put in the bind group.
    wgpu::BufferDescriptor bDesc;
    bDesc.size = 4;
    bDesc.usage = wgpu::BufferUsage::Storage;
    wgpu::Buffer buffer = device.CreateBuffer(&bDesc);

    // Create a bind group on that layout with one or two static entries for 0/1.
    wgpu::BindGroupDynamicBindingArray dynamic;
    dynamic.dynamicArraySize = 10;
    wgpu::BindGroupDescriptor desc;
    desc.nextInChain = &dynamic;
    desc.layout = device.CreateBindGroupLayout(&layoutDesc);
    desc.entryCount = 1;

    // Control case: entry is a texture, which is valid.
    wgpu::BindGroupEntry textureEntry;
    textureEntry.binding = 0;
    textureEntry.textureView = texture.CreateView();
    desc.entries = &textureEntry;
    device.CreateBindGroup(&desc);

    // Error case: entry is a buffer, which is an error.
    wgpu::BindGroupEntry bufferEntry;
    bufferEntry.binding = 0;
    bufferEntry.buffer = buffer;
    desc.entries = &bufferEntry;
    ASSERT_DEVICE_ERROR(device.CreateBindGroup(&desc));
}

// Test that it is an error to call .Destroy() on a bind group without a dynamic array.
TEST_F(DynamicBindingArrayTests, DestroyDisallowedOnStaticOnlyBindGroup) {
    // Create an empty dynamic bind group.
    wgpu::BindGroupLayout layoutDynamic =
        MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture);
    wgpu::BindGroup bgDynamic = MakeBindGroup(layoutDynamic, 0, {});

    // Create a static only bind group.
    wgpu::BindGroupLayout staticLayout = utils::MakeBindGroupLayout(device, {});
    wgpu::BindGroup bgStatic = utils::MakeBindGroup(device, staticLayout, {});

    // Success case: calling .Destroy() on a dynamic array bind group is valid.
    bgDynamic.Destroy();
    // Calling it multiple times is even valid!
    bgDynamic.Destroy();

    // Error case: calling .Destroy() on a static only bind group is an error.
    ASSERT_DEVICE_ERROR(bgStatic.Destroy());
}

// Test that using a destroyed dynamic binding array in a render pass in a submit is an error.
TEST_F(DynamicBindingArrayTests, DestroyedDynamicBindingArrayUsedInRenderPass) {
    // Create an empty dynamic bind group.
    wgpu::BindGroupLayout layout = MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture);
    wgpu::BindGroup bg = MakeBindGroup(layout, 0, {});

    for (bool destroy : {false, true}) {
        if (destroy) {
            bg.Destroy();
        }

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        auto rp = utils::CreateBasicRenderPass(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        pass.SetBindGroup(0, bg);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        if (destroy) {
            ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
        } else {
            device.GetQueue().Submit(1, &commands);
        }
    }
}

// Test that using a destroyed dynamic binding array in a compute pass in a submit is an error.
TEST_F(DynamicBindingArrayTests, DestroyedDynamicBindingArrayUsedInComputePass) {
    // Create an empty dynamic bind group.
    wgpu::BindGroupLayout layout = MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture);
    wgpu::BindGroup bg = MakeBindGroup(layout, 0, {});

    for (bool destroy : {false, true}) {
        if (destroy) {
            bg.Destroy();
        }

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetBindGroup(0, bg);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        if (destroy) {
            ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
        } else {
            device.GetQueue().Submit(1, &commands);
        }
    }
}

// Test that using a destroyed dynamic binding array in a render bundle in a submit is an error.
TEST_F(DynamicBindingArrayTests, DestroyedDynamicBindingArrayUsedInRenderBundle) {
    // Create an empty dynamic bind group.
    wgpu::BindGroupLayout layout = MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture);
    wgpu::BindGroup bg = MakeBindGroup(layout, 0, {});

    // Create the render bundle
    wgpu::TextureFormat passFormat = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::RenderBundleEncoderDescriptor rbDesc;
    rbDesc.colorFormatCount = 1;
    rbDesc.colorFormats = &passFormat;

    wgpu::RenderBundleEncoder rbEncoder = device.CreateRenderBundleEncoder(&rbDesc);
    rbEncoder.SetBindGroup(0, bg);
    wgpu::RenderBundle bundle = rbEncoder.Finish();

    for (bool destroy : {false, true}) {
        if (destroy) {
            bg.Destroy();
        }

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        auto rp = utils::CreateBasicRenderPass(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        pass.ExecuteBundles(1, &bundle);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        if (destroy) {
            ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
        } else {
            device.GetQueue().Submit(1, &commands);
        }
    }
}

}  // anonymous namespace
}  // namespace dawn
