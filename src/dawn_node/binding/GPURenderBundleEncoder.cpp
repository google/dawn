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

#include "src/dawn_node/binding/GPURenderBundleEncoder.h"

#include "src/dawn_node/binding/Converter.h"
#include "src/dawn_node/binding/GPUBindGroup.h"
#include "src/dawn_node/binding/GPUBuffer.h"
#include "src/dawn_node/binding/GPURenderBundle.h"
#include "src/dawn_node/binding/GPURenderPipeline.h"
#include "src/dawn_node/utils/Debug.h"

namespace wgpu { namespace binding {

    ////////////////////////////////////////////////////////////////////////////////
    // wgpu::bindings::GPURenderBundleEncoder
    ////////////////////////////////////////////////////////////////////////////////
    GPURenderBundleEncoder::GPURenderBundleEncoder(wgpu::RenderBundleEncoder enc)
        : enc_(std::move(enc)) {
    }

    interop::Interface<interop::GPURenderBundle> GPURenderBundleEncoder::finish(
        Napi::Env env,
        std::optional<interop::GPURenderBundleDescriptor> descriptor) {
        wgpu::RenderBundleDescriptor desc{};

        return interop::GPURenderBundle::Create<GPURenderBundle>(env, enc_.Finish(&desc));
    }

    void GPURenderBundleEncoder::setBindGroup(
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

    void GPURenderBundleEncoder::setBindGroup(Napi::Env env,
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

    void GPURenderBundleEncoder::pushDebugGroup(Napi::Env, std::string groupLabel) {
        enc_.PushDebugGroup(groupLabel.c_str());
    }

    void GPURenderBundleEncoder::popDebugGroup(Napi::Env) {
        enc_.PopDebugGroup();
    }

    void GPURenderBundleEncoder::insertDebugMarker(Napi::Env, std::string markerLabel) {
        enc_.InsertDebugMarker(markerLabel.c_str());
    }

    void GPURenderBundleEncoder::setPipeline(
        Napi::Env env,
        interop::Interface<interop::GPURenderPipeline> pipeline) {
        Converter conv(env);

        wgpu::RenderPipeline p{};
        if (!conv(p, pipeline)) {
            return;
        }

        enc_.SetPipeline(p);
    }

    void GPURenderBundleEncoder::setIndexBuffer(Napi::Env env,
                                                interop::Interface<interop::GPUBuffer> buffer,
                                                interop::GPUIndexFormat indexFormat,
                                                std::optional<interop::GPUSize64> offset,
                                                std::optional<interop::GPUSize64> size) {
        Converter conv(env);

        wgpu::Buffer b{};
        wgpu::IndexFormat f{};
        uint64_t o = 0;
        uint64_t s = 0;
        if (!conv(b, buffer) ||       //
            !conv(f, indexFormat) ||  //
            !conv(o, offset) ||       //
            !conv(s, size)) {
            return;
        }

        enc_.SetIndexBuffer(b, f, o, s);
    }

    void GPURenderBundleEncoder::setVertexBuffer(Napi::Env env,
                                                 interop::GPUIndex32 slot,
                                                 interop::Interface<interop::GPUBuffer> buffer,
                                                 std::optional<interop::GPUSize64> offset,
                                                 std::optional<interop::GPUSize64> size) {
        Converter conv(env);

        uint32_t s = 0;
        wgpu::Buffer b{};
        uint64_t o = 0;
        uint64_t sz = 0;
        if (!conv(s, slot) ||    //
            !conv(b, buffer) ||  //
            !conv(o, offset) ||  //
            !conv(sz, size)) {
            return;
        }

        enc_.SetVertexBuffer(s, b, o, sz);
    }

    void GPURenderBundleEncoder::draw(Napi::Env env,
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

    void GPURenderBundleEncoder::drawIndexed(Napi::Env env,
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

    void GPURenderBundleEncoder::drawIndirect(Napi::Env env,
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

    void GPURenderBundleEncoder::drawIndexedIndirect(
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

    std::optional<std::string> GPURenderBundleEncoder::getLabel(Napi::Env) {
        UNIMPLEMENTED();
    }

    void GPURenderBundleEncoder::setLabel(Napi::Env, std::optional<std::string> value) {
        UNIMPLEMENTED();
    }

}}  // namespace wgpu::binding
