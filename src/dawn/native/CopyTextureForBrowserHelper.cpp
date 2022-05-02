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

#include "dawn/native/CopyTextureForBrowserHelper.h"

#include <unordered_set>
#include <utility>

#include "dawn/common/Log.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Device.h"
#include "dawn/native/InternalPipelineStore.h"
#include "dawn/native/Queue.h"
#include "dawn/native/RenderPassEncoder.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/Sampler.h"
#include "dawn/native/Texture.h"
#include "dawn/native/ValidationUtils_autogen.h"
#include "dawn/native/utils/WGPUHelpers.h"

namespace dawn::native {
namespace {

static const char sCopyTextureForBrowserShader[] = R"(
            struct GammaTransferParams {
                G: f32,
                A: f32,
                B: f32,
                C: f32,
                D: f32,
                E: f32,
                F: f32,
                padding: u32,
            };

            struct Uniforms {                                            // offset   align   size
                scale: vec2<f32>,                                        // 0        8       8
                offset: vec2<f32>,                                       // 8        8       8
                steps_mask: u32,                                         // 16       4       4
                // implicit padding;                                     // 20               12
                conversion_matrix: mat3x3<f32>,                          // 32       16      48
                gamma_decoding_params: GammaTransferParams,              // 80       4       32
                gamma_encoding_params: GammaTransferParams,              // 112      4       32
                gamma_decoding_for_dst_srgb_params: GammaTransferParams, // 144      4       32
            };

            @binding(0) @group(0) var<uniform> uniforms : Uniforms;

            struct VertexOutputs {
                @location(0) texcoords : vec2<f32>,
                @builtin(position) position : vec4<f32>,
            };

            // Chromium uses unified equation to construct gamma decoding function
            // and gamma encoding function.
            // The logic is:
            //  if x < D
            //      linear = C * x + F
            //  nonlinear = pow(A * x + B, G) + E
            // (https://source.chromium.org/chromium/chromium/src/+/main:ui/gfx/color_transform.cc;l=541)
            // Expand the equation with sign() to make it handle all gamma conversions.
            fn gamma_conversion(v: f32, params: GammaTransferParams) -> f32 {
                // Linear part: C * x + F
                if (abs(v) < params.D) {
                    return sign(v) * (params.C * abs(v) + params.F);
                }

                // Gamma part: pow(A * x + B, G) + E
                return sign(v) * (pow(params.A * abs(v) + params.B, params.G) + params.E);
            }

            @stage(vertex)
            fn vs_main(
                @builtin(vertex_index) VertexIndex : u32
            ) -> VertexOutputs {
                var texcoord = array<vec2<f32>, 3>(
                    vec2<f32>(-0.5, 0.0),
                    vec2<f32>( 1.5, 0.0),
                    vec2<f32>( 0.5, 2.0));

                var output : VertexOutputs;
                output.position = vec4<f32>((texcoord[VertexIndex] * 2.0 - vec2<f32>(1.0, 1.0)), 0.0, 1.0);

                // Y component of scale is calculated by the copySizeHeight / textureHeight. Only
                // flipY case can get negative number.
                var flipY = uniforms.scale.y < 0.0;

                // Texture coordinate takes top-left as origin point. We need to map the
                // texture to triangle carefully.
                if (flipY) {
                    // We need to get the mirror positions(mirrored based on y = 0.5) on flip cases.
                    // Adopt transform to src texture and then mapping it to triangle coord which
                    // do a +1 shift on Y dimension will help us got that mirror position perfectly.
                    output.texcoords = (texcoord[VertexIndex] * uniforms.scale + uniforms.offset) *
                        vec2<f32>(1.0, -1.0) + vec2<f32>(0.0, 1.0);
                } else {
                    // For the normal case, we need to get the exact position.
                    // So mapping texture to triangle firstly then adopt the transform.
                    output.texcoords = (texcoord[VertexIndex] *
                        vec2<f32>(1.0, -1.0) + vec2<f32>(0.0, 1.0)) *
                        uniforms.scale + uniforms.offset;
                }

                return output;
            }

            @binding(1) @group(0) var mySampler: sampler;
            @binding(2) @group(0) var myTexture: texture_2d<f32>;

