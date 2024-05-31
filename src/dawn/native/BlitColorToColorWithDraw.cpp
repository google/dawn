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

#include "absl/container/inlined_vector.h"
#include "dawn/common/Assert.h"
#include "dawn/common/Enumerator.h"
#include "dawn/common/HashUtils.h"
#include "dawn/native/BindGroup.h"
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

std::string GenerateFS(const BlitColorToColorWithDrawPipelineKey& pipelineKey) {
    std::ostringstream outputStructStream;
    std::ostringstream assignOutputsStream;
    std::ostringstream finalStream;

    for (auto i : IterateBitSet(pipelineKey.attachmentsToExpandResolve)) {
        finalStream << absl::StrFormat("@group(0) @binding(%u) var srcTex%u : texture_2d<f32>;\n",
                                       i, i);

        outputStructStream << absl::StrFormat("@location(%u) output%u : vec4f,\n", i, i);

        assignOutputsStream << absl::StrFormat(
            "\toutputColor.output%u = textureLoad(srcTex%u, vec2u(position.xy), 0);\n", i, i);
    }

    finalStream << "struct OutputColor {\n" << outputStructStream.str() << "}\n" << std::endl;
    finalStream << R"(
@fragment fn blit_to_color(@builtin(position) position : vec4f) -> OutputColor {
    var outputColor : OutputColor;
)" << assignOutputsStream.str()
                << R"(
    return outputColor;
})";

    return finalStream.str();
}

ResultOrError<Ref<RenderPipelineBase>> GetOrCreateColorBlitPipeline(
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
    ShaderModuleWGSLDescriptor wgslDesc = {};
    ShaderModuleDescriptor shaderModuleDesc = {};
    shaderModuleDesc.nextInChain = &wgslDesc;
    wgslDesc.code = kBlitToColorVS;

    Ref<ShaderModuleBase> vshaderModule;
    DAWN_TRY_ASSIGN(vshaderModule, device->CreateShaderModule(&shaderModuleDesc));

    // fragment shader's source will depend on pipeline key.
    std::string fsCode = GenerateFS(pipelineKey);
    wgslDesc.code = fsCode.c_str();
    Ref<ShaderModuleBase> fshaderModule;
    DAWN_TRY_ASSIGN(fshaderModule, device->CreateShaderModule(&shaderModuleDesc));

    FragmentState fragmentState = {};
    fragmentState.module = fshaderModule.Get();
    fragmentState.entryPoint = "blit_to_color";

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
    renderPipelineDesc.label = "blit_color_to_color";
    renderPipelineDesc.vertex.module = vshaderModule.Get();
    renderPipelineDesc.vertex.entryPoint = "vert_fullscreen_quad";
    renderPipelineDesc.fragment = &fragmentState;

    // Depth stencil state.
    DepthStencilState depthStencilState = {};
    if (pipelineKey.depthStencilFormat != wgpu::TextureFormat::Undefined) {
        depthStencilState.format = pipelineKey.depthStencilFormat;
        depthStencilState.depthWriteEnabled = false;
        depthStencilState.depthCompare = wgpu::CompareFunction::Always;

        renderPipelineDesc.depthStencil = &depthStencilState;
    }

    // Multisample state.
    DAWN_ASSERT(pipelineKey.sampleCount > 1);
    renderPipelineDesc.multisample.count = pipelineKey.sampleCount;

    // Bind group layout.
    absl::InlinedVector<BindGroupLayoutEntry, kMaxColorAttachments> bglEntries;
    for (auto colorIdx : IterateBitSet(pipelineKey.attachmentsToExpandResolve)) {
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

    Ref<PipelineLayoutBase> pipelineLayout;
    DAWN_TRY_ASSIGN(pipelineLayout, utils::MakeBasicPipelineLayout(device, bindGroupLayout));
    renderPipelineDesc.layout = pipelineLayout.Get();

    Ref<RenderPipelineBase> pipeline;
    DAWN_TRY_ASSIGN(pipeline, device->CreateRenderPipeline(&renderPipelineDesc));

    store->expandResolveTexturePipelines.emplace(pipelineKey, pipeline);
    return pipeline;
}

}  // namespace

MaybeError ExpandResolveTextureWithDraw(DeviceBase* device,
                                        RenderPassEncoder* renderEncoder,
                                        const RenderPassDescriptor* renderPassDescriptor) {
    DAWN_ASSERT(device->IsLockedByCurrentThreadIfNeeded());
    DAWN_ASSERT(device->IsResolveTextureBlitWithDrawSupported());

    BlitColorToColorWithDrawPipelineKey pipelineKey;
    for (uint8_t i = 0; i < renderPassDescriptor->colorAttachmentCount; ++i) {
        ColorAttachmentIndex colorIdx(i);
        const auto& colorAttachment = renderPassDescriptor->colorAttachments[i];
        TextureViewBase* view = colorAttachment.view;
        if (!view) {
            continue;
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
        pipelineKey.resolveTargetsMask.set(colorIdx, colorAttachment.resolveTarget);

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
    DAWN_TRY_ASSIGN(pipeline, GetOrCreateColorBlitPipeline(
                                  device, pipelineKey, renderPassDescriptor->colorAttachmentCount));

    Ref<BindGroupLayoutBase> bgl;
    DAWN_TRY_ASSIGN(bgl, pipeline->GetBindGroupLayout(0));

    Ref<BindGroupBase> bindGroup;
    {
        absl::InlinedVector<BindGroupEntry, kMaxColorAttachments> bgEntries = {};

        for (auto colorIdx : IterateBitSet(pipelineKey.attachmentsToExpandResolve)) {
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

    // Draw to perform the blit.
    renderEncoder->APISetBindGroup(0, bindGroup.Get(), 0, nullptr);
    renderEncoder->APISetPipeline(pipeline.Get());
    renderEncoder->APIDraw(3, 1, 0, 0);

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

}  // namespace dawn::native
