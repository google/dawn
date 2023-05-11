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

#include "dawn/native/d3d11/BackendD3D11.h"

#include <memory>
#include <utility>

#include "dawn/common/Log.h"
#include "dawn/native/D3D11Backend.h"
#include "dawn/native/Instance.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/PhysicalDeviceD3D11.h"
#include "dawn/native/d3d11/PlatformFunctionsD3D11.h"

namespace dawn::native::d3d11 {

Backend::Backend(InstanceBase* instance) : Base(instance, wgpu::BackendType::D3D11) {}

MaybeError Backend::Initialize() {
    auto functions = std::make_unique<PlatformFunctions>();
    DAWN_TRY(functions->LoadFunctions());

    DAWN_TRY(Base::Initialize(std::move(functions)));

    return {};
}

const PlatformFunctions* Backend::GetFunctions() const {
    return static_cast<const PlatformFunctions*>(Base::GetFunctions());
}

ResultOrError<Ref<PhysicalDeviceBase>> Backend::CreatePhysicalDeviceFromIDXGIAdapter(
    ComPtr<IDXGIAdapter> dxgiAdapter) {
    ComPtr<IDXGIAdapter3> dxgiAdapter3;
    DAWN_TRY(CheckHRESULT(dxgiAdapter.As(&dxgiAdapter3), "DXGIAdapter retrieval"));
    Ref<PhysicalDevice> physicalDevice =
        AcquireRef(new PhysicalDevice(this, std::move(dxgiAdapter3)));
    DAWN_TRY(physicalDevice->Initialize());

    return {std::move(physicalDevice)};
}

BackendConnection* Connect(InstanceBase* instance) {
    Backend* backend = new Backend(instance);

    if (instance->ConsumedError(backend->Initialize())) {
        delete backend;
        return nullptr;
    }

    return backend;
}

}  // namespace dawn::native::d3d11
