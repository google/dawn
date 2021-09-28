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

#include "src/dawn_node/binding/GPURenderPassEncoder.h"

#include "src/dawn_node/binding/Converter.h"
#include "src/dawn_node/binding/GPUBindGroup.h"
#include "src/dawn_node/binding/GPUBuffer.h"
#include "src/dawn_node/binding/GPUQuerySet.h"
#include "src/dawn_node/binding/GPURenderBundle.h"
#include "src/dawn_node/binding/GPURenderPipeline.h"
#include "src/dawn_node/utils/Debug.h"

namespace wgpu { namespace binding {

    ////////////////////////////////////////////////////////////////////////////////
    // wgpu::bindings::GPURenderPassEncoder
    ////////////////////////////////////////////////////////////////////////////////
    GPURenderPassEncoder::GPURenderPassEncoder(wgpu::RenderPassEncoder enc) : enc_(std::move(enc)) {
    }

    void GPURenderPassEncoder::setViewport(Napi::Env,
                                           float x,
                                           float y,
                                           float width,
                                           float height,
                                           float minDepth,
                                           float maxDepth) {
        enc_.SetViewport(x, y, width, height, minDepth, maxDepth);
    }

    void GPURenderPassEncoder::setScissorRect(Napi::Env,
                                              interop::GPUIntegerCoordinate x,
                                              interop::GPUIntegerCoordinate y,
                                              interop::GPUIntegerCoordinate width,
                                              interop::GPUIntegerCoordinate height) {
        enc_.SetScissorRect(x, y, width, height);
    }

    void GPURenderPassEncoder::setBlendConstant(Napi::Env env, interop::GPUColor color) {
        Converter conv(env);

        wgpu::Color c{};
        if (!conv(c, color)) {
            return;
        }

        enc_.SetBlendConstant(&c);
    }

    void GPURenderPassEncoder::setStencilReference(Napi::Env, interop::GPUStencilValue reference) {
        enc_.SetStencilReference(reference);
    }

    void GPURenderPassEncoder::beginOcclusionQuery(Napi::Env, interop::GPUSize32 queryIndex) {
        enc_.BeginOcclusionQuery(queryIndex);
    }

    void GPURenderPassEncoder::endOcclusionQuery(Napi::Env) {
        enc_.EndOcclusionQuery();
    }

    void GPURenderPassEncoder::beginPipelineStatisticsQuery(
        Napi::Env,
        interop::Interface<interop::GPUQuerySet> querySet,
        interop::GPUSize32 queryIndex) {
        UNIMPLEMENTED();
    }

    void GPURenderPassEncoder::endPipelineStatisticsQuery(Napi::Env) {
        UNIMPLEMENTED();
    }

    void GPURenderPassEncoder::writeTimestamp(Napi::Env env,
                                              interop::Interface<interop::GPUQuerySet> querySet,
                                              interop::GPUSize32 queryIndex) {
        Converter conv(env);

        wgpu::QuerySet q{};
        if (!conv(q, querySet)) {
            return;
        }

        enc_.WriteTimestamp(q, queryIndex);
    }

    void GPURenderPassEncoder::executeBundles(
        Napi::Env env,
        std::vector<interop::Interface<interop::GPURenderBundle>> bundles_in) {
        Converter conv(env);

        wgpu::RenderBundle* bundles = nullptr;
        uint32_t bundleCount = 0;
        if (!conv(bundles, bundleCount, bundles_in)) {
            return;
        }

        enc_.ExecuteBundles(bundleCount, bundles);
    }

    void GPURenderPassEncoder::endPass(Napi::Env) {
        enc_.EndPass();
    }

    void GPURenderPassEncoder::setBindGroup(
        Napi::Env env,
        interop::GPUIndex32 index,
        interop::Interface<interop::GPUBindGroup> bindGroup,
        std::optional<std::vector<interop::GPUBufferDynamicOffset>> dynamicOffsets) {
        Converter conv(env);

        wgpu::BindGroup bg{};
        if (!conv(bg, bindGroup)) {
            return;
        }
        uint32_t* offsets = nullptr;
        uint32_t offset_count = 0;
        if (dynamicOffsets.has_value() && dynamicOffsets->size() > 0) {
            if (!conv(offsets, offset_count, dynamicOffsets.value())) {
                return;
            }
        }
        enc_.SetBindGroup(index, bg, offset_count, offsets);
    }

