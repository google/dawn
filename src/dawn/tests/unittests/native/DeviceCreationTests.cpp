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
#include <vector>

#include "dawn/dawn_proc.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/DawnNative.h"
#include "dawn/native/Device.h"
#include "dawn/native/Toggles.h"
#include "dawn/native/dawn_platform.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/utils/SystemUtils.h"
#include "dawn/utils/WGPUHelpers.h"
#include "gtest/gtest.h"

namespace dawn::native {
namespace {

using testing::Contains;
using testing::MockCallback;
using testing::NotNull;
using testing::SaveArg;
using testing::StrEq;

class DeviceCreationTest : public testing::Test {
  protected:
    void SetUp() override {
        dawnProcSetProcs(&GetProcs());

        // Create an instance with default toggles and create an adapter from it.
        WGPUInstanceDescriptor safeInstanceDesc = {};
        instance = std::make_unique<Instance>(&safeInstanceDesc);

        wgpu::RequestAdapterOptions options = {};
        options.backendType = wgpu::BackendType::Null;

        // Get the null adapter with default toggles.
        adapter = instance->EnumerateAdapters(&options)[0];

        // Create an instance with toggle AllowUnsafeAPIs enabled, and create an unsafe adapter
        // from it.
        const char* allowUnsafeApisToggle = "allow_unsafe_apis";
        WGPUDawnTogglesDescriptor unsafeInstanceTogglesDesc = {};
        unsafeInstanceTogglesDesc.chain.sType = WGPUSType::WGPUSType_DawnTogglesDescriptor;
        unsafeInstanceTogglesDesc.enabledToggleCount = 1;
        unsafeInstanceTogglesDesc.enabledToggles = &allowUnsafeApisToggle;
        WGPUInstanceDescriptor unsafeInstanceDesc = {};
        unsafeInstanceDesc.nextInChain = &unsafeInstanceTogglesDesc.chain;

        unsafeInstance = std::make_unique<Instance>(&unsafeInstanceDesc);
        unsafeAdapter = unsafeInstance->EnumerateAdapters(&options)[0];

        ASSERT_NE(adapter.Get(), nullptr);
        ASSERT_NE(unsafeAdapter.Get(), nullptr);
    }

    void TearDown() override {
        adapter = nullptr;
        unsafeAdapter = nullptr;
        instance = nullptr;
        unsafeInstance = nullptr;
        dawnProcSetProcs(nullptr);
    }

    static constexpr size_t kTotalFeaturesCount = static_cast<size_t>(kEnumCount<Feature>);

    std::unique_ptr<Instance> instance;
    std::unique_ptr<Instance> unsafeInstance;
    Adapter adapter;
    Adapter unsafeAdapter;
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
    deviceTogglesDesc.enabledToggleCount = 1;

    wgpu::Device device = adapter.CreateDevice(&desc);
    EXPECT_NE(device, nullptr);

