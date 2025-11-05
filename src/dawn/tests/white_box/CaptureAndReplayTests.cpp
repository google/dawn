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
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class CaptureAndReplayTests : public DawnTest {
  public:
    void SetUp() override {
        DawnTest::SetUp();

        // TODO(crbug.com/452924800): Remove once these tests work properly with
        // the WebGPU on WebGPU backend with wire.
        DAWN_SUPPRESS_TEST_IF(IsWebGPUOn(wgpu::BackendType::Metal) && UsesWire());
        DAWN_SUPPRESS_TEST_IF(IsWebGPUOn(wgpu::BackendType::Vulkan) && UsesWire());
    }

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

TEST_P(CaptureAndReplayTests, WriteTexture) {
    const uint8_t myData[] = {0x11, 0x22, 0x33, 0x44};

    wgpu::TextureDescriptor textureDesc;
    textureDesc.label = "myTexture";
    textureDesc.size = {4, 1, 1};
    textureDesc.format = wgpu::TextureFormat::R8Unorm;
    textureDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
    wgpu::Texture texture = device.CreateTexture(&textureDesc);

    auto recorder = Recorder::CreateAndStart(device);

    {
        wgpu::TexelCopyBufferLayout texelCopyBufferLayout =
            utils::CreateTexelCopyBufferLayout(0, 4);
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(texture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
        wgpu::Extent3D extent = {4, 1, 1};
        queue.WriteTexture(&texelCopyTextureInfo, myData, sizeof(myData), &texelCopyBufferLayout,
                           &extent);
    }

    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Texture texture = replay->GetObjectByLabel<wgpu::Texture>("myTexture");
        ASSERT_NE(texture, nullptr);
        EXPECT_TEXTURE_EQ(&myData[0], texture, {0, 0}, {4, 1}, 0, wgpu::TextureAspect::All);
    }
}

TEST_P(CaptureAndReplayTests, CaptureCopyBufferToTexture) {
    const uint8_t myData[] = {0x11, 0x22, 0x33, 0x44};

    wgpu::BufferDescriptor descriptor;
    descriptor.label = "srcBuffer";
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer srcBuffer = device.CreateBuffer(&descriptor);

    queue.WriteBuffer(srcBuffer, 0, &myData, sizeof(myData));

    wgpu::TextureDescriptor textureDesc;
    textureDesc.label = "dstTexture";
    textureDesc.size = {4, 1, 1};
    textureDesc.format = wgpu::TextureFormat::R8Unorm;
    textureDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
    wgpu::Texture dstTexture = device.CreateTexture(&textureDesc);

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
            utils::CreateTexelCopyBufferInfo(srcBuffer, 0, 256, 1);
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
        wgpu::Extent3D extent = {4, 1, 1};

        encoder.CopyBufferToTexture(&texelCopyBufferInfo, &texelCopyTextureInfo, &extent);
        commands = encoder.Finish();
    }

    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Texture texture = replay->GetObjectByLabel<wgpu::Texture>("dstTexture");
        ASSERT_NE(texture, nullptr);
        EXPECT_TEXTURE_EQ(&myData[0], texture, {0, 0}, {4, 1}, 0, wgpu::TextureAspect::All);
    }
}

