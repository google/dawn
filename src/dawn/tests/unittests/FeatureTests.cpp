// Copyright 2019 The Dawn & Tint Authors
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

#include <vector>

#include "gtest/gtest.h"
#include "src/dawn/native/Features.h"
#include "src/dawn/native/Instance.h"
#include "src/dawn/native/Toggles.h"
#include "src/dawn/native/null/DeviceNull.h"
#include "src/dawn/utils/WGPUHelpers.h"
#include "src/utils/compiler.h"
#include "src/utils/span.h"

namespace dawn {
namespace {

using ::testing::Test;

class FeatureTests : public Test {
  public:
    FeatureTests()
        : Test(),
          mInstanceBase([]() -> Ref<dawn::native::InstanceBase> {
              static constexpr auto kMultipleDevicesPerAdapter =
                  wgpu::InstanceFeatureName::MultipleDevicesPerAdapter;
              dawn::native::InstanceDescriptor instanceDesc = {
                  .requiredFeatures = SpanFromRef(kMultipleDevicesPerAdapter),
              };
              return dawn::native::APICreateInstance(&instanceDesc);
          }()),
          mPhysicalDevice(native::null::PhysicalDevice::Create(mInstanceBase.Get())),
          mAdapterBase(mInstanceBase.Get(),
                       mPhysicalDevice.Get(),
                       wgpu::FeatureLevel::Core,
                       native::TogglesState(native::ToggleStage::Adapter)
                           .SetForTesting(native::Toggle::AllowExperimentalAPIs, true, false),
                       wgpu::PowerPreference::Undefined),
          mUnsafeAdapterBase(mInstanceBase.Get(),
                             mPhysicalDevice.Get(),
                             wgpu::FeatureLevel::Core,
                             native::TogglesState(native::ToggleStage::Adapter)
                                 .SetForTesting(native::Toggle::AllowUnsafeAPIs, true, true)
                                 .SetForTesting(native::Toggle::AllowExperimentalAPIs, true, true),
                             wgpu::PowerPreference::Undefined),
          mStableOnlyAdapterBase(
              mInstanceBase.Get(),
              mPhysicalDevice.Get(),
              wgpu::FeatureLevel::Core,
              native::TogglesState(native::ToggleStage::Adapter)
                  .SetForTesting(native::Toggle::AllowUnsafeAPIs, false, false)
                  .SetForTesting(native::Toggle::AllowExperimentalAPIs, false, false),
              wgpu::PowerPreference::Undefined) {}

    std::vector<wgpu::FeatureName> GetAllFeatureNames() {
        std::vector<wgpu::FeatureName> allFeatureNames(kTotalFeaturesCount);
        for (size_t i = 0; i < kTotalFeaturesCount; ++i) {
            allFeatureNames[i] = native::ToCppAPI(static_cast<native::Feature>(i));
        }
        return allFeatureNames;
    }

    static constexpr size_t kTotalFeaturesCount =
        static_cast<size_t>(native::kEnumCount<native::Feature>);

