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

}  // namespace

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPU
////////////////////////////////////////////////////////////////////////////////
GPU::GPU(Flags flags) : flags_(std::move(flags)) {
    // TODO(dawn:1123): Disable in 'release'
    instance_.EnableBackendValidation(true);
    instance_.SetBackendValidationLevel(dawn::native::BackendValidationLevel::Full);

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

    auto targetBackendType = defaultBackendType;
    std::string forceBackend;

    // Check for override from env var
    if (std::string envVar = GetEnvVar("DAWNNODE_BACKEND"); !envVar.empty()) {
        forceBackend = envVar;
    }

    // Check for override from flag
    if (auto f = flags_.Get("dawn-backend")) {
        forceBackend = *f;
    }

    std::transform(forceBackend.begin(), forceBackend.end(), forceBackend.begin(),
                   [](char c) { return std::tolower(c); });

    // Default to first adapter if a backend is not specified
    size_t adapterIndex = 0;

    if (!forceBackend.empty()) {
        if (forceBackend == "null") {
            targetBackendType = wgpu::BackendType::Null;
        } else if (forceBackend == "webgpu") {
            targetBackendType = wgpu::BackendType::WebGPU;
        } else if (forceBackend == "d3d11") {
            targetBackendType = wgpu::BackendType::D3D11;
        } else if (forceBackend == "d3d12" || forceBackend == "d3d") {
            targetBackendType = wgpu::BackendType::D3D12;
        } else if (forceBackend == "metal") {
            targetBackendType = wgpu::BackendType::Metal;
        } else if (forceBackend == "vulkan" || forceBackend == "vk") {
            targetBackendType = wgpu::BackendType::Vulkan;
        } else if (forceBackend == "opengl" || forceBackend == "gl") {
            targetBackendType = wgpu::BackendType::OpenGL;
        } else if (forceBackend == "opengles" || forceBackend == "gles") {
            targetBackendType = wgpu::BackendType::OpenGLES;
        } else {
            promise.Reject("unknown backend '" + forceBackend + "'");
            return promise;
        }
    }

    bool found = false;
    for (size_t i = 0; i < adapters.size(); ++i) {
        wgpu::AdapterProperties props;
        adapters[i].GetProperties(&props);
        if (props.backendType == targetBackendType) {
            adapterIndex = i;
            found = true;
            break;
        }
    }
    if (!found) {
        if (!forceBackend.empty()) {
            promise.Reject("backend '" + forceBackend + "' not found");
        } else {
            promise.Reject("no suitable backends found");
        }
        return promise;
    }

    auto adapter = GPUAdapter::Create<GPUAdapter>(env, adapters[adapterIndex], flags_);
    promise.Resolve(std::optional<interop::Interface<interop::GPUAdapter>>(adapter));
    return promise;
}

}  // namespace wgpu::binding
