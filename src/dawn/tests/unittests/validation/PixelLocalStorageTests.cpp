// Copyright 2023 The Dawn Authors
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

#include "dawn/common/NonCopyable.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class PixelLocalStorageDisabledTest : public ValidationTest {};

// Check that creating a StorageAttachment texture is disallowed without the extension.
TEST_F(PixelLocalStorageDisabledTest, StorageAttachmentTextureNotAllowed) {
    wgpu::TextureDescriptor desc;
    desc.size = {1, 1, 1};
    desc.format = wgpu::TextureFormat::R32Uint;
    desc.usage = wgpu::TextureUsage::TextureBinding;

    // Control case: creating the texture without StorageAttachment is allowed.
    device.CreateTexture(&desc);

    // Error case: creating the texture without StorageAttachment is disallowed.
    desc.usage = wgpu::TextureUsage::StorageAttachment;
    ASSERT_DEVICE_ERROR(device.CreateTexture(&desc));
}

// Check that creating a pipeline layout with a PipelineLayoutPixelLocalStorage is disallowed
// without the extension.
TEST_F(PixelLocalStorageDisabledTest, PipelineLayoutPixelLocalStorageDisallowed) {
    wgpu::PipelineLayoutDescriptor desc;
    desc.bindGroupLayoutCount = 0;

    // Control case: creating the pipeline layout without the PLS is allowed.
    device.CreatePipelineLayout(&desc);

    // Error case: creating the pipeline layout with a PLS is disallowed even if it is empty.
    wgpu::PipelineLayoutPixelLocalStorage pls;
    pls.totalPixelLocalStorageSize = 0;
    pls.storageAttachmentCount = 0;
    desc.nextInChain = &pls;

    ASSERT_DEVICE_ERROR(device.CreatePipelineLayout(&desc));
}

