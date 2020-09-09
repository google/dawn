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

#ifndef DAWNNATIVE_INTEGERTYPES_H_
#define DAWNNATIVE_INTEGERTYPES_H_

#include "common/Constants.h"
#include "common/TypedInteger.h"

#include <cstdint>

namespace dawn_native {
    // Binding numbers in the shader and BindGroup/BindGroupLayoutDescriptors
    using BindingNumber = TypedInteger<struct BindingNumberT, uint32_t>;

    // Binding numbers get mapped to a packed range of indices
    using BindingIndex = TypedInteger<struct BindingIndexT, uint32_t>;

    using BindGroupIndex = TypedInteger<struct BindGroupIndexT, uint32_t>;

    static constexpr BindGroupIndex kMaxBindGroupsTyped = BindGroupIndex(kMaxBindGroups);

    using ColorAttachmentIndex = TypedInteger<struct ColorAttachmentIndexT, uint8_t>;

    constexpr ColorAttachmentIndex kMaxColorAttachmentsTyped =
        ColorAttachmentIndex(kMaxColorAttachments);

}  // namespace dawn_native

#endif  // DAWNNATIVE_INTEGERTYPES_H_
