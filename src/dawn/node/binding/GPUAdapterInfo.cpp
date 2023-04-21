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

#include "src/dawn/node/binding/GPUAdapterInfo.h"

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUAdapterInfo
////////////////////////////////////////////////////////////////////////////////

GPUAdapterInfo::GPUAdapterInfo(WGPUAdapterProperties properties)
    : vendor_(properties.vendorName),
      architecture_(properties.architecture),
      device_(properties.name),
      description_(properties.driverDescription) {}

std::string GPUAdapterInfo::getVendor(Napi::Env) {
    return vendor_;
}

std::string GPUAdapterInfo::getArchitecture(Napi::Env) {
    return architecture_;
}

std::string GPUAdapterInfo::getDevice(Napi::Env) {
    return device_;
}

std::string GPUAdapterInfo::getDescription(Napi::Env) {
    return description_;
}

}  // namespace wgpu::binding
