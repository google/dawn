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

#include "dawn/native/BlitColorToColorWithDraw.h"

#include <sstream>
#include <string>
#include <utility>

#include "absl/container/inlined_vector.h"
#include "dawn/common/Assert.h"
#include "dawn/common/Enumerator.h"
#include "dawn/common/HashUtils.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/Device.h"
#include "dawn/native/InternalPipelineStore.h"
#include "dawn/native/RenderPassEncoder.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/utils/WGPUHelpers.h"
#include "dawn/native/webgpu_absl_format.h"

namespace dawn::native {

namespace {

constexpr char kBlitToColorVS[] = R"(

@vertex fn vert_fullscreen_quad(
  @builtin(vertex_index) vertex_index : u32,
) -> @builtin(position) vec4f {
  const pos = array(
      vec2f(-1.0, -1.0),
      vec2f( 3.0, -1.0),
      vec2f(-1.0,  3.0));
  return vec4f(pos[vertex_index], 0.0, 1.0);
}
)";

std::string GenerateExpandFS(const BlitColorToColorWithDrawPipelineKey& pipelineKey) {
    std::ostringstream outputStructStream;
    std::ostringstream assignOutputsStream;
    std::ostringstream finalStream;
    finalStream << absl::StrFormat(
        "struct Params {\n"
        "offset: vec2i,\n"
        "};\n"
        "@group(1) @binding(0) var<uniform> params : Params;\n");
    for (auto i : pipelineKey.attachmentsToExpandResolve) {
        finalStream << absl::StrFormat("@group(0) @binding(%u) var srcTex%u : texture_2d<f32>;\n",
                                       i, i);

        outputStructStream << absl::StrFormat("@location(%u) output%u : vec4f,\n", i, i);

        assignOutputsStream << absl::StrFormat(
            "\toutputColor.output%u = textureLoad(srcTex%u, vec2i(position.xy) + "
            "params.offset, 0);\n",
            i, i);
    }

    finalStream << "struct OutputColor {\n" << outputStructStream.str() << "}\n\n";
    finalStream << R"(
@fragment fn expand_multisample(@builtin(position) position : vec4f) -> OutputColor {
    var outputColor : OutputColor;
)" << assignOutputsStream.str()
                << R"(
    return outputColor;
})";

    return finalStream.str();
}

// Generate the fragment shader to average multiple samples into one.
std::string GenerateResolveFS(uint32_t sampleCount) {
    std::ostringstream ss;

    ss << R"(
@group(0) @binding(0) var<uniform> params : Params;
@group(0) @binding(1) var srcTex : texture_multisampled_2d<f32>;
struct Params {
  offset: vec2i,
};
@fragment
fn resolve_multisample(@builtin(position) position : vec4f) -> @location(0) vec4f {
    var sum = vec4f(0.0, 0.0, 0.0, 0.0);
    var offsetPos = vec2i(position.xy) - params.offset;)";
    ss << "\n";
    for (uint32_t sample = 0; sample < sampleCount; ++sample) {
        ss << absl::StrFormat("    sum += textureLoad(srcTex, offsetPos, %u);\n", sample);
    }
    ss << absl::StrFormat("    return sum / %u;\n", sampleCount) << "}\n";

    return ss.str();
}

