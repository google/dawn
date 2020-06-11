// Copyright 2017 The Dawn Authors
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

#include "tests/DawnTest.h"

#include <cstring>

class BufferMapReadTests : public DawnTest {
    protected:
      static void MapReadCallback(WGPUBufferMapAsyncStatus status,
                                  const void* data,
                                  uint64_t,
                                  void* userdata) {
          ASSERT_EQ(WGPUBufferMapAsyncStatus_Success, status);
          ASSERT_NE(nullptr, data);

          static_cast<BufferMapReadTests*>(userdata)->mappedData = data;
      }

      const void* MapReadAsyncAndWait(const wgpu::Buffer& buffer) {
          buffer.MapReadAsync(MapReadCallback, this);

          while (mappedData == nullptr) {
              WaitABit();
          }

          return mappedData;
      }

      void UnmapBuffer(const wgpu::Buffer& buffer) {
          buffer.Unmap();
          mappedData = nullptr;
      }

    private:
        const void* mappedData = nullptr;
};

// Test that the simplest map read works.
TEST_P(BufferMapReadTests, SmallReadAtZero) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    uint32_t myData = 0x01020304;
    queue.WriteBuffer(buffer, 0, &myData, sizeof(myData));

    const void* mappedData = MapReadAsyncAndWait(buffer);
    ASSERT_EQ(myData, *reinterpret_cast<const uint32_t*>(mappedData));

    UnmapBuffer(buffer);
}

// Map, read and unmap twice. Test that both of these two iterations work.
TEST_P(BufferMapReadTests, MapTwice) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    uint32_t myData = 0x01020304;
    queue.WriteBuffer(buffer, 0, &myData, sizeof(myData));

    const void* mappedData = MapReadAsyncAndWait(buffer);
    EXPECT_EQ(myData, *reinterpret_cast<const uint32_t*>(mappedData));

    UnmapBuffer(buffer);

    myData = 0x05060708;
    queue.WriteBuffer(buffer, 0, &myData, sizeof(myData));

    const void* mappedData1 = MapReadAsyncAndWait(buffer);
    EXPECT_EQ(myData, *reinterpret_cast<const uint32_t*>(mappedData1));

    UnmapBuffer(buffer);
}

// Test mapping a large buffer.
TEST_P(BufferMapReadTests, LargeRead) {
    constexpr uint32_t kDataSize = 1000 * 1000;
    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }

    wgpu::BufferDescriptor descriptor;
    descriptor.size = static_cast<uint32_t>(kDataSize * sizeof(uint32_t));
    descriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    queue.WriteBuffer(buffer, 0, myData.data(), kDataSize * sizeof(uint32_t));

    const void* mappedData = MapReadAsyncAndWait(buffer);
    ASSERT_EQ(0, memcmp(mappedData, myData.data(), kDataSize * sizeof(uint32_t)));

    UnmapBuffer(buffer);
}

// Test mapping a zero-sized buffer.
TEST_P(BufferMapReadTests, ZeroSized) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 0;
    descriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    MapReadAsyncAndWait(buffer);
    UnmapBuffer(buffer);
}

DAWN_INSTANTIATE_TEST(BufferMapReadTests, D3D12Backend(), MetalBackend(), OpenGLBackend(), VulkanBackend());

class BufferMapWriteTests : public DawnTest {
    protected:
      static void MapWriteCallback(WGPUBufferMapAsyncStatus status,
                                   void* data,
                                   uint64_t,
                                   void* userdata) {
          ASSERT_EQ(WGPUBufferMapAsyncStatus_Success, status);
          ASSERT_NE(nullptr, data);

          static_cast<BufferMapWriteTests*>(userdata)->mappedData = data;
      }

      void* MapWriteAsyncAndWait(const wgpu::Buffer& buffer) {
          buffer.MapWriteAsync(MapWriteCallback, this);

          while (mappedData == nullptr) {
              WaitABit();
          }

          // Ensure the prior write's status is updated.
          void* resultPointer = mappedData;
          mappedData = nullptr;

          return resultPointer;
      }

      void UnmapBuffer(const wgpu::Buffer& buffer) {
          buffer.Unmap();
          mappedData = nullptr;
      }

    private:
        void* mappedData = nullptr;
};

// Test that the simplest map write works.
TEST_P(BufferMapWriteTests, SmallWriteAtZero) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    uint32_t myData = 2934875;
    void* mappedData = MapWriteAsyncAndWait(buffer);
    memcpy(mappedData, &myData, sizeof(myData));
    UnmapBuffer(buffer);

    EXPECT_BUFFER_U32_EQ(myData, buffer, 0);
}

