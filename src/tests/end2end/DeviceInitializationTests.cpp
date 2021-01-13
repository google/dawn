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

#include "dawn/dawn_proc.h"
#include "tests/DawnTest.h"
#include "utils/SystemUtils.h"
#include "utils/WGPUHelpers.h"

class DeviceInitializationTest : public testing::Test {
    void SetUp() override {
        dawnProcSetProcs(&dawn_native::GetProcs());
    }

    void TearDown() override {
        dawnProcSetProcs(nullptr);
    }
};

// Test that device operations are still valid if the reference to the instance
// is dropped.
TEST_F(DeviceInitializationTest, DeviceOutlivesInstance) {
    // Get properties of all available adapters and then free the instance.
    // We want to create a device on a fresh instance and adapter each time.
    std::vector<wgpu::AdapterProperties> availableAdapterProperties;
    {
        auto instance = std::make_unique<dawn_native::Instance>();
        instance->DiscoverDefaultAdapters();
        for (const dawn_native::Adapter& adapter : instance->GetAdapters()) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);

            if (properties.backendType == wgpu::BackendType::Null) {
                continue;
            }
            availableAdapterProperties.push_back(properties);
        }
    }

    for (const wgpu::AdapterProperties& desiredProperties : availableAdapterProperties) {
        wgpu::Device device;

        auto instance = std::make_unique<dawn_native::Instance>();
        instance->DiscoverDefaultAdapters();
        for (dawn_native::Adapter& adapter : instance->GetAdapters()) {
            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);

            if (properties.deviceID == desiredProperties.deviceID &&
                properties.vendorID == desiredProperties.vendorID &&
                properties.adapterType == desiredProperties.adapterType &&
                properties.backendType == desiredProperties.backendType) {
                // Create the device, destroy the instance, and break out of the loop.
                dawn_native::DeviceDescriptor deviceDescriptor = {};
                device = wgpu::Device(adapter.CreateDevice(&deviceDescriptor));
                instance.reset();
                break;
            }
        }

        // Now, test that the device can still be used by testing a buffer copy.
        wgpu::Buffer src =
            utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::CopySrc, {1, 2, 3, 4});

        wgpu::Buffer dst = utils::CreateBufferFromData<uint32_t>(
            device, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead, {0, 0, 0, 0});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(src, 0, dst, 0, 4 * sizeof(uint32_t));

        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetDefaultQueue().Submit(1, &commands);

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
}
