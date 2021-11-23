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

#include "src/dawn_node/binding/GPUAdapter.h"

#include <unordered_set>

#include "src/dawn_node/binding/Flags.h"
#include "src/dawn_node/binding/GPUDevice.h"
#include "src/dawn_node/binding/GPUSupportedLimits.h"

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

namespace wgpu { namespace binding {

    namespace {

        ////////////////////////////////////////////////////////////////////////////////
        // wgpu::binding::<anon>::Features
        // Implements interop::GPUSupportedFeatures
        ////////////////////////////////////////////////////////////////////////////////
        class Features : public interop::GPUSupportedFeatures {
          public:
            Features(WGPUDeviceProperties properties) {
                if (properties.depthClamping) {
                    enabled_.emplace(interop::GPUFeatureName::kDepthClamping);
                }
                if (properties.pipelineStatisticsQuery) {
                    enabled_.emplace(interop::GPUFeatureName::kPipelineStatisticsQuery);
                }
                if (properties.textureCompressionBC) {
                    enabled_.emplace(interop::GPUFeatureName::kTextureCompressionBc);
                }
                if (properties.timestampQuery) {
                    enabled_.emplace(interop::GPUFeatureName::kTimestampQuery);
                }

                // TODO(crbug.com/dawn/1130)
                // interop::GPUFeatureName::kDepth24UnormStencil8:
                // interop::GPUFeatureName::kDepth32FloatStencil8:
            }

            bool has(interop::GPUFeatureName feature) {
                return enabled_.count(feature) != 0;
            }

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
    GPUAdapter::GPUAdapter(dawn_native::Adapter a, const Flags& flags)
        : adapter_(a), flags_(flags) {
    }

    std::string GPUAdapter::getName(Napi::Env) {
        return "dawn-adapter";
    }

    interop::Interface<interop::GPUSupportedFeatures> GPUAdapter::getFeatures(Napi::Env env) {
        return interop::GPUSupportedFeatures::Create<Features>(env,
                                                               adapter_.GetAdapterProperties());
    }

    interop::Interface<interop::GPUSupportedLimits> GPUAdapter::getLimits(Napi::Env env) {
        WGPUSupportedLimits limits{};
        if (!adapter_.GetLimits(&limits)) {
            Napi::Error::New(env, "failed to get adapter limits").ThrowAsJavaScriptException();
        }

        wgpu::SupportedLimits wgpuLimits{};

#define COPY_LIMIT(LIMIT) wgpuLimits.limits.LIMIT = limits.limits.LIMIT
        COPY_LIMIT(maxTextureDimension1D);
        COPY_LIMIT(maxTextureDimension2D);
        COPY_LIMIT(maxTextureDimension3D);
        COPY_LIMIT(maxTextureArrayLayers);
        COPY_LIMIT(maxBindGroups);
        COPY_LIMIT(maxDynamicUniformBuffersPerPipelineLayout);
        COPY_LIMIT(maxDynamicStorageBuffersPerPipelineLayout);
        COPY_LIMIT(maxSampledTexturesPerShaderStage);
        COPY_LIMIT(maxSamplersPerShaderStage);
        COPY_LIMIT(maxStorageBuffersPerShaderStage);
        COPY_LIMIT(maxStorageTexturesPerShaderStage);
        COPY_LIMIT(maxUniformBuffersPerShaderStage);
        COPY_LIMIT(maxUniformBufferBindingSize);
        COPY_LIMIT(maxStorageBufferBindingSize);
        COPY_LIMIT(minUniformBufferOffsetAlignment);
        COPY_LIMIT(minStorageBufferOffsetAlignment);
        COPY_LIMIT(maxVertexBuffers);
        COPY_LIMIT(maxVertexAttributes);
        COPY_LIMIT(maxVertexBufferArrayStride);
        COPY_LIMIT(maxInterStageShaderComponents);
        COPY_LIMIT(maxComputeWorkgroupStorageSize);
        COPY_LIMIT(maxComputeInvocationsPerWorkgroup);
        COPY_LIMIT(maxComputeWorkgroupSizeX);
        COPY_LIMIT(maxComputeWorkgroupSizeY);
        COPY_LIMIT(maxComputeWorkgroupSizeZ);
        COPY_LIMIT(maxComputeWorkgroupsPerDimension);
#undef COPY_LIMIT

        return interop::GPUSupportedLimits::Create<GPUSupportedLimits>(env, wgpuLimits);
    }

    bool GPUAdapter::getIsFallbackAdapter(Napi::Env) {
        UNIMPLEMENTED();
    }

    interop::Promise<interop::Interface<interop::GPUDevice>> GPUAdapter::requestDevice(
        Napi::Env env,
        interop::GPUDeviceDescriptor descriptor) {
        dawn_native::DawnDeviceDescriptor desc{};  // TODO(crbug.com/dawn/1133): Fill in.
        interop::Promise<interop::Interface<interop::GPUDevice>> promise(env, PROMISE_INFO);

        // See src/dawn_native/Features.cpp for enum <-> string mappings.
        for (auto required : descriptor.requiredFeatures) {
            switch (required) {
                case interop::GPUFeatureName::kDepthClamping:
                    desc.requiredFeatures.emplace_back("depth-clamping");
                    continue;
                case interop::GPUFeatureName::kPipelineStatisticsQuery:
                    desc.requiredFeatures.emplace_back("pipeline-statistics-query");
                    continue;
                case interop::GPUFeatureName::kTextureCompressionBc:
                    desc.requiredFeatures.emplace_back("texture-compression-bc");
                    continue;
                case interop::GPUFeatureName::kTimestampQuery:
                    desc.requiredFeatures.emplace_back("timestamp-query");
                    continue;
                case interop::GPUFeatureName::kDepth24UnormStencil8:
                case interop::GPUFeatureName::kDepth32FloatStencil8:
                    continue;  // TODO(crbug.com/dawn/1130)
            }
            UNIMPLEMENTED("required: ", required);
        }

        // Propogate enabled/disabled dawn features
        // Note: DawnDeviceDescriptor::forceEnabledToggles and forceDisabledToggles are vectors of
        // 'const char*', so we make sure the parsed strings survive the CreateDevice() call by
        // storing them on the stack.
        std::vector<std::string> enabledToggles;
        std::vector<std::string> disabledToggles;
        if (auto values = flags_.Get("enable-dawn-features")) {
            enabledToggles = Split(*values, ',');
            for (auto& t : enabledToggles) {
                desc.forceEnabledToggles.emplace_back(t.c_str());
            }
        }
        if (auto values = flags_.Get("disable-dawn-features")) {
            disabledToggles = Split(*values, ',');
            for (auto& t : disabledToggles) {
                desc.forceDisabledToggles.emplace_back(t.c_str());
            }
        }

        auto wgpu_device = adapter_.CreateDevice(&desc);
        if (wgpu_device) {
            promise.Resolve(interop::GPUDevice::Create<GPUDevice>(env, env, wgpu_device));
        } else {
            Napi::Error::New(env, "failed to create device").ThrowAsJavaScriptException();
        }
        return promise;
    }
}}  // namespace wgpu::binding