            @stage(fragment)
            fn fs_main(
                @location(0) texcoord : vec2<f32>
            ) -> @location(0) vec4<f32> {
                // Clamp the texcoord and discard the out-of-bound pixels.
                var clampedTexcoord =
                    clamp(texcoord, vec2<f32>(0.0, 0.0), vec2<f32>(1.0, 1.0));

                // Swizzling of texture formats when sampling / rendering is handled by the
                // hardware so we don't need special logic in this shader. This is covered by tests.
                var color = textureSample(myTexture, mySampler, texcoord);

                if (!all(clampedTexcoord == texcoord)) {
                    discard;
                }

                let kUnpremultiplyStep = 0x01u;
                let kDecodeToLinearStep = 0x02u;
                let kConvertToDstGamutStep = 0x04u;
                let kEncodeToGammaStep = 0x08u;
                let kPremultiplyStep = 0x10u;
                let kDecodeForSrgbDstFormat = 0x20u;

                // Unpremultiply step. Appling color space conversion op on premultiplied source texture
                // also needs to unpremultiply first.
                if (bool(uniforms.steps_mask & kUnpremultiplyStep)) {
                    if (color.a != 0.0) {
                        color = vec4<f32>(color.rgb / color.a, color.a);
                    }
                }

                // Linearize the source color using the source color space’s
                // transfer function if it is non-linear.
                if (bool(uniforms.steps_mask & kDecodeToLinearStep)) {
                    color = vec4<f32>(gamma_conversion(color.r, uniforms.gamma_decoding_params),
                                      gamma_conversion(color.g, uniforms.gamma_decoding_params),
                                      gamma_conversion(color.b, uniforms.gamma_decoding_params),
                                      color.a);
                }

                // Convert unpremultiplied, linear source colors to the destination gamut by
                // multiplying by a 3x3 matrix. Calculate transformFromXYZD50 * transformToXYZD50
                // in CPU side and upload the final result in uniforms.
                if (bool(uniforms.steps_mask & kConvertToDstGamutStep)) {
                    color = vec4<f32>(uniforms.conversion_matrix * color.rgb, color.a);
                }

                // Encode that color using the inverse of the destination color
                // space’s transfer function if it is non-linear.
                if (bool(uniforms.steps_mask & kEncodeToGammaStep)) {
                    color = vec4<f32>(gamma_conversion(color.r, uniforms.gamma_encoding_params),
                                      gamma_conversion(color.g, uniforms.gamma_encoding_params),
                                      gamma_conversion(color.b, uniforms.gamma_encoding_params),
                                      color.a);
                }

                // Premultiply step.
                if (bool(uniforms.steps_mask & kPremultiplyStep)) {
                    color = vec4<f32>(color.rgb * color.a, color.a);
                }

                // Decode for copying from non-srgb formats to srgb formats
                if (bool(uniforms.steps_mask & kDecodeForSrgbDstFormat)) {
                    color = vec4<f32>(gamma_conversion(color.r, uniforms.gamma_decoding_for_dst_srgb_params),
                                      gamma_conversion(color.g, uniforms.gamma_decoding_for_dst_srgb_params),
                                      gamma_conversion(color.b, uniforms.gamma_decoding_for_dst_srgb_params),
                                      color.a);
                }

                return color;
            }
        )";

// Follow the same order of skcms_TransferFunction
// https://source.chromium.org/chromium/chromium/src/+/main:third_party/skia/include/third_party/skcms/skcms.h;l=46;
struct GammaTransferParams {
    float G = 0.0;
    float A = 0.0;
    float B = 0.0;
    float C = 0.0;
    float D = 0.0;
    float E = 0.0;
    float F = 0.0;
    uint32_t padding = 0;
};

struct Uniform {
    float scaleX;
    float scaleY;
    float offsetX;
    float offsetY;
    uint32_t stepsMask = 0;
    const std::array<uint32_t, 3> padding = {};  // 12 bytes padding
    std::array<float, 12> conversionMatrix = {};
    GammaTransferParams gammaDecodingParams = {};
    GammaTransferParams gammaEncodingParams = {};
    GammaTransferParams gammaDecodingForDstSrgbParams = {};
};
static_assert(sizeof(Uniform) == 176);

// TODO(crbug.com/dawn/856): Expand copyTextureForBrowser to support any
// non-depth, non-stencil, non-compressed texture format pair copy.
MaybeError ValidateCopyTextureFormatConversion(const wgpu::TextureFormat srcFormat,
                                               const wgpu::TextureFormat dstFormat) {
    switch (srcFormat) {
        case wgpu::TextureFormat::BGRA8Unorm:
        case wgpu::TextureFormat::RGBA8Unorm:
            break;
        default:
            return DAWN_FORMAT_VALIDATION_ERROR("Source texture format (%s) is not supported.",
                                                srcFormat);
    }

    switch (dstFormat) {
        case wgpu::TextureFormat::R8Unorm:
        case wgpu::TextureFormat::R16Float:
        case wgpu::TextureFormat::R32Float:
        case wgpu::TextureFormat::RG8Unorm:
        case wgpu::TextureFormat::RG16Float:
        case wgpu::TextureFormat::RG32Float:
        case wgpu::TextureFormat::RGBA8Unorm:
        case wgpu::TextureFormat::RGBA8UnormSrgb:
        case wgpu::TextureFormat::BGRA8Unorm:
        case wgpu::TextureFormat::BGRA8UnormSrgb:
        case wgpu::TextureFormat::RGB10A2Unorm:
        case wgpu::TextureFormat::RGBA16Float:
        case wgpu::TextureFormat::RGBA32Float:
            break;
        default:
            return DAWN_FORMAT_VALIDATION_ERROR("Destination texture format (%s) is not supported.",
                                                dstFormat);
    }

    return {};
}

RenderPipelineBase* GetCachedPipeline(InternalPipelineStore* store, wgpu::TextureFormat dstFormat) {
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
        if (store->copyTextureForBrowser == nullptr) {
            DAWN_TRY_ASSIGN(store->copyTextureForBrowser,
                            utils::CreateShaderModule(device, sCopyTextureForBrowserShader));
        }

        ShaderModuleBase* shaderModule = store->copyTextureForBrowser.Get();

        // Prepare vertex stage.
        VertexState vertex = {};
        vertex.module = shaderModule;
        vertex.entryPoint = "vs_main";

        // Prepare frgament stage.
        FragmentState fragment = {};
        fragment.module = shaderModule;
        fragment.entryPoint = "fs_main";

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

    DAWN_INVALID_IF(source->texture->GetTextureState() == TextureBase::TextureState::Destroyed,
                    "Source texture %s is destroyed.", source->texture);

    DAWN_INVALID_IF(destination->texture->GetTextureState() == TextureBase::TextureState::Destroyed,
                    "Destination texture %s is destroyed.", destination->texture);

    DAWN_TRY_CONTEXT(ValidateImageCopyTexture(device, *source, *copySize),
                     "validating the ImageCopyTexture for the source");
    DAWN_TRY_CONTEXT(ValidateImageCopyTexture(device, *destination, *copySize),
                     "validating the ImageCopyTexture for the destination");

    DAWN_TRY_CONTEXT(ValidateTextureCopyRange(device, *source, *copySize),
                     "validating that the copy fits in the source");
    DAWN_TRY_CONTEXT(ValidateTextureCopyRange(device, *destination, *copySize),
                     "validating that the copy fits in the destination");

    DAWN_TRY(ValidateTextureToTextureCopyCommonRestrictions(*source, *destination, *copySize));

    DAWN_INVALID_IF(source->origin.z > 0, "Source has a non-zero z origin (%u).", source->origin.z);
    DAWN_INVALID_IF(copySize->depthOrArrayLayers > 1, "Copy is for more than one array layer (%u)",
                    copySize->depthOrArrayLayers);

    DAWN_INVALID_IF(
        source->texture->GetSampleCount() > 1 || destination->texture->GetSampleCount() > 1,
        "The source texture sample count (%u) or the destination texture sample count (%u) is "
        "not 1.",
        source->texture->GetSampleCount(), destination->texture->GetSampleCount());

    DAWN_TRY(ValidateCanUseAs(source->texture, wgpu::TextureUsage::CopySrc,
                              UsageValidationMode::Default));
    DAWN_TRY(ValidateCanUseAs(source->texture, wgpu::TextureUsage::TextureBinding,
                              UsageValidationMode::Default));

    DAWN_TRY(ValidateCanUseAs(destination->texture, wgpu::TextureUsage::CopyDst,
                              UsageValidationMode::Default));
    DAWN_TRY(ValidateCanUseAs(destination->texture, wgpu::TextureUsage::RenderAttachment,
                              UsageValidationMode::Default));

    DAWN_TRY(ValidateCopyTextureFormatConversion(source->texture->GetFormat().format,
                                                 destination->texture->GetFormat().format));

    DAWN_INVALID_IF(options->nextInChain != nullptr, "nextInChain must be nullptr");

    DAWN_TRY(ValidateAlphaMode(options->srcAlphaMode));
    DAWN_TRY(ValidateAlphaMode(options->dstAlphaMode));

    if (options->needsColorSpaceConversion) {
        DAWN_INVALID_IF(options->srcTransferFunctionParameters == nullptr,
                        "srcTransferFunctionParameters is nullptr when doing color conversion");
        DAWN_INVALID_IF(options->conversionMatrix == nullptr,
                        "conversionMatrix is nullptr when doing color conversion");
        DAWN_INVALID_IF(options->dstTransferFunctionParameters == nullptr,
                        "dstTransferFunctionParameters is nullptr when doing color conversion");
    }
    return {};
}

// Whether the format of dst texture of CopyTextureForBrowser() is srgb or non-srgb.
bool IsSrgbDstFormat(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::RGBA8UnormSrgb:
        case wgpu::TextureFormat::BGRA8UnormSrgb:
            return true;
        default:
            return false;
    }
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

