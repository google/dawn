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
      static void MapReadCallback(DawnBufferMapAsyncStatus status,
                                  const void* data,
                                  uint64_t,
                                  void* userdata) {
          ASSERT_EQ(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, status);
          ASSERT_NE(nullptr, data);

          static_cast<BufferMapReadTests*>(userdata)->mappedData = data;
      }

      const void* MapReadAsyncAndWait(const dawn::Buffer& buffer) {
          buffer.MapReadAsync(MapReadCallback, this);

          while (mappedData == nullptr) {
              WaitABit();
          }

          return mappedData;
      }

    private:
        const void* mappedData = nullptr;
};

// Test that the simplest map read works.
TEST_P(BufferMapReadTests, SmallReadAtZero) {
    dawn::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = dawn::BufferUsage::MapRead | dawn::BufferUsage::CopyDst;
    dawn::Buffer buffer = device.CreateBuffer(&descriptor);

    uint32_t myData = 0x01020304;
    buffer.SetSubData(0, sizeof(myData), &myData);

    const void* mappedData = MapReadAsyncAndWait(buffer);
    ASSERT_EQ(myData, *reinterpret_cast<const uint32_t*>(mappedData));

    buffer.Unmap();
}

// Test mapping a large buffer.
TEST_P(BufferMapReadTests, LargeRead) {
    constexpr uint32_t kDataSize = 1000 * 1000;
    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }

    dawn::BufferDescriptor descriptor;
    descriptor.size = static_cast<uint32_t>(kDataSize * sizeof(uint32_t));
    descriptor.usage = dawn::BufferUsage::MapRead | dawn::BufferUsage::CopyDst;
    dawn::Buffer buffer = device.CreateBuffer(&descriptor);

    buffer.SetSubData(0, kDataSize * sizeof(uint32_t), myData.data());

    const void* mappedData = MapReadAsyncAndWait(buffer);
    ASSERT_EQ(0, memcmp(mappedData, myData.data(), kDataSize * sizeof(uint32_t)));

    buffer.Unmap();
}

DAWN_INSTANTIATE_TEST(BufferMapReadTests, D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend);

class BufferMapWriteTests : public DawnTest {
    protected:
      static void MapWriteCallback(DawnBufferMapAsyncStatus status,
                                   void* data,
                                   uint64_t,
                                   void* userdata) {
          ASSERT_EQ(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, status);
          ASSERT_NE(nullptr, data);

          static_cast<BufferMapWriteTests*>(userdata)->mappedData = data;
      }

      void* MapWriteAsyncAndWait(const dawn::Buffer& buffer) {
          buffer.MapWriteAsync(MapWriteCallback, this);

          while (mappedData == nullptr) {
              WaitABit();
          }

          // Ensure the prior write's status is updated.
          void* resultPointer = mappedData;
          mappedData = nullptr;

          return resultPointer;
      }

    private:
        void* mappedData = nullptr;
};

// Test that the simplest map write works.
TEST_P(BufferMapWriteTests, SmallWriteAtZero) {
    dawn::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = dawn::BufferUsage::MapWrite | dawn::BufferUsage::CopySrc;
    dawn::Buffer buffer = device.CreateBuffer(&descriptor);

    uint32_t myData = 2934875;
    void* mappedData = MapWriteAsyncAndWait(buffer);
    memcpy(mappedData, &myData, sizeof(myData));
    buffer.Unmap();

    EXPECT_BUFFER_U32_EQ(myData, buffer, 0);
}

// Test mapping a large buffer.
TEST_P(BufferMapWriteTests, LargeWrite) {
    constexpr uint32_t kDataSize = 1000 * 1000;
    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }

    dawn::BufferDescriptor descriptor;
    descriptor.size = static_cast<uint32_t>(kDataSize * sizeof(uint32_t));
    descriptor.usage = dawn::BufferUsage::MapWrite | dawn::BufferUsage::CopySrc;
    dawn::Buffer buffer = device.CreateBuffer(&descriptor);

    void* mappedData = MapWriteAsyncAndWait(buffer);
    memcpy(mappedData, myData.data(), kDataSize * sizeof(uint32_t));
    buffer.Unmap();

    EXPECT_BUFFER_U32_RANGE_EQ(myData.data(), buffer, 0, kDataSize);
}

