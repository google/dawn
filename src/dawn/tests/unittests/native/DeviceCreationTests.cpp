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
#include <vector>

#include "dawn/dawn_proc.h"
#include "dawn/native/DawnNative.h"
#include "dawn/native/Device.h"
#include "dawn/native/dawn_platform.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/utils/SystemUtils.h"
#include "dawn/utils/WGPUHelpers.h"
#include "gtest/gtest.h"

namespace {

using testing::Contains;
using testing::MockCallback;
using testing::NotNull;
using testing::SaveArg;
using testing::StrEq;

class DeviceCreationTest : public testing::Test {
  protected:
    void SetUp() override {
        dawnProcSetProcs(&dawn::native::GetProcs());

        instance = std::make_unique<dawn::native::Instance>();
        instance->DiscoverDefaultAdapters();
        for (dawn::native::Adapter& nativeAdapter : instance->GetAdapters()) {
            wgpu::AdapterProperties properties;
            nativeAdapter.GetProperties(&properties);

            if (properties.backendType == wgpu::BackendType::Null) {
                adapter = wgpu::Adapter(nativeAdapter.Get());
                break;
            }
        }
        ASSERT_NE(adapter, nullptr);
    }

    void TearDown() override {
        adapter = nullptr;
        instance = nullptr;
        dawnProcSetProcs(nullptr);
    }

