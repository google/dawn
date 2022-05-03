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

#include "src/dawn/node/binding/GPUQueue.h"

#include <cassert>
#include <limits>
#include <memory>
#include <utility>

#include "src/dawn/node/binding/Converter.h"
#include "src/dawn/node/binding/GPUBuffer.h"
#include "src/dawn/node/binding/GPUCommandBuffer.h"
#include "src/dawn/node/utils/Debug.h"

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUQueue
////////////////////////////////////////////////////////////////////////////////
GPUQueue::GPUQueue(wgpu::Queue queue, std::shared_ptr<AsyncRunner> async)
    : queue_(std::move(queue)), async_(std::move(async)) {}

void GPUQueue::submit(Napi::Env env,
                      std::vector<interop::Interface<interop::GPUCommandBuffer>> commandBuffers) {
    std::vector<wgpu::CommandBuffer> bufs(commandBuffers.size());
    for (size_t i = 0; i < commandBuffers.size(); i++) {
        bufs[i] = *commandBuffers[i].As<GPUCommandBuffer>();
    }
    Converter conv(env);
    uint32_t bufs_size;
    if (!conv(bufs_size, bufs.size())) {
        return;
    }
    queue_.Submit(bufs_size, bufs.data());
}

interop::Promise<void> GPUQueue::onSubmittedWorkDone(Napi::Env env) {
    struct Context {
        Napi::Env env;
        interop::Promise<void> promise;
        AsyncTask task;
    };
    auto ctx = new Context{env, interop::Promise<void>(env, PROMISE_INFO), AsyncTask(async_)};
    auto promise = ctx->promise;

    queue_.OnSubmittedWorkDone(
        0,
        [](WGPUQueueWorkDoneStatus status, void* userdata) {
            auto c = std::unique_ptr<Context>(static_cast<Context*>(userdata));
            if (status != WGPUQueueWorkDoneStatus::WGPUQueueWorkDoneStatus_Success) {
                Napi::Error::New(c->env, "onSubmittedWorkDone() failed")
                    .ThrowAsJavaScriptException();
            }
            c->promise.Resolve();
        },
        ctx);

    return promise;
}

void GPUQueue::writeBuffer(Napi::Env env,
                           interop::Interface<interop::GPUBuffer> buffer,
                           interop::GPUSize64 bufferOffset,
                           interop::BufferSource data,
                           interop::GPUSize64 dataOffsetElements,
                           std::optional<interop::GPUSize64> sizeElements) {
    wgpu::Buffer buf = *buffer.As<GPUBuffer>();
    Converter::BufferSource src{};
    Converter conv(env);
    if (!conv(src, data)) {
        return;
    }

    // Note that in the JS semantics of WebGPU, writeBuffer works in number of elements of the
    // typed arrays.
    if (dataOffsetElements > uint64_t(src.size / src.bytesPerElement)) {
        binding::Errors::OperationError(env, "dataOffset is larger than data's size.")
            .ThrowAsJavaScriptException();
        return;
    }
    uint64_t dataOffset = dataOffsetElements * src.bytesPerElement;
    src.data = reinterpret_cast<uint8_t*>(src.data) + dataOffset;
    src.size -= dataOffset;

    // Size defaults to dataSize - dataOffset. Instead of computing in elements, we directly
    // use it in bytes, and convert the provided value, if any, in bytes.
    uint64_t size64 = uint64_t(src.size);
    if (sizeElements.has_value()) {
        if (sizeElements.value() > std::numeric_limits<uint64_t>::max() / src.bytesPerElement) {
            binding::Errors::OperationError(env, "size overflows.").ThrowAsJavaScriptException();
            return;
        }
        size64 = sizeElements.value() * src.bytesPerElement;
    }

    if (size64 > uint64_t(src.size)) {
        binding::Errors::OperationError(env, "size + dataOffset is larger than data's size.")
            .ThrowAsJavaScriptException();
        return;
    }

    if (size64 % 4 != 0) {
        binding::Errors::OperationError(env, "size is not a multiple of 4 bytes.")
            .ThrowAsJavaScriptException();
        return;
    }

    assert(size64 <= std::numeric_limits<size_t>::max());
    queue_.WriteBuffer(buf, bufferOffset, src.data, static_cast<size_t>(size64));
}

void GPUQueue::writeTexture(Napi::Env env,
                            interop::GPUImageCopyTexture destination,
                            interop::BufferSource data,
                            interop::GPUImageDataLayout dataLayout,
                            interop::GPUExtent3D size) {
    wgpu::ImageCopyTexture dst{};
    Converter::BufferSource src{};
    wgpu::TextureDataLayout layout{};
    wgpu::Extent3D sz{};
    Converter conv(env);
    if (!conv(dst, destination) ||    //
        !conv(src, data) ||           //
        !conv(layout, dataLayout) ||  //
        !conv(sz, size)) {
        return;
    }

    queue_.WriteTexture(&dst, src.data, src.size, &layout, &sz);
}

void GPUQueue::copyExternalImageToTexture(Napi::Env,
                                          interop::GPUImageCopyExternalImage source,
                                          interop::GPUImageCopyTextureTagged destination,
                                          interop::GPUExtent3D copySize) {
    UNIMPLEMENTED();
}

std::string GPUQueue::getLabel(Napi::Env) {
    UNIMPLEMENTED();
}

void GPUQueue::setLabel(Napi::Env, std::string value) {
    UNIMPLEMENTED();
}

}  // namespace wgpu::binding
