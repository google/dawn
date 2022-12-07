// Copyright 2022 The Dawn Authors
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

#include "src/dawn/node/binding/GPUSupportedFeatures.h"

#include "src/dawn/node/binding/Converter.h"

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUSupportedFeatures
////////////////////////////////////////////////////////////////////////////////

GPUSupportedFeatures::GPUSupportedFeatures(Napi::Env env, std::vector<wgpu::FeatureName> features) {
    Converter conv(env);

    // Add all known GPUFeatureNames that are known by dawn.node and skip the other ones are they
    // may be native-only extension, Dawn-specific or other special cases.
    for (wgpu::FeatureName feature : features) {
        interop::GPUFeatureName gpuFeature;
        if (conv(gpuFeature, feature)) {
            enabled_.emplace(gpuFeature);
        }
    }
}

bool GPUSupportedFeatures::has(Napi::Env, std::string name) {
    interop::GPUFeatureName feature;
    if (!interop::Converter<interop::GPUFeatureName>::FromString(name, feature)) {
        return false;
    }

    return enabled_.count(feature);
}

std::vector<std::string> GPUSupportedFeatures::keys(Napi::Env) {
    std::vector<std::string> out;
    out.reserve(enabled_.size());
    for (auto feature : enabled_) {
        out.push_back(interop::Converter<interop::GPUFeatureName>::ToString(feature));
    }
    return out;
}

}  // namespace wgpu::binding
