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

#include "dawn/tests/DawnTest.h"

#include "dawn/common/Constants.h"
#include "dawn/common/Log.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"
#include "webgpu/webgpu_glfw.h"

#include "GLFW/glfw3.h"

namespace dawn {
namespace {

class SwapChainValidationTests : public DawnTest {
  public:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
        DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

        glfwSetErrorCallback([](int code, const char* message) {
            ErrorLog() << "GLFW error " << code << " " << message;
        });
        DAWN_TEST_UNSUPPORTED_IF(!glfwInit());

        // Set GLFW_NO_API to avoid GLFW bringing up a GL context that we won't use.
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(400, 400, "SwapChainValidationTests window", nullptr, nullptr);

        surface = wgpu::glfw::CreateSurfaceForWindow(GetInstance(), window);
        ASSERT_NE(surface, nullptr);

        goodDescriptor.width = 1;
        goodDescriptor.height = 1;
        goodDescriptor.usage = wgpu::TextureUsage::RenderAttachment;
        goodDescriptor.format = wgpu::TextureFormat::BGRA8Unorm;
        goodDescriptor.presentMode = wgpu::PresentMode::Mailbox;

        badDescriptor = goodDescriptor;
        badDescriptor.width = 0;
    }

    void TearDown() override {
        // Destroy the surface before the window as required by webgpu-native.
        surface = wgpu::Surface();
        if (window != nullptr) {
            glfwDestroyWindow(window);
        }
        DawnTest::TearDown();
    }

  protected:
    GLFWwindow* window = nullptr;
    wgpu::Surface surface;
    wgpu::SwapChainDescriptor goodDescriptor;
    wgpu::SwapChainDescriptor badDescriptor;

    // Checks that a RenderAttachment view is an error by trying to create a render pass on it.
    void CheckTextureIsError(wgpu::Texture texture) { CheckTexture(texture, true, false); }

    // Checks that a RenderAttachment view is an error by trying to submit a render pass on it.
    void CheckTextureIsDestroyed(wgpu::Texture texture) { CheckTexture(texture, false, true); }

    // Checks that a RenderAttachment view is valid by submitting a render pass on it.
    void CheckTextureIsValid(wgpu::Texture texture) { CheckTexture(texture, false, false); }

  private:
    void CheckTexture(wgpu::Texture texture, bool error, bool destroyed) {
        wgpu::TextureView view;
        if (error) {
            ASSERT_DEVICE_ERROR(view = texture.CreateView());
        } else {
            view = texture.CreateView();
        }
        utils::ComboRenderPassDescriptor renderPassDesc({view});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
        pass.End();

        if (error) {
            ASSERT_DEVICE_ERROR(encoder.Finish());
        } else {
            wgpu::CommandBuffer commands = encoder.Finish();

            if (destroyed) {
                ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
            } else {
                queue.Submit(1, &commands);
            }
        }
    }
};

void CheckTextureMatchesDescriptor(const wgpu::Texture& tex,
                                   const wgpu::SwapChainDescriptor& desc) {
    EXPECT_EQ(desc.width, tex.GetWidth());
    EXPECT_EQ(desc.height, tex.GetHeight());
    EXPECT_EQ(desc.usage, tex.GetUsage());
    EXPECT_EQ(desc.format, tex.GetFormat());
    EXPECT_EQ(1u, tex.GetDepthOrArrayLayers());
    EXPECT_EQ(1u, tex.GetMipLevelCount());
    EXPECT_EQ(1u, tex.GetSampleCount());
    EXPECT_EQ(wgpu::TextureDimension::e2D, tex.GetDimension());
}

// Control case for a successful swapchain creation and presenting.
TEST_P(SwapChainValidationTests, CreationSuccess) {
    wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &goodDescriptor);
    swapchain.GetCurrentTexture();
    swapchain.Present();
}

