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
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
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

// Check that dynamic binding arrays are not allowed in combination with an external texture in the
// static bindings part.
// TODO(https://issues.chromium.org/435251399): Remove this constraint that's only a workaround
// while prototyping.
TEST_F(DynamicBindingArrayTests, NotAllowedWithExternalTextures) {
    // Control case, static buffer binding + dynamic binding array is allowed.
    MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture, 5,
                        {{
                            0,
                            wgpu::ShaderStage::Fragment,
                            wgpu::BufferBindingType::Uniform,
                        }});

    // Error case, static buffer binding + dynamic binding array is an error.
    ASSERT_DEVICE_ERROR(MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture, 5,
                                            {{
                                                0,
                                                wgpu::ShaderStage::Fragment,
                                                &utils::kExternalTextureBindingLayout,
                                            }}));
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

// Test that it is an error to call .Destroy() on an error bind group. This is a regression test for
// an issues where doing so was triggering an ASSERT.
TEST_F(DynamicBindingArrayTests, DestroyOnErrorBindGroup) {
    // Create an error bind group.
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform}});
    wgpu::BindGroup bg;
    ASSERT_DEVICE_ERROR(bg = utils::MakeBindGroup(device, layout, {}));

    ASSERT_DEVICE_ERROR(bg.Destroy());
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

// Test that using the WGSL enable requires the bindless extension.
TEST_F(DynamicBindingArrayTests_FeatureDisabled, WGSLEnableNotAllowed) {
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        enable chromium_experimental_dynamic_binding;
        @group(0) @binding(0) var a : resource_binding;

        @compute @workgroup_size(1) fn main() {
            _ = hasBinding<texture_2d<f32>>(a, 42);
        }
    )"));
}

// Test that a shader using a dynamic binding array requires a layout with one.
TEST_F(DynamicBindingArrayTests, ShaderRequiresLayoutWithDynamicArray) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_dynamic_binding;
        @group(0) @binding(0) var a : resource_binding;

        @compute @workgroup_size(1) fn main() {
            _ = hasBinding<texture_2d<f32>>(a, 42);
        }
    )");

    // Success case, the layout has a dynamic binding array.
    wgpu::BindGroupLayout bglDynamic =
        MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture);
    csDesc.layout = utils::MakeBasicPipelineLayout(device, &bglDynamic);
    device.CreateComputePipeline(&csDesc);

    // Error case, the layout doesn't have a dynamic binding array.
    wgpu::BindGroupLayout bglStatic = utils::MakeBindGroupLayout(device, {});
    csDesc.layout = utils::MakeBasicPipelineLayout(device, &bglStatic);
    ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&csDesc));

    // Error case, the layout doesn't have a dynamic binding array (even if there is a similar
    // looking binding).
    wgpu::BindGroupLayout bglStaticWithTexture = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::Float}});
    csDesc.layout = utils::MakeBasicPipelineLayout(device, &bglStaticWithTexture);
    ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&csDesc));
}

// Test that it is valid to have a layout specifying a dynamic binding array with a shader that
// doesn't have one.
TEST_F(DynamicBindingArrayTests, ShaderNoDynamicArrayWithLayoutThatHasOne) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(1) fn main() {
        }
    )");
    wgpu::BindGroupLayout bglDynamic =
        MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture);
    csDesc.layout = utils::MakeBasicPipelineLayout(device, &bglDynamic);
    device.CreateComputePipeline(&csDesc);
}

// Test that the dynamic array start must match between shader and layout.
TEST_F(DynamicBindingArrayTests, ShaderAndLayoutDynamicArrayStartMatches) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_dynamic_binding;
        @group(0) @binding(1) var a : resource_binding;

        @compute @workgroup_size(1) fn main() {
            _ = hasBinding<texture_2d<f32>>(a, 42);
        }
    )");

    // Success case, start of the array matches.
    wgpu::BindGroupLayout bgl1 = MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture, 1);
    csDesc.layout = utils::MakeBasicPipelineLayout(device, &bgl1);
    device.CreateComputePipeline(&csDesc);

    // Error case, layout start is before the shader's start.
    wgpu::BindGroupLayout bgl0 = MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture, 0);
    csDesc.layout = utils::MakeBasicPipelineLayout(device, &bgl0);
    ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&csDesc));

    // Error case, layout start is after the shader's start.
    wgpu::BindGroupLayout bgl2 = MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture, 2);
    csDesc.layout = utils::MakeBasicPipelineLayout(device, &bgl2);
    ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&csDesc));
}

