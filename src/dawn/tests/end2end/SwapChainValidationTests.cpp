// Copyright 2020 The Dawn Authors
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

#include "dawn/tests/DawnTest.h"

#include "dawn/common/Constants.h"
#include "dawn/common/Log.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/GLFWUtils.h"
#include "dawn/utils/WGPUHelpers.h"

#include "GLFW/glfw3.h"

class SwapChainValidationTests : public DawnTest {
  public:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
        DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

        glfwSetErrorCallback([](int code, const char* message) {
            dawn::ErrorLog() << "GLFW error " << code << " " << message;
        });
        DAWN_TEST_UNSUPPORTED_IF(!glfwInit());

        // The SwapChainValidationTests don't create devices so we don't need to call
        // SetupGLFWWindowHintsForBackend. Set GLFW_NO_API anyway to avoid GLFW bringing up a GL
        // context that we won't use.
        ASSERT_TRUE(!IsOpenGL());
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(400, 400, "SwapChainValidationTests window", nullptr, nullptr);

        surface = utils::CreateSurfaceForWindow(GetInstance(), window);
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
    void CheckTextureViewIsError(wgpu::TextureView view) {
        CheckTextureView(view, true, false);
    }

    // Checks that a RenderAttachment view is an error by trying to submit a render pass on it.
    void CheckTextureViewIsDestroyed(wgpu::TextureView view) {
        CheckTextureView(view, false, true);
    }

    // Checks that a RenderAttachment view is valid by submitting a render pass on it.
    void CheckTextureViewIsValid(wgpu::TextureView view) {
        CheckTextureView(view, false, false);
    }

  private:
    void CheckTextureView(wgpu::TextureView view, bool errorAtFinish, bool errorAtSubmit) {
        utils::ComboRenderPassDescriptor renderPassDesc({view});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
        pass.End();

        if (errorAtFinish) {
            ASSERT_DEVICE_ERROR(encoder.Finish());
        } else {
            wgpu::CommandBuffer commands = encoder.Finish();

            if (errorAtSubmit) {
                ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
            } else {
                queue.Submit(1, &commands);
            }
        }
    }
};

// Control case for a successful swapchain creation and presenting.
TEST_P(SwapChainValidationTests, CreationSuccess) {
    wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &goodDescriptor);
    wgpu::TextureView view = swapchain.GetCurrentTextureView();
    swapchain.Present();
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

// Checks that the implementation must be zero.
TEST_P(SwapChainValidationTests, InvalidWithImplementation) {
    wgpu::SwapChainDescriptor desc = goodDescriptor;
    desc.implementation = 1;
    ASSERT_DEVICE_ERROR(device.CreateSwapChain(surface, &desc));
}

// Check swapchain operations with an error swapchain are errors
TEST_P(SwapChainValidationTests, OperationsOnErrorSwapChain) {
    wgpu::SwapChain swapchain;
    ASSERT_DEVICE_ERROR(swapchain = device.CreateSwapChain(surface, &badDescriptor));

    wgpu::TextureView view;
    ASSERT_DEVICE_ERROR(view = swapchain.GetCurrentTextureView());
    CheckTextureViewIsError(view);

    ASSERT_DEVICE_ERROR(swapchain.Present());
}

// Check it is invalid to call present without getting a current view.
TEST_P(SwapChainValidationTests, PresentWithoutCurrentView) {
    wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &goodDescriptor);

    // Check it is invalid if we never called GetCurrentTextureView
    ASSERT_DEVICE_ERROR(swapchain.Present());

    // Check it is invalid if we never called since the last present.
    swapchain.GetCurrentTextureView();
    swapchain.Present();
    ASSERT_DEVICE_ERROR(swapchain.Present());
}

// Check that the current view isn't destroyed when the ref to the swapchain is lost because the
// swapchain is kept alive by the surface. Also check after we lose all refs to the surface, the
// texture is destroyed.
TEST_P(SwapChainValidationTests, ViewValidAfterSwapChainRefLost) {
    wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &goodDescriptor);
    wgpu::TextureView view = swapchain.GetCurrentTextureView();

    swapchain = nullptr;
    CheckTextureViewIsValid(view);

    surface = nullptr;
    CheckTextureViewIsDestroyed(view);
}

// Check that the current view is the destroyed state after present.
TEST_P(SwapChainValidationTests, ViewDestroyedAfterPresent) {
    wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &goodDescriptor);
    wgpu::TextureView view = swapchain.GetCurrentTextureView();
    swapchain.Present();

    CheckTextureViewIsDestroyed(view);
}

