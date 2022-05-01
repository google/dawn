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

#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "dawn/dawn_proc.h"
#include "src/dawn/node/binding/Flags.h"
#include "src/dawn/node/binding/GPU.h"

namespace {
Napi::Value CreateGPU(const Napi::CallbackInfo& info) {
    const auto& env = info.Env();

    std::tuple<std::vector<std::string>> args;
    auto res = wgpu::interop::FromJS(info, args);
    if (res != wgpu::interop::Success) {
        Napi::Error::New(env, res.error).ThrowAsJavaScriptException();
        return env.Undefined();
    }

    wgpu::binding::Flags flags;

    // Parse out the key=value flags out of the input args array
    for (const auto& arg : std::get<0>(args)) {
        const size_t sep_index = arg.find("=");
        if (sep_index == std::string::npos) {
            Napi::Error::New(env, "Flags expected argument format is <key>=<value>")
                .ThrowAsJavaScriptException();
            return env.Undefined();
        }
        flags.Set(arg.substr(0, sep_index), arg.substr(sep_index + 1));
    }

    // Construct a wgpu::interop::GPU interface, implemented by wgpu::bindings::GPU.
    return wgpu::interop::GPU::Create<wgpu::binding::GPU>(env, std::move(flags));
}

}  // namespace

// Initialize() initializes the Dawn node module, registering all the WebGPU
// types into the global object, and adding the 'create' function on the exported
// object.
Napi::Object Initialize(Napi::Env env, Napi::Object exports) {
    // Begin by setting the Dawn procedure function pointers.
    dawnProcSetProcs(&dawn::native::GetProcs());

    // Register all the interop types
    wgpu::interop::Initialize(env);

    // Export function that creates and returns the wgpu::interop::GPU interface
    exports.Set(Napi::String::New(env, "create"), Napi::Function::New<CreateGPU>(env));

    return exports;
}

NODE_API_MODULE(addon, Initialize)
