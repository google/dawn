// Copyright 2023 The Dawn Authors
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

#ifndef SRC_DAWN_NODE_BINDING_GPUADAPTERINFO_H_
#define SRC_DAWN_NODE_BINDING_GPUADAPTERINFO_H_

#include <string>

#include "dawn/native/DawnNative.h"
#include "dawn/webgpu_cpp.h"

#include "src/dawn/node/interop/Napi.h"
#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {

// GPUAdapterInfo is an implementation of interop::GPUAdapterInfo.
class GPUAdapterInfo final : public interop::GPUAdapterInfo {
  public:
    explicit GPUAdapterInfo(WGPUAdapterProperties);

    // interop::GPUAdapterInfo interface compliance
    std::string getVendor(Napi::Env) override;
    std::string getArchitecture(Napi::Env) override;
    std::string getDevice(Napi::Env) override;
    std::string getDescription(Napi::Env) override;

  private:
    std::string vendor_;
    std::string architecture_;
    std::string device_;
    std::string description_;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_GPUADAPTERINFO_H_
