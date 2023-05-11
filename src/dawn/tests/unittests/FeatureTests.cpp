// Copyright 2019 The Dawn Authors
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

#include <vector>

#include "dawn/native/Features.h"
#include "dawn/native/Instance.h"
#include "dawn/native/Toggles.h"
#include "dawn/native/null/DeviceNull.h"
#include "gtest/gtest.h"

class FeatureTests : public testing::Test {
  public:
    FeatureTests()
        : testing::Test(),
          mInstanceBase(dawn::native::InstanceBase::Create()),
          mPhysicalDevice(mInstanceBase.Get()),
          mUnsafePhysicalDeviceDisallow(mInstanceBase.Get()),
          mUnsafePhysicalDevice(mInstanceBase.Get()),
          mAdapterBase(&mPhysicalDevice, dawn::native::FeatureLevel::Core),
          mUnsafeAdapterBaseDisallow(
              &mUnsafePhysicalDeviceDisallow,
              dawn::native::FeatureLevel::Core,
              dawn::native::TogglesState(dawn::native::ToggleStage::Adapter)
                  .SetForTesting(dawn::native::Toggle::DisallowUnsafeAPIs, false, false)),
          mUnsafeAdapterBase(
              &mUnsafePhysicalDevice,
              dawn::native::FeatureLevel::Core,
              dawn::native::TogglesState(dawn::native::ToggleStage::Adapter)
                  .SetForTesting(dawn::native::Toggle::AllowUnsafeAPIs, true, true)) {}

    std::vector<wgpu::FeatureName> GetAllFeatureNames() {
        std::vector<wgpu::FeatureName> allFeatureNames(kTotalFeaturesCount);
        for (size_t i = 0; i < kTotalFeaturesCount; ++i) {
            allFeatureNames[i] = FeatureEnumToAPIFeature(static_cast<dawn::native::Feature>(i));
        }
        return allFeatureNames;
    }

    static constexpr size_t kTotalFeaturesCount =
        static_cast<size_t>(dawn::native::Feature::EnumCount);

  protected:
    // By default DisallowUnsafeAPIs is enabled in this instance.
    Ref<dawn::native::InstanceBase> mInstanceBase;
    dawn::native::null::PhysicalDevice mPhysicalDevice;
    // TODO(dawn:1685) Remove duplicated unsafe objects once DisallowUnsafeAPIs is removed.
    dawn::native::null::PhysicalDevice mUnsafePhysicalDeviceDisallow;
    dawn::native::null::PhysicalDevice mUnsafePhysicalDevice;
    // The adapter that inherit toggles states from the instance, also have DisallowUnsafeAPIs
    // enabled.
    dawn::native::AdapterBase mAdapterBase;
    // TODO(dawn:1685) Remove duplicated unsafe objects once DisallowUnsafeAPIs is removed.
    dawn::native::AdapterBase mUnsafeAdapterBaseDisallow;
    dawn::native::AdapterBase mUnsafeAdapterBase;
};

// Test the creation of a device will fail if the requested feature is not supported on the
// Adapter.
TEST_F(FeatureTests, AdapterWithRequiredFeatureDisabled) {
    const std::vector<wgpu::FeatureName> kAllFeatureNames = GetAllFeatureNames();
    for (size_t i = 0; i < kTotalFeaturesCount; ++i) {
        dawn::native::Feature notSupportedFeature = static_cast<dawn::native::Feature>(i);

        std::vector<wgpu::FeatureName> featureNamesWithoutOne = kAllFeatureNames;
        featureNamesWithoutOne.erase(featureNamesWithoutOne.begin() + i);

        // Test that the default adapter validates features as expected.
        {
            mPhysicalDevice.SetSupportedFeaturesForTesting(featureNamesWithoutOne);
            dawn::native::Adapter adapterWithoutFeature(&mAdapterBase);

            wgpu::DeviceDescriptor deviceDescriptor;
            wgpu::FeatureName featureName = FeatureEnumToAPIFeature(notSupportedFeature);
            deviceDescriptor.requiredFeatures = &featureName;
            deviceDescriptor.requiredFeaturesCount = 1;

            WGPUDevice deviceWithFeature = adapterWithoutFeature.CreateDevice(
                reinterpret_cast<const WGPUDeviceDescriptor*>(&deviceDescriptor));
            ASSERT_EQ(nullptr, deviceWithFeature);
        }

        // Test that an adapter with DisallowUnsafeApis disabled validates features as expected.
        // TODO(dawn:1685) Remove this block once DisallowUnsafeAPIs is removed.
        {
            mUnsafePhysicalDeviceDisallow.SetSupportedFeaturesForTesting(featureNamesWithoutOne);
            dawn::native::Adapter adapterWithoutFeature(&mUnsafeAdapterBaseDisallow);

            wgpu::DeviceDescriptor deviceDescriptor;
            wgpu::FeatureName featureName = FeatureEnumToAPIFeature(notSupportedFeature);
            deviceDescriptor.requiredFeatures = &featureName;
            deviceDescriptor.requiredFeaturesCount = 1;

            WGPUDevice deviceWithFeature = adapterWithoutFeature.CreateDevice(
                reinterpret_cast<const WGPUDeviceDescriptor*>(&deviceDescriptor));
            ASSERT_EQ(nullptr, deviceWithFeature);
        }

        // Test that an adapter with AllowUnsafeApis enabled validates features as expected.
        {
            mUnsafePhysicalDevice.SetSupportedFeaturesForTesting(featureNamesWithoutOne);
            dawn::native::Adapter adapterWithoutFeature(&mUnsafeAdapterBase);

            wgpu::DeviceDescriptor deviceDescriptor;
            wgpu::FeatureName featureName = FeatureEnumToAPIFeature(notSupportedFeature);
            deviceDescriptor.requiredFeatures = &featureName;
            deviceDescriptor.requiredFeaturesCount = 1;

            WGPUDevice deviceWithFeature = adapterWithoutFeature.CreateDevice(
                reinterpret_cast<const WGPUDeviceDescriptor*>(&deviceDescriptor));
            ASSERT_EQ(nullptr, deviceWithFeature);
        }
    }
}

