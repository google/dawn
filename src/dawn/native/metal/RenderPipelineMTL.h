// Copyright 2017 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_METAL_RENDERPIPELINEMTL_H_
#define SRC_DAWN_NATIVE_METAL_RENDERPIPELINEMTL_H_

#include "dawn/native/RenderPipeline.h"

#include "dawn/common/NSRef.h"

#import <Metal/Metal.h>

namespace dawn::native::metal {

class Device;

class RenderPipeline final : public RenderPipelineBase {
  public:
    static Ref<RenderPipelineBase> CreateUninitialized(Device* device,
                                                       const RenderPipelineDescriptor* descriptor);
    static void InitializeAsync(Ref<RenderPipelineBase> renderPipeline,
                                WGPUCreateRenderPipelineAsyncCallback callback,
                                void* userdata);

    RenderPipeline(DeviceBase* device, const RenderPipelineDescriptor* descriptor);
    ~RenderPipeline() override;

    MTLPrimitiveType GetMTLPrimitiveTopology() const;
    MTLWinding GetMTLFrontFace() const;
    MTLCullMode GetMTLCullMode() const;

    void Encode(id<MTLRenderCommandEncoder> encoder);

    id<MTLDepthStencilState> GetMTLDepthStencilState();

    // For each Dawn vertex buffer, give the index in which it will be positioned in the Metal
    // vertex buffer table.
    uint32_t GetMtlVertexBufferIndex(VertexBufferSlot slot) const;

    wgpu::ShaderStage GetStagesRequiringStorageBufferLength() const;

    MaybeError Initialize() override;

  private:
    using RenderPipelineBase::RenderPipelineBase;

    NSRef<MTLVertexDescriptor> MakeVertexDesc();

    MTLPrimitiveType mMtlPrimitiveTopology;
    MTLWinding mMtlFrontFace;
    MTLCullMode mMtlCullMode;
    NSPRef<id<MTLRenderPipelineState>> mMtlRenderPipelineState;
    NSPRef<id<MTLDepthStencilState>> mMtlDepthStencilState;
    ityp::array<VertexBufferSlot, uint32_t, kMaxVertexBuffers> mMtlVertexBufferIndices;

    wgpu::ShaderStage mStagesRequiringStorageBufferLength = wgpu::ShaderStage::None;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_RENDERPIPELINEMTL_H_
