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

#include "src/dawn/node/binding/GPUBuffer.h"

#include <memory>
#include <utility>

#include "src/dawn/node/binding/Converter.h"
#include "src/dawn/node/binding/Errors.h"
#include "src/dawn/node/utils/Debug.h"

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUBuffer
// TODO(crbug.com/dawn/1134): We may be doing more validation here than necessary. Once CTS is
// robustly passing, pull out validation and see what / if breaks.
////////////////////////////////////////////////////////////////////////////////
GPUBuffer::GPUBuffer(wgpu::Buffer buffer,
                     wgpu::BufferDescriptor desc,
                     wgpu::Device device,
                     std::shared_ptr<AsyncRunner> async)
    : buffer_(std::move(buffer)),
      desc_(desc),
      device_(std::move(device)),
      async_(std::move(async)) {
    if (desc.mappedAtCreation) {
        state_ = State::MappedAtCreation;
    }
}

interop::Promise<void> GPUBuffer::mapAsync(Napi::Env env,
                                           interop::GPUMapModeFlags mode,
                                           interop::GPUSize64 offset,
                                           std::optional<interop::GPUSize64> size) {
    wgpu::MapMode md{};
    Converter conv(env);
    if (!conv(md, mode)) {
        interop::Promise<void> promise(env, PROMISE_INFO);
        promise.Reject(Errors::OperationError(env));
        return promise;
    }

    if (state_ != State::Unmapped) {
        interop::Promise<void> promise(env, PROMISE_INFO);
        promise.Reject(Errors::OperationError(env));
        device_.InjectError(wgpu::ErrorType::Validation,
                            "mapAsync called on buffer that is not in the unmapped state");
        return promise;
    }

    struct Context {
        Napi::Env env;
        interop::Promise<void> promise;
        AsyncTask task;
        State& state;
    };
    auto ctx =
        new Context{env, interop::Promise<void>(env, PROMISE_INFO), AsyncTask(async_), state_};
    auto promise = ctx->promise;

    uint64_t s = size.has_value() ? size.value().value : (desc_.size - offset);

    state_ = State::MappingPending;

    buffer_.MapAsync(
        md, offset, s,
        [](WGPUBufferMapAsyncStatus status, void* userdata) {
            auto c = std::unique_ptr<Context>(static_cast<Context*>(userdata));
            c->state = State::Unmapped;
            switch (status) {
                case WGPUBufferMapAsyncStatus_Force32:
                    UNREACHABLE("WGPUBufferMapAsyncStatus_Force32");
                    break;
                case WGPUBufferMapAsyncStatus_Success:
                    c->promise.Resolve();
                    c->state = State::Mapped;
                    break;
                case WGPUBufferMapAsyncStatus_Error:
                    c->promise.Reject(Errors::OperationError(c->env));
                    break;
                case WGPUBufferMapAsyncStatus_UnmappedBeforeCallback:
                case WGPUBufferMapAsyncStatus_DestroyedBeforeCallback:
                    c->promise.Reject(Errors::AbortError(c->env));
                    break;
                case WGPUBufferMapAsyncStatus_Unknown:
                case WGPUBufferMapAsyncStatus_DeviceLost:
                    // TODO(dawn:1123): The spec is a bit vague around what the promise should
                    // do here.
                    c->promise.Reject(Errors::UnknownError(c->env));
                    break;
            }
        },
        ctx);

    return promise;
}

interop::ArrayBuffer GPUBuffer::getMappedRange(Napi::Env env,
                                               interop::GPUSize64 offset,
                                               std::optional<interop::GPUSize64> size) {
    if (state_ != State::Mapped && state_ != State::MappedAtCreation) {
        Errors::OperationError(env).ThrowAsJavaScriptException();
        return {};
    }

    uint64_t s = size.has_value() ? size.value().value : (desc_.size - offset);

    uint64_t start = offset;
    uint64_t end = offset + s;
    for (auto& mapping : mapped_) {
        if (mapping.Intersects(start, end)) {
            Errors::OperationError(env).ThrowAsJavaScriptException();
            return {};
        }
    }

    auto* ptr = (desc_.usage & wgpu::BufferUsage::MapWrite)
                    ? buffer_.GetMappedRange(offset, s)
                    : const_cast<void*>(buffer_.GetConstMappedRange(offset, s));
    if (!ptr) {
        Errors::OperationError(env).ThrowAsJavaScriptException();
        return {};
    }
    auto array_buffer = Napi::ArrayBuffer::New(env, ptr, s);
    // TODO(crbug.com/dawn/1135): Ownership here is the wrong way around.
    mapped_.emplace_back(Mapping{start, end, Napi::Persistent(array_buffer)});
    return array_buffer;
}

void GPUBuffer::unmap(Napi::Env env) {
    buffer_.Unmap();

    if (state_ != State::Destroyed && state_ != State::Unmapped) {
        DetachMappings();
        state_ = State::Unmapped;
    }
}

void GPUBuffer::destroy(Napi::Env) {
    if (state_ == State::Destroyed) {
        return;
    }

    if (state_ != State::Unmapped) {
        DetachMappings();
    }

    buffer_.Destroy();
    state_ = State::Destroyed;
}

interop::GPUSize64 GPUBuffer::getSize(Napi::Env) {
    return buffer_.GetSize();
}

interop::GPUBufferMapState GPUBuffer::getMapState(Napi::Env) {
    UNIMPLEMENTED();
}

interop::GPUBufferUsageFlags GPUBuffer::getUsage(Napi::Env env) {
    interop::GPUBufferUsageFlags result;

    Converter conv(env);
    if (!conv(result, buffer_.GetUsage())) {
        Napi::Error::New(env, "Couldn't convert usage to a JavaScript value.")
            .ThrowAsJavaScriptException();
        return {0u};  // Doesn't get used.
    }

    return result;
}

void GPUBuffer::DetachMappings() {
    for (auto& mapping : mapped_) {
        mapping.buffer.Value().Detach();
    }
    mapped_.clear();
}

std::string GPUBuffer::getLabel(Napi::Env) {
    UNIMPLEMENTED();
}

void GPUBuffer::setLabel(Napi::Env, std::string value) {
    UNIMPLEMENTED();
}

}  // namespace wgpu::binding
