// Copyright 2023 The Dawn & Tint Authors
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

#include "dawn/native/BlitTextureToBuffer.h"

#include <string>
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
        const mask = 0xffff0000u;
        result = (original & mask) | (pack2x16unorm(v) & ~mask);
    }
)";

// Storing snorm8 texel values
// later called by pack4x8snorm to convert to u32.
constexpr std::string_view kPackRGBA8SnormToU32 = R"(
    let v = textureLoadGeneral(src_tex, coord0, 0);
    let result: u32 = pack4x8snorm(v);
)";

// Storing and swizzling bgra8unorm texel values
// later called by pack4x8unorm to convert to u32.
constexpr std::string_view kPackBGRA8UnormToU32 = R"(
    var v: vec4<f32>;

    let texel0 = textureLoadGeneral(src_tex, coord0, 0);
    v = texel0.bgra;

    let result: u32 = pack4x8unorm(v);
)";

// Storing rgb9e5ufloat texel values
// In this format float is represented as
// 2^(exponent - bias) * (mantissa / 2^numMantissaBits)
// Packing algorithm is from:
// https://registry.khronos.org/OpenGL/extensions/EXT/EXT_texture_shared_exponent.txt
//
// Note: there are multiple bytes that could represent the same value in this format.
// e.g.
// 0x0a090807 and 0x0412100e both unpack to
// [8.344650268554688e-7, 0.000015735626220703125, 0.000015497207641601562]
// So the bytes copied via blit could be different.
constexpr std::string_view kPackRGB9E5UfloatToU32 = R"(
    let v = textureLoadGeneral(src_tex, coord0, 0);

    const n = 9; // number of mantissa bits
    const e_max = 31; // max exponent
    const b = 15; // exponent bias
    const sharedexp_max: f32 = (f32((1 << n) - 1) / f32(1 << n)) * (1 << (e_max - b));

    let red_c = clamp(v.r, 0.0, sharedexp_max);
    let green_c = clamp(v.g, 0.0, sharedexp_max);
    let blue_c = clamp(v.b, 0.0, sharedexp_max);

    let max_c = max(max(red_c, green_c), blue_c);
    let exp_shared_p: i32 = max(-b - 1, i32(floor(log2(max_c)))) + 1 + b;
    let max_s = u32(floor(max_c / exp2(f32(exp_shared_p - b - n)) + 0.5));
    var exp_shared = exp_shared_p;
    if (max_s == (1 << n)) {
        exp_shared += 1;
    }

    let scalar = 1.0 / exp2(f32(exp_shared - b - n));
    let red_s = u32(red_c * scalar + 0.5);
    let green_s = u32(green_c * scalar + 0.5);
    let blue_s = u32(blue_c * scalar + 0.5);

    const mask_9 = 0x1ffu;
    let result = (u32(exp_shared) << 27u) |
        ((blue_s & mask_9) << 18u) |
        ((green_s & mask_9) << 9u) |
        (red_s & mask_9);
)";

