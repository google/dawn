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

#include "src/dawn/node/binding/GPU.h"

#include <algorithm>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#include "src/dawn/node/binding/GPUAdapter.h"
#include "src/dawn/node/binding/TogglesLoader.h"

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
}

interop::Promise<std::optional<interop::Interface<interop::GPUAdapter>>> GPU::requestAdapter(
    Napi::Env env,
    interop::GPURequestAdapterOptions options) {
    auto promise =
        interop::Promise<std::optional<interop::Interface<interop::GPUAdapter>>>(env, PROMISE_INFO);

    RequestAdapterOptions nativeOptions;
    nativeOptions.forceFallbackAdapter = options.forceFallbackAdapter;
    nativeOptions.compatibilityMode = options.compatibilityMode;

    // Convert the power preference.
    nativeOptions.powerPreference = PowerPreference::Undefined;
    if (options.powerPreference.has_value()) {
        switch (options.powerPreference.value()) {
            case interop::GPUPowerPreference::kLowPower:
                nativeOptions.powerPreference = PowerPreference::LowPower;
                break;
            case interop::GPUPowerPreference::kHighPerformance:
                nativeOptions.powerPreference = PowerPreference::HighPerformance;
                break;
        }
    }

    // Choose the backend to use.
#if defined(_WIN32)
    constexpr auto kDefaultBackendType = wgpu::BackendType::D3D12;
#elif defined(__linux__)
    constexpr auto kDefaultBackendType = wgpu::BackendType::Vulkan;
#elif defined(__APPLE__)
    constexpr auto kDefaultBackendType = wgpu::BackendType::Metal;
#else
#error "Unsupported platform"
#endif
    nativeOptions.backendType = kDefaultBackendType;

    // Check for backend override from env var / flag.
    std::string forceBackend;
    if (auto f = flags_.Get("backend")) {
        forceBackend = *f;
    } else if (std::string envVar = GetEnvVar("DAWNNODE_BACKEND"); !envVar.empty()) {
        forceBackend = envVar;
    }
    std::transform(forceBackend.begin(), forceBackend.end(), forceBackend.begin(),
                   [](char c) { return std::tolower(c); });

    if (!forceBackend.empty()) {
        if (auto parsed = ParseBackend(forceBackend)) {
            nativeOptions.backendType = parsed.value();
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

    // Propagate toggles.
    TogglesLoader togglesLoader(flags_);
    DawnTogglesDescriptor togglesDescriptor = togglesLoader.GetDescriptor();
    nativeOptions.nextInChain = &togglesDescriptor;

    auto adapters = instance_.EnumerateAdapters(&nativeOptions);
    if (adapters.empty()) {
        promise.Resolve({});
        return promise;
    }

    // Check for specific adapter name
    std::string adapterName;
    if (auto f = flags_.Get("adapter")) {
        adapterName = *f;
    }

    dawn::native::Adapter* adapter = nullptr;
    for (auto& a : adapters) {
        wgpu::AdapterProperties props;
        a.GetProperties(&props);
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
#if defined(__ANDROID__)
    return interop::GPUTextureFormat::kRgba8Unorm;
#else
    return interop::GPUTextureFormat::kBgra8Unorm;
#endif  // defined(__ANDROID__)
}

interop::Interface<interop::WGSLLanguageFeatures> GPU::getWgslLanguageFeatures(Napi::Env env) {
    // TODO(crbug.com/dawn/1777)
    struct Features : public interop::WGSLLanguageFeatures {
        ~Features() = default;
        bool has(Napi::Env env, std::string) { UNIMPLEMENTED(env, {}); }
        std::vector<std::string> keys(Napi::Env env) { UNIMPLEMENTED(env, {}); }
    };
    return interop::WGSLLanguageFeatures::Create<Features>(env);
}

}  // namespace wgpu::binding
