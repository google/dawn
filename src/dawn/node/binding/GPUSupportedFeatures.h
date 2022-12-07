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

#ifndef SRC_DAWN_NODE_BINDING_GPUSUPPORTEDFEATURES_H_
#define SRC_DAWN_NODE_BINDING_GPUSUPPORTEDFEATURES_H_

#include <string>
#include <unordered_set>
#include <vector>

#include "dawn/webgpu_cpp.h"

#include "src/dawn/node/interop/Napi.h"
#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {

// GPUSupportedLFeatures is an implementation of interop::GPUSupportedFeatures.
class GPUSupportedFeatures final : public interop::GPUSupportedFeatures {
  public:
    GPUSupportedFeatures(Napi::Env env, std::vector<wgpu::FeatureName> features);

    // interop::GPUSupportedFeatures interface compliance
    bool has(Napi::Env, std::string name) override;
    std::vector<std::string> keys(Napi::Env) override;

  private:
    std::unordered_set<interop::GPUFeatureName> enabled_;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_GPUSUPPORTEDFEATURES_H_
