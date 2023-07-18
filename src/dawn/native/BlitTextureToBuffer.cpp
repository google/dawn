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

#include "dawn/native/BlitTextureToBuffer.h"

#include <string_view>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/ComputePassEncoder.h"
#include "dawn/native/ComputePipeline.h"
#include "dawn/native/Device.h"
#include "dawn/native/InternalPipelineStore.h"
#include "dawn/native/PhysicalDevice.h"
#include "dawn/native/Queue.h"
#include "dawn/native/utils/WGPUHelpers.h"

namespace dawn::native {

namespace {

constexpr uint32_t kWorkgroupSizeX = 8;
constexpr uint32_t kWorkgroupSizeY = 8;

// Helper to join constexpr std::string_view
template <std::string_view const&... Strs>
struct ConcatStringViewsImpl {
    // Join all strings into a single std::array of chars
    static constexpr auto impl() noexcept {
        constexpr std::size_t len = (Strs.size() + ... + 0);
        std::array<char, len + 1> a{};
        auto append = [i = 0, &a](auto const& s) mutable {
            for (auto c : s) {
                a[i++] = c;
            }
        };
        (append(Strs), ...);
        a[len] = 0;
        return a;
    }
    // Give the joined string static storage
    static constexpr auto arr = impl();
    // View as a std::string_view
    static constexpr std::string_view value{arr.data(), arr.size() - 1};
};
// Helper to get the value out
template <std::string_view const&... Strs>
static constexpr auto ConcatStringViews = ConcatStringViewsImpl<Strs...>::value;

constexpr std::string_view kFloatTexture1D = R"(
fn textureLoadGeneral(tex: texture_1d<f32>, coords: vec3u, level: u32) -> vec4<f32> {
    return textureLoad(tex, coords.x, level);
}
@group(0) @binding(0) var src_tex : texture_1d<f32>;
@group(0) @binding(1) var<storage, read_write> dst_buf : array<u32>;
)";

constexpr std::string_view kFloatTexture2D = R"(
fn textureLoadGeneral(tex: texture_2d<f32>, coords: vec3u, level: u32) -> vec4<f32> {
    return textureLoad(tex, coords.xy, level);
}
@group(0) @binding(0) var src_tex : texture_2d<f32>;
@group(0) @binding(1) var<storage, read_write> dst_buf : array<u32>;
)";

constexpr std::string_view kFloatTexture2DArray = R"(
fn textureLoadGeneral(tex: texture_2d_array<f32>, coords: vec3u, level: u32) -> vec4<f32> {
    return textureLoad(tex, coords.xy, coords.z, level);
}
@group(0) @binding(0) var src_tex : texture_2d_array<f32>;
@group(0) @binding(1) var<storage, read_write> dst_buf : array<u32>;
)";

constexpr std::string_view kFloatTexture3D = R"(
fn textureLoadGeneral(tex: texture_3d<f32>, coords: vec3u, level: u32) -> vec4<f32> {
    return textureLoad(tex, coords, level);
}
@group(0) @binding(0) var src_tex : texture_3d<f32>;
@group(0) @binding(1) var<storage, read_write> dst_buf : array<u32>;
)";

constexpr std::string_view kStencilTexture = R"(
fn textureLoadGeneral(tex: texture_2d<u32>, coords: vec3u, level: u32) -> vec4<u32> {
    return textureLoad(tex, coords.xy, level);
}
@group(0) @binding(0) var src_tex : texture_2d<u32>;
@group(0) @binding(1) var<storage, read_write> dst_buf : array<u32>;
)";

constexpr std::string_view kStencilTextureArray = R"(
fn textureLoadGeneral(tex: texture_2d_array<u32>, coords: vec3u, level: u32) -> vec4<u32> {
    return textureLoad(tex, coords.xy, coords.z, level);
}
@group(0) @binding(0) var src_tex : texture_2d_array<u32>;
@group(0) @binding(1) var<storage, read_write> dst_buf : array<u32>;
)";

constexpr std::string_view kDepthTexture = R"(
fn textureLoadGeneral(tex: texture_depth_2d, coords: vec3u, level: u32) -> f32 {
    return textureLoad(tex, coords.xy, level);
}
@group(0) @binding(0) var src_tex : texture_depth_2d;
@group(0) @binding(1) var<storage, read_write> dst_buf : array<u32>;
)";

