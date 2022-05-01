// Copyright 2021 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <EGL/egl.h>

#include <utility>
#include <vector>

#include "dawn/common/DynamicLib.h"
#include "dawn/native/OpenGLBackend.h"
#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace {

class EGLFunctions {
  public:
    EGLFunctions() {
#ifdef DAWN_PLATFORM_WINDOWS
        const char* eglLib = "libEGL.dll";
#else
        const char* eglLib = "libEGL.so";
#endif
        EXPECT_TRUE(mlibEGL.Open(eglLib));
        CreateImage = reinterpret_cast<PFNEGLCREATEIMAGEPROC>(LoadProc("eglCreateImage"));
        DestroyImage = reinterpret_cast<PFNEGLDESTROYIMAGEPROC>(LoadProc("eglDestroyImage"));
        GetCurrentContext =
            reinterpret_cast<PFNEGLGETCURRENTCONTEXTPROC>(LoadProc("eglGetCurrentContext"));
        GetCurrentDisplay =
            reinterpret_cast<PFNEGLGETCURRENTDISPLAYPROC>(LoadProc("eglGetCurrentDisplay"));
    }

  private:
    void* LoadProc(const char* name) {
        void* proc = mlibEGL.GetProc(name);
        EXPECT_NE(proc, nullptr);
        return proc;
    }

  public:
    PFNEGLCREATEIMAGEPROC CreateImage;
    PFNEGLDESTROYIMAGEPROC DestroyImage;
    PFNEGLGETCURRENTCONTEXTPROC GetCurrentContext;
    PFNEGLGETCURRENTDISPLAYPROC GetCurrentDisplay;

  private:
    DynamicLib mlibEGL;
};

class ScopedEGLImage {
  public:
    ScopedEGLImage(PFNEGLDESTROYIMAGEPROC destroyImage,
                   PFNGLDELETETEXTURESPROC deleteTextures,
                   EGLDisplay display,
                   EGLImage image,
                   GLuint texture)
        : mDestroyImage(destroyImage),
          mDeleteTextures(deleteTextures),
          mDisplay(display),
          mImage(image),
          mTexture(texture) {}

    ScopedEGLImage(ScopedEGLImage&& other) {
        if (mImage != nullptr) {
            mDestroyImage(mDisplay, mImage);
        }
        if (mTexture != 0) {
            mDeleteTextures(1, &mTexture);
        }
        mDestroyImage = std::move(other.mDestroyImage);
        mDeleteTextures = std::move(other.mDeleteTextures);
        mDisplay = std::move(other.mDisplay);
        mImage = std::move(other.mImage);
        mTexture = std::move(other.mTexture);
    }

    ~ScopedEGLImage() {
        if (mTexture != 0) {
            mDeleteTextures(1, &mTexture);
        }
        if (mImage != nullptr) {
            mDestroyImage(mDisplay, mImage);
        }
    }

    EGLImage getImage() const { return mImage; }

    GLuint getTexture() const { return mTexture; }

  private:
    PFNEGLDESTROYIMAGEPROC mDestroyImage = nullptr;
    PFNGLDELETETEXTURESPROC mDeleteTextures = nullptr;
    EGLDisplay mDisplay = nullptr;
    EGLImage mImage = nullptr;
    GLuint mTexture = 0;
};

}  // anonymous namespace

class EGLImageTestBase : public DawnTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::DawnInternalUsages};
    }

  public:
    ScopedEGLImage CreateEGLImage(uint32_t width,
                                  uint32_t height,
                                  GLenum internalFormat,
                                  GLenum format,
                                  GLenum type,
                                  void* data,
                                  size_t size) {
        dawn::native::opengl::Device* openglDevice =
            dawn::native::opengl::ToBackend(dawn::native::FromAPI(device.Get()));
        const dawn::native::opengl::OpenGLFunctions& gl = openglDevice->gl;
        GLuint tex;
        gl.GenTextures(1, &tex);
        gl.BindTexture(GL_TEXTURE_2D, tex);
        gl.TexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);
        EGLAttrib attribs[1] = {EGL_NONE};
        EGLClientBuffer buffer = reinterpret_cast<EGLClientBuffer>(static_cast<intptr_t>(tex));
        EGLDisplay dpy = egl.GetCurrentDisplay();
        EGLContext ctx = egl.GetCurrentContext();
        EGLImage eglImage = egl.CreateImage(dpy, ctx, EGL_GL_TEXTURE_2D, buffer, attribs);
        EXPECT_NE(nullptr, eglImage);

        return ScopedEGLImage(egl.DestroyImage, gl.DeleteTextures, dpy, eglImage, tex);
    }
    wgpu::Texture WrapEGLImage(const wgpu::TextureDescriptor* descriptor, EGLImage eglImage) {
        dawn::native::opengl::ExternalImageDescriptorEGLImage externDesc;
        externDesc.cTextureDescriptor = reinterpret_cast<const WGPUTextureDescriptor*>(descriptor);
        externDesc.image = eglImage;
        WGPUTexture texture = dawn::native::opengl::WrapExternalEGLImage(device.Get(), &externDesc);
        return wgpu::Texture::Acquire(texture);
    }
    EGLFunctions egl;
};

