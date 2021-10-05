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
        interop::GPUDeviceDescriptor descriptor) {
        dawn_native::DeviceDescriptor desc{};  // TODO(crbug.com/dawn/1133): Fill in.
        interop::Promise<interop::Interface<interop::GPUDevice>> promise(env);

        // See src/dawn_native/Features.cpp for enum <-> string mappings.
        for (auto required : descriptor.requiredFeatures) {
            switch (required) {
                case interop::GPUFeatureName::kDepthClamping:
                    desc.requiredFeatures.emplace_back("depth_clamping");
                    continue;
                case interop::GPUFeatureName::kPipelineStatisticsQuery:
                    desc.requiredFeatures.emplace_back("pipeline_statistics_query");
                    continue;
                case interop::GPUFeatureName::kTextureCompressionBc:
                    desc.requiredFeatures.emplace_back("texture_compression_bc");
                    continue;
                case interop::GPUFeatureName::kTimestampQuery:
                    desc.requiredFeatures.emplace_back("timestamp_query");
                    continue;
                case interop::GPUFeatureName::kDepth24UnormStencil8:
                case interop::GPUFeatureName::kDepth32FloatStencil8:
                    continue;  // TODO(crbug.com/dawn/1130)
            }
            UNIMPLEMENTED("required: ", required);
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
