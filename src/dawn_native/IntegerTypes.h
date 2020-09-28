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

    using VertexBufferSlot = TypedInteger<struct VertexBufferSlotT, uint8_t>;
    using VertexAttributeLocation = TypedInteger<struct VertexAttributeLocationT, uint8_t>;

    constexpr VertexBufferSlot kMaxVertexBuffersTyped = VertexBufferSlot(kMaxVertexBuffers);
    constexpr VertexAttributeLocation kMaxVertexAttributesTyped =
        VertexAttributeLocation(kMaxVertexAttributes);

    // Serials are 64bit integers that are incremented by one each time to produce unique values.
    // Some serials (like queue serials) are compared numerically to know which one is before
    // another, while some serials are only checked for equality. We call serials only checked
    // for equality IDs.

    // Buffer mapping requests are stored outside of the buffer while they are being processed and
    // cannot be invalidated. Instead they are associated with an ID, and when a map request is
    // finished, the mapping callback is fired only if its ID matches the ID if the last request
    // that was sent.
    using MapRequestID = TypedInteger<struct MapRequestIDT, uint64_t>;

    // The type for the WebGPU API fence serial values.
    using FenceAPISerial = TypedInteger<struct FenceAPISerialT, uint64_t>;

    // A serial used to watch the progression of GPU execution on a queue, each time operations
    // that need to be followed individually are scheduled for execution on a queue, the serial
    // is incremented by one. This way to know if something is done executing, we just need to
    // compare its serial with the currently completed serial.
    using ExecutionSerial = TypedInteger<struct QueueSerialT, uint64_t>;
    constexpr ExecutionSerial kMaxExecutionSerial = ExecutionSerial(~uint64_t(0));

}  // namespace dawn_native

#endif  // DAWNNATIVE_INTEGERTYPES_H_
