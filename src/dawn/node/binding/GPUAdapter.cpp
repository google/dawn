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

#include "src/dawn/node/binding/GPUAdapter.h"

#include <unordered_set>
#include <vector>

#include "src/dawn/node/binding/Errors.h"
#include "src/dawn/node/binding/Flags.h"
#include "src/dawn/node/binding/GPUDevice.h"
#include "src/dawn/node/binding/GPUSupportedLimits.h"

namespace {
// TODO(amaiorano): Move to utility header
std::vector<std::string> Split(const std::string& s, char delim) {
    if (s.empty())
        return {};

    std::vector<std::string> result;
    const size_t lastIndex = s.length() - 1;
    size_t startIndex = 0;
    size_t i = startIndex;

    while (i <= lastIndex) {
        if (s[i] == delim) {
            auto token = s.substr(startIndex, i - startIndex);
            if (!token.empty())  // Discard empty tokens
                result.push_back(token);
            startIndex = i + 1;
        } else if (i == lastIndex) {
            auto token = s.substr(startIndex, i - startIndex + 1);
            if (!token.empty())  // Discard empty tokens
                result.push_back(token);
        }
        ++i;
    }
    return result;
}
}  // namespace

#define FOR_EACH_LIMIT(X)                        \
    X(maxTextureDimension1D)                     \
    X(maxTextureDimension2D)                     \
    X(maxTextureDimension3D)                     \
    X(maxTextureArrayLayers)                     \
    X(maxBindGroups)                             \
    X(maxDynamicUniformBuffersPerPipelineLayout) \
    X(maxDynamicStorageBuffersPerPipelineLayout) \
    X(maxSampledTexturesPerShaderStage)          \
    X(maxSamplersPerShaderStage)                 \
    X(maxStorageBuffersPerShaderStage)           \
    X(maxStorageTexturesPerShaderStage)          \
    X(maxUniformBuffersPerShaderStage)           \
    X(maxUniformBufferBindingSize)               \
    X(maxStorageBufferBindingSize)               \
    X(minUniformBufferOffsetAlignment)           \
    X(minStorageBufferOffsetAlignment)           \
    X(maxVertexBuffers)                          \
    X(maxVertexAttributes)                       \
    X(maxVertexBufferArrayStride)                \
    X(maxInterStageShaderComponents)             \
    X(maxComputeWorkgroupStorageSize)            \
    X(maxComputeInvocationsPerWorkgroup)         \
    X(maxComputeWorkgroupSizeX)                  \
    X(maxComputeWorkgroupSizeY)                  \
    X(maxComputeWorkgroupSizeZ)                  \
    X(maxComputeWorkgroupsPerDimension)

namespace wgpu::binding {

namespace {

////////////////////////////////////////////////////////////////////////////////
// wgpu::binding::<anon>::Features
// Implements interop::GPUSupportedFeatures
////////////////////////////////////////////////////////////////////////////////
class Features : public interop::GPUSupportedFeatures {
  public:
    explicit Features(WGPUDeviceProperties properties) {
        if (properties.depth24UnormStencil8) {
            enabled_.emplace(interop::GPUFeatureName::kDepth24UnormStencil8);
        }
        if (properties.depth32FloatStencil8) {
            enabled_.emplace(interop::GPUFeatureName::kDepth32FloatStencil8);
        }
        if (properties.timestampQuery) {
            enabled_.emplace(interop::GPUFeatureName::kTimestampQuery);
        }
        if (properties.textureCompressionBC) {
            enabled_.emplace(interop::GPUFeatureName::kTextureCompressionBc);
        }
        if (properties.textureCompressionETC2) {
            enabled_.emplace(interop::GPUFeatureName::kTextureCompressionEtc2);
        }
        if (properties.textureCompressionASTC) {
            enabled_.emplace(interop::GPUFeatureName::kTextureCompressionAstc);
        }
        if (properties.timestampQuery) {
            enabled_.emplace(interop::GPUFeatureName::kTimestampQuery);
        }

        // TODO(dawn:1123) add support for these extensions when possible.
        // wgpu::interop::GPUFeatureName::kIndirectFirstInstance
        // wgpu::interop::GPUFeatureName::kDepthClipControl
    }

    bool has(interop::GPUFeatureName feature) { return enabled_.count(feature) != 0; }

    // interop::GPUSupportedFeatures compliance
    bool has(Napi::Env, std::string name) override {
        interop::GPUFeatureName feature;
        if (interop::Converter<interop::GPUFeatureName>::FromString(name, feature)) {
            return has(feature);
        }
        return false;
    }
    std::vector<std::string> keys(Napi::Env) override {
        std::vector<std::string> out;
        out.reserve(enabled_.size());
        for (auto feature : enabled_) {
            out.push_back(interop::Converter<interop::GPUFeatureName>::ToString(feature));
        }
        return out;
    }

