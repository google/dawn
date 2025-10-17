// Copyright 2025 The Dawn & Tint Authors
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
#include <ostream>
#include <sstream>
#include <string>

#include "dawn/native/WebGPUBackend.h"
#include "dawn/replay/Capture.h"
#include "dawn/replay/Replay.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class CaptureAndReplayTests : public DawnTest {
  public:
    class Capture {
      public:
        Capture(const std::string& commandData, const std::string& contentData)
            : mCommandData(commandData), mContentData(contentData) {}

        std::unique_ptr<replay::Replay> Replay(wgpu::Device device) {
            std::istringstream commandIStream(mCommandData);
            std::istringstream contentIStream(mContentData);

            std::unique_ptr<replay::Capture> capture = replay::Capture::Create(
                commandIStream, mCommandData.size(), contentIStream, mContentData.size());
            std::unique_ptr<replay::Replay> replay = replay::Replay::Create(device, capture.get());

            auto result = replay->Play();
            EXPECT_TRUE(result.IsSuccess());
            return replay;
        }

      private:
        std::string mCommandData;
        std::string mContentData;
    };

    class Recorder {
      public:
        static Recorder CreateAndStart(wgpu::Device device) { return Recorder(device); }

        Capture Finish() {
            native::webgpu::EndCapture(mDevice.Get());
            return Capture(mCommandStream.str(), mContentStream.str());
        }

      private:
        explicit Recorder(wgpu::Device device) : mDevice(device) {
            native::webgpu::StartCapture(device.Get(), mCommandStream, mContentStream);
        }

        wgpu::Device mDevice;
        std::ostringstream mCommandStream;
        std::ostringstream mContentStream;
    };
};

// During capture, makes a buffer, puts data in it.
// Then, replays and checks the data is correct.
TEST_P(CaptureAndReplayTests, Basic) {
    const char* label = "MyBuffer";
    const uint8_t myData[] = {0x11, 0x22, 0x33, 0x44};

    auto recorder = Recorder::CreateAndStart(device);

    {
        wgpu::BufferDescriptor descriptor;
        descriptor.label = label;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::CopyDst;
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

        queue.WriteBuffer(buffer, 0, &myData, sizeof(myData));
    }

    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Buffer buffer = replay->GetObjectByLabel<wgpu::Buffer>(label);
        ASSERT_NE(buffer, nullptr);

        EXPECT_BUFFER_U8_RANGE_EQ(myData, buffer, 0, sizeof(myData));
    }
}

// During capture, makes a buffer, puts data in it.
// Then, replays and checks the data is correct.
// It uses on label with 5 characters which may skew alignment.
TEST_P(CaptureAndReplayTests, NonMultipleOf4LabelLength) {
    const char* label = "MyBuf";
    const uint8_t myData[] = {0x11, 0x22, 0x33, 0x44};

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    {
        wgpu::BufferDescriptor descriptor;
        descriptor.label = label;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::CopyDst;
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

        queue.WriteBuffer(buffer, 0, &myData, sizeof(myData));
    }

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Buffer buffer = replay->GetObjectByLabel<wgpu::Buffer>(label);
        ASSERT_NE(buffer, nullptr);

        EXPECT_BUFFER_U8_RANGE_EQ(myData, buffer, 0, sizeof(myData));
    }
}

// Before capture, creates a buffer and sets half of it with WriteBuffer.
// It then starts a capture and writes the other half with WriteBuffer.
// On replay both halves should have the correct data..
TEST_P(CaptureAndReplayTests, StartCaptureAfterBufferCreationWriteBuffer) {
    const char* label = "MyBuffer";
    const uint8_t myData0[] = {0x11, 0x22, 0x33, 0x44};
    const uint8_t myData1[] = {0x55, 0x66, 0x77, 0x88};

    wgpu::BufferDescriptor descriptor;
    descriptor.label = label;
    descriptor.size = 8;
    descriptor.usage = wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    queue.WriteBuffer(buffer, 0, &myData0, sizeof(myData0));

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.WriteBuffer(buffer, 4, &myData1, sizeof(myData1));

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Buffer buffer = replay->GetObjectByLabel<wgpu::Buffer>(label);
        ASSERT_NE(buffer, nullptr);

        uint8_t expected[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
        EXPECT_BUFFER_U8_RANGE_EQ(expected, buffer, 0, sizeof(expected));
    }
}