// Check that a render pass with a RenderPassPixelLocalStorage is disallowed without the extension.
TEST_F(PixelLocalStorageDisabledTest, RenderPassPixelLocalStorageDisallowed) {
    utils::BasicRenderPass rp = utils::CreateBasicRenderPass(device, 1, 1);

    // Control case: beginning the render pass without the PLS is allowed.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        pass.End();
        encoder.Finish();
    }

    // Error case: beginning the render pass without the PLS is disallowed, even if it is empty.
    {
        wgpu::RenderPassPixelLocalStorage pls;
        pls.totalPixelLocalStorageSize = 0;
        pls.storageAttachmentCount = 0;
        rp.renderPassInfo.nextInChain = &pls;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Check that PixelLocalStorageBarrier() is disallowed without the extension.
TEST_F(PixelLocalStorageDisabledTest, PixelLocalStorageBarrierDisallowed) {
    utils::BasicRenderPass rp = utils::CreateBasicRenderPass(device, 1, 1);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
    pass.PixelLocalStorageBarrier();
    pass.End();
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

class PixelLocalStorageOtherExtensionTest : public ValidationTest {
  protected:
    WGPUDevice CreateTestDevice(native::Adapter dawnAdapter,
                                wgpu::DeviceDescriptor descriptor) override {
        // Only test the coherent extension. The non-coherent one has the rest of the validation
        // tests.
        wgpu::FeatureName requiredFeatures[1] = {wgpu::FeatureName::PixelLocalStorageCoherent};
        descriptor.requiredFeatures = requiredFeatures;
        descriptor.requiredFeatureCount = 1;
        return dawnAdapter.CreateDevice(&descriptor);
    }
};

// Simple test checking all the various things that are normally validated out without PLS are
// available if the coherent PLS extension is enabled.
TEST_F(PixelLocalStorageOtherExtensionTest, SmokeTest) {
    // Creating a StorageAttachment texture is allowed.
    wgpu::TextureDescriptor textureDesc;
    textureDesc.size = {1, 1, 1};
    textureDesc.format = wgpu::TextureFormat::R32Uint;
    textureDesc.usage = wgpu::TextureUsage::StorageAttachment;
    wgpu::Texture tex = device.CreateTexture(&textureDesc);

    // Creating a pipeline layout with PLS is allowed.
    wgpu::PipelineLayoutPixelLocalStorage plPlsDesc;
    plPlsDesc.totalPixelLocalStorageSize = 0;
    plPlsDesc.storageAttachmentCount = 0;

    wgpu::PipelineLayoutDescriptor plDesc;
    plDesc.bindGroupLayoutCount = 0;
    plDesc.nextInChain = &plPlsDesc;
    device.CreatePipelineLayout(&plDesc);

    // Creating a PLS render pass is allowed.
    wgpu::RenderPassPixelLocalStorage rpPlsDesc;
    rpPlsDesc.totalPixelLocalStorageSize = 0;
    rpPlsDesc.storageAttachmentCount = 0;

    utils::BasicRenderPass rp = utils::CreateBasicRenderPass(device, 1, 1);
    rp.renderPassInfo.nextInChain = &rpPlsDesc;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
    // Calling PixelLocalStorageBarrier is allowed.
    pass.PixelLocalStorageBarrier();
    pass.End();
    encoder.Finish();
}

struct OffsetAndFormat {
    uint64_t offset;
    wgpu::TextureFormat format;
};
struct PLSSpec {
    uint64_t totalSize;
    std::vector<OffsetAndFormat> attachments;
    bool active = true;
};

constexpr std::array<wgpu::TextureFormat, 3> kStorageAttachmentFormats = {
    wgpu::TextureFormat::R32Float,
    wgpu::TextureFormat::R32Uint,
    wgpu::TextureFormat::R32Sint,
};
bool IsStorageAttachmentFormat(wgpu::TextureFormat format) {
    return std::find(kStorageAttachmentFormats.begin(), kStorageAttachmentFormats.end(), format) !=
           kStorageAttachmentFormats.end();
}

struct ComboTestPLSRenderPassDescriptor : NonMovable {
    std::array<wgpu::RenderPassStorageAttachment, 8> storageAttachments;
    wgpu::RenderPassPixelLocalStorage pls;
    wgpu::RenderPassColorAttachment colorAttachment;
    wgpu::RenderPassDescriptor rpDesc;
};

class PixelLocalStorageTest : public ValidationTest {
  protected:
    WGPUDevice CreateTestDevice(native::Adapter dawnAdapter,
                                wgpu::DeviceDescriptor descriptor) override {
        // Test only the non-coherent version, and assume that the same validation code paths are
        // taken for the coherent path.
        wgpu::FeatureName requiredFeatures[1] = {wgpu::FeatureName::PixelLocalStorageNonCoherent};
        descriptor.requiredFeatures = requiredFeatures;
        descriptor.requiredFeatureCount = 1;
        return dawnAdapter.CreateDevice(&descriptor);
    }

    void InitializePLSRenderPass(ComboTestPLSRenderPassDescriptor* desc) {
        // Set up a single storage attachment.
        wgpu::TextureDescriptor storageDesc;
        storageDesc.size = {1, 1};
        storageDesc.format = wgpu::TextureFormat::R32Uint;
        storageDesc.usage = wgpu::TextureUsage::StorageAttachment;
        wgpu::Texture storage = device.CreateTexture(&storageDesc);

        desc->storageAttachments[0].storage = storage.CreateView();
        desc->storageAttachments[0].offset = 0;
        desc->storageAttachments[0].loadOp = wgpu::LoadOp::Load;
        desc->storageAttachments[0].storeOp = wgpu::StoreOp::Store;

        desc->pls.totalPixelLocalStorageSize = 4;
        desc->pls.storageAttachmentCount = 1;
        desc->pls.storageAttachments = desc->storageAttachments.data();

        // Add at least one color attachment to make the render pass valid if there's no storage
        // attachment.
        wgpu::TextureDescriptor colorDesc;
        colorDesc.size = {1, 1};
        colorDesc.format = kColorAttachmentFormat;
        colorDesc.usage = wgpu::TextureUsage::RenderAttachment;
        wgpu::Texture color = device.CreateTexture(&colorDesc);

        desc->colorAttachment.view = color.CreateView();
        desc->colorAttachment.loadOp = wgpu::LoadOp::Load;
        desc->colorAttachment.storeOp = wgpu::StoreOp::Store;

        desc->rpDesc.nextInChain = &desc->pls;
        desc->rpDesc.colorAttachmentCount = 1;
        desc->rpDesc.colorAttachments = &desc->colorAttachment;
    }

    void RecordRenderPass(const wgpu::RenderPassDescriptor* desc,
                          wgpu::RenderPipeline pipeline = {}) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(desc);

        if (pipeline) {
            pass.SetPipeline(pipeline);
        }

        pass.End();
        encoder.Finish();
    }

    void RecordPLSRenderPass(const PLSSpec& spec, wgpu::RenderPipeline pipeline = {}) {
        ComboTestPLSRenderPassDescriptor desc;
        InitializePLSRenderPass(&desc);

        // Convert the PLSSpec to a RenderPassPLS
        for (size_t i = 0; i < spec.attachments.size(); i++) {
            wgpu::TextureDescriptor tDesc;
            tDesc.size = {1, 1};
            tDesc.format = spec.attachments[i].format;
            tDesc.usage = wgpu::TextureUsage::StorageAttachment;
            wgpu::Texture texture = device.CreateTexture(&tDesc);

            desc.storageAttachments[i].storage = texture.CreateView();
            desc.storageAttachments[i].offset = spec.attachments[i].offset;
            desc.storageAttachments[i].loadOp = wgpu::LoadOp::Load;
            desc.storageAttachments[i].storeOp = wgpu::StoreOp::Store;
        }

        desc.pls.totalPixelLocalStorageSize = spec.totalSize;
        desc.pls.storageAttachmentCount = spec.attachments.size();

        // Add the PLS if needed and record the render pass.
        if (!spec.active) {
            desc.rpDesc.nextInChain = nullptr;
        }

        RecordRenderPass(&desc.rpDesc, pipeline);
    }

    wgpu::PipelineLayout MakePipelineLayout(const PLSSpec& spec) {
        // Convert the PLSSpec to a PipelineLayoutPLS
        std::vector<wgpu::PipelineLayoutStorageAttachment> storageAttachments;
        for (const auto& attachmentSpec : spec.attachments) {
            wgpu::PipelineLayoutStorageAttachment attachment;
            attachment.format = attachmentSpec.format;
            attachment.offset = attachmentSpec.offset;
            storageAttachments.push_back(attachment);
        }

        wgpu::PipelineLayoutPixelLocalStorage pls;
        pls.totalPixelLocalStorageSize = spec.totalSize;
        pls.storageAttachmentCount = storageAttachments.size();
        pls.storageAttachments = storageAttachments.data();

        // Add the PLS if needed and make the pipeline layout.
        wgpu::PipelineLayoutDescriptor plDesc;
        plDesc.bindGroupLayoutCount = 0;
        if (spec.active) {
            plDesc.nextInChain = &pls;
        }
        return device.CreatePipelineLayout(&plDesc);
    }

    wgpu::RenderPipeline MakePipeline(const PLSSpec& spec) {
        utils::ComboRenderPipelineDescriptor desc;
        wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            @vertex fn vs() -> @builtin(position) vec4f {
                return vec4f();
            }
            @fragment fn fs() -> @location(0) u32 {
                return 0u;
            }
        )");

        desc.layout = MakePipelineLayout(spec);
        desc.cFragment.module = module;
        desc.cFragment.entryPoint = "fs";
        desc.vertex.module = module;
        desc.vertex.entryPoint = "vs";
        desc.cTargets[0].format = kColorAttachmentFormat;
        return device.CreateRenderPipeline(&desc);
    }

    void CheckPLSStateMatching(const PLSSpec& rpSpec, const PLSSpec& pipelineSpec, bool success) {
        wgpu::RenderPipeline pipeline = MakePipeline(pipelineSpec);

        if (success) {
            RecordPLSRenderPass(rpSpec, pipeline);
        } else {
            ASSERT_DEVICE_ERROR(RecordPLSRenderPass(rpSpec, pipeline));
        }
    }

    static constexpr wgpu::TextureFormat kColorAttachmentFormat = wgpu::TextureFormat::R32Uint;
};

