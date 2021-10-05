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

#ifndef DAWN_NODE_BINDING_GPUCOMPUTEPASSENCODER_H_
#define DAWN_NODE_BINDING_GPUCOMPUTEPASSENCODER_H_

#include "dawn/webgpu_cpp.h"
#include "dawn_native/DawnNative.h"
#include "napi.h"
#include "src/dawn_node/interop/WebGPU.h"

namespace wgpu { namespace binding {

    // GPUComputePassEncoder is an implementation of interop::GPUComputePassEncoder that wraps a
    // wgpu::ComputePassEncoder.
    class GPUComputePassEncoder final : public interop::GPUComputePassEncoder {
      public:
        GPUComputePassEncoder(wgpu::ComputePassEncoder enc);

        // Implicit cast operator to Dawn GPU object
        inline operator const wgpu::ComputePassEncoder &() const {
            return enc_;
        }

        // interop::GPUComputePassEncoder interface compliance
        void setPipeline(Napi::Env,
                         interop::Interface<interop::GPUComputePipeline> pipeline) override;
        void dispatch(Napi::Env,
                      interop::GPUSize32 x,
                      interop::GPUSize32 y,
                      interop::GPUSize32 z) override;
        void dispatchIndirect(Napi::Env,
                              interop::Interface<interop::GPUBuffer> indirectBuffer,
                              interop::GPUSize64 indirectOffset) override;
        void beginPipelineStatisticsQuery(Napi::Env,
                                          interop::Interface<interop::GPUQuerySet> querySet,
                                          interop::GPUSize32 queryIndex) override;
        void endPipelineStatisticsQuery(Napi::Env) override;
        void writeTimestamp(Napi::Env,
                            interop::Interface<interop::GPUQuerySet> querySet,
                            interop::GPUSize32 queryIndex) override;
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
        std::optional<std::string> getLabel(Napi::Env) override;
        void setLabel(Napi::Env, std::optional<std::string> value) override;

      private:
        wgpu::ComputePassEncoder enc_;
    };

}}  // namespace wgpu::binding

#endif  // DAWN_NODE_BINDING_GPUCOMPUTEPASSENCODER_H_
