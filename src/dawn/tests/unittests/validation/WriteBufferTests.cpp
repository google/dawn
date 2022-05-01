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

#include <vector>

#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace {

class WriteBufferTest : public ValidationTest {
  public:
    wgpu::Buffer CreateWritableBuffer(uint64_t size) {
        wgpu::BufferDescriptor desc;
        desc.usage = wgpu::BufferUsage::CopyDst;
        desc.size = size;
        return device.CreateBuffer(&desc);
    }

    wgpu::CommandBuffer EncodeWriteBuffer(wgpu::Buffer buffer,
                                          uint64_t bufferOffset,
                                          uint64_t size) {
        std::vector<uint8_t> data(size);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteBuffer(buffer, bufferOffset, data.data(), size);
        return encoder.Finish();
    }
};

// Tests that the buffer offset is validated to be a multiple of 4 bytes.
TEST_F(WriteBufferTest, OffsetAlignment) {
    wgpu::Buffer buffer = CreateWritableBuffer(64);
    EncodeWriteBuffer(buffer, 0, 4);
    EncodeWriteBuffer(buffer, 4, 4);
    EncodeWriteBuffer(buffer, 60, 4);
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 1, 4));
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 2, 4));
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 3, 4));
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 5, 4));
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 11, 4));
}

// Tests that the buffer size is validated to be a multiple of 4 bytes.
TEST_F(WriteBufferTest, SizeAlignment) {
    wgpu::Buffer buffer = CreateWritableBuffer(64);
    EncodeWriteBuffer(buffer, 0, 64);
    EncodeWriteBuffer(buffer, 4, 60);
    EncodeWriteBuffer(buffer, 40, 24);
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 0, 63));
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 4, 1));
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 4, 2));
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 40, 23));
}

// Tests that the buffer size and offset are validated to fit within the bounds of the buffer.
TEST_F(WriteBufferTest, BufferBounds) {
    wgpu::Buffer buffer = CreateWritableBuffer(64);
    EncodeWriteBuffer(buffer, 0, 64);
    EncodeWriteBuffer(buffer, 4, 60);
    EncodeWriteBuffer(buffer, 40, 24);
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 0, 68));
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 4, 64));
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 60, 8));
    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 64, 4));
}

// Tests that the destination buffer's usage is validated to contain CopyDst.
TEST_F(WriteBufferTest, RequireCopyDstUsage) {
    wgpu::BufferDescriptor desc;
    desc.usage = wgpu::BufferUsage::CopySrc;
    desc.size = 64;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    ASSERT_DEVICE_ERROR(EncodeWriteBuffer(buffer, 0, 64));
}

// Tests that the destination buffer's state is validated at submission.
TEST_F(WriteBufferTest, ValidBufferState) {
    wgpu::BufferDescriptor desc;
    desc.usage = wgpu::BufferUsage::CopyDst;
    desc.size = 64;
    desc.mappedAtCreation = true;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    wgpu::CommandBuffer commands = EncodeWriteBuffer(buffer, 0, 64);
    ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));

    commands = EncodeWriteBuffer(buffer, 0, 64);
    buffer.Unmap();
    device.GetQueue().Submit(1, &commands);
}

}  // namespace