    bool isSrgbDstFormat = IsSrgbDstFormat(destination->texture->GetFormat().format);
    RenderPipelineBase* pipeline;
    DAWN_TRY_ASSIGN(pipeline, GetOrCreateCopyTextureForBrowserPipeline(
                                  device, destination->texture->GetFormat().format));

    // Prepare bind group layout.
    Ref<BindGroupLayoutBase> layout;
    DAWN_TRY_ASSIGN(layout, pipeline->GetBindGroupLayout(0));

    Extent3D srcTextureSize = source->texture->GetSize();

    // Prepare binding 0 resource: uniform buffer.
    Uniform uniformData = {
        copySize->width / static_cast<float>(srcTextureSize.width),
        copySize->height / static_cast<float>(srcTextureSize.height),  // scale
        source->origin.x / static_cast<float>(srcTextureSize.width),
        source->origin.y / static_cast<float>(srcTextureSize.height)  // offset
    };

    // Handle flipY. FlipY here means we flip the source texture firstly and then
    // do copy. This helps on the case which source texture is flipped and the copy
    // need to unpack the flip.
    if (options->flipY) {
        uniformData.scaleY *= -1.0;
        uniformData.offsetY += copySize->height / static_cast<float>(srcTextureSize.height);
    }

    uint32_t stepsMask = 0u;