constexpr std::string_view kDepthTextureArray = R"(
fn textureLoadGeneral(tex: texture_depth_2d_array, coords: vec3u, level: u32) -> f32 {
    return textureLoad(tex, coords.xy, coords.z, level);
}
@group(0) @binding(0) var src_tex : texture_depth_2d_array;
@group(0) @binding(1) var<storage, read_write> dst_buf : array<u32>;
)";

constexpr std::string_view kDepth32FloatTexture = R"(
fn textureLoadGeneral(tex: texture_depth_2d, coords: vec3u, level: u32) -> f32 {
    return textureLoad(tex, coords.xy, level);
}
@group(0) @binding(0) var src_tex : texture_depth_2d;
// Can directly use f32 for the buffer array data type
@group(0) @binding(1) var<storage, read_write> dst_buf : array<f32>;
)";

constexpr std::string_view kDepth32FloatTextureArray = R"(
fn textureLoadGeneral(tex: texture_depth_2d_array, coords: vec3u, level: u32) -> f32 {
    return textureLoad(tex, coords.xy, coords.z, level);
}
@group(0) @binding(0) var src_tex : texture_depth_2d_array;
// Can directly use f32 for the buffer array data type
@group(0) @binding(1) var<storage, read_write> dst_buf : array<f32>;
)";

constexpr std::string_view kCommon = R"(

struct Params {
    // copyExtent
    srcOrigin: vec3u,
    // How many texel values one thread needs to pack (1, 2, or 4)
    packTexelCount: u32,
    srcExtent: vec3u,
    pad1: u32,

    // GPUImageDataLayout
    indicesPerRow: u32,
    rowsPerImage: u32,
    indicesOffset: u32,
};

@group(0) @binding(2) var<uniform> params : Params;

override workgroupSizeX: u32;
override workgroupSizeY: u32;

// Load the texel value and write to storage buffer.
// Each thread is responsible for reading (packTexelCount) byte and packing them into a 4-byte u32.
@compute @workgroup_size(workgroupSizeX, workgroupSizeY, 1) fn main
(@builtin(global_invocation_id) id : vec3u) {
    let srcBoundary = params.srcOrigin + params.srcExtent;

    let coord0 = vec3u(id.x * params.packTexelCount, id.y, id.z) + params.srcOrigin;

    if (any(coord0 >= srcBoundary)) {
        return;
    }

    let dstOffset = params.indicesOffset + id.x + id.y * params.indicesPerRow + id.z * params.indicesPerRow * params.rowsPerImage;
)";

constexpr std::string_view kCommonEnd = R"(
    dst_buf[dstOffset] = result;
}
)";

constexpr std::string_view kPackStencil8ToU32 = R"(
    // Storing stencil8 texel values
    var result: u32 = 0xff & textureLoadGeneral(src_tex, coord0, 0).r;

    if (coord0.x + 4u <= srcBoundary.x) {
        // All 4 texels for this thread are within texture bounds.
        for (var i = 1u; i < 4u; i += 1u) {
            let coordi = coord0 + vec3u(i, 0, 0);
            let ri = 0xff & textureLoadGeneral(src_tex, coordi, 0).r;
            result |= ri << (i * 8u);
        }
    } else {
        // Otherwise, srcExtent.x is not a multiple of 4 and this thread is at right edge of the texture
        // To preserve the original buffer content, we need to read from the buffer and pack it together with other values.
        let original: u32 = dst_buf[dstOffset];
        result |= original & 0xffffff00;

        for (var i = 1u; i < 4u; i += 1u) {
            let coordi = coord0 + vec3u(i, 0, 0);
            if (coordi.x >= srcBoundary.x) {
                break;
            }
            let ri = 0xff & textureLoadGeneral(src_tex, coordi, 0).r;
            result |= ri << (i * 8u);
        }
    }
)";