// A small fixture used to initialize default data for the EGLImage validation tests.
// These tests are skipped if the harness is using the wire.
class EGLImageValidationTests : public EGLImageTestBase {
  public:
    EGLImageValidationTests() {
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.size = {10, 10, 1};
        descriptor.sampleCount = 1;
        descriptor.mipLevelCount = 1;
        descriptor.usage = wgpu::TextureUsage::RenderAttachment;
    }

    ScopedEGLImage CreateDefaultEGLImage() {
        return CreateEGLImage(10, 10, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, nullptr, 0);
    }

  protected:
    wgpu::TextureDescriptor descriptor;
};

// Test a successful wrapping of an EGLImage in a texture
TEST_P(EGLImageValidationTests, Success) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    ScopedEGLImage image = CreateDefaultEGLImage();
    wgpu::Texture texture = WrapEGLImage(&descriptor, image.getImage());
    ASSERT_NE(texture.Get(), nullptr);
}

// Test a successful wrapping of an EGLImage in a texture with DawnTextureInternalUsageDescriptor
TEST_P(EGLImageValidationTests, SuccessWithInternalUsageDescriptor) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
    descriptor.nextInChain = &internalDesc;
    internalDesc.internalUsage = wgpu::TextureUsage::CopySrc;
    internalDesc.sType = wgpu::SType::DawnTextureInternalUsageDescriptor;

    ScopedEGLImage image = CreateDefaultEGLImage();
    wgpu::Texture texture = WrapEGLImage(&descriptor, image.getImage());
    ASSERT_NE(texture.Get(), nullptr);
}

// Test an error occurs if an invalid sType is the nextInChain
TEST_P(EGLImageValidationTests, InvalidTextureDescriptor) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    wgpu::ChainedStruct chainedDescriptor;
    chainedDescriptor.sType = wgpu::SType::SurfaceDescriptorFromWindowsSwapChainPanel;
    descriptor.nextInChain = &chainedDescriptor;

    ScopedEGLImage image = CreateDefaultEGLImage();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapEGLImage(&descriptor, image.getImage()));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor dimension isn't 2D
TEST_P(EGLImageValidationTests, InvalidTextureDimension) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    descriptor.dimension = wgpu::TextureDimension::e3D;

    ScopedEGLImage image = CreateDefaultEGLImage();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapEGLImage(&descriptor, image.getImage()));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the texture usage is not RenderAttachment
TEST_P(EGLImageValidationTests, InvalidTextureUsage) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    descriptor.usage = wgpu::TextureUsage::TextureBinding;

    ScopedEGLImage image = CreateDefaultEGLImage();
    wgpu::Texture texture;
    ASSERT_DEVICE_ERROR(texture = WrapEGLImage(&descriptor, image.getImage()));

    ASSERT_EQ(texture.Get(), nullptr);
    descriptor.usage = wgpu::TextureUsage::StorageBinding;

    ASSERT_DEVICE_ERROR(texture = WrapEGLImage(&descriptor, image.getImage()));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor mip level count isn't 1