    // Steps to do color space conversion
    // From https://skia.org/docs/user/color/
    // - unpremultiply if the source color is premultiplied; Alpha is not involved in color
    // management, and we need to divide it out if it’s multiplied in.
    // - linearize the source color using the source color space’s transfer function
    // - convert those unpremultiplied, linear source colors to XYZ D50 gamut by multiplying by
    // a 3x3 matrix.
    // - convert those XYZ D50 colors to the destination gamut by multiplying by a 3x3 matrix.
    // - encode that color using the inverse of the destination color space’s transfer function.
    // - premultiply by alpha if the destination is premultiplied.
    // The reason to choose XYZ D50 as intermediate color space:
    // From http://www.brucelindbloom.com/index.html?WorkingSpaceInfo.html
    // "Since the Lab TIFF specification, the ICC profile specification and
    // Adobe Photoshop all use a D50"
    constexpr uint32_t kUnpremultiplyStep = 0x01;
    constexpr uint32_t kDecodeToLinearStep = 0x02;
    constexpr uint32_t kConvertToDstGamutStep = 0x04;
    constexpr uint32_t kEncodeToGammaStep = 0x08;
    constexpr uint32_t kPremultiplyStep = 0x10;
    constexpr uint32_t kDecodeForSrgbDstFormat = 0x20;

    if (options->srcAlphaMode == wgpu::AlphaMode::Premultiplied) {
        if (options->needsColorSpaceConversion || options->srcAlphaMode != options->dstAlphaMode) {
            stepsMask |= kUnpremultiplyStep;
        }
    }

    if (options->needsColorSpaceConversion) {
        stepsMask |= kDecodeToLinearStep;
        const float* decodingParams = options->srcTransferFunctionParameters;

        uniformData.gammaDecodingParams = {decodingParams[0], decodingParams[1], decodingParams[2],
                                           decodingParams[3], decodingParams[4], decodingParams[5],
                                           decodingParams[6]};

        stepsMask |= kConvertToDstGamutStep;
        const float* matrix = options->conversionMatrix;
        uniformData.conversionMatrix = {{
            matrix[0],
            matrix[1],
            matrix[2],
            0.0,
            matrix[3],
            matrix[4],
            matrix[5],
            0.0,
            matrix[6],
            matrix[7],
            matrix[8],
            0.0,
        }};

        stepsMask |= kEncodeToGammaStep;
        const float* encodingParams = options->dstTransferFunctionParameters;

        uniformData.gammaEncodingParams = {encodingParams[0], encodingParams[1], encodingParams[2],
                                           encodingParams[3], encodingParams[4], encodingParams[5],
                                           encodingParams[6]};
    }

