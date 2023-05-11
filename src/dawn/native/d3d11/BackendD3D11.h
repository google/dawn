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

#ifndef SRC_DAWN_NATIVE_D3D11_BACKENDD3D11_H_
#define SRC_DAWN_NATIVE_D3D11_BACKENDD3D11_H_

#include <vector>

#include "dawn/native/d3d/BackendD3D.h"

namespace dawn::native::d3d11 {

class PlatformFunctions;

class Backend : public d3d::Backend {
  public:
    explicit Backend(InstanceBase* instance);

    MaybeError Initialize();
    const PlatformFunctions* GetFunctions() const;

  protected:
    ResultOrError<Ref<PhysicalDeviceBase>> CreatePhysicalDeviceFromIDXGIAdapter(
        ComPtr<IDXGIAdapter> dxgiAdapter) override;

  private:
    using Base = d3d::Backend;
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_BACKENDD3D11_H_
