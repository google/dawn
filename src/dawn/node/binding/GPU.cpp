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

#include "src/dawn/node/binding/GPU.h"

#include <algorithm>
#include <cstdlib>
#include <string>
#include <utility>

#include "src/dawn/node/binding/GPUAdapter.h"

#if defined(_WIN32)
#include <Windows.h>
#endif

namespace {
std::string GetEnvVar(const char* varName) {
#if defined(_WIN32)
    // Use _dupenv_s to avoid unsafe warnings about std::getenv
    char* value = nullptr;
    _dupenv_s(&value, nullptr, varName);
    if (value) {
        std::string result = value;
        free(value);
        return result;
    }
    return "";
#else
    if (auto* val = std::getenv(varName)) {
        return val;
    }
    return "";
#endif
}

void SetDllDir(const char* dir) {
    (void)dir;
#if defined(_WIN32)
    ::SetDllDirectory(dir);
#endif
}

struct BackendInfo {
    const char* const name;
    const char* const alias;  // may be nullptr
    wgpu::BackendType const backend;
};

constexpr BackendInfo kBackends[] = {
    {"null", nullptr, wgpu::BackendType::Null},         //
    {"webgpu", nullptr, wgpu::BackendType::WebGPU},     //
    {"d3d11", nullptr, wgpu::BackendType::D3D11},       //
    {"d3d12", "d3d", wgpu::BackendType::D3D12},         //
    {"metal", nullptr, wgpu::BackendType::Metal},       //
    {"vulkan", "vk", wgpu::BackendType::Vulkan},        //
    {"opengl", "gl", wgpu::BackendType::OpenGL},        //
    {"opengles", "gles", wgpu::BackendType::OpenGLES},  //
};

std::optional<wgpu::BackendType> ParseBackend(std::string_view name) {
    for (auto& info : kBackends) {
        if (info.name == name || (info.alias && info.alias == name)) {
            return info.backend;
        }
    }
    return std::nullopt;
}

const char* BackendName(wgpu::BackendType backend) {
    for (auto& info : kBackends) {
        if (info.backend == backend) {
            return info.name;
        }
    }
    return "<unknown>";
}

}  // namespace

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPU
////////////////////////////////////////////////////////////////////////////////
GPU::GPU(Flags flags) : flags_(std::move(flags)) {
    if (auto validate = flags_.Get("validate"); validate == "1" || validate == "true") {
        instance_.EnableBackendValidation(true);
        instance_.SetBackendValidationLevel(dawn::native::BackendValidationLevel::Full);
    }

    // Setting the DllDir changes where we load adapter DLLs from (e.g. d3dcompiler_47.dll)
    if (auto dir = flags_.Get("dlldir")) {
        SetDllDir(dir->c_str());
    }
    instance_.DiscoverDefaultAdapters();
}

interop::Promise<std::optional<interop::Interface<interop::GPUAdapter>>> GPU::requestAdapter(
    Napi::Env env,
    interop::GPURequestAdapterOptions options) {
    auto promise =
        interop::Promise<std::optional<interop::Interface<interop::GPUAdapter>>>(env, PROMISE_INFO);

    if (options.forceFallbackAdapter) {
        // Software adapters are not currently supported.
        promise.Resolve({});
        return promise;
    }

    auto adapters = instance_.GetAdapters();
    if (adapters.empty()) {
        promise.Resolve({});
        return promise;
    }

#if defined(_WIN32)
    constexpr auto defaultBackendType = wgpu::BackendType::D3D12;
#elif defined(__linux__)
    constexpr auto defaultBackendType = wgpu::BackendType::Vulkan;
#elif defined(__APPLE__)
    constexpr auto defaultBackendType = wgpu::BackendType::Metal;
#else
#error "Unsupported platform"
#endif

    // Check for backend override from env var / flag
    std::string forceBackend;
    if (auto f = flags_.Get("backend")) {
        forceBackend = *f;
    } else if (std::string envVar = GetEnvVar("DAWNNODE_BACKEND"); !envVar.empty()) {
        forceBackend = envVar;
    }

    // Check for specific adapter name
    std::string adapterName;
    if (auto f = flags_.Get("adapter")) {
        adapterName = *f;
    }

    std::transform(forceBackend.begin(), forceBackend.end(), forceBackend.begin(),
                   [](char c) { return std::tolower(c); });

    auto targetBackendType = defaultBackendType;
    if (!forceBackend.empty()) {
        if (auto parsed = ParseBackend(forceBackend)) {
            targetBackendType = parsed.value();
        } else {
            std::stringstream msg;
            msg << "unrecognised backend '" + forceBackend + "'" << std::endl
                << "Possible backends: ";
            for (auto& info : kBackends) {
                if (&info != &kBackends[0]) {
                    msg << ", ";
                }
                msg << "'" << info.name << "'";
            }
            promise.Reject(msg.str());
            return promise;
        }
    }

    dawn::native::Adapter* adapter = nullptr;
    for (auto& a : adapters) {
        wgpu::AdapterProperties props;
        a.GetProperties(&props);
        if (props.backendType != targetBackendType) {
            continue;
        }
        if (!adapterName.empty() && props.name &&
            std::string(props.name).find(adapterName) == std::string::npos) {
            continue;
        }
        adapter = &a;
        break;
    }

    if (!adapter) {
        std::stringstream msg;
        if (!forceBackend.empty() || adapterName.empty()) {
            msg << "no adapter ";
            if (!forceBackend.empty()) {
                msg << "with backend '" << forceBackend << "'";
                if (!adapterName.empty()) {
                    msg << " and name '" << adapterName << "'";
                }
            } else {
                msg << " with name '" << adapterName << "'";
            }
            msg << " found";
        } else {
            msg << "no suitable backends found";
        }
        msg << std::endl << "Available adapters:";
        for (auto& a : adapters) {
            wgpu::AdapterProperties props;
            a.GetProperties(&props);
            msg << std::endl
                << " * backend: '" << BackendName(props.backendType) << "', name: '" << props.name
                << "'";
        }
        promise.Reject(msg.str());
        return promise;
    }

    if (flags_.Get("verbose")) {
        wgpu::AdapterProperties props;
        adapter->GetProperties(&props);
        printf("using GPU adapter: %s\n", props.name);
    }

    auto gpuAdapter = GPUAdapter::Create<GPUAdapter>(env, *adapter, flags_);
    promise.Resolve(std::optional<interop::Interface<interop::GPUAdapter>>(gpuAdapter));
    return promise;
}

interop::GPUTextureFormat GPU::getPreferredCanvasFormat(Napi::Env) {
    UNIMPLEMENTED();
}

}  // namespace wgpu::binding
