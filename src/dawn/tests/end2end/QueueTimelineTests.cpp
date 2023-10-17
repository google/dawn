// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <memory>

#include "dawn/tests/DawnTest.h"
#include "gmock/gmock.h"

namespace dawn {
namespace {

using testing::InSequence;

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

static std::unique_ptr<MockQueueWorkDoneCallback> mockQueueWorkDoneCallback1;
static void ToMockQueueWorkDone1(WGPUQueueWorkDoneStatus status, void* userdata) {
    mockQueueWorkDoneCallback1->Call(status, userdata);
}

class QueueTimelineTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        mockMapCallback = std::make_unique<MockMapCallback>();
        mockQueueWorkDoneCallback = std::make_unique<MockQueueWorkDoneCallback>();
        mockQueueWorkDoneCallback1 = std::make_unique<MockQueueWorkDoneCallback>();

        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::MapRead;
        mMapReadBuffer = device.CreateBuffer(&descriptor);
    }

    void TearDown() override {
        mockMapCallback = nullptr;
        mockQueueWorkDoneCallback = nullptr;
        mockQueueWorkDoneCallback1 = nullptr;
        DawnTest::TearDown();
    }

    wgpu::Buffer mMapReadBuffer;
};

// Test that mMapReadBuffer.MapAsync callback happens before queue.OnWorkDone callback
// when queue.OnSubmittedWorkDone is called after mMapReadBuffer.MapAsync. The callback order should
// happen in the order the functions are called.
TEST_P(QueueTimelineTests, MapRead_OnWorkDone) {
    InSequence sequence;
    EXPECT_CALL(*mockMapCallback, Call(WGPUBufferMapAsyncStatus_Success, this)).Times(1);
    EXPECT_CALL(*mockQueueWorkDoneCallback, Call(WGPUQueueWorkDoneStatus_Success, this)).Times(1);

    mMapReadBuffer.MapAsync(wgpu::MapMode::Read, 0, wgpu::kWholeMapSize, ToMockMapCallback, this);

    queue.OnSubmittedWorkDone(ToMockQueueWorkDone, this);

    WaitForAllOperations();
    mMapReadBuffer.Unmap();
}

// Test that the OnSubmittedWorkDone callbacks should happen in the order the functions are called.
TEST_P(QueueTimelineTests, OnWorkDone_OnWorkDone) {
    InSequence sequence;
    EXPECT_CALL(*mockQueueWorkDoneCallback, Call(WGPUQueueWorkDoneStatus_Success, this)).Times(1);
    EXPECT_CALL(*mockQueueWorkDoneCallback1, Call(WGPUQueueWorkDoneStatus_Success, this)).Times(1);

    queue.OnSubmittedWorkDone(ToMockQueueWorkDone, this);
    queue.OnSubmittedWorkDone(ToMockQueueWorkDone1, this);

    WaitForAllOperations();
}

DAWN_INSTANTIATE_TEST(QueueTimelineTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
