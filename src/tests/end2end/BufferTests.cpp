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
      static void MapReadCallback(dawnBufferMapAsyncStatus status,
                                  const void* data,
                                  uint32_t,
                                  dawnCallbackUserdata userdata) {
          ASSERT_EQ(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, status);
          ASSERT_NE(nullptr, data);

          auto test = reinterpret_cast<BufferMapReadTests*>(static_cast<uintptr_t>(userdata));
          test->mappedData = data;
      }

      const void* MapReadAsyncAndWait(const dawn::Buffer& buffer) {
          buffer.MapReadAsync(MapReadCallback, static_cast<dawn::CallbackUserdata>(
                                                   reinterpret_cast<uintptr_t>(this)));

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
    descriptor.size = 1;
    descriptor.usage = dawn::BufferUsageBit::MapRead | dawn::BufferUsageBit::TransferDst;
    dawn::Buffer buffer = device.CreateBuffer(&descriptor);

    uint8_t myData = 187;
    buffer.SetSubData(0, sizeof(myData), &myData);

    const void* mappedData = MapReadAsyncAndWait(buffer);
    ASSERT_EQ(myData, *reinterpret_cast<const uint8_t*>(mappedData));

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
    descriptor.usage = dawn::BufferUsageBit::MapRead | dawn::BufferUsageBit::TransferDst;
    dawn::Buffer buffer = device.CreateBuffer(&descriptor);

    buffer.SetSubData(0, kDataSize * sizeof(uint32_t), reinterpret_cast<uint8_t*>(myData.data()));

    const void* mappedData = MapReadAsyncAndWait(buffer);
    ASSERT_EQ(0, memcmp(mappedData, myData.data(), kDataSize * sizeof(uint32_t)));

    buffer.Unmap();
}

DAWN_INSTANTIATE_TEST(BufferMapReadTests, D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend)

class BufferMapWriteTests : public DawnTest {
    protected:
      static void MapWriteCallback(dawnBufferMapAsyncStatus status,
                                   void* data,
                                   uint32_t,
                                   dawnCallbackUserdata userdata) {
          ASSERT_EQ(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, status);
          ASSERT_NE(nullptr, data);

          auto test = reinterpret_cast<BufferMapWriteTests*>(static_cast<uintptr_t>(userdata));
          test->mappedData = data;
      }

      void* MapWriteAsyncAndWait(const dawn::Buffer& buffer) {
          buffer.MapWriteAsync(MapWriteCallback, static_cast<dawn::CallbackUserdata>(
                                                     reinterpret_cast<uintptr_t>(this)));

          while (mappedData == nullptr) {
              WaitABit();
          }

          return mappedData;
      }

    private:
        void* mappedData = nullptr;
};

// Test that the simplest map write works.
TEST_P(BufferMapWriteTests, SmallWriteAtZero) {
    dawn::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = dawn::BufferUsageBit::MapWrite | dawn::BufferUsageBit::TransferSrc;
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
    descriptor.usage = dawn::BufferUsageBit::MapWrite | dawn::BufferUsageBit::TransferSrc;
    dawn::Buffer buffer = device.CreateBuffer(&descriptor);

    void* mappedData = MapWriteAsyncAndWait(buffer);
    memcpy(mappedData, myData.data(), kDataSize * sizeof(uint32_t));
    buffer.Unmap();

    EXPECT_BUFFER_U32_RANGE_EQ(myData.data(), buffer, 0, kDataSize);
}

DAWN_INSTANTIATE_TEST(BufferMapWriteTests, D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend)

class BufferSetSubDataTests : public DawnTest {
};

// Test the simplest set sub data: setting one u8 at offset 0.
TEST_P(BufferSetSubDataTests, SmallDataAtZero) {
    dawn::BufferDescriptor descriptor;
    descriptor.size = 1;
    descriptor.usage = dawn::BufferUsageBit::TransferSrc | dawn::BufferUsageBit::TransferDst;
    dawn::Buffer buffer = device.CreateBuffer(&descriptor);

    uint8_t value = 171;
    buffer.SetSubData(0, sizeof(value), &value);

    EXPECT_BUFFER_U8_EQ(value, buffer, 0);
}

// Test that SetSubData offset works.
TEST_P(BufferSetSubDataTests, SmallDataAtOffset) {
    dawn::BufferDescriptor descriptor;
    descriptor.size = 4000;
    descriptor.usage = dawn::BufferUsageBit::TransferSrc | dawn::BufferUsageBit::TransferDst;
    dawn::Buffer buffer = device.CreateBuffer(&descriptor);

    constexpr uint32_t kOffset = 2000;
    uint8_t value = 231;
    buffer.SetSubData(kOffset, sizeof(value), &value);

    EXPECT_BUFFER_U8_EQ(value, buffer, kOffset);
}

// Stress test for many calls to SetSubData
TEST_P(BufferSetSubDataTests, ManySetSubData) {
    // Note: Increasing the size of the buffer will likely cause timeout issues.
    // In D3D12, timeout detection occurs when the GPU scheduler tries but cannot preempt the task
    // executing these commands in-flight. If this takes longer than ~2s, a device reset occurs and
    // fails the test. Since GPUs may or may not complete by then, this test must be disabled OR
    // modified to be well-below the timeout limit.
    constexpr uint32_t kSize = 4000 * 1000;
    constexpr uint32_t kElements = 500 * 500;
    dawn::BufferDescriptor descriptor;
    descriptor.size = kSize;
    descriptor.usage = dawn::BufferUsageBit::TransferSrc | dawn::BufferUsageBit::TransferDst;
    dawn::Buffer buffer = device.CreateBuffer(&descriptor);

    std::vector<uint32_t> expectedData;
    for (uint32_t i = 0; i < kElements; ++i) {
        buffer.SetSubData(i * sizeof(uint32_t), sizeof(i), reinterpret_cast<uint8_t*>(&i));
        expectedData.push_back(i);
    }

    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), buffer, 0, kElements);
}

// Test using SetSubData for lots of data
TEST_P(BufferSetSubDataTests, LargeSetSubData) {
    constexpr uint32_t kSize = 4000 * 1000;
    constexpr uint32_t kElements = 1000 * 1000;
    dawn::BufferDescriptor descriptor;
    descriptor.size = kSize;
    descriptor.usage = dawn::BufferUsageBit::TransferSrc | dawn::BufferUsageBit::TransferDst;
    dawn::Buffer buffer = device.CreateBuffer(&descriptor);

    std::vector<uint32_t> expectedData;
    for (uint32_t i = 0; i < kElements; ++i) {
        expectedData.push_back(i);
    }

    buffer.SetSubData(0, kElements * sizeof(uint32_t), reinterpret_cast<uint8_t*>(expectedData.data()));

    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), buffer, 0, kElements);
}

DAWN_INSTANTIATE_TEST(BufferSetSubDataTests,
                     D3D12Backend,
                     MetalBackend,
                     OpenGLBackend,
                     VulkanBackend)
