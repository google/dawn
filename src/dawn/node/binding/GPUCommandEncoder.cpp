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

#include "src/dawn/node/binding/GPUCommandEncoder.h"

#include <utility>

#include "src/dawn/node/binding/Converter.h"
#include "src/dawn/node/binding/GPU.h"
#include "src/dawn/node/binding/GPUBuffer.h"
#include "src/dawn/node/binding/GPUCommandBuffer.h"
#include "src/dawn/node/binding/GPUComputePassEncoder.h"
#include "src/dawn/node/binding/GPUQuerySet.h"
#include "src/dawn/node/binding/GPURenderPassEncoder.h"
#include "src/dawn/node/binding/GPUTexture.h"
#include "src/dawn/node/utils/Debug.h"

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUCommandEncoder
////////////////////////////////////////////////////////////////////////////////
GPUCommandEncoder::GPUCommandEncoder(wgpu::Device device, wgpu::CommandEncoder enc)
    : device_(std::move(device)), enc_(std::move(enc)) {}

interop::Interface<interop::GPURenderPassEncoder> GPUCommandEncoder::beginRenderPass(
    Napi::Env env,
    interop::GPURenderPassDescriptor descriptor) {
    Converter conv(env, device_);

    wgpu::RenderPassDescriptor desc{};
    wgpu::RenderPassDescriptorMaxDrawCount maxDrawCountDesc{};
    desc.nextInChain = &maxDrawCountDesc;

    if (!conv(desc.colorAttachments, desc.colorAttachmentCount, descriptor.colorAttachments) ||
        !conv(desc.depthStencilAttachment, descriptor.depthStencilAttachment) ||
        !conv(desc.label, descriptor.label) ||
        !conv(desc.occlusionQuerySet, descriptor.occlusionQuerySet) ||
        !conv(desc.timestampWrites, desc.timestampWriteCount, descriptor.timestampWrites) ||
        !conv(maxDrawCountDesc.maxDrawCount, descriptor.maxDrawCount)) {
        return {};
    }

    return interop::GPURenderPassEncoder::Create<GPURenderPassEncoder>(env,
                                                                       enc_.BeginRenderPass(&desc));
}

interop::Interface<interop::GPUComputePassEncoder> GPUCommandEncoder::beginComputePass(
    Napi::Env env,
    interop::GPUComputePassDescriptor descriptor) {
    Converter conv(env);

    wgpu::ComputePassDescriptor desc{};
    if (!conv(desc.timestampWrites, desc.timestampWriteCount, descriptor.timestampWrites)) {
        return {};
    }

    return interop::GPUComputePassEncoder::Create<GPUComputePassEncoder>(
        env, enc_.BeginComputePass(&desc));
}

void GPUCommandEncoder::clearBuffer(Napi::Env env,
                                    interop::Interface<interop::GPUBuffer> buffer,
                                    interop::GPUSize64 offset,
                                    std::optional<interop::GPUSize64> size) {
    Converter conv(env);

    wgpu::Buffer b{};
    uint64_t s = wgpu::kWholeSize;
    if (!conv(b, buffer) ||  //
        !conv(s, size)) {
        return;
    }

    enc_.ClearBuffer(b, offset, s);
}

void GPUCommandEncoder::copyBufferToBuffer(Napi::Env env,
                                           interop::Interface<interop::GPUBuffer> source,
                                           interop::GPUSize64 sourceOffset,
                                           interop::Interface<interop::GPUBuffer> destination,
                                           interop::GPUSize64 destinationOffset,
                                           interop::GPUSize64 size) {
    Converter conv(env);

    wgpu::Buffer src{};
    wgpu::Buffer dst{};
    if (!conv(src, source) ||  //
        !conv(dst, destination)) {
        return;
    }

    enc_.CopyBufferToBuffer(src, sourceOffset, dst, destinationOffset, size);
}

