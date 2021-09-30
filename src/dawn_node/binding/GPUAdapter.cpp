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

#include "src/dawn_node/binding/GPUDevice.h"
#include "src/dawn_node/binding/GPUSupportedLimits.h"

namespace wgpu { namespace binding {

    namespace {

        ////////////////////////////////////////////////////////////////////////////////
        // wgpu::binding::<anon>::Features
        // Implements interop::GPUSupportedFeatures
        ////////////////////////////////////////////////////////////////////////////////
        class Features : public interop::GPUSupportedFeatures {
          public:
            Features(WGPUDeviceProperties properties) : properties_(std::move(properties)) {
            }

            bool has(interop::GPUFeatureName feature) {
                switch (feature) {
                    case interop::GPUFeatureName::kDepthClamping:
                        return properties_.depthClamping;
                    case interop::GPUFeatureName::kDepth24UnormStencil8:
                        return false;  // TODO(crbug.com/dawn/1130)
                    case interop::GPUFeatureName::kDepth32FloatStencil8:
                        return false;  // TODO(crbug.com/dawn/1130)
                    case interop::GPUFeatureName::kPipelineStatisticsQuery:
                        return properties_.pipelineStatisticsQuery;
                    case interop::GPUFeatureName::kTextureCompressionBc:
                        return properties_.textureCompressionBC;
                    case interop::GPUFeatureName::kTimestampQuery:
                        return properties_.timestampQuery;
                }
                UNIMPLEMENTED("feature: ", feature);
                return false;
            }

            // interop::GPUSupportedFeatures compliance
            bool has(Napi::Env, std::string name) override {
                interop::GPUFeatureName feature;
                if (interop::Converter<interop::GPUFeatureName>::FromString(name, feature)) {
                    return has(feature);
                }
                return false;
            }

          private:
            WGPUDeviceProperties properties_;
        };

    }  // namespace

    ////////////////////////////////////////////////////////////////////////////////
    // wgpu::bindings::GPUAdapter
    // TODO(crbug.com/dawn/1133): This is a stub implementation. Properly implement.
    ////////////////////////////////////////////////////////////////////////////////
    GPUAdapter::GPUAdapter(dawn_native::Adapter a) : adapter_(a) {
    }

    std::string GPUAdapter::getName(Napi::Env) {
        return "dawn-adapter";
    }

    interop::Interface<interop::GPUSupportedFeatures> GPUAdapter::getFeatures(Napi::Env env) {
        return interop::GPUSupportedFeatures::Create<Features>(env,
                                                               adapter_.GetAdapterProperties());
    }

    interop::Interface<interop::GPUSupportedLimits> GPUAdapter::getLimits(Napi::Env env) {
        return interop::GPUSupportedLimits::Create<GPUSupportedLimits>(env);
    }

    bool GPUAdapter::getIsFallbackAdapter(Napi::Env) {
        UNIMPLEMENTED();
    }

    interop::Promise<interop::Interface<interop::GPUDevice>> GPUAdapter::requestDevice(
        Napi::Env env,
        std::optional<interop::GPUDeviceDescriptor> descriptor) {
        dawn_native::DeviceDescriptor desc{};  // TODO(crbug.com/dawn/1133): Fill in.
        interop::Promise<interop::Interface<interop::GPUDevice>> promise(env);

        if (descriptor.has_value()) {
            // See src/dawn_native/Extensions.cpp for feature <-> extension mappings.
            for (auto required : descriptor->requiredFeatures) {
                switch (required) {
                    case interop::GPUFeatureName::kDepthClamping:
                        desc.requiredExtensions.emplace_back("depth_clamping");
                        continue;
                    case interop::GPUFeatureName::kPipelineStatisticsQuery:
                        desc.requiredExtensions.emplace_back("pipeline_statistics_query");
                        continue;
                    case interop::GPUFeatureName::kTextureCompressionBc:
                        desc.requiredExtensions.emplace_back("texture_compression_bc");
                        continue;
                    case interop::GPUFeatureName::kTimestampQuery:
                        desc.requiredExtensions.emplace_back("timestamp_query");
                        continue;
                    case interop::GPUFeatureName::kDepth24UnormStencil8:
                    case interop::GPUFeatureName::kDepth32FloatStencil8:
                        continue;  // TODO(crbug.com/dawn/1130)
                }
                UNIMPLEMENTED("required: ", required);
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
