// Copyright 2026 The Dawn & Tint Authors
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

#include "src/dawn/native/opengl/BufferGL.h"
#include "src/dawn/native/opengl/DeviceGL.h"
#include "src/dawn/tests/DawnTest.h"

namespace dawn {
namespace {

class GLBufferTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    }

    native::opengl::Buffer* ToBackendBuffer(const wgpu::Buffer& buffer) {
        return native::opengl::ToBackend(native::FromAPI(buffer.Get()));
    }
};

// Test that the backend clears its cached mapping pointer when a buffer that was
// mapped via MapAsync is unmapped. If the pointer is left in place a later
// FinalizeMap that runs without a fresh backend mapping would latch stale memory.
TEST_P(GLBufferTests, MapAsyncUnmapClearsMappedData) {
    wgpu::BufferDescriptor desc = {};
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    native::opengl::Buffer* bufferGL = ToBackendBuffer(buffer);
    EXPECT_EQ(bufferGL->GetMappedDataForTesting(), nullptr);

    MapAsyncAndWait(buffer, wgpu::MapMode::Write, 0, 4);
    EXPECT_NE(bufferGL->GetMappedDataForTesting(), nullptr);
    EXPECT_NE(buffer.GetMappedRange(0, 4), nullptr);

    buffer.Unmap();
    EXPECT_EQ(bufferGL->GetMappedDataForTesting(), nullptr);
}

// Test that the backend clears its cached mapping pointer when a buffer that was
// mapped at creation is unmapped.
TEST_P(GLBufferTests, MappedAtCreationUnmapClearsMappedData) {
    wgpu::BufferDescriptor desc = {};
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
    desc.mappedAtCreation = true;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    native::opengl::Buffer* bufferGL = ToBackendBuffer(buffer);
    EXPECT_NE(bufferGL->GetMappedDataForTesting(), nullptr);
    EXPECT_NE(buffer.GetMappedRange(0, 4), nullptr);

    buffer.Unmap();
    EXPECT_EQ(bufferGL->GetMappedDataForTesting(), nullptr);
}

DAWN_INSTANTIATE_TEST(GLBufferTests,
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      OpenGLESBackend({"gl_defer"}));

}  // anonymous namespace
}  // namespace dawn