TEST_P(CaptureAndReplayTests, CaptureCopyTextureToBuffer) {
    const uint8_t myData[] = {0x11, 0x22, 0x33, 0x44};

    wgpu::TextureDescriptor textureDesc;
    textureDesc.label = "srcTexture";
    textureDesc.size = {4, 1, 1};
    textureDesc.format = wgpu::TextureFormat::R8Unorm;
    textureDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
    wgpu::Texture srcTexture = device.CreateTexture(&textureDesc);

    // Put data in source texture
    {
        wgpu::TexelCopyBufferLayout texelCopyBufferLayout =
            utils::CreateTexelCopyBufferLayout(0, 4);
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
        wgpu::Extent3D extent = {4, 1, 1};
        queue.WriteTexture(&texelCopyTextureInfo, myData, sizeof(myData), &texelCopyBufferLayout,
                           &extent);
    }

    wgpu::BufferDescriptor descriptor;
    descriptor.label = "dstBuffer";
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer dstBuffer = device.CreateBuffer(&descriptor);

    wgpu::CommandBuffer commands;
    {
        // Copy srcTexture to dstBuffer
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
        wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
            utils::CreateTexelCopyBufferInfo(dstBuffer, 0, 256, 1);
        wgpu::Extent3D extent = {4, 1, 1};

        encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &texelCopyBufferInfo, &extent);
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

TEST_P(CaptureAndReplayTests, CaptureCopyTextureToTexture) {
    const uint8_t myData[] = {0x11, 0x22, 0x33, 0x44};

    wgpu::TextureDescriptor textureDesc;
    textureDesc.label = "srcTexture";
    textureDesc.size = {4, 1, 1};
    textureDesc.format = wgpu::TextureFormat::R8Unorm;
    textureDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
    wgpu::Texture srcTexture = device.CreateTexture(&textureDesc);

    // Put data in source texture
    {
        wgpu::TexelCopyBufferLayout texelCopyBufferLayout =
            utils::CreateTexelCopyBufferLayout(0, 4);
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
        wgpu::Extent3D extent = {4, 1, 1};
        queue.WriteTexture(&texelCopyTextureInfo, myData, sizeof(myData), &texelCopyBufferLayout,
                           &extent);
    }

    textureDesc.label = "dstTexture";
    wgpu::Texture dstTexture = device.CreateTexture(&textureDesc);

    wgpu::CommandBuffer commands;
    {
        // Copy srcTexture to dstBuffer
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::TexelCopyTextureInfo srcTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
        wgpu::TexelCopyTextureInfo dstTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
        wgpu::Extent3D extent = {4, 1, 1};

        encoder.CopyTextureToTexture(&srcTexelCopyTextureInfo, &dstTexelCopyTextureInfo, &extent);
        commands = encoder.Finish();
    }

    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Texture texture = replay->GetObjectByLabel<wgpu::Texture>("dstTexture");
        ASSERT_NE(texture, nullptr);
        EXPECT_TEXTURE_EQ(&myData[0], texture, {0, 0}, {4, 1}, 0, wgpu::TextureAspect::All);
    }
}

// We make 3 textures. Put data in the first one. Copy to the 2nd one.
// Copy tha the 3rd. Check the results. The reason for this test is that
// the first texture is marked as initialized by WriteTexture. The 2nd is
// not. So, if the texture is not marked as initialized by CopyT2T then
// capture will fail as it will not copy the contents of the 2nd texture.
TEST_P(CaptureAndReplayTests, CaptureCopyTextureToTextureFromCopyT2TTexture) {
    const uint8_t myData[] = {0x11, 0x22, 0x33, 0x44};

    wgpu::TextureDescriptor textureDesc;
    textureDesc.label = "dataTexture";
    textureDesc.size = {4, 1, 1};
    textureDesc.format = wgpu::TextureFormat::R8Unorm;
    textureDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
    wgpu::Texture dataTexture = device.CreateTexture(&textureDesc);

    // Put data in data texture
    {
        wgpu::TexelCopyBufferLayout texelCopyBufferLayout =
            utils::CreateTexelCopyBufferLayout(0, 4);
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(dataTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
        wgpu::Extent3D extent = {4, 1, 1};
        queue.WriteTexture(&texelCopyTextureInfo, myData, sizeof(myData), &texelCopyBufferLayout,
                           &extent);
    }

    textureDesc.label = "srcTexture";
    wgpu::Texture srcTexture = device.CreateTexture(&textureDesc);

    // Copy the data texture ot the src texture
    {
        wgpu::CommandBuffer commands;
        {
            // Copy srcTexture to dstBuffer
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::TexelCopyTextureInfo srcTexelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
                dataTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
            wgpu::TexelCopyTextureInfo dstTexelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
                srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
            wgpu::Extent3D extent = {4, 1, 1};

            encoder.CopyTextureToTexture(&srcTexelCopyTextureInfo, &dstTexelCopyTextureInfo,
                                         &extent);
            commands = encoder.Finish();
        }
        queue.Submit(1, &commands);
    }

    textureDesc.label = "dstTexture";
    wgpu::Texture dstTexture = device.CreateTexture(&textureDesc);

    wgpu::CommandBuffer commands;
    {
        // Copy srcTexture to dstBuffer
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::TexelCopyTextureInfo srcTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
        wgpu::TexelCopyTextureInfo dstTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
        wgpu::Extent3D extent = {4, 1, 1};

        encoder.CopyTextureToTexture(&srcTexelCopyTextureInfo, &dstTexelCopyTextureInfo, &extent);
        commands = encoder.Finish();
    }

    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Texture texture = replay->GetObjectByLabel<wgpu::Texture>("dstTexture");
        ASSERT_NE(texture, nullptr);
        EXPECT_TEXTURE_EQ(&myData[0], texture, {0, 0}, {4, 1}, 0, wgpu::TextureAspect::All);
    }
}