ResultOrError<Ref<RenderPipelineBase>> GetOrCreateExpandMultisamplePipeline(
    DeviceBase* device,
    const BlitColorToColorWithDrawPipelineKey& pipelineKey,
    uint8_t colorAttachmentCount) {
    InternalPipelineStore* store = device->GetInternalPipelineStore();
    {
        auto it = store->expandResolveTexturePipelines.find(pipelineKey);
        if (it != store->expandResolveTexturePipelines.end()) {
            return it->second;
        }
    }

    // vertex shader's source.
    ShaderSourceWGSL wgslDesc = {};
    ShaderModuleDescriptor shaderModuleDesc = {};
    shaderModuleDesc.nextInChain = &wgslDesc;
    wgslDesc.code = kBlitToColorVS;

    Ref<ShaderModuleBase> vshaderModule;
    DAWN_TRY_ASSIGN(vshaderModule, device->CreateShaderModule(&shaderModuleDesc));

    // fragment shader's source will depend on pipeline key.
    std::string fsCode = GenerateExpandFS(pipelineKey);
    wgslDesc.code = fsCode.c_str();
    Ref<ShaderModuleBase> fshaderModule;
    DAWN_TRY_ASSIGN(fshaderModule, device->CreateShaderModule(&shaderModuleDesc));

    FragmentState fragmentState = {};
    fragmentState.module = fshaderModule.Get();
    fragmentState.entryPoint = "expand_multisample";

    // Color target states.
    PerColorAttachment<ColorTargetState> colorTargets = {};
    PerColorAttachment<wgpu::ColorTargetStateExpandResolveTextureDawn> msaaExpandResolveStates;

    for (auto [i, target] : Enumerate(colorTargets)) {
        target.format = pipelineKey.colorTargetFormats[i];
        // We shouldn't change the color targets that are not involved in.
        if (pipelineKey.resolveTargetsMask[i]) {
            target.nextInChain = &msaaExpandResolveStates[i];
            msaaExpandResolveStates[i].enabled = pipelineKey.attachmentsToExpandResolve[i];
            if (msaaExpandResolveStates[i].enabled) {
                target.writeMask = wgpu::ColorWriteMask::All;
            } else {
                target.writeMask = wgpu::ColorWriteMask::None;
            }
        } else {
            target.writeMask = wgpu::ColorWriteMask::None;
        }
    }

    fragmentState.targetCount = colorAttachmentCount;
    fragmentState.targets = colorTargets.data();

    RenderPipelineDescriptor renderPipelineDesc = {};
    renderPipelineDesc.label = "expand_multisample";
    renderPipelineDesc.vertex.module = vshaderModule.Get();
    renderPipelineDesc.vertex.entryPoint = "vert_fullscreen_quad";
    renderPipelineDesc.fragment = &fragmentState;

    // Depth stencil state.
    DepthStencilState depthStencilState = {};
    if (pipelineKey.depthStencilFormat != wgpu::TextureFormat::Undefined) {
        depthStencilState.format = pipelineKey.depthStencilFormat;
        depthStencilState.depthWriteEnabled = wgpu::OptionalBool::False;
        depthStencilState.depthCompare = wgpu::CompareFunction::Always;

        renderPipelineDesc.depthStencil = &depthStencilState;
    }

    // Multisample state.
    DAWN_ASSERT(pipelineKey.sampleCount > 1);
    renderPipelineDesc.multisample.count = pipelineKey.sampleCount;

    // Bind group layout.
    absl::InlinedVector<BindGroupLayoutEntry, kMaxColorAttachments> bglEntries;
    for (auto colorIdx : pipelineKey.attachmentsToExpandResolve) {
        bglEntries.push_back({});
        auto& bglEntry = bglEntries.back();
        bglEntry.binding = static_cast<uint8_t>(colorIdx);
        bglEntry.visibility = wgpu::ShaderStage::Fragment;
        bglEntry.texture.sampleType = kInternalResolveAttachmentSampleType;
        bglEntry.texture.viewDimension = wgpu::TextureViewDimension::e2D;
    }
    BindGroupLayoutDescriptor bglDesc = {};
    bglDesc.entries = bglEntries.data();
    bglDesc.entryCount = bglEntries.size();

    Ref<BindGroupLayoutBase> bindGroupLayout;
    DAWN_TRY_ASSIGN(bindGroupLayout,
                    device->CreateBindGroupLayout(&bglDesc, /* allowInternalBinding */ true));

    Ref<BindGroupLayoutBase> bindGroupLayout1;
    DAWN_TRY_ASSIGN(bindGroupLayout1,
                    utils::MakeBindGroupLayout(
                        device,
                        {
                            {0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform},
                        },
                        /* allowInternalBinding */ true));

    std::array<BindGroupLayoutBase*, 2> bindGroupLayouts = {bindGroupLayout.Get(),
                                                            bindGroupLayout1.Get()};
    Ref<PipelineLayoutBase> pipelineLayout;
    PipelineLayoutDescriptor descriptor;
    descriptor.bindGroupLayoutCount = bindGroupLayouts.size();
    descriptor.bindGroupLayouts = bindGroupLayouts.data();
    DAWN_TRY_ASSIGN(pipelineLayout, device->CreatePipelineLayout(&descriptor));

    renderPipelineDesc.layout = pipelineLayout.Get();

    Ref<RenderPipelineBase> pipeline;
    DAWN_TRY_ASSIGN(pipeline, device->CreateRenderPipeline(&renderPipelineDesc));

    store->expandResolveTexturePipelines.emplace(pipelineKey, pipeline);
    return pipeline;
}