// Check that StorageAttachment textures must be one of the supported formats.
TEST_F(PixelLocalStorageTest, TextureFormatMustSupportStorageAttachment) {
    for (wgpu::TextureFormat format : utils::kAllTextureFormats) {
        wgpu::TextureDescriptor desc;
        desc.size = {1, 1};
        desc.format = format;
        desc.usage = wgpu::TextureUsage::StorageAttachment;

        if (IsStorageAttachmentFormat(format)) {
            device.CreateTexture(&desc);
        } else {
            ASSERT_DEVICE_ERROR(device.CreateTexture(&desc));
        }
    }
}

// Check that StorageAttachment textures must have a sample count of 1.
TEST_F(PixelLocalStorageTest, TextureMustBeSingleSampled) {
    wgpu::TextureDescriptor desc;
    desc.size = {1, 1};
    desc.format = wgpu::TextureFormat::R32Uint;
    desc.usage = wgpu::TextureUsage::StorageAttachment;

    // Control case: sampleCount = 1 is valid.
    desc.sampleCount = 1;
    device.CreateTexture(&desc);

    // Error case: sampledCount != 1 is an error.
    desc.sampleCount = 4;
    ASSERT_DEVICE_ERROR(device.CreateTexture(&desc));
}

// Check that the format in PLS must be one of the enabled ones.
TEST_F(PixelLocalStorageTest, PLSStateFormatMustSupportStorageAttachment) {
    for (wgpu::TextureFormat format : utils::kFormatsInCoreSpec) {
        PLSSpec spec = {4, {{0, format}}};

        // Note that BeginRenderPass is not tested here as a different test checks that the
        // StorageAttachment texture must indeed have been created with the StorageAttachment usage.
        if (IsStorageAttachmentFormat(format)) {
            MakePipelineLayout(spec);
        } else {
            ASSERT_DEVICE_ERROR(MakePipelineLayout(spec));
        }
    }
}