    void GPURenderPassEncoder::setBindGroup(Napi::Env env,
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

    void GPURenderPassEncoder::pushDebugGroup(Napi::Env, std::string groupLabel) {
        enc_.PushDebugGroup(groupLabel.c_str());
    }

    void GPURenderPassEncoder::popDebugGroup(Napi::Env) {
        enc_.PopDebugGroup();
    }

    void GPURenderPassEncoder::insertDebugMarker(Napi::Env, std::string markerLabel) {
        enc_.InsertDebugMarker(markerLabel.c_str());
    }

    void GPURenderPassEncoder::setPipeline(
        Napi::Env env,
        interop::Interface<interop::GPURenderPipeline> pipeline) {
        Converter conv(env);
        wgpu::RenderPipeline rp{};
        if (!conv(rp, pipeline)) {
            return;
        }
        enc_.SetPipeline(rp);
    }

    void GPURenderPassEncoder::setIndexBuffer(Napi::Env env,
                                              interop::Interface<interop::GPUBuffer> buffer,
                                              interop::GPUIndexFormat indexFormat,
                                              std::optional<interop::GPUSize64> offset,
                                              std::optional<interop::GPUSize64> size) {
        Converter conv(env);

        wgpu::Buffer b{};
        wgpu::IndexFormat f;
        uint64_t o = 0;
        uint64_t s = wgpu::kWholeSize;
        if (!conv(b, buffer) ||       //
            !conv(f, indexFormat) ||  //
            !conv(o, offset) ||       //
            !conv(s, size)) {
            return;
        }
        enc_.SetIndexBuffer(b, f, o, s);
    }

    void GPURenderPassEncoder::setVertexBuffer(Napi::Env env,
                                               interop::GPUIndex32 slot,
                                               interop::Interface<interop::GPUBuffer> buffer,
                                               std::optional<interop::GPUSize64> offset,
                                               std::optional<interop::GPUSize64> size) {
        Converter conv(env);

        wgpu::Buffer b{};
        uint64_t o = 0;
        uint64_t s = wgpu::kWholeSize;
        if (!conv(b, buffer) ||  //
            !conv(o, offset) ||  //
            !conv(s, size)) {
            return;
        }
        enc_.SetVertexBuffer(slot, b, o, s);
    }

    void GPURenderPassEncoder::draw(Napi::Env env,
                                    interop::GPUSize32 vertexCount,
                                    std::optional<interop::GPUSize32> instanceCount,
                                    std::optional<interop::GPUSize32> firstVertex,
                                    std::optional<interop::GPUSize32> firstInstance) {
        Converter conv(env);

        uint32_t vc = 0;
        uint32_t ic = 1;
        uint32_t fv = 0;
        uint32_t fi = 0;
        if (!conv(vc, vertexCount) ||    //
            !conv(ic, instanceCount) ||  //
            !conv(fv, firstVertex) ||    //
            !conv(fi, firstInstance)) {
            return;
        }

        enc_.Draw(vc, ic, fv, fi);
    }

    void GPURenderPassEncoder::drawIndexed(Napi::Env env,
                                           interop::GPUSize32 indexCount,
                                           std::optional<interop::GPUSize32> instanceCount,
                                           std::optional<interop::GPUSize32> firstIndex,
                                           std::optional<interop::GPUSignedOffset32> baseVertex,
                                           std::optional<interop::GPUSize32> firstInstance) {
        Converter conv(env);

        uint32_t idx_c = 0;
        uint32_t ins_c = 1;
        uint32_t f_idx = 0;
        int32_t bv = 0;
        uint32_t f_ins = 0;

        if (!conv(idx_c, indexCount) ||     //
            !conv(ins_c, instanceCount) ||  //
            !conv(f_idx, firstIndex) ||     //
            !conv(bv, baseVertex) ||        //
            !conv(f_ins, firstInstance)) {
            return;
        }

        enc_.DrawIndexed(idx_c, ins_c, f_idx, bv, f_ins);
    }

    void GPURenderPassEncoder::drawIndirect(Napi::Env env,
                                            interop::Interface<interop::GPUBuffer> indirectBuffer,
                                            interop::GPUSize64 indirectOffset) {
        Converter conv(env);

        wgpu::Buffer b{};
        uint32_t o = 0;

        if (!conv(b, indirectBuffer) ||  //
            !conv(o, indirectOffset)) {
            return;
        }
        enc_.DrawIndirect(b, o);
    }

    void GPURenderPassEncoder::drawIndexedIndirect(
        Napi::Env env,
        interop::Interface<interop::GPUBuffer> indirectBuffer,
        interop::GPUSize64 indirectOffset) {
        Converter conv(env);

        wgpu::Buffer b{};
        uint32_t o = 0;

        if (!conv(b, indirectBuffer) ||  //
            !conv(o, indirectOffset)) {
            return;
        }
        enc_.DrawIndexedIndirect(b, o);
    }

    std::optional<std::string> GPURenderPassEncoder::getLabel(Napi::Env) {
        UNIMPLEMENTED();
    }

    void GPURenderPassEncoder::setLabel(Napi::Env, std::optional<std::string> value) {
        UNIMPLEMENTED();
    }

}}  // namespace wgpu::binding