// Test that the @binding decoration of the dynamic array must be less than maxBindingsPerBindGroup.
TEST_F(DynamicBindingArrayTests, ShaderArrayStartLessThanMaxBindingsPerBindGroup) {
    wgpu::BindGroupLayout bgl =
        MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture, kMaxBindingsPerBindGroup - 1);

    // Control case, we are just below the limit.
    {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = utils::CreateShaderModule(device, R"(
                enable chromium_experimental_dynamic_binding;
                @group(0) @binding()" + std::to_string(kMaxBindingsPerBindGroup - 1) +
                                                                      R"() var a : resource_binding;

                @compute @workgroup_size(1) fn main() {
                    _ = hasBinding<texture_2d<f32>>(a, 42);
                }
            )");
        csDesc.layout = utils::MakeBasicPipelineLayout(device, &bgl);
        device.CreateComputePipeline(&csDesc);
    }

    // Error case, we are above the limit.
    {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = utils::CreateShaderModule(device, R"(
                enable chromium_experimental_dynamic_binding;
                @group(0) @binding()" + std::to_string(kMaxBindingsPerBindGroup) +
                                                                      R"() var a : resource_binding;

                @compute @workgroup_size(1) fn main() {
                    _ = hasBinding<texture_2d<f32>>(a, 42);
                }
            )");
        csDesc.layout = utils::MakeBasicPipelineLayout(device, &bgl);
        // Two errors happen because we cannot create a layout that matches, but check that the
        // shader's validation about maxBindingsPerBindGroup is the one that's reported.
        ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&csDesc),
                            testing::HasSubstr("maxBindingsPerBindGroup"));
    }
}

// Test that the @group decoration of the dynamic array must be less than maxBindGroups.
TEST_F(DynamicBindingArrayTests, ShaderArrayAtMaxBindGroups) {
    std::array<wgpu::BindGroupLayout, kMaxBindGroups> bgls;
    bgls[bgls.size() - 1] = MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture);

    wgpu::PipelineLayoutDescriptor plDesc;
    plDesc.bindGroupLayoutCount = bgls.size();
    plDesc.bindGroupLayouts = bgls.data();
    wgpu::PipelineLayout pl = device.CreatePipelineLayout(&plDesc);

    // Control case, we are just below the limit.
    {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module =
            utils::CreateShaderModule(device,
                                      R"(
                enable chromium_experimental_dynamic_binding;
                @group()" + std::to_string(kMaxBindGroups - 1) +
                                          R"() @binding(0) var a : resource_binding;

                @compute @workgroup_size(1) fn main() {
                    _ = hasBinding<texture_2d<f32>>(a, 42);
                }
            )");
        csDesc.layout = pl;
        device.CreateComputePipeline(&csDesc);
    }

    // Error case, we are above the limit.
    {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module =
            utils::CreateShaderModule(device,
                                      R"(
                enable chromium_experimental_dynamic_binding;
                @group()" + std::to_string(kMaxBindGroups) +
                                          R"() @binding(0) var a : resource_binding;

                @compute @workgroup_size(1) fn main() {
                    _ = hasBinding<texture_2d<f32>>(a, 42);
                }
            )");
        csDesc.layout = pl;
        // Two errors happen because we cannot create a layout that matches, but check that the
        // shader's validation about maxBindingsPerBindGroup is the one that's reported.
        ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&csDesc),
                            testing::HasSubstr("maxBindGroups"));
    }
}