// Test that creating a swapchain with an invalid surface is an error.
TEST_P(SwapChainValidationTests, InvalidSurface) {
    wgpu::SurfaceDescriptor surface_desc = {};
    wgpu::Surface surface = GetInstance().CreateSurface(&surface_desc);

    ASSERT_DEVICE_ERROR_MSG(device.CreateSwapChain(surface, &goodDescriptor),
                            testing::HasSubstr("[Surface] is invalid"));
}

// Checks that the creation size must be a valid 2D texture size.
TEST_P(SwapChainValidationTests, InvalidCreationSize) {
    wgpu::Limits supportedLimits = GetSupportedLimits().limits;
    // A width of 0 is invalid.
    {
        wgpu::SwapChainDescriptor desc = goodDescriptor;
        desc.width = 0;
        ASSERT_DEVICE_ERROR(device.CreateSwapChain(surface, &desc));
    }
    // A height of 0 is invalid.
    {
        wgpu::SwapChainDescriptor desc = goodDescriptor;
        desc.height = 0;
        ASSERT_DEVICE_ERROR(device.CreateSwapChain(surface, &desc));
    }

    // A width of maxTextureDimension2D is valid but maxTextureDimension2D + 1 isn't.
    {
        wgpu::SwapChainDescriptor desc = goodDescriptor;
        desc.width = supportedLimits.maxTextureDimension2D;
        device.CreateSwapChain(surface, &desc);

        desc.width = supportedLimits.maxTextureDimension2D + 1;
        ASSERT_DEVICE_ERROR(device.CreateSwapChain(surface, &desc));
    }

    // A height of maxTextureDimension2D is valid but maxTextureDimension2D + 1 isn't.
    {
        wgpu::SwapChainDescriptor desc = goodDescriptor;
        desc.height = supportedLimits.maxTextureDimension2D;
        device.CreateSwapChain(surface, &desc);

        desc.height = supportedLimits.maxTextureDimension2D + 1;
        ASSERT_DEVICE_ERROR(device.CreateSwapChain(surface, &desc));
    }
}

// Checks that the creation usage must be RenderAttachment
TEST_P(SwapChainValidationTests, InvalidCreationUsage) {
    wgpu::SwapChainDescriptor desc = goodDescriptor;
    desc.usage = wgpu::TextureUsage::TextureBinding;
    ASSERT_DEVICE_ERROR(device.CreateSwapChain(surface, &desc));
}

// Checks that the creation format must (currently) be BGRA8Unorm
TEST_P(SwapChainValidationTests, InvalidCreationFormat) {
    wgpu::SwapChainDescriptor desc = goodDescriptor;
    desc.format = wgpu::TextureFormat::RGBA8Unorm;
    ASSERT_DEVICE_ERROR(device.CreateSwapChain(surface, &desc));
}

// Check swapchain operations with an error swapchain are errors
TEST_P(SwapChainValidationTests, OperationsOnErrorSwapChain) {
    wgpu::SwapChain swapchain;
    ASSERT_DEVICE_ERROR(swapchain = device.CreateSwapChain(surface, &badDescriptor));

    wgpu::Texture texture;
    ASSERT_DEVICE_ERROR(texture = swapchain.GetCurrentTexture());
    CheckTextureIsError(texture);

    ASSERT_DEVICE_ERROR(swapchain.Present());
}

// Check it is invalid to call present without getting a current texture.
TEST_P(SwapChainValidationTests, PresentWithoutCurrentTexture) {
    wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &goodDescriptor);

    // Check it is invalid if we never called GetCurrentTexture
    ASSERT_DEVICE_ERROR(swapchain.Present());

    // Check it is invalid if we never called since the last present.
    swapchain.GetCurrentTexture();
    swapchain.Present();
    ASSERT_DEVICE_ERROR(swapchain.Present());
}

