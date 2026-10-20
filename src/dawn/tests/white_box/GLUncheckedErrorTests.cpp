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

#include "src/dawn/native/ErrorData.h"
#include "src/dawn/native/opengl/DeviceGL.h"
#include "src/dawn/native/opengl/UtilsGL.h"
#include "src/dawn/tests/DawnTest.h"

namespace dawn::native::opengl {
namespace {

using testing::_;

// An invalid capability, used to generate a GL_INVALID_ENUM error with glEnable.
constexpr GLenum kInvalidCapability = 0;

// When asserts are disabled, DAWN_GL_TRY does not check for GL errors. These tests simulate such
// unchecked errors by making raw GL calls that fail, and check that they are caught before data
// becomes visible to the app.
class GLUncheckedErrorTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
        // GL errors abort in the debug output callback when backend validation is enabled.
        DAWN_TEST_UNSUPPORTED_IF(IsBackendValidationEnabled());
    }
};

// CheckPendingErrorBeforeUserVisibleData succeeds when there are no errors, fails when there is a
// pending error and drains the pending errors.
TEST_P(GLUncheckedErrorTests, CheckPendingErrorBeforeUserVisibleData) {
    Device* deviceGL = ToBackend(FromAPI(device.Get()));
    // Runs the check on the GL context, after optionally generating an unchecked GL error.
    auto Check = [deviceGL](bool generateError) -> MaybeError {
        return deviceGL->ExecuteGL(ExecutionQueueBase::SubmitMode::Passive,
                                   [generateError](const OpenGLFunctions& gl) -> MaybeError {
                                       if (generateError) {
                                           gl.Enable(kInvalidCapability);
                                       }
                                       return CheckPendingErrorBeforeUserVisibleData(
                                           gl, "Unchecked OpenGL calls in test");
                                   });
    };

    EXPECT_TRUE(Check(false).IsSuccess());

    {
        MaybeError result = Check(true);
        ASSERT_TRUE(result.IsError());
        auto error = result.AcquireError();
        EXPECT_EQ(error->GetType(), InternalErrorType::Unrecoverable);
    }

    // The error was drained by the previous check.
    EXPECT_TRUE(Check(false).IsSuccess());
}

// An unchecked GL error before a submit, loses the device on the submit.
TEST_P(GLUncheckedErrorTests, ErrorBeforeSubmitLosesDevice) {
    Device* deviceGL = ToBackend(FromAPI(device.Get()));
    EXPECT_TRUE(deviceGL
                    ->EnqueueGL([](const OpenGLFunctions& gl) -> MaybeError {
                        gl.Enable(kInvalidCapability);
                        return {};
                    })
                    .IsSuccess());

    EXPECT_CALL(mDeviceLostCallback,
                Call(CHandleIs(device.Get()), wgpu::DeviceLostReason::Unknown, _))
        .Times(1);
    queue.Submit(0, nullptr);
}

// An unchecked GL error loses the device on MapAsync before the mapped data is exposed.
TEST_P(GLUncheckedErrorTests, ErrorBeforeMapAsyncLosesDevice) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
    // Flush the (possibly deferred) buffer creation.
    queue.Submit(0, nullptr);

    Device* deviceGL = ToBackend(FromAPI(device.Get()));
    EXPECT_TRUE(deviceGL
                    ->EnqueueGL([](const OpenGLFunctions& gl) -> MaybeError {
                        gl.Enable(kInvalidCapability);
                        return {};
                    })
                    .IsSuccess());

    EXPECT_CALL(mDeviceLostCallback,
                Call(CHandleIs(device.Get()), wgpu::DeviceLostReason::Unknown, _))
        .Times(1);
    bool done = false;
    buffer.MapAsync(wgpu::MapMode::Read, 0, 4, wgpu::CallbackMode::AllowProcessEvents,
                    [&done](wgpu::MapAsyncStatus status, wgpu::StringView) {
                        EXPECT_NE(status, wgpu::MapAsyncStatus::Success);
                        done = true;
                    });
    while (!done) {
        WaitABit();
    }
}

// An unchecked GL error is not dropped by a later checked GL call (here the allocation of a buffer)
// and still loses the device.
TEST_P(GLUncheckedErrorTests, ErrorNotDroppedByLaterCheckedCall) {
    Device* deviceGL = ToBackend(FromAPI(device.Get()));
    EXPECT_TRUE(deviceGL
                    ->EnqueueGL([](const OpenGLFunctions& gl) -> MaybeError {
                        gl.Enable(kInvalidCapability);
                        return {};
                    })
                    .IsSuccess());

    EXPECT_CALL(mDeviceLostCallback,
                Call(CHandleIs(device.Get()), wgpu::DeviceLostReason::Unknown, _))
        .Times(1);

    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
    queue.Submit(0, nullptr);
}

DAWN_INSTANTIATE_TEST(GLUncheckedErrorTests,
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      OpenGLESBackend({"gl_defer"}));

}  // anonymous namespace
}  // namespace dawn::native::opengl
