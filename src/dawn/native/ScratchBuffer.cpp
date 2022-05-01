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

#include "dawn/native/ScratchBuffer.h"

#include "dawn/native/Device.h"

namespace dawn::native {

ScratchBuffer::ScratchBuffer(DeviceBase* device, wgpu::BufferUsage usage)
    : mDevice(device), mUsage(usage) {}

ScratchBuffer::~ScratchBuffer() = default;

void ScratchBuffer::Reset() {
    mBuffer = nullptr;
}

MaybeError ScratchBuffer::EnsureCapacity(uint64_t capacity) {
    if (!mBuffer.Get() || mBuffer->GetSize() < capacity) {
        BufferDescriptor descriptor;
        descriptor.size = capacity;
        descriptor.usage = mUsage;
        DAWN_TRY_ASSIGN(mBuffer, mDevice->CreateBuffer(&descriptor));
        mBuffer->SetIsDataInitialized();
    }
    return {};
}

BufferBase* ScratchBuffer::GetBuffer() const {
    ASSERT(mBuffer.Get() != nullptr);
    return mBuffer.Get();
}

}  // namespace dawn::native