// Check that the current texture isn't destroyed when the ref to the swapchain is lost because the
// swapchain is kept alive by the surface. Also check after we lose all refs to the surface, the
// texture is destroyed.
TEST_P(SwapChainValidationTests, TextureValidAfterSwapChainRefLost) {
    wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &goodDescriptor);
    wgpu::Texture texture = swapchain.GetCurrentTexture();

    swapchain = nullptr;
    CheckTextureIsValid(texture);

    surface = nullptr;
    CheckTextureIsDestroyed(texture);
}

// Check that the current texture is the destroyed state after present.
TEST_P(SwapChainValidationTests, TextureDestroyedAfterPresent) {
    wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &goodDescriptor);
    wgpu::Texture texture = swapchain.GetCurrentTexture();
    swapchain.Present();

    CheckTextureIsDestroyed(texture);
}

// Check that returned texture is of the current format / usage / dimension / size / sample count
TEST_P(SwapChainValidationTests, ReturnedTextureCharacteristics) {
    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");
    pipelineDesc.cFragment.module = utils::CreateShaderModule(device, R"(
        struct FragmentOut {
            @location(0) target0 : vec4f,
            @location(1) target1 : f32,
        }
        @fragment fn main() -> FragmentOut {
            var out : FragmentOut;
            out.target0 = vec4f(0.0, 1.0, 0.0, 1.0);
            out.target1 = 0.5;
            return out;
        })");
    // Validation will check that the sample count of the view matches this format.
    pipelineDesc.multisample.count = 1;
    pipelineDesc.cFragment.targetCount = 2;
    // Validation will check that the format of the view matches this format.
    pipelineDesc.cTargets[0].format = wgpu::TextureFormat::BGRA8Unorm;
    pipelineDesc.cTargets[1].format = wgpu::TextureFormat::R8Unorm;
    device.CreateRenderPipeline(&pipelineDesc);

    // Create a second texture to be used as render pass attachment. Validation will check that the
    // size of the view matches the size of this texture.
    wgpu::TextureDescriptor textureDesc;
    textureDesc.usage = wgpu::TextureUsage::RenderAttachment;
    textureDesc.dimension = wgpu::TextureDimension::e2D;
    textureDesc.size = {1, 1, 1};
    textureDesc.format = wgpu::TextureFormat::R8Unorm;
    textureDesc.sampleCount = 1;
    wgpu::Texture secondTexture = device.CreateTexture(&textureDesc);

    // Get the swapchain view and try to use it in the render pass to trigger all the validation.
    wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &goodDescriptor);
    wgpu::TextureView view = swapchain.GetCurrentTexture().CreateView();

    // Validation will also check the dimension of the view is 2D, and it's usage contains
    // RenderAttachment
    utils::ComboRenderPassDescriptor renderPassDesc({view, secondTexture.CreateView()});
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();

    queue.Submit(1, &commands);

    // Check that view doesn't have an extra Sampled usage.
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}});
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl, {{0, view}}));

    // Check that view doesn't have an extra Storage usage.
    bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::StorageTextureAccess::WriteOnly,
                  wgpu::TextureFormat::R32Uint}});
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl, {{0, view}}));
}

// Check the reflection of textures returned by GetCurrentTexture on valid swapchain.
TEST_P(SwapChainValidationTests, ReflectionValidGetCurrentTexture) {
    // Check with the goodDescriptor.
    {
        wgpu::SwapChain swapChain = device.CreateSwapChain(surface, &goodDescriptor);
        CheckTextureMatchesDescriptor(swapChain.GetCurrentTexture(), goodDescriptor);
    }
    // Check with properties that can be changed while keeping a valid descriptor.
    {
        wgpu::SwapChainDescriptor otherDescriptor = goodDescriptor;
        otherDescriptor.width = 2;
        otherDescriptor.height = 7;
        wgpu::SwapChain swapChain = device.CreateSwapChain(surface, &goodDescriptor);
        CheckTextureMatchesDescriptor(swapChain.GetCurrentTexture(), goodDescriptor);
    }
}

