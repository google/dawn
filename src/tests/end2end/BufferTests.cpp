// Copyright 2017 The NXT Authors
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

#include "tests/NXTTest.h"

#include <cstring>

class BufferMapReadTests : public NXTTest {
    protected:

        static void MapReadCallback(nxtBufferMapReadStatus status, const void* data, nxtCallbackUserdata userdata) {
            ASSERT_EQ(NXT_BUFFER_MAP_READ_STATUS_SUCCESS, status);
            ASSERT_NE(nullptr, data);

            auto test = reinterpret_cast<BufferMapReadTests*>(static_cast<uintptr_t>(userdata));
            test->mappedData = data;
        }

        const void* MapReadAsyncAndWait(const nxt::Buffer& buffer, uint32_t start, uint32_t offset) {
            buffer.MapReadAsync(start, offset, MapReadCallback, static_cast<nxt::CallbackUserdata>(reinterpret_cast<uintptr_t>(this)));

            while (mappedData == nullptr) {
                WaitABit();
            }

            return mappedData;
        }

    private:
        const void* mappedData = nullptr;
};

TEST_P(BufferMapReadTests, SmallReadAtZero) {
    nxt::Buffer buffer = device.CreateBufferBuilder()
        .SetSize(4)
        .SetAllowedUsage(nxt::BufferUsageBit::MapRead | nxt::BufferUsageBit::TransferDst)
        .SetInitialUsage(nxt::BufferUsageBit::TransferDst)
        .GetResult();

    uint32_t myData = 2934875;
    buffer.SetSubData(0, 1, &myData);

    buffer.TransitionUsage(nxt::BufferUsageBit::MapRead);
    const void* mappedData = MapReadAsyncAndWait(buffer, 0, 4);
    ASSERT_EQ(myData, *reinterpret_cast<const uint32_t*>(mappedData));

    buffer.Unmap();
}

TEST_P(BufferMapReadTests, SmallReadAtOffset) {
    nxt::Buffer buffer = device.CreateBufferBuilder()
        .SetSize(4000)
        .SetAllowedUsage(nxt::BufferUsageBit::MapRead | nxt::BufferUsageBit::TransferDst)
        .SetInitialUsage(nxt::BufferUsageBit::TransferDst)
        .GetResult();

    uint32_t myData = 2934875;
    buffer.SetSubData(2048 / sizeof(uint32_t), 1, &myData);

    buffer.TransitionUsage(nxt::BufferUsageBit::MapRead);
    const void* mappedData = MapReadAsyncAndWait(buffer, 2048, 4);
    ASSERT_EQ(myData, *reinterpret_cast<const uint32_t*>(mappedData));

    buffer.Unmap();
}

TEST_P(BufferMapReadTests, LargeRead) {
    constexpr uint32_t kDataSize = 1000 * 1000;
    std::vector<uint32_t> myData;
    for (uint32_t i = 0; i < kDataSize; ++i) {
        myData.push_back(i);
    }

    nxt::Buffer buffer = device.CreateBufferBuilder()
        .SetSize(static_cast<uint32_t>(kDataSize * sizeof(uint32_t)))
        .SetAllowedUsage(nxt::BufferUsageBit::MapRead | nxt::BufferUsageBit::TransferDst)
        .SetInitialUsage(nxt::BufferUsageBit::TransferDst)
        .GetResult();

    buffer.SetSubData(0, static_cast<uint32_t>(kDataSize), myData.data());

    buffer.TransitionUsage(nxt::BufferUsageBit::MapRead);
    const void* mappedData = MapReadAsyncAndWait(buffer, 0, static_cast<uint32_t>(kDataSize * sizeof(uint32_t)));
    ASSERT_EQ(0, memcmp(mappedData, myData.data(), kDataSize * sizeof(uint32_t)));

    buffer.Unmap();
}

NXT_INSTANTIATE_TEST(BufferMapReadTests, D3D12Backend, MetalBackend, OpenGLBackend)