// Check that the total size must be a multiple of 4.
TEST_F(PixelLocalStorageTest, PLSStateTotalSizeMultipleOf4) {
    // Control case: total size is a multiple of 4.
    {
        PLSSpec spec = {4, {}};
        MakePipelineLayout(spec);
        RecordPLSRenderPass(spec);
    }

    // Control case: total size isn't a multiple of 4.
    {
        PLSSpec spec = {2, {}};
        ASSERT_DEVICE_ERROR(MakePipelineLayout(spec));
        ASSERT_DEVICE_ERROR(RecordPLSRenderPass(spec));
    }
}

// Check that the total size must be less than 16.
// TODO(dawn:1704): Have a proper limit for totalSize.
TEST_F(PixelLocalStorageTest, PLSStateTotalLessThan16) {
    // Control case: total size is 16.
    {
        PLSSpec spec = {16, {}};
        MakePipelineLayout(spec);
        RecordPLSRenderPass(spec);
    }

    // Control case: total size is greater than 16.
    {
        PLSSpec spec = {20, {}};
        ASSERT_DEVICE_ERROR(MakePipelineLayout(spec));
        ASSERT_DEVICE_ERROR(RecordPLSRenderPass(spec));
    }
}

// Check that the offset of a storage attachment must be a multiple of 4.
TEST_F(PixelLocalStorageTest, PLSStateOffsetMultipleOf4) {
    // Control case: offset is a multiple of 4.
    {
        PLSSpec spec = {8, {{4, wgpu::TextureFormat::R32Uint}}};
        MakePipelineLayout(spec);
        RecordPLSRenderPass(spec);
    }

    // Error case: offset isn't a multiple of 4.
    {
        PLSSpec spec = {8, {{2, wgpu::TextureFormat::R32Uint}}};
        ASSERT_DEVICE_ERROR(MakePipelineLayout(spec));
        ASSERT_DEVICE_ERROR(RecordPLSRenderPass(spec));
    }
}

// Check that the storage attachment is in bounds of the total size.
TEST_F(PixelLocalStorageTest, PLSStateAttachmentInBoundsOfTotalSize) {
    // Note that all storage attachment formats are currently 4 byte wide.

    // Control case: 0 + 4 <= 4
    {
        PLSSpec spec = {4, {{0, wgpu::TextureFormat::R32Uint}}};
        MakePipelineLayout(spec);
        RecordPLSRenderPass(spec);
    }

    // Error case: 4 + 4 > 4
    {
        PLSSpec spec = {4, {{4, wgpu::TextureFormat::R32Uint}}};
        ASSERT_DEVICE_ERROR(MakePipelineLayout(spec));
        ASSERT_DEVICE_ERROR(RecordPLSRenderPass(spec));
    }

    // Control case: 8 + 4 <= 12
    {
        PLSSpec spec = {12, {{8, wgpu::TextureFormat::R32Uint}}};
        MakePipelineLayout(spec);
        RecordPLSRenderPass(spec);
    }

    // Error case: 12 + 4 > 12
    {
        PLSSpec spec = {4, {{12, wgpu::TextureFormat::R32Uint}}};
        ASSERT_DEVICE_ERROR(MakePipelineLayout(spec));
        ASSERT_DEVICE_ERROR(RecordPLSRenderPass(spec));
    }

    // Check that overflows don't incorrectly pass the validation.
    {
        PLSSpec spec = {4, {{uint64_t(0) - uint64_t(4), wgpu::TextureFormat::R32Uint}}};
        ASSERT_DEVICE_ERROR(MakePipelineLayout(spec));
        ASSERT_DEVICE_ERROR(RecordPLSRenderPass(spec));
    }
}

