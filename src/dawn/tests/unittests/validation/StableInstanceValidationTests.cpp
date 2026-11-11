// Copyright 2026 The Dawn & Tint Authors
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

#include <webgpu/webgpu_cpp.h>

#include <algorithm>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "dawn/native/DawnNative.h"
#include "src/dawn/tests/unittests/validation/ValidationTest.h"
#include "src/utils/numeric.h"

namespace dawn {
namespace {

using ::testing::HasSubstr;

// The ValidationTest instance, adapter, and device are created with allow_experimental_apis (and
// allow_unsafe_apis) disabled. A regular instance, adapter, and device are created next to them
// for comparison.
class StableInstanceValidationTest : public ValidationTest {
  protected:
    void SetUp() override {
        ValidationTest::SetUp();
        // The tests check dawn::native objects (toggles, feature info, error objects) directly.
        DAWN_SKIP_TEST_IF(UsesWire());

        mRegularInstance = wgpu::CreateInstance();
        ASSERT_NE(mRegularInstance, nullptr);
        mRegularAdapter = GetNullAdapter(mRegularInstance);
        ASSERT_NE(mRegularAdapter, nullptr);

        wgpu::DeviceDescriptor deviceDesc = {};
        wgpu::Adapter regularDeviceAdapter = GetNullAdapter(mRegularInstance);
        ASSERT_NE(regularDeviceAdapter, nullptr);
        ASSERT_EQ(RequestDeviceMessage(regularDeviceAdapter, &deviceDesc, &mRegularDevice), "");
        ASSERT_NE(mRegularDevice, nullptr);
    }

    void TearDown() override {
        mRegularDevice = nullptr;
        mRegularAdapter = nullptr;
        mRegularInstance = nullptr;
        ValidationTest::TearDown();
    }

    bool AllowUnsafeAPIs() override { return false; }
    bool AllowExperimentalAPIs() override { return false; }

    static wgpu::Instance CreateStableModeInstance(
        const wgpu::InstanceDescriptor* baseDesc = nullptr) {
        static const char* const kDisabled = "allow_experimental_apis";
        wgpu::DawnTogglesDescriptor togglesDesc = {};
        togglesDesc.disabledToggleCount = 1;
        togglesDesc.disabledToggles = &kDisabled;

        wgpu::InstanceDescriptor localDesc =
            baseDesc != nullptr ? *baseDesc : wgpu::InstanceDescriptor{};
        togglesDesc.nextInChain = localDesc.nextInChain;
        localDesc.nextInChain = &togglesDesc;
        return wgpu::CreateInstance(&localDesc);
    }

    static wgpu::Adapter GetNullAdapter(const wgpu::Instance& targetInstance) {
        wgpu::RequestAdapterOptions options = {};
        options.backendType = wgpu::BackendType::Null;
        wgpu::Adapter result;
        targetInstance.RequestAdapter(
            &options, wgpu::CallbackMode::AllowSpontaneous,
            [&](wgpu::RequestAdapterStatus status, wgpu::Adapter found, wgpu::StringView) {
                if (status == wgpu::RequestAdapterStatus::Success) {
                    result = std::move(found);
                }
            });
        return result;
    }

    static bool IsStableFeature(wgpu::FeatureName feature) {
        const native::FeatureInfo* info = native::GetFeatureInfo(feature);
        return info != nullptr &&
               info->featureStability == native::FeatureInfo::FeatureStability::Stable;
    }

    static bool HasToggle(const std::vector<const char*>& toggles, std::string_view name) {
        return std::ranges::any_of(toggles, [name](const char* t) { return t == name; });
    }
    static bool HasToggle(const wgpu::Instance& target, std::string_view name) {
        return HasToggle(native::GetTogglesUsed(target), name);
    }
    static bool HasToggle(const wgpu::Adapter& target, std::string_view name) {
        return HasToggle(native::GetTogglesUsed(target), name);
    }
    static bool HasToggle(const wgpu::Device& target, std::string_view name) {
        return HasToggle(native::GetTogglesUsed(target.Get()), name);
    }