  private:
    std::unordered_set<interop::GPUFeatureName> enabled_;
};

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUAdapter
// TODO(crbug.com/dawn/1133): This is a stub implementation. Properly implement.
////////////////////////////////////////////////////////////////////////////////
GPUAdapter::GPUAdapter(dawn::native::Adapter a, const Flags& flags) : adapter_(a), flags_(flags) {}

std::string GPUAdapter::getName(Napi::Env) {
    return "dawn-adapter";
}

interop::Interface<interop::GPUSupportedFeatures> GPUAdapter::getFeatures(Napi::Env env) {
    return interop::GPUSupportedFeatures::Create<Features>(env, adapter_.GetAdapterProperties());
}

interop::Interface<interop::GPUSupportedLimits> GPUAdapter::getLimits(Napi::Env env) {
    WGPUSupportedLimits limits{};
    if (!adapter_.GetLimits(&limits)) {
        Napi::Error::New(env, "failed to get adapter limits").ThrowAsJavaScriptException();
    }

    wgpu::SupportedLimits wgpuLimits{};

#define COPY_LIMIT(LIMIT) wgpuLimits.limits.LIMIT = limits.limits.LIMIT;
    FOR_EACH_LIMIT(COPY_LIMIT)
#undef COPY_LIMIT

    return interop::GPUSupportedLimits::Create<GPUSupportedLimits>(env, wgpuLimits);
}

bool GPUAdapter::getIsFallbackAdapter(Napi::Env) {
    UNIMPLEMENTED();
}

interop::Promise<interop::Interface<interop::GPUDevice>> GPUAdapter::requestDevice(
    Napi::Env env,
    interop::GPUDeviceDescriptor descriptor) {
    wgpu::DeviceDescriptor desc{};  // TODO(crbug.com/dawn/1133): Fill in.
    interop::Promise<interop::Interface<interop::GPUDevice>> promise(env, PROMISE_INFO);

    std::vector<wgpu::FeatureName> requiredFeatures;
    // See src/dawn/native/Features.cpp for enum <-> string mappings.
    for (auto required : descriptor.requiredFeatures) {
        switch (required) {
            case interop::GPUFeatureName::kTextureCompressionBc:
                requiredFeatures.emplace_back(wgpu::FeatureName::TextureCompressionBC);
                continue;
            case interop::GPUFeatureName::kTextureCompressionEtc2:
                requiredFeatures.emplace_back(wgpu::FeatureName::TextureCompressionETC2);
                continue;
            case interop::GPUFeatureName::kTextureCompressionAstc:
                requiredFeatures.emplace_back(wgpu::FeatureName::TextureCompressionASTC);
                continue;
            case interop::GPUFeatureName::kTimestampQuery:
                requiredFeatures.emplace_back(wgpu::FeatureName::TimestampQuery);
                continue;
            case interop::GPUFeatureName::kDepth24UnormStencil8:
                requiredFeatures.emplace_back(wgpu::FeatureName::Depth24UnormStencil8);
                continue;
            case interop::GPUFeatureName::kDepth32FloatStencil8:
                requiredFeatures.emplace_back(wgpu::FeatureName::Depth32FloatStencil8);
                continue;
            case interop::GPUFeatureName::kDepthClipControl:
            case interop::GPUFeatureName::kIndirectFirstInstance:
                // TODO(dawn:1123) Add support for these extensions when possible.
                continue;
        }
        UNIMPLEMENTED("required: ", required);
    }

    wgpu::RequiredLimits limits;
#define COPY_LIMIT(LIMIT)                                        \
    if (descriptor.requiredLimits.count(#LIMIT)) {               \
        limits.limits.LIMIT = descriptor.requiredLimits[#LIMIT]; \
        descriptor.requiredLimits.erase(#LIMIT);                 \
    }
    FOR_EACH_LIMIT(COPY_LIMIT)
#undef COPY_LIMIT

    for (auto [key, _] : descriptor.requiredLimits) {
        promise.Reject(binding::Errors::OperationError(env, "Unknown limit \"" + key + "\""));
        return promise;
    }

    // Propogate enabled/disabled dawn features
    // Note: DawnDeviceTogglesDescriptor::forceEnabledToggles and forceDisabledToggles are
    // vectors of 'const char*', so we make sure the parsed strings survive the CreateDevice()
    // call by storing them on the stack.
    std::vector<std::string> enabledToggles;
    std::vector<std::string> disabledToggles;
    std::vector<const char*> forceEnabledToggles;
    std::vector<const char*> forceDisabledToggles;
    if (auto values = flags_.Get("enable-dawn-features")) {
        enabledToggles = Split(*values, ',');
        for (auto& t : enabledToggles) {
            forceEnabledToggles.emplace_back(t.c_str());
        }
    }
    if (auto values = flags_.Get("disable-dawn-features")) {
        disabledToggles = Split(*values, ',');
        for (auto& t : disabledToggles) {
            forceDisabledToggles.emplace_back(t.c_str());
        }
    }

    desc.requiredFeaturesCount = requiredFeatures.size();
    desc.requiredFeatures = requiredFeatures.data();
    desc.requiredLimits = &limits;

    DawnTogglesDeviceDescriptor togglesDesc = {};
    desc.nextInChain = &togglesDesc;
    togglesDesc.forceEnabledTogglesCount = forceEnabledToggles.size();
    togglesDesc.forceEnabledToggles = forceEnabledToggles.data();
    togglesDesc.forceDisabledTogglesCount = forceDisabledToggles.size();
    togglesDesc.forceDisabledToggles = forceDisabledToggles.data();

    auto wgpu_device = adapter_.CreateDevice(&desc);
    if (wgpu_device) {
        promise.Resolve(interop::GPUDevice::Create<GPUDevice>(env, env, wgpu_device));
    } else {
        promise.Reject(binding::Errors::OperationError(env, "failed to create device"));
    }
    return promise;
}
}  // namespace wgpu::binding