// Check that collisions between storage attachments are not allowed.
TEST_F(PixelLocalStorageTest, PLSStateCollisionsDisallowed) {
    // Control case: no collisions, all is good!
    {
        PLSSpec spec = {8, {{0, wgpu::TextureFormat::R32Uint}, {4, wgpu::TextureFormat::R32Uint}}};
        MakePipelineLayout(spec);
        RecordPLSRenderPass(spec);
    }
    // Error case: collisions, boo!
    {
        PLSSpec spec = {8, {{0, wgpu::TextureFormat::R32Uint}, {0, wgpu::TextureFormat::R32Uint}}};
        ASSERT_DEVICE_ERROR(MakePipelineLayout(spec));
        ASSERT_DEVICE_ERROR(RecordPLSRenderPass(spec));
    }
    {
        PLSSpec spec = {8,
                        {{0, wgpu::TextureFormat::R32Uint},
                         {4, wgpu::TextureFormat::R32Uint},
                         {0, wgpu::TextureFormat::R32Uint}}};
        ASSERT_DEVICE_ERROR(MakePipelineLayout(spec));
        ASSERT_DEVICE_ERROR(RecordPLSRenderPass(spec));
    }
}

// Check that using an error view as storage attachment is an error.
TEST_F(PixelLocalStorageTest, RenderPassStorageAttachmentErrorView) {
    ComboTestPLSRenderPassDescriptor desc;
    InitializePLSRenderPass(&desc);

    wgpu::TextureDescriptor tDesc;
    tDesc.size = {1, 1};
    tDesc.usage = wgpu::TextureUsage::StorageAttachment;
    tDesc.format = wgpu::TextureFormat::R32Uint;
    wgpu::Texture t = device.CreateTexture(&tDesc);

    wgpu::TextureViewDescriptor viewDesc;

    // Control case: valid texture view.
    desc.storageAttachments[0].storage = t.CreateView(&viewDesc);
    RecordRenderPass(&desc.rpDesc);

    // Error case: invalid texture view because of the base array layer.
    viewDesc.baseArrayLayer = 10;
    ASSERT_DEVICE_ERROR(desc.storageAttachments[0].storage = t.CreateView(&viewDesc));
    ASSERT_DEVICE_ERROR(RecordRenderPass(&desc.rpDesc));
}

// Check that using a multi-subresource view as a storage attachment is an error (layers and levels
// cases).
TEST_F(PixelLocalStorageTest, RenderPassStorageAttachmentSingleSubresource) {
    ComboTestPLSRenderPassDescriptor desc;
    InitializePLSRenderPass(&desc);

    wgpu::TextureDescriptor colorDesc;
    colorDesc.size = {2, 2};
    colorDesc.usage = wgpu::TextureUsage::RenderAttachment;
    colorDesc.format = kColorAttachmentFormat;

    // Replace the render pass attachment with a 2x2 texture for mip level testing.
    desc.colorAttachment.view = device.CreateTexture(&colorDesc).CreateView();

    // Control case: single subresource view.
    wgpu::TextureDescriptor tDesc;
    tDesc.size = {2, 2};
    tDesc.usage = wgpu::TextureUsage::StorageAttachment;
    tDesc.format = wgpu::TextureFormat::R32Uint;

    desc.storageAttachments[0].storage = device.CreateTexture(&tDesc).CreateView();
    RecordRenderPass(&desc.rpDesc);

    // Error case: two array layers.
    tDesc.size.depthOrArrayLayers = 2;
    desc.storageAttachments[0].storage = device.CreateTexture(&tDesc).CreateView();
    ASSERT_DEVICE_ERROR(RecordRenderPass(&desc.rpDesc));

    // Error case: two mip levels.
    tDesc.size.depthOrArrayLayers = 1;
    tDesc.mipLevelCount = 2;
    desc.storageAttachments[0].storage = device.CreateTexture(&tDesc).CreateView();
    ASSERT_DEVICE_ERROR(RecordRenderPass(&desc.rpDesc));
}