// Stress test mapping many buffers.
TEST_P(BufferMapWriteTests, ManyWrites) {
    constexpr uint32_t kDataSize = 1000;
    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }

    std::vector<dawn::Buffer> buffers;

    constexpr uint32_t kBuffers = 100;
    for (uint32_t i = 0; i < kBuffers; ++i) {
        dawn::BufferDescriptor descriptor;
        descriptor.size = static_cast<uint32_t>(kDataSize * sizeof(uint32_t));
        descriptor.usage = dawn::BufferUsage::MapWrite | dawn::BufferUsage::CopySrc;
        dawn::Buffer buffer = device.CreateBuffer(&descriptor);

        void* mappedData = MapWriteAsyncAndWait(buffer);
        memcpy(mappedData, myData.data(), kDataSize * sizeof(uint32_t));
        buffer.Unmap();

        buffers.push_back(buffer);  // Destroy buffers upon return.
    }

    for (uint32_t i = 0; i < kBuffers; ++i) {
        EXPECT_BUFFER_U32_RANGE_EQ(myData.data(), buffers[i], 0, kDataSize);
    }
}

DAWN_INSTANTIATE_TEST(BufferMapWriteTests, D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend);

class BufferSetSubDataTests : public DawnTest {
};

// Test the simplest set sub data: setting one u32 at offset 0.
TEST_P(BufferSetSubDataTests, SmallDataAtZero) {
    dawn::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = dawn::BufferUsage::CopySrc | dawn::BufferUsage::CopyDst;
    dawn::Buffer buffer = device.CreateBuffer(&descriptor);

    uint32_t value = 0x01020304;
    buffer.SetSubData(0, sizeof(value), &value);

    EXPECT_BUFFER_U32_EQ(value, buffer, 0);
}

// Test that SetSubData offset works.
TEST_P(BufferSetSubDataTests, SmallDataAtOffset) {
    dawn::BufferDescriptor descriptor;
    descriptor.size = 4000;
    descriptor.usage = dawn::BufferUsage::CopySrc | dawn::BufferUsage::CopyDst;
    dawn::Buffer buffer = device.CreateBuffer(&descriptor);

    constexpr uint64_t kOffset = 2000;
    uint32_t value = 0x01020304;
    buffer.SetSubData(kOffset, sizeof(value), &value);

    EXPECT_BUFFER_U32_EQ(value, buffer, kOffset);
}

// Stress test for many calls to SetSubData
TEST_P(BufferSetSubDataTests, ManySetSubData) {
    // Note: Increasing the size of the buffer will likely cause timeout issues.
    // In D3D12, timeout detection occurs when the GPU scheduler tries but cannot preempt the task
    // executing these commands in-flight. If this takes longer than ~2s, a device reset occurs and
    // fails the test. Since GPUs may or may not complete by then, this test must be disabled OR
    // modified to be well-below the timeout limit.

    // TODO (jiawei.shao@intel.com): find out why this test fails on Intel Vulkan Linux bots.
    DAWN_SKIP_TEST_IF(IsIntel() && IsVulkan() && IsLinux());

    constexpr uint64_t kSize = 4000 * 1000;
    constexpr uint32_t kElements = 500 * 500;
    dawn::BufferDescriptor descriptor;
    descriptor.size = kSize;
    descriptor.usage = dawn::BufferUsage::CopySrc | dawn::BufferUsage::CopyDst;
    dawn::Buffer buffer = device.CreateBuffer(&descriptor);

    std::vector<uint32_t> expectedData;
    for (uint32_t i = 0; i < kElements; ++i) {
        buffer.SetSubData(i * sizeof(uint32_t), sizeof(i), &i);
        expectedData.push_back(i);
    }

    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), buffer, 0, kElements);
}

