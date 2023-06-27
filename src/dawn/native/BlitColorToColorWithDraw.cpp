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

#include "dawn/native/BlitColorToColorWithDraw.h"

#include "dawn/common/Assert.h"
#include "dawn/common/HashUtils.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/Device.h"
#include "dawn/native/InternalPipelineStore.h"
#include "dawn/native/RenderPassEncoder.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/utils/WGPUHelpers.h"

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

constexpr char kBlitToFloatColorFS[] = R"(
@group(0) @binding(0) var src_tex : texture_2d<f32>;

@fragment fn blit_to_color(@builtin(position) position : vec4f) -> @location(0) vec4f {
  return textureLoad(src_tex, vec2u(position.xy), 0);
}

)";

ResultOrError<Ref<RenderPipelineBase>> GetOrCreateColorBlitPipeline(
    DeviceBase* device,
    const Format& colorInternalFormat,
    wgpu::TextureFormat depthStencilFormat,
    uint32_t sampleCount) {
    InternalPipelineStore* store = device->GetInternalPipelineStore();
    BlitColorToColorWithDrawPipelineKey pipelineKey;
    pipelineKey.colorFormat = colorInternalFormat.format;
    pipelineKey.depthStencilFormat = depthStencilFormat;
    pipelineKey.sampleCount = sampleCount;
    {
        auto it = store->msaaRenderToSingleSampledColorBlitPipelines.find(pipelineKey);
        if (it != store->msaaRenderToSingleSampledColorBlitPipelines.end()) {
            return it->second;
        }
    }

    const auto& formatAspectInfo = colorInternalFormat.GetAspectInfo(Aspect::Color);

    // vertex shader's source.
    ShaderModuleWGSLDescriptor wgslDesc = {};
    ShaderModuleDescriptor shaderModuleDesc = {};
    shaderModuleDesc.nextInChain = &wgslDesc;
    wgslDesc.code = kBlitToColorVS;

    Ref<ShaderModuleBase> vshaderModule;
    DAWN_TRY_ASSIGN(vshaderModule, device->CreateShaderModule(&shaderModuleDesc));

    // fragment shader's source will depend on color format type.
    switch (formatAspectInfo.baseType) {
        case TextureComponentType::Float:
            wgslDesc.code = kBlitToFloatColorFS;
            break;
        default:
            // TODO(dawn:1710): blitting integer textures are not currently supported.
            UNREACHABLE();
            break;
    }
    Ref<ShaderModuleBase> fshaderModule;
    DAWN_TRY_ASSIGN(fshaderModule, device->CreateShaderModule(&shaderModuleDesc));

    FragmentState fragmentState = {};
    fragmentState.module = fshaderModule.Get();
    fragmentState.entryPoint = "blit_to_color";

    // Color target state.
    ColorTargetState colorTarget;
    colorTarget.format = colorInternalFormat.format;

    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

    RenderPipelineDescriptor renderPipelineDesc = {};
    renderPipelineDesc.label = "blit_color_to_color";
    renderPipelineDesc.vertex.module = vshaderModule.Get();
    renderPipelineDesc.vertex.entryPoint = "vert_fullscreen_quad";
    renderPipelineDesc.fragment = &fragmentState;

    // Depth stencil state.
    DepthStencilState depthStencilState = {};
    if (depthStencilFormat != wgpu::TextureFormat::Undefined) {
        depthStencilState.format = depthStencilFormat;
        depthStencilState.depthWriteEnabled = false;
        depthStencilState.depthCompare = wgpu::CompareFunction::Always;

        renderPipelineDesc.depthStencil = &depthStencilState;
    }

    // Multisample state.
    ASSERT(sampleCount > 1);
    renderPipelineDesc.multisample.count = sampleCount;
    DawnMultisampleStateRenderToSingleSampled msaaRenderToSingleSampledDesc = {};
    msaaRenderToSingleSampledDesc.enabled = true;
    renderPipelineDesc.multisample.nextInChain = &msaaRenderToSingleSampledDesc;

    // Bind group layout.
    Ref<BindGroupLayoutBase> bindGroupLayout;
    DAWN_TRY_ASSIGN(bindGroupLayout,
                    utils::MakeBindGroupLayout(
                        device,
                        {
                            {0, wgpu::ShaderStage::Fragment, kInternalResolveAttachmentSampleType,
                             wgpu::TextureViewDimension::e2D},
                        },
                        /* allowInternalBinding */ true));
    Ref<PipelineLayoutBase> pipelineLayout;
    DAWN_TRY_ASSIGN(pipelineLayout, utils::MakeBasicPipelineLayout(device, bindGroupLayout));
    renderPipelineDesc.layout = pipelineLayout.Get();

    Ref<RenderPipelineBase> pipeline;
    DAWN_TRY_ASSIGN(pipeline, device->CreateRenderPipeline(&renderPipelineDesc));

    store->msaaRenderToSingleSampledColorBlitPipelines[pipelineKey] = pipeline;
    return pipeline;
}

}  // namespace

