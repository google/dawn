// Copyright 2024 The Dawn & Tint Authors
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

#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderBundleEncoderDescriptor.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"
#include "webgpu/webgpu_glfw.h"

#include "GLFW/glfw3.h"

namespace dawn::utils {
static constexpr std::array<wgpu::CompositeAlphaMode, 5> kAllAlphaModes = {
    wgpu::CompositeAlphaMode::Auto,          wgpu::CompositeAlphaMode::Opaque,
    wgpu::CompositeAlphaMode::Premultiplied, wgpu::CompositeAlphaMode::Unpremultiplied,
    wgpu::CompositeAlphaMode::Inherit,
};
static constexpr std::array<wgpu::PresentMode, 3> kAllPresentModes = {
    wgpu::PresentMode::Fifo,
    wgpu::PresentMode::Immediate,
    wgpu::PresentMode::Mailbox,
};
}  // namespace dawn::utils

namespace dawn {
namespace {

struct GLFWindowDestroyer {
    void operator()(GLFWwindow* ptr) { glfwDestroyWindow(ptr); }
};

class SurfaceConfigurationValidationTests : public DawnTest {
  public:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
        DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

        // TODO(dawn:2320): Reconfiguring/unconfiguring fails with swift shader.
        // NB: Fixing SurfaceTests.ReconfigureBasic should fix the other test cases as well.
        DAWN_TEST_UNSUPPORTED_IF(IsSwiftshader());

        glfwSetErrorCallback([](int code, const char* message) {
            ErrorLog() << "GLFW error " << code << " " << message;
        });

        // GLFW can fail to start in headless environments, in which SwapChainTests are
        // inapplicable. Skip this cases without producing a test failure.
        if (glfwInit() == GLFW_FALSE) {
            GTEST_SKIP();
        }

        // Set GLFW_NO_API to avoid GLFW bringing up a GL context that we won't use.
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window.reset(glfwCreateWindow(500, 400, "SurfaceConfigurationValidationTests window",
                                      nullptr, nullptr));

        int width;
        int height;
        glfwGetFramebufferSize(window.get(), &width, &height);

        baseConfig.device = device;
        baseConfig.width = width;
        baseConfig.height = height;
        baseConfig.usage = wgpu::TextureUsage::RenderAttachment;
        baseConfig.viewFormatCount = 0;
        baseConfig.viewFormats = nullptr;
    }

    wgpu::Surface CreateTestSurface() {
        return wgpu::glfw::CreateSurfaceForWindow(GetInstance(), window.get());
    }

    wgpu::SurfaceConfiguration GetPreferredConfiguration(wgpu::Surface surface) {
        wgpu::SurfaceCapabilities capabilities;
        surface.GetCapabilities(adapter, &capabilities);

        wgpu::TextureFormat preferredFormat = surface.GetPreferredFormat(adapter);

        wgpu::SurfaceConfiguration config = baseConfig;
        config.format = preferredFormat;
        config.alphaMode = capabilities.alphaModes[0];
        config.presentMode = capabilities.presentModes[0];
        return config;
    }

    bool SupportsFormat(const wgpu::SurfaceCapabilities& capabilities, wgpu::TextureFormat format) {
        for (size_t i = 0; i < capabilities.formatCount; ++i) {
            if (capabilities.formats[i] == format) {
                return true;
            }
        }
        return false;
    }

    bool SupportsAlphaMode(const wgpu::SurfaceCapabilities& capabilities,
                           wgpu::CompositeAlphaMode mode) {
        for (size_t i = 0; i < capabilities.alphaModeCount; ++i) {
            if (capabilities.alphaModes[i] == mode) {
                return true;
            }
        }
        return false;
    }

    bool SupportsPresentMode(const wgpu::SurfaceCapabilities& capabilities,
                             wgpu::PresentMode mode) {
        for (size_t i = 0; i < capabilities.presentModeCount; ++i) {
            if (capabilities.presentModes[i] == mode) {
                return true;
            }
        }
        return false;
    }