    static std::string RequestDeviceMessage(const wgpu::Adapter& target,
                                            const wgpu::DeviceDescriptor* desc,
                                            wgpu::Device* outDevice = nullptr) {
        std::string result;
        bool called = false;
        target.RequestDevice(
            desc, wgpu::CallbackMode::AllowSpontaneous,
            [&](wgpu::RequestDeviceStatus status, wgpu::Device created, wgpu::StringView message) {
                called = true;
                if (status != wgpu::RequestDeviceStatus::Success) {
                    result = std::string(std::string_view(message));
                } else if (outDevice != nullptr) {
                    *outDevice = std::move(created);
                }
            });
        EXPECT_TRUE(called);
        return result;
    }

    wgpu::Instance mRegularInstance;
    wgpu::Adapter mRegularAdapter;
    wgpu::Device mRegularDevice;
};

// The two feature classifications are independent and come from different sources: Safe/Unsafe
// from Features.cpp, Stable/Experimental from the dawn_version.json stable_version annotations.
TEST_F(StableInstanceValidationTest, FeatureLabels) {
    // Upstream feature: Safe and Stable.
    const native::FeatureInfo* timestampInfo =
        native::GetFeatureInfo(wgpu::FeatureName::TimestampQuery);
    ASSERT_NE(timestampInfo, nullptr);
    EXPECT_EQ(timestampInfo->featureSafety, native::FeatureInfo::FeatureSafety::Safe);
    EXPECT_EQ(timestampInfo->featureStability, native::FeatureInfo::FeatureStability::Stable);

    // Dawn-only feature that is safe without allow_unsafe_apis: Safe and Experimental.
    const native::FeatureInfo* ahbInfo =
        native::GetFeatureInfo(wgpu::FeatureName::SharedTextureMemoryAHardwareBuffer);
    ASSERT_NE(ahbInfo, nullptr);
    EXPECT_EQ(ahbInfo->featureSafety, native::FeatureInfo::FeatureSafety::Safe);
    EXPECT_EQ(ahbInfo->featureStability, native::FeatureInfo::FeatureStability::Experimental);

    // Dawn-only feature that requires allow_unsafe_apis: Unsafe and Experimental.
    const native::FeatureInfo* insidePassesInfo =
        native::GetFeatureInfo(wgpu::FeatureName::ChromiumExperimentalTimestampQueryInsidePasses);
    ASSERT_NE(insidePassesInfo, nullptr);
    EXPECT_EQ(insidePassesInfo->featureSafety, native::FeatureInfo::FeatureSafety::Unsafe);
    EXPECT_EQ(insidePassesInfo->featureStability,
              native::FeatureInfo::FeatureStability::Experimental);
}

// A regular instance keeps allow_experimental_apis enabled by default and allow_unsafe_apis
// disabled, whereas an instance with allow_experimental_apis disabled propagates that state to its
// adapters and devices.
TEST_F(StableInstanceValidationTest, ToggleDefaults) {
    // The stable-mode instance has both allow_experimental_apis and allow_unsafe_apis disabled.
    EXPECT_FALSE(HasToggle(instance, "allow_experimental_apis"));
    EXPECT_FALSE(HasToggle(instance, "allow_unsafe_apis"));

    // A regular instance enables allow_experimental_apis by default and keeps allow_unsafe_apis
    // disabled.
    EXPECT_TRUE(HasToggle(mRegularInstance, "allow_experimental_apis"));
    EXPECT_FALSE(HasToggle(mRegularInstance, "allow_unsafe_apis"));

    // Adapters and devices inherit the disabled toggle state from the stable-mode instance.
    EXPECT_FALSE(HasToggle(adapter, "allow_experimental_apis"));
    EXPECT_FALSE(HasToggle(adapter, "allow_unsafe_apis"));
    EXPECT_FALSE(HasToggle(device, "allow_experimental_apis"));
    EXPECT_FALSE(HasToggle(device, "allow_unsafe_apis"));
}

// Instance features: unsupported values are rejected everywhere; Experimental ones
// (ShaderSourceSPIRV) are rejected when allow_experimental_apis is disabled.
TEST_F(StableInstanceValidationTest, RequiredInstanceFeatures) {
    // Stable instance features (TimedWaitAny, MultipleDevicesPerAdapter) are accepted on both
    // stable-mode and regular instances.
    {
        wgpu::InstanceFeatureName features[] = {
            wgpu::InstanceFeatureName::TimedWaitAny,
            wgpu::InstanceFeatureName::MultipleDevicesPerAdapter};
        wgpu::InstanceDescriptor desc = {};
        desc.requiredFeatureCount = 2;
        desc.requiredFeatures = features;
        EXPECT_NE(CreateStableModeInstance(&desc), nullptr);
        EXPECT_NE(wgpu::CreateInstance(&desc), nullptr);
    }
    // ShaderSourceSPIRV is a supported Dawn instance feature that is not part of the stable ABI,
    // so stable-mode instances reject it while regular instances accept it.
    {
        wgpu::InstanceFeatureName unstableFeature = wgpu::InstanceFeatureName::ShaderSourceSPIRV;
        wgpu::InstanceDescriptor desc = {};
        desc.requiredFeatureCount = 1;
        desc.requiredFeatures = &unstableFeature;
        EXPECT_EQ(CreateStableModeInstance(&desc), nullptr);
        EXPECT_NE(wgpu::CreateInstance(&desc), nullptr);
    }
    // Unknown instance feature enum values are rejected on both stable-mode and regular instances.
    {
        wgpu::InstanceFeatureName invalidFeature =
            static_cast<wgpu::InstanceFeatureName>(0x7FFFFFF0);
        wgpu::InstanceDescriptor desc = {};
        desc.requiredFeatureCount = 1;
        desc.requiredFeatures = &invalidFeature;
        EXPECT_EQ(CreateStableModeInstance(&desc), nullptr);
        EXPECT_EQ(wgpu::CreateInstance(&desc), nullptr);
    }
}

// An adapter created from a stable-mode instance neither reports nor accepts Experimental
// features, while an adapter on a regular instance continues reporting and accepting them.
TEST_F(StableInstanceValidationTest, AdapterHidesExperimentalFeatures) {
    // DawnInternalUsages is enabled on every physical device but is Experimental, whereas
    // CoreFeaturesAndLimits is Stable.
    EXPECT_TRUE(mRegularAdapter.HasFeature(wgpu::FeatureName::DawnInternalUsages));
    EXPECT_FALSE(adapter.HasFeature(wgpu::FeatureName::DawnInternalUsages));
    EXPECT_TRUE(adapter.HasFeature(wgpu::FeatureName::CoreFeaturesAndLimits));

    // GetFeatures on the stable adapter only returns Stable features.
    wgpu::SupportedFeatures stableFeatures;
    adapter.GetFeatures(&stableFeatures);
    EXPECT_GT(stableFeatures.featureCount, 0u);
    for (wgpu::FeatureName f : std::span(stableFeatures.features, stableFeatures.featureCount)) {
        EXPECT_TRUE(IsStableFeature(f)) << dawn::to_underlying(f);
    }

    wgpu::SupportedFeatures regularFeatures;
    mRegularAdapter.GetFeatures(&regularFeatures);
    EXPECT_GT(regularFeatures.featureCount, stableFeatures.featureCount);

    // Requesting an Experimental feature on the stable adapter fails with an explicit message
    // naming allow_experimental_apis.
    wgpu::FeatureName feature = wgpu::FeatureName::DawnInternalUsages;
    wgpu::DeviceDescriptor desc = {};
    desc.requiredFeatureCount = 1;
    desc.requiredFeatures = &feature;
    EXPECT_THAT(RequestDeviceMessage(adapter, &desc), HasSubstr("allow_experimental_apis"));
    EXPECT_EQ(RequestDeviceMessage(mRegularAdapter, &desc), "");
}

}  // anonymous namespace
}  // namespace dawn