MaybeError BlitMSAARenderToSingleSampledColorWithDraw(
    DeviceBase* device,
    RenderPassEncoder* renderEncoder,
    const RenderPassDescriptor* renderPassDescriptor,
    uint32_t renderPassImplicitSampleCount) {
    ASSERT(device->IsLockedByCurrentThreadIfNeeded());
    ASSERT(device->IsResolveTextureBlitWithDrawSupported());

    // TODO(dawn:1710): support multiple attachments.
    ASSERT(renderPassDescriptor->colorAttachmentCount == 1);

    // The original color attachment of the render pass will be used as source.
    TextureViewBase* src = renderPassDescriptor->colorAttachments[0].view;
    TextureBase* srcTexture = src->GetTexture();

    // ASSERT that the src texture is not multisampled nor having more than 1 layer.
    ASSERT(srcTexture->GetSampleCount() == 1u);
    ASSERT(src->GetLayerCount() == 1u);
    ASSERT(src->GetDimension() == wgpu::TextureViewDimension::e2D);

    wgpu::TextureFormat depthStencilFormat = wgpu::TextureFormat::Undefined;
    if (renderPassDescriptor->depthStencilAttachment != nullptr) {
        depthStencilFormat = renderPassDescriptor->depthStencilAttachment->view->GetFormat().format;
    }

    ASSERT(renderPassImplicitSampleCount > 1);

    Ref<RenderPipelineBase> pipeline;
    DAWN_TRY_ASSIGN(pipeline,
                    GetOrCreateColorBlitPipeline(device, src->GetFormat(), depthStencilFormat,
                                                 renderPassImplicitSampleCount));

    Ref<BindGroupLayoutBase> bgl;
    DAWN_TRY_ASSIGN(bgl, pipeline->GetBindGroupLayout(0));

    Ref<BindGroupBase> bindGroup;
    {
        BindGroupEntry bgEntry = {};
        bgEntry.binding = 0;
        bgEntry.textureView = src;

        BindGroupDescriptor bgDesc = {};
        bgDesc.layout = bgl.Get();
        bgDesc.entryCount = 1;
        bgDesc.entries = &bgEntry;
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

    HashCombine(&hash, key.colorFormat);
    HashCombine(&hash, key.depthStencilFormat);
    HashCombine(&hash, key.sampleCount);

    return hash;
}

bool BlitColorToColorWithDrawPipelineKey::EqualityFunc::operator()(
    const BlitColorToColorWithDrawPipelineKey& a,
    const BlitColorToColorWithDrawPipelineKey& b) const {
    return a.colorFormat == b.colorFormat && a.depthStencilFormat == b.depthStencilFormat &&
           a.sampleCount == b.sampleCount;
}

}  // namespace dawn::native
