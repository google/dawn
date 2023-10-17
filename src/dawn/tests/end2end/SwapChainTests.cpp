// Copyright 2020 The Dawn & Tint Authors
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

#include "dawn/common/Constants.h"
#include "dawn/common/Log.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"
#include "webgpu/webgpu_glfw.h"

#include "GLFW/glfw3.h"

namespace dawn {
namespace {

class SwapChainTests : public DawnTest {
  public:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());

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
        window = glfwCreateWindow(400, 400, "SwapChainValidationTests window", nullptr, nullptr);

        int width;
        int height;
        glfwGetFramebufferSize(window, &width, &height);

        surface = wgpu::glfw::CreateSurfaceForWindow(GetInstance(), window);
        ASSERT_NE(surface, nullptr);

        baseDescriptor.width = width;
        baseDescriptor.height = height;
        baseDescriptor.usage = wgpu::TextureUsage::RenderAttachment;
        baseDescriptor.format = wgpu::TextureFormat::BGRA8Unorm;
        baseDescriptor.presentMode = wgpu::PresentMode::Mailbox;
    }

    void TearDown() override {
        // Destroy the surface before the window as required by webgpu-native.
        surface = wgpu::Surface();
        if (window != nullptr) {
            glfwDestroyWindow(window);
        }
        DawnTest::TearDown();
    }

    void ClearTexture(wgpu::Texture texture, wgpu::Color color) {
        utils::ComboRenderPassDescriptor desc({texture.CreateView()});
        desc.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
        desc.cColorAttachments[0].clearValue = color;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&desc);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

  protected:
    GLFWwindow* window = nullptr;
    wgpu::Surface surface;

    wgpu::SwapChainDescriptor baseDescriptor;
};

// Basic test for creating a swapchain and presenting one frame.
TEST_P(SwapChainTests, Basic) {
    wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &baseDescriptor);
    ClearTexture(swapchain.GetCurrentTexture(), {1.0, 0.0, 0.0, 1.0});
    swapchain.Present();
}

// Test replacing the swapchain
TEST_P(SwapChainTests, ReplaceBasic) {
    wgpu::SwapChain swapchain1 = device.CreateSwapChain(surface, &baseDescriptor);
    ClearTexture(swapchain1.GetCurrentTexture(), {1.0, 0.0, 0.0, 1.0});
    swapchain1.Present();

    wgpu::SwapChain swapchain2 = device.CreateSwapChain(surface, &baseDescriptor);
    ClearTexture(swapchain2.GetCurrentTexture(), {0.0, 1.0, 0.0, 1.0});
    swapchain2.Present();
}

// Test replacing the swapchain after GetCurrentTexture
TEST_P(SwapChainTests, ReplaceAfterGet) {
    wgpu::SwapChain swapchain1 = device.CreateSwapChain(surface, &baseDescriptor);
    ClearTexture(swapchain1.GetCurrentTexture(), {1.0, 0.0, 0.0, 1.0});

    wgpu::SwapChain swapchain2 = device.CreateSwapChain(surface, &baseDescriptor);
    ClearTexture(swapchain2.GetCurrentTexture(), {0.0, 1.0, 0.0, 1.0});
    swapchain2.Present();
}

// Test destroying the swapchain after GetCurrentTexture
TEST_P(SwapChainTests, DestroyAfterGet) {
    wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &baseDescriptor);
    ClearTexture(swapchain.GetCurrentTexture(), {1.0, 0.0, 0.0, 1.0});
}

// Test destroying the surface before the swapchain
TEST_P(SwapChainTests, DestroySurface) {
    wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &baseDescriptor);
    surface = nullptr;
}

// Test destroying the surface before the swapchain but after GetCurrentTexture
TEST_P(SwapChainTests, DestroySurfaceAfterGet) {
    wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &baseDescriptor);
    ClearTexture(swapchain.GetCurrentTexture(), {1.0, 0.0, 0.0, 1.0});
    surface = nullptr;
}