// Check that the size of storage attachments must match the size of other attachments.
TEST_F(PixelLocalStorageTest, RenderPassStorageAttachmentSizeMustMatch) {
    ComboTestPLSRenderPassDescriptor desc;
    InitializePLSRenderPass(&desc);

    // Explicitly set the color attachment to a 1x1 texture.
    wgpu::TextureDescriptor colorDesc;
    colorDesc.size = {1, 1};
    colorDesc.usage = wgpu::TextureUsage::RenderAttachment;
    colorDesc.format = kColorAttachmentFormat;
    desc.colorAttachment.view = device.CreateTexture(&colorDesc).CreateView();

    // Control case: the storage attachment size matches
    wgpu::TextureDescriptor tDesc;
    tDesc.size = {1, 1};
    tDesc.usage = wgpu::TextureUsage::StorageAttachment;
    tDesc.format = wgpu::TextureFormat::R32Uint;

    desc.storageAttachments[0].storage = device.CreateTexture(&tDesc).CreateView();
    RecordRenderPass(&desc.rpDesc);

    // Error case: width doesn't match.
    tDesc.size = {2, 1};
    desc.storageAttachments[0].storage = device.CreateTexture(&tDesc).CreateView();
    ASSERT_DEVICE_ERROR(RecordRenderPass(&desc.rpDesc));

    // Error case: height doesn't match.
    tDesc.size = {1, 2};
    desc.storageAttachments[0].storage = device.CreateTexture(&tDesc).CreateView();
    ASSERT_DEVICE_ERROR(RecordRenderPass(&desc.rpDesc));
}

// Check that the textures used as storage attachment must have the StorageAttachment TextureUsage.
TEST_F(PixelLocalStorageTest, RenderPassStorageAttachmentUsage) {
    ComboTestPLSRenderPassDescriptor desc;
    InitializePLSRenderPass(&desc);

    // Control case: the storage attachment has the correct usage.
    wgpu::TextureDescriptor tDesc;
    tDesc.size = {1, 1};
    tDesc.usage = wgpu::TextureUsage::StorageAttachment;
    tDesc.format = wgpu::TextureFormat::R32Uint;

    desc.storageAttachments[0].storage = device.CreateTexture(&tDesc).CreateView();
    RecordRenderPass(&desc.rpDesc);

    // Error case: the storage attachment doesn't have the usage.
    tDesc.usage = wgpu::TextureUsage::RenderAttachment;
    desc.storageAttachments[0].storage = device.CreateTexture(&tDesc).CreateView();
    ASSERT_DEVICE_ERROR(RecordRenderPass(&desc.rpDesc));
}

// Check that the same texture subresource cannot be used twice as a storage attachment.
TEST_F(PixelLocalStorageTest, RenderPassSubresourceUsedTwiceAsStorage) {
    ComboTestPLSRenderPassDescriptor desc;
    InitializePLSRenderPass(&desc);

    // Control case: two different subresources for two storage attachments.
    wgpu::TextureDescriptor tDesc;
    tDesc.size = {1, 1};
    tDesc.usage = wgpu::TextureUsage::StorageAttachment;
    tDesc.format = wgpu::TextureFormat::R32Uint;

    desc.storageAttachments[0].storage = device.CreateTexture(&tDesc).CreateView();

    desc.storageAttachments[1].storage = device.CreateTexture(&tDesc).CreateView();
    desc.storageAttachments[1].offset = 4;
    desc.storageAttachments[1].loadOp = wgpu::LoadOp::Load;
    desc.storageAttachments[1].storeOp = wgpu::StoreOp::Store;
    desc.pls.storageAttachmentCount = 2;
    desc.pls.totalPixelLocalStorageSize = 8;

    RecordRenderPass(&desc.rpDesc);

    // Error case: the same subresource is used twice as a storage attachment.
    desc.storageAttachments[0].storage = desc.storageAttachments[1].storage;
    ASSERT_DEVICE_ERROR(RecordRenderPass(&desc.rpDesc));
}

// Check that the same texture subresource cannot be used twice as a storage and render attachment.
TEST_F(PixelLocalStorageTest, RenderPassSubresourceUsedAsStorageAndRender) {
    ComboTestPLSRenderPassDescriptor desc;
    InitializePLSRenderPass(&desc);

    // Control case: two different subresources for storage and render attachments.
    wgpu::TextureDescriptor tDesc;
    tDesc.size = {1, 1};
    tDesc.usage = wgpu::TextureUsage::StorageAttachment | wgpu::TextureUsage::RenderAttachment;
    tDesc.format = kColorAttachmentFormat;

    desc.storageAttachments[0].storage = device.CreateTexture(&tDesc).CreateView();
    RecordRenderPass(&desc.rpDesc);

    // Error case: the same view is used twice, once as storage, once as render attachment.
    desc.colorAttachment.view = desc.storageAttachments[0].storage;
    ASSERT_DEVICE_ERROR(RecordRenderPass(&desc.rpDesc));
}