// Before capture, creates a texture and sets it in a compute pass as a storage texture.
// Then, captures a copyT2T to a 2nd texture. Checks the 2nd texture has the correct data on replay.
TEST_P(CaptureAndReplayTests, CaptureCopyTextureToTextureFromComputeTexture) {
    wgpu::TextureDescriptor textureDesc;
    textureDesc.label = "srcTexture";
    textureDesc.size = {1, 1, 1};
    textureDesc.format = wgpu::TextureFormat::RGBA8Uint;
    textureDesc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::StorageBinding;
    wgpu::Texture srcTexture = device.CreateTexture(&textureDesc);

    const char* shader = R"(
        @group(0) @binding(0) var tex: texture_storage_2d<rgba8uint, write>;

        @compute @workgroup_size(1) fn main() {
            textureStore(tex, vec2u(0), vec4<u32>(0x11, 0x22, 0x33, 0x44));
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, srcTexture.CreateView()},
                                                     });

    {
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
    }

    textureDesc.label = "dstTexture";
    textureDesc.usage = wgpu::TextureUsage::CopyDst;
    wgpu::Texture dstTexture = device.CreateTexture(&textureDesc);

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    {
        wgpu::CommandBuffer commands;
        {
            // Copy srcTexture to dstBuffer
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::TexelCopyTextureInfo srcTexelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
                srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
            wgpu::TexelCopyTextureInfo dstTexelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
                dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
            wgpu::Extent3D extent = {1, 1, 1};

            encoder.CopyTextureToTexture(&srcTexelCopyTextureInfo, &dstTexelCopyTextureInfo,
                                         &extent);
            commands = encoder.Finish();
        }
        queue.Submit(1, &commands);
    }

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Texture texture = replay->GetObjectByLabel<wgpu::Texture>("dstTexture");
        ASSERT_NE(texture, nullptr);

        uint8_t expected[] = {0x11, 0x22, 0x33, 0x44};
        EXPECT_TEXTURE_EQ(&expected[0], texture, {0, 0}, {1, 1}, 0, wgpu::TextureAspect::All);
    }
}

