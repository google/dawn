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
#include "dawn_native/ValidationUtils_autogen.h"

#include <unordered_set>

namespace dawn_native {
    namespace {
// TODO(crbug.com/1221110): Remove this header macro by merging vertex and
// fragment shaders into one shader source. Now it blocks by
// crbug.com/dawn/947 and crbug.com/tint/915
#define HEADER                              \
    "        [[block]] struct Uniforms {\n" \
    "            u_scale: vec2<f32>;\n"     \
    "            u_offset: vec2<f32>;\n"    \
    "            u_alphaOp: u32;\n"         \
    "        };\n"

        static const char sCopyTextureForBrowserVertex[] = HEADER R"(
            [[binding(0), group(0)]] var<uniform> uniforms : Uniforms;

            struct VertexOutputs {
                [[location(0)]] texcoords : vec2<f32>;
                [[builtin(position)]] position : vec4<f32>;
            };

            [[stage(vertex)]] fn main(
                [[builtin(vertex_index)]] VertexIndex : u32
            ) -> VertexOutputs {
                var texcoord = array<vec2<f32>, 3>(
                    vec2<f32>(-0.5, 0.0),
                    vec2<f32>( 1.5, 0.0),
                    vec2<f32>( 0.5, 2.0));

                var output : VertexOutputs;
                output.position = vec4<f32>((texcoord[VertexIndex] * 2.0 - vec2<f32>(1.0, 1.0)), 0.0, 1.0);

                // Y component of scale is calculated by the copySizeHeight / textureHeight. Only
                // flipY case can get negative number.
                var flipY = uniforms.u_scale.y < 0.0;

                // Texture coordinate takes top-left as origin point. We need to map the
                // texture to triangle carefully.
                if (flipY) {
                    // We need to get the mirror positions(mirrored based on y = 0.5) on flip cases.
                    // Adopt transform to src texture and then mapping it to triangle coord which
                    // do a +1 shift on Y dimension will help us got that mirror position perfectly.
                    output.texcoords = (texcoord[VertexIndex] * uniforms.u_scale + uniforms.u_offset) *
                        vec2<f32>(1.0, -1.0) + vec2<f32>(0.0, 1.0);
                } else {
                    // For the normal case, we need to get the exact position.
                    // So mapping texture to triangle firstly then adopt the transform.
                    output.texcoords = (texcoord[VertexIndex] *
                        vec2<f32>(1.0, -1.0) + vec2<f32>(0.0, 1.0)) *
                        uniforms.u_scale + uniforms.u_offset;
                }

                return output;
            }
        )";

        static const char sCopyTextureForBrowserFragment[] = HEADER R"(
            [[binding(0), group(0)]] var<uniform> uniforms : Uniforms;
            [[binding(1), group(0)]] var mySampler: sampler;
            [[binding(2), group(0)]] var myTexture: texture_2d<f32>;

