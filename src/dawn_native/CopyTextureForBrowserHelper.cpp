// Copyright 2020 The Dawn Authors
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

#include "dawn_native/CopyTextureForBrowserHelper.h"

#include "dawn_native/BindGroup.h"
#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/Buffer.h"
#include "dawn_native/CommandBuffer.h"
#include "dawn_native/CommandEncoder.h"
#include "dawn_native/CommandValidation.h"
#include "dawn_native/Device.h"
#include "dawn_native/InternalPipelineStore.h"
#include "dawn_native/Queue.h"
#include "dawn_native/RenderPassEncoder.h"
#include "dawn_native/RenderPipeline.h"
#include "dawn_native/Sampler.h"
#include "dawn_native/Texture.h"

#include <unordered_set>

namespace dawn_native {
    namespace {
        // TODO(shaobo.yan@intel.com) : Support premultiplay-alpha, flipY.
        static const char sCopyTextureForBrowserVertex[] = R"(
            [[block]] struct Uniforms {
                [[offset(0)]] u_scale : vec2<f32>;
                [[offset(8)]] u_offset : vec2<f32>;
            };
            const texcoord : array<vec2<f32>, 3> = array<vec2<f32>, 3>(
                vec2<f32>(-0.5, 0.0),
                vec2<f32>( 1.5, 0.0),
                vec2<f32>( 0.5, 2.0));
            [[location(0)]] var<out> v_texcoord: vec2<f32>;
            [[builtin(position)]] var<out> Position : vec4<f32>;
            [[builtin(vertex_idx)]] var<in> VertexIndex : u32;
            [[binding(0), set(0)]] var<uniform> uniforms : Uniforms;
            [[stage(vertex)]] fn main() -> void {
                Position = vec4<f32>((texcoord[VertexIndex] * 2.0 - vec2<f32>(1.0, 1.0)), 0.0, 1.0);

                // Texture coordinate takes top-left as origin point. We need to map the
                // texture to triangle carefully.
                v_texcoord = (texcoord[VertexIndex] * vec2<f32>(1.0, -1.0) + vec2<f32>(0.0, 1.0)) *
                    uniforms.u_scale + uniforms.u_offset;
            }
        )";