// Before capture, creates a texture and sets in a render pass as a render attachment
// Then, captures a copyT2T to a 2nd texture. Checks the 2nd texture has the correct data on replay.
TEST_P(CaptureAndReplayTests, CaptureCopyTextureToTextureFromRenderTexture) {
    wgpu::TextureDescriptor textureDesc;
    textureDesc.label = "srcTexture";
    textureDesc.size = {1, 1, 1};
    textureDesc.format = wgpu::TextureFormat::RGBA8Uint;
    textureDesc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture srcTexture = device.CreateTexture(&textureDesc);

    {
        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

            utils::ComboRenderPassDescriptor passDescriptor({srcTexture.CreateView()});
            passDescriptor.cColorAttachments[0].clearValue = {0x11, 0x22, 0x33, 0x44};
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
            pass.End();

            commands = encoder.Finish();
        }
        queue.Submit(1, &commands);
    }

    textureDesc.label = "dstTexture";
    textureDesc.usage = wgpu::TextureUsage::CopyDst;
    wgpu::Texture dstTexture = device.CreateTexture(&textureDesc);

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    {
        wgpu::CommandBuffer commands;
        {
            // Copy srcTexture to dstBuffer
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::TexelCopyTextureInfo srcTexelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
                srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
            wgpu::TexelCopyTextureInfo dstTexelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
                dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
            wgpu::Extent3D extent = {1, 1, 1};

            encoder.CopyTextureToTexture(&srcTexelCopyTextureInfo, &dstTexelCopyTextureInfo,
                                         &extent);
            commands = encoder.Finish();
        }
        queue.Submit(1, &commands);
    }

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Texture texture = replay->GetObjectByLabel<wgpu::Texture>("dstTexture");
        ASSERT_NE(texture, nullptr);

        uint8_t expected[] = {0x11, 0x22, 0x33, 0x44};
        EXPECT_TEXTURE_EQ(&expected[0], texture, {0, 0}, {1, 1}, 0, wgpu::TextureAspect::All);
    }
}

// Capture and replay the simplest compute shader.
TEST_P(CaptureAndReplayTests, CaptureComputeShaderBasic) {
    const char* label = "MyBuffer";

    wgpu::BufferDescriptor descriptor;
    descriptor.label = label;
    descriptor.size = 4;
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

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Buffer buffer = replay->GetObjectByLabel<wgpu::Buffer>(label);
        ASSERT_NE(buffer, nullptr);

        uint8_t expected[] = {0x11, 0x22, 0x33, 0x44};
        EXPECT_BUFFER_U8_RANGE_EQ(expected, buffer, 0, sizeof(expected));
    }
}

// Capture and replay the simplest compute shader but set the bindGroup
// before setting the pipeline.
TEST_P(CaptureAndReplayTests, CaptureComputeShaderBasicSetBindGroupFirst) {
    const char* label = "MyBuffer";

    wgpu::BufferDescriptor descriptor;
    descriptor.label = label;
    descriptor.size = 4;
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
        pass.SetBindGroup(0, bindGroup);
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Buffer buffer = replay->GetObjectByLabel<wgpu::Buffer>(label);
        ASSERT_NE(buffer, nullptr);

        uint8_t expected[] = {0x11, 0x22, 0x33, 0x44};
        EXPECT_BUFFER_U8_RANGE_EQ(expected, buffer, 0, sizeof(expected));
    }
}

// Capture and replay 2 auto-layout compute pipelines with the same layout
// This is to verify a bug fix. In Dawn, there is BindGroupLayout and
// BindGroupLayoutInternal. In this test, given 2 pipelines with the same
// layout, there will be 2 BindGroupLayout objects pointing to one
// BindGroupLayoutInternal. That means that when serializing, one of them
// will get the wrong Pipeline if the pipeline is incorrectly associated
// with the one BindGroupLayoutInternal instead of each of the 2 pipelines
// being separately associated with one of the 2 BindGroupLayout objects.
// This is a regression test for crbug.com/455605671
TEST_P(CaptureAndReplayTests, CaptureTwoMatchingAutoLayoutComputePipelines) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
    descriptor.label = "buffer1";
    wgpu::Buffer buffer1 = device.CreateBuffer(&descriptor);
    descriptor.label = "buffer2";
    wgpu::Buffer buffer2 = device.CreateBuffer(&descriptor);

    const char* shader = R"(
        @group(0) @binding(0) var<storage, read_write> result : u32;

        @compute @workgroup_size(1) fn cs1() {
            result = 0x44332211;
        }

        @compute @workgroup_size(1) fn cs2() {
            result = 0x88776655;
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.label = "pipeline1";
    csDesc.compute.module = module;
    csDesc.compute.entryPoint = "cs1";
    wgpu::ComputePipeline pipeline1 = device.CreateComputePipeline(&csDesc);
    csDesc.label = "pipeline2";
    csDesc.compute.entryPoint = "cs2";
    wgpu::ComputePipeline pipeline2 = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup1 = utils::MakeBindGroup(device, pipeline1.GetBindGroupLayout(0),
                                                      {
                                                          {0, buffer1},
                                                      });
    wgpu::BindGroup bindGroup2 = utils::MakeBindGroup(device, pipeline2.GetBindGroupLayout(0),
                                                      {
                                                          {0, buffer2},
                                                      });

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline1);
        pass.SetBindGroup(0, bindGroup1);
        pass.DispatchWorkgroups(1);
        pass.SetPipeline(pipeline2);
        pass.SetBindGroup(0, bindGroup2);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Buffer buffer = replay->GetObjectByLabel<wgpu::Buffer>("buffer1");
        ASSERT_NE(buffer, nullptr);

        uint8_t expected[] = {0x11, 0x22, 0x33, 0x44};
        EXPECT_BUFFER_U8_RANGE_EQ(expected, buffer, 0, sizeof(expected));
    }

    {
        wgpu::Buffer buffer = replay->GetObjectByLabel<wgpu::Buffer>("buffer2");
        ASSERT_NE(buffer, nullptr);

        uint8_t expected[] = {0x55, 0x66, 0x77, 0x88};
        EXPECT_BUFFER_U8_RANGE_EQ(expected, buffer, 0, sizeof(expected));
    }
}

