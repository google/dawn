// Copyright 2022 The Dawn Authors
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

#include "dawn/tests/unittests/native/mocks/BufferMock.h"

namespace dawn::native {

using ::testing::Return;

BufferMock::BufferMock(DeviceMock* device, const BufferDescriptor* descriptor)
    : BufferBase(device, descriptor) {
    mBackingData = std::unique_ptr<uint8_t[]>(new uint8_t[GetSize()]);
    mAllocatedSize = GetSize();

    ON_CALL(*this, DestroyImpl).WillByDefault([this] { this->BufferBase::DestroyImpl(); });
    ON_CALL(*this, GetMappedPointer).WillByDefault(Return(mBackingData.get()));
    ON_CALL(*this, IsCPUWritableAtCreation).WillByDefault([this] {
        return (GetUsage() & (wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite)) != 0;
    });
}

BufferMock::~BufferMock() = default;

}  // namespace dawn::native