// Before capture, creates a buffer and sets half of it with mappedAtCreation.
// It then starts a capture and writes the other half with WriteBuffer.
// On replay both halves should have the correct data..
TEST_P(CaptureAndReplayTests, StartCaptureAfterBufferCreationMappedAtCreation) {
    const char* label = "MyBuffer";
    const uint8_t myData0[] = {0x11, 0x22, 0x33, 0x44};
    const uint8_t myData1[] = {0x55, 0x66, 0x77, 0x88};

    wgpu::BufferDescriptor descriptor;
    descriptor.label = label;
    descriptor.size = 8;
    descriptor.usage = wgpu::BufferUsage::CopyDst;
    descriptor.mappedAtCreation = true;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    std::memcpy(buffer.GetMappedRange(), myData0, sizeof(myData0));
    buffer.Unmap();

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.WriteBuffer(buffer, 4, &myData1, sizeof(myData1));

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Buffer buffer = replay->GetObjectByLabel<wgpu::Buffer>(label);
        ASSERT_NE(buffer, nullptr);

        uint8_t expected[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
        EXPECT_BUFFER_U8_RANGE_EQ(expected, buffer, 0, sizeof(expected));
    }
}

// Before capture, creates a buffer and sets half of it with a compute shader.
// It then starts a capture and writes the other half with WriteBuffer.
// On replay both halves should have the correct data..
TEST_P(CaptureAndReplayTests, StartCaptureAfterBufferCreationComputeShader) {
    const char* label = "MyBuffer";

    wgpu::BufferDescriptor descriptor;
    descriptor.label = label;
    descriptor.size = 8;
    descriptor.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    const char* shader = R"(
        @group(0) @binding(0) var<storage, read_write> result : u32;

        @compute @workgroup_size(1) fn main() {
            result = 0x44332211;
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, buffer},
                                                     });

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }
    queue.Submit(1, &commands);

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    const uint8_t myData[] = {0x55, 0x66, 0x77, 0x88};
    queue.WriteBuffer(buffer, 4, &myData, sizeof(myData));

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Buffer buffer = replay->GetObjectByLabel<wgpu::Buffer>(label);
        ASSERT_NE(buffer, nullptr);

        uint8_t expected[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
        EXPECT_BUFFER_U8_RANGE_EQ(expected, buffer, 0, sizeof(expected));
    }
}

// Before capture, creates a buffer and sets half of it with copyBufferToBuffer.
// It then starts a capture and writes the other half with WriteBuffer.
// On replay both halves should have the correct data..
TEST_P(CaptureAndReplayTests, StartCaptureAfterBufferCreationCopyB2B) {
    const char* srcLabel = "SrcBuffer";
    const char* dstLabel = "DstBuffer";

    wgpu::BufferDescriptor descriptor;
    descriptor.label = dstLabel;
    descriptor.size = 8;
    descriptor.usage = wgpu::BufferUsage::CopyDst;
    wgpu::Buffer dstBuffer = device.CreateBuffer(&descriptor);

    descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    descriptor.label = srcLabel;
    wgpu::Buffer srcBuffer = device.CreateBuffer(&descriptor);
    const uint8_t myData1[] = {0x11, 0x22, 0x33, 0x44};
    queue.WriteBuffer(srcBuffer, 0, &myData1, sizeof(myData1));

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(srcBuffer, 0, dstBuffer, 0, 4);
        commands = encoder.Finish();
    }
    queue.Submit(1, &commands);

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    const uint8_t myData2[] = {0x55, 0x66, 0x77, 0x88};
    queue.WriteBuffer(dstBuffer, 4, &myData2, sizeof(myData2));

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Buffer buffer = replay->GetObjectByLabel<wgpu::Buffer>(dstLabel);
        ASSERT_NE(buffer, nullptr);

        uint8_t expected[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
        EXPECT_BUFFER_U8_RANGE_EQ(expected, buffer, 0, sizeof(expected));
    }
}