// Test switching between present modes.
TEST_P(SwapChainTests, SwitchPresentMode) {
    // Fails with "internal drawable creation failed" on the Windows NVIDIA CQ builders but not
    // locally.
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsVulkan() && IsNvidia());

    // TODO(jiawei.shao@intel.com): find out why this test sometimes hangs on the latest Linux Intel
    // Vulkan drivers.
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsVulkan() && IsIntel());

    constexpr wgpu::PresentMode kAllPresentModes[] = {
        wgpu::PresentMode::Immediate,
        wgpu::PresentMode::Fifo,
        wgpu::PresentMode::Mailbox,
    };

    for (wgpu::PresentMode mode1 : kAllPresentModes) {
        for (wgpu::PresentMode mode2 : kAllPresentModes) {
            wgpu::SwapChainDescriptor desc = baseDescriptor;

            desc.presentMode = mode1;
            wgpu::SwapChain swapchain1 = device.CreateSwapChain(surface, &desc);
            ClearTexture(swapchain1.GetCurrentTexture(), {0.0, 0.0, 0.0, 1.0});
            swapchain1.Present();

            desc.presentMode = mode2;
            wgpu::SwapChain swapchain2 = device.CreateSwapChain(surface, &desc);
            ClearTexture(swapchain2.GetCurrentTexture(), {0.0, 0.0, 0.0, 1.0});
            swapchain2.Present();
        }
    }
}

// Test resizing the swapchain and without resizing the window.
TEST_P(SwapChainTests, ResizingSwapChainOnly) {
    for (int i = 0; i < 10; i++) {
        wgpu::SwapChainDescriptor desc = baseDescriptor;
        desc.width += i * 10;
        desc.height -= i * 10;

        wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &desc);
        ClearTexture(swapchain.GetCurrentTexture(), {0.05f * i, 0.0f, 0.0f, 1.0f});
        swapchain.Present();
    }
}

// Test resizing the window but not the swapchain.
TEST_P(SwapChainTests, ResizingWindowOnly) {
    wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &baseDescriptor);

    for (int i = 0; i < 10; i++) {
        glfwSetWindowSize(window, 400 - 10 * i, 400 + 10 * i);
        glfwPollEvents();

        ClearTexture(swapchain.GetCurrentTexture(), {0.05f * i, 0.0f, 0.0f, 1.0f});
        swapchain.Present();
    }
}

// Test resizing both the window and the swapchain at the same time.
TEST_P(SwapChainTests, ResizingWindowAndSwapChain) {
    // TODO(crbug.com/dawn/1205) Currently failing on new NVIDIA GTX 1660s on Linux/Vulkan.
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsVulkan() && IsNvidia());
    for (int i = 0; i < 10; i++) {
        glfwSetWindowSize(window, 400 - 10 * i, 400 + 10 * i);
        glfwPollEvents();

        int width;
        int height;
        glfwGetFramebufferSize(window, &width, &height);

        wgpu::SwapChainDescriptor desc = baseDescriptor;
        desc.width = width;
        desc.height = height;

        wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &desc);
        ClearTexture(swapchain.GetCurrentTexture(), {0.05f * i, 0.0f, 0.0f, 1.0f});
        swapchain.Present();
    }
}

// Test switching devices on the same adapter.
TEST_P(SwapChainTests, SwitchingDevice) {
    // TODO(https://crbug.com/dawn/2116): Disabled due to new Validation Layer failures.
    DAWN_SUPPRESS_TEST_IF(IsVulkan());

    wgpu::Device device2 = CreateDevice();

    for (int i = 0; i < 3; i++) {
        wgpu::Device deviceToUse;
        if (i % 2 == 0) {
            deviceToUse = device;
        } else {
            deviceToUse = device2;
        }

        wgpu::SwapChain swapchain = deviceToUse.CreateSwapChain(surface, &baseDescriptor);
        swapchain.GetCurrentTexture();
        swapchain.Present();
    }
}

