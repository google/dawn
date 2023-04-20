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

#ifndef SRC_DAWN_NATIVE_D3D_DEVICED3D_H_
#define SRC_DAWN_NATIVE_D3D_DEVICED3D_H_

#include "dawn/native/Device.h"

#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d {

class PlatformFunctions;

class Device : public DeviceBase {
  public:
    Device(AdapterBase* adapter,
           const DeviceDescriptor* descriptor,
           const TogglesState& deviceToggles);
    ~Device() override;

    ResultOrError<wgpu::TextureUsage> GetSupportedSurfaceUsageImpl(
        const Surface* surface) const override;

    const PlatformFunctions* GetFunctions() const;
    ComPtr<IDXGIFactory4> GetFactory() const;

    // Those DXC methods are needed by d3d::ShaderModule
    // TODO(penghuang): remove them when related code is refactored to
    // d3d12::ShaderModule.
    ComPtr<IDxcLibrary> GetDxcLibrary() const;
    ComPtr<IDxcCompiler> GetDxcCompiler() const;
    ComPtr<IDxcValidator> GetDxcValidator() const;
};

}  // namespace dawn::native::d3d

#endif  // SRC_DAWN_NATIVE_D3D_DEVICED3D_H_
