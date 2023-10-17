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

#ifndef SRC_DAWN_NATIVE_BLITCOLORTOCOLORWITHDRAW_H_
#define SRC_DAWN_NATIVE_BLITCOLORTOCOLORWITHDRAW_H_

#include <unordered_map>

#include "dawn/native/Error.h"

namespace dawn::native {

class DeviceBase;
class RenderPassEncoder;
struct RenderPassDescriptor;
class TextureViewBase;

struct BlitColorToColorWithDrawPipelineKey {
    wgpu::TextureFormat colorFormat;
    wgpu::TextureFormat depthStencilFormat;
    uint32_t sampleCount = 1;

    struct HashFunc {
        size_t operator()(const BlitColorToColorWithDrawPipelineKey& key) const;
    };

    struct EqualityFunc {
        bool operator()(const BlitColorToColorWithDrawPipelineKey& a,
                        const BlitColorToColorWithDrawPipelineKey& b) const;
    };
};

using BlitColorToColorWithDrawPipelinesCache =
    std::unordered_map<BlitColorToColorWithDrawPipelineKey,
                       Ref<RenderPipelineBase>,
                       BlitColorToColorWithDrawPipelineKey::HashFunc,
                       BlitColorToColorWithDrawPipelineKey::EqualityFunc>;

// In a MSAA render to single sampled render pass, a color attachment will be used as resolve
// target internally and an implicit MSAA texture will be used as the actual color attachment.
//
// This function performs the load operation for the render pass by blitting the resolve target (the
// original color attachment) to the implicit MSAA attachment.
//
// The function assumes that the render pass is already started. It won't break the render pass,
// just performing a draw call to blit.
// This is only valid if the device's IsResolveTextureBlitWithDrawSupported() is true.
MaybeError BlitMSAARenderToSingleSampledColorWithDraw(
    DeviceBase* device,
    RenderPassEncoder* renderEncoder,
    const RenderPassDescriptor* renderPassDescriptor,
    uint32_t renderPassImplicitSampleCount);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BLITCOLORTOCOLORWITHDRAW_H_