constexpr std::string_view kPackR8SnormToU32 = R"(
    // Result bits to store into dst_buf
    var result: u32 = 0u;
    // Storing snorm8 texel values
    // later called by pack4x8snorm to convert to u32.
    var v: vec4<f32>;
    v[0] = textureLoadGeneral(src_tex, coord0, 0).r;

    if (coord0.x + 4u <= srcBoundary.x) {
        // All 4 texels for this thread are within texture bounds.
        for (var i = 1u; i < 4u; i += 1u) {
            let coordi = coord0 + vec3u(i, 0, 0);
            v[i] = textureLoadGeneral(src_tex, coordi, 0).r;
        }
        result = pack4x8snorm(v);
    } else {
        // Otherwise, srcExtent.x is not a multiple of 4 and this thread is at right edge of the texture
        // To preserve the original buffer content, we need to read from the buffer and pack it together with other values.
        let original: u32 = dst_buf[dstOffset];

        var i = 1u;
        for (; i < 4u; i += 1u) {
            let coordi = coord0 + vec3u(i, 0, 0);
            if (coordi.x >= srcBoundary.x) {
                break;
            }
            v[i] = textureLoadGeneral(src_tex, coordi, 0).r;
        }
        let mask: u32 = 0xffffffffu << (i * 8u);

        result = (original & mask) | (pack4x8snorm(v) & ~mask);
    }
)";

constexpr std::string_view kPackRG8SnormToU32 = R"(
    // Result bits to store into dst_buf
    var result: u32 = 0u;
    // Storing snorm8 texel values
    // later called by pack4x8snorm to convert to u32.
    var v: vec4<f32>;
    let texel0 = textureLoadGeneral(src_tex, coord0, 0).rg;
    v[0] = texel0.r;
    v[1] = texel0.g;

    let coord1 = coord0 + vec3u(1, 0, 0);
    if (coord1.x < srcBoundary.x) {
        // Make sure coord1 is still within the copy boundary.
        let texel1 = textureLoadGeneral(src_tex, coord1, 0).rg;
        v[2] = texel1.r;
        v[3] = texel1.g;
        result = pack4x8snorm(v);
    } else {
        // Otherwise, srcExtent.x is not a multiple of 2 and this thread is at right edge of the texture
        // To preserve the original buffer content, we need to read from the buffer and pack it together with other values.
        let original: u32 = dst_buf[dstOffset];
        let mask = 0xffff0000u;
        result = (original & mask) | (pack4x8snorm(v) & ~mask);
    }
)";

// ShaderF16 extension is only enabled by GL_AMD_gpu_shader_half_float for GL
// so we should not use it generally for the emulation.
// As a result we are using f32 and array<u32> to do all the math and byte manipulation.
// If we have 2-byte scalar type (f16, u16) it can be a bit easier when writing to the storage
// buffer.

constexpr std::string_view kPackDepth16UnormToU32 = R"(
    // Result bits to store into dst_buf
    var result: u32 = 0u;
    // Storing depth16unorm texel values
    // later called by pack2x16unorm to convert to u32.
    var v: vec2<f32>;
    v[0] = textureLoadGeneral(src_tex, coord0, 0);

    let coord1 = coord0 + vec3u(1, 0, 0);
    if (coord1.x < srcBoundary.x) {
        // Make sure coord1 is still within the copy boundary.
        v[1] = textureLoadGeneral(src_tex, coord1, 0);
        result = pack2x16unorm(v);
    } else {
        // Otherwise, srcExtent.x is not a multiple of 2 and this thread is at right edge of the texture
        // To preserve the original buffer content, we need to read from the buffer and pack it together with other values.
        // TODO(dawn:1782): profiling against making a separate pass for this edge case
        // as it requires reading from dst_buf.
        let original: u32 = dst_buf[dstOffset];
        let mask = 0xffff0000u;
        result = (original & mask) | (pack2x16unorm(v) & ~mask);
    }
)";

constexpr std::string_view kPackRGBA8SnormToU32 = R"(
    // Storing snorm8 texel values
    // later called by pack4x8snorm to convert to u32.
    var v: vec4<f32>;

    let texel0 = textureLoadGeneral(src_tex, coord0, 0);
    v[0] = texel0.r;
    v[1] = texel0.g;
    v[2] = texel0.b;
    v[3] = texel0.a;

    let result: u32 = pack4x8snorm(v);
)";

