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