  protected:
    std::unique_ptr<GLFWwindow, GLFWindowDestroyer> window = nullptr;
    wgpu::SurfaceConfiguration baseConfig;
};

// Using undefined format is not valid
TEST_P(SurfaceConfigurationValidationTests, UndefinedFormat) {
    wgpu::SurfaceConfiguration config;
    config.device = device;
    config.format = wgpu::TextureFormat::Undefined;
    ASSERT_DEVICE_ERROR(CreateTestSurface().Configure(&config));
}

// Supports at least one configuration
TEST_P(SurfaceConfigurationValidationTests, AtLeastOneSupportedConfiguration) {
    wgpu::Surface surface = CreateTestSurface();

    wgpu::SurfaceCapabilities capabilities;
    surface.GetCapabilities(adapter, &capabilities);

    ASSERT_GT(capabilities.formatCount, 0u);
    ASSERT_GT(capabilities.alphaModeCount, 0u);
    ASSERT_GT(capabilities.presentModeCount, 0u);
}

// Using any combination of the reported capability is ok for configuring the surface.
TEST_P(SurfaceConfigurationValidationTests, AnyCombinationOfCapabilities) {
    // TODO(dawn:2320): This test crashes width SwiftShader (NB: fixing
    // SurfaceTests.ReconfigureBasic should fix this test as well)
    DAWN_SUPPRESS_TEST_IF(IsSwiftshader());

    wgpu::Surface surface = CreateTestSurface();

    wgpu::SurfaceConfiguration config = baseConfig;

    wgpu::SurfaceCapabilities capabilities;
    surface.GetCapabilities(adapter, &capabilities);

    for (wgpu::TextureFormat format : dawn::utils::kAllTextureFormats) {
        for (wgpu::CompositeAlphaMode alphaMode : dawn::utils::kAllAlphaModes) {
            for (wgpu::PresentMode presentMode : dawn::utils::kAllPresentModes) {
                config.format = format;
                config.alphaMode = alphaMode;
                config.presentMode = presentMode;

                // TODO(dawn:2320) Switching to Mailbox from a different present mode may raise a
                // VK_ERROR_OUT_OF_DEVICE_MEMORY
                if (IsVulkan() && presentMode == wgpu::PresentMode::Mailbox) {
                    continue;
                }

                if (!SupportsFormat(capabilities, config.format) ||
                    !SupportsAlphaMode(capabilities, config.alphaMode) ||
                    !SupportsPresentMode(capabilities, config.presentMode)) {
                    ASSERT_DEVICE_ERROR(surface.Configure(&config));
                } else {
                    surface.Configure(&config);

                    // Check that we can present
                    wgpu::SurfaceTexture surfaceTexture;
                    surface.GetCurrentTexture(&surfaceTexture);
                    surface.Present();
                }
                device.Tick();
            }
        }
    }
}

// Preferred format is always valid.
TEST_P(SurfaceConfigurationValidationTests, PreferredFormatIsValid) {
    wgpu::Surface surface = CreateTestSurface();

    wgpu::SurfaceCapabilities capabilities;
    surface.GetCapabilities(adapter, &capabilities);

    wgpu::TextureFormat preferredFormat = surface.GetPreferredFormat(adapter);
    bool found = false;
    for (size_t i = 0; i < capabilities.formatCount; ++i) {
        found = found || capabilities.formats[i] == preferredFormat;
    }
    ASSERT_TRUE(found);
}

// Invalid view format fails
TEST_P(SurfaceConfigurationValidationTests, InvalidViewFormat) {
    wgpu::Surface surface = CreateTestSurface();
    auto invalid = wgpu::TextureFormat::R32Uint;

    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
    config.viewFormatCount = 1;
    config.viewFormats = &invalid;
    ASSERT_DEVICE_ERROR(surface.Configure(&config));
}

// View format is valid when it matches the config format
TEST_P(SurfaceConfigurationValidationTests, ValidViewFormat) {
    wgpu::Surface surface = CreateTestSurface();

    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
    config.viewFormatCount = 1;
    config.viewFormats = &config.format;
    surface.Configure(&config);

    // TODO(dawn:2320) Also test the equivalent (non-)sRGB view format
}

// A width of 0 fails
TEST_P(SurfaceConfigurationValidationTests, ZeroWidth) {
    wgpu::Surface surface = CreateTestSurface();

    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
    config.width = 0;
    ASSERT_DEVICE_ERROR(surface.Configure(&config));
}

// A height of 0 fails
TEST_P(SurfaceConfigurationValidationTests, ZeroHeight) {
    wgpu::Surface surface = CreateTestSurface();

    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
    config.height = 0;
    ASSERT_DEVICE_ERROR(surface.Configure(&config));
}

// A width that exceeds the maximum texture size fails
TEST_P(SurfaceConfigurationValidationTests, ExcessiveWidth) {
    wgpu::Surface surface = CreateTestSurface();
    wgpu::SupportedLimits supported;
    device.GetLimits(&supported);

    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
    config.width = supported.limits.maxTextureDimension1D + 1;
    ASSERT_DEVICE_ERROR(surface.Configure(&config));
}

// A height that exceeds the maximum texture size fails
TEST_P(SurfaceConfigurationValidationTests, ExcessiveHeight) {
    wgpu::Surface surface = CreateTestSurface();
    wgpu::SupportedLimits supported;
    device.GetLimits(&supported);

    wgpu::SurfaceConfiguration config = GetPreferredConfiguration(surface);
    config.height = supported.limits.maxTextureDimension2D + 1;
    ASSERT_DEVICE_ERROR(surface.Configure(&config));
}

// A surface that was not configured must not be unconfigured
TEST_P(SurfaceConfigurationValidationTests, UnconfigureNonConfiguredSurfaceFails) {
    // TODO(dawn:2320): With SwiftShader, this throws a device error anyways (maybe because
    // mInstance->ConsumedError calls the device error callback?). We should have a
    // ASSERT_INSTANCE_ERROR to fully fix this test case.
    DAWN_SUPPRESS_TEST_IF(IsSwiftshader());

    // TODO(dawn:2320) This cannot throw a device error since the surface is
    // not aware of the device at this stage.
    /*ASSERT_DEVICE_ERROR(*/ CreateTestSurface().Unconfigure() /*)*/;
}

DAWN_INSTANTIATE_TEST(SurfaceConfigurationValidationTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
