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

#include "dawn/native/webgpu/ToWGPU.h"

#include "dawn/common/StringViewUtils.h"
#include "dawn/native/dawn_platform_autogen.h"

namespace dawn::native::webgpu {

WGPUStencilFaceState ToWGPU(const StencilFaceState* desc) {
    return {
        .compare = ToAPI(desc->compare),
        .failOp = ToAPI(desc->failOp),
        .depthFailOp = ToAPI(desc->depthFailOp),
        .passOp = ToAPI(desc->passOp),
    };
}

WGPUDepthStencilState ToWGPU(const DepthStencilState* desc) {
    return {
        .nextInChain = nullptr,
        .format = ToAPI(desc->format),
        .depthWriteEnabled = desc->depthWriteEnabled,
        .depthCompare = ToAPI(desc->depthCompare),
        .stencilFront = ToWGPU(&desc->stencilFront),
        .stencilBack = ToWGPU(&desc->stencilBack),
        .stencilReadMask = desc->stencilReadMask,
        .stencilWriteMask = desc->stencilWriteMask,
        .depthBias = desc->depthBias,
        .depthBiasSlopeScale = desc->depthBiasSlopeScale,
        .depthBiasClamp = desc->depthBiasClamp,
    };
}

WGPUPrimitiveState ToWGPU(const PrimitiveState* desc) {
    return {
        .nextInChain = nullptr,
        .topology = ToAPI(desc->topology),
        .stripIndexFormat = ToAPI(desc->stripIndexFormat),
        .frontFace = ToAPI(desc->frontFace),
        .cullMode = ToAPI(desc->cullMode),
        .unclippedDepth = desc->unclippedDepth,
    };
}

WGPUMultisampleState ToWGPU(const MultisampleState* desc) {
    return {
        .nextInChain = nullptr,
        .count = desc->count,
        .mask = desc->mask,
        .alphaToCoverageEnabled = desc->alphaToCoverageEnabled,
    };
}

WGPUBlendState ToWGPU(const BlendState* desc) {
    return {
        .color =
            {
                .operation = ToAPI(desc->color.operation),
                .srcFactor = ToAPI(desc->color.srcFactor),
                .dstFactor = ToAPI(desc->color.dstFactor),
            },
        .alpha =
            {
                .operation = ToAPI(desc->alpha.operation),
                .srcFactor = ToAPI(desc->alpha.srcFactor),
                .dstFactor = ToAPI(desc->alpha.dstFactor),
            },
    };
}

void PopulateWGPUConstants(std::vector<WGPUConstantEntry>* constants,
                           std::vector<std::string>* keys,
                           const PipelineConstantEntries& entries) {
    keys->reserve(entries.size());
    constants->reserve(entries.size());
    for (const auto& [key, value] : entries) {
        keys->push_back(key);
        constants->push_back(WGPUConstantEntry{
            .nextInChain = nullptr,
            .key = ToOutputStringView(keys->back()),
            .value = value,
        });
    }
}

}  // namespace dawn::native::webgpu