            [[stage(fragment)]] fn main(
                [[location(0)]] texcoord : vec2<f32>
            ) -> [[location(0)]] vec4<f32> {
                // Clamp the texcoord and discard the out-of-bound pixels.
                var clampedTexcoord =
                    clamp(texcoord, vec2<f32>(0.0, 0.0), vec2<f32>(1.0, 1.0));
                if (!all(clampedTexcoord == texcoord)) {
                    discard;
                }

                // Swizzling of texture formats when sampling / rendering is handled by the
                // hardware so we don't need special logic in this shader. This is covered by tests.
                var srcColor = textureSample(myTexture, mySampler, texcoord);

                // Handle alpha. Alpha here helps on the source content and dst content have
                // different alpha config. There are three possible ops: DontChange, Premultiply
                // and Unpremultiply.
                // TODO(crbug.com/1217153): if wgsl support `constexpr` and allow it
                // to be case selector, Replace 0u/1u/2u with a constexpr variable with
                // meaningful name.
                switch(uniforms.u_alphaOp) {
                    case 0u: { // AlphaOp: DontChange
                        break;
                    }
                    case 1u: { // AlphaOp: Premultiply
                        srcColor = vec4<f32>(srcColor.rgb * srcColor.a, srcColor.a);
                        break;
                    }
                    case 2u: { // AlphaOp: Unpremultiply
                        if (srcColor.a != 0.0) {
                            srcColor = vec4<f32>(srcColor.rgb / srcColor.a, srcColor.a);
                        }
                        break;
                    }
                    default: {
                        break;
                    }
                }

                return srcColor;
            }
        )";

        struct Uniform {
            float scaleX;
            float scaleY;
            float offsetX;
            float offsetY;
            wgpu::AlphaOp alphaOp;
        };
        static_assert(sizeof(Uniform) == 20, "");

        // TODO(crbug.com/dawn/856): Expand copyTextureForBrowser to support any
        // non-depth, non-stencil, non-compressed texture format pair copy. Now this API
        // supports CopyImageBitmapToTexture normal format pairs.
        MaybeError ValidateCopyTextureFormatConversion(const wgpu::TextureFormat srcFormat,
                                                       const wgpu::TextureFormat dstFormat) {
            switch (srcFormat) {
                case wgpu::TextureFormat::BGRA8Unorm:
                case wgpu::TextureFormat::RGBA8Unorm:
                    break;
                default:
                    return DAWN_VALIDATION_ERROR(
                        "Unsupported src texture format for CopyTextureForBrowser.");
            }

            switch (dstFormat) {
                case wgpu::TextureFormat::RGBA8Unorm:
                case wgpu::TextureFormat::BGRA8Unorm:
                case wgpu::TextureFormat::RGBA32Float:
                case wgpu::TextureFormat::RG8Unorm:
                case wgpu::TextureFormat::RGBA16Float:
                case wgpu::TextureFormat::RG16Float:
                case wgpu::TextureFormat::RGB10A2Unorm:
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

            DAWN_TRY(ValidateAlphaOp(options->alphaOp));

            return {};
        }

        MaybeError ValidateSourceOriginAndCopyExtent(const ImageCopyTexture source,
                                                     const Extent3D copySize) {
            if (source.origin.z > 0) {
                return DAWN_VALIDATION_ERROR("Source origin cannot have non-zero z value");
            }

            if (copySize.depthOrArrayLayers > 1) {
                return DAWN_VALIDATION_ERROR("Cannot copy to multiple slices");
            }

            return {};
        }

        MaybeError ValidateSourceAndDestinationTextureSampleCount(
            const ImageCopyTexture source,
            const ImageCopyTexture destination) {
            if (source.texture->GetSampleCount() > 1 || destination.texture->GetSampleCount() > 1) {
                return DAWN_VALIDATION_ERROR(
                    "Source and destiantion textures cannot be multisampled");
            }

            return {};
        }

        RenderPipelineBase* GetCachedPipeline(InternalPipelineStore* store,
                                              wgpu::TextureFormat dstFormat) {
            auto pipeline = store->copyTextureForBrowserPipelines.find(dstFormat);
            if (pipeline != store->copyTextureForBrowserPipelines.end()) {
                return pipeline->second.Get();
            }
            return nullptr;
        }

        ResultOrError<RenderPipelineBase*> GetOrCreateCopyTextureForBrowserPipeline(
            DeviceBase* device,
            wgpu::TextureFormat dstFormat) {
            InternalPipelineStore* store = device->GetInternalPipelineStore();

            if (GetCachedPipeline(store, dstFormat) == nullptr) {
                // Create vertex shader module if not cached before.
                if (store->copyTextureForBrowserVS == nullptr) {
                    ShaderModuleDescriptor descriptor;
                    ShaderModuleWGSLDescriptor wgslDesc;
                    wgslDesc.source = sCopyTextureForBrowserVertex;
                    descriptor.nextInChain = reinterpret_cast<ChainedStruct*>(&wgslDesc);

                    DAWN_TRY_ASSIGN(store->copyTextureForBrowserVS,
                                    device->CreateShaderModule(&descriptor));
                }

                ShaderModuleBase* vertexModule = store->copyTextureForBrowserVS.Get();

                // Create fragment shader module if not cached before.
                if (store->copyTextureForBrowserFS == nullptr) {
                    ShaderModuleDescriptor descriptor;
                    ShaderModuleWGSLDescriptor wgslDesc;
                    wgslDesc.source = sCopyTextureForBrowserFragment;
                    descriptor.nextInChain = reinterpret_cast<ChainedStruct*>(&wgslDesc);
                    DAWN_TRY_ASSIGN(store->copyTextureForBrowserFS,
                                    device->CreateShaderModule(&descriptor));
                }

                ShaderModuleBase* fragmentModule = store->copyTextureForBrowserFS.Get();

                // Prepare vertex stage.
                VertexState vertex = {};
                vertex.module = vertexModule;
                vertex.entryPoint = "main";

                // Prepare frgament stage.
                FragmentState fragment = {};
                fragment.module = fragmentModule;
                fragment.entryPoint = "main";

                // Prepare color state.
                ColorTargetState target = {};
                target.format = dstFormat;

                // Create RenderPipeline.
                RenderPipelineDescriptor renderPipelineDesc = {};

                // Generate the layout based on shader modules.
                renderPipelineDesc.layout = nullptr;

                renderPipelineDesc.vertex = vertex;
                renderPipelineDesc.fragment = &fragment;

                renderPipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;

                fragment.targetCount = 1;
                fragment.targets = &target;

                Ref<RenderPipelineBase> pipeline;
                DAWN_TRY_ASSIGN(pipeline, device->CreateRenderPipeline(&renderPipelineDesc));
                store->copyTextureForBrowserPipelines.insert({dstFormat, std::move(pipeline)});
            }

            return GetCachedPipeline(store, dstFormat);
        }

    }  // anonymous namespace

    MaybeError ValidateCopyTextureForBrowser(DeviceBase* device,
                                             const ImageCopyTexture* source,
                                             const ImageCopyTexture* destination,
                                             const Extent3D* copySize,
                                             const CopyTextureForBrowserOptions* options) {
        DAWN_TRY(device->ValidateObject(source->texture));
        DAWN_TRY(device->ValidateObject(destination->texture));

        DAWN_TRY(ValidateImageCopyTexture(device, *source, *copySize));
        DAWN_TRY(ValidateImageCopyTexture(device, *destination, *copySize));

        DAWN_TRY(ValidateSourceOriginAndCopyExtent(*source, *copySize));
        DAWN_TRY(ValidateCopyTextureForBrowserRestrictions(*source, *destination, *copySize));
        DAWN_TRY(ValidateSourceAndDestinationTextureSampleCount(*source, *destination));

        DAWN_TRY(ValidateTextureCopyRange(device, *source, *copySize));
        DAWN_TRY(ValidateTextureCopyRange(device, *destination, *copySize));

        DAWN_TRY(ValidateCanUseAs(source->texture, wgpu::TextureUsage::CopySrc));
        DAWN_TRY(ValidateCanUseAs(source->texture, wgpu::TextureUsage::Sampled));

        DAWN_TRY(ValidateCanUseAs(destination->texture, wgpu::TextureUsage::CopyDst));
        DAWN_TRY(ValidateCanUseAs(destination->texture, wgpu::TextureUsage::RenderAttachment));

        DAWN_TRY(ValidateCopyTextureFormatConversion(source->texture->GetFormat().format,
                                                     destination->texture->GetFormat().format));

        DAWN_TRY(ValidateCopyTextureForBrowserOptions(options));

        return {};
    }

    MaybeError DoCopyTextureForBrowser(DeviceBase* device,
                                       const ImageCopyTexture* source,
                                       const ImageCopyTexture* destination,
                                       const Extent3D* copySize,
                                       const CopyTextureForBrowserOptions* options) {
        // TODO(crbug.com/dawn/856): In D3D12 and Vulkan, compatible texture format can directly
        // copy to each other. This can be a potential fast path.

        // Noop copy
        if (copySize->width == 0 || copySize->height == 0 || copySize->depthOrArrayLayers == 0) {
            return {};
        }

        RenderPipelineBase* pipeline;
        DAWN_TRY_ASSIGN(pipeline, GetOrCreateCopyTextureForBrowserPipeline(
                                      device, destination->texture->GetFormat().format));

        // Prepare bind group layout.
        Ref<BindGroupLayoutBase> layout;
        DAWN_TRY_ASSIGN(layout, pipeline->GetBindGroupLayout(0));

        // Prepare bind group descriptor
        BindGroupEntry bindGroupEntries[3] = {};
        BindGroupDescriptor bgDesc = {};
        bgDesc.layout = layout.Get();
        bgDesc.entryCount = 3;
        bgDesc.entries = bindGroupEntries;

        Extent3D srcTextureSize = source->texture->GetSize();

        // Prepare binding 0 resource: uniform buffer.
        Uniform uniformData = {
            copySize->width / static_cast<float>(srcTextureSize.width),
            copySize->height / static_cast<float>(srcTextureSize.height),  // scale
            source->origin.x / static_cast<float>(srcTextureSize.width),
            source->origin.y / static_cast<float>(srcTextureSize.height),  // offset
            wgpu::AlphaOp::DontChange  // alphaOp default value: DontChange
        };

        // Handle flipY. FlipY here means we flip the source texture firstly and then
        // do copy. This helps on the case which source texture is flipped and the copy
        // need to unpack the flip.
        if (options->flipY) {
            uniformData.scaleY *= -1.0;
            uniformData.offsetY += copySize->height / static_cast<float>(srcTextureSize.height);
        }

        // Set alpha op.
        uniformData.alphaOp = options->alphaOp;

        BufferDescriptor uniformDesc = {};
        uniformDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
        uniformDesc.size = sizeof(uniformData);
        Ref<BufferBase> uniformBuffer;
        DAWN_TRY_ASSIGN(uniformBuffer, device->CreateBuffer(&uniformDesc));

        DAWN_TRY(device->GetQueue()->WriteBuffer(uniformBuffer.Get(), 0, &uniformData,
                                                 sizeof(uniformData)));

        // Prepare binding 1 resource: sampler
        // Use default configuration, filterMode set to Nearest for min and mag.
        SamplerDescriptor samplerDesc = {};
        Ref<SamplerBase> sampler;
        DAWN_TRY_ASSIGN(sampler, device->CreateSampler(&samplerDesc));

        // Prepare binding 2 resource: sampled texture
        TextureViewDescriptor srcTextureViewDesc = {};
        srcTextureViewDesc.baseMipLevel = source->mipLevel;
        srcTextureViewDesc.mipLevelCount = 1;
        srcTextureViewDesc.arrayLayerCount = 1;
        Ref<TextureViewBase> srcTextureView;
        DAWN_TRY_ASSIGN(srcTextureView,
                        device->CreateTextureView(source->texture, &srcTextureViewDesc));

        // Set bind group entries.
        bindGroupEntries[0].binding = 0;
        bindGroupEntries[0].buffer = uniformBuffer.Get();
        bindGroupEntries[0].size = sizeof(uniformData);
        bindGroupEntries[1].binding = 1;
        bindGroupEntries[1].sampler = sampler.Get();
        bindGroupEntries[2].binding = 2;
        bindGroupEntries[2].textureView = srcTextureView.Get();

        // Create bind group after all binding entries are set.
        Ref<BindGroupBase> bindGroup;
        DAWN_TRY_ASSIGN(bindGroup, device->CreateBindGroup(&bgDesc));

        // Create command encoder.
        CommandEncoderDescriptor encoderDesc = {};
        // TODO(dawn:723): change to not use AcquireRef for reentrant object creation.
        Ref<CommandEncoder> encoder = AcquireRef(device->APICreateCommandEncoder(&encoderDesc));

        // Prepare dst texture view as color Attachment.
        TextureViewDescriptor dstTextureViewDesc;
        dstTextureViewDesc.baseMipLevel = destination->mipLevel;
        dstTextureViewDesc.mipLevelCount = 1;
        dstTextureViewDesc.baseArrayLayer = destination->origin.z;
        dstTextureViewDesc.arrayLayerCount = 1;
        Ref<TextureViewBase> dstView;
        DAWN_TRY_ASSIGN(dstView,
                        device->CreateTextureView(destination->texture, &dstTextureViewDesc));

        // Prepare render pass color attachment descriptor.
        RenderPassColorAttachmentDescriptor colorAttachmentDesc;

        colorAttachmentDesc.view = dstView.Get();
        colorAttachmentDesc.loadOp = wgpu::LoadOp::Load;
        colorAttachmentDesc.storeOp = wgpu::StoreOp::Store;
        colorAttachmentDesc.clearColor = {0.0, 0.0, 0.0, 1.0};

        // Create render pass.
        RenderPassDescriptor renderPassDesc;
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &colorAttachmentDesc;
        // TODO(dawn:723): change to not use AcquireRef for reentrant object creation.
        Ref<RenderPassEncoder> passEncoder =
            AcquireRef(encoder->APIBeginRenderPass(&renderPassDesc));

        // Start pipeline  and encode commands to complete
        // the copy from src texture to dst texture with transformation.
        passEncoder->APISetPipeline(pipeline);
        passEncoder->APISetBindGroup(0, bindGroup.Get());
        passEncoder->APISetViewport(destination->origin.x, destination->origin.y, copySize->width,
                                    copySize->height, 0.0, 1.0);
        passEncoder->APIDraw(3);
        passEncoder->APIEndPass();

        // Finsh encoding.
        // TODO(dawn:723): change to not use AcquireRef for reentrant object creation.
        Ref<CommandBufferBase> commandBuffer = AcquireRef(encoder->APIFinish());
        CommandBufferBase* submitCommandBuffer = commandBuffer.Get();

        // Submit command buffer.
        device->GetQueue()->APISubmit(1, &submitCommandBuffer);

        return {};
    }

}  // namespace dawn_native
