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

#include "dawn/native/d3d/DeviceD3D.h"

#include "dawn/native/d3d/AdapterD3D.h"
#include "dawn/native/d3d/BackendD3D.h"
#include "dawn/native/d3d/Forward.h"

namespace dawn::native::d3d {

Device::Device(AdapterBase* adapter,
               const DeviceDescriptor* descriptor,
               const TogglesState& deviceToggles)
    : DeviceBase(adapter, descriptor, deviceToggles) {}

Device::~Device() = default;

ResultOrError<wgpu::TextureUsage> Device::GetSupportedSurfaceUsageImpl(
    const Surface* surface) const {
    wgpu::TextureUsage usages =
        wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;
    return usages;
}

const PlatformFunctions* Device::GetFunctions() const {
    return ToBackend(GetAdapter())->GetBackend()->GetFunctions();
}

ComPtr<IDXGIFactory4> Device::GetFactory() const {
    return ToBackend(GetAdapter())->GetBackend()->GetFactory();
}

ComPtr<IDxcLibrary> Device::GetDxcLibrary() const {
    return ToBackend(GetAdapter())->GetBackend()->GetDxcLibrary();
}

ComPtr<IDxcCompiler> Device::GetDxcCompiler() const {
    return ToBackend(GetAdapter())->GetBackend()->GetDxcCompiler();
}

ComPtr<IDxcValidator> Device::GetDxcValidator() const {
    return ToBackend(GetAdapter())->GetBackend()->GetDxcValidator();
}

}  // namespace dawn::native::d3d