// Check that using a subresource as storage attachment prevents other usages in the render pass.
TEST_F(PixelLocalStorageTest, RenderPassSubresourceUsedInsidePass) {
    ComboTestPLSRenderPassDescriptor desc;
    InitializePLSRenderPass(&desc);

    wgpu::TextureDescriptor tDesc;
    tDesc.size = {1, 1};
    tDesc.usage = wgpu::TextureUsage::StorageAttachment | wgpu::TextureUsage::TextureBinding;
    tDesc.format = wgpu::TextureFormat::R32Uint;

    desc.storageAttachments[0].storage = device.CreateTexture(&tDesc).CreateView();

    // Control case: the storage attachment is used only as storage attachment.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&desc.rpDesc);
        pass.End();
        encoder.Finish();
    }

    // Error case: the storage attachment is also used as a texture binding in a bind group.
    {
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Uint}});
        wgpu::BindGroup bg =
            utils::MakeBindGroup(device, bgl, {{0, desc.storageAttachments[0].storage}});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&desc.rpDesc);
        pass.SetBindGroup(0, bg);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Check that the load and store op must not be undefined.
TEST_F(PixelLocalStorageTest, RenderPassLoadAndStoreOpNotUndefined) {
    ComboTestPLSRenderPassDescriptor desc;
    InitializePLSRenderPass(&desc);

    // Control case: ops are not undefined.
    RecordRenderPass(&desc.rpDesc);

    // Error case: LoadOp::Undefined
    desc.storageAttachments[0].loadOp = wgpu::LoadOp::Undefined;
    ASSERT_DEVICE_ERROR(RecordRenderPass(&desc.rpDesc));
    desc.storageAttachments[0].loadOp = wgpu::LoadOp::Load;

    // Error case: StoreOp::Undefined
    desc.storageAttachments[0].storeOp = wgpu::StoreOp::Undefined;
    ASSERT_DEVICE_ERROR(RecordRenderPass(&desc.rpDesc));
}

// Check that the clear value, if used, must not have NaNs.
TEST_F(PixelLocalStorageTest, RenderPassClearValueNaNs) {
    ComboTestPLSRenderPassDescriptor desc;
    InitializePLSRenderPass(&desc);

    const float kNaN = std::nan("");

    // Check that NaNs are allowed if the loadOp is not Clear.
    desc.storageAttachments[0].loadOp = wgpu::LoadOp::Load;
    desc.storageAttachments[0].clearValue = {kNaN, kNaN, kNaN, kNaN};
    RecordRenderPass(&desc.rpDesc);

    // Control case, a non-NaN clear value is allowed.
    desc.storageAttachments[0].loadOp = wgpu::LoadOp::Clear;
    desc.storageAttachments[0].clearValue = {0, 0, 0, 0};
    RecordRenderPass(&desc.rpDesc);

    // Error case: NaN in one of the components of clearValue.
    desc.storageAttachments[0].clearValue = {kNaN, 0, 0, 0};
    ASSERT_DEVICE_ERROR(RecordRenderPass(&desc.rpDesc));

    desc.storageAttachments[0].clearValue = {0, kNaN, 0, 0};
    ASSERT_DEVICE_ERROR(RecordRenderPass(&desc.rpDesc));

    desc.storageAttachments[0].clearValue = {0, 0, kNaN, 0};
    ASSERT_DEVICE_ERROR(RecordRenderPass(&desc.rpDesc));

    desc.storageAttachments[0].clearValue = {0, 0, 0, kNaN};
    ASSERT_DEVICE_ERROR(RecordRenderPass(&desc.rpDesc));
}

// Check that using a subresource as storage attachment prevents other usages in the render pass.
TEST_F(PixelLocalStorageTest, PixelLocalStorageBarrierRequiresPLS) {
    ComboTestPLSRenderPassDescriptor desc;
    InitializePLSRenderPass(&desc);

    // Control case: there is a PLS, the barrier is allowed.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&desc.rpDesc);
        pass.PixelLocalStorageBarrier();
        pass.End();
        encoder.Finish();
    }

    // Error case: there is no PLS (it is unlinked from chained structs), the barrier is not
    // allowed.
    {
        desc.rpDesc.nextInChain = nullptr;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&desc.rpDesc);
        pass.PixelLocalStorageBarrier();
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Check that PLS state differs between no PLS and empty PLS
TEST_F(PixelLocalStorageTest, PLSStateMatching_EmptyPLSVsNoPLS) {
    PLSSpec emptyPLS = {0, {}, true};
    PLSSpec noPLS = {0, {}, false};

    CheckPLSStateMatching(emptyPLS, emptyPLS, true);
    CheckPLSStateMatching(noPLS, noPLS, true);
    CheckPLSStateMatching(emptyPLS, noPLS, false);
    CheckPLSStateMatching(noPLS, emptyPLS, false);
}

