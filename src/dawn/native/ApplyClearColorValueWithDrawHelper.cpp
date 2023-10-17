// Copyright 2022 The Dawn & Tint Authors
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

#include "dawn/native/ApplyClearColorValueWithDrawHelper.h"

#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "dawn/native/BindGroup.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/Device.h"
#include "dawn/native/InternalPipelineStore.h"
#include "dawn/native/ObjectContentHasher.h"
#include "dawn/native/RenderPassEncoder.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/utils/WGPUHelpers.h"

namespace dawn::native {

namespace {

// General helper functions and data structures for applying clear values with draw
static const char kVSSource[] = R"(
@vertex
fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
    var pos = array(
        vec2f( 0.0, -1.0),
        vec2f( 1.0, -1.0),
        vec2f( 0.0,  1.0),
        vec2f( 0.0,  1.0),
        vec2f( 1.0, -1.0),
        vec2f( 1.0,  1.0));
        return vec4f(pos[VertexIndex], 0.0, 1.0);
})";

const char* GetTextureComponentTypeString(DeviceBase* device, wgpu::TextureFormat format) {
    DAWN_ASSERT(format != wgpu::TextureFormat::Undefined);

    const Format& formatInfo = device->GetValidInternalFormat(format);
    switch (formatInfo.GetAspectInfo(Aspect::Color).baseType) {
        case TextureComponentType::Sint:
            return "i32";
        case TextureComponentType::Uint:
            return "u32";
        case TextureComponentType::Float:
            break;
    }
    DAWN_UNREACHABLE();
}

// Construct the fragment shader to apply the input color values to the corresponding color
// attachments of KeyOfApplyClearColorValueWithDrawPipelines.
std::string ConstructFragmentShader(DeviceBase* device,
                                    const KeyOfApplyClearColorValueWithDrawPipelines& key) {
    std::ostringstream outputColorDeclarationStream;
    std::ostringstream clearValueUniformBufferDeclarationStream;
    std::ostringstream assignOutputColorStream;

    outputColorDeclarationStream << "struct OutputColor {" << std::endl;
    clearValueUniformBufferDeclarationStream << "struct ClearColors {" << std::endl;

    // Only generate the assignments we need.
    for (uint32_t i : IterateBitSet(key.colorTargetsToApplyClearColorValue)) {
        wgpu::TextureFormat currentFormat = key.colorTargetFormats[i];
        DAWN_ASSERT(currentFormat != wgpu::TextureFormat::Undefined);

        const char* type = GetTextureComponentTypeString(device, currentFormat);

        outputColorDeclarationStream << "@location(" << i << ") output" << i << " : vec4<" << type
                                     << ">," << std::endl;
        clearValueUniformBufferDeclarationStream << "color" << i << " : vec4<" << type << ">,"
                                                 << std::endl;
        assignOutputColorStream << "outputColor.output" << i << " = clearColors.color" << i << ";"
                                << std::endl;
    }
    outputColorDeclarationStream << "}" << std::endl;
    clearValueUniformBufferDeclarationStream << "}" << std::endl;

    std::ostringstream fragmentShaderStream;
    fragmentShaderStream << outputColorDeclarationStream.str()
                         << clearValueUniformBufferDeclarationStream.str() << R"(
@group(0) @binding(0) var<uniform> clearColors : ClearColors;

@fragment
fn main() -> OutputColor {
    var outputColor : OutputColor;
)" << assignOutputColorStream.str()
                         << R"(
return outputColor;
})";

    return fragmentShaderStream.str();
}

RenderPipelineBase* GetCachedPipeline(InternalPipelineStore* store,
                                      const KeyOfApplyClearColorValueWithDrawPipelines& key) {
    auto iter = store->applyClearColorValueWithDrawPipelines.find(key);
    if (iter != store->applyClearColorValueWithDrawPipelines.end()) {
        return iter->second.Get();
    }
    return nullptr;
}

