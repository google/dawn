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

#include "dawn/tests/unittests/validation/ValidationTest.h"

#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class PixelLocalStorageDisabledTest : public ValidationTest {};

// Check that creating a StorageAttachment texture is disallowed without the extension.
TEST_F(PixelLocalStorageDisabledTest, StorageAttachmentTextureNotAllowed) {
    wgpu::TextureDescriptor desc;
    desc.size = {1, 1, 1};
    desc.format = wgpu::TextureFormat::RGBA8Unorm;
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

}  // anonymous namespace
}  // namespace dawn