    std::unique_ptr<dawn::native::Instance> instance;
    wgpu::Adapter adapter;
};

// Test successful call to CreateDevice with no descriptor
TEST_F(DeviceCreationTest, CreateDeviceNoDescriptorSuccess) {
    wgpu::Device device = adapter.CreateDevice();
    EXPECT_NE(device, nullptr);
}

// Test successful call to CreateDevice with descriptor.
TEST_F(DeviceCreationTest, CreateDeviceSuccess) {
    wgpu::DeviceDescriptor desc = {};
    wgpu::Device device = adapter.CreateDevice(&desc);
    EXPECT_NE(device, nullptr);
}

// Test successful call to CreateDevice with toggle descriptor.
TEST_F(DeviceCreationTest, CreateDeviceWithTogglesSuccess) {
    wgpu::DeviceDescriptor desc = {};
    wgpu::DawnTogglesDescriptor deviceTogglesDesc = {};
    desc.nextInChain = &deviceTogglesDesc;

    const char* toggle = "skip_validation";
    deviceTogglesDesc.enabledToggles = &toggle;
    deviceTogglesDesc.enabledTogglesCount = 1;

    wgpu::Device device = adapter.CreateDevice(&desc);
    EXPECT_NE(device, nullptr);

    auto toggles = dawn::native::GetTogglesUsed(device.Get());
    EXPECT_THAT(toggles, Contains(StrEq(toggle)));
}

// Test features guarded by toggles are validated when creating devices.
TEST_F(DeviceCreationTest, CreateDeviceRequiringFeaturesGuardedByToggle) {
    std::vector<wgpu::FeatureName> featuresGuardedByToggle = {
        wgpu::FeatureName::ShaderF16, wgpu::FeatureName::ChromiumExperimentalDp4a};

    for (auto feature : featuresGuardedByToggle) {
        wgpu::DeviceDescriptor deviceDescriptor;
        deviceDescriptor.requiredFeatures = &feature;
        deviceDescriptor.requiredFeaturesCount = 1;

        // Test creating device without toggle would fail.
        {
            wgpu::Device device = adapter.CreateDevice(&deviceDescriptor);
            EXPECT_EQ(device, nullptr);
        }

        // Test creating device without DisallowUnsafeApis toggle disabled.
        {
            const char* const disableToggles[] = {"disallow_unsafe_apis"};
            wgpu::DawnTogglesDescriptor deviceTogglesDesc;
            deviceTogglesDesc.disabledToggles = disableToggles;
            deviceTogglesDesc.disabledTogglesCount = 1;
            deviceDescriptor.nextInChain = &deviceTogglesDesc;

            wgpu::Device device = adapter.CreateDevice(&deviceDescriptor);
            EXPECT_NE(device, nullptr);

            ASSERT_EQ(1u, device.EnumerateFeatures(nullptr));
            wgpu::FeatureName enabledFeature;
            device.EnumerateFeatures(&enabledFeature);
            EXPECT_EQ(enabledFeature, feature);
            device.Release();
        }
    }
}

TEST_F(DeviceCreationTest, CreateDeviceWithCacheSuccess) {
    // Default device descriptor should have the same cache key as a device descriptor with a
    // default cache descriptor.
    {
        wgpu::DeviceDescriptor desc = {};
        wgpu::Device device1 = adapter.CreateDevice(&desc);
        EXPECT_NE(device1, nullptr);

        wgpu::DawnCacheDeviceDescriptor cacheDesc = {};
        desc.nextInChain = &cacheDesc;
        wgpu::Device device2 = adapter.CreateDevice(&desc);

        EXPECT_EQ(dawn::native::FromAPI(device1.Get())->GetCacheKey(),
                  dawn::native::FromAPI(device2.Get())->GetCacheKey());
    }
    // Default device descriptor should not have the same cache key as a device descriptor with
    // a non-default cache descriptor.
    {
        wgpu::DeviceDescriptor desc = {};
        wgpu::Device device1 = adapter.CreateDevice(&desc);
        EXPECT_NE(device1, nullptr);

        wgpu::DawnCacheDeviceDescriptor cacheDesc = {};
        desc.nextInChain = &cacheDesc;
        const char* isolationKey = "isolation key";
        cacheDesc.isolationKey = isolationKey;
        wgpu::Device device2 = adapter.CreateDevice(&desc);
        EXPECT_NE(device2, nullptr);

        EXPECT_NE(dawn::native::FromAPI(device1.Get())->GetCacheKey(),
                  dawn::native::FromAPI(device2.Get())->GetCacheKey());
    }
    // Two non-default cache descriptors should not have the same cache key.
    {
        wgpu::DawnCacheDeviceDescriptor cacheDesc = {};
        const char* isolationKey1 = "isolation key 1";
        const char* isolationKey2 = "isolation key 2";
        wgpu::DeviceDescriptor desc = {};
        desc.nextInChain = &cacheDesc;

        cacheDesc.isolationKey = isolationKey1;
        wgpu::Device device1 = adapter.CreateDevice(&desc);
        EXPECT_NE(device1, nullptr);

        cacheDesc.isolationKey = isolationKey2;
        wgpu::Device device2 = adapter.CreateDevice(&desc);
        EXPECT_NE(device2, nullptr);

        EXPECT_NE(dawn::native::FromAPI(device1.Get())->GetCacheKey(),
                  dawn::native::FromAPI(device2.Get())->GetCacheKey());
    }
}

// Test successful call to RequestDevice with descriptor
TEST_F(DeviceCreationTest, RequestDeviceSuccess) {
    WGPUDevice cDevice;
    {
        MockCallback<WGPURequestDeviceCallback> cb;
        EXPECT_CALL(cb, Call(WGPURequestDeviceStatus_Success, NotNull(), nullptr, this))
            .WillOnce(SaveArg<1>(&cDevice));

        wgpu::DeviceDescriptor desc = {};
        adapter.RequestDevice(&desc, cb.Callback(), cb.MakeUserdata(this));
    }

    wgpu::Device device = wgpu::Device::Acquire(cDevice);
    EXPECT_NE(device, nullptr);
}

// Test successful call to RequestDevice with a null descriptor
TEST_F(DeviceCreationTest, RequestDeviceNullDescriptorSuccess) {
    WGPUDevice cDevice;
    {
        MockCallback<WGPURequestDeviceCallback> cb;
        EXPECT_CALL(cb, Call(WGPURequestDeviceStatus_Success, NotNull(), nullptr, this))
            .WillOnce(SaveArg<1>(&cDevice));

        adapter.RequestDevice(nullptr, cb.Callback(), cb.MakeUserdata(this));
    }

    wgpu::Device device = wgpu::Device::Acquire(cDevice);
    EXPECT_NE(device, nullptr);
}

// Test failing call to RequestDevice with invalid feature
TEST_F(DeviceCreationTest, RequestDeviceFailure) {
    MockCallback<WGPURequestDeviceCallback> cb;
    EXPECT_CALL(cb, Call(WGPURequestDeviceStatus_Error, nullptr, NotNull(), this)).Times(1);

    wgpu::DeviceDescriptor desc = {};
    wgpu::FeatureName invalidFeature = static_cast<wgpu::FeatureName>(WGPUFeatureName_Force32);
    desc.requiredFeatures = &invalidFeature;
    desc.requiredFeaturesCount = 1;

    adapter.RequestDevice(&desc, cb.Callback(), cb.MakeUserdata(this));
}

}  // anonymous namespace
