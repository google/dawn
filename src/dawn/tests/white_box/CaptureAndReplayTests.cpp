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

DAWN_INSTANTIATE_TEST(CaptureAndReplayTests, WebGPUBackend());

}  // anonymous namespace
}  // namespace dawn