// Check that PLS state differs between empty PLS and non-empty PLS with no storage attachments
TEST_F(PixelLocalStorageTest, PLSStateMatching_EmptyPLSVsNotEmpty) {
    PLSSpec emptyPLS = {0, {}};
    PLSSpec notEmptyPLS = {4, {}};

    CheckPLSStateMatching(emptyPLS, emptyPLS, true);
    CheckPLSStateMatching(notEmptyPLS, notEmptyPLS, true);
    CheckPLSStateMatching(emptyPLS, notEmptyPLS, false);
    CheckPLSStateMatching(notEmptyPLS, emptyPLS, false);
}

// Check that PLS state differs between implicit PLS vs storage attachment
TEST_F(PixelLocalStorageTest, PLSStateMatching_AttachmentVsImplicit) {
    PLSSpec implicitPLS = {4, {}};
    PLSSpec storagePLS = {4, {{0, wgpu::TextureFormat::R32Uint}}};

    CheckPLSStateMatching(implicitPLS, implicitPLS, true);
    CheckPLSStateMatching(storagePLS, storagePLS, true);
    CheckPLSStateMatching(implicitPLS, storagePLS, false);
    CheckPLSStateMatching(storagePLS, implicitPLS, false);
}

// Check that PLS state differs between storage attachment formats
TEST_F(PixelLocalStorageTest, PLSStateMatching_Format) {
    PLSSpec intPLS = {4, {{0, wgpu::TextureFormat::R32Sint}}};
    PLSSpec uintPLS = {4, {{0, wgpu::TextureFormat::R32Uint}}};

    CheckPLSStateMatching(intPLS, intPLS, true);
    CheckPLSStateMatching(uintPLS, uintPLS, true);
    CheckPLSStateMatching(intPLS, uintPLS, false);
    CheckPLSStateMatching(uintPLS, intPLS, false);
}

// Check that PLS state are equal even if attachments are specified in different orders
TEST_F(PixelLocalStorageTest, PLSStateMatching_StorageAttachmentOrder) {
    PLSSpec pls1 = {8, {{4, wgpu::TextureFormat::R32Uint}, {0, wgpu::TextureFormat::R32Sint}}};
    PLSSpec pls2 = {8, {{0, wgpu::TextureFormat::R32Sint}, {4, wgpu::TextureFormat::R32Uint}}};

    CheckPLSStateMatching(pls1, pls2, true);
}

class PixelLocalStorageAndRenderToSingleSampledTest : public PixelLocalStorageTest {
  protected:
    WGPUDevice CreateTestDevice(native::Adapter dawnAdapter,
                                wgpu::DeviceDescriptor descriptor) override {
        // TODO(dawn:1704): Do we need to test both extensions?
        wgpu::FeatureName requiredFeatures[2] = {wgpu::FeatureName::PixelLocalStorageNonCoherent,
                                                 wgpu::FeatureName::MSAARenderToSingleSampled};
        descriptor.requiredFeatures = requiredFeatures;
        descriptor.requiredFeatureCount = 2;
        return dawnAdapter.CreateDevice(&descriptor);
    }
};

// Check that PLS + MSAA render to single sampled is not allowed
TEST_F(PixelLocalStorageAndRenderToSingleSampledTest, CombinationIsNotAllowed) {
    ComboTestPLSRenderPassDescriptor desc;
    InitializePLSRenderPass(&desc);

    // Control case: no MSAA render to single sampled.
    RecordRenderPass(&desc.rpDesc);

    // Error case: MSAA render to single sampled is added to the color attachment.
    wgpu::DawnRenderPassColorAttachmentRenderToSingleSampled msaaRenderToSingleSampledDesc;
    msaaRenderToSingleSampledDesc.implicitSampleCount = 4;
    desc.colorAttachment.nextInChain = &msaaRenderToSingleSampledDesc;
    ASSERT_DEVICE_ERROR(RecordRenderPass(&desc.rpDesc));
}

// TODO(dawn:1704): Add tests for limits

}  // anonymous namespace
}  // namespace dawn
