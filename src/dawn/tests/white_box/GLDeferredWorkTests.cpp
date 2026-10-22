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

#include <vector>

#include "dawn/native/DawnNative.h"
#include "src/dawn/native/Error.h"
#include "src/dawn/native/opengl/DeviceGL.h"
#include "src/dawn/tests/DawnTest.h"
#include "src/dawn/utils/WGPUHelpers.h"

namespace dawn::native::opengl {
namespace {

using ::testing::_;

class GLDeferredWorkTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    }

    wgpu::Texture CreateTexture() {
        wgpu::TextureDescriptor descriptor;
        descriptor.size = {kSize, kSize, 1};
        descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
        wgpu::Texture texture = device.CreateTexture(&descriptor);

        // Flush the (possibly deferred) texture creation so that the next batch of deferred work
        // only contains the work enqueued by the test.
        queue.Submit(0, nullptr);
        return texture;
    }

    void WriteWholeTexture(const wgpu::Texture& texture, const std::vector<utils::RGBA8>& data) {
        wgpu::TexelCopyTextureInfo destination =
            utils::CreateTexelCopyTextureInfo(texture, 0, {0, 0, 0});
        wgpu::TexelCopyBufferLayout dataLayout =
            utils::CreateTexelCopyBufferLayout(0, kSize * sizeof(utils::RGBA8), kSize);
        wgpu::Extent3D writeSize = {kSize, kSize, 1};
        queue.WriteTexture(&destination, data.data(), data.size() * sizeof(utils::RGBA8),
                           &dataLayout, &writeSize);
    }

    static constexpr uint32_t kSize = 4;
};

// GLSL translation validation errors should be produced at pipeline creation time, not when
// flushing deferred work. The shader passes WGSL validation but is rejected by the GLSL writer
// (struct member padding over Tint's limit).
TEST_P(GLDeferredWorkTests, GLSLTranslationErrorAtPipelineCreation) {
    wgpu::Texture texture = CreateTexture();

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        struct S {
            a : u32,
            @size(20000000) pad : u32,
            b : u32,
        }
        @group(0) @binding(0) var<storage, read_write> buf : S;

        @compute @workgroup_size(1) fn main() {
            buf.a = buf.b;
        }
    )");
    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.compute.module = module;
    ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&pipelineDesc));

    // Work deferred after the failed pipeline creation still executes.
    std::vector<utils::RGBA8> data(kSize * kSize, utils::RGBA8(100, 100, 100, 100));
    WriteWholeTexture(texture, data);
    queue.Submit(0, nullptr);

    EXPECT_TRUE(native::IsTextureSubresourceInitialized(texture.Get(), 0, 1, 0, 1));
    EXPECT_TEXTURE_EQ(data.data(), texture, {0, 0}, {kSize, kSize});
}

// A validation error returned by deferred GL work becomes an unrecoverable error (GL work returns
// MaybeError) and loses the device, since the rest of the deferred work (here a WriteTexture) is
// dropped.
TEST_P(GLDeferredWorkTests, ValidationErrorInDeferredWorkLosesDevice) {
    DAWN_TEST_UNSUPPORTED_IF(!HasToggleEnabled("gl_defer"));

    wgpu::Texture texture = CreateTexture();

    Device* deviceGL = ToBackend(FromAPI(device.Get()));
    ASSERT_TRUE(deviceGL
                    ->EnqueueGL([](const OpenGLFunctions&) -> MaybeError {
                        return DAWN_VALIDATION_ERROR("Injected validation error for testing");
                    })
                    .IsSuccess());

    std::vector<utils::RGBA8> data(kSize * kSize, utils::RGBA8(100, 100, 100, 100));
    WriteWholeTexture(texture, data);

    EXPECT_CALL(mDeviceLostCallback,
                Call(CHandleIs(device.Get()), wgpu::DeviceLostReason::Unknown, _))
        .Times(1);
    queue.Submit(0, nullptr);
}

// A pipeline error (e.g. a driver GLSL compile or link failure) returned by deferred GL work loses
// the device, since the rest of the deferred work is dropped.
TEST_P(GLDeferredWorkTests, PipelineErrorInDeferredWorkLosesDevice) {
    DAWN_TEST_UNSUPPORTED_IF(!HasToggleEnabled("gl_defer"));

    Device* deviceGL = ToBackend(FromAPI(device.Get()));
    ASSERT_TRUE(deviceGL
                    ->EnqueueGL([](const OpenGLFunctions&) -> MaybeError {
                        return DAWN_PIPELINE_UNCATEGORIZED_ERROR(
                            "Injected pipeline error for testing");
                    })
                    .IsSuccess());

    EXPECT_CALL(mDeviceLostCallback,
                Call(CHandleIs(device.Get()), wgpu::DeviceLostReason::Unknown, _))
        .Times(1);
    queue.Submit(0, nullptr);
}

DAWN_INSTANTIATE_TEST(GLDeferredWorkTests,
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      OpenGLESBackend({"gl_defer"}));

}  // anonymous namespace
}  // namespace dawn::native::opengl