        static const char sPassthrough2D4ChannelFrag[] = R"(
            [[binding(1), set(0)]] var<uniform_constant> mySampler: sampler;
            [[binding(2), set(0)]] var<uniform_constant> myTexture: texture_sampled_2d<f32>;
            [[location(0)]] var<in> v_texcoord : vec2<f32>;
            [[location(0)]] var<out> rgbaColor : vec4<f32>;
            [[stage(fragment)]] fn main() -> void {
                // Clamp the texcoord and discard the out-of-bound pixels.
                var clampedTexcoord : vec2<f32> =
                    clamp(v_texcoord, vec2<f32>(0.0, 0.0), vec2<f32>(1.0, 1.0));
                if (all(clampedTexcoord == v_texcoord)) {
                    rgbaColor = textureSample(myTexture, mySampler, v_texcoord);
                }
            }
        )";

        // TODO(shaobo.yan@intel.com): Expand supported texture formats
        MaybeError ValidateCopyTextureFormatConversion(const wgpu::TextureFormat srcFormat,
                                                       const wgpu::TextureFormat dstFormat) {
            switch (srcFormat) {
                case wgpu::TextureFormat::RGBA8Unorm:
                    break;
                default:
                    return DAWN_VALIDATION_ERROR(
                        "Unsupported src texture format for CopyTextureForBrowser.");
            }

            switch (dstFormat) {
                case wgpu::TextureFormat::RGBA8Unorm:
                    break;
                default:
                    return DAWN_VALIDATION_ERROR(
                        "Unsupported dst texture format for CopyTextureForBrowser.");
            }

            return {};
        }

        MaybeError ValidateCopyTextureForBrowserOptions(
            const CopyTextureForBrowserOptions* options) {
            if (options->nextInChain != nullptr) {
                return DAWN_VALIDATION_ERROR(
                    "CopyTextureForBrowserOptions: nextInChain must be nullptr");
            }

            return {};
        }

        RenderPipelineBase* GetOrCreateCopyTextureForBrowserPipeline(DeviceBase* device) {
            InternalPipelineStore* store = device->GetInternalPipelineStore();

            if (store->copyTextureForBrowserPipeline == nullptr) {
                // Create vertex shader module if not cached before.
                if (store->copyTextureForBrowserVS == nullptr) {
                    ShaderModuleDescriptor descriptor;
                    ShaderModuleWGSLDescriptor wgslDesc;
                    wgslDesc.source = sCopyTextureForBrowserVertex;
                    descriptor.nextInChain = reinterpret_cast<ChainedStruct*>(&wgslDesc);

                    store->copyTextureForBrowserVS =
                        AcquireRef(device->CreateShaderModule(&descriptor));
                }

                ShaderModuleBase* vertexModule = store->copyTextureForBrowserVS.Get();

                // Create fragment shader module if not cached before.
                if (store->copyTextureForBrowserFS == nullptr) {
                    ShaderModuleDescriptor descriptor;
                    ShaderModuleWGSLDescriptor wgslDesc;
                    wgslDesc.source = sPassthrough2D4ChannelFrag;
                    descriptor.nextInChain = reinterpret_cast<ChainedStruct*>(&wgslDesc);
                    store->copyTextureForBrowserFS =
                        AcquireRef(device->CreateShaderModule(&descriptor));
                }

                ShaderModuleBase* fragmentModule = store->copyTextureForBrowserFS.Get();

                // Prepare vertex stage.
                ProgrammableStageDescriptor vertexStage = {};
                vertexStage.module = vertexModule;
                vertexStage.entryPoint = "main";

                // Prepare frgament stage.
                ProgrammableStageDescriptor fragmentStage = {};
                fragmentStage.module = fragmentModule;
                fragmentStage.entryPoint = "main";

                // Prepare color state.
                ColorStateDescriptor colorState = {};
                colorState.format = wgpu::TextureFormat::RGBA8Unorm;

                // Create RenderPipeline.
                RenderPipelineDescriptor renderPipelineDesc = {};

                // Generate the layout based on shader modules.
                renderPipelineDesc.layout = nullptr;

                renderPipelineDesc.vertexStage = vertexStage;
                renderPipelineDesc.fragmentStage = &fragmentStage;

                renderPipelineDesc.primitiveTopology = wgpu::PrimitiveTopology::TriangleList;

                renderPipelineDesc.colorStateCount = 1;
                renderPipelineDesc.colorStates = &colorState;

                store->copyTextureForBrowserPipeline =
                    AcquireRef(device->CreateRenderPipeline(&renderPipelineDesc));
            }

            return store->copyTextureForBrowserPipeline.Get();
        }

    }  // anonymous namespace

    MaybeError ValidateCopyTextureForBrowser(DeviceBase* device,
                                             const TextureCopyView* source,
                                             const TextureCopyView* destination,
                                             const Extent3D* copySize,
                                             const CopyTextureForBrowserOptions* options) {
        DAWN_TRY(device->ValidateObject(source->texture));
        DAWN_TRY(device->ValidateObject(destination->texture));

        DAWN_TRY(ValidateTextureCopyView(device, *source, *copySize));
        DAWN_TRY(ValidateTextureCopyView(device, *destination, *copySize));

        DAWN_TRY(ValidateTextureToTextureCopyRestrictions(*source, *destination, *copySize));

        DAWN_TRY(ValidateTextureCopyRange(*source, *copySize));
        DAWN_TRY(ValidateTextureCopyRange(*destination, *copySize));

        DAWN_TRY(ValidateCanUseAs(source->texture, wgpu::TextureUsage::CopySrc));
        DAWN_TRY(ValidateCanUseAs(destination->texture, wgpu::TextureUsage::CopyDst));

        DAWN_TRY(ValidateCopyTextureFormatConversion(source->texture->GetFormat().format,
                                                     destination->texture->GetFormat().format));

        DAWN_TRY(ValidateCopyTextureForBrowserOptions(options));

        // TODO(shaobo.yan@intel.com): Support the simplest case for now that source and destination
        // texture has the same size and do full texture blit. Will address sub texture blit in
        // future and remove these validations.
        if (source->origin.x != 0 || source->origin.y != 0 || source->origin.z != 0 ||
            destination->origin.x != 0 || destination->origin.y != 0 ||
            destination->origin.z != 0 || source->mipLevel != 0 || destination->mipLevel != 0 ||
            source->texture->GetWidth() != destination->texture->GetWidth() ||
            source->texture->GetHeight() != destination->texture->GetHeight()) {
            return DAWN_VALIDATION_ERROR("Cannot support sub blit now.");
        }

        return {};
    }

    MaybeError DoCopyTextureForBrowser(DeviceBase* device,
                                       const TextureCopyView* source,
                                       const TextureCopyView* destination,
                                       const Extent3D* copySize,
                                       const CopyTextureForBrowserOptions* options) {
        // TODO(shaobo.yan@intel.com): In D3D12 and Vulkan, compatible texture format can directly
        // copy to each other. This can be a potential fast path.
        RenderPipelineBase* pipeline = GetOrCreateCopyTextureForBrowserPipeline(device);

        // Prepare bind group layout.
        Ref<BindGroupLayoutBase> layout = AcquireRef(pipeline->GetBindGroupLayout(0));

        // Prepare bind group descriptor
        BindGroupEntry bindGroupEntries[3] = {};
        BindGroupDescriptor bgDesc = {};
        bgDesc.layout = layout.Get();
        bgDesc.entryCount = 3;
        bgDesc.entries = bindGroupEntries;

        // Prepare binding 0 resource: uniform buffer.
        float uniformData[] = {
            1.0, 1.0,  // scale
            0.0, 0.0   // offset
        };

        // Handle flipY.
        if (options && options->flipY) {
            uniformData[1] *= -1.0;
            uniformData[3] += 1.0;
        }

        BufferDescriptor uniformDesc = {};
        uniformDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
        uniformDesc.size = sizeof(uniformData);
        Ref<BufferBase> uniformBuffer = AcquireRef(device->CreateBuffer(&uniformDesc));

        device->GetDefaultQueue()->WriteBuffer(uniformBuffer.Get(), 0, uniformData,
                                               sizeof(uniformData));

        // Prepare binding 1 resource: sampler
        // Use default configuration, filterMode set to Nearest for min and mag.
        SamplerDescriptor samplerDesc = {};
        Ref<SamplerBase> sampler = AcquireRef(device->CreateSampler(&samplerDesc));

        // Prepare binding 2 resource: sampled texture
        TextureViewDescriptor srcTextureViewDesc = {};
        srcTextureViewDesc.baseMipLevel = source->mipLevel;
        srcTextureViewDesc.mipLevelCount = 1;
        Ref<TextureViewBase> srcTextureView =
            AcquireRef(source->texture->CreateView(&srcTextureViewDesc));

        // Set bind group entries.
        bindGroupEntries[0].binding = 0;
        bindGroupEntries[0].buffer = uniformBuffer.Get();
        bindGroupEntries[0].size = sizeof(uniformData);
        bindGroupEntries[1].binding = 1;
        bindGroupEntries[1].sampler = sampler.Get();
        bindGroupEntries[2].binding = 2;
        bindGroupEntries[2].textureView = srcTextureView.Get();

        // Create bind group after all binding entries are set.
        Ref<BindGroupBase> bindGroup = AcquireRef(device->CreateBindGroup(&bgDesc));

        // Create command encoder.
        CommandEncoderDescriptor encoderDesc = {};
        Ref<CommandEncoder> encoder = AcquireRef(device->CreateCommandEncoder(&encoderDesc));

        // Prepare dst texture view as color Attachment.
        TextureViewDescriptor dstTextureViewDesc;
        dstTextureViewDesc.baseMipLevel = destination->mipLevel;
        dstTextureViewDesc.mipLevelCount = 1;
        Ref<TextureViewBase> dstView =
            AcquireRef(destination->texture->CreateView(&dstTextureViewDesc));

        // Prepare render pass color attachment descriptor.
        RenderPassColorAttachmentDescriptor colorAttachmentDesc;
        colorAttachmentDesc.attachment = dstView.Get();
        colorAttachmentDesc.loadOp = wgpu::LoadOp::Load;
        colorAttachmentDesc.storeOp = wgpu::StoreOp::Store;
        colorAttachmentDesc.clearColor = {0.0, 0.0, 0.0, 1.0};

        // Create render pass.
        RenderPassDescriptor renderPassDesc;
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &colorAttachmentDesc;
        Ref<RenderPassEncoder> passEncoder = AcquireRef(encoder->BeginRenderPass(&renderPassDesc));

        // Start pipeline  and encode commands to complete
        // the copy from src texture to dst texture with transformation.
        passEncoder->SetPipeline(pipeline);
        passEncoder->SetBindGroup(0, bindGroup.Get());
        passEncoder->Draw(3);
        passEncoder->EndPass();

        // Finsh encoding.
        Ref<CommandBufferBase> commandBuffer = AcquireRef(encoder->Finish());
        CommandBufferBase* submitCommandBuffer = commandBuffer.Get();

        // Submit command buffer.
        Ref<QueueBase> queue = AcquireRef(device->GetDefaultQueue());
        queue->Submit(1, &submitCommandBuffer);

        return {};
    }

}  // namespace dawn_native
