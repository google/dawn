// Copyright 2018 The Dawn Authors
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

#include "dawn/utils/ComboRenderPipelineDescriptor.h"

#include "dawn/utils/WGPUHelpers.h"

namespace utils {

ComboVertexState::ComboVertexState() {
    vertexBufferCount = 0;

    // Fill the default values for vertexBuffers and vertexAttributes in buffers.
    wgpu::VertexAttribute vertexAttribute;
    vertexAttribute.shaderLocation = 0;
    vertexAttribute.offset = 0;
    vertexAttribute.format = wgpu::VertexFormat::Float32;
    for (uint32_t i = 0; i < kMaxVertexAttributes; ++i) {
        cAttributes[i] = vertexAttribute;
    }
    for (uint32_t i = 0; i < kMaxVertexBuffers; ++i) {
        cVertexBuffers[i].arrayStride = 0;
        cVertexBuffers[i].stepMode = wgpu::VertexStepMode::Vertex;
        cVertexBuffers[i].attributeCount = 0;
        cVertexBuffers[i].attributes = nullptr;
    }
    // cVertexBuffers[i].attributes points to somewhere in cAttributes.
    // cVertexBuffers[0].attributes points to &cAttributes[0] by default. Assuming
    // cVertexBuffers[0] has two attributes, then cVertexBuffers[1].attributes should point to
    // &cAttributes[2]. Likewise, if cVertexBuffers[1] has 3 attributes, then
    // cVertexBuffers[2].attributes should point to &cAttributes[5].
    cVertexBuffers[0].attributes = &cAttributes[0];
}

ComboRenderPipelineDescriptor::ComboRenderPipelineDescriptor() {
    wgpu::RenderPipelineDescriptor* descriptor = this;

    // Set defaults for the vertex state.
    {
        wgpu::VertexState* vertex = &descriptor->vertex;
        vertex->module = nullptr;
        vertex->entryPoint = "main";
        vertex->bufferCount = 0;

        // Fill the default values for vertexBuffers and vertexAttributes in buffers.
        for (uint32_t i = 0; i < kMaxVertexAttributes; ++i) {
            cAttributes[i].shaderLocation = 0;
            cAttributes[i].offset = 0;
            cAttributes[i].format = wgpu::VertexFormat::Float32;
        }
        for (uint32_t i = 0; i < kMaxVertexBuffers; ++i) {
            cBuffers[i].arrayStride = 0;
            cBuffers[i].stepMode = wgpu::VertexStepMode::Vertex;
            cBuffers[i].attributeCount = 0;
            cBuffers[i].attributes = nullptr;
        }
        // cBuffers[i].attributes points to somewhere in cAttributes.
        // cBuffers[0].attributes points to &cAttributes[0] by default. Assuming
        // cBuffers[0] has two attributes, then cBuffers[1].attributes should point to
        // &cAttributes[2]. Likewise, if cBuffers[1] has 3 attributes, then
        // cBuffers[2].attributes should point to &cAttributes[5].
        cBuffers[0].attributes = &cAttributes[0];
        vertex->buffers = &cBuffers[0];
    }

    // Set the defaults for the primitive state
    {
        wgpu::PrimitiveState* primitive = &descriptor->primitive;
        primitive->topology = wgpu::PrimitiveTopology::TriangleList;
        primitive->stripIndexFormat = wgpu::IndexFormat::Undefined;
        primitive->frontFace = wgpu::FrontFace::CCW;
        primitive->cullMode = wgpu::CullMode::None;
    }

    // Set the defaults for the depth-stencil state
    {
        wgpu::StencilFaceState stencilFace;
        stencilFace.compare = wgpu::CompareFunction::Always;
        stencilFace.failOp = wgpu::StencilOperation::Keep;
        stencilFace.depthFailOp = wgpu::StencilOperation::Keep;
        stencilFace.passOp = wgpu::StencilOperation::Keep;

        cDepthStencil.format = wgpu::TextureFormat::Depth24PlusStencil8;
        cDepthStencil.depthWriteEnabled = false;
        cDepthStencil.depthCompare = wgpu::CompareFunction::Always;
        cDepthStencil.stencilBack = stencilFace;
        cDepthStencil.stencilFront = stencilFace;
        cDepthStencil.stencilReadMask = 0xff;
        cDepthStencil.stencilWriteMask = 0xff;
        cDepthStencil.depthBias = 0;
        cDepthStencil.depthBiasSlopeScale = 0.0;
        cDepthStencil.depthBiasClamp = 0.0;
    }

    // Set the defaults for the multisample state
    {
        wgpu::MultisampleState* multisample = &descriptor->multisample;
        multisample->count = 1;
        multisample->mask = 0xFFFFFFFF;
        multisample->alphaToCoverageEnabled = false;
    }

    // Set the defaults for the fragment state
    {
        cFragment.module = nullptr;
        cFragment.entryPoint = "main";
        cFragment.targetCount = 1;
        cFragment.targets = &cTargets[0];
        descriptor->fragment = &cFragment;

        wgpu::BlendComponent blendComponent;
        blendComponent.srcFactor = wgpu::BlendFactor::One;
        blendComponent.dstFactor = wgpu::BlendFactor::Zero;
        blendComponent.operation = wgpu::BlendOperation::Add;

        for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
            cTargets[i].format = wgpu::TextureFormat::RGBA8Unorm;
            cTargets[i].blend = nullptr;
            cTargets[i].writeMask = wgpu::ColorWriteMask::All;

            cBlends[i].color = blendComponent;
            cBlends[i].alpha = blendComponent;
        }
    }
}

wgpu::DepthStencilState* ComboRenderPipelineDescriptor::EnableDepthStencil(
    wgpu::TextureFormat format) {
    this->depthStencil = &cDepthStencil;
    cDepthStencil.format = format;
    return &cDepthStencil;
}

}  // namespace utils