TEST_P(EGLImageValidationTests, InvalidMipLevelCount) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    descriptor.mipLevelCount = 2;

    ScopedEGLImage image = CreateDefaultEGLImage();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapEGLImage(&descriptor, image.getImage()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor depth isn't 1
TEST_P(EGLImageValidationTests, InvalidDepth) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    descriptor.size.depthOrArrayLayers = 2;

    ScopedEGLImage image = CreateDefaultEGLImage();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapEGLImage(&descriptor, image.getImage()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor sample count isn't 1
TEST_P(EGLImageValidationTests, InvalidSampleCount) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    descriptor.sampleCount = 4;

    ScopedEGLImage image = CreateDefaultEGLImage();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapEGLImage(&descriptor, image.getImage()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor width doesn't match the surface's
TEST_P(EGLImageValidationTests, InvalidWidth) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    descriptor.size.width = 11;

    ScopedEGLImage image = CreateDefaultEGLImage();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapEGLImage(&descriptor, image.getImage()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor height doesn't match the surface's
TEST_P(EGLImageValidationTests, InvalidHeight) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    descriptor.size.height = 11;

    ScopedEGLImage image = CreateDefaultEGLImage();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapEGLImage(&descriptor, image.getImage()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Fixture to test using EGLImages through different usages.
// These tests are skipped if the harness is using the wire.
class EGLImageUsageTests : public EGLImageTestBase {
  public:
    // Test that clearing using BeginRenderPass writes correct data in the eglImage.
    void DoClearTest(EGLImage eglImage,
                     GLuint texture,
                     wgpu::TextureFormat format,
                     GLenum glFormat,
                     GLenum glType,
                     void* data,
                     size_t dataSize) {
        dawn::native::opengl::Device* openglDevice =
            dawn::native::opengl::ToBackend(dawn::native::FromAPI(device.Get()));
        const dawn::native::opengl::OpenGLFunctions& gl = openglDevice->gl;

        // Get a texture view for the eglImage
        wgpu::TextureDescriptor textureDescriptor;
        textureDescriptor.dimension = wgpu::TextureDimension::e2D;
        textureDescriptor.format = format;
        textureDescriptor.size = {1, 1, 1};
        textureDescriptor.sampleCount = 1;
        textureDescriptor.mipLevelCount = 1;
        textureDescriptor.usage = wgpu::TextureUsage::RenderAttachment;

        wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
        textureDescriptor.nextInChain = &internalDesc;
        internalDesc.internalUsage = wgpu::TextureUsage::CopySrc;
        internalDesc.sType = wgpu::SType::DawnTextureInternalUsageDescriptor;

        wgpu::Texture eglImageTexture = WrapEGLImage(&textureDescriptor, eglImage);
        ASSERT_NE(eglImageTexture, nullptr);

        wgpu::TextureView eglImageView = eglImageTexture.CreateView();

        utils::ComboRenderPassDescriptor renderPassDescriptor({eglImageView}, {});
        renderPassDescriptor.cColorAttachments[0].clearValue = {1 / 255.0f, 2 / 255.0f, 3 / 255.0f,
                                                                4 / 255.0f};

        // Execute commands to clear the eglImage
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDescriptor);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Check the correct data was written
        std::vector<uint8_t> result(dataSize);
        GLuint fbo;
        gl.GenFramebuffers(1, &fbo);
        gl.BindFramebuffer(GL_FRAMEBUFFER, fbo);
        gl.FramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture,
                                0);
        gl.ReadPixels(0, 0, 1, 1, glFormat, glType, result.data());
        gl.BindFramebuffer(GL_FRAMEBUFFER, 0);
        gl.DeleteFramebuffers(1, &fbo);
        ASSERT_EQ(0, memcmp(result.data(), data, dataSize));
    }
};

// Test clearing a R8 EGLImage
TEST_P(EGLImageUsageTests, ClearR8EGLImage) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    ScopedEGLImage eglImage = CreateEGLImage(1, 1, GL_R8, GL_RED, GL_UNSIGNED_BYTE, nullptr, 0);

    uint8_t data = 0x01;
    DoClearTest(eglImage.getImage(), eglImage.getTexture(), wgpu::TextureFormat::R8Unorm, GL_RED,
                GL_UNSIGNED_BYTE, &data, sizeof(data));
}

// Test clearing a RG8 EGLImage
TEST_P(EGLImageUsageTests, ClearRG8EGLImage) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    ScopedEGLImage eglImage = CreateEGLImage(1, 1, GL_RG8, GL_RG, GL_UNSIGNED_BYTE, nullptr, 0);

    uint16_t data = 0x0201;
    DoClearTest(eglImage.getImage(), eglImage.getTexture(), wgpu::TextureFormat::RG8Unorm, GL_RG,
                GL_UNSIGNED_BYTE, &data, sizeof(data));
}

// Test clearing an RGBA8 EGLImage
TEST_P(EGLImageUsageTests, ClearRGBA8EGLImage) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    ScopedEGLImage eglImage = CreateEGLImage(1, 1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, nullptr, 0);

    uint32_t data = 0x04030201;
    DoClearTest(eglImage.getImage(), eglImage.getTexture(), wgpu::TextureFormat::RGBA8Unorm,
                GL_RGBA, GL_UNSIGNED_BYTE, &data, sizeof(data));
}

DAWN_INSTANTIATE_TEST(EGLImageValidationTests, OpenGLESBackend());
DAWN_INSTANTIATE_TEST(EGLImageUsageTests, OpenGLESBackend());