ResultOrError<Ref<RenderPipelineBase>> GetOrCreateResolveMultisamplePipeline(
    DeviceBase* device,
    const ResolveMultisampleWithDrawPipelineKey& pipelineKey) {
    // Try finding the pipeline from the cache.
    InternalPipelineStore* store = device->GetInternalPipelineStore();
    {
        auto it = store->resolveMultisamplePipelines.find(pipelineKey);
        if (it != store->resolveMultisamplePipelines.end()) {
            return it->second;
        }
    }

    // vertex shader's source.
    ShaderSourceWGSL wgslDesc = {};
    ShaderModuleDescriptor shaderModuleDesc = {};
    shaderModuleDesc.nextInChain = &wgslDesc;
    wgslDesc.code = kBlitToColorVS;

    Ref<ShaderModuleBase> vshaderModule;
    DAWN_TRY_ASSIGN(vshaderModule, device->CreateShaderModule(&shaderModuleDesc));

    // fragment shader's source will depend on sample count.
    std::string fsCode = GenerateResolveFS(pipelineKey.sampleCount);
    wgslDesc.code = fsCode.c_str();
    Ref<ShaderModuleBase> fshaderModule;
    DAWN_TRY_ASSIGN(fshaderModule, device->CreateShaderModule(&shaderModuleDesc));

    FragmentState fragmentState = {};
    fragmentState.module = fshaderModule.Get();
    fragmentState.entryPoint = "resolve_multisample";

    // Color target states.
    ColorTargetState colorTarget = {};
    colorTarget.format = pipelineKey.colorTargetFormat;

    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

    RenderPipelineDescriptor renderPipelineDesc = {};
    renderPipelineDesc.label = "resolve_multisample";
    renderPipelineDesc.vertex.module = vshaderModule.Get();
    renderPipelineDesc.vertex.entryPoint = "vert_fullscreen_quad";
    renderPipelineDesc.fragment = &fragmentState;

    Ref<RenderPipelineBase> pipeline;
    DAWN_TRY_ASSIGN(pipeline, device->CreateRenderPipeline(&renderPipelineDesc));

    store->resolveMultisamplePipelines.emplace(pipelineKey, pipeline);
    return pipeline;
}

}  // namespace

