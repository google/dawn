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
#include "src/dawn/native/opengl/DisplayEGL.h"
#include "src/dawn/native/opengl/EGLFunctions.h"
#include "src/dawn/native/opengl/PhysicalDeviceGL.h"
#include "src/dawn/native/opengl/UtilsGL.h"
#include "src/dawn/tests/DawnTest.h"

namespace dawn::native::opengl {
namespace {

using ::testing::_;

// A GPU reset can discard GPU work while fences still signal and no GL error is generated (for
// example leaving the destination of a readback unwritten). These tests check that the GL context
// can report resets and that a reset loses the device before data becomes visible to the app.
class GLGraphicsResetTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
        DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("disable_robustness"));
        // GL errors abort in the debug output callback when backend validation is enabled.
        DAWN_TEST_UNSUPPORTED_IF(IsBackendValidationEnabled());
    }
};

// The context is created with the EGL_LOSE_CONTEXT_ON_RESET notification strategy, without which
// glGetGraphicsResetStatus never reports resets.
TEST_P(GLGraphicsResetTests, ContextUsesLoseContextOnReset) {
    Device* deviceGL = ToBackend(FromAPI(device.Get()));

    GLint strategy = 0;
    GLenum status = GL_NO_ERROR;
    EXPECT_TRUE(deviceGL
                    ->ExecuteGL(ExecutionQueueBase::SubmitMode::Passive,
                                [&](const OpenGLFunctions& gl) -> MaybeError {
                                    DAWN_GL_TRY_ALWAYS_CHECK(
                                        gl, GetIntegerv(GL_RESET_NOTIFICATION_STRATEGY, &strategy));
                                    status = gl.GetGraphicsResetStatus();
                                    return {};
                                })
                    .IsSuccess());
    EXPECT_EQ(strategy, GL_LOSE_CONTEXT_ON_RESET);
    EXPECT_EQ(status, static_cast<GLenum>(GL_NO_ERROR));
}

// A context reset (simulated with GL_CHROMIUM_lose_context) is reported by
// CheckGraphicsResetStatus and loses the device on the next submit.
TEST_P(GLGraphicsResetTests, ResetIsDetected) {
    Device* deviceGL = ToBackend(FromAPI(device.Get()));
    bool supportsLoseContext = false;
    EXPECT_TRUE(deviceGL
                    ->ExecuteGL(ExecutionQueueBase::SubmitMode::Passive,
                                [&](const OpenGLFunctions& gl) -> MaybeError {
                                    supportsLoseContext =
                                        gl.IsGLExtensionSupported("GL_CHROMIUM_lose_context");
                                    return {};
                                })
                    .IsSuccess());
    DAWN_TEST_UNSUPPORTED_IF(!supportsLoseContext);

    using PFNGLLOSECONTEXTCHROMIUMPROC = void(KHRONOS_APIENTRY*)(GLenum current, GLenum other);
    const EGLFunctions& egl = ToBackend(deviceGL->GetPhysicalDevice())->GetDisplay()->egl.get();
    auto loseContext =
        reinterpret_cast<PFNGLLOSECONTEXTCHROMIUMPROC>(egl.GetProcAddress("glLoseContextCHROMIUM"));
    ASSERT_NE(loseContext, nullptr);

    MaybeError result = deviceGL->ExecuteGL(
        ExecutionQueueBase::SubmitMode::Passive, [&](const OpenGLFunctions& gl) -> MaybeError {
            DAWN_TRY(CheckGraphicsResetStatus(gl, "Before the reset"));
            loseContext(GL_GUILTY_CONTEXT_RESET, GL_INNOCENT_CONTEXT_RESET);
            return CheckGraphicsResetStatus(gl, "After the reset");
        });
    ASSERT_TRUE(result.IsError());
    EXPECT_EQ(result.AcquireError()->GetType(), InternalErrorType::BackendDeviceLost);

    EXPECT_CALL(mDeviceLostCallback,
                Call(CHandleIs(device.Get()), wgpu::DeviceLostReason::Unknown, _))
        .Times(1);
    queue.Submit(0, nullptr);
}

DAWN_INSTANTIATE_TEST(GLGraphicsResetTests,
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      OpenGLESBackend({"gl_defer"}),
                      OpenGLESBackend({"gl_force_es_31_and_no_extensions"}));

}  // anonymous namespace
}  // namespace dawn::native::opengl