// Check the reflection of textures returned by GetCurrentTexture on valid swapchain.
TEST_P(SwapChainValidationTests, ReflectionErrorGetCurrentTexture) {
    wgpu::SwapChain swapChain;
    ASSERT_DEVICE_ERROR(swapChain = device.CreateSwapChain(surface, &badDescriptor));
    wgpu::Texture texture;
    ASSERT_DEVICE_ERROR(texture = swapChain.GetCurrentTexture());
    CheckTextureMatchesDescriptor(texture, badDescriptor);
}

// Check that failing to create a new swapchain doesn't replace the previous one.
TEST_P(SwapChainValidationTests, ErrorSwapChainDoesntReplacePreviousOne) {
    wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &goodDescriptor);
    ASSERT_DEVICE_ERROR(device.CreateSwapChain(surface, &badDescriptor));

    swapchain.GetCurrentTexture();
    swapchain.Present();
}

// Check that after replacement, all swapchain operations are errors and the texture is destroyed.
TEST_P(SwapChainValidationTests, ReplacedSwapChainIsInvalid) {
    {
        wgpu::SwapChain replacedSwapChain = device.CreateSwapChain(surface, &goodDescriptor);
        device.CreateSwapChain(surface, &goodDescriptor);
        ASSERT_DEVICE_ERROR(replacedSwapChain.GetCurrentTexture());
    }

    {
        wgpu::SwapChain replacedSwapChain = device.CreateSwapChain(surface, &goodDescriptor);
        wgpu::Texture texture = replacedSwapChain.GetCurrentTexture();
        device.CreateSwapChain(surface, &goodDescriptor);

        CheckTextureIsDestroyed(texture);
        ASSERT_DEVICE_ERROR(replacedSwapChain.Present());
    }
}

// Check that after surface destruction, all swapchain operations are errors and the texture is
// destroyed. The test is split in two to reset the wgpu::Surface in the middle.
TEST_P(SwapChainValidationTests, SwapChainIsInvalidAfterSurfaceDestruction_GetTexture) {
    wgpu::SwapChain replacedSwapChain = device.CreateSwapChain(surface, &goodDescriptor);
    surface = nullptr;
    ASSERT_DEVICE_ERROR(replacedSwapChain.GetCurrentTexture());
}
TEST_P(SwapChainValidationTests, SwapChainIsInvalidAfterSurfaceDestruction_AfterGetTexture) {
    wgpu::SwapChain replacedSwapChain = device.CreateSwapChain(surface, &goodDescriptor);
    wgpu::Texture texture = replacedSwapChain.GetCurrentTexture();
    surface = nullptr;

    CheckTextureIsDestroyed(texture);
    ASSERT_DEVICE_ERROR(replacedSwapChain.Present());
}

// Test that new swap chain present fails after device is lost
TEST_P(SwapChainValidationTests, SwapChainPresentFailsAfterDeviceLost) {
    wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &goodDescriptor);
    swapchain.GetCurrentTexture();

    LoseDeviceForTesting();
    ASSERT_DEVICE_ERROR(swapchain.Present());
}

// Test that new swap chain get current texture fails after device is lost
TEST_P(SwapChainValidationTests, SwapChainGetCurrentTextureFailsAfterDevLost) {
    wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &goodDescriptor);

    LoseDeviceForTesting();
    ASSERT_DEVICE_ERROR(swapchain.GetCurrentTexture());
}

// Test that creation of a new swapchain fails after device is lost
TEST_P(SwapChainValidationTests, CreateSwapChainFailsAfterDevLost) {
    LoseDeviceForTesting();
    ASSERT_DEVICE_ERROR(device.CreateSwapChain(surface, &goodDescriptor));
}

DAWN_INSTANTIATE_TEST(SwapChainValidationTests, MetalBackend(), NullBackend());

}  // anonymous namespace
}  // namespace dawn
