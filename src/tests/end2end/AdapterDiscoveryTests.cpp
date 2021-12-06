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

#include "common/GPUInfo.h"
#include "common/Platform.h"
#include "common/SystemUtils.h"
#include "dawn/webgpu_cpp.h"
#include "dawn_native/DawnNative.h"

#if defined(DAWN_ENABLE_BACKEND_VULKAN)
#    include "dawn_native/VulkanBackend.h"
#endif  // defined(DAWN_ENABLE_BACKEND_VULKAN)

#if defined(DAWN_ENABLE_BACKEND_D3D12)
#    include "dawn_native/D3D12Backend.h"
#endif  // defined(DAWN_ENABLE_BACKEND_D3D12)

#if defined(DAWN_ENABLE_BACKEND_METAL)
#    include "dawn_native/MetalBackend.h"
#endif  // defined(DAWN_ENABLE_BACKEND_METAL)

#if defined(DAWN_ENABLE_BACKEND_METAL)
#    include "dawn_native/MetalBackend.h"
#endif  // defined(DAWN_ENABLE_BACKEND_METAL)

#if defined(DAWN_ENABLE_BACKEND_DESKTOP_GL) || defined(DAWN_ENABLE_BACKEND_OPENGLES)
#    include "GLFW/glfw3.h"
#    include "dawn_native/OpenGLBackend.h"
#endif  // defined(DAWN_ENABLE_BACKEND_DESKTOP_GL) || defined(DAWN_ENABLE_BACKEND_OPENGLES)

#include <gtest/gtest.h>

namespace {

    class AdapterDiscoveryTests : public ::testing::Test {};

#if defined(DAWN_ENABLE_BACKEND_VULKAN)
    // Test only discovering the SwiftShader adapter
    TEST(AdapterDiscoveryTests, OnlySwiftShader) {
        dawn_native::Instance instance;

        dawn_native::vulkan::AdapterDiscoveryOptions options;
        options.forceSwiftShader = true;
        instance.DiscoverAdapters(&options);

        const auto& adapters = instance.GetAdapters();
        EXPECT_LE(adapters.size(), 1u);  // 0 or 1 SwiftShader adapters.
        for (const auto& adapter : adapters) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);

            EXPECT_EQ(properties.backendType, wgpu::BackendType::Vulkan);
            EXPECT_EQ(properties.adapterType, wgpu::AdapterType::CPU);
            EXPECT_TRUE(gpu_info::IsSwiftshader(properties.vendorID, properties.deviceID));
        }
    }

    // Test discovering only Vulkan adapters
    TEST(AdapterDiscoveryTests, OnlyVulkan) {
        dawn_native::Instance instance;

        dawn_native::vulkan::AdapterDiscoveryOptions options;
        instance.DiscoverAdapters(&options);

        const auto& adapters = instance.GetAdapters();
        for (const auto& adapter : adapters) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);

            EXPECT_EQ(properties.backendType, wgpu::BackendType::Vulkan);
        }
    }
#endif  // defined(DAWN_ENABLE_BACKEND_VULKAN)

#if defined(DAWN_ENABLE_BACKEND_D3D12)
    // Test discovering only D3D12 adapters
    TEST(AdapterDiscoveryTests, OnlyD3D12) {
        dawn_native::Instance instance;

        dawn_native::d3d12::AdapterDiscoveryOptions options;
        instance.DiscoverAdapters(&options);

        const auto& adapters = instance.GetAdapters();
        for (const auto& adapter : adapters) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);

            EXPECT_EQ(properties.backendType, wgpu::BackendType::D3D12);
        }
    }

    // Test discovering a D3D12 adapter from a prexisting DXGI adapter
    TEST(AdapterDiscoveryTests, MatchingDXGIAdapter) {
        using Microsoft::WRL::ComPtr;

        ComPtr<IDXGIFactory4> dxgiFactory;
        HRESULT hr = ::CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory));
        ASSERT_EQ(hr, S_OK);

        for (uint32_t adapterIndex = 0;; ++adapterIndex) {
            ComPtr<IDXGIAdapter1> dxgiAdapter = nullptr;
            if (dxgiFactory->EnumAdapters1(adapterIndex, &dxgiAdapter) == DXGI_ERROR_NOT_FOUND) {
                break;  // No more adapters to enumerate.
            }

            dawn_native::Instance instance;

            dawn_native::d3d12::AdapterDiscoveryOptions options;
            options.dxgiAdapter = std::move(dxgiAdapter);
            instance.DiscoverAdapters(&options);

            const auto& adapters = instance.GetAdapters();
            for (const auto& adapter : adapters) {
                wgpu::AdapterProperties properties;
                adapter.GetProperties(&properties);

                EXPECT_EQ(properties.backendType, wgpu::BackendType::D3D12);
            }
        }
    }