// Directly loading depth32float values into dst_buf
// No bit manipulation and packing is needed.
constexpr std::string_view kLoadDepth32Float = R"(
    dst_buf[dstOffset] = textureLoadGeneral(src_tex, coord0, 0);
}
)";

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
    std::string shader;

    auto AppendFloatTextureHead = [&]() {
        switch (viewDimension) {
            case wgpu::TextureViewDimension::e1D:
                shader += kFloatTexture1D;
                break;
            case wgpu::TextureViewDimension::e2D:
                shader += kFloatTexture2D;
                break;
            case wgpu::TextureViewDimension::e2DArray:
                shader += kFloatTexture2DArray;
                break;
            case wgpu::TextureViewDimension::e3D:
                shader += kFloatTexture3D;
                break;
            default:
                DAWN_UNREACHABLE();
        }
    };
    auto AppendDepthTextureHead = [&]() {
        switch (viewDimension) {
            case wgpu::TextureViewDimension::e2D:
                shader += kDepthTexture;
                break;
            case wgpu::TextureViewDimension::e2DArray:
                shader += kDepthTextureArray;
                break;
            default:
                DAWN_UNREACHABLE();
        }
    };
    auto AppendDepth32FloatTextureHead = [&]() {
        switch (viewDimension) {
            case wgpu::TextureViewDimension::e2D:
                shader += kDepth32FloatTexture;
                break;
            case wgpu::TextureViewDimension::e2DArray:
                shader += kDepth32FloatTextureArray;
                break;
            default:
                DAWN_UNREACHABLE();
        }
    };
    auto AppendStencilTextureHead = [&]() {
        switch (viewDimension) {
            case wgpu::TextureViewDimension::e2D:
                shader += kStencilTexture;
                break;
            case wgpu::TextureViewDimension::e2DArray:
                shader += kStencilTextureArray;
                break;
            default:
                DAWN_UNREACHABLE();
        }
    };

    switch (format.format) {
        case wgpu::TextureFormat::R8Snorm:
            AppendFloatTextureHead();
            shader += kCommon;
            shader += kPackR8SnormToU32;
            shader += kCommonEnd;
            textureSampleType = wgpu::TextureSampleType::Float;
            break;
        case wgpu::TextureFormat::RG8Snorm:
            AppendFloatTextureHead();
            shader += kCommon;
            shader += kPackRG8SnormToU32;
            shader += kCommonEnd;
            textureSampleType = wgpu::TextureSampleType::Float;
            break;
        case wgpu::TextureFormat::RGBA8Snorm:
            AppendFloatTextureHead();
            shader += kCommon;
            shader += kPackRGBA8SnormToU32;
            shader += kCommonEnd;
            textureSampleType = wgpu::TextureSampleType::Float;
            break;
        case wgpu::TextureFormat::BGRA8Unorm:
            AppendFloatTextureHead();
            shader += kCommon;
            shader += kPackBGRA8UnormToU32;
            shader += kCommonEnd;
            textureSampleType = wgpu::TextureSampleType::Float;
            break;
        case wgpu::TextureFormat::RGB9E5Ufloat:
            AppendFloatTextureHead();
            shader += kCommon;
            shader += kPackRGB9E5UfloatToU32;
            shader += kCommonEnd;
            textureSampleType = wgpu::TextureSampleType::Float;
            break;
        case wgpu::TextureFormat::Depth16Unorm:
            AppendDepthTextureHead();
            shader += kCommon;
            shader += kPackDepth16UnormToU32;
            shader += kCommonEnd;
            textureSampleType = wgpu::TextureSampleType::Depth;
            break;
        case wgpu::TextureFormat::Depth32Float:
            AppendDepth32FloatTextureHead();
            shader += kCommon;
            shader += kLoadDepth32Float;
            textureSampleType = wgpu::TextureSampleType::Depth;
            break;
        case wgpu::TextureFormat::Stencil8:
        case wgpu::TextureFormat::Depth24PlusStencil8:
            // Depth24PlusStencil8 can only copy with stencil aspect and is gated by validation.
            AppendStencilTextureHead();
            shader += kCommon;
            shader += kPackStencil8ToU32;
            shader += kCommonEnd;
            textureSampleType = wgpu::TextureSampleType::Uint;
            break;
        case wgpu::TextureFormat::Depth32FloatStencil8: {
            // Depth32FloatStencil8 is not supported on OpenGL/OpenGLES where the blit path is
            // enabled by default. But could be hit if the blit path toggle is manually set on other
            // backends.
            switch (src.aspect) {
                case Aspect::Depth:
                    AppendDepth32FloatTextureHead();
                    shader += kCommon;
                    shader += kLoadDepth32Float;
                    textureSampleType = wgpu::TextureSampleType::Depth;
                    break;
                case Aspect::Stencil:
                    AppendStencilTextureHead();
                    shader += kCommon;
                    shader += kPackStencil8ToU32;
                    shader += kCommonEnd;
                    textureSampleType = wgpu::TextureSampleType::Uint;
                    break;
                default:
                    DAWN_UNREACHABLE();
            }
            break;
        }
        default:
            DAWN_UNREACHABLE();
    }

    wgslDesc.code = shader.c_str();

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
            DAWN_UNREACHABLE();
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
            DAWN_UNREACHABLE();
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
        DAWN_ASSERT(destinationBuffer->GetSize() <= dst.buffer->GetAllocatedSize());
        commandEncoder->InternalCopyBufferToBufferWithAllocatedSize(
            destinationBuffer.Get(), 0, dst.buffer.Get(), 0, destinationBuffer->GetSize());
    }

    return {};
}

}  // namespace dawn::native