ResultOrError<RenderPipelineBase*> GetOrCreateApplyClearValueWithDrawPipeline(
    DeviceBase* device,
    const KeyOfApplyClearColorValueWithDrawPipelines& key) {
    InternalPipelineStore* store = device->GetInternalPipelineStore();
    RenderPipelineBase* cachedPipeline = GetCachedPipeline(store, key);
    if (cachedPipeline != nullptr) {
        return cachedPipeline;
    }

    // Prepare the vertex stage
    Ref<ShaderModuleBase> vertexModule;
    DAWN_TRY_ASSIGN(vertexModule, utils::CreateShaderModule(device, kVSSource));
    VertexState vertex = {};
    vertex.module = vertexModule.Get();
    vertex.entryPoint = "main";

    // Prepare the fragment stage
    std::string fragmentShader = ConstructFragmentShader(device, key);
    Ref<ShaderModuleBase> fragmentModule;
    DAWN_TRY_ASSIGN(fragmentModule, utils::CreateShaderModule(device, fragmentShader.c_str()));
    FragmentState fragment = {};
    fragment.module = fragmentModule.Get();
    fragment.entryPoint = "main";

    // Prepare the color states
    std::array<ColorTargetState, kMaxColorAttachments> colorTargets = {};
    for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
        colorTargets[i].format = key.colorTargetFormats[i];
        // We shouldn't change the color targets that are not involved in.
        if (!key.colorTargetsToApplyClearColorValue[i]) {
            colorTargets[i].writeMask = wgpu::ColorWriteMask::None;
        }
    }

    // Create RenderPipeline
    RenderPipelineDescriptor renderPipelineDesc = {};

    renderPipelineDesc.vertex = vertex;
    renderPipelineDesc.fragment = &fragment;
    renderPipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    fragment.targetCount = key.colorAttachmentCount;
    fragment.targets = colorTargets.data();

    Ref<RenderPipelineBase> pipeline;
    DAWN_TRY_ASSIGN(pipeline, device->CreateRenderPipeline(&renderPipelineDesc));
    store->applyClearColorValueWithDrawPipelines.insert({key, std::move(pipeline)});

    return GetCachedPipeline(store, key);
}

Color GetClearColorValue(const RenderPassColorAttachment& attachment) {
    return attachment.clearValue;
}

ResultOrError<Ref<BufferBase>> CreateUniformBufferWithClearValues(
    DeviceBase* device,
    const RenderPassDescriptor* renderPassDescriptor,
    const KeyOfApplyClearColorValueWithDrawPipelines& key) {
    std::array<uint8_t, sizeof(uint32_t)* 4 * kMaxColorAttachments> clearValues = {};
    uint32_t offset = 0;
    for (uint32_t i : IterateBitSet(key.colorTargetsToApplyClearColorValue)) {
        const Format& format = renderPassDescriptor->colorAttachments[i].view->GetFormat();
        TextureComponentType baseType = format.GetAspectInfo(Aspect::Color).baseType;

        Color initialClearValue = GetClearColorValue(renderPassDescriptor->colorAttachments[i]);
        Color clearValue = ClampClearColorValueToLegalRange(initialClearValue, format);
        switch (baseType) {
            case TextureComponentType::Uint: {
                uint32_t* clearValuePtr = reinterpret_cast<uint32_t*>(clearValues.data() + offset);
                clearValuePtr[0] = static_cast<uint32_t>(clearValue.r);
                clearValuePtr[1] = static_cast<uint32_t>(clearValue.g);
                clearValuePtr[2] = static_cast<uint32_t>(clearValue.b);
                clearValuePtr[3] = static_cast<uint32_t>(clearValue.a);
                break;
            }
            case TextureComponentType::Sint: {
                int32_t* clearValuePtr = reinterpret_cast<int32_t*>(clearValues.data() + offset);
                clearValuePtr[0] = static_cast<int32_t>(clearValue.r);
                clearValuePtr[1] = static_cast<int32_t>(clearValue.g);
                clearValuePtr[2] = static_cast<int32_t>(clearValue.b);
                clearValuePtr[3] = static_cast<int32_t>(clearValue.a);
                break;
            }
            case TextureComponentType::Float: {
                float* clearValuePtr = reinterpret_cast<float*>(clearValues.data() + offset);
                clearValuePtr[0] = static_cast<float>(clearValue.r);
                clearValuePtr[1] = static_cast<float>(clearValue.g);
                clearValuePtr[2] = static_cast<float>(clearValue.b);
                clearValuePtr[3] = static_cast<float>(clearValue.a);
                break;
            }
        }
        offset += sizeof(uint32_t) * 4;
    }

    DAWN_ASSERT(offset > 0);

    Ref<BufferBase> outputBuffer;
    DAWN_TRY_ASSIGN(
        outputBuffer,
        utils::CreateBufferFromData(device, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform,
                                    clearValues.data(), offset));

    return std::move(outputBuffer);
}