// Capture and replay 2 bindGroups that use implicit bindGroupLayouts from
// different pipelines but for 1, never set the pipeline nor dispatch. This effectively
// makes it a no-op. The issue is, we can't easily serialize a bindGroup that uses an
// implicit bindGroupLayout unless the pipeline that created that bindGroupLayout is
// used in the command buffer. So, we just don't serialize those calls to setBindGroup
// since they are effectively no-ops. This test checks things don't crash as if the
// call was actually serialized it would reference a bindGroupLayout that does not
// exist.
TEST_P(CaptureAndReplayTests, CaptureTwoAutoLayoutComputePipelinesOneIsBoundButUnused) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
    descriptor.label = "buffer";
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    const char* shader = R"(
        @group(0) @binding(0) var<storage, read_write> result : u32;

        @compute @workgroup_size(1) fn cs1() {
            result = 0x44332211;
        }

        @compute @workgroup_size(1) fn cs2() {
            result = 0x88776655;
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.label = "pipeline1";
    csDesc.compute.module = module;
    csDesc.compute.entryPoint = "cs1";
    wgpu::ComputePipeline pipeline1 = device.CreateComputePipeline(&csDesc);
    csDesc.label = "pipeline2";
    csDesc.compute.entryPoint = "cs2";
    wgpu::ComputePipeline pipeline2 = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup1 = utils::MakeBindGroup(device, pipeline1.GetBindGroupLayout(0),
                                                      {
                                                          {0, buffer},
                                                      });
    wgpu::BindGroup bindGroup2 = utils::MakeBindGroup(device, pipeline2.GetBindGroupLayout(0),
                                                      {
                                                          {0, buffer},
                                                      });

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetBindGroup(0, bindGroup1);
        pass.SetPipeline(pipeline2);
        pass.SetBindGroup(0, bindGroup2);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Buffer buffer = replay->GetObjectByLabel<wgpu::Buffer>("buffer");
        ASSERT_NE(buffer, nullptr);

        uint8_t expected[] = {0x55, 0x66, 0x77, 0x88};
        EXPECT_BUFFER_U8_RANGE_EQ(expected, buffer, 0, sizeof(expected));
    }
}

// Capture and replay the simplest render pass.
// It just starts and ends a render pass and uses the clearValue to set
// a texture.
TEST_P(CaptureAndReplayTests, CaptureRenderPassBasic) {
    wgpu::TextureDescriptor textureDesc;
    textureDesc.label = "myTexture";
    textureDesc.size = {1, 1, 1};
    textureDesc.format = wgpu::TextureFormat::RGBA8Uint;
    textureDesc.usage = wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture texture = device.CreateTexture(&textureDesc);

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        utils::ComboRenderPassDescriptor passDescriptor({texture.CreateView()});
        passDescriptor.cColorAttachments[0].clearValue = {0x11, 0x22, 0x33, 0x44};
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Texture texture = replay->GetObjectByLabel<wgpu::Texture>("myTexture");
        ASSERT_NE(texture, nullptr);

        uint8_t expected[] = {0x11, 0x22, 0x33, 0x44};
        EXPECT_TEXTURE_EQ(&expected[0], texture, {0, 0}, {1, 1}, 0, wgpu::TextureAspect::All);
    }
}

