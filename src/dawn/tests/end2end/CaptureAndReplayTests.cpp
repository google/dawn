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

namespace dawn {
namespace {

class CaptureAndReplayTests : public DawnTest {};

// Test that the simplest map read works
TEST_P(CaptureAndReplayTests, Basic) {
    std::ostringstream commandStream;
    std::ostringstream contentStream;

    const char* label = "MyBuffer";
    const uint8_t myData[] = {0x11, 0x22, 0x33, 0x44};

    {
        native::webgpu::StartCapture(device.Get(), commandStream, contentStream);
        wgpu::BufferDescriptor descriptor;
        descriptor.label = label;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::CopyDst;
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

        constexpr size_t kSize = sizeof(myData);
        queue.WriteBuffer(buffer, 0, &myData, kSize);

        native::webgpu::EndCapture(device.Get());
    }

    std::string commandData = commandStream.str();
    std::string contentData = contentStream.str();

    {
        std::istringstream commandIStream(commandData);
        std::istringstream contentIStream(contentData);

        std::unique_ptr<replay::Capture> capture = replay::Capture::Create(
            commandIStream, commandData.size(), contentIStream, contentData.size());
        std::unique_ptr<replay::Replay> replay = replay::Replay::Create(device, capture.get());

        auto result = replay->Play();
        ASSERT_TRUE(result.IsSuccess());

        wgpu::Buffer buffer = replay->GetObjectByLabel<wgpu::Buffer>(label);
        ASSERT_NE(buffer, nullptr);

        EXPECT_BUFFER_U8_RANGE_EQ(myData, buffer, 0, sizeof(myData));
    }
}

// MAINTAINENCE_TODO(crbug.com/413053623): test a 1 character label. This would force the next
// command to / data to be offset by 1 byte. It's possible all uint64_t should be 8 byte aligned? Or
// if not that it's possible that buffer/texture data should be 4 or 8 byte aligned so the data can
// be used in place.

DAWN_INSTANTIATE_TEST(CaptureAndReplayTests, WebGPUBackend());

}  // anonymous namespace
}  // namespace dawn