// Helper functions for applying big integer clear values with draw
bool ShouldApplyClearBigIntegerColorValueWithDraw(
    const RenderPassColorAttachment& colorAttachmentInfo) {
    if (colorAttachmentInfo.view == nullptr) {
        return false;
    }

    if (colorAttachmentInfo.loadOp != wgpu::LoadOp::Clear) {
        return false;
    }

    // We should only apply this workaround on 32-bit signed and unsigned integer formats.
    const Format& format = colorAttachmentInfo.view->GetFormat();
    switch (format.format) {
        case wgpu::TextureFormat::R32Sint:
        case wgpu::TextureFormat::RG32Sint:
        case wgpu::TextureFormat::RGBA32Sint:
        case wgpu::TextureFormat::R32Uint:
        case wgpu::TextureFormat::RG32Uint:
        case wgpu::TextureFormat::RGBA32Uint:
            break;
        default:
            return false;
    }

    // TODO(dawn:537): only check the color channels that are available in the current color format.
    Color clearValue = GetClearColorValue(colorAttachmentInfo);
    switch (format.GetAspectInfo(Aspect::Color).baseType) {
        case TextureComponentType::Uint: {
            constexpr double kMaxUintRepresentableInFloat = 1 << std::numeric_limits<float>::digits;
            if (clearValue.r <= kMaxUintRepresentableInFloat &&
                clearValue.g <= kMaxUintRepresentableInFloat &&
                clearValue.b <= kMaxUintRepresentableInFloat &&
                clearValue.a <= kMaxUintRepresentableInFloat) {
                return false;
            }
            break;
        }
        case TextureComponentType::Sint: {
            constexpr double kMaxSintRepresentableInFloat = 1 << std::numeric_limits<float>::digits;
            constexpr double kMinSintRepresentableInFloat = -kMaxSintRepresentableInFloat;
            if (clearValue.r <= kMaxSintRepresentableInFloat &&
                clearValue.r >= kMinSintRepresentableInFloat &&
                clearValue.g <= kMaxSintRepresentableInFloat &&
                clearValue.g >= kMinSintRepresentableInFloat &&
                clearValue.b <= kMaxSintRepresentableInFloat &&
                clearValue.b >= kMinSintRepresentableInFloat &&
                clearValue.a <= kMaxSintRepresentableInFloat &&
                clearValue.a >= kMinSintRepresentableInFloat) {
                return false;
            }
            break;
        }
        case TextureComponentType::Float:
            DAWN_UNREACHABLE();
            return false;
    }

    return true;
}

