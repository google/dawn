// Copyright 2021 The Dawn & Tint Authors
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

#include <memory>
#include <utility>
#include <vector>

#include "dawn/dawn_proc.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/SystemUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class DeviceInitializationTest : public testing::Test {
  protected:
    void SetUp() override { dawnProcSetProcs(&native::GetProcs()); }

    void TearDown() override { dawnProcSetProcs(nullptr); }

    // Test that the device can still be used by testing a buffer copy.
    void ExpectDeviceUsable(wgpu::Device device) {
        wgpu::Buffer src =
            utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::CopySrc, {1, 2, 3, 4});

        wgpu::Buffer dst = utils::CreateBufferFromData<uint32_t>(
            device, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead, {0, 0, 0, 0});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(src, 0, dst, 0, 4 * sizeof(uint32_t));

        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);

        bool done = false;
        dst.MapAsync(
            wgpu::MapMode::Read, 0, 4 * sizeof(uint32_t),
            [](WGPUBufferMapAsyncStatus status, void* userdata) {
                EXPECT_EQ(status, WGPUBufferMapAsyncStatus_Success);
                *static_cast<bool*>(userdata) = true;
            },
            &done);

        // Note: we can't actually test this if Tick moves over to
        // wgpuInstanceProcessEvents. We can still test that object creation works
        // without crashing.
        while (!done) {
            device.Tick();
            utils::USleep(100);
        }

        const uint32_t* mapping = static_cast<const uint32_t*>(dst.GetConstMappedRange());
        EXPECT_EQ(mapping[0], 1u);
        EXPECT_EQ(mapping[1], 2u);
        EXPECT_EQ(mapping[2], 3u);
        EXPECT_EQ(mapping[3], 4u);
    }
};

// Test that device operations are still valid if the reference to the instance
// is dropped.
TEST_F(DeviceInitializationTest, DeviceOutlivesInstance) {
    // Get properties of all available adapters and then free the instance.
    // We want to create a device on a fresh instance and adapter each time.
    std::vector<wgpu::AdapterProperties> availableAdapterProperties;
    {
        auto instance = std::make_unique<native::Instance>();
        instance->EnableAdapterBlocklist(false);
        for (const native::Adapter& adapter : instance->EnumerateAdapters()) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);

            if (properties.backendType == wgpu::BackendType::Null) {
                continue;
            }

            availableAdapterProperties.push_back(std::move(properties));
        }
    }

    for (const wgpu::AdapterProperties& desiredProperties : availableAdapterProperties) {
        wgpu::Device device;

        auto instance = std::make_unique<native::Instance>();
        instance->EnableAdapterBlocklist(false);
        for (native::Adapter& adapter : instance->EnumerateAdapters()) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);

            if (properties.deviceID == desiredProperties.deviceID &&
                properties.vendorID == desiredProperties.vendorID &&
                properties.adapterType == desiredProperties.adapterType &&
                properties.backendType == desiredProperties.backendType) {
                // Create the device, destroy the instance, and break out of the loop.
                device = wgpu::Device::Acquire(adapter.CreateDevice());
                instance.reset();
                break;
            }
        }

        if (device) {
            ExpectDeviceUsable(std::move(device));
        }
    }
}

// Test that it is still possible to create a device from an adapter after the reference to the
// instance is dropped.
TEST_F(DeviceInitializationTest, AdapterOutlivesInstance) {
    // Get properties of all available adapters and then free the instance.
    // We want to create a device on a fresh instance and adapter each time.
    std::vector<wgpu::AdapterProperties> availableAdapterProperties;
    {
        auto instance = std::make_unique<native::Instance>();
        instance->EnableAdapterBlocklist(false);
        for (const native::Adapter& adapter : instance->EnumerateAdapters()) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);

            if (properties.backendType == wgpu::BackendType::Null) {
                continue;
            }
            // TODO(dawn:1705): Remove this once D3D11 backend is fully implemented.
            if (properties.backendType == wgpu::BackendType::D3D11) {
                continue;
            }
            availableAdapterProperties.push_back(std::move(properties));
        }
    }

    for (const wgpu::AdapterProperties& desiredProperties : availableAdapterProperties) {
        wgpu::Adapter adapter;

        auto instance = std::make_unique<native::Instance>();
        instance->EnableAdapterBlocklist(false);
        // Save a pointer to the instance.
        // It will only be valid as long as the instance is alive.
        WGPUInstance unsafeInstancePtr = instance->Get();

        for (native::Adapter& nativeAdapter : instance->EnumerateAdapters()) {
            wgpu::AdapterProperties properties;
            nativeAdapter.GetProperties(&properties);

            if (properties.deviceID == desiredProperties.deviceID &&
                properties.vendorID == desiredProperties.vendorID &&
                properties.adapterType == desiredProperties.adapterType &&
                properties.backendType == desiredProperties.backendType) {
                // Save the adapter, and reset the instance.
                // Check that the number of adapters before the reset is > 0, and after the reset
                // is 0. Unsafe, but we assume the pointer is still valid since the adapter is
                // holding onto the instance. The instance should have cleared all internal
                // references to adapters when the last external ref is dropped.
                adapter = wgpu::Adapter(nativeAdapter.Get());
                EXPECT_GT(native::GetPhysicalDeviceCountForTesting(unsafeInstancePtr), 0u);
                instance.reset();
                EXPECT_EQ(native::GetPhysicalDeviceCountForTesting(unsafeInstancePtr), 0u);
                break;
            }
        }

        if (adapter) {
            ExpectDeviceUsable(adapter.CreateDevice());
        }
    }
}

}  // anonymous namespace
}  // namespace dawn
