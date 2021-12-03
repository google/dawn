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

#include <gtest/gtest.h>

#include "dawn_native/Features.h"
#include "dawn_native/Instance.h"
#include "dawn_native/null/DeviceNull.h"

class FeatureTests : public testing::Test {
  public:
    FeatureTests()
        : testing::Test(),
          mInstanceBase(dawn_native::InstanceBase::Create()),
          mAdapterBase(mInstanceBase.Get()) {
    }

    std::vector<const char*> GetAllFeatureNames() {
        std::vector<const char*> allFeatureNames(kTotalFeaturesCount);
        for (size_t i = 0; i < kTotalFeaturesCount; ++i) {
            allFeatureNames[i] = FeatureEnumToName(static_cast<dawn_native::Feature>(i));
        }
        return allFeatureNames;
    }

    static constexpr size_t kTotalFeaturesCount =
        static_cast<size_t>(dawn_native::Feature::EnumCount);

  protected:
    Ref<dawn_native::InstanceBase> mInstanceBase;
    dawn_native::null::Adapter mAdapterBase;
};

// Test the creation of a device will fail if the requested feature is not supported on the
// Adapter.
TEST_F(FeatureTests, AdapterWithRequiredFeatureDisabled) {
    const std::vector<const char*> kAllFeatureNames = GetAllFeatureNames();
    for (size_t i = 0; i < kTotalFeaturesCount; ++i) {
        dawn_native::Feature notSupportedFeature = static_cast<dawn_native::Feature>(i);

        std::vector<const char*> featureNamesWithoutOne = kAllFeatureNames;
        featureNamesWithoutOne.erase(featureNamesWithoutOne.begin() + i);

        mAdapterBase.SetSupportedFeatures(featureNamesWithoutOne);
        dawn_native::Adapter adapterWithoutFeature(&mAdapterBase);

        dawn_native::DawnDeviceDescriptor deviceDescriptor;
        const char* featureName = FeatureEnumToName(notSupportedFeature);
        deviceDescriptor.requiredFeatures = std::vector<const char*>(1, featureName);
        WGPUDevice deviceWithFeature = adapterWithoutFeature.CreateDevice(&deviceDescriptor);
        ASSERT_EQ(nullptr, deviceWithFeature);
    }
}

// Test Device.GetEnabledFeatures() can return the names of the enabled features correctly.
TEST_F(FeatureTests, GetEnabledFeatures) {
    dawn_native::Adapter adapter(&mAdapterBase);
    for (size_t i = 0; i < kTotalFeaturesCount; ++i) {
        dawn_native::Feature feature = static_cast<dawn_native::Feature>(i);
        const char* featureName = FeatureEnumToName(feature);

        dawn_native::DawnDeviceDescriptor deviceDescriptor;
        deviceDescriptor.requiredFeatures = {featureName};
        dawn_native::DeviceBase* deviceBase =
            dawn_native::FromAPI(adapter.CreateDevice(&deviceDescriptor));
        std::vector<const char*> enabledFeatures = deviceBase->GetEnabledFeatures();
        ASSERT_EQ(1u, enabledFeatures.size());
        ASSERT_EQ(0, std::strcmp(featureName, enabledFeatures[0]));
    }
}