// Test that calling Device.GetSupportedSurfaceUsage() will throw an error because
// SurfaceCapabilities is not enabled.
TEST_P(SwapChainTests, ErrorGetSurfaceSupportedUsage) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));
    EXPECT_FALSE(device.HasFeature(wgpu::FeatureName::SurfaceCapabilities));

    ASSERT_DEVICE_ERROR_MSG(
        {
            auto usageFlags = device.GetSupportedSurfaceUsage(surface);
            EXPECT_EQ(usageFlags, wgpu::TextureUsage::None);
        },
        testing::HasSubstr("FeatureName::SurfaceCapabilities is not enabled"));
}

// Test that creating swapchain with TextureBinding usage without enabling SurfaceCapabilities
// feature should fail.
TEST_P(SwapChainTests, ErrorCreateWithTextureBindingUsage) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));
    EXPECT_FALSE(device.HasFeature(wgpu::FeatureName::SurfaceCapabilities));

    auto desc = baseDescriptor;
    desc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment;

    ASSERT_DEVICE_ERROR_MSG(
        { auto swapchain = device.CreateSwapChain(surface, &desc); },
        testing::HasSubstr("require enabling FeatureName::SurfaceCapabilities"));
}

class SwapChainWithAdditionalUsageTests : public SwapChainTests {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> features;
        if (!UsesWire() && SupportsFeatures({wgpu::FeatureName::SurfaceCapabilities})) {
            features.push_back(wgpu::FeatureName::SurfaceCapabilities);
        }
        return features;
    }

    void SetUp() override {
        SwapChainTests::SetUp();

        // If parent class skipped the test, we should skip as well.
        if (surface == nullptr) {
            GTEST_SKIP();
        }

        DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::SurfaceCapabilities}));
    }

    void SampleTexture(wgpu::Texture texture, utils::RGBA8 expectedColor) {
        wgpu::TextureDescriptor texDescriptor;
        texDescriptor.size = {texture.GetWidth(), texture.GetHeight(), 1};
        texDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        texDescriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc |
                              wgpu::TextureUsage::CopyDst;
        texDescriptor.mipLevelCount = 1;
        texDescriptor.sampleCount = 1;

        wgpu::Texture dstTexture = device.CreateTexture(&texDescriptor);
        wgpu::TextureView dstView = dstTexture.CreateView();

        // Create a render pipeline to blit |view| into |dstView|.
        utils::ComboRenderPipelineDescriptor pipelineDesc;
        pipelineDesc.vertex.module = utils::CreateShaderModule(device, R"(
            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var pos = array(
                                            vec2f(-1.0, -1.0),
                                            vec2f(-1.0,  1.0),
                                            vec2f( 1.0, -1.0),
                                            vec2f(-1.0,  1.0),
                                            vec2f( 1.0, -1.0),
                                            vec2f( 1.0,  1.0));
                return vec4f(pos[VertexIndex], 0.0, 1.0);
            }
        )");
        pipelineDesc.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var texture : texture_2d<f32>;

            @fragment
            fn main(@builtin(position) coord: vec4f) -> @location(0) vec4f {
                return textureLoad(texture, vec2i(coord.xy), 0);
            }
        )");
        pipelineDesc.cTargets[0].format = texDescriptor.format;

        // Submit a render pass to perform the blit from |view| to |dstView|.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

            wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                             {{0, texture.CreateView()}});

            utils::ComboRenderPassDescriptor renderPassInfo({dstView});

            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(6);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_TEXTURE_EQ(expectedColor, dstTexture, {0, 0});
        EXPECT_TEXTURE_EQ(expectedColor, dstTexture,
                          {texture.GetWidth() - 1, texture.GetHeight() - 1});
    }

    void WriteTexture(wgpu::Texture texture, const utils::RGBA8& data) {
        wgpu::Extent3D writeSize = {1, 1, 1};
        wgpu::ImageCopyTexture dest = {};
        dest.texture = texture;
        wgpu::TextureDataLayout dataLayout = {};
        queue.WriteTexture(&dest, &data, sizeof(utils::RGBA8), &dataLayout, &writeSize);
    }
};