constexpr std::string_view kPackBGRA8UnormToU32 = R"(
    // Storing and swizzling bgra8unorm texel values
    // later called by pack4x8unorm to convert to u32.
    var v: vec4<f32>;

    let texel0 = textureLoadGeneral(src_tex, coord0, 0);
    v = texel0.bgra;

    let result: u32 = pack4x8unorm(v);
)";

constexpr std::string_view kLoadDepth32Float = R"(
    dst_buf[dstOffset] = textureLoadGeneral(src_tex, coord0, 0);
}
)";

constexpr std::string_view kBlitR8Snorm1D =
    ConcatStringViews<kFloatTexture1D, kCommon, kPackR8SnormToU32, kCommonEnd>;
constexpr std::string_view kBlitRG8Snorm1D =
    ConcatStringViews<kFloatTexture1D, kCommon, kPackRG8SnormToU32, kCommonEnd>;
constexpr std::string_view kBlitRGBA8Snorm1D =
    ConcatStringViews<kFloatTexture1D, kCommon, kPackRGBA8SnormToU32, kCommonEnd>;

constexpr std::string_view kBlitR8Snorm2D =
    ConcatStringViews<kFloatTexture2D, kCommon, kPackR8SnormToU32, kCommonEnd>;
constexpr std::string_view kBlitRG8Snorm2D =
    ConcatStringViews<kFloatTexture2D, kCommon, kPackRG8SnormToU32, kCommonEnd>;
constexpr std::string_view kBlitRGBA8Snorm2D =
    ConcatStringViews<kFloatTexture2D, kCommon, kPackRGBA8SnormToU32, kCommonEnd>;

constexpr std::string_view kBlitR8Snorm2DArray =
    ConcatStringViews<kFloatTexture2DArray, kCommon, kPackR8SnormToU32, kCommonEnd>;
constexpr std::string_view kBlitRG8Snorm2DArray =
    ConcatStringViews<kFloatTexture2DArray, kCommon, kPackRG8SnormToU32, kCommonEnd>;
constexpr std::string_view kBlitRGBA8Snorm2DArray =
    ConcatStringViews<kFloatTexture2DArray, kCommon, kPackRGBA8SnormToU32, kCommonEnd>;

constexpr std::string_view kBlitR8Snorm3D =
    ConcatStringViews<kFloatTexture3D, kCommon, kPackR8SnormToU32, kCommonEnd>;
constexpr std::string_view kBlitRG8Snorm3D =
    ConcatStringViews<kFloatTexture3D, kCommon, kPackRG8SnormToU32, kCommonEnd>;
constexpr std::string_view kBlitRGBA8Snorm3D =
    ConcatStringViews<kFloatTexture3D, kCommon, kPackRGBA8SnormToU32, kCommonEnd>;

constexpr std::string_view kBlitBGRA8Unorm1D =
    ConcatStringViews<kFloatTexture1D, kCommon, kPackBGRA8UnormToU32, kCommonEnd>;
constexpr std::string_view kBlitBGRA8Unorm2D =
    ConcatStringViews<kFloatTexture2D, kCommon, kPackBGRA8UnormToU32, kCommonEnd>;
constexpr std::string_view kBlitBGRA8Unorm2DArray =
    ConcatStringViews<kFloatTexture2DArray, kCommon, kPackBGRA8UnormToU32, kCommonEnd>;
constexpr std::string_view kBlitBGRA8Unorm3D =
    ConcatStringViews<kFloatTexture3D, kCommon, kPackBGRA8UnormToU32, kCommonEnd>;

constexpr std::string_view kBlitStencil8 =
    ConcatStringViews<kStencilTexture, kCommon, kPackStencil8ToU32, kCommonEnd>;
constexpr std::string_view kBlitStencil8Array =
    ConcatStringViews<kStencilTextureArray, kCommon, kPackStencil8ToU32, kCommonEnd>;

constexpr std::string_view kBlitDepth16Unorm =
    ConcatStringViews<kDepthTexture, kCommon, kPackDepth16UnormToU32, kCommonEnd>;
constexpr std::string_view kBlitDepth16UnormArray =
    ConcatStringViews<kDepthTextureArray, kCommon, kPackDepth16UnormToU32, kCommonEnd>;
