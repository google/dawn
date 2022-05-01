// Copyright 2020 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_D3D12_INTEGERTYPES_H_
#define SRC_DAWN_NATIVE_D3D12_INTEGERTYPES_H_

#include <cstdint>

#include "dawn/common/Constants.h"
#include "dawn/common/TypedInteger.h"

namespace dawn::native::d3d12 {

// An ID used to desambiguate between multiple uses of the same descriptor heap in the
// BindGroup allocations.
using HeapVersionID = TypedInteger<struct HeapVersionIDT, uint64_t>;

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_INTEGERTYPES_H_