#endif  // defined(DAWN_ENABLE_BACKEND_D3D12)

#if defined(DAWN_ENABLE_BACKEND_METAL)
    // Test discovering only Metal adapters
    TEST(AdapterDiscoveryTests, OnlyMetal) {
        dawn_native::Instance instance;

        dawn_native::metal::AdapterDiscoveryOptions options;
        instance.DiscoverAdapters(&options);

        const auto& adapters = instance.GetAdapters();
        for (const auto& adapter : adapters) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);

            EXPECT_EQ(properties.backendType, wgpu::BackendType::Metal);
        }
    }
#endif  // defined(DAWN_ENABLE_BACKEND_METAL)

#if defined(DAWN_ENABLE_BACKEND_DESKTOP_GL)
    // Test discovering only desktop OpenGL adapters
    TEST(AdapterDiscoveryTests, OnlyDesktopGL) {
        if (!glfwInit()) {
            GTEST_SKIP() << "glfwInit() failed";
        }
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        GLFWwindow* window =
            glfwCreateWindow(400, 400, "Dawn OpenGL test window", nullptr, nullptr);
        glfwMakeContextCurrent(window);

        dawn_native::Instance instance;

        dawn_native::opengl::AdapterDiscoveryOptions options;
        options.getProc = reinterpret_cast<void* (*)(const char*)>(glfwGetProcAddress);
        instance.DiscoverAdapters(&options);
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

        const auto& adapters = instance.GetAdapters();
        for (const auto& adapter : adapters) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);

            EXPECT_EQ(properties.backendType, wgpu::BackendType::OpenGL);
        }

        glfwDestroyWindow(window);
    }
#endif  // defined(DAWN_ENABLE_BACKEND_DESKTOP_GL)

#if defined(DAWN_ENABLE_BACKEND_OPENGLES)
    // Test discovering only OpenGLES adapters
    TEST(AdapterDiscoveryTests, OnlyOpenGLES) {
        ScopedEnvironmentVar angleDefaultPlatform;
        if (GetEnvironmentVar("ANGLE_DEFAULT_PLATFORM").first.empty()) {
            angleDefaultPlatform.Set("ANGLE_DEFAULT_PLATFORM", "swiftshader");
        }

        if (!glfwInit()) {
            GTEST_SKIP() << "glfwInit() failed";
        }
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        GLFWwindow* window =
            glfwCreateWindow(400, 400, "Dawn OpenGLES test window", nullptr, nullptr);
        glfwMakeContextCurrent(window);

        dawn_native::Instance instance;

        dawn_native::opengl::AdapterDiscoveryOptionsES options;
        options.getProc = reinterpret_cast<void* (*)(const char*)>(glfwGetProcAddress);
        instance.DiscoverAdapters(&options);
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

        const auto& adapters = instance.GetAdapters();
        for (const auto& adapter : adapters) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);

            EXPECT_EQ(properties.backendType, wgpu::BackendType::OpenGLES);
        }

        glfwDestroyWindow(window);
    }
#endif  // defined(DAWN_ENABLE_BACKEND_OPENGLES)

#if defined(DAWN_ENABLE_BACKEND_METAL) && defined(DAWN_ENABLE_BACKEND_VULKAN)
    // Test discovering the Metal backend, then the Vulkan backend
    // does not duplicate adapters.
    TEST(AdapterDiscoveryTests, OneBackendThenTheOther) {
        dawn_native::Instance instance;
        uint32_t metalAdapterCount = 0;
        {
            dawn_native::metal::AdapterDiscoveryOptions options;
            instance.DiscoverAdapters(&options);

            const auto& adapters = instance.GetAdapters();
            metalAdapterCount = adapters.size();
            for (const auto& adapter : adapters) {
                wgpu::AdapterProperties properties;
                adapter.GetProperties(&properties);

                ASSERT_EQ(properties.backendType, wgpu::BackendType::Metal);
            }
        }
        {
            dawn_native::vulkan::AdapterDiscoveryOptions options;
            instance.DiscoverAdapters(&options);

            uint32_t metalAdapterCount2 = 0;
            const auto& adapters = instance.GetAdapters();
            for (const auto& adapter : adapters) {
                wgpu::AdapterProperties properties;
                adapter.GetProperties(&properties);

                EXPECT_TRUE(properties.backendType == wgpu::BackendType::Metal ||
                            properties.backendType == wgpu::BackendType::Vulkan);
                if (properties.backendType == wgpu::BackendType::Metal) {
                    metalAdapterCount2++;
                }
            }
            EXPECT_EQ(metalAdapterCount, metalAdapterCount2);
        }
    }
#endif  // defined(DAWN_ENABLE_BACKEND_VULKAN) && defined(DAWN_ENABLE_BACKEND_METAL)

}  // anonymous namespace