// Map, write and unmap twice. Test that both of these two iterations work.
TEST_P(BufferMapWriteTests, MapTwice) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    uint32_t myData = 2934875;
    void* mappedData = MapWriteAsyncAndWait(buffer);
    memcpy(mappedData, &myData, sizeof(myData));
    UnmapBuffer(buffer);

    EXPECT_BUFFER_U32_EQ(myData, buffer, 0);

    myData = 9999999;
    void* mappedData1 = MapWriteAsyncAndWait(buffer);
    memcpy(mappedData1, &myData, sizeof(myData));
    UnmapBuffer(buffer);

    EXPECT_BUFFER_U32_EQ(myData, buffer, 0);
}

// Test mapping a large buffer.
TEST_P(BufferMapWriteTests, LargeWrite) {
    constexpr uint32_t kDataSize = 1000 * 1000;
    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }

    wgpu::BufferDescriptor descriptor;
    descriptor.size = static_cast<uint32_t>(kDataSize * sizeof(uint32_t));
    descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    void* mappedData = MapWriteAsyncAndWait(buffer);
    memcpy(mappedData, myData.data(), kDataSize * sizeof(uint32_t));
    UnmapBuffer(buffer);

    EXPECT_BUFFER_U32_RANGE_EQ(myData.data(), buffer, 0, kDataSize);
}

// Test mapping a zero-sized buffer.
TEST_P(BufferMapWriteTests, ZeroSized) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 0;
    descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    MapWriteAsyncAndWait(buffer);
    UnmapBuffer(buffer);
}

// Stress test mapping many buffers.
TEST_P(BufferMapWriteTests, ManyWrites) {
    constexpr uint32_t kDataSize = 1000;
    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }

    std::vector<wgpu::Buffer> buffers;

    constexpr uint32_t kBuffers = 100;
    for (uint32_t i = 0; i < kBuffers; ++i) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = static_cast<uint32_t>(kDataSize * sizeof(uint32_t));
        descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

        void* mappedData = MapWriteAsyncAndWait(buffer);
        memcpy(mappedData, myData.data(), kDataSize * sizeof(uint32_t));
        UnmapBuffer(buffer);

        buffers.push_back(buffer);  // Destroy buffers upon return.
    }

    for (uint32_t i = 0; i < kBuffers; ++i) {
        EXPECT_BUFFER_U32_RANGE_EQ(myData.data(), buffers[i], 0, kDataSize);
    }
}

DAWN_INSTANTIATE_TEST(BufferMapWriteTests, D3D12Backend(), MetalBackend(), OpenGLBackend(), VulkanBackend());

// TODO(enga): These tests should use the testing toggle to initialize resources to 1.
class CreateBufferMappedTests : public DawnTest {
    protected:
      static void MapReadCallback(WGPUBufferMapAsyncStatus status,
                                  const void* data,
                                  uint64_t,
                                  void* userdata) {
          ASSERT_EQ(WGPUBufferMapAsyncStatus_Success, status);
          ASSERT_NE(nullptr, data);

          static_cast<CreateBufferMappedTests*>(userdata)->mappedData = data;
      }

      const void* MapReadAsyncAndWait(const wgpu::Buffer& buffer) {
          buffer.MapReadAsync(MapReadCallback, this);

          while (mappedData == nullptr) {
              WaitABit();
          }

          return mappedData;
      }

      void UnmapBuffer(const wgpu::Buffer& buffer) {
          buffer.Unmap();
          mappedData = nullptr;
      }

      void CheckResultStartsZeroed(const wgpu::CreateBufferMappedResult& result, uint64_t size) {
          ASSERT_EQ(result.dataLength, size);
          for (uint64_t i = 0; i < result.dataLength; ++i) {
              uint8_t value = *(reinterpret_cast<uint8_t*>(result.data) + i);
              ASSERT_EQ(value, 0u);
          }
      }

      wgpu::CreateBufferMappedResult CreateBufferMapped(wgpu::BufferUsage usage, uint64_t size) {
          wgpu::BufferDescriptor descriptor = {};
          descriptor.size = size;
          descriptor.usage = usage;

          wgpu::CreateBufferMappedResult result = device.CreateBufferMapped(&descriptor);
          CheckResultStartsZeroed(result, size);
          return result;
      }

      wgpu::CreateBufferMappedResult CreateBufferMappedWithData(wgpu::BufferUsage usage,
                                                                const std::vector<uint32_t>& data) {
          size_t byteLength = data.size() * sizeof(uint32_t);
          wgpu::CreateBufferMappedResult result = CreateBufferMapped(usage, byteLength);
          memcpy(result.data, data.data(), byteLength);

          return result;
      }