constexpr std::string_view kBlitDepth32Float =
    ConcatStringViews<kDepth32FloatTexture, kCommon, kLoadDepth32Float>;
constexpr std::string_view kBlitDepth32FloatArray =
    ConcatStringViews<kDepth32FloatTextureArray, kCommon, kLoadDepth32Float>;

ResultOrError<Ref<ComputePipelineBase>> GetOrCreateTextureToBufferPipeline(
    DeviceBase* device,
    const TextureCopy& src,
    wgpu::TextureViewDimension viewDimension) {
    InternalPipelineStore* store = device->GetInternalPipelineStore();

    const Format& format = src.texture->GetFormat();

    auto iter = store->blitTextureToBufferComputePipelines.find({format.format, viewDimension});
    if (iter != store->blitTextureToBufferComputePipelines.end()) {
        return iter->second;
    }

    ShaderModuleWGSLDescriptor wgslDesc = {};
    ShaderModuleDescriptor shaderModuleDesc = {};
    shaderModuleDesc.nextInChain = &wgslDesc;

    wgpu::TextureSampleType textureSampleType;
    switch (format.format) {
        case wgpu::TextureFormat::R8Snorm:
            switch (viewDimension) {
                case wgpu::TextureViewDimension::e1D:
                    wgslDesc.code = kBlitR8Snorm1D.data();
                    break;
                case wgpu::TextureViewDimension::e2D:
                    wgslDesc.code = kBlitR8Snorm2D.data();
                    break;
                case wgpu::TextureViewDimension::e2DArray:
                    wgslDesc.code = kBlitR8Snorm2DArray.data();
                    break;
                case wgpu::TextureViewDimension::e3D:
                    wgslDesc.code = kBlitR8Snorm3D.data();
                    break;
                default:
                    UNREACHABLE();
            }
            textureSampleType = wgpu::TextureSampleType::Float;
            break;
        case wgpu::TextureFormat::RG8Snorm:
            switch (viewDimension) {
                case wgpu::TextureViewDimension::e1D:
                    wgslDesc.code = kBlitRG8Snorm1D.data();
                    break;
                case wgpu::TextureViewDimension::e2D:
                    wgslDesc.code = kBlitRG8Snorm2D.data();
                    break;
                case wgpu::TextureViewDimension::e2DArray:
                    wgslDesc.code = kBlitRG8Snorm2DArray.data();
                    break;
                case wgpu::TextureViewDimension::e3D:
                    wgslDesc.code = kBlitRG8Snorm3D.data();
                    break;
                default:
                    UNREACHABLE();
            }
            textureSampleType = wgpu::TextureSampleType::Float;
            break;
        case wgpu::TextureFormat::RGBA8Snorm:
            switch (viewDimension) {
                case wgpu::TextureViewDimension::e1D:
                    wgslDesc.code = kBlitRGBA8Snorm1D.data();
                    break;
                case wgpu::TextureViewDimension::e2D:
                    wgslDesc.code = kBlitRGBA8Snorm2D.data();
                    break;
                case wgpu::TextureViewDimension::e2DArray:
                    wgslDesc.code = kBlitRGBA8Snorm2DArray.data();
                    break;
                case wgpu::TextureViewDimension::e3D:
                    wgslDesc.code = kBlitRGBA8Snorm3D.data();
                    break;
                default:
                    UNREACHABLE();
            }
            textureSampleType = wgpu::TextureSampleType::Float;
            break;
        case wgpu::TextureFormat::BGRA8Unorm:
            switch (viewDimension) {
                case wgpu::TextureViewDimension::e1D:
                    wgslDesc.code = kBlitBGRA8Unorm1D.data();
                    break;
                case wgpu::TextureViewDimension::e2D:
                    wgslDesc.code = kBlitBGRA8Unorm2D.data();
                    break;
                case wgpu::TextureViewDimension::e2DArray:
                    wgslDesc.code = kBlitBGRA8Unorm2DArray.data();
                    break;
                case wgpu::TextureViewDimension::e3D:
                    wgslDesc.code = kBlitBGRA8Unorm3D.data();
                    break;
                default:
                    UNREACHABLE();
            }
            textureSampleType = wgpu::TextureSampleType::Float;
            break;
        case wgpu::TextureFormat::Depth16Unorm:
            switch (viewDimension) {
                case wgpu::TextureViewDimension::e2D:
                    wgslDesc.code = kBlitDepth16Unorm.data();
                    break;
                case wgpu::TextureViewDimension::e2DArray:
                    wgslDesc.code = kBlitDepth16UnormArray.data();
                    break;
                default:
                    UNREACHABLE();
            }
            textureSampleType = wgpu::TextureSampleType::Depth;
            break;
        case wgpu::TextureFormat::Depth32Float:
            switch (viewDimension) {
                case wgpu::TextureViewDimension::e2D:
                    wgslDesc.code = kBlitDepth32Float.data();
                    break;
                case wgpu::TextureViewDimension::e2DArray:
                    wgslDesc.code = kBlitDepth32FloatArray.data();
                    break;
                default:
                    UNREACHABLE();
            }
            textureSampleType = wgpu::TextureSampleType::Depth;
            break;
        case wgpu::TextureFormat::Stencil8:
        case wgpu::TextureFormat::Depth24PlusStencil8:
            // Depth24PlusStencil8 can only copy with stencil aspect and is gated by validation.
            switch (viewDimension) {
                case wgpu::TextureViewDimension::e2D:
                    wgslDesc.code = kBlitStencil8.data();
                    break;
                case wgpu::TextureViewDimension::e2DArray:
                    wgslDesc.code = kBlitStencil8Array.data();
                    break;
                default:
                    UNREACHABLE();
            }
            textureSampleType = wgpu::TextureSampleType::Uint;
            break;
        case wgpu::TextureFormat::Depth32FloatStencil8: {
            // Depth32FloatStencil8 is not supported on OpenGL/OpenGLES where the blit path is
            // enabled by default. But could be hit if the blit path toggle is manually set on other
            // backends.
            DAWN_ASSERT(viewDimension == wgpu::TextureViewDimension::e2D ||
                        viewDimension == wgpu::TextureViewDimension::e2DArray);
            bool is2DArray = viewDimension == wgpu::TextureViewDimension::e2DArray;
            switch (src.aspect) {
                case Aspect::Depth:
                    wgslDesc.code =
                        is2DArray ? kBlitDepth32FloatArray.data() : kBlitDepth32Float.data();
                    textureSampleType = wgpu::TextureSampleType::Depth;
                    break;
                case Aspect::Stencil:
                    wgslDesc.code = is2DArray ? kBlitStencil8Array.data() : kBlitStencil8.data();
                    textureSampleType = wgpu::TextureSampleType::Uint;
                    break;
                default:
                    UNREACHABLE();
            }
        } break;
        default:
            UNREACHABLE();
    }

    Ref<ShaderModuleBase> shaderModule;
    DAWN_TRY_ASSIGN(shaderModule, device->CreateShaderModule(&shaderModuleDesc));

    Ref<BindGroupLayoutBase> bindGroupLayout;
    DAWN_TRY_ASSIGN(bindGroupLayout,
                    utils::MakeBindGroupLayout(
                        device,
                        {
                            {0, wgpu::ShaderStage::Compute, textureSampleType, viewDimension},
                            {1, wgpu::ShaderStage::Compute, kInternalStorageBufferBinding},
                            {2, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform},
                        },
                        /* allowInternalBinding */ true));

    Ref<PipelineLayoutBase> pipelineLayout;
    DAWN_TRY_ASSIGN(pipelineLayout, utils::MakeBasicPipelineLayout(device, bindGroupLayout));

    ComputePipelineDescriptor computePipelineDescriptor = {};
    computePipelineDescriptor.layout = pipelineLayout.Get();
    computePipelineDescriptor.compute.module = shaderModule.Get();
    computePipelineDescriptor.compute.entryPoint = "main";

    const uint32_t adjustedWorkGroupSizeY =
        (viewDimension == wgpu::TextureViewDimension::e1D) ? 1 : kWorkgroupSizeY;
    const std::array<ConstantEntry, 2> constants = {{
        {nullptr, "workgroupSizeX", kWorkgroupSizeX},
        {nullptr, "workgroupSizeY", static_cast<double>(adjustedWorkGroupSizeY)},
    }};
    computePipelineDescriptor.compute.constantCount = constants.size();
    computePipelineDescriptor.compute.constants = constants.data();

    Ref<ComputePipelineBase> pipeline;
    DAWN_TRY_ASSIGN(pipeline, device->CreateComputePipeline(&computePipelineDescriptor));
    store->blitTextureToBufferComputePipelines.insert(
        {std::make_pair(format.format, viewDimension), pipeline});
    return pipeline;
}

}  // anonymous namespace