// Test using SetSubData for lots of data
TEST_P(BufferSetSubDataTests, LargeSetSubData) {
    constexpr uint64_t kSize = 4000 * 1000;
    constexpr uint32_t kElements = 1000 * 1000;
    dawn::BufferDescriptor descriptor;
    descriptor.size = kSize;
    descriptor.usage = dawn::BufferUsage::CopySrc | dawn::BufferUsage::CopyDst;
    dawn::Buffer buffer = device.CreateBuffer(&descriptor);

    std::vector<uint32_t> expectedData;
    for (uint32_t i = 0; i < kElements; ++i) {
        expectedData.push_back(i);
    }

    buffer.SetSubData(0, kElements * sizeof(uint32_t), expectedData.data());

    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), buffer, 0, kElements);
}

DAWN_INSTANTIATE_TEST(BufferSetSubDataTests,
                     D3D12Backend,
                     MetalBackend,
                     OpenGLBackend,
                     VulkanBackend);

// TODO(enga): These tests should use the testing toggle to initialize resources to 1.
class CreateBufferMappedTests : public DawnTest {
    protected:
      static void MapReadCallback(DawnBufferMapAsyncStatus status,
                                  const void* data,
                                  uint64_t,
                                  void* userdata) {
          ASSERT_EQ(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, status);
          ASSERT_NE(nullptr, data);

          static_cast<CreateBufferMappedTests*>(userdata)->mappedData = data;
      }

      const void* MapReadAsyncAndWait(const dawn::Buffer& buffer) {
          buffer.MapReadAsync(MapReadCallback, this);

          while (mappedData == nullptr) {
              WaitABit();
          }

          return mappedData;
      }

      void CheckResultStartsZeroed(const dawn::CreateBufferMappedResult& result, uint64_t size) {
          ASSERT_EQ(result.dataLength, size);
          for (uint64_t i = 0; i < result.dataLength; ++i) {
              uint8_t value = *(reinterpret_cast<uint8_t*>(result.data) + i);
              ASSERT_EQ(value, 0u);
          }
      }

      dawn::CreateBufferMappedResult CreateBufferMapped(dawn::BufferUsage usage, uint64_t size) {
          dawn::BufferDescriptor descriptor;
          descriptor.nextInChain = nullptr;
          descriptor.size = size;
          descriptor.usage = usage;

          dawn::CreateBufferMappedResult result = device.CreateBufferMapped(&descriptor);
          CheckResultStartsZeroed(result, size);
          return result;
      }

      dawn::CreateBufferMappedResult CreateBufferMappedWithData(dawn::BufferUsage usage,
                                                                const std::vector<uint32_t>& data) {
          size_t byteLength = data.size() * sizeof(uint32_t);
          dawn::CreateBufferMappedResult result = CreateBufferMapped(usage, byteLength);
          memcpy(result.data, data.data(), byteLength);

          return result;
      }

      template <DawnBufferMapAsyncStatus expectedStatus = DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS>
      dawn::CreateBufferMappedResult CreateBufferMappedAsyncAndWait(dawn::BufferUsage usage,
                                                                    uint64_t size) {
          dawn::BufferDescriptor descriptor;
          descriptor.nextInChain = nullptr;
          descriptor.size = size;
          descriptor.usage = usage;

          struct ResultInfo {
              dawn::CreateBufferMappedResult result;
              bool done = false;
          } resultInfo;

          device.CreateBufferMappedAsync(
              &descriptor,
              [](DawnBufferMapAsyncStatus status, DawnCreateBufferMappedResult result,
                 void* userdata) {
                  ASSERT_EQ(status, expectedStatus);
                  auto* resultInfo = reinterpret_cast<ResultInfo*>(userdata);
                  resultInfo->result.buffer = dawn::Buffer::Acquire(result.buffer);
                  resultInfo->result.data = result.data;
                  resultInfo->result.dataLength = result.dataLength;
                  resultInfo->done = true;
              },
              &resultInfo);

          while (!resultInfo.done) {
              WaitABit();
          }

          CheckResultStartsZeroed(resultInfo.result, size);

          return resultInfo.result;
      }