// Capture and replay the a render pass where a texture is rendered into another.
TEST_P(CaptureAndReplayTests, CaptureRenderPassBasicWithBindGroup) {
    const uint8_t myData[] = {0x11, 0x22, 0x33, 0x44};

    wgpu::TextureDescriptor textureDesc;
    textureDesc.label = "srcTexture";
    textureDesc.size = {1, 1, 1};
    textureDesc.format = wgpu::TextureFormat::RGBA8Uint;
    textureDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
    wgpu::Texture srcTexture = device.CreateTexture(&textureDesc);

    {
        wgpu::TexelCopyBufferLayout texelCopyBufferLayout =
            utils::CreateTexelCopyBufferLayout(0, 4);
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
        wgpu::Extent3D extent = {1, 1, 1};
        queue.WriteTexture(&texelCopyTextureInfo, myData, sizeof(myData), &texelCopyBufferLayout,
                           &extent);
    }

    textureDesc.usage = wgpu::TextureUsage::RenderAttachment;
    textureDesc.label = "dstTexture";
    wgpu::Texture dstTexture = device.CreateTexture(&textureDesc);

    const char* shader = R"(
        @group(0) @binding(0) var tex: texture_2d<u32>;

        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }

        @fragment fn fs() -> @location(0) vec4u {
            return textureLoad(tex, vec2u(0), 0);
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;
    desc.cFragment.targetCount = 1;
    desc.cTargets[0].format = wgpu::TextureFormat::RGBA8Uint;
    desc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, srcTexture.CreateView()},
                                                     });
    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        utils::ComboRenderPassDescriptor passDescriptor({dstTexture.CreateView()});
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(1);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Texture texture = replay->GetObjectByLabel<wgpu::Texture>("dstTexture");
        ASSERT_NE(texture, nullptr);

        uint8_t expected[] = {0x11, 0x22, 0x33, 0x44};
        EXPECT_TEXTURE_EQ(&expected[0], texture, {0, 0}, {1, 1}, 0, wgpu::TextureAspect::All);
    }
}

TEST_P(CaptureAndReplayTests, CaptureRenderPassBasicWithAttributes) {
    const float myVertices[] = {
        -1, -1, 3, -1, -1, 3,
    };

    wgpu::BufferDescriptor descriptor;
    descriptor.label = "vertexBuffer";
    descriptor.size = sizeof(myVertices);
    descriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;
    wgpu::Buffer vertexBuffer = device.CreateBuffer(&descriptor);

    queue.WriteBuffer(vertexBuffer, 0, &myVertices, sizeof(myVertices));

    wgpu::TextureDescriptor textureDesc;
    textureDesc.label = "dstTexture";
    textureDesc.size = {1, 1, 1};
    textureDesc.format = wgpu::TextureFormat::RGBA8Uint;
    textureDesc.usage = wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture dstTexture = device.CreateTexture(&textureDesc);

    const char* shader = R"(
        @vertex fn vs(@location(0) pos: vec4f) -> @builtin(position) vec4f {
            return pos;
        }

        @fragment fn fs() -> @location(0) vec4u {
            return vec4u(0x11, 0x22, 0x33, 0x44);
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;
    desc.cFragment.targetCount = 1;
    desc.cTargets[0].format = wgpu::TextureFormat::RGBA8Uint;
    desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    desc.cBuffers[0].arrayStride = 2 * sizeof(float);
    desc.cBuffers[0].attributeCount = 1;
    desc.cBuffers[0].attributes = &desc.cAttributes[0];
    desc.cAttributes[0].shaderLocation = 0;
    desc.cAttributes[0].format = wgpu::VertexFormat::Float32x2;
    desc.vertex.bufferCount = 1;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        utils::ComboRenderPassDescriptor passDescriptor({dstTexture.CreateView()});
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.Draw(3);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Texture texture = replay->GetObjectByLabel<wgpu::Texture>("dstTexture");
        ASSERT_NE(texture, nullptr);

        uint8_t expected[] = {0x11, 0x22, 0x33, 0x44};
        EXPECT_TEXTURE_EQ(&expected[0], texture, {0, 0}, {1, 1}, 0, wgpu::TextureAspect::All);
    }
}