MaybeError BlitTextureToBuffer(DeviceBase* device,
                               CommandEncoder* commandEncoder,
                               const TextureCopy& src,
                               const BufferCopy& dst,
                               const Extent3D& copyExtent) {
    wgpu::TextureViewDimension textureViewDimension;
    {
        wgpu::TextureDimension dimension = src.texture->GetDimension();
        switch (dimension) {
            case wgpu::TextureDimension::e1D:
                textureViewDimension = wgpu::TextureViewDimension::e1D;
                break;
            case wgpu::TextureDimension::e2D:
                if (src.texture->GetArrayLayers() > 1) {
                    textureViewDimension = wgpu::TextureViewDimension::e2DArray;
                } else {
                    textureViewDimension = wgpu::TextureViewDimension::e2D;
                }
                break;
            case wgpu::TextureDimension::e3D:
                textureViewDimension = wgpu::TextureViewDimension::e3D;
                break;
        }
    }

    Ref<ComputePipelineBase> pipeline;
    DAWN_TRY_ASSIGN(pipeline,
                    GetOrCreateTextureToBufferPipeline(device, src, textureViewDimension));

    const Format& format = src.texture->GetFormat();

    uint32_t texelFormatByteSize = format.GetAspectInfo(src.aspect).block.byteSize;
    uint32_t workgroupCountX = 1;
    uint32_t workgroupCountY = (textureViewDimension == wgpu::TextureViewDimension::e1D)
                                   ? 1
                                   : (copyExtent.height + kWorkgroupSizeY - 1) / kWorkgroupSizeY;
    uint32_t workgroupCountZ = copyExtent.depthOrArrayLayers;
    switch (texelFormatByteSize) {
        case 1:
            // One thread is responsible for writing four texel values (x, y) ~ (x+3, y).
            workgroupCountX = (copyExtent.width + 4 * kWorkgroupSizeX - 1) / (4 * kWorkgroupSizeX);
            break;
        case 2:
            // One thread is responsible for writing two texel values (x, y) and (x+1, y).
            workgroupCountX = (copyExtent.width + 2 * kWorkgroupSizeX - 1) / (2 * kWorkgroupSizeX);
            break;
        case 4:
            workgroupCountX = (copyExtent.width + kWorkgroupSizeX - 1) / kWorkgroupSizeX;
            break;
        default:
            UNREACHABLE();
    }

    Ref<BufferBase> destinationBuffer = dst.buffer;
    bool useIntermediateCopyBuffer = false;
    if (texelFormatByteSize < 4 && dst.buffer->GetSize() % 4 != 0 &&
        copyExtent.width % (4 / texelFormatByteSize) != 0) {
        // This path is made for OpenGL/GLES bliting a texture with an width % (4 / texelByteSize)
        // != 0, to a compact buffer. When we copy the last texel, we inevitably need to access an
        // out of bounds location given by dst.buffer.size as we use array<u32> in the shader for
        // the storage buffer. Although the allocated size of dst.buffer is aligned to 4 bytes for
        // OpenGL/GLES backend, the size of the storage buffer binding for the shader is not. Thus
        // we make an intermediate buffer aligned to 4 bytes for the compute shader to safely
        // access, and perform an additional buffer to buffer copy at the end. This path should be
        // hit rarely.
        useIntermediateCopyBuffer = true;
        BufferDescriptor descriptor = {};
        descriptor.size = Align(dst.buffer->GetSize(), 4);
        // TODO(dawn:1485): adding CopyDst usage to add kInternalStorageBuffer usage internally.
        descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        DAWN_TRY_ASSIGN(destinationBuffer, device->CreateBuffer(&descriptor));
    }

    // Allow internal usages since we need to use the source as a texture binding
    // and buffer as a storage binding.
    auto scope = commandEncoder->MakeInternalUsageScope();

    Ref<BindGroupLayoutBase> bindGroupLayout;
    DAWN_TRY_ASSIGN(bindGroupLayout, pipeline->GetBindGroupLayout(0));

    Ref<BufferBase> uniformBuffer;
    {
        BufferDescriptor bufferDesc = {};
        // Uniform buffer size needs to be multiple of 16 bytes
        bufferDesc.size = sizeof(uint32_t) * 12;
        bufferDesc.usage = wgpu::BufferUsage::Uniform;
        bufferDesc.mappedAtCreation = true;
        DAWN_TRY_ASSIGN(uniformBuffer, device->CreateBuffer(&bufferDesc));

        uint32_t* params =
            static_cast<uint32_t*>(uniformBuffer->GetMappedRange(0, bufferDesc.size));
        // srcOrigin: vec3u
        params[0] = src.origin.x;
        params[1] = src.origin.y;
        if (textureViewDimension == wgpu::TextureViewDimension::e2DArray) {
            // src.origin.z is set at textureView.baseArrayLayer
            params[2] = 0;
        } else {
            params[2] = src.origin.z;
        }

        // packTexelCount: number of texel values (1, 2, or 4) one thread packs into the dst buffer
        params[3] = 4 / texelFormatByteSize;
        // srcExtent: vec3u
        params[4] = copyExtent.width;
        params[5] = copyExtent.height;
        params[6] = copyExtent.depthOrArrayLayers;

        params[7] = src.mipLevel;

        // Turn bytesPerRow, (bytes)offset to use array index as unit
        // We pack values into array<u32> or array<f32>
        params[8] = dst.bytesPerRow / 4;
        params[9] = dst.rowsPerImage;
        params[10] = dst.offset / 4;

        DAWN_TRY(uniformBuffer->Unmap());
    }

    TextureViewDescriptor viewDesc = {};
    switch (src.aspect) {
        case Aspect::Color:
            viewDesc.aspect = wgpu::TextureAspect::All;
            break;
        case Aspect::Depth:
            viewDesc.aspect = wgpu::TextureAspect::DepthOnly;
            break;
        case Aspect::Stencil:
            viewDesc.aspect = wgpu::TextureAspect::StencilOnly;
            break;
        default:
            UNREACHABLE();
    }

    viewDesc.dimension = textureViewDimension;
    viewDesc.baseMipLevel = src.mipLevel;
    viewDesc.mipLevelCount = 1;
    if (viewDesc.dimension == wgpu::TextureViewDimension::e2DArray) {
        viewDesc.baseArrayLayer = src.origin.z;
        viewDesc.arrayLayerCount = copyExtent.depthOrArrayLayers;
    } else {
        viewDesc.baseArrayLayer = 0;
        viewDesc.arrayLayerCount = 1;
    }

    Ref<TextureViewBase> srcView;
    DAWN_TRY_ASSIGN(srcView, src.texture->CreateView(&viewDesc));

    Ref<BindGroupBase> bindGroup;
    DAWN_TRY_ASSIGN(bindGroup, utils::MakeBindGroup(device, bindGroupLayout,
                                                    {
                                                        {0, srcView},
                                                        {1, destinationBuffer},
                                                        {2, uniformBuffer},
                                                    },
                                                    UsageValidationMode::Internal));

    Ref<ComputePassEncoder> pass = commandEncoder->BeginComputePass();
    pass->APISetPipeline(pipeline.Get());
    pass->APISetBindGroup(0, bindGroup.Get());
    pass->APIDispatchWorkgroups(workgroupCountX, workgroupCountY, workgroupCountZ);
    pass->APIEnd();

    if (useIntermediateCopyBuffer) {
        ASSERT(destinationBuffer->GetSize() <= dst.buffer->GetAllocatedSize());
        commandEncoder->InternalCopyBufferToBufferWithAllocatedSize(
            destinationBuffer.Get(), 0, dst.buffer.Get(), 0, destinationBuffer->GetSize());
    }

    return {};
}

}  // namespace dawn::native
