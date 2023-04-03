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

// D3D12Backend.cpp: contains the definition of symbols exported by D3D12Backend.h so that they
// can be compiled twice: once export (shared library), once not exported (static library)

#include "dawn/native/D3DBackend.h"

#include <utility>

namespace dawn::native::d3d {

AdapterDiscoveryOptions::AdapterDiscoveryOptions(WGPUBackendType type,
                                                 Microsoft::WRL::ComPtr<IDXGIAdapter> adapter)
    : AdapterDiscoveryOptionsBase(type), dxgiAdapter(std::move(adapter)) {}

}  // namespace dawn::native::d3d
