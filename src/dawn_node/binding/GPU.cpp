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

#include "src/dawn_node/binding/GPU.h"

#include "src/dawn_node/binding/GPUAdapter.h"

#include <cstdlib>

namespace {
    std::string getEnvVar(const char* varName) {
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
}  // namespace

namespace wgpu { namespace binding {

    ////////////////////////////////////////////////////////////////////////////////
    // wgpu::bindings::GPU
    ////////////////////////////////////////////////////////////////////////////////
    GPU::GPU() {
        // TODO: Disable in 'release'
        instance_.EnableBackendValidation(true);
        instance_.SetBackendValidationLevel(dawn_native::BackendValidationLevel::Full);

        instance_.DiscoverDefaultAdapters();
    }

    interop::Promise<std::optional<interop::Interface<interop::GPUAdapter>>> GPU::requestAdapter(
        Napi::Env env,
        interop::GPURequestAdapterOptions options) {
        auto promise =
            interop::Promise<std::optional<interop::Interface<interop::GPUAdapter>>>(env);

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
#    error "Unsupported platform"
#endif

        auto targetBackendType = defaultBackendType;

        // Check for override from env var
        std::string envVar = getEnvVar("DAWNNODE_BACKEND");
        std::transform(envVar.begin(), envVar.end(), envVar.begin(),
                       [](char c) { return std::tolower(c); });
        if (envVar == "null") {
            targetBackendType = wgpu::BackendType::Null;
        } else if (envVar == "webgpu") {
            targetBackendType = wgpu::BackendType::WebGPU;
        } else if (envVar == "d3d11") {
            targetBackendType = wgpu::BackendType::D3D11;
        } else if (envVar == "d3d12" || envVar == "d3d") {
            targetBackendType = wgpu::BackendType::D3D12;
        } else if (envVar == "metal") {
            targetBackendType = wgpu::BackendType::Metal;
        } else if (envVar == "vulkan" || envVar == "vk") {
            targetBackendType = wgpu::BackendType::Vulkan;
        } else if (envVar == "opengl" || envVar == "gl") {
            targetBackendType = wgpu::BackendType::OpenGL;
        } else if (envVar == "opengles" || envVar == "gles") {
            targetBackendType = wgpu::BackendType::OpenGLES;
        }

        // Default to first adapter if we don't find a match
        size_t adapterIndex = 0;
        for (size_t i = 0; i < adapters.size(); ++i) {
            wgpu::AdapterProperties props;
            adapters[i].GetProperties(&props);
            if (props.backendType == targetBackendType) {
                adapterIndex = i;
                break;
            }
        }

        auto adapter = GPUAdapter::Create<GPUAdapter>(env, adapters[adapterIndex]);
        promise.Resolve(std::optional<interop::Interface<interop::GPUAdapter>>(adapter));
        return promise;
    }

}}  // namespace wgpu::binding
