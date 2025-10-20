// Copyright 2025 The Dawn & Tint Authors
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

#include "src/dawn/native/webgpu/RenderPipelineWGPU.h"

#include <string>
#include <vector>
#include "dawn/common/StringViewUtils.h"
#include "dawn/native/webgpu/DeviceWGPU.h"
#include "dawn/native/webgpu/PipelineLayoutWGPU.h"
#include "dawn/native/webgpu/ShaderModuleWGPU.h"
#include "dawn/native/webgpu/ToWGPU.h"

namespace dawn::native::webgpu {

// static
Ref<RenderPipeline> RenderPipeline::CreateUninitialized(
    Device* device,
    const UnpackedPtr<RenderPipelineDescriptor>& descriptor) {
    return AcquireRef(new RenderPipeline(device, descriptor));
}

RenderPipeline::RenderPipeline(Device* device,
                               const UnpackedPtr<RenderPipelineDescriptor>& descriptor)
    : RenderPipelineBase(device, descriptor), ObjectWGPU(device->wgpu.renderPipelineRelease) {}

MaybeError RenderPipeline::InitializeImpl() {
    auto device = ToBackend(GetDevice());

    WGPURenderPipelineDescriptor desc;
    std::vector<WGPUConstantEntry> vertexConstants;
    std::vector<std::string> vertexConstantsKeys;
    PerVertexBuffer<WGPUVertexBufferLayout> vertexBuffers = {};
    PerVertexBuffer<absl::InlinedVector<WGPUVertexAttribute, kMaxVertexAttributes>>
        vertexAttributes = {};
    WGPUDepthStencilState depthStencil;
    WGPUFragmentState fragmentState;
    std::vector<WGPUConstantEntry> fragmentConstants;
    std::vector<std::string> fragmentConstantsKeys;
    PerColorAttachment<WGPUColorTargetState> colorTargets = {};
    PerColorAttachment<WGPUBlendState> blends = {};
    PerColorAttachment<WGPUColorTargetStateExpandResolveTextureDawn>
        colorTargetStateExpandResolveTextureDawnExtensions = {};

    desc.nextInChain = nullptr;
    desc.label = ToOutputStringView(GetLabel());
    auto layout = GetLayout();
    if (layout != nullptr) {
        desc.layout = ToBackend(layout)->GetInnerHandle();
    } else {
        desc.layout = nullptr;
    }

    // Vertex State
    const ProgrammableStage& vertex = GetStage(SingleShaderStage::Vertex);
    desc.vertex.nextInChain = nullptr;
    desc.vertex.module = ToBackend(vertex.module.Get())->GetInnerHandle();
    desc.vertex.entryPoint = ToOutputStringView(vertex.entryPoint);
    PopulateWGPUConstants(&vertexConstants, &vertexConstantsKeys, vertex.constants);
    desc.vertex.constants = vertexConstants.data();
    desc.vertex.constantCount = vertexConstants.size();

    // Vertex Buffers
    for (VertexAttributeLocation location : GetAttributeLocationsUsed()) {
        const VertexAttributeInfo& dawnAttr = GetAttribute(location);
        vertexAttributes[dawnAttr.vertexBufferSlot].push_back({
            .nextInChain = nullptr,
            .format = ToAPI(dawnAttr.format),
            .offset = dawnAttr.offset,
            .shaderLocation = static_cast<uint32_t>(dawnAttr.shaderLocation),
        });
    }

    size_t bufferCount = 0;
    for (VertexBufferSlot slot : GetVertexBuffersUsed()) {
        const VertexBufferInfo& dawnBuffer = GetVertexBuffer(slot);
        WGPUVertexBufferLayout* wgpuBuffer = &vertexBuffers[slot];
        wgpuBuffer->arrayStride = dawnBuffer.arrayStride;
        wgpuBuffer->stepMode = ToAPI(dawnBuffer.stepMode);

        auto& wgpuAttributes = vertexAttributes[slot];
        wgpuBuffer->attributes = wgpuAttributes.data();
        wgpuBuffer->attributeCount = wgpuAttributes.size();
        bufferCount = static_cast<size_t>(slot) + 1;
    }
    desc.vertex.bufferCount = bufferCount;
    desc.vertex.buffers = vertexBuffers.data();

    // Primitive State
    desc.primitive.nextInChain = nullptr;
    desc.primitive.topology = ToAPI(GetPrimitiveTopology());
    if (IsStripPrimitiveTopology(GetPrimitiveTopology())) {
        desc.primitive.stripIndexFormat = ToAPI(GetStripIndexFormat());
    } else {
        desc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    }
    desc.primitive.frontFace = ToAPI(GetFrontFace());
    desc.primitive.cullMode = ToAPI(GetCullMode());
    desc.primitive.unclippedDepth = HasUnclippedDepth();

    // Depth Stencil State
    if (HasDepthStencilAttachment()) {
        depthStencil = ToWGPU(GetDepthStencilState());
        desc.depthStencil = &depthStencil;
    } else {
        desc.depthStencil = nullptr;
    }

    // Multisample State
    desc.multisample.nextInChain = nullptr;
    desc.multisample.count = GetSampleCount();
    desc.multisample.mask = GetSampleMask();
    desc.multisample.alphaToCoverageEnabled = IsAlphaToCoverageEnabled();

    // Fragment State
    if (HasStage(SingleShaderStage::Fragment)) {
        const ProgrammableStage& fragment = GetStage(SingleShaderStage::Fragment);
        fragmentState.nextInChain = nullptr;
        fragmentState.module = ToBackend(fragment.module.Get())->GetInnerHandle();
        fragmentState.entryPoint = ToOutputStringView(fragment.entryPoint);
        PopulateWGPUConstants(&fragmentConstants, &fragmentConstantsKeys, fragment.constants);
        fragmentState.constants = fragmentConstants.data();
        fragmentState.constantCount = fragmentConstants.size();

        uint32_t targetCount = 0;
        for (auto i : GetColorAttachmentsMask()) {
            const ColorTargetState* dawnTarget = GetColorTargetState(i);
            WGPUColorTargetState* wgpuTarget = &colorTargets[i];
            wgpuTarget->nextInChain = nullptr;
            wgpuTarget->format = ToAPI(dawnTarget->format);

            if (dawnTarget->blend != nullptr) {
                blends[i] = ToWGPU(dawnTarget->blend);
                wgpuTarget->blend = &blends[i];
            } else {
                wgpuTarget->blend = nullptr;
            }
            wgpuTarget->writeMask = ToAPI(dawnTarget->writeMask);

            if (GetAttachmentState()->GetExpandResolveInfo().resolveTargetsMask.test(i)) {
                auto& e = colorTargetStateExpandResolveTextureDawnExtensions[i];
                e = WGPU_COLOR_TARGET_STATE_EXPAND_RESOLVE_TEXTURE_DAWN_INIT;
                e.enabled =
                    GetAttachmentState()->GetExpandResolveInfo().attachmentsToExpandResolve.test(i);
                e.chain.next = wgpuTarget->nextInChain;
                wgpuTarget->nextInChain = &(e.chain);
            }

            targetCount = static_cast<size_t>(i) + 1;
        }
        fragmentState.targetCount = targetCount;
        fragmentState.targets = colorTargets.data();

        desc.fragment = &fragmentState;
    } else {
        desc.fragment = nullptr;
    }

    mInnerHandle = device->wgpu.deviceCreateRenderPipeline(device->GetInnerHandle(), &desc);
    DAWN_ASSERT(mInnerHandle);
    return {};
}

}  // namespace dawn::native::webgpu
