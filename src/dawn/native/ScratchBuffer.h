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

#ifndef SRC_DAWN_NATIVE_SCRATCHBUFFER_H_
#define SRC_DAWN_NATIVE_SCRATCHBUFFER_H_

#include <cstdint>

#include "dawn/common/RefCounted.h"
#include "dawn/native/Buffer.h"

namespace dawn::native {

class DeviceBase;

// A ScratchBuffer is a lazily allocated and lazily grown GPU buffer for intermittent use by
// commands in the GPU queue. Note that scratch buffers are not zero-initialized, so users must
// be careful not to exposed uninitialized bytes to client shaders.
class ScratchBuffer {
  public:
    // Note that this object does not retain a reference to `device`, so `device` MUST outlive
    // this object.
    ScratchBuffer(DeviceBase* device, wgpu::BufferUsage usage);
    ~ScratchBuffer();

    // Resets this ScratchBuffer, guaranteeing that the next EnsureCapacity call allocates a
    // fresh buffer.
    void Reset();

    // Ensures that this ScratchBuffer is backed by a buffer on `device` with at least
    // `capacity` bytes of storage.
    MaybeError EnsureCapacity(uint64_t capacity);

    BufferBase* GetBuffer() const;

  private:
    DeviceBase* const mDevice;
    const wgpu::BufferUsage mUsage;
    Ref<BufferBase> mBuffer;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_SCRATCHBUFFER_H_
