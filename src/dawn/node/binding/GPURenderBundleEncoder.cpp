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

#include "src/dawn/node/binding/GPURenderBundleEncoder.h"

#include <utility>

#include "src/dawn/node/binding/Converter.h"
#include "src/dawn/node/binding/GPUBindGroup.h"
#include "src/dawn/node/binding/GPUBuffer.h"
#include "src/dawn/node/binding/GPURenderBundle.h"
#include "src/dawn/node/binding/GPURenderPipeline.h"
#include "src/dawn/node/utils/Debug.h"

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPURenderBundleEncoder
////////////////////////////////////////////////////////////////////////////////
GPURenderBundleEncoder::GPURenderBundleEncoder(wgpu::RenderBundleEncoder enc)
    : enc_(std::move(enc)) {}

interop::Interface<interop::GPURenderBundle> GPURenderBundleEncoder::finish(
    Napi::Env env,
    interop::GPURenderBundleDescriptor descriptor) {
    wgpu::RenderBundleDescriptor desc{};

    return interop::GPURenderBundle::Create<GPURenderBundle>(env, enc_.Finish(&desc));
}

void GPURenderBundleEncoder::setBindGroup(
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

void GPURenderBundleEncoder::setPipeline(Napi::Env env,
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
                                            interop::GPUSize64 offset,
                                            std::optional<interop::GPUSize64> size) {
    Converter conv(env);

    wgpu::Buffer b{};
    wgpu::IndexFormat f{};
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

void GPURenderBundleEncoder::setVertexBuffer(Napi::Env env,
                                             interop::GPUIndex32 slot,
                                             interop::Interface<interop::GPUBuffer> buffer,
                                             interop::GPUSize64 offset,
                                             std::optional<interop::GPUSize64> size) {
    Converter conv(env);

    wgpu::Buffer b{};
    uint64_t s = wgpu::kWholeSize;
    if (!conv(b, buffer) || !conv(s, size)) {
        return;
    }
    enc_.SetVertexBuffer(slot, b, offset, s);
}

void GPURenderBundleEncoder::draw(Napi::Env env,
                                  interop::GPUSize32 vertexCount,
                                  interop::GPUSize32 instanceCount,
                                  interop::GPUSize32 firstVertex,
                                  interop::GPUSize32 firstInstance) {
    enc_.Draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void GPURenderBundleEncoder::drawIndexed(Napi::Env env,
                                         interop::GPUSize32 indexCount,
                                         interop::GPUSize32 instanceCount,
                                         interop::GPUSize32 firstIndex,
                                         interop::GPUSignedOffset32 baseVertex,
                                         interop::GPUSize32 firstInstance) {
    enc_.DrawIndexed(indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
}

void GPURenderBundleEncoder::drawIndirect(Napi::Env env,
                                          interop::Interface<interop::GPUBuffer> indirectBuffer,
                                          interop::GPUSize64 indirectOffset) {
    Converter conv(env);

    wgpu::Buffer b{};
    uint64_t o = 0;

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
    uint64_t o = 0;

    if (!conv(b, indirectBuffer) ||  //
        !conv(o, indirectOffset)) {
        return;
    }
    enc_.DrawIndexedIndirect(b, o);
}

std::string GPURenderBundleEncoder::getLabel(Napi::Env) {
    UNIMPLEMENTED();
}

void GPURenderBundleEncoder::setLabel(Napi::Env, std::string value) {
    UNIMPLEMENTED();
}

}  // namespace wgpu::binding
