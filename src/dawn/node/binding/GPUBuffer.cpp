// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
      async_(std::move(async)),
      label_(desc.label ? desc.label : "") {
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
                case WGPUBufferMapAsyncStatus_ValidationError:
                    c->promise.Reject(Errors::OperationError(c->env));
                    break;
                case WGPUBufferMapAsyncStatus_UnmappedBeforeCallback:
                case WGPUBufferMapAsyncStatus_DestroyedBeforeCallback:
                case WGPUBufferMapAsyncStatus_MappingAlreadyPending:
                case WGPUBufferMapAsyncStatus_OffsetOutOfRange:
                case WGPUBufferMapAsyncStatus_SizeOutOfRange:
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

interop::GPUSize64Out GPUBuffer::getSize(Napi::Env) {
    return buffer_.GetSize();
}

interop::GPUBufferMapState GPUBuffer::getMapState(Napi::Env env) {
    interop::GPUBufferMapState result;

    Converter conv(env);
    if (!conv(result, buffer_.GetMapState())) {
        Napi::Error::New(env, "Couldn't convert usage to a JavaScript value.")
            .ThrowAsJavaScriptException();
        return interop::GPUBufferMapState::kUnmapped;
    }

    return result;
}

interop::GPUFlagsConstant GPUBuffer::getUsage(Napi::Env env) {
    interop::GPUBufferUsageFlags result;

    Converter conv(env);
    if (!conv(result, buffer_.GetUsage())) {
        Napi::Error::New(env, "Couldn't convert usage to a JavaScript value.")
            .ThrowAsJavaScriptException();
        return 0u;  // Doesn't get used.
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
    return label_;
}

void GPUBuffer::setLabel(Napi::Env, std::string value) {
    buffer_.SetLabel(value.c_str());
    label_ = value;
}

}  // namespace wgpu::binding
