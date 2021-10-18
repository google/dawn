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

#include <gmock/gmock.h>
#include "tests/DawnTest.h"

using namespace testing;

class MockMapCallback {
  public:
    MOCK_METHOD(void, Call, (WGPUBufferMapAsyncStatus status, void* userdata));
};

static std::unique_ptr<MockMapCallback> mockMapCallback;
static void ToMockMapCallback(WGPUBufferMapAsyncStatus status, void* userdata) {
    EXPECT_EQ(status, WGPUBufferMapAsyncStatus_Success);
    mockMapCallback->Call(status, userdata);
}

class MockQueueWorkDoneCallback {
  public:
    MOCK_METHOD(void, Call, (WGPUQueueWorkDoneStatus status, void* userdata));
};

static std::unique_ptr<MockQueueWorkDoneCallback> mockQueueWorkDoneCallback;
static void ToMockQueueWorkDone(WGPUQueueWorkDoneStatus status, void* userdata) {
    mockQueueWorkDoneCallback->Call(status, userdata);
}

class QueueTimelineTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        mockMapCallback = std::make_unique<MockMapCallback>();
        mockQueueWorkDoneCallback = std::make_unique<MockQueueWorkDoneCallback>();

        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::MapRead;
        mMapReadBuffer = device.CreateBuffer(&descriptor);
    }

    void TearDown() override {
        mockMapCallback = nullptr;
        mockQueueWorkDoneCallback = nullptr;
        DawnTest::TearDown();
    }

    wgpu::Buffer mMapReadBuffer;
};

// Test that mMapReadBuffer.MapAsync callback happens before queue.OnWorkDone callback
// when queue.OnSubmittedWorkDone is called after mMapReadBuffer.MapAsync. The callback order should
// happen in the order the functions are called.
TEST_P(QueueTimelineTests, MapRead_OnWorkDone) {
    testing::InSequence sequence;
    EXPECT_CALL(*mockMapCallback, Call(WGPUBufferMapAsyncStatus_Success, this)).Times(1);
    EXPECT_CALL(*mockQueueWorkDoneCallback, Call(WGPUQueueWorkDoneStatus_Success, this)).Times(1);

    mMapReadBuffer.MapAsync(wgpu::MapMode::Read, 0, wgpu::kWholeMapSize, ToMockMapCallback, this);

    queue.OnSubmittedWorkDone(0u, ToMockQueueWorkDone, this);

    WaitForAllOperations();
    mMapReadBuffer.Unmap();
}

// Test that queue.OnWorkDone callback happens before mMapReadBuffer.MapAsync callback when
// queue.Signal is called before mMapReadBuffer.MapAsync. The callback order should
// happen in the order the functions are called.
TEST_P(QueueTimelineTests, OnWorkDone_MapRead) {
    testing::InSequence sequence;
    EXPECT_CALL(*mockQueueWorkDoneCallback, Call(WGPUQueueWorkDoneStatus_Success, this)).Times(1);
    EXPECT_CALL(*mockMapCallback, Call(WGPUBufferMapAsyncStatus_Success, this)).Times(1);

    queue.OnSubmittedWorkDone(0u, ToMockQueueWorkDone, this);

    mMapReadBuffer.MapAsync(wgpu::MapMode::Read, 0, wgpu::kWholeMapSize, ToMockMapCallback, this);

    WaitForAllOperations();
    mMapReadBuffer.Unmap();
}

DAWN_INSTANTIATE_TEST(QueueTimelineTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
