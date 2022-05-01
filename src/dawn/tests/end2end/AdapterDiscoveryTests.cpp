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

#include <memory>
#include <utility>

#include "dawn/common/GPUInfo.h"
#include "dawn/common/Log.h"
#include "dawn/common/Platform.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/dawn_proc.h"
#include "dawn/native/DawnNative.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/webgpu_cpp.h"

#if defined(DAWN_ENABLE_BACKEND_VULKAN)
#include "dawn/native/VulkanBackend.h"
#endif  // defined(DAWN_ENABLE_BACKEND_VULKAN)

#if defined(DAWN_ENABLE_BACKEND_D3D12)
#include "dawn/native/D3D12Backend.h"
#endif  // defined(DAWN_ENABLE_BACKEND_D3D12)

#if defined(DAWN_ENABLE_BACKEND_METAL)
#include "dawn/native/MetalBackend.h"
#endif  // defined(DAWN_ENABLE_BACKEND_METAL)

#if defined(DAWN_ENABLE_BACKEND_DESKTOP_GL) || defined(DAWN_ENABLE_BACKEND_OPENGLES)
#include "GLFW/glfw3.h"
#include "dawn/native/OpenGLBackend.h"
#endif  // defined(DAWN_ENABLE_BACKEND_DESKTOP_GL) || defined(DAWN_ENABLE_BACKEND_OPENGLES)

#include <gtest/gtest.h>

namespace {

using testing::_;
using testing::MockCallback;
using testing::SaveArg;

class AdapterDiscoveryTests : public ::testing::Test {};

#if defined(DAWN_ENABLE_BACKEND_VULKAN)
// Test only discovering the SwiftShader adapter
TEST(AdapterDiscoveryTests, OnlySwiftShader) {
    dawn::native::Instance instance;

    dawn::native::vulkan::AdapterDiscoveryOptions options;
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
    dawn::native::Instance instance;

    dawn::native::vulkan::AdapterDiscoveryOptions options;
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
    dawn::native::Instance instance;

    dawn::native::d3d12::AdapterDiscoveryOptions options;
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

        dawn::native::Instance instance;

        dawn::native::d3d12::AdapterDiscoveryOptions options;
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
    dawn::native::Instance instance;

    dawn::native::metal::AdapterDiscoveryOptions options;
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

    GLFWwindow* window = glfwCreateWindow(400, 400, "Dawn OpenGL test window", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    dawn::native::Instance instance;

    dawn::native::opengl::AdapterDiscoveryOptions options;
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

    GLFWwindow* window = glfwCreateWindow(400, 400, "Dawn OpenGLES test window", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    dawn::native::Instance instance;

    dawn::native::opengl::AdapterDiscoveryOptionsES options;
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
    dawn::native::Instance instance;
    uint32_t metalAdapterCount = 0;
    {
        dawn::native::metal::AdapterDiscoveryOptions options;
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
        dawn::native::vulkan::AdapterDiscoveryOptions options;
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

class AdapterCreationTest : public ::testing::Test {
  protected:
    void SetUp() override {
        dawnProcSetProcs(&dawn_native::GetProcs());

        {
            auto nativeInstance = std::make_unique<dawn_native::Instance>();
            nativeInstance->DiscoverDefaultAdapters();
            for (dawn_native::Adapter& nativeAdapter : nativeInstance->GetAdapters()) {
                anyAdapterAvailable = true;

                wgpu::AdapterProperties properties;
                nativeAdapter.GetProperties(&properties);
                swiftShaderAvailable =
                    swiftShaderAvailable ||
                    gpu_info::IsSwiftshader(properties.vendorID, properties.deviceID);
                discreteGPUAvailable = discreteGPUAvailable ||
                                       properties.adapterType == wgpu::AdapterType::DiscreteGPU;
                integratedGPUAvailable = integratedGPUAvailable ||
                                         properties.adapterType == wgpu::AdapterType::IntegratedGPU;
            }
        }

        instance = wgpu::CreateInstance();
    }

    void TearDown() override {
        instance = nullptr;
        dawnProcSetProcs(nullptr);
    }

    wgpu::Instance instance;
    bool anyAdapterAvailable = false;
    bool swiftShaderAvailable = false;
    bool discreteGPUAvailable = false;
    bool integratedGPUAvailable = false;
};

// Test that requesting the default adapter works
TEST_F(AdapterCreationTest, DefaultAdapter) {
    wgpu::RequestAdapterOptions options = {};

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    instance.RequestAdapter(&options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
}

// Test that passing nullptr for the options gets the default adapter
TEST_F(AdapterCreationTest, NullGivesDefaultAdapter) {
    wgpu::RequestAdapterOptions options = {};

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    instance.RequestAdapter(&options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);

    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this + 1))
        .WillOnce(SaveArg<1>(&cAdapter));
    instance.RequestAdapter(nullptr, cb.Callback(), cb.MakeUserdata(this + 1));

    wgpu::Adapter adapter2 = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter.Get(), adapter2.Get());
}

// Test that requesting the fallback adapter returns SwiftShader.
TEST_F(AdapterCreationTest, FallbackAdapter) {
    wgpu::RequestAdapterOptions options = {};
    options.forceFallbackAdapter = true;

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    instance.RequestAdapter(&options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, swiftShaderAvailable);
    if (adapter != nullptr) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);

        EXPECT_EQ(properties.adapterType, wgpu::AdapterType::CPU);
        EXPECT_TRUE(gpu_info::IsSwiftshader(properties.vendorID, properties.deviceID));
    }
}

// Test that requesting a high performance GPU works
TEST_F(AdapterCreationTest, PreferHighPerformance) {
    wgpu::RequestAdapterOptions options = {};
    options.powerPreference = wgpu::PowerPreference::HighPerformance;

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    instance.RequestAdapter(&options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
    if (discreteGPUAvailable) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);
        EXPECT_EQ(properties.adapterType, wgpu::AdapterType::DiscreteGPU);
    }
}

// Test that requesting a low power GPU works
TEST_F(AdapterCreationTest, PreferLowPower) {
    wgpu::RequestAdapterOptions options = {};
    options.powerPreference = wgpu::PowerPreference::LowPower;

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    instance.RequestAdapter(&options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
    if (integratedGPUAvailable) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);
        EXPECT_EQ(properties.adapterType, wgpu::AdapterType::IntegratedGPU);
    }
}

}  // anonymous namespace