// During first capture, makes a buffer, puts data in it.
// During 2nd capture, puts a little data in the same buffer.
// Then, replays the 2nd capture and checks the data is correct.
TEST_P(CaptureAndReplayTests, TwoCaptures) {
    const char* label = "MyBuffer";
    const uint8_t myData1[] = {0x11, 0x22, 0x33, 0x44};
    const uint8_t myData2[] = {0x55, 0x66, 0x77, 0x88};

    wgpu::Buffer buffer;

    {
        auto recorder = Recorder::CreateAndStart(device);

        wgpu::BufferDescriptor descriptor;
        descriptor.label = label;
        descriptor.size = 8;
        descriptor.usage = wgpu::BufferUsage::CopyDst;
        buffer = device.CreateBuffer(&descriptor);

        queue.WriteBuffer(buffer, 0, &myData1, sizeof(myData1));

        recorder.Finish();
    }

    {
        auto recorder = Recorder::CreateAndStart(device);

        queue.WriteBuffer(buffer, 4, &myData2, sizeof(myData2));

        auto capture = recorder.Finish();
        auto replay = capture.Replay(device);

        {
            wgpu::Buffer buffer = replay->GetObjectByLabel<wgpu::Buffer>(label);
            ASSERT_NE(buffer, nullptr);

            uint8_t expected[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
            EXPECT_BUFFER_U8_RANGE_EQ(expected, buffer, 0, sizeof(expected));
        }
    }
}

// We make a buffer before capture. During capture write map it, put data in it.
// Then check the data is correct on replay.
TEST_P(CaptureAndReplayTests, MapWrite) {
    const char* label = "myBuffer";
    const uint8_t myData[] = {0x11, 0x22, 0x33, 0x44};

    wgpu::BufferDescriptor descriptor;
    descriptor.label = label;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    auto recorder = Recorder::CreateAndStart(device);

    MapAsyncAndWait(buffer, wgpu::MapMode::Write, 0, 4);
    buffer.WriteMappedRange(0, &myData, sizeof(myData));
    buffer.Unmap();

    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Buffer buffer = replay->GetObjectByLabel<wgpu::Buffer>(label);
        ASSERT_NE(buffer, nullptr);

        EXPECT_BUFFER_U8_RANGE_EQ(myData, buffer, 0, sizeof(myData));
    }
}

// We make 2 buffers before capture. During capture we map one buffer
// put some data it in via map/unmap. We then copy from that buffer to the other buffer.
// On replay check the data is correct.
TEST_P(CaptureAndReplayTests, CaptureWithMapWriteDuringCapture) {
    const char* srcLabel = "srcBuffer";
    const char* dstLabel = "dstBuffer";
    const uint8_t myData1[] = {0x11, 0x22, 0x33, 0x44};
    const uint8_t myData2[] = {0x55, 0x66, 0x77, 0x88};

    wgpu::BufferDescriptor descriptor;
    descriptor.label = dstLabel;
    descriptor.size = 8;
    descriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer dstBuffer = device.CreateBuffer(&descriptor);
    queue.WriteBuffer(dstBuffer, 0, &myData1, sizeof(myData1));

    descriptor.label = srcLabel;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer srcBuffer = device.CreateBuffer(&descriptor);

    auto recorder = Recorder::CreateAndStart(device);

    MapAsyncAndWait(srcBuffer, wgpu::MapMode::Write, 0, 4);
    srcBuffer.WriteMappedRange(0, &myData2, sizeof(myData2));
    srcBuffer.Unmap();

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(srcBuffer, 0, dstBuffer, 4, 4);
        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Buffer buffer = replay->GetObjectByLabel<wgpu::Buffer>(dstLabel);
        ASSERT_NE(buffer, nullptr);

        uint8_t expected[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
        EXPECT_BUFFER_U8_RANGE_EQ(expected, buffer, 0, sizeof(expected));
    }
}

TEST_P(CaptureAndReplayTests, CaptureCopyBufferToBuffer) {
    const uint8_t myData[] = {0x11, 0x22, 0x33, 0x44};

    wgpu::BufferDescriptor descriptor;
    descriptor.label = "srcBuffer";
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer srcBuffer = device.CreateBuffer(&descriptor);

    queue.WriteBuffer(srcBuffer, 0, &myData, sizeof(myData));

    descriptor.label = "dstBuffer";
    descriptor.usage = wgpu::BufferUsage::CopyDst;
    wgpu::Buffer dstBuffer = device.CreateBuffer(&descriptor);

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(srcBuffer, 0, dstBuffer, 0, 4);
        commands = encoder.Finish();
    }

    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Buffer buffer = replay->GetObjectByLabel<wgpu::Buffer>("dstBuffer");
        ASSERT_NE(buffer, nullptr);

        EXPECT_BUFFER_U8_RANGE_EQ(myData, buffer, 0, sizeof(myData));
    }
}

DAWN_INSTANTIATE_TEST(CaptureAndReplayTests, WebGPUBackend());

}  // anonymous namespace
}  // namespace dawn