// Test creating device requiring a supported feature can succeed (with DisallowUnsafeApis adapter
// toggle disabled for experimental features), and Device.GetEnabledFeatures() can return the names
// of the enabled features correctly.
TEST_F(FeatureTests, RequireAndGetEnabledFeatures) {
    dawn::native::Adapter adapter(&mAdapterBase);
    dawn::native::Adapter unsafeAdapterDisallow(&mUnsafeAdapterBaseDisallow);
    dawn::native::Adapter unsafeAdapterAllow(&mUnsafeAdapterBase);
    dawn::native::FeaturesInfo featuresInfo;

    for (size_t i = 0; i < kTotalFeaturesCount; ++i) {
        dawn::native::Feature feature = static_cast<dawn::native::Feature>(i);
        wgpu::FeatureName featureName = FeatureEnumToAPIFeature(feature);

        wgpu::DeviceDescriptor deviceDescriptor;
        deviceDescriptor.requiredFeatures = &featureName;
        deviceDescriptor.requiredFeaturesCount = 1;

        // Test with the default adapter.
        {
            dawn::native::DeviceBase* deviceBase = dawn::native::FromAPI(adapter.CreateDevice(
                reinterpret_cast<const WGPUDeviceDescriptor*>(&deviceDescriptor)));

            // Creating a device with experimental feature requires the adapter enables
            // AllowUnsafeAPIs or disables DisallowUnsafeApis, otherwise expect validation error.
            if (featuresInfo.GetFeatureInfo(featureName)->featureState ==
                dawn::native::FeatureInfo::FeatureState::Experimental) {
                ASSERT_EQ(nullptr, deviceBase) << i;
            } else {
                // Requiring stable features should succeed.
                ASSERT_NE(nullptr, deviceBase);
                ASSERT_EQ(1u, deviceBase->APIEnumerateFeatures(nullptr));
                wgpu::FeatureName enabledFeature;
                deviceBase->APIEnumerateFeatures(&enabledFeature);
                EXPECT_EQ(enabledFeature, featureName);
                deviceBase->APIRelease();
            }
        }

        // Test with the adapter with DisallowUnsafeApis toggles disabled, creating device should
        // always succeed.
        // TODO(dawn:1685) Remove this block once DisallowUnsafeAPIs is removed.
        {
            dawn::native::DeviceBase* deviceBase =
                dawn::native::FromAPI(unsafeAdapterDisallow.CreateDevice(
                    reinterpret_cast<const WGPUDeviceDescriptor*>(&deviceDescriptor)));

            ASSERT_NE(nullptr, deviceBase);
            ASSERT_EQ(1u, deviceBase->APIEnumerateFeatures(nullptr));
            wgpu::FeatureName enabledFeature;
            deviceBase->APIEnumerateFeatures(&enabledFeature);
            EXPECT_EQ(enabledFeature, featureName);
            deviceBase->APIRelease();
        }

        // Test with the adapter with AllowUnsafeApis toggles enabled, creating device should always
        // succeed.
        {
            dawn::native::DeviceBase* deviceBase =
                dawn::native::FromAPI(unsafeAdapterAllow.CreateDevice(
                    reinterpret_cast<const WGPUDeviceDescriptor*>(&deviceDescriptor)));

            ASSERT_NE(nullptr, deviceBase);
            ASSERT_EQ(1u, deviceBase->APIEnumerateFeatures(nullptr));
            wgpu::FeatureName enabledFeature;
            deviceBase->APIEnumerateFeatures(&enabledFeature);
            EXPECT_EQ(enabledFeature, featureName);
            deviceBase->APIRelease();
        }
    }
}
