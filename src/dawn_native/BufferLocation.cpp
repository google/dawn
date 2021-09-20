// Copyright 2021 The Dawn Authors
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

#include "dawn_native/BufferLocation.h"

namespace dawn_native {

    BufferLocation::BufferLocation() = default;

    BufferLocation::BufferLocation(BufferBase* buffer, uint64_t offset)
        : mBuffer(buffer), mOffset(offset) {
    }

    BufferLocation::~BufferLocation() = default;

    // static
    Ref<BufferLocation> BufferLocation::New() {
        return AcquireRef(new BufferLocation());
    }

    // static
    Ref<BufferLocation> BufferLocation::New(BufferBase* buffer, uint64_t offset) {
        return AcquireRef(new BufferLocation(buffer, offset));
    }

    bool BufferLocation::IsNull() const {
        return mBuffer.Get() == nullptr;
    }

    BufferBase* BufferLocation::GetBuffer() const {
        return mBuffer.Get();
    }

    uint64_t BufferLocation::GetOffset() const {
        return mOffset;
    }

    void BufferLocation::Set(BufferBase* buffer, uint64_t offset) {
        mBuffer = buffer;
        mOffset = offset;
    }

}  // namespace dawn_native
