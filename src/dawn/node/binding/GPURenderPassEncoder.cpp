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

#include "src/dawn/node/binding/GPURenderPassEncoder.h"

#include <utility>

#include "src/dawn/node/binding/Converter.h"
#include "src/dawn/node/binding/GPUBindGroup.h"
#include "src/dawn/node/binding/GPUBuffer.h"
#include "src/dawn/node/binding/GPUQuerySet.h"
#include "src/dawn/node/binding/GPURenderBundle.h"
#include "src/dawn/node/binding/GPURenderPipeline.h"
#include "src/dawn/node/utils/Debug.h"

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPURenderPassEncoder
////////////////////////////////////////////////////////////////////////////////
GPURenderPassEncoder::GPURenderPassEncoder(wgpu::RenderPassEncoder enc) : enc_(std::move(enc)) {}

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

void GPURenderPassEncoder::end(Napi::Env) {
    enc_.End();
}

void GPURenderPassEncoder::setBindGroup(
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

    if (dynamicOffsetsDataStart > dynamicOffsetsData.ElementLength()) {
        Napi::RangeError::New(env, "dynamicOffsetsDataStart is out of bound of dynamicOffsetData")
            .ThrowAsJavaScriptException();
        return;
    }

    if (dynamicOffsetsDataLength > dynamicOffsetsData.ElementLength() - dynamicOffsetsDataStart) {
        Napi::RangeError::New(env,
                              "dynamicOffsetsDataLength + dynamicOffsetsDataStart is out of "
                              "bound of dynamicOffsetData")
            .ThrowAsJavaScriptException();
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

void GPURenderPassEncoder::setPipeline(Napi::Env env,
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
                                          interop::GPUSize64 offset,
                                          std::optional<interop::GPUSize64> size) {
    Converter conv(env);

    wgpu::Buffer b{};
    wgpu::IndexFormat f;
    uint64_t s = wgpu::kWholeSize;
    if (!conv(b, buffer) ||       //
        !conv(f, indexFormat) ||  //
        !conv(s, size)) {
        return;
    }
    enc_.SetIndexBuffer(b, f, offset, s);
}

void GPURenderPassEncoder::setVertexBuffer(Napi::Env env,
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

void GPURenderPassEncoder::draw(Napi::Env env,
                                interop::GPUSize32 vertexCount,
                                interop::GPUSize32 instanceCount,
                                interop::GPUSize32 firstVertex,
                                interop::GPUSize32 firstInstance) {
    enc_.Draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void GPURenderPassEncoder::drawIndexed(Napi::Env env,
                                       interop::GPUSize32 indexCount,
                                       interop::GPUSize32 instanceCount,
                                       interop::GPUSize32 firstIndex,
                                       interop::GPUSignedOffset32 baseVertex,
                                       interop::GPUSize32 firstInstance) {
    enc_.DrawIndexed(indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
}

void GPURenderPassEncoder::drawIndirect(Napi::Env env,
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

void GPURenderPassEncoder::drawIndexedIndirect(
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

std::string GPURenderPassEncoder::getLabel(Napi::Env) {
    UNIMPLEMENTED();
}

void GPURenderPassEncoder::setLabel(Napi::Env, std::string value) {
    UNIMPLEMENTED();
}

}  // namespace wgpu::binding
