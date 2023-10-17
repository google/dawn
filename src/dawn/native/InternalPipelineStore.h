// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_INTERNALPIPELINESTORE_H_
#define SRC_DAWN_NATIVE_INTERNALPIPELINESTORE_H_

#include <unordered_map>
#include <utility>

#include "dawn/common/HashUtils.h"
#include "dawn/native/ApplyClearColorValueWithDrawHelper.h"
#include "dawn/native/BlitColorToColorWithDraw.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/ScratchBuffer.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

class DeviceBase;
class RenderPipelineBase;
class ShaderModuleBase;

// Every DeviceBase owns an InternalPipelineStore. This is a general-purpose cache for
// long-lived objects scoped to a device and used to support arbitrary pipeline operations.
struct InternalPipelineStore {
    explicit InternalPipelineStore(DeviceBase* device);
    ~InternalPipelineStore();

    std::unordered_map<wgpu::TextureFormat, Ref<RenderPipelineBase>> copyTextureForBrowserPipelines;
    std::unordered_map<wgpu::TextureFormat, Ref<RenderPipelineBase>>
        copyExternalTextureForBrowserPipelines;

    Ref<ShaderModuleBase> copyForBrowser;

    Ref<ComputePipelineBase> timestampComputePipeline;
    Ref<ShaderModuleBase> timestampCS;

    ApplyClearColorValueWithDrawPipelinesCache applyClearColorValueWithDrawPipelines;

    Ref<ShaderModuleBase> placeholderFragmentShader;

    // A scratch buffer suitable for use as a copy destination and storage binding.
    ScratchBuffer scratchStorage;

    // A scratch buffer suitable for use as a copy destination, storage binding, and indirect
    // buffer for indirect dispatch or draw calls.
    ScratchBuffer scratchIndirectStorage;

    Ref<ComputePipelineBase> renderValidationPipeline;
    Ref<ShaderModuleBase> renderValidationShader;
    Ref<ComputePipelineBase> dispatchIndirectValidationPipeline;

    Ref<RenderPipelineBase> blitRG8ToDepth16UnormPipeline;

    using BlitTextureToBufferComputePipelineKeyType =
        std::pair<wgpu::TextureFormat, wgpu::TextureViewDimension>;
    struct BlitTextureToBufferComputePipelineHash {
        std::size_t operator()(const BlitTextureToBufferComputePipelineKeyType& k) const {
            size_t hash = 0;
            HashCombine(&hash, k.first);
            HashCombine(&hash, k.second);
            return hash;
        }
    };
    std::unordered_map<BlitTextureToBufferComputePipelineKeyType,
                       Ref<ComputePipelineBase>,
                       BlitTextureToBufferComputePipelineHash>
        blitTextureToBufferComputePipelines;

    struct BlitR8ToStencilPipelines {
        Ref<RenderPipelineBase> clearPipeline;
        std::array<Ref<RenderPipelineBase>, 8> setStencilPipelines;
    };
    std::unordered_map<wgpu::TextureFormat, BlitR8ToStencilPipelines> blitR8ToStencilPipelines;

    std::unordered_map<wgpu::TextureFormat, Ref<RenderPipelineBase>> depthBlitPipelines;

    BlitColorToColorWithDrawPipelinesCache msaaRenderToSingleSampledColorBlitPipelines;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_INTERNALPIPELINESTORE_H_