// Test that the group for the dynamic binding array must be in the PipelineLayout.
TEST_F(DynamicBindingArrayTests, ShaderBindingArrayMustHaveGroupInPipelineLayout) {
    std::array<wgpu::BindGroupLayout, 3> bgls = {
        nullptr, MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture), nullptr};

    wgpu::PipelineLayoutDescriptor plDesc;
    plDesc.bindGroupLayoutCount = bgls.size();
    plDesc.bindGroupLayouts = bgls.data();

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.layout = device.CreatePipelineLayout(&plDesc);

    // Control case, the group is in the pipeline layout.
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_dynamic_binding;
        @group(1) @binding(0) var a : resource_binding;

        @compute @workgroup_size(1) fn main() {
            _ = hasBinding<texture_2d<f32>>(a, 42);
        }
    )");
    device.CreateComputePipeline(&csDesc);

    // Error case, the group is not in the layout (@group(0) case)
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_dynamic_binding;
        @group(0) @binding(0) var a : resource_binding;

        @compute @workgroup_size(1) fn main() {
            _ = hasBinding<texture_2d<f32>>(a, 42);
        }
    )");
    ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&csDesc));

    // Error case, the group is not in the layout (@group(2) case)
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_dynamic_binding;
        @group(2) @binding(0) var a : resource_binding;

        @compute @workgroup_size(1) fn main() {
            _ = hasBinding<texture_2d<f32>>(a, 42);
        }
    )");
    ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&csDesc));
}

// Test that a shader cannot have two dynamic binding arrays on the same group.
TEST_F(DynamicBindingArrayTests, ShaderTwoDynamicArraysSameGroupIsAnError) {
    // Control case, the two dynamic binding arrays are on different groups.
    utils::CreateShaderModule(device, R"(
        enable chromium_experimental_dynamic_binding;
        @group(0) @binding(0) var a : resource_binding;
        @group(1) @binding(1) var b : resource_binding;

        @compute @workgroup_size(1) fn main() {
            _ = hasBinding<texture_2d<f32>>(a, 42);
            _ = hasBinding<texture_2d<f32>>(b, 42);
        }
    )");

    // Error case, the two dynamic binding arrays are on the same group.
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        enable chromium_experimental_dynamic_binding;
        @group(0) @binding(0) var a : resource_binding;
        @group(0) @binding(1) var b : resource_binding;

        @compute @workgroup_size(1) fn main() {
            _ = hasBinding<texture_2d<f32>>(a, 42);
            _ = hasBinding<texture_2d<f32>>(b, 42);
        }
    )"));
}

// Test that a shader using only arrayLength on the dynamic binding array is compatible with any
// DynamicBindingKind.
TEST_F(DynamicBindingArrayTests, DynamicArrayKindWithoutTypeInfoValidWithAllLayouts) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_dynamic_binding;
        @group(0) @binding(0) var a : resource_binding;

        @compute @workgroup_size(1) fn main() {
            _ = arrayLength(a);
        }
    )");

    // Check that it is compatible with DynamicBindingKind::SampledTexture.
    {
        wgpu::BindGroupLayout bgl = MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture);
        csDesc.layout = utils::MakeBasicPipelineLayout(device, &bgl);
        device.CreateComputePipeline(&csDesc);
    }

    // TODO(https://crbug.com/435317394): Add tests with additional DynamicBindingKind.
}

// TODO(https://crbug.com/435317394): Add tests for the DynamicArrayKind. It is not possible to do
// it at the moment because we cannot reflect DynamicArrayKind::Undefined (would require referencing
// but not indexing the array) or any value that's not DynamicArrayKind::SampledTexture (no support
// in Dawn or Tint for other cases). Tests to add after that are:
//  - The kind in the layout must match the deduced kind for the shader.
//     - Case with a resource_binding
//  - An error is produced at shader module compilation time if it uses the same resource_binding
//    with different DynamicArrayKinds.
//

// Test that BGL defaulting works with dynamic binding arrays.
TEST_F(DynamicBindingArrayTests, GetBGLSuccess) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.layout = nullptr;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_dynamic_binding;
        @group(0) @binding(0) var a : resource_binding;

        @compute @workgroup_size(1) fn main() {
            _ = hasBinding<texture_2d<f32>>(a, 42);
        }
    )");
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    MakeBindGroup(pipeline.GetBindGroupLayout(0), 18, {});
}

