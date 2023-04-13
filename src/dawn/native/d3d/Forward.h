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

#ifndef SRC_DAWN_NATIVE_D3D_FORWARD_H_
#define SRC_DAWN_NATIVE_D3D_FORWARD_H_

#include "dawn/native/ToBackend.h"

namespace dawn::native::d3d {

class Adapter;
class Device;
class SwapChain;

struct D3DBackendTraits {
    using AdapterType = Adapter;
    using DeviceType = Device;
    using SwapChainType = SwapChain;
};

template <typename T>
auto ToBackend(T&& common) -> decltype(ToBackendBase<D3DBackendTraits>(common)) {
    return ToBackendBase<D3DBackendTraits>(common);
}

}  // namespace dawn::native::d3d

#endif  // SRC_DAWN_NATIVE_D3D_FORWARD_H_