  protected:
    // By default AllowUnsafeAPIs is disabled in this instance.
    Ref<dawn::native::InstanceBase> mInstanceBase;
    Ref<native::null::PhysicalDevice> mPhysicalDevice;
    // Adapters configured with different toggle states on the same physical device:
    // - mAdapterBase: AllowUnsafeAPIs disabled, AllowExperimentalAPIs enabled.
    // - mUnsafeAdapterBase: both AllowUnsafeAPIs and AllowExperimentalAPIs enabled.
    // - mStableOnlyAdapterBase: both AllowUnsafeAPIs and AllowExperimentalAPIs disabled.
    native::AdapterBase mAdapterBase;
    native::AdapterBase mUnsafeAdapterBase;
    native::AdapterBase mStableOnlyAdapterBase;
};

// Test the creation of a device will fail if the requested feature is not supported on the
// Adapter.
TEST_F(FeatureTests, AdapterWithRequiredFeatureDisabled) {
    const std::vector<wgpu::FeatureName> kAllFeatureNames = GetAllFeatureNames();
    for (size_t i = 0; i < kTotalFeaturesCount; ++i) {
        native::Feature notSupportedFeature = static_cast<native::Feature>(i);

        std::vector<wgpu::FeatureName> featureNamesWithoutOne = kAllFeatureNames;
        featureNamesWithoutOne.erase(featureNamesWithoutOne.begin() + i);
        mPhysicalDevice->SetSupportedFeaturesForTesting(featureNamesWithoutOne);

        // Test that the default adapter validates features as expected.
        {
            native::Adapter adapterWithoutFeature(&mAdapterBase);

            wgpu::DeviceDescriptor deviceDescriptor;
            wgpu::FeatureName featureName = native::ToCppAPI(notSupportedFeature);
            deviceDescriptor.requiredFeatures = &featureName;
            deviceDescriptor.requiredFeatureCount = 1;

            WGPUDevice deviceWithFeature = adapterWithoutFeature.CreateDevice(
                reinterpret_cast<const WGPUDeviceDescriptor*>(&deviceDescriptor));
            ASSERT_EQ(nullptr, deviceWithFeature);
        }

        // Test that an adapter with AllowUnsafeApis enabled validates features as expected.
        {
            native::Adapter adapterWithoutFeature(&mUnsafeAdapterBase);

            wgpu::DeviceDescriptor deviceDescriptor;
            wgpu::FeatureName featureName = native::ToCppAPI(notSupportedFeature);
            deviceDescriptor.requiredFeatures = &featureName;
            deviceDescriptor.requiredFeatureCount = 1;

            WGPUDevice deviceWithFeature = adapterWithoutFeature.CreateDevice(
                reinterpret_cast<const WGPUDeviceDescriptor*>(&deviceDescriptor));
            ASSERT_EQ(nullptr, deviceWithFeature);
        }

        // Test that an adapter with both AllowUnsafeAPIs and AllowExperimentalAPIs disabled
        // validates features as expected.
        {
            native::Adapter adapterWithoutFeature(&mStableOnlyAdapterBase);

            wgpu::DeviceDescriptor deviceDescriptor;
            wgpu::FeatureName featureName = native::ToCppAPI(notSupportedFeature);
            deviceDescriptor.requiredFeatures = &featureName;
            deviceDescriptor.requiredFeatureCount = 1;

            WGPUDevice deviceWithFeature = adapterWithoutFeature.CreateDevice(
                reinterpret_cast<const WGPUDeviceDescriptor*>(&deviceDescriptor));
            ASSERT_EQ(nullptr, deviceWithFeature);
        }
    }
}

bool IsUnsafe(wgpu::FeatureName feature) {
    return native::kFeatureNameAndInfoList[native::FromCppAPI(feature)].featureSafety ==
           native::FeatureInfo::FeatureSafety::Unsafe;
}

bool IsExperimental(wgpu::FeatureName feature) {
    return native::kFeatureNameAndInfoList[native::FromCppAPI(feature)].featureStability ==
           native::FeatureInfo::FeatureStability::Experimental;
}

// Test creating device requiring a supported feature can succeed (with AllowUnsafeAPIs enabled on
// the adapter for unsafe features), and Device.APIGetFeatures() can return the enabled features
// correctly.
TEST_F(FeatureTests, RequireAndGetEnabledFeatures) {
    native::Adapter adapter(&mAdapterBase);
    native::Adapter unsafeAdapterAllow(&mUnsafeAdapterBase);
    native::Adapter stableOnlyAdapter(&mStableOnlyAdapterBase);

    for (size_t i = 0; i < kTotalFeaturesCount; ++i) {
        native::Feature feature = static_cast<native::Feature>(i);
        wgpu::FeatureName featureName = native::ToCppAPI(feature);

        // Enable features that are implicitly enabled by other features.
        absl::flat_hash_set<wgpu::FeatureName> requiredFeaturesSet =
            utils::FeatureAndImplicitlyEnabled(featureName);

        std::vector<wgpu::FeatureName> features(requiredFeaturesSet.cbegin(),
                                                requiredFeaturesSet.cend());
        bool requiredUnsafeFeature = false;
        bool requiredExperimentalFeature = false;
        for (auto requiredFeature : features) {
            requiredUnsafeFeature = requiredUnsafeFeature || IsUnsafe(requiredFeature);
            requiredExperimentalFeature =
                requiredExperimentalFeature || IsExperimental(requiredFeature);
        }

        wgpu::DeviceDescriptor deviceDescriptor;
        deviceDescriptor.requiredFeatures = features.data();
        deviceDescriptor.requiredFeatureCount = features.size();

        // Helper to check the returned device has all required features
        auto ExpectDeviceHasRequiredFeatures =
            [&requiredFeaturesSet](native::DeviceBase* deviceBase) {
                native::SupportedFeatures enabledFeatures;
                deviceBase->APIGetFeatures(&enabledFeatures);
                bool explicitlyRequireCore =
                    requiredFeaturesSet.contains(wgpu::FeatureName::CoreFeaturesAndLimits);
                // wgpu::FeatureName::CoreFeaturesAndLimits is required implicitly in core mode
                ASSERT_EQ(requiredFeaturesSet.size() + (explicitlyRequireCore ? 0 : 1),
                          enabledFeatures.features.size());
                for (wgpu::FeatureName enabledFeature : enabledFeatures.features) {
                    if (!explicitlyRequireCore &&
                        enabledFeature == wgpu::FeatureName::CoreFeaturesAndLimits) {
                        continue;
                    }
                    EXPECT_TRUE(requiredFeaturesSet.contains(enabledFeature));
                }
            };

        // Test with the default adapter.
        {
            native::DeviceBase* deviceBase = native::FromAPI(adapter.CreateDevice(
                reinterpret_cast<const WGPUDeviceDescriptor*>(&deviceDescriptor)));

            // Creating a device with unsafe feature requires the adapter enables
            // AllowUnsafeAPIs, otherwise expect validation error.
            if (requiredUnsafeFeature) {
                ASSERT_EQ(nullptr, deviceBase) << i;
            } else {
                // Requiring safe features should succeed.
                ASSERT_NE(nullptr, deviceBase);
                ExpectDeviceHasRequiredFeatures(deviceBase);
                deviceBase->APIRelease();
            }
        }

        // Test with the adapter with AllowUnsafeApis toggles enabled, creating device should always
        // succeed.
        {
            native::DeviceBase* deviceBase = native::FromAPI(unsafeAdapterAllow.CreateDevice(
                reinterpret_cast<const WGPUDeviceDescriptor*>(&deviceDescriptor)));

            ASSERT_NE(nullptr, deviceBase);
            ExpectDeviceHasRequiredFeatures(deviceBase);
            deviceBase->APIRelease();
        }

        // Test with an adapter that disables both AllowUnsafeAPIs and AllowExperimentalAPIs:
        // only Safe + Stable features succeed.
        {
            native::DeviceBase* deviceBase = native::FromAPI(stableOnlyAdapter.CreateDevice(
                reinterpret_cast<const WGPUDeviceDescriptor*>(&deviceDescriptor)));

            if (requiredUnsafeFeature || requiredExperimentalFeature) {
                ASSERT_EQ(nullptr, deviceBase) << i;
            } else {
                ASSERT_NE(nullptr, deviceBase) << i;
                ExpectDeviceHasRequiredFeatures(deviceBase);
                deviceBase->APIRelease();
            }
        }
    }
}

}  // anonymous namespace
}  // namespace dawn
