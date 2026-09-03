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
#include "src/dawn/native/ErrorInjector.h"
#include "src/dawn/native/opengl/DeviceGL.h"
#include "src/dawn/native/opengl/TextureGL.h"
#include "src/dawn/native/opengl/UtilsGL.h"
#include "src/dawn/tests/DawnTest.h"

namespace dawn::native::opengl {
namespace {

class GLFramebufferCompletenessTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    }
};

// CheckFramebufferComplete must succeed for a valid color attachment and fail for a
// framebuffer with no attachments.
TEST_P(GLFramebufferCompletenessTests, DrawFramebuffer) {
    Device* deviceGL = ToBackend(FromAPI(device.Get()));
    const OpenGLFunctions& gl = deviceGL->GetGL();

    GLint prevDrawFBO = 0;
    gl.GetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDrawFBO);

    GLuint fbo = 0;
    gl.GenFramebuffers(1, &fbo);
    gl.BindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

    // A newly created framebuffer with no attachments and no default width/height is not
    // complete (GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT).
    {
        MaybeError result = CheckFramebufferComplete(gl, GL_DRAW_FRAMEBUFFER);
        EXPECT_TRUE(result.IsError());
        result.AcquireError();
    }

    // Attaching a color-renderable texture makes the framebuffer complete.
    GLuint tex = 0;
    gl.GenTextures(1, &tex);
    gl.BindTexture(GL_TEXTURE_2D, tex);
    gl.TexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 4, 4);
    gl.FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    {
        MaybeError result = CheckFramebufferComplete(gl, GL_DRAW_FRAMEBUFFER);
        EXPECT_FALSE(result.IsError());
        result.AcquireError();
    }

    gl.DeleteTextures(1, &tex);
    gl.DeleteFramebuffers(1, &fbo);
    gl.BindFramebuffer(GL_DRAW_FRAMEBUFFER, prevDrawFBO);
}

// GLFramebufferStatusAsString must return a readable name for known status values.
TEST_P(GLFramebufferCompletenessTests, StatusAsString) {
    EXPECT_STREQ("GL_FRAMEBUFFER_COMPLETE", GLFramebufferStatusAsString(GL_FRAMEBUFFER_COMPLETE));
    EXPECT_STREQ("GL_FRAMEBUFFER_UNSUPPORTED",
                 GLFramebufferStatusAsString(GL_FRAMEBUFFER_UNSUPPORTED));
    EXPECT_STREQ("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT",
                 GLFramebufferStatusAsString(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT));
}

#if defined(DAWN_ENABLE_ERROR_INJECTION)

