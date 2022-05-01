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

#ifndef SRC_DAWN_NATIVE_ENUMCLASSBITMASKS_H_
#define SRC_DAWN_NATIVE_ENUMCLASSBITMASKS_H_

#include "dawn/EnumClassBitmasks.h"

namespace dawn::native {

// EnumClassBitmmasks is a helper in the dawn:: namespace.
// Re-export it in the dawn_native namespace.
DAWN_IMPORT_BITMASK_OPERATORS

// Specify this for usage with EnumMaskIterator
template <typename T>
struct EnumBitmaskSize {
    static constexpr unsigned value = 0;
};

template <typename T>
constexpr bool HasOneBit(T value) {
    return HasZeroOrOneBits(value) && value != T(0);
}

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_ENUMCLASSBITMASKS_H_