      dawn::CreateBufferMappedResult CreateBufferMappedAsyncWithDataAndWait(
          dawn::BufferUsage usage,
          const std::vector<uint32_t>& data) {
          size_t byteLength = data.size() * sizeof(uint32_t);
          dawn::CreateBufferMappedResult result = CreateBufferMappedAsyncAndWait(usage, byteLength);
          memcpy(result.data, data.data(), byteLength);

          return result;
      }

    private:
        const void* mappedData = nullptr;
};

// Test that the simplest CreateBufferMapped works for MapWrite buffers.
TEST_P(CreateBufferMappedTests, MapWriteUsageSmall) {
    uint32_t myData = 230502;
    dawn::CreateBufferMappedResult result = CreateBufferMappedWithData(
        dawn::BufferUsage::MapWrite | dawn::BufferUsage::CopySrc, {myData});
    result.buffer.Unmap();
    EXPECT_BUFFER_U32_EQ(myData, result.buffer, 0);
}

// Test that the simplest CreateBufferMapped works for MapRead buffers.
TEST_P(CreateBufferMappedTests, MapReadUsageSmall) {
    uint32_t myData = 230502;
    dawn::CreateBufferMappedResult result =
        CreateBufferMappedWithData(dawn::BufferUsage::MapRead, {myData});
    result.buffer.Unmap();

    const void* mappedData = MapReadAsyncAndWait(result.buffer);
    ASSERT_EQ(myData, *reinterpret_cast<const uint32_t*>(mappedData));
    result.buffer.Unmap();
}

// Test that the simplest CreateBufferMapped works for non-mappable buffers.
TEST_P(CreateBufferMappedTests, NonMappableUsageSmall) {
    uint32_t myData = 4239;
    dawn::CreateBufferMappedResult result =
        CreateBufferMappedWithData(dawn::BufferUsage::CopySrc, {myData});
    result.buffer.Unmap();

    EXPECT_BUFFER_U32_EQ(myData, result.buffer, 0);
}

// Test CreateBufferMapped for a large MapWrite buffer
TEST_P(CreateBufferMappedTests, MapWriteUsageLarge) {
    constexpr uint64_t kDataSize = 1000 * 1000;
    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }

    dawn::CreateBufferMappedResult result = CreateBufferMappedWithData(
        dawn::BufferUsage::MapWrite | dawn::BufferUsage::CopySrc, {myData});
    result.buffer.Unmap();

    EXPECT_BUFFER_U32_RANGE_EQ(myData.data(), result.buffer, 0, kDataSize);
}

// Test CreateBufferMapped for a large MapRead buffer
TEST_P(CreateBufferMappedTests, MapReadUsageLarge) {
    constexpr uint64_t kDataSize = 1000 * 1000;
    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }

    dawn::CreateBufferMappedResult result =
        CreateBufferMappedWithData(dawn::BufferUsage::MapRead, myData);
    result.buffer.Unmap();

    const void* mappedData = MapReadAsyncAndWait(result.buffer);
    ASSERT_EQ(0, memcmp(mappedData, myData.data(), kDataSize * sizeof(uint32_t)));
    result.buffer.Unmap();
}

// Test CreateBufferMapped for a large non-mappable buffer
TEST_P(CreateBufferMappedTests, NonMappableUsageLarge) {
    constexpr uint64_t kDataSize = 1000 * 1000;
    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }

    dawn::CreateBufferMappedResult result =
        CreateBufferMappedWithData(dawn::BufferUsage::CopySrc, {myData});
    result.buffer.Unmap();

    EXPECT_BUFFER_U32_RANGE_EQ(myData.data(), result.buffer, 0, kDataSize);
}