TEST_P(SwapChainWithAdditionalUsageTests, GetSurfaceSupportedUsage) {
    auto usageFlags = device.GetSupportedSurfaceUsage(surface);
    EXPECT_NE(usageFlags, wgpu::TextureUsage::None);
}

// Test that sampling from swapchain is supported.
TEST_P(SwapChainWithAdditionalUsageTests, SamplingFromSwapChain) {
    // Skip all tests if readable surface doesn't support texture binding
    DAWN_TEST_UNSUPPORTED_IF(
        !(device.GetSupportedSurfaceUsage(surface) & wgpu::TextureUsage::TextureBinding));

    auto desc = baseDescriptor;
    desc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment;

    wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &desc);
    ClearTexture(swapchain.GetCurrentTexture(), {1.0, 0.0, 0.0, 1.0});

    SampleTexture(swapchain.GetCurrentTexture(), utils::RGBA8::kRed);

    swapchain.Present();
}

// Test that including unsupported usage flag will result in error.
TEST_P(SwapChainWithAdditionalUsageTests, ErrorIncludeUnsupportedUsage) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    auto supportedUsage = device.GetSupportedSurfaceUsage(surface);

    // Assuming StorageBinding is not supported.
    DAWN_TEST_UNSUPPORTED_IF(supportedUsage & wgpu::TextureUsage::StorageBinding);

    auto desc = baseDescriptor;
    desc.usage = supportedUsage | wgpu::TextureUsage::StorageBinding;

    ASSERT_DEVICE_ERROR_MSG({ auto swapchain = device.CreateSwapChain(surface, &desc); },
                            testing::HasSubstr("is not supported"));
}

// Test copying to a swapchain texture when it is supported.
TEST_P(SwapChainWithAdditionalUsageTests, CopyingToSwapChain) {
    wgpu::TextureUsage supportedUsages = device.GetSupportedSurfaceUsage(surface);
    // We need the swapchain to support copying to the texture and at least one readback method.
    DAWN_TEST_UNSUPPORTED_IF(!(supportedUsages & wgpu::TextureUsage::CopyDst));
    DAWN_TEST_UNSUPPORTED_IF(
        !(supportedUsages & (wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding)));

    wgpu::SwapChainDescriptor desc = baseDescriptor;
    desc.usage |= supportedUsages;
    desc.width = 1;
    desc.height = 1;

    wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &desc);
    wgpu::Texture texture = swapchain.GetCurrentTexture();
    WriteTexture(texture, utils::RGBA8::kRed);

    if (supportedUsages & wgpu::TextureUsage::CopySrc) {
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, swapchain.GetCurrentTexture(), 0, 0);
    } else {
        // kBlue because the texture is actually BGRA
        SampleTexture(texture, utils::RGBA8::kBlue);
    }
}

// Test copying from a swapchain texture when it is supported.
TEST_P(SwapChainWithAdditionalUsageTests, CopyingFromSwapChain) {
    // We need the swapchain to support copying from the texture
    DAWN_TEST_UNSUPPORTED_IF(
        !(device.GetSupportedSurfaceUsage(surface) & wgpu::TextureUsage::CopySrc));

    wgpu::SwapChainDescriptor desc = baseDescriptor;
    desc.usage |= wgpu::TextureUsage::CopySrc;

    wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &desc);
    wgpu::Texture texture = swapchain.GetCurrentTexture();

    ClearTexture(swapchain.GetCurrentTexture(), {1.0, 0.0, 0.0, 1.0});
    // kBlue because the texture is actually BGRA8
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kBlue, swapchain.GetCurrentTexture(), 0, 0);
}

DAWN_INSTANTIATE_TEST(SwapChainTests, MetalBackend(), VulkanBackend());
DAWN_INSTANTIATE_TEST(SwapChainWithAdditionalUsageTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