    auto toggles = GetTogglesUsed(device.Get());
    EXPECT_THAT(toggles, Contains(StrEq(toggle)));
}

// Test experimental features are guarded by DisallowUnsafeApis adapter toggle, it is inherited from
// instance but can be overriden by device toggles.
TEST_F(DeviceCreationTest, CreateDeviceRequiringExperimentalFeatures) {
    // Ensure that unsafe apis are disallowed on safe adapter.
    ASSERT_FALSE(FromAPI(adapter.Get())->GetTogglesState().IsEnabled(Toggle::AllowUnsafeAPIs));
    // Ensure that unsafe apis are allowed unsafe adapter(s).
    ASSERT_TRUE(FromAPI(unsafeAdapter.Get())->GetTogglesState().IsEnabled(Toggle::AllowUnsafeAPIs));

    for (size_t i = 0; i < kTotalFeaturesCount; i++) {
        Feature feature = static_cast<Feature>(i);
        wgpu::FeatureName featureName = ToAPI(feature);

        // Only test experimental features.
        if (kFeatureNameAndInfoList[feature].featureState == FeatureInfo::FeatureState::Stable) {
            continue;
        }

        wgpu::DeviceDescriptor deviceDescriptor;
        deviceDescriptor.requiredFeatures = &featureName;
        deviceDescriptor.requiredFeatureCount = 1;

        // Test creating device on default adapter would fail.
        {
            wgpu::Device device = adapter.CreateDevice(&deviceDescriptor);
            EXPECT_EQ(device, nullptr);
        }

        // Test creating device on the adapter with AllowUnsafeApis toggle enabled would succeed.
        {
            deviceDescriptor.nextInChain = nullptr;

            wgpu::Device device = unsafeAdapter.CreateDevice(&deviceDescriptor);
            EXPECT_NE(device, nullptr);

            ASSERT_EQ(1u, device.EnumerateFeatures(nullptr));
            wgpu::FeatureName enabledFeature;
            device.EnumerateFeatures(&enabledFeature);
            EXPECT_EQ(enabledFeature, featureName);
        }

        // Test creating device with AllowUnsafeApis enabled in device toggle descriptor will
        // success on both adapter, as device toggles will override the inherited adapter toggles.
        {
            const char* const enableToggles[] = {"allow_unsafe_apis"};
            wgpu::DawnTogglesDescriptor deviceTogglesDesc;
            deviceTogglesDesc.enabledToggles = enableToggles;
            deviceTogglesDesc.enabledToggleCount = 1;
            deviceDescriptor.nextInChain = &deviceTogglesDesc;

            // Test on adapter with AllowUnsafeApis disabled.
            {
                wgpu::Device device = adapter.CreateDevice(&deviceDescriptor);
                EXPECT_NE(device, nullptr);

                ASSERT_EQ(1u, device.EnumerateFeatures(nullptr));
                wgpu::FeatureName enabledFeature;
                device.EnumerateFeatures(&enabledFeature);
                EXPECT_EQ(enabledFeature, featureName);
            }

            // Test on adapter with AllowUnsafeApis disabled.
            {
                wgpu::Device device = unsafeAdapter.CreateDevice(&deviceDescriptor);
                EXPECT_NE(device, nullptr);

                ASSERT_EQ(1u, device.EnumerateFeatures(nullptr));
                wgpu::FeatureName enabledFeature;
                device.EnumerateFeatures(&enabledFeature);
                EXPECT_EQ(enabledFeature, featureName);
            }
        }

        // Test creating device with AllowUnsafeApis disabled in device toggle descriptor will fail
        // on both adapter, as device toggles will override the inherited adapter toggles.
        {
            const char* const disableToggles[] = {"allow_unsafe_apis"};
            wgpu::DawnTogglesDescriptor deviceToggleDesc;
            deviceToggleDesc.disabledToggles = disableToggles;
            deviceToggleDesc.disabledToggleCount = 1;
            deviceDescriptor.nextInChain = &deviceToggleDesc;

            // Test on adapter with DisallowUnsafeApis enabled.
            {
                wgpu::Device device = adapter.CreateDevice(&deviceDescriptor);
                EXPECT_EQ(device, nullptr);
            }

            // Test on adapter with DisallowUnsafeApis disabled.
            {
                wgpu::Device device = unsafeAdapter.CreateDevice(&deviceDescriptor);
                EXPECT_EQ(device, nullptr);
            }
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

        EXPECT_EQ(FromAPI(device1.Get())->GetCacheKey(), FromAPI(device2.Get())->GetCacheKey());
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

        EXPECT_NE(FromAPI(device1.Get())->GetCacheKey(), FromAPI(device2.Get())->GetCacheKey());
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

        EXPECT_NE(FromAPI(device1.Get())->GetCacheKey(), FromAPI(device2.Get())->GetCacheKey());
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
    desc.requiredFeatureCount = 1;

    adapter.RequestDevice(&desc, cb.Callback(), cb.MakeUserdata(this));
}

}  // anonymous namespace
}  // namespace dawn::native
