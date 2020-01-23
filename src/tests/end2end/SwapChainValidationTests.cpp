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

#include "tests/unittests/validation/ValidationTest.h"

#include "common/Constants.h"
#include "common/Log.h"
#include "utils/GLFWUtils.h"
#include "utils/WGPUHelpers.h"

#include "GLFW/glfw3.h"

class SwapChainValidationTests : public ValidationTest {
  public:
    void SetUp() override {
        glfwSetErrorCallback([](int code, const char* message) {
            dawn::ErrorLog() << "GLFW error " << code << " " << message;
        });
        glfwInit();

        // The SwapChainValidationTests tests don't create devices so we don't need to call
        // SetupGLFWWindowHintsForBackend. Set GLFW_NO_API anyway to avoid GLFW bringing up a GL
        // context that we won't use.
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(400, 400, "SwapChainValidationTests window", nullptr, nullptr);

        surface = utils::CreateSurfaceForWindow(instance->Get(), window);
        ASSERT_NE(surface, nullptr);

        goodDescriptor.width = 1;
        goodDescriptor.height = 1;
        goodDescriptor.usage = wgpu::TextureUsage::OutputAttachment;
        goodDescriptor.format = wgpu::TextureFormat::BGRA8Unorm;
        goodDescriptor.presentMode = wgpu::PresentMode::VSync;

        badDescriptor = goodDescriptor;
        badDescriptor.width = 0;
    }

    void TearDown() override {
        // Destroy the surface before the window as required by webgpu-native.
        surface = wgpu::Surface();
        glfwDestroyWindow(window);
    }

  protected:
    GLFWwindow* window = nullptr;
    wgpu::Surface surface;
    wgpu::SwapChainDescriptor goodDescriptor;
    wgpu::SwapChainDescriptor badDescriptor;

    // Checks that an OutputAttachment view is an error by trying to create a render pass on it.
    void CheckTextureViewIsError(wgpu::TextureView view) {
        utils::ComboRenderPassDescriptor renderPassDesc({view});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
};

// Control case for a successful swapchain creation.
TEST_F(SwapChainValidationTests, CreationSuccess) {
    device.CreateSwapChain(surface, &goodDescriptor);
}

// Checks that the creation size must be a valid 2D texture size.
TEST_F(SwapChainValidationTests, InvalidCreationSize) {
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

    // A width of kMaxTextureSize is valid but kMaxTextureSize + 1 isn't.
    {
        wgpu::SwapChainDescriptor desc = goodDescriptor;
        desc.width = kMaxTextureSize;
        device.CreateSwapChain(surface, &desc);

        desc.width = kMaxTextureSize + 1;
        ASSERT_DEVICE_ERROR(device.CreateSwapChain(surface, &desc));
    }

    // A height of kMaxTextureSize is valid but kMaxTextureSize + 1 isn't.
    {
        wgpu::SwapChainDescriptor desc = goodDescriptor;
        desc.height = kMaxTextureSize;
        device.CreateSwapChain(surface, &desc);

        desc.height = kMaxTextureSize + 1;
        ASSERT_DEVICE_ERROR(device.CreateSwapChain(surface, &desc));
    }
}

// Checks that the creation usage must be OutputAttachment
TEST_F(SwapChainValidationTests, InvalidCreationUsage) {
    wgpu::SwapChainDescriptor desc = goodDescriptor;
    desc.usage = wgpu::TextureUsage::Sampled;
    ASSERT_DEVICE_ERROR(device.CreateSwapChain(surface, &desc));
}

// Checks that the creation format must (currently) be BGRA8Unorm
TEST_F(SwapChainValidationTests, InvalidCreationFormat) {
    wgpu::SwapChainDescriptor desc = goodDescriptor;
    desc.format = wgpu::TextureFormat::RGBA8Unorm;
    ASSERT_DEVICE_ERROR(device.CreateSwapChain(surface, &desc));
}

// Checks that the implementation must be zero.
TEST_F(SwapChainValidationTests, InvalidWithImplementation) {
    wgpu::SwapChainDescriptor desc = goodDescriptor;
    desc.implementation = 1;
    ASSERT_DEVICE_ERROR(device.CreateSwapChain(surface, &desc));
}

// Check swapchain operations with an error swapchain are errors
TEST_F(SwapChainValidationTests, OperationsOnErrorSwapChain) {
    wgpu::SwapChain swapchain;
    ASSERT_DEVICE_ERROR(swapchain = device.CreateSwapChain(surface, &badDescriptor));

    wgpu::TextureView view;
    ASSERT_DEVICE_ERROR(view = swapchain.GetCurrentTextureView());
    CheckTextureViewIsError(view);

    ASSERT_DEVICE_ERROR(swapchain.Present());
}
