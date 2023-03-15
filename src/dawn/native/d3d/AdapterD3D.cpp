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

#include "dawn/native/d3d/AdapterD3D.h"

#include <utility>

#include "dawn/native/d3d/BackendD3D.h"

namespace dawn::native::d3d {

Adapter::Adapter(Backend* backend,
                 ComPtr<IDXGIAdapter3> hardwareAdapter,
                 wgpu::BackendType backendType,
                 const TogglesState& adapterToggles)
    : AdapterBase(backend->GetInstance(), backendType, adapterToggles),
      mHardwareAdapter(std::move(hardwareAdapter)),
      mBackend(backend) {}

Adapter::~Adapter() = default;

IDXGIAdapter3* Adapter::GetHardwareAdapter() const {
    return mHardwareAdapter.Get();
}

Backend* Adapter::GetBackend() const {
    return mBackend;
}

}  // namespace dawn::native::d3d