void GPUCommandEncoder::copyBufferToTexture(Napi::Env env,
                                            interop::GPUImageCopyBuffer source,
                                            interop::GPUImageCopyTexture destination,
                                            interop::GPUExtent3D copySize) {
    Converter conv(env);

    wgpu::ImageCopyBuffer src{};
    wgpu::ImageCopyTexture dst{};
    wgpu::Extent3D size{};
    if (!conv(src, source) ||       //
        !conv(dst, destination) ||  //
        !conv(size, copySize)) {
        return;
    }

    enc_.CopyBufferToTexture(&src, &dst, &size);
}

void GPUCommandEncoder::copyTextureToBuffer(Napi::Env env,
                                            interop::GPUImageCopyTexture source,
                                            interop::GPUImageCopyBuffer destination,
                                            interop::GPUExtent3D copySize) {
    Converter conv(env);

    wgpu::ImageCopyTexture src{};
    wgpu::ImageCopyBuffer dst{};
    wgpu::Extent3D size{};
    if (!conv(src, source) ||       //
        !conv(dst, destination) ||  //
        !conv(size, copySize)) {
        return;
    }

    enc_.CopyTextureToBuffer(&src, &dst, &size);
}

void GPUCommandEncoder::copyTextureToTexture(Napi::Env env,
                                             interop::GPUImageCopyTexture source,
                                             interop::GPUImageCopyTexture destination,
                                             interop::GPUExtent3D copySize) {
    Converter conv(env);

    wgpu::ImageCopyTexture src{};
    wgpu::ImageCopyTexture dst{};
    wgpu::Extent3D size{};
    if (!conv(src, source) ||       //
        !conv(dst, destination) ||  //
        !conv(size, copySize)) {
        return;
    }

    enc_.CopyTextureToTexture(&src, &dst, &size);
}

void GPUCommandEncoder::pushDebugGroup(Napi::Env, std::string groupLabel) {
    enc_.PushDebugGroup(groupLabel.c_str());
}

void GPUCommandEncoder::popDebugGroup(Napi::Env) {
    enc_.PopDebugGroup();
}

void GPUCommandEncoder::insertDebugMarker(Napi::Env, std::string markerLabel) {
    enc_.InsertDebugMarker(markerLabel.c_str());
}

void GPUCommandEncoder::writeTimestamp(Napi::Env env,
                                       interop::Interface<interop::GPUQuerySet> querySet,
                                       interop::GPUSize32 queryIndex) {
    if (!device_.HasFeature(wgpu::FeatureName::TimestampQuery)) {
        Napi::TypeError::New(env, "timestamp-query feature is not enabled.")
            .ThrowAsJavaScriptException();
        return;
    }

    Converter conv(env);

    wgpu::QuerySet q{};
    if (!conv(q, querySet)) {
        return;
    }

    enc_.WriteTimestamp(q, queryIndex);
}

void GPUCommandEncoder::resolveQuerySet(Napi::Env env,
                                        interop::Interface<interop::GPUQuerySet> querySet,
                                        interop::GPUSize32 firstQuery,
                                        interop::GPUSize32 queryCount,
                                        interop::Interface<interop::GPUBuffer> destination,
                                        interop::GPUSize64 destinationOffset) {
    Converter conv(env);

    wgpu::QuerySet q{};
    uint32_t f = 0;
    uint32_t c = 0;
    wgpu::Buffer b{};
    uint64_t o = 0;

    if (!conv(q, querySet) ||     //
        !conv(f, firstQuery) ||   //
        !conv(c, queryCount) ||   //
        !conv(b, destination) ||  //
        !conv(o, destinationOffset)) {
        return;
    }

    enc_.ResolveQuerySet(q, f, c, b, o);
}

interop::Interface<interop::GPUCommandBuffer> GPUCommandEncoder::finish(
    Napi::Env env,
    interop::GPUCommandBufferDescriptor descriptor) {
    wgpu::CommandBufferDescriptor desc{};
    return interop::GPUCommandBuffer::Create<GPUCommandBuffer>(env, enc_.Finish(&desc));
}

std::string GPUCommandEncoder::getLabel(Napi::Env) {
    UNIMPLEMENTED();
}

void GPUCommandEncoder::setLabel(Napi::Env, std::string value) {
    UNIMPLEMENTED();
}

}  // namespace wgpu::binding