    if (options->dstAlphaMode == wgpu::AlphaMode::Premultiplied) {
        if (options->needsColorSpaceConversion || options->srcAlphaMode != options->dstAlphaMode) {
            stepsMask |= kPremultiplyStep;
        }
    }

    // Copy to *-srgb texture should keep the bytes exactly the same as copy
    // to non-srgb texture. Add an extra decode-to-linear step so that after the
    // sampler of *-srgb format texture applying encoding, the bytes keeps the same
    // as non-srgb format texture.
    // NOTE: CopyTextureForBrowser() doesn't need to accept *-srgb format texture as
    // source input. But above operation also valid for *-srgb format texture input and
    // non-srgb format dst texture.
    // TODO(crbug.com/dawn/1195): Reinterpret to non-srgb texture view on *-srgb texture
    // and use it as render attachment when possible.
    // TODO(crbug.com/dawn/1195): Opt the condition for this extra step. It is possible to
    // bypass this extra step in some cases.
    if (isSrgbDstFormat) {
        stepsMask |= kDecodeForSrgbDstFormat;
        // Get gamma-linear conversion params from https://en.wikipedia.org/wiki/SRGB with some
        // mathematics. Order: {G, A, B, C, D, E, F, }
        uniformData.gammaDecodingForDstSrgbParams = {
            2.4, 1.0 / 1.055, 0.055 / 1.055, 1.0 / 12.92, 4.045e-02, 0.0, 0.0};
    }

    uniformData.stepsMask = stepsMask;

    Ref<BufferBase> uniformBuffer;
    DAWN_TRY_ASSIGN(
        uniformBuffer,
        utils::CreateBufferFromData(device, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform,
                                    {uniformData}));

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

    // Create bind group after all binding entries are set.
    Ref<BindGroupBase> bindGroup;
    DAWN_TRY_ASSIGN(bindGroup,
                    utils::MakeBindGroup(device, layout,
                                         {{0, uniformBuffer}, {1, sampler}, {2, srcTextureView}}));

    // Create command encoder.
    Ref<CommandEncoder> encoder;
    DAWN_TRY_ASSIGN(encoder, device->CreateCommandEncoder());

    // Prepare dst texture view as color Attachment.
    TextureViewDescriptor dstTextureViewDesc;
    dstTextureViewDesc.baseMipLevel = destination->mipLevel;
    dstTextureViewDesc.mipLevelCount = 1;
    dstTextureViewDesc.baseArrayLayer = destination->origin.z;
    dstTextureViewDesc.arrayLayerCount = 1;
    Ref<TextureViewBase> dstView;

    DAWN_TRY_ASSIGN(dstView, device->CreateTextureView(destination->texture, &dstTextureViewDesc));
    // Prepare render pass color attachment descriptor.
    RenderPassColorAttachment colorAttachmentDesc;

    colorAttachmentDesc.view = dstView.Get();
    colorAttachmentDesc.loadOp = wgpu::LoadOp::Load;
    colorAttachmentDesc.storeOp = wgpu::StoreOp::Store;
    colorAttachmentDesc.clearValue = {0.0, 0.0, 0.0, 1.0};

    // Create render pass.
    RenderPassDescriptor renderPassDesc;
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachmentDesc;
    Ref<RenderPassEncoder> passEncoder = encoder->BeginRenderPass(&renderPassDesc);

    // Start pipeline  and encode commands to complete
    // the copy from src texture to dst texture with transformation.
    passEncoder->APISetPipeline(pipeline);
    passEncoder->APISetBindGroup(0, bindGroup.Get());
    passEncoder->APISetViewport(destination->origin.x, destination->origin.y, copySize->width,
                                copySize->height, 0.0, 1.0);
    passEncoder->APIDraw(3);
    passEncoder->APIEnd();

    // Finsh encoding.
    Ref<CommandBufferBase> commandBuffer;
    DAWN_TRY_ASSIGN(commandBuffer, encoder->Finish());
    CommandBufferBase* submitCommandBuffer = commandBuffer.Get();

    // Submit command buffer.
    device->GetQueue()->APISubmit(1, &submitCommandBuffer);
    return {};
}

}  // namespace dawn::native