// Test that Texture::EnsureSubresourceContentInitialized (which calls Texture::ClearTexture)
// verifies framebuffer completeness before clearing.
// Injecting GL_FRAMEBUFFER_UNSUPPORTED on glCheckFramebufferStatus mimics OpenGL ES drivers
// where certain formats (such as floating-point textures on drivers without
// GL_EXT_color_buffer_float, or drivers returning GL_FRAMEBUFFER_UNSUPPORTED) cannot be attached
// as complete framebuffer attachments. On such drivers, glClearBuffer* generates
// GL_INVALID_FRAMEBUFFER_OPERATION and performs no writes; verifying completeness ensures Dawn
// catches the error instead of silently marking uninitialized/dirty GPU memory as initialized.
TEST_P(GLFramebufferCompletenessTests, LazyClearChecksCompleteness) {
    Device* deviceGL = ToBackend(FromAPI(device.Get()));
    const OpenGLFunctions& gl = deviceGL->GetGL();

    // 1. 2D Color Texture (e.g. RGBA16Float)
    {
        wgpu::TextureDescriptor desc;
        desc.size = {4, 4, 1};
        desc.format = wgpu::TextureFormat::RGBA16Float;
        desc.usage = wgpu::TextureUsage::TextureBinding;
        wgpu::Texture tex = device.CreateTexture(&desc);

        Texture* texGL = ToBackend(FromAPI(tex.Get()));

        EnableErrorInjector();
        InjectErrorAt(0u);
        MaybeError err =
            texGL->EnsureSubresourceContentInitialized(gl, texGL->GetAllSubresources());
        EXPECT_TRUE(err.IsError());
        err.AcquireError();
        EXPECT_EQ(AcquireErrorInjectorCallCount(), 1u);
        DisableErrorInjector();
    }

    // 2. Depth/Stencil Texture
    {
        wgpu::TextureDescriptor desc;
        desc.size = {4, 4, 1};
        desc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        desc.usage = wgpu::TextureUsage::TextureBinding;
        wgpu::Texture tex = device.CreateTexture(&desc);

        Texture* texGL = ToBackend(FromAPI(tex.Get()));

        EnableErrorInjector();
        InjectErrorAt(0u);
        MaybeError err =
            texGL->EnsureSubresourceContentInitialized(gl, texGL->GetAllSubresources());
        EXPECT_TRUE(err.IsError());
        err.AcquireError();
        EXPECT_EQ(AcquireErrorInjectorCallCount(), 1u);
        DisableErrorInjector();
    }

    // 3. 3D Color Texture
    {
        wgpu::TextureDescriptor desc;
        desc.dimension = wgpu::TextureDimension::e3D;
        desc.size = {4, 4, 4};
        desc.format = wgpu::TextureFormat::RGBA8Unorm;
        desc.usage = wgpu::TextureUsage::TextureBinding;
        wgpu::Texture tex = device.CreateTexture(&desc);

        Texture* texGL = ToBackend(FromAPI(tex.Get()));

        EnableErrorInjector();
        InjectErrorAt(0u);
        MaybeError err =
            texGL->EnsureSubresourceContentInitialized(gl, texGL->GetAllSubresources());
        EXPECT_TRUE(err.IsError());
        err.AcquireError();
        EXPECT_EQ(AcquireErrorInjectorCallCount(), 1u);
        DisableErrorInjector();
    }
}

// Test that CopyImageSubData checks framebuffer completeness for both read and draw framebuffers.
TEST_P(GLFramebufferCompletenessTests, CopyImageSubDataChecksCompleteness) {
    Device* deviceGL = ToBackend(FromAPI(device.Get()));
    const OpenGLFunctions& gl = deviceGL->GetGL();

    GLuint srcTex = 0;
    GLuint dstTex = 0;
    gl.GenTextures(1, &srcTex);
    gl.GenTextures(1, &dstTex);
    gl.BindTexture(GL_TEXTURE_2D, srcTex);
    gl.TexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 4, 4);
    gl.BindTexture(GL_TEXTURE_2D, dstTex);
    gl.TexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 4, 4);

    EnableErrorInjector();

    // 1. Error on GL_READ_FRAMEBUFFER (index 0)
    {
        InjectErrorAt(0u);
        MaybeError err = CopyImageSubData(gl, Aspect::Color, srcTex, GL_TEXTURE_2D, 0, {0, 0, 0},
                                          dstTex, GL_TEXTURE_2D, 0, {0, 0, 0}, {4, 4, 1});
        EXPECT_TRUE(err.IsError());
        err.AcquireError();
        EXPECT_EQ(AcquireErrorInjectorCallCount(), 1u);
    }

    // 2. Error on GL_DRAW_FRAMEBUFFER (index 1)
    {
        InjectErrorAt(1u);
        MaybeError err = CopyImageSubData(gl, Aspect::Color, srcTex, GL_TEXTURE_2D, 0, {0, 0, 0},
                                          dstTex, GL_TEXTURE_2D, 0, {0, 0, 0}, {4, 4, 1});
        EXPECT_TRUE(err.IsError());
        err.AcquireError();
        EXPECT_EQ(AcquireErrorInjectorCallCount(), 2u);
    }

    DisableErrorInjector();
    gl.DeleteTextures(1, &srcTex);
    gl.DeleteTextures(1, &dstTex);
}

#endif  // defined(DAWN_ENABLE_ERROR_INJECTION)

DAWN_INSTANTIATE_TEST(GLFramebufferCompletenessTests, OpenGLESBackend());

}  // anonymous namespace
}  // namespace dawn::native::opengl