// Check that returned view is of the current format / usage / dimension / size / sample count
TEST_P(SwapChainValidationTests, ReturnedViewCharacteristics) {
    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = utils::CreateShaderModule(device, R"(
        @stage(vertex) fn main() -> @builtin(position) vec4<f32> {
            return vec4<f32>(0.0, 0.0, 0.0, 1.0);
        })");
    pipelineDesc.cFragment.module = utils::CreateShaderModule(device, R"(
        struct FragmentOut {
            @location(0) target0 : vec4<f32>,
            @location(1) target1 : f32,
        }
        @stage(fragment) fn main() -> FragmentOut {
            var out : FragmentOut;
            out.target0 = vec4<f32>(0.0, 1.0, 0.0, 1.0);
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
    wgpu::TextureView view = swapchain.GetCurrentTextureView();

    // Validation will also check the dimension of the view is 2D, and it's usage contains
    // RenderAttachment
    utils::ComboRenderPassDescriptor renderPassDesc({view, secondTexture.CreateView()});
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();

    queue.Submit(1, &commands);

    // Check that view doesn't have extra formats like Sampled.
    // TODO(cwallez@chromium.org): also check for [Readonly]Storage once that's implemented.
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}});
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl, {{0, view}}));
}

// Check that failing to create a new swapchain doesn't replace the previous one.
TEST_P(SwapChainValidationTests, ErrorSwapChainDoesntReplacePreviousOne) {
    wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &goodDescriptor);
    ASSERT_DEVICE_ERROR(device.CreateSwapChain(surface, &badDescriptor));

    wgpu::TextureView view = swapchain.GetCurrentTextureView();
    swapchain.Present();
}

// Check that after replacement, all swapchain operations are errors and the view is destroyed.
TEST_P(SwapChainValidationTests, ReplacedSwapChainIsInvalid) {
    {
        wgpu::SwapChain replacedSwapChain = device.CreateSwapChain(surface, &goodDescriptor);
        device.CreateSwapChain(surface, &goodDescriptor);
        ASSERT_DEVICE_ERROR(replacedSwapChain.GetCurrentTextureView());
    }

    {
        wgpu::SwapChain replacedSwapChain = device.CreateSwapChain(surface, &goodDescriptor);
        wgpu::TextureView view = replacedSwapChain.GetCurrentTextureView();
        device.CreateSwapChain(surface, &goodDescriptor);

        CheckTextureViewIsDestroyed(view);
        ASSERT_DEVICE_ERROR(replacedSwapChain.Present());
    }
}

// Check that after surface destruction, all swapchain operations are errors and the view is
// destroyed. The test is split in two to reset the wgpu::Surface in the middle.
TEST_P(SwapChainValidationTests, SwapChainIsInvalidAfterSurfaceDestruction_GetView) {
    wgpu::SwapChain replacedSwapChain = device.CreateSwapChain(surface, &goodDescriptor);
    surface = nullptr;
    ASSERT_DEVICE_ERROR(replacedSwapChain.GetCurrentTextureView());
}
TEST_P(SwapChainValidationTests, SwapChainIsInvalidAfterSurfaceDestruction_AfterGetView) {
    wgpu::SwapChain replacedSwapChain = device.CreateSwapChain(surface, &goodDescriptor);
    wgpu::TextureView view = replacedSwapChain.GetCurrentTextureView();
    surface = nullptr;

    CheckTextureViewIsDestroyed(view);
    ASSERT_DEVICE_ERROR(replacedSwapChain.Present());
}

// Test that new swap chain present fails after device is lost
TEST_P(SwapChainValidationTests, NewSwapChainPresentFailsAfterDeviceLost) {
    wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &goodDescriptor);
    wgpu::TextureView view = swapchain.GetCurrentTextureView();

    LoseDeviceForTesting();
    ASSERT_DEVICE_ERROR(swapchain.Present());
}

// Test that new swap chain get current texture view fails after device is lost
TEST_P(SwapChainValidationTests, NewSwapChainGetCurrentTextureViewFailsAfterDevLost) {
    wgpu::SwapChain swapchain = device.CreateSwapChain(surface, &goodDescriptor);

    LoseDeviceForTesting();
    ASSERT_DEVICE_ERROR(swapchain.GetCurrentTextureView());
}

// Test that creation of a new swapchain fails after device is lost
TEST_P(SwapChainValidationTests, CreateNewSwapChainFailsAfterDevLost) {
    LoseDeviceForTesting();
    ASSERT_DEVICE_ERROR(device.CreateSwapChain(surface, &goodDescriptor));
}

DAWN_INSTANTIATE_TEST(SwapChainValidationTests, MetalBackend(), NullBackend());
