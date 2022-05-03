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

#include "src/dawn/node/binding/GPUShaderModule.h"

#include <memory>
#include <utility>
#include <vector>

#include "src/dawn/node/utils/Debug.h"

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUShaderModule
////////////////////////////////////////////////////////////////////////////////
GPUShaderModule::GPUShaderModule(wgpu::ShaderModule shader, std::shared_ptr<AsyncRunner> async)
    : shader_(std::move(shader)), async_(std::move(async)) {}

interop::Promise<interop::Interface<interop::GPUCompilationInfo>> GPUShaderModule::compilationInfo(
    Napi::Env env) {
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
                    UNIMPLEMENTED();
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
    UNIMPLEMENTED();
}

void GPUShaderModule::setLabel(Napi::Env, std::string value) {
    UNIMPLEMENTED();
}

}  // namespace wgpu::binding