// Test that mapping a buffer is valid after CreateBufferMapped and Unmap
TEST_P(CreateBufferMappedTests, CreateThenMapSuccess) {
    static uint32_t myData = 230502;
    static uint32_t myData2 = 1337;
    dawn::CreateBufferMappedResult result = CreateBufferMappedWithData(
        dawn::BufferUsage::MapWrite | dawn::BufferUsage::CopySrc, {myData});
    result.buffer.Unmap();

    EXPECT_BUFFER_U32_EQ(myData, result.buffer, 0);

    bool done = false;
    result.buffer.MapWriteAsync(
        [](DawnBufferMapAsyncStatus status, void* data, uint64_t, void* userdata) {
            ASSERT_EQ(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, status);
            ASSERT_NE(nullptr, data);

            *static_cast<uint32_t*>(data) = myData2;
            *static_cast<bool*>(userdata) = true;
        },
        &done);

    while (!done) {
        WaitABit();
    }

    result.buffer.Unmap();
    EXPECT_BUFFER_U32_EQ(myData2, result.buffer, 0);
}

// Test that is is invalid to map a buffer twice when using CreateBufferMapped
TEST_P(CreateBufferMappedTests, CreateThenMapBeforeUnmapFailure) {
    uint32_t myData = 230502;
    dawn::CreateBufferMappedResult result = CreateBufferMappedWithData(
        dawn::BufferUsage::MapWrite | dawn::BufferUsage::CopySrc, {myData});

    ASSERT_DEVICE_ERROR([&]() {
        bool done = false;
        result.buffer.MapWriteAsync(
            [](DawnBufferMapAsyncStatus status, void* data, uint64_t, void* userdata) {
                ASSERT_EQ(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, status);
                ASSERT_EQ(nullptr, data);

                *static_cast<bool*>(userdata) = true;
            },
            &done);

        while (!done) {
            WaitABit();
        }
    }());

    // CreateBufferMapped is unaffected by the MapWrite error.
    result.buffer.Unmap();
    EXPECT_BUFFER_U32_EQ(myData, result.buffer, 0);
}

// Test that the simplest CreateBufferMappedAsync works for MapWrite buffers.
TEST_P(CreateBufferMappedTests, MapWriteUsageSmallAsync) {
    uint32_t myData = 230502;
    dawn::CreateBufferMappedResult result = CreateBufferMappedAsyncWithDataAndWait(
        dawn::BufferUsage::MapWrite | dawn::BufferUsage::CopySrc, {myData});
    result.buffer.Unmap();
    EXPECT_BUFFER_U32_EQ(myData, result.buffer, 0);
}

// Test that the simplest CreateBufferMappedAsync works for MapRead buffers.
TEST_P(CreateBufferMappedTests, MapReadUsageSmallAsync) {
    uint32_t myData = 230502;
    dawn::CreateBufferMappedResult result =
        CreateBufferMappedAsyncWithDataAndWait(dawn::BufferUsage::MapRead, {myData});
    result.buffer.Unmap();

    const void* mappedData = MapReadAsyncAndWait(result.buffer);
    ASSERT_EQ(myData, *reinterpret_cast<const uint32_t*>(mappedData));
    result.buffer.Unmap();
}

// Test that the simplest CreateBufferMappedAsync works for non-mappable buffers.
TEST_P(CreateBufferMappedTests, NonMappableUsageSmallAsync) {
    uint32_t myData = 4239;
    dawn::CreateBufferMappedResult result =
        CreateBufferMappedAsyncWithDataAndWait(dawn::BufferUsage::CopySrc, {myData});
    result.buffer.Unmap();

    EXPECT_BUFFER_U32_EQ(myData, result.buffer, 0);
}