    private:
        const void* mappedData = nullptr;
};

// Test that the simplest CreateBufferMapped works for MapWrite buffers.
TEST_P(CreateBufferMappedTests, MapWriteUsageSmall) {
    uint32_t myData = 230502;
    wgpu::CreateBufferMappedResult result = CreateBufferMappedWithData(
        wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc, {myData});
    UnmapBuffer(result.buffer);
    EXPECT_BUFFER_U32_EQ(myData, result.buffer, 0);
}

// Test that the simplest CreateBufferMapped works for MapRead buffers.
TEST_P(CreateBufferMappedTests, MapReadUsageSmall) {
    uint32_t myData = 230502;
    wgpu::CreateBufferMappedResult result =
        CreateBufferMappedWithData(wgpu::BufferUsage::MapRead, {myData});
    UnmapBuffer(result.buffer);

    const void* mappedData = MapReadAsyncAndWait(result.buffer);
    ASSERT_EQ(myData, *reinterpret_cast<const uint32_t*>(mappedData));
    UnmapBuffer(result.buffer);
}

// Test that the simplest CreateBufferMapped works for non-mappable buffers.
TEST_P(CreateBufferMappedTests, NonMappableUsageSmall) {
    uint32_t myData = 4239;
    wgpu::CreateBufferMappedResult result =
        CreateBufferMappedWithData(wgpu::BufferUsage::CopySrc, {myData});
    UnmapBuffer(result.buffer);

    EXPECT_BUFFER_U32_EQ(myData, result.buffer, 0);
}

// Test CreateBufferMapped for a large MapWrite buffer
TEST_P(CreateBufferMappedTests, MapWriteUsageLarge) {
    constexpr uint64_t kDataSize = 1000 * 1000;
    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }

    wgpu::CreateBufferMappedResult result = CreateBufferMappedWithData(
        wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc, {myData});
    UnmapBuffer(result.buffer);

    EXPECT_BUFFER_U32_RANGE_EQ(myData.data(), result.buffer, 0, kDataSize);
}

// Test CreateBufferMapped for a large MapRead buffer
TEST_P(CreateBufferMappedTests, MapReadUsageLarge) {
    constexpr uint64_t kDataSize = 1000 * 1000;
    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }

    wgpu::CreateBufferMappedResult result =
        CreateBufferMappedWithData(wgpu::BufferUsage::MapRead, myData);
    UnmapBuffer(result.buffer);

    const void* mappedData = MapReadAsyncAndWait(result.buffer);
    ASSERT_EQ(0, memcmp(mappedData, myData.data(), kDataSize * sizeof(uint32_t)));
    UnmapBuffer(result.buffer);
}

// Test CreateBufferMapped for a large non-mappable buffer
TEST_P(CreateBufferMappedTests, NonMappableUsageLarge) {
    constexpr uint64_t kDataSize = 1000 * 1000;
    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }

    wgpu::CreateBufferMappedResult result =
        CreateBufferMappedWithData(wgpu::BufferUsage::CopySrc, {myData});
    UnmapBuffer(result.buffer);

    EXPECT_BUFFER_U32_RANGE_EQ(myData.data(), result.buffer, 0, kDataSize);
}

// Test destroying a non-mappable buffer mapped at creation.
// This is a regression test for an issue where the D3D12 backend thought the buffer was actually
// mapped and tried to unlock the heap residency (when actually the buffer was using a staging
// buffer)
TEST_P(CreateBufferMappedTests, DestroyNonMappableWhileMappedForCreation) {
    wgpu::CreateBufferMappedResult result = CreateBufferMapped(wgpu::BufferUsage::CopySrc, 4);
    result.buffer.Destroy();
}

// Test destroying a mappable buffer mapped at creation.
TEST_P(CreateBufferMappedTests, DestroyMappableWhileMappedForCreation) {
    wgpu::CreateBufferMappedResult result = CreateBufferMapped(wgpu::BufferUsage::MapRead, 4);
    result.buffer.Destroy();
}

// Test that mapping a buffer is valid after CreateBufferMapped and Unmap
TEST_P(CreateBufferMappedTests, CreateThenMapSuccess) {
    static uint32_t myData = 230502;
    static uint32_t myData2 = 1337;
    wgpu::CreateBufferMappedResult result = CreateBufferMappedWithData(
        wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc, {myData});
    UnmapBuffer(result.buffer);

    EXPECT_BUFFER_U32_EQ(myData, result.buffer, 0);

    bool done = false;
    result.buffer.MapWriteAsync(
        [](WGPUBufferMapAsyncStatus status, void* data, uint64_t, void* userdata) {
            ASSERT_EQ(WGPUBufferMapAsyncStatus_Success, status);
            ASSERT_NE(nullptr, data);

            *static_cast<uint32_t*>(data) = myData2;
            *static_cast<bool*>(userdata) = true;
        },
        &done);

    while (!done) {
        WaitABit();
    }

    UnmapBuffer(result.buffer);
    EXPECT_BUFFER_U32_EQ(myData2, result.buffer, 0);
}