MaybeError ExpandResolveTextureWithDraw(
    DeviceBase* device,
    RenderPassEncoder* renderEncoder,
    const UnpackedPtr<RenderPassDescriptor>& renderPassDescriptor) {
    DAWN_ASSERT(device->IsLockedByCurrentThreadIfNeeded());
    DAWN_ASSERT(device->CanTextureLoadResolveTargetInTheSameRenderpass());

    BlitColorToColorWithDrawPipelineKey pipelineKey;
    uint32_t colorAttachmentWidth = 0;
    uint32_t colorAttachmentHeight = 0;
    for (uint8_t i = 0; i < renderPassDescriptor->colorAttachmentCount; ++i) {
        ColorAttachmentIndex colorIdx(i);
        const auto& colorAttachment = renderPassDescriptor->colorAttachments[i];
        TextureViewBase* view = colorAttachment.view;
        if (!view) {
            continue;
        }
        if (colorAttachmentWidth == 0) {
            Extent3D renderSize = view->GetSingleSubresourceVirtualSize();
            colorAttachmentWidth = renderSize.width;
            colorAttachmentHeight = renderSize.height;
        }
        const Format& format = view->GetFormat();
        TextureComponentType baseType = format.GetAspectInfo(Aspect::Color).baseType;
        // TODO(dawn:1710): blitting integer textures are not currently supported.
        DAWN_ASSERT(baseType == TextureComponentType::Float);

        if (colorAttachment.loadOp == wgpu::LoadOp::ExpandResolveTexture) {
            DAWN_ASSERT(colorAttachment.resolveTarget->GetLayerCount() == 1u);
            DAWN_ASSERT(colorAttachment.resolveTarget->GetDimension() ==
                        wgpu::TextureViewDimension::e2D);
            pipelineKey.attachmentsToExpandResolve.set(colorIdx);
        }
        pipelineKey.resolveTargetsMask.set(colorIdx, colorAttachment.resolveTarget != nullptr);

        pipelineKey.colorTargetFormats[colorIdx] = format.format;
        pipelineKey.sampleCount = view->GetTexture()->GetSampleCount();
    }

    if (!pipelineKey.attachmentsToExpandResolve.any()) {
        return {};
    }

    pipelineKey.depthStencilFormat = wgpu::TextureFormat::Undefined;
    if (renderPassDescriptor->depthStencilAttachment != nullptr) {
        pipelineKey.depthStencilFormat =
            renderPassDescriptor->depthStencilAttachment->view->GetFormat().format;
    }

    Ref<RenderPipelineBase> pipeline;
    DAWN_TRY_ASSIGN(pipeline, GetOrCreateExpandMultisamplePipeline(
                                  device, pipelineKey, renderPassDescriptor->colorAttachmentCount));

    Ref<BindGroupLayoutBase> bgl;
    DAWN_TRY_ASSIGN(bgl, pipeline->GetBindGroupLayout(0));

    Ref<BindGroupBase> bindGroup;
    {
        absl::InlinedVector<BindGroupEntry, kMaxColorAttachments> bgEntries = {};

        for (auto colorIdx : pipelineKey.attachmentsToExpandResolve) {
            uint8_t i = static_cast<uint8_t>(colorIdx);
            const auto& colorAttachment = renderPassDescriptor->colorAttachments[i];
            bgEntries.push_back({});
            auto& bgEntry = bgEntries.back();
            bgEntry.binding = i;
            bgEntry.textureView = colorAttachment.resolveTarget;
        }

        BindGroupDescriptor bgDesc = {};
        bgDesc.layout = bgl.Get();
        bgDesc.entryCount = bgEntries.size();
        bgDesc.entries = bgEntries.data();
        DAWN_TRY_ASSIGN(bindGroup, device->CreateBindGroup(&bgDesc, UsageValidationMode::Internal));
    }
    renderEncoder->APISetBindGroup(0, bindGroup.Get());

    std::optional<RenderPassDescriptorResolveRect> expandResolveRect;
    if (auto* legacyResolveRect =
            renderPassDescriptor.Get<RenderPassDescriptorExpandResolveRect>()) {
        // This is a deprecated option.
        // TODO(417768364): Remove this once the all the call sites are updated to use the new rect.
        RenderPassDescriptorResolveRect rect{};
        rect.colorOffsetX = legacyResolveRect->x;
        rect.colorOffsetY = legacyResolveRect->y;
        rect.resolveOffsetX = legacyResolveRect->x;
        rect.resolveOffsetY = legacyResolveRect->y;
        rect.width = legacyResolveRect->width;
        rect.height = legacyResolveRect->height;
        expandResolveRect = rect;
    } else if (auto* resolveRect = renderPassDescriptor.Get<RenderPassDescriptorResolveRect>()) {
        expandResolveRect = *resolveRect;
    }

    Ref<BindGroupLayoutBase> bgl1;
    DAWN_TRY_ASSIGN(bgl1, pipeline->GetBindGroupLayout(1));
    Ref<BindGroupBase> bindGroup1;
    {
        // TODO(417770951): Use immediates as offsets.
        Ref<BufferBase> paramsBuffer;
        if (expandResolveRect) {
            DAWN_TRY_ASSIGN(
                paramsBuffer,
                utils::CreateBufferFromData(
                    device, wgpu::BufferUsage::Uniform,
                    {expandResolveRect->resolveOffsetX - expandResolveRect->colorOffsetX,
                     expandResolveRect->resolveOffsetY - expandResolveRect->colorOffsetY}));
        } else {
            DAWN_TRY_ASSIGN(paramsBuffer, utils::CreateBufferFromData(
                                              device, wgpu::BufferUsage::Uniform, {0, 0}));
        }

        BindGroupEntry bgEntry = {};
        bgEntry.binding = 0;
        bgEntry.buffer = paramsBuffer.Get();
        BindGroupDescriptor bgDesc = {};

        bgDesc.layout = bgl1.Get();
        bgDesc.entryCount = 1;
        bgDesc.entries = &bgEntry;
        DAWN_TRY_ASSIGN(bindGroup1,
                        device->CreateBindGroup(&bgDesc, UsageValidationMode::Internal));
    }
    renderEncoder->APISetBindGroup(1, bindGroup1.Get());
    renderEncoder->APISetPipeline(pipeline.Get());

    if (expandResolveRect) {
        // TODO(chromium:344814092): Prevent the scissor to be reset to outside of this region by
        // passing the scissor bound to the render pass creation.
        renderEncoder->APISetScissorRect(expandResolveRect->colorOffsetX,
                                         expandResolveRect->colorOffsetY, expandResolveRect->width,
                                         expandResolveRect->height);
    }
    // Draw to perform the blit.
    renderEncoder->APIDraw(3);
    // After expanding the resolve texture, we reset the scissor rect to the full size of the color
    // attachment to prevent the previous scissor rect from affecting all subsequent user draws.
    if (expandResolveRect) {
        renderEncoder->APISetScissorRect(0, 0, colorAttachmentWidth, colorAttachmentHeight);
    }

    return {};
}