// Test that the correct array start is defaulted.
TEST_F(DynamicBindingArrayTests, GetBGLDefaultedArrayStart) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.layout = nullptr;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_dynamic_binding;
        @group(0) @binding(7) var a : resource_binding;

        @compute @workgroup_size(1) fn main() {
            _ = hasBinding<texture_2d<f32>>(a ,42);
        }
    )");
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Create the texture to use in the bind group.
    wgpu::TextureDescriptor tDesc;
    tDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    tDesc.size = {1, 1};
    tDesc.usage = wgpu::TextureUsage::TextureBinding;
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    // Success case, the array starts at 7 so a texture at binding 7 and 7 + 1 is valid.
    MakeBindGroup(pipeline.GetBindGroupLayout(0), 2,
                  {
                      {7, tex.CreateView()},
                      {8, tex.CreateView()},
                  });

    // Error case, the texture is before the start of the binding array.
    ASSERT_DEVICE_ERROR(MakeBindGroup(pipeline.GetBindGroupLayout(0), 2,
                                      {
                                          {6, tex.CreateView()},
                                      }));

    // Error case, the texture is after the end of the binding array.
    ASSERT_DEVICE_ERROR(MakeBindGroup(pipeline.GetBindGroupLayout(0), 2,
                                      {
                                          {9, tex.CreateView()},
                                      }));
}

// Test that the correct array kind is defaulted.
TEST_F(DynamicBindingArrayTests, GetBGLDefaultedArrayKind) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.layout = nullptr;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_dynamic_binding;
        @group(0) @binding(0) var a : resource_binding;

        @compute @workgroup_size(1) fn main() {
            _ = hasBinding<texture_2d<f32>>(a, 42);
        }
    )");
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Create the texture to use in the bind group.
    wgpu::TextureDescriptor tDesc;
    tDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    tDesc.size = {1, 1};
    tDesc.usage = wgpu::TextureUsage::TextureBinding;
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    // Success case, the BGL will have DynamicArrayKind::SampledTexture.
    MakeBindGroup(pipeline.GetBindGroupLayout(0), 1,
                  {
                      {0, tex.CreateView()},
                  });

    // TODO(https://crbug.com/435317394): Add tests for the DynamicArrayKind. It is not possible to
    // create other DynamicArrayKind than SampledTexture at the moment so we cannot test error
    // cases.
}

// Test that defaulting fails if we go above maxBindingsPerBindGroup
TEST_F(DynamicBindingArrayTests, GetBGLMaxBindingsPerGroupLimit) {
    // Control case, we are just below the limit.
    {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.layout = nullptr;
        csDesc.compute.module = utils::CreateShaderModule(device, R"(
                enable chromium_experimental_dynamic_binding;
                @group(0) @binding()" + std::to_string(kMaxBindingsPerBindGroup - 1) +
                                                                      R"() var a : resource_binding;

                @compute @workgroup_size(1) fn main() {
                    _ = hasBinding<texture_2d<f32>>(a, 42);
                }
            )");
        device.CreateComputePipeline(&csDesc);
    }

    // Error case, we are above the limit.
    {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.layout = nullptr;
        csDesc.compute.module = utils::CreateShaderModule(device, R"(
                enable chromium_experimental_dynamic_binding;
                @group(0) @binding()" + std::to_string(kMaxBindingsPerBindGroup) +
                                                                      R"() var a : resource_binding;

                @compute @workgroup_size(1) fn main() {
                    _ = hasBinding<texture_2d<f32>>(a, 42);
                }
            )");
        ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&csDesc));
    }
}

// Test that defaulting fails if we go above maxBindGroups
TEST_F(DynamicBindingArrayTests, GetBGLMaxBindGroupsLimit) {
    // Control case, we are just below the limit.
    {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.layout = nullptr;
        csDesc.compute.module =
            utils::CreateShaderModule(device,
                                      R"(
                enable chromium_experimental_dynamic_binding;
                @group()" + std::to_string(kMaxBindGroups - 1) +
                                          R"() @binding(0) var a : resource_binding;

                @compute @workgroup_size(1) fn main() {
                    _ = hasBinding<texture_2d<f32>>(a, 42);
                }
            )");
        device.CreateComputePipeline(&csDesc);
    }

    // Error case, we are above the limit.
    {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.layout = nullptr;
        csDesc.compute.module =
            utils::CreateShaderModule(device,
                                      R"(
                enable chromium_experimental_dynamic_binding;
                @group()" + std::to_string(kMaxBindGroups) +
                                          R"() @binding(0) var a : resource_binding;

                @compute @workgroup_size(1) fn main() {
                    _ = hasBinding<texture_2d<f32>>(a, 42);
                }
            )");
        ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&csDesc));
    }
}