// Test that is is invalid to map a buffer twice when using CreateBufferMapped
TEST_P(CreateBufferMappedTests, CreateThenMapBeforeUnmapFailure) {
    uint32_t myData = 230502;
    wgpu::CreateBufferMappedResult result = CreateBufferMappedWithData(
        wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc, {myData});

    ASSERT_DEVICE_ERROR([&]() {
        bool done = false;
        result.buffer.MapWriteAsync(
            [](WGPUBufferMapAsyncStatus status, void* data, uint64_t, void* userdata) {
                ASSERT_EQ(WGPUBufferMapAsyncStatus_Error, status);
                ASSERT_EQ(nullptr, data);

                *static_cast<bool*>(userdata) = true;
            },
            &done);

        while (!done) {
            WaitABit();
        }
    }());

    // CreateBufferMapped is unaffected by the MapWrite error.
    UnmapBuffer(result.buffer);
    EXPECT_BUFFER_U32_EQ(myData, result.buffer, 0);
}

// Test that creating a very large buffers fails gracefully.
TEST_P(CreateBufferMappedTests, LargeBufferFails) {
    // TODO(http://crbug.com/dawn/27): Missing support.
    DAWN_SKIP_TEST_IF(IsOpenGL());

    wgpu::BufferDescriptor descriptor;
    descriptor.size = std::numeric_limits<uint64_t>::max();
    descriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    ASSERT_DEVICE_ERROR(device.CreateBuffer(&descriptor));
}

// Test that creating a zero-sized buffer mapped is allowed.
TEST_P(CreateBufferMappedTests, ZeroSized) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 0;
    descriptor.usage = wgpu::BufferUsage::Vertex;
    wgpu::CreateBufferMappedResult result = device.CreateBufferMapped(&descriptor);

    ASSERT_EQ(0u, result.dataLength);
    ASSERT_NE(nullptr, result.data);

    // Check that unmapping the buffer works too.
    UnmapBuffer(result.buffer);
}

// Test that creating a zero-sized mapppable buffer mapped. (it is a different code path)
TEST_P(CreateBufferMappedTests, ZeroSizedMappableBuffer) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 0;
    descriptor.usage = wgpu::BufferUsage::MapWrite;
    wgpu::CreateBufferMappedResult result = device.CreateBufferMapped(&descriptor);

    ASSERT_EQ(0u, result.dataLength);
    ASSERT_NE(nullptr, result.data);

    // Check that unmapping the buffer works too.
    UnmapBuffer(result.buffer);
}

// Test that creating a zero-sized error buffer mapped. (it is a different code path)
TEST_P(CreateBufferMappedTests, ZeroSizedErrorBuffer) {
    DAWN_SKIP_TEST_IF(IsDawnValidationSkipped());

    wgpu::BufferDescriptor descriptor;
    descriptor.size = 0;
    descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::Storage;
    wgpu::CreateBufferMappedResult result;
    ASSERT_DEVICE_ERROR(result = device.CreateBufferMapped(&descriptor));

    ASSERT_EQ(0u, result.dataLength);
    ASSERT_NE(nullptr, result.data);
}

DAWN_INSTANTIATE_TEST(CreateBufferMappedTests,
                      D3D12Backend(),
                      D3D12Backend({}, {"use_d3d12_resource_heap_tier2"}),
                      MetalBackend(),
                      OpenGLBackend(),
                      VulkanBackend());

class BufferTests : public DawnTest {};

// Test that creating a zero-buffer is allowed.
TEST_P(BufferTests, ZeroSizedBuffer) {
    wgpu::BufferDescriptor desc;
    desc.size = 0;
    desc.usage = wgpu::BufferUsage::CopyDst;
    device.CreateBuffer(&desc);
}

// Test that creating a very large buffers fails gracefully.
TEST_P(BufferTests, LargeBufferFails) {
    // TODO(http://crbug.com/dawn/27): Missing support.
    DAWN_SKIP_TEST_IF(IsOpenGL());

    wgpu::BufferDescriptor descriptor;
    descriptor.size = std::numeric_limits<uint64_t>::max();
    descriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    ASSERT_DEVICE_ERROR(device.CreateBuffer(&descriptor));
}

DAWN_INSTANTIATE_TEST(BufferTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      VulkanBackend());