size_t BlitColorToColorWithDrawPipelineKey::HashFunc::operator()(
    const BlitColorToColorWithDrawPipelineKey& key) const {
    size_t hash = 0;

    HashCombine(&hash, key.attachmentsToExpandResolve);
    HashCombine(&hash, key.resolveTargetsMask);

    for (auto format : key.colorTargetFormats) {
        HashCombine(&hash, format);
    }

    HashCombine(&hash, key.depthStencilFormat);
    HashCombine(&hash, key.sampleCount);

    return hash;
}

bool BlitColorToColorWithDrawPipelineKey::EqualityFunc::operator()(
    const BlitColorToColorWithDrawPipelineKey& a,
    const BlitColorToColorWithDrawPipelineKey& b) const {
    if (a.attachmentsToExpandResolve != b.attachmentsToExpandResolve) {
        return false;
    }
    if (a.resolveTargetsMask != b.resolveTargetsMask) {
        return false;
    }

    for (auto [i, format] : Enumerate(a.colorTargetFormats)) {
        if (format != b.colorTargetFormats[i]) {
            return false;
        }
    }

    return a.depthStencilFormat == b.depthStencilFormat && a.sampleCount == b.sampleCount;
}

MaybeError ResolveMultisampleWithDraw(DeviceBase* device,
                                      CommandEncoder* encoder,
                                      const RenderPassDescriptorResolveRect& rect,
                                      TextureViewBase* src,
                                      TextureViewBase* dst) {
    DAWN_ASSERT(device->IsLockedByCurrentThreadIfNeeded());

    ResolveMultisampleWithDrawPipelineKey pipelineKey{dst->GetFormat().format,
                                                      src->GetTexture()->GetSampleCount()};
    Ref<RenderPipelineBase> pipeline;
    DAWN_TRY_ASSIGN(pipeline, GetOrCreateResolveMultisamplePipeline(device, pipelineKey));

    Ref<BindGroupLayoutBase> bindGroupLayout;
    DAWN_TRY_ASSIGN(bindGroupLayout, pipeline->GetBindGroupLayout(0));

    Ref<BufferBase> paramsBuffer;
    DAWN_TRY_ASSIGN(paramsBuffer,
                    utils::CreateBufferFromData(device, wgpu::BufferUsage::Uniform,
                                                {rect.resolveOffsetX - rect.colorOffsetX,
                                                 rect.resolveOffsetY - rect.colorOffsetY}));

    Ref<BindGroupBase> bindGroup;
    DAWN_TRY_ASSIGN(bindGroup,
                    utils::MakeBindGroup(device, bindGroupLayout, {{0, paramsBuffer}, {1, src}},
                                         UsageValidationMode::Internal));

    // Color attachment descriptor.
    RenderPassColorAttachment colorAttachmentDesc;
    colorAttachmentDesc.view = dst;
    colorAttachmentDesc.loadOp = wgpu::LoadOp::Load;
    colorAttachmentDesc.storeOp = wgpu::StoreOp::Store;

    // Create render pass.
    RenderPassDescriptor renderPassDesc;
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachmentDesc;
    Ref<RenderPassEncoder> renderEncoder = encoder->BeginRenderPass(&renderPassDesc);

    // Draw to perform the resolve.
    renderEncoder->APISetBindGroup(0, bindGroup.Get(), 0, nullptr);
    renderEncoder->APISetPipeline(pipeline.Get());
    renderEncoder->APISetScissorRect(rect.resolveOffsetX, rect.resolveOffsetY, rect.width,
                                     rect.height);
    renderEncoder->APIDraw(3);
    renderEncoder->End();

    return {};
}

size_t ResolveMultisampleWithDrawPipelineKey::HashFunc::operator()(
    const ResolveMultisampleWithDrawPipelineKey& key) const {
    size_t hash = 0;

    HashCombine(&hash, key.colorTargetFormat);
    HashCombine(&hash, key.sampleCount);

    return hash;
}

bool ResolveMultisampleWithDrawPipelineKey::EqualityFunc::operator()(
    const ResolveMultisampleWithDrawPipelineKey& a,
    const ResolveMultisampleWithDrawPipelineKey& b) const {
    return a.colorTargetFormat == b.colorTargetFormat && a.sampleCount == b.sampleCount;
}

}  // namespace dawn::native
