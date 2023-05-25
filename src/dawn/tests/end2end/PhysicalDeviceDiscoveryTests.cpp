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
#include "dawn/native/DawnNative.h"
#include "dawn/webgpu_cpp.h"

#if defined(DAWN_ENABLE_BACKEND_VULKAN)
#include "dawn/native/VulkanBackend.h"
#endif  // defined(DAWN_ENABLE_BACKEND_VULKAN)

#if defined(DAWN_ENABLE_BACKEND_D3D11)
#include "dawn/native/D3D11Backend.h"
#endif  // defined(DAWN_ENABLE_BACKEND_D3D11)

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

namespace dawn {
namespace {

class PhysicalDeviceDiscoveryTests : public ::testing::Test {};

#if defined(DAWN_ENABLE_BACKEND_VULKAN)
// Test only discovering the SwiftShader physical devices
TEST(PhysicalDeviceDiscoveryTests, OnlySwiftShader) {
    native::Instance instance;

    native::vulkan::PhysicalDeviceDiscoveryOptions options;
    options.forceSwiftShader = true;
    instance.DiscoverPhysicalDevices(&options);

    const auto& adapters = instance.GetAdapters();
    EXPECT_LE(adapters.size(), 2u);  // 0 or 2 SwiftShader adapters.
    for (const auto& adapter : adapters) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);

        EXPECT_EQ(properties.backendType, wgpu::BackendType::Vulkan);
        EXPECT_EQ(properties.adapterType, wgpu::AdapterType::CPU);
        EXPECT_TRUE(gpu_info::IsGoogleSwiftshader(properties.vendorID, properties.deviceID));
    }
}

// Test discovering only Vulkan physical devices
TEST(PhysicalDeviceDiscoveryTests, OnlyVulkan) {
    native::Instance instance;

    native::vulkan::PhysicalDeviceDiscoveryOptions options;
    instance.DiscoverPhysicalDevices(&options);

    const auto& adapters = instance.GetAdapters();
    for (const auto& adapter : adapters) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);

        EXPECT_EQ(properties.backendType, wgpu::BackendType::Vulkan);
    }
}
#endif  // defined(DAWN_ENABLE_BACKEND_VULKAN)

#if defined(DAWN_ENABLE_BACKEND_D3D11)
// Test discovering only D3D11 physical devices
TEST(PhysicalDeviceDiscoveryTests, OnlyD3D11) {
    native::Instance instance;

    native::d3d11::PhysicalDeviceDiscoveryOptions options;
    instance.DiscoverPhysicalDevices(&options);

    const auto& adapters = instance.GetAdapters();
    for (const auto& adapter : adapters) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);

        EXPECT_EQ(properties.backendType, wgpu::BackendType::D3D11);
    }
}

// Test discovering a D3D11 physical device from a prexisting DXGI adapter
TEST(PhysicalDeviceDiscoveryTests, MatchingDXGIAdapterD3D11) {
    using Microsoft::WRL::ComPtr;

    ComPtr<IDXGIFactory4> dxgiFactory;
    HRESULT hr = ::CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory));
    ASSERT_EQ(hr, S_OK);

    for (uint32_t adapterIndex = 0;; ++adapterIndex) {
        ComPtr<IDXGIAdapter1> dxgiAdapter = nullptr;
        if (dxgiFactory->EnumAdapters1(adapterIndex, &dxgiAdapter) == DXGI_ERROR_NOT_FOUND) {
            break;  // No more adapters to enumerate.
        }

        native::Instance instance;

        native::d3d11::PhysicalDeviceDiscoveryOptions options;
        options.dxgiAdapter = std::move(dxgiAdapter);
        instance.DiscoverPhysicalDevices(&options);

        const auto& adapters = instance.GetAdapters();
        for (const auto& adapter : adapters) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);

            EXPECT_EQ(properties.backendType, wgpu::BackendType::D3D11);
        }
    }
}
#endif  // defined(DAWN_ENABLE_BACKEND_D3D11)

#if defined(DAWN_ENABLE_BACKEND_D3D12)
// Test discovering only D3D12 physical devices
TEST(PhysicalDeviceDiscoveryTests, OnlyD3D12) {
    native::Instance instance;

    native::d3d12::PhysicalDeviceDiscoveryOptions options;
    instance.DiscoverPhysicalDevices(&options);

    const auto& adapters = instance.GetAdapters();
    for (const auto& adapter : adapters) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);

        EXPECT_EQ(properties.backendType, wgpu::BackendType::D3D12);
    }
}

// Test discovering a D3D12 physical device from a prexisting DXGI adapter
TEST(PhysicalDeviceDiscoveryTests, MatchingDXGIAdapterD3D12) {
    using Microsoft::WRL::ComPtr;

    ComPtr<IDXGIFactory4> dxgiFactory;
    HRESULT hr = ::CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory));
    ASSERT_EQ(hr, S_OK);

    for (uint32_t adapterIndex = 0;; ++adapterIndex) {
        ComPtr<IDXGIAdapter1> dxgiAdapter = nullptr;
        if (dxgiFactory->EnumAdapters1(adapterIndex, &dxgiAdapter) == DXGI_ERROR_NOT_FOUND) {
            break;  // No more adapters to enumerate.
        }

        native::Instance instance;

        native::d3d12::PhysicalDeviceDiscoveryOptions options;
        options.dxgiAdapter = std::move(dxgiAdapter);
        instance.DiscoverPhysicalDevices(&options);

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
// Test discovering only Metal physical devices
TEST(PhysicalDeviceDiscoveryTests, OnlyMetal) {
    native::Instance instance;

    native::metal::PhysicalDeviceDiscoveryOptions options;
    instance.DiscoverPhysicalDevices(&options);

    const auto& adapters = instance.GetAdapters();
    for (const auto& adapter : adapters) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);

        EXPECT_EQ(properties.backendType, wgpu::BackendType::Metal);
    }
}
#endif  // defined(DAWN_ENABLE_BACKEND_METAL)

#if defined(DAWN_ENABLE_BACKEND_METAL) && defined(DAWN_ENABLE_BACKEND_VULKAN)
// Test discovering the Metal backend, then the Vulkan backend
// does not duplicate physical devices.
TEST(PhysicalDeviceDiscoveryTests, OneBackendThenTheOther) {
    native::Instance instance;
    uint32_t metalAdapterCount = 0;
    {
        native::metal::PhysicalDeviceDiscoveryOptions options;
        instance.DiscoverPhysicalDevices(&options);

        const auto& adapters = instance.GetAdapters();
        metalAdapterCount = adapters.size();
        for (const auto& adapter : adapters) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);

            ASSERT_EQ(properties.backendType, wgpu::BackendType::Metal);
        }
    }
    {
        native::vulkan::PhysicalDeviceDiscoveryOptions options;
        instance.DiscoverPhysicalDevices(&options);

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
}  // namespace dawn
