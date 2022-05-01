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
#include "dawn/native/null/DeviceNull.h"
#include "gtest/gtest.h"

class FeatureTests : public testing::Test {
  public:
    FeatureTests()
        : testing::Test(),
          mInstanceBase(dawn::native::InstanceBase::Create()),
          mAdapterBase(mInstanceBase.Get()) {}

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
    Ref<dawn::native::InstanceBase> mInstanceBase;
    dawn::native::null::Adapter mAdapterBase;
};

// Test the creation of a device will fail if the requested feature is not supported on the
// Adapter.
TEST_F(FeatureTests, AdapterWithRequiredFeatureDisabled) {
    const std::vector<wgpu::FeatureName> kAllFeatureNames = GetAllFeatureNames();
    for (size_t i = 0; i < kTotalFeaturesCount; ++i) {
        dawn::native::Feature notSupportedFeature = static_cast<dawn::native::Feature>(i);

        std::vector<wgpu::FeatureName> featureNamesWithoutOne = kAllFeatureNames;
        featureNamesWithoutOne.erase(featureNamesWithoutOne.begin() + i);

        mAdapterBase.SetSupportedFeatures(featureNamesWithoutOne);
        dawn::native::Adapter adapterWithoutFeature(&mAdapterBase);

        wgpu::DeviceDescriptor deviceDescriptor;
        wgpu::FeatureName featureName = FeatureEnumToAPIFeature(notSupportedFeature);
        deviceDescriptor.requiredFeatures = &featureName;
        deviceDescriptor.requiredFeaturesCount = 1;

        WGPUDevice deviceWithFeature = adapterWithoutFeature.CreateDevice(
            reinterpret_cast<const WGPUDeviceDescriptor*>(&deviceDescriptor));
        ASSERT_EQ(nullptr, deviceWithFeature);
    }
}

// Test Device.GetEnabledFeatures() can return the names of the enabled features correctly.
TEST_F(FeatureTests, GetEnabledFeatures) {
    dawn::native::Adapter adapter(&mAdapterBase);
    for (size_t i = 0; i < kTotalFeaturesCount; ++i) {
        dawn::native::Feature feature = static_cast<dawn::native::Feature>(i);
        wgpu::FeatureName featureName = FeatureEnumToAPIFeature(feature);

        wgpu::DeviceDescriptor deviceDescriptor;
        deviceDescriptor.requiredFeatures = &featureName;
        deviceDescriptor.requiredFeaturesCount = 1;

        dawn::native::DeviceBase* deviceBase = dawn::native::FromAPI(
            adapter.CreateDevice(reinterpret_cast<const WGPUDeviceDescriptor*>(&deviceDescriptor)));

        ASSERT_EQ(1u, deviceBase->APIEnumerateFeatures(nullptr));
        wgpu::FeatureName enabledFeature;
        deviceBase->APIEnumerateFeatures(&enabledFeature);
        EXPECT_EQ(enabledFeature, featureName);
    }
}