// Test that the dynamic binding arrays start must match between stages.
TEST_F(DynamicBindingArrayTests, GetBGLArrayStartMatchesBetweenStages) {
    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.layout = nullptr;
    pDesc.vertex.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_dynamic_binding;
        @group(0) @binding(38) var a : resource_binding;

        @vertex fn vs() -> @builtin(position) vec4f {
            _ = hasBinding<texture_2d<f32>>(a, 42);
            return vec4f();
        }
    )");

    // Success case: dynamic binding arrays match between the stages.
    {
        pDesc.cFragment.module = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_dynamic_binding;
            @group(0) @binding(38) var a : resource_binding;

            @fragment fn fs() -> @location(0) vec4f {
                _ = hasBinding<texture_2d<f32>>(a, 42);
                return vec4f();
            }
        )");
        device.CreateRenderPipeline(&pDesc);
    }

    // Success case: dynamic binding arrays have different starts on different groups.
    {
        pDesc.cFragment.module = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_dynamic_binding;
            @group(1) @binding(3) var a : resource_binding;

            @fragment fn fs() -> @location(0) vec4f {
                _ = hasBinding<texture_2d<f32>>(a, 42);
                return vec4f();
            }
        )");
        device.CreateRenderPipeline(&pDesc);
    }

    // Error case: dynamic binding arrays have different starts on the same group.
    {
        pDesc.cFragment.module = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_dynamic_binding;
            @group(0) @binding(3) var a : resource_binding;

            @fragment fn fs() -> @location(0) vec4f {
                _ = hasBinding<texture_2d<f32>>(a, 42);
                return vec4f();
            }
        )");
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&pDesc));
    }
}

// Test that defaulting the layout when no type information is given for the dynamic binding array
// is an error.
TEST_F(DynamicBindingArrayTests, DefaultedDynamicArrayKindMustNotBeUndefined) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_dynamic_binding;
        @group(0) @binding(0) var a : resource_binding;

        @compute @workgroup_size(1) fn main() {
            _ = arrayLength(a);
        }
    )");

    // Control case, explicitly giving a layout works
    wgpu::BindGroupLayout bgl = MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture);
    csDesc.layout = utils::MakeBasicPipelineLayout(device, &bgl);
    device.CreateComputePipeline(&csDesc);

    // Error case, defaulting the layout is not possible.
    csDesc.layout = nullptr;
    ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&csDesc));
}

// Test merging of DynamicBindingKind between VS and FS (FS typed case)
TEST_F(DynamicBindingArrayTests, MergingUnknowVSDynamicArrayKindInFS) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_dynamic_binding;

        @group(0) @binding(0) var untyped : resource_binding;
        @vertex fn vs() -> @builtin(position) vec4f {
            _ = arrayLength(untyped);
            return vec4f();
        }

        @group(0) @binding(0) var typed : resource_binding;
        @fragment fn fs() -> @location(0) vec4f {
            _ = hasBinding<texture_2d<f32>>(typed, 42);
            return vec4f();
        }
    )");

    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.layout = nullptr;
    pDesc.vertex.module = module;
    pDesc.cFragment.module = module;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

    // Check that this is a DynamicBindingKind::SampledTexture dynamic binding array.
    wgpu::TextureDescriptor tDesc = {
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);
    MakeBindGroup(pipeline.GetBindGroupLayout(0), 1, {{0, tex.CreateView()}});
}

// Test merging of DynamicBindingKind between VS and FS (VS typed case)
TEST_F(DynamicBindingArrayTests, MergingUnknowFSDynamicArrayKindInVS) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_dynamic_binding;

        @group(0) @binding(0) var typed : resource_binding;
        @vertex fn vs() -> @builtin(position) vec4f {
            _ = hasBinding<texture_2d<f32>>(typed, 42);
            return vec4f();
        }

        @group(0) @binding(0) var untyped : resource_binding;
        @fragment fn fs() -> @location(0) vec4f {
            _ = arrayLength(untyped);
            return vec4f();
        }
    )");

    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.layout = nullptr;
    pDesc.vertex.module = module;
    pDesc.cFragment.module = module;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

    // Check that this is a DynamicBindingKind::SampledTexture dynamic binding array.
    wgpu::TextureDescriptor tDesc = {
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);
    MakeBindGroup(pipeline.GetBindGroupLayout(0), 1, {{0, tex.CreateView()}});
}