// Test CreateBufferMappedAsync for a large MapWrite buffer
TEST_P(CreateBufferMappedTests, MapWriteUsageLargeAsync) {
    constexpr uint64_t kDataSize = 1000 * 1000;
    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }

    dawn::CreateBufferMappedResult result = CreateBufferMappedAsyncWithDataAndWait(
        dawn::BufferUsage::MapWrite | dawn::BufferUsage::CopySrc, {myData});
    result.buffer.Unmap();

    EXPECT_BUFFER_U32_RANGE_EQ(myData.data(), result.buffer, 0, kDataSize);
}

// Test CreateBufferMappedAsync for a large MapRead buffer
TEST_P(CreateBufferMappedTests, MapReadUsageLargeAsync) {
    constexpr uint64_t kDataSize = 1000 * 1000;
    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }

    dawn::CreateBufferMappedResult result =
        CreateBufferMappedAsyncWithDataAndWait(dawn::BufferUsage::MapRead, {myData});
    result.buffer.Unmap();

    const void* mappedData = MapReadAsyncAndWait(result.buffer);
    ASSERT_EQ(0, memcmp(mappedData, myData.data(), kDataSize * sizeof(uint32_t)));
    result.buffer.Unmap();
}

// Test CreateBufferMappedAsync for a large non-mappable buffer
TEST_P(CreateBufferMappedTests, NonMappableUsageLargeAsync) {
    constexpr uint64_t kDataSize = 1000 * 1000;
    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }

    dawn::CreateBufferMappedResult result =
        CreateBufferMappedAsyncWithDataAndWait(dawn::BufferUsage::CopySrc, {myData});
    result.buffer.Unmap();

    EXPECT_BUFFER_U32_RANGE_EQ(myData.data(), result.buffer, 0, kDataSize);
}

// Test that mapping a buffer is valid after CreateBufferMappedAsync and Unmap
TEST_P(CreateBufferMappedTests, CreateThenMapSuccessAsync) {
    static uint32_t myData = 230502;
    static uint32_t myData2 = 1337;
    dawn::CreateBufferMappedResult result = CreateBufferMappedAsyncWithDataAndWait(
        dawn::BufferUsage::MapWrite | dawn::BufferUsage::CopySrc, {myData});
    result.buffer.Unmap();

    EXPECT_BUFFER_U32_EQ(myData, result.buffer, 0);

    bool done = false;
    result.buffer.MapWriteAsync(
        [](DawnBufferMapAsyncStatus status, void* data, uint64_t, void* userdata) {
            ASSERT_EQ(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, status);
            ASSERT_NE(nullptr, data);

            *static_cast<uint32_t*>(data) = myData2;
            *static_cast<bool*>(userdata) = true;
        },
        &done);

    while (!done) {
        WaitABit();
    }

    result.buffer.Unmap();
    EXPECT_BUFFER_U32_EQ(myData2, result.buffer, 0);
}

// Test that is is invalid to map a buffer twice when using CreateBufferMappedAsync
TEST_P(CreateBufferMappedTests, CreateThenMapBeforeUnmapFailureAsync) {
    uint32_t myData = 230502;
    dawn::CreateBufferMappedResult result = CreateBufferMappedAsyncWithDataAndWait(
        dawn::BufferUsage::MapWrite | dawn::BufferUsage::CopySrc, {myData});

    ASSERT_DEVICE_ERROR([&]() {
        bool done = false;
        result.buffer.MapWriteAsync(
            [](DawnBufferMapAsyncStatus status, void* data, uint64_t, void* userdata) {
                ASSERT_EQ(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, status);
                ASSERT_EQ(nullptr, data);

                *static_cast<bool*>(userdata) = true;
            },
            &done);

        while (!done) {
            WaitABit();
        }
    }());

    // CreateBufferMappedAsync is unaffected by the MapWrite error.
    result.buffer.Unmap();
    EXPECT_BUFFER_U32_EQ(myData, result.buffer, 0);
}

DAWN_INSTANTIATE_TEST(CreateBufferMappedTests,
                      D3D12Backend,
                      MetalBackend,
                      OpenGLBackend,
                      VulkanBackend);