// Capture and replay a compute shader with an explicit bindGroupLayout
TEST_P(CaptureAndReplayTests, CaptureComputeShaderBasicExplicitBindGroup) {
    wgpu::BindGroupLayoutEntry entries[1];
    entries[0].binding = 0;
    entries[0].visibility = wgpu::ShaderStage::Compute;
    entries[0].buffer.type = wgpu::BufferBindingType::Storage;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.entryCount = 1;
    bglDesc.entries = entries;
    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&bglDesc);

    wgpu::PipelineLayoutDescriptor plDesc;
    plDesc.bindGroupLayoutCount = 1;
    plDesc.bindGroupLayouts = &layout;
    wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&plDesc);

    const char* label = "MyBuffer";

    wgpu::BufferDescriptor descriptor;
    descriptor.label = label;
    descriptor.size = 4;
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
    csDesc.layout = pipelineLayout;
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
        pass.SetBindGroup(0, bindGroup);
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Buffer buffer = replay->GetObjectByLabel<wgpu::Buffer>(label);
        ASSERT_NE(buffer, nullptr);

        uint8_t expected[] = {0x11, 0x22, 0x33, 0x44};
        EXPECT_BUFFER_U8_RANGE_EQ(expected, buffer, 0, sizeof(expected));
    }
}

// Capture and replay a pass that uses a storage texture
TEST_P(CaptureAndReplayTests, CaptureStorageTextureUsageWithExplicitBindGroupLayout) {
    wgpu::BindGroupLayoutEntry entries[1];
    entries[0].binding = 0;
    entries[0].visibility = wgpu::ShaderStage::Compute;
    entries[0].storageTexture.access = wgpu::StorageTextureAccess::WriteOnly;
    entries[0].storageTexture.format = wgpu::TextureFormat::RGBA8Uint;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.entryCount = 1;
    bglDesc.entries = entries;
    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&bglDesc);

    wgpu::PipelineLayoutDescriptor plDesc;
    plDesc.bindGroupLayoutCount = 1;
    plDesc.bindGroupLayouts = &layout;
    wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&plDesc);

    wgpu::TextureDescriptor textureDesc;
    textureDesc.label = "myTexture";
    textureDesc.size = {1, 1, 1};
    textureDesc.format = wgpu::TextureFormat::RGBA8Uint;
    textureDesc.usage = wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc;
    wgpu::Texture texture = device.CreateTexture(&textureDesc);

    const char* shader = R"(
        @group(0) @binding(0) var tex: texture_storage_2d<rgba8uint, write>;

        @compute @workgroup_size(1) fn main() {
            textureStore(tex, vec2u(0), vec4u(0x11, 0x22, 0x33, 0x44));
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.layout = pipelineLayout;
    csDesc.compute.module = module;
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, texture.CreateView()},
                                                     });

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetBindGroup(0, bindGroup);
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        wgpu::Texture texture = replay->GetObjectByLabel<wgpu::Texture>("myTexture");
        ASSERT_NE(texture, nullptr);

        uint8_t expected[] = {0x11, 0x22, 0x33, 0x44};
        EXPECT_TEXTURE_EQ(&expected[0], texture, {0, 0}, {1, 1}, 0, wgpu::TextureAspect::All);
    }
}

DAWN_INSTANTIATE_TEST(CaptureAndReplayTests, WebGPUBackend());

}  // anonymous namespace
}  // namespace dawn
