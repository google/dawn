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

#include "src/dawn/node/binding/GPUShaderModule.h"

#include <memory>
#include <utility>
#include <vector>

#include "src/dawn/node/utils/Debug.h"

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUShaderModule
////////////////////////////////////////////////////////////////////////////////
GPUShaderModule::GPUShaderModule(const wgpu::ShaderModuleDescriptor& desc,
                                 wgpu::ShaderModule shader,
                                 std::shared_ptr<AsyncRunner> async)
    : shader_(std::move(shader)), async_(std::move(async)), label_(desc.label ? desc.label : "") {}

interop::Promise<interop::Interface<interop::GPUCompilationInfo>>
GPUShaderModule::getCompilationInfo(Napi::Env env) {
    struct GPUCompilationMessage : public interop::GPUCompilationMessage {
        WGPUCompilationMessage message;

        explicit GPUCompilationMessage(const WGPUCompilationMessage& m) : message(m) {}
        std::string getMessage(Napi::Env) override { return message.message; }
        interop::GPUCompilationMessageType getType(Napi::Env) override {
            switch (message.type) {
                case WGPUCompilationMessageType_Error:
                    return interop::GPUCompilationMessageType::kError;
                case WGPUCompilationMessageType_Warning:
                    return interop::GPUCompilationMessageType::kWarning;
                case WGPUCompilationMessageType_Info:
                    return interop::GPUCompilationMessageType::kInfo;
                default:
                    UNREACHABLE();
            }
        }
        uint64_t getLineNum(Napi::Env) override { return message.lineNum; }
        uint64_t getLinePos(Napi::Env) override { return message.linePos; }
        uint64_t getOffset(Napi::Env) override { return message.offset; }
        uint64_t getLength(Napi::Env) override { return message.length; }
    };

    using Messages = std::vector<interop::Interface<interop::GPUCompilationMessage>>;

    struct GPUCompilationInfo : public interop::GPUCompilationInfo {
        std::vector<Napi::ObjectReference> messages;

        GPUCompilationInfo(Napi::Env env, Messages msgs) {
            messages.reserve(msgs.size());
            for (auto& msg : msgs) {
                messages.emplace_back(Napi::Persistent(Napi::Object(env, msg)));
            }
        }
        Messages getMessages(Napi::Env) override {
            Messages out;
            out.reserve(messages.size());
            for (auto& msg : messages) {
                out.emplace_back(msg.Value());
            }
            return out;
        }
    };

    using Promise = interop::Promise<interop::Interface<interop::GPUCompilationInfo>>;

    struct Context {
        Napi::Env env;
        Promise promise;
        AsyncTask task;
    };
    auto ctx = new Context{env, Promise(env, PROMISE_INFO), AsyncTask(async_)};
    auto promise = ctx->promise;

    shader_.GetCompilationInfo(
        [](WGPUCompilationInfoRequestStatus status, WGPUCompilationInfo const* compilationInfo,
           void* userdata) {
            auto c = std::unique_ptr<Context>(static_cast<Context*>(userdata));

            Messages messages(compilationInfo->messageCount);
            for (uint32_t i = 0; i < compilationInfo->messageCount; i++) {
                auto& msg = compilationInfo->messages[i];
                messages[i] =
                    interop::GPUCompilationMessage::Create<GPUCompilationMessage>(c->env, msg);
            }

            c->promise.Resolve(interop::GPUCompilationInfo::Create<GPUCompilationInfo>(
                c->env, c->env, std::move(messages)));
        },
        ctx);

    return promise;
}

std::string GPUShaderModule::getLabel(Napi::Env) {
    return label_;
}

void GPUShaderModule::setLabel(Napi::Env, std::string value) {
    shader_.SetLabel(value.c_str());
    label_ = value;
}

}  // namespace wgpu::binding