// TODO(https://crbug.com/435317394): Add tests for the DynamicArrayKind defaulting / merging
// between stages. Tests to add are:
//  - Using two incompatible kinds between stages produces an error.

// Test that pinning / unpinning is valid for a simple case. This is a control for the test that
// errors are produced when the feature is not enabled.
TEST_F(DynamicBindingArrayTests, PinUnpinTextureSuccess) {
    wgpu::TextureDescriptor desc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&desc);

    tex.Pin(wgpu::TextureUsage::TextureBinding);
    tex.Unpin();
}

// Test that calling pin/unpin is an error when the feature is not enabled.
TEST_F(DynamicBindingArrayTests_FeatureDisabled, PinUnpinTextureSuccess) {
    wgpu::TextureDescriptor desc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&desc);

    ASSERT_DEVICE_ERROR(tex.Pin(wgpu::TextureUsage::TextureBinding));
    ASSERT_DEVICE_ERROR(tex.Unpin());
}

// Test the validation of the usage parameter of Pin.
TEST_F(DynamicBindingArrayTests, PinUnpinTextureUsageConstraint) {
    wgpu::TextureDescriptor desc{
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };

    desc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc |
                 wgpu::TextureUsage::StorageBinding;
    wgpu::Texture testTexture = device.CreateTexture(&desc);

    desc.usage = wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture renderOnlyTexture = device.CreateTexture(&desc);

    // Control case, pinning the sampled texture to TextureBinding is valid.
    testTexture.Pin(wgpu::TextureUsage::TextureBinding);

    // Error case, pinning to a usage not in the texture is invalid.
    ASSERT_DEVICE_ERROR(renderOnlyTexture.Pin(wgpu::TextureUsage::TextureBinding));

    // Error case, pinning to an invalid usage is invalid.
    ASSERT_DEVICE_ERROR(testTexture.Pin(static_cast<wgpu::TextureUsage>(0x8000'0000)));

    // Error case, pinning must be to a shader usage.
    ASSERT_DEVICE_ERROR(testTexture.Pin(wgpu::TextureUsage::CopySrc));

    // Error case, pinning must be to a shader usage.
    // TODO(https://crbug.com/435317394): Lift this constraint and allow other shader usages.
    ASSERT_DEVICE_ERROR(testTexture.Pin(wgpu::TextureUsage::StorageBinding));
}

// Test that pinning / unpinning don't need to be balanced.
TEST_F(DynamicBindingArrayTests, PinUnpinUnbalancedIsValid) {
    wgpu::TextureDescriptor desc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&desc);

    // Pinning right after creation is valid.
    tex.Unpin();

    // Pinning twice is valid.
    tex.Pin(wgpu::TextureUsage::TextureBinding);
    // TODO(https://crbug.com/435317394): Use a different usage here when another is valid.
    tex.Pin(wgpu::TextureUsage::TextureBinding);

    // Unpinning twice (plus one more to make sure we are unbalanced) is valid.
    tex.Unpin();
    tex.Unpin();
    tex.Unpin();
}

// Test that pinning is not allowed on a destroyed texture.
TEST_F(DynamicBindingArrayTests, PinDestroyedTextureInvalid) {
    wgpu::TextureDescriptor desc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&desc);

    // Success case, pinning before Destroy() is valid.
    tex.Pin(wgpu::TextureUsage::TextureBinding);
    tex.Unpin();

    // Error case, pinning a destroyed texture is not allowed.
    tex.Destroy();
    ASSERT_DEVICE_ERROR(tex.Pin(wgpu::TextureUsage::TextureBinding));
}

// TODO(https://crbug.com/435317394): This is missing the most important part of the pinned usage
// validation: at submit time (or writeTexture and friends) we must ensure that the usages don't
// conflict with the pinned usage. However adding this validation is a bit risky for the prototyping
// of bindless as it could cause perf regressions. Instead backends will ASSERT when possible that
// no barriers are emitted for the texture while it is pinned.

}  // anonymous namespace
}  // namespace dawn
