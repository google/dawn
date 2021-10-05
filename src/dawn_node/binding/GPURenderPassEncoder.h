// Copyright 2021 The Dawn Authors
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

#ifndef DAWN_NODE_BINDING_GPURENDERPASSENCODER_H_
#define DAWN_NODE_BINDING_GPURENDERPASSENCODER_H_

#include "dawn/webgpu_cpp.h"
#include "dawn_native/DawnNative.h"
#include "napi.h"
#include "src/dawn_node/interop/WebGPU.h"

namespace wgpu { namespace binding {

    // GPURenderPassEncoder is an implementation of interop::GPURenderPassEncoder that wraps a
    // wgpu::RenderPassEncoder.
    class GPURenderPassEncoder final : public interop::GPURenderPassEncoder {
      public:
        GPURenderPassEncoder(wgpu::RenderPassEncoder enc);

        // Implicit cast operator to Dawn GPU object
        inline operator const wgpu::RenderPassEncoder &() const {
            return enc_;
        }

        // interop::GPURenderPassEncoder interface compliance
        void setViewport(Napi::Env,
                         float x,
                         float y,
                         float width,
                         float height,
                         float minDepth,
                         float maxDepth) override;
        void setScissorRect(Napi::Env,
                            interop::GPUIntegerCoordinate x,
                            interop::GPUIntegerCoordinate y,
                            interop::GPUIntegerCoordinate width,
                            interop::GPUIntegerCoordinate height) override;
        void setBlendConstant(Napi::Env, interop::GPUColor color) override;
        void setStencilReference(Napi::Env, interop::GPUStencilValue reference) override;
        void beginOcclusionQuery(Napi::Env, interop::GPUSize32 queryIndex) override;
        void endOcclusionQuery(Napi::Env) override;
        void beginPipelineStatisticsQuery(Napi::Env,
                                          interop::Interface<interop::GPUQuerySet> querySet,
                                          interop::GPUSize32 queryIndex) override;
        void endPipelineStatisticsQuery(Napi::Env) override;
        void writeTimestamp(Napi::Env,
                            interop::Interface<interop::GPUQuerySet> querySet,
                            interop::GPUSize32 queryIndex) override;
        void executeBundles(
            Napi::Env,
            std::vector<interop::Interface<interop::GPURenderBundle>> bundles) override;
        void endPass(Napi::Env) override;
        void setBindGroup(Napi::Env,
                          interop::GPUIndex32 index,
                          interop::Interface<interop::GPUBindGroup> bindGroup,
                          std::vector<interop::GPUBufferDynamicOffset> dynamicOffsets) override;
        void setBindGroup(Napi::Env,
                          interop::GPUIndex32 index,
                          interop::Interface<interop::GPUBindGroup> bindGroup,
                          interop::Uint32Array dynamicOffsetsData,
                          interop::GPUSize64 dynamicOffsetsDataStart,
                          interop::GPUSize32 dynamicOffsetsDataLength) override;
        void pushDebugGroup(Napi::Env, std::string groupLabel) override;
        void popDebugGroup(Napi::Env) override;
        void insertDebugMarker(Napi::Env, std::string markerLabel) override;
        void setPipeline(Napi::Env,
                         interop::Interface<interop::GPURenderPipeline> pipeline) override;
        void setIndexBuffer(Napi::Env,
                            interop::Interface<interop::GPUBuffer> buffer,
                            interop::GPUIndexFormat indexFormat,
                            interop::GPUSize64 offset,
                            std::optional<interop::GPUSize64> size) override;
        void setVertexBuffer(Napi::Env,
                             interop::GPUIndex32 slot,
                             interop::Interface<interop::GPUBuffer> buffer,
                             interop::GPUSize64 offset,
                             std::optional<interop::GPUSize64> size) override;
        void draw(Napi::Env,
                  interop::GPUSize32 vertexCount,
                  interop::GPUSize32 instanceCount,
                  interop::GPUSize32 firstVertex,
                  interop::GPUSize32 firstInstance) override;
        void drawIndexed(Napi::Env,
                         interop::GPUSize32 indexCount,
                         interop::GPUSize32 instanceCount,
                         interop::GPUSize32 firstIndex,
                         interop::GPUSignedOffset32 baseVertex,
                         interop::GPUSize32 firstInstance) override;
        void drawIndirect(Napi::Env,
                          interop::Interface<interop::GPUBuffer> indirectBuffer,
                          interop::GPUSize64 indirectOffset) override;
        void drawIndexedIndirect(Napi::Env,
                                 interop::Interface<interop::GPUBuffer> indirectBuffer,
                                 interop::GPUSize64 indirectOffset) override;
        std::optional<std::string> getLabel(Napi::Env) override;
        void setLabel(Napi::Env, std::optional<std::string> value) override;

      private:
        wgpu::RenderPassEncoder enc_;
    };

}}  // namespace wgpu::binding

#endif  // DAWN_NODE_BINDING_GPURENDERPASSENCODER_H_