KeyOfApplyClearColorValueWithDrawPipelines GetKeyOfApplyClearColorValueWithDrawPipelines(
    const RenderPassDescriptor* renderPassDescriptor) {
    KeyOfApplyClearColorValueWithDrawPipelines key;
    key.colorAttachmentCount = renderPassDescriptor->colorAttachmentCount;

    key.colorTargetFormats.fill(wgpu::TextureFormat::Undefined);
    for (uint32_t i = 0; i < renderPassDescriptor->colorAttachmentCount; ++i) {
        if (renderPassDescriptor->colorAttachments[i].view != nullptr) {
            key.colorTargetFormats[i] =
                renderPassDescriptor->colorAttachments[i].view->GetFormat().format;
        }

        if (ShouldApplyClearBigIntegerColorValueWithDraw(
                renderPassDescriptor->colorAttachments[i])) {
            key.colorTargetsToApplyClearColorValue.set(i);
        }
    }
    return key;
}

}  // namespace

size_t KeyOfApplyClearColorValueWithDrawPipelinesHashFunc::operator()(
    KeyOfApplyClearColorValueWithDrawPipelines key) const {
    size_t hash = 0;

    HashCombine(&hash, key.colorAttachmentCount);

    HashCombine(&hash, key.colorTargetsToApplyClearColorValue);

    for (wgpu::TextureFormat format : key.colorTargetFormats) {
        HashCombine(&hash, format);
    }

    return hash;
}

bool KeyOfApplyClearColorValueWithDrawPipelinesEqualityFunc::operator()(
    KeyOfApplyClearColorValueWithDrawPipelines key1,
    KeyOfApplyClearColorValueWithDrawPipelines key2) const {
    if (key1.colorAttachmentCount != key2.colorAttachmentCount) {
        return false;
    }

    if (key1.colorTargetsToApplyClearColorValue != key2.colorTargetsToApplyClearColorValue) {
        return false;
    }

    for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
        if (key1.colorTargetFormats[i] != key2.colorTargetFormats[i]) {
            return false;
        }
    }
    return true;
}

bool ShouldApplyClearBigIntegerColorValueWithDraw(
    const DeviceBase* device,
    const RenderPassDescriptor* renderPassDescriptor) {
    if (!device->IsToggleEnabled(Toggle::ApplyClearBigIntegerColorValueWithDraw)) {
        return false;
    }

    for (uint32_t i = 0; i < renderPassDescriptor->colorAttachmentCount; ++i) {
        if (ShouldApplyClearBigIntegerColorValueWithDraw(
                renderPassDescriptor->colorAttachments[i])) {
            return true;
        }
    }

    return false;
}

MaybeError ApplyClearBigIntegerColorValueWithDraw(
    RenderPassEncoder* renderPassEncoder,
    const RenderPassDescriptor* renderPassDescriptor) {
    DeviceBase* device = renderPassEncoder->GetDevice();

    KeyOfApplyClearColorValueWithDrawPipelines key =
        GetKeyOfApplyClearColorValueWithDrawPipelines(renderPassDescriptor);

    RenderPipelineBase* pipeline = nullptr;
    DAWN_TRY_ASSIGN(pipeline, GetOrCreateApplyClearValueWithDrawPipeline(device, key));

    Ref<BindGroupLayoutBase> layout;
    DAWN_TRY_ASSIGN(layout, pipeline->GetBindGroupLayout(0));

    Ref<BufferBase> uniformBufferWithClearColorValues;
    DAWN_TRY_ASSIGN(uniformBufferWithClearColorValues,
                    CreateUniformBufferWithClearValues(device, renderPassDescriptor, key));

    Ref<BindGroupBase> bindGroup;
    DAWN_TRY_ASSIGN(bindGroup,
                    utils::MakeBindGroup(device, layout, {{0, uniformBufferWithClearColorValues}},
                                         UsageValidationMode::Internal));

    renderPassEncoder->APISetBindGroup(0, bindGroup.Get());
    renderPassEncoder->APISetPipeline(pipeline);
    renderPassEncoder->APIDraw(6);

    return {};
}

}  // namespace dawn::native
