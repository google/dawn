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

#include "src/dawn_node/binding/GPUComputePassEncoder.h"

#include "src/dawn_node/binding/Converter.h"
#include "src/dawn_node/binding/GPUBindGroup.h"
#include "src/dawn_node/binding/GPUBuffer.h"
#include "src/dawn_node/binding/GPUComputePipeline.h"
#include "src/dawn_node/binding/GPUQuerySet.h"
#include "src/dawn_node/utils/Debug.h"

namespace wgpu { namespace binding {

    ////////////////////////////////////////////////////////////////////////////////
    // wgpu::bindings::GPUComputePassEncoder
    ////////////////////////////////////////////////////////////////////////////////
    GPUComputePassEncoder::GPUComputePassEncoder(wgpu::ComputePassEncoder enc)
        : enc_(std::move(enc)) {
    }

    void GPUComputePassEncoder::setPipeline(
        Napi::Env,
        interop::Interface<interop::GPUComputePipeline> pipeline) {
        enc_.SetPipeline(*pipeline.As<GPUComputePipeline>());
    }

    void GPUComputePassEncoder::dispatch(Napi::Env,
                                         interop::GPUSize32 x,
                                         interop::GPUSize32 y,
                                         interop::GPUSize32 z) {
        enc_.Dispatch(x, y, z);
    }

    void GPUComputePassEncoder::dispatchIndirect(
        Napi::Env,
        interop::Interface<interop::GPUBuffer> indirectBuffer,
        interop::GPUSize64 indirectOffset) {
        enc_.DispatchIndirect(*indirectBuffer.As<GPUBuffer>(), indirectOffset);
    }

    void GPUComputePassEncoder::beginPipelineStatisticsQuery(
        Napi::Env,
        interop::Interface<interop::GPUQuerySet> querySet,
        interop::GPUSize32 queryIndex) {
        UNIMPLEMENTED();
    }

    void GPUComputePassEncoder::endPipelineStatisticsQuery(Napi::Env) {
        UNIMPLEMENTED();
    }

    void GPUComputePassEncoder::writeTimestamp(Napi::Env env,
                                               interop::Interface<interop::GPUQuerySet> querySet,
                                               interop::GPUSize32 queryIndex) {
        Converter conv(env);

        wgpu::QuerySet q{};
        if (!conv(q, querySet)) {
            return;
        }

        enc_.WriteTimestamp(q, queryIndex);
    }

    void GPUComputePassEncoder::endPass(Napi::Env) {
        enc_.EndPass();
    }

    void GPUComputePassEncoder::setBindGroup(
        Napi::Env env,
        interop::GPUIndex32 index,
        interop::Interface<interop::GPUBindGroup> bindGroup,
        std::vector<interop::GPUBufferDynamicOffset> dynamicOffsets) {
        Converter conv(env);

        wgpu::BindGroup bg{};
        uint32_t* offsets = nullptr;
        uint32_t num_offsets = 0;
        if (!conv(bg, bindGroup) || !conv(offsets, num_offsets, dynamicOffsets)) {
            return;
        }

        enc_.SetBindGroup(index, bg, num_offsets, offsets);
    }

    void GPUComputePassEncoder::setBindGroup(Napi::Env env,
                                             interop::GPUIndex32 index,
                                             interop::Interface<interop::GPUBindGroup> bindGroup,
                                             interop::Uint32Array dynamicOffsetsData,
                                             interop::GPUSize64 dynamicOffsetsDataStart,
                                             interop::GPUSize32 dynamicOffsetsDataLength) {
        Converter conv(env);

        wgpu::BindGroup bg{};
        if (!conv(bg, bindGroup)) {
            return;
        }

        enc_.SetBindGroup(index, bg, dynamicOffsetsDataLength,
                          dynamicOffsetsData.Data() + dynamicOffsetsDataStart);
    }

    void GPUComputePassEncoder::pushDebugGroup(Napi::Env, std::string groupLabel) {
        enc_.PushDebugGroup(groupLabel.c_str());
    }

    void GPUComputePassEncoder::popDebugGroup(Napi::Env) {
        enc_.PopDebugGroup();
    }

    void GPUComputePassEncoder::insertDebugMarker(Napi::Env, std::string markerLabel) {
        enc_.InsertDebugMarker(markerLabel.c_str());
    }

    std::optional<std::string> GPUComputePassEncoder::getLabel(Napi::Env) {
        UNIMPLEMENTED();
    }

    void GPUComputePassEncoder::setLabel(Napi::Env, std::optional<std::string> value) {
        UNIMPLEMENTED();
    }

}}  // namespace wgpu::binding
