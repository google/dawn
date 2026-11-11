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
using ::testing::Not;

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

// A regular instance that explicitly disables allow_experimental_apis accepts only the toggles
// descriptor on InstanceDescriptor and rejects any additional chained structs.
TEST_F(StableInstanceValidationTest, RegularInstanceWithExperimentalAPIsDisabled) {
    const char* kToggle = "allow_experimental_apis";
    wgpu::DawnTogglesDescriptor togglesDesc = {};
    togglesDesc.disabledToggleCount = 1;
    togglesDesc.disabledToggles = &kToggle;

    wgpu::InstanceDescriptor desc = {};
    desc.nextInChain = &togglesDesc;
    wgpu::Instance disabledInstance = wgpu::CreateInstance(&desc);
    ASSERT_NE(disabledInstance, nullptr);
    EXPECT_FALSE(HasToggle(disabledInstance, "allow_experimental_apis"));

    wgpu::Adapter disabledAdapter = GetNullAdapter(disabledInstance);
    ASSERT_NE(disabledAdapter, nullptr);
    EXPECT_FALSE(disabledAdapter.HasFeature(wgpu::FeatureName::DawnInternalUsages));

    // Co-chaining DawnInstanceDescriptor when allow_experimental_apis is disabled is rejected.
    native::DawnInstanceDescriptor dawnDesc = {};
    togglesDesc.nextInChain = &dawnDesc;
    EXPECT_EQ(wgpu::CreateInstance(&desc), nullptr);
}

// When allow_experimental_apis is disabled, Experimental chained structs on RequestAdapterOptions
// cause adapter enumeration and RequestAdapter to fail, preventing toggles from being re-enabled.
TEST_F(StableInstanceValidationTest, RequestAdapterRejectsExperimentalChain) {
    const char* kToggle = "allow_experimental_apis";
    wgpu::DawnTogglesDescriptor togglesDesc = {};
    togglesDesc.enabledToggleCount = 1;
    togglesDesc.enabledToggles = &kToggle;

    wgpu::RequestAdapterOptions options = {};
    options.backendType = wgpu::BackendType::Null;
    options.nextInChain = &togglesDesc;

    // RequestAdapter with DawnTogglesDescriptor fails on the stable-mode instance.
    bool stableCalled = false;
    instance.RequestAdapter(
        &options, wgpu::CallbackMode::AllowSpontaneous,
        [&](wgpu::RequestAdapterStatus status, wgpu::Adapter found, wgpu::StringView) {
            stableCalled = true;
            EXPECT_NE(status, wgpu::RequestAdapterStatus::Success);
            EXPECT_EQ(found, nullptr);
        });
    EXPECT_TRUE(stableCalled);

    // The same RequestAdapterOptions succeeds on a regular instance.
    bool regularCalled = false;
    mRegularInstance.RequestAdapter(
        &options, wgpu::CallbackMode::AllowSpontaneous,
        [&](wgpu::RequestAdapterStatus status, wgpu::Adapter found, wgpu::StringView) {
            regularCalled = true;
            EXPECT_EQ(status, wgpu::RequestAdapterStatus::Success);
            EXPECT_NE(found, nullptr);
        });
    EXPECT_TRUE(regularCalled);

    // Without the chained struct, the stable-mode instance finds the adapter.
    EXPECT_NE(GetNullAdapter(instance), nullptr);
}

// On a stable-mode instance, Experimental chained structs on DeviceDescriptor (including
// DawnTogglesDescriptor and DawnCacheDeviceDescriptor) are rejected at device creation.
TEST_F(StableInstanceValidationTest, RequestDeviceRejectsExperimentalChains) {
    // Chaining DawnTogglesDescriptor on DeviceDescriptor is rejected on the stable adapter.
    {
        const char* kToggles[] = {"allow_experimental_apis", "allow_unsafe_apis"};
        wgpu::DawnTogglesDescriptor togglesDesc = {};
        togglesDesc.enabledToggleCount = 2;
        togglesDesc.enabledToggles = kToggles;
        wgpu::DeviceDescriptor desc = {};
        desc.nextInChain = &togglesDesc;

        EXPECT_THAT(RequestDeviceMessage(adapter, &desc), HasSubstr("allow_experimental_apis"));

        wgpu::Device regularDevice;
        EXPECT_EQ(RequestDeviceMessage(mRegularAdapter, &desc, &regularDevice), "");
        ASSERT_NE(regularDevice, nullptr);
        EXPECT_TRUE(HasToggle(regularDevice, "allow_unsafe_apis"));
    }
    // Chaining DawnCacheDeviceDescriptor on DeviceDescriptor is rejected on the stable adapter.
    {
        wgpu::DawnCacheDeviceDescriptor cacheDesc = {};
        wgpu::DeviceDescriptor desc = {};
        desc.nextInChain = &cacheDesc;
        EXPECT_THAT(RequestDeviceMessage(adapter, &desc), HasSubstr("allow_experimental_apis"));

        wgpu::Adapter secondRegularAdapter = GetNullAdapter(mRegularInstance);
        ASSERT_NE(secondRegularAdapter, nullptr);
        EXPECT_EQ(RequestDeviceMessage(secondRegularAdapter, &desc), "");
    }
}

// Surface creation on a stable-mode instance rejects Dawn-only surface descriptors.
TEST_F(StableInstanceValidationTest, SurfaceDescriptorValidation) {
    wgpu::SurfaceDescriptorFromWindowsCoreWindow coreWindow = {};
    wgpu::SurfaceDescriptor desc = {};
    desc.nextInChain = &coreWindow;

    wgpu::Surface stableSurface = instance.CreateSurface(&desc);
    EXPECT_TRUE(native::CheckIsErrorForTesting(stableSurface.Get()));
    wgpu::SurfaceCapabilities caps = {};
    EXPECT_EQ(stableSurface.GetCapabilities(adapter, &caps), wgpu::Status::Error);
}

// Runtime checks on stable-mode vs. regular adapters and devices for AdapterInfo, Limits,
// SamplerDescriptor, and ShaderModuleDescriptor chains.
TEST_F(StableInstanceValidationTest, DeviceAndAdapterChains) {
    // Adapter::GetInfo rejects Experimental chained structs on a stable adapter while accepting
    // them on a regular adapter.
    {
        wgpu::DawnAdapterPropertiesPowerPreference powerPref = {};
        wgpu::AdapterInfo info = {};
        info.nextInChain = &powerPref;
        EXPECT_EQ(adapter.GetInfo(&info), wgpu::Status::Error);
        EXPECT_EQ(mRegularAdapter.GetInfo(&info), wgpu::Status::Success);
    }

    // Adapter::GetLimits and Device::GetLimits accept Stable chained limits structs
    // (CompatibilityModeLimits) and reject Experimental ones
    // (DawnTexelCopyBufferRowAlignmentLimits) on a stable instance, while accepting both on a
    // regular instance.
    {
        wgpu::CompatibilityModeLimits compatLimits = {};
        wgpu::Limits stableLimits = {};
        stableLimits.nextInChain = &compatLimits;
        EXPECT_EQ(adapter.GetLimits(&stableLimits), wgpu::Status::Success);
        EXPECT_EQ(device.GetLimits(&stableLimits), wgpu::Status::Success);

        wgpu::DawnTexelCopyBufferRowAlignmentLimits dawnLimits = {};
        wgpu::Limits expLimits = {};
        expLimits.nextInChain = &dawnLimits;
        EXPECT_EQ(adapter.GetLimits(&expLimits), wgpu::Status::Error);
        EXPECT_EQ(device.GetLimits(&expLimits), wgpu::Status::Error);
        EXPECT_EQ(mRegularAdapter.GetLimits(&expLimits), wgpu::Status::Success);
        EXPECT_EQ(mRegularDevice.GetLimits(&expLimits), wgpu::Status::Success);
    }

    // SamplerDescriptor chaining YCbCrVkDescriptor is rejected with allow_experimental_apis
    // disabled before reaching feature checks.
    {
        wgpu::YCbCrVkDescriptor ycbcr = {};
        wgpu::SamplerDescriptor samplerDesc = {};
        samplerDesc.nextInChain = &ycbcr;
        ASSERT_DEVICE_ERROR(device.CreateSampler(&samplerDesc),
                            HasSubstr("allow_experimental_apis"));
    }

    // ShaderModuleDescriptor accepts ShaderSourceWGSL and rejects ShaderSourceSPIRV on a stable
    // device.
    {
        wgpu::ShaderSourceWGSL wgsl = {};
        wgsl.code = "@compute @workgroup_size(1) fn main() {}";
        wgpu::ShaderModuleDescriptor wgslDesc = {};
        wgslDesc.nextInChain = &wgsl;
        device.CreateShaderModule(&wgslDesc);

        static constexpr uint32_t kDummySpirv[] = {0x07230203, 0x00010000, 0, 1, 0};
        wgpu::ShaderSourceSPIRV spirv = {};
        spirv.codeSize = 5;
        spirv.code = kDummySpirv;
        wgpu::ShaderModuleDescriptor spirvDesc = {};
        spirvDesc.nextInChain = &spirv;
        ASSERT_DEVICE_ERROR(device.CreateShaderModule(&spirvDesc),
                            HasSubstr("allow_experimental_apis"));
    }
}

// InstanceLimits and SurfaceCapabilities reject unexpected chained structs.
TEST_F(StableInstanceValidationTest, InstanceLimitsAndSurfaceCapabilitiesChainsRejected) {
    // InstanceDescriptor.requiredLimits with a non-null nextInChain is rejected.
    {
        wgpu::ChainedStructOut dummyChain = {};
        dummyChain.sType = wgpu::SType::AdapterPropertiesMemoryHeaps;
        wgpu::InstanceLimits limits = {};
        limits.nextInChain = &dummyChain;
        wgpu::InstanceDescriptor desc = {};
        desc.requiredLimits = &limits;
        EXPECT_EQ(CreateStableModeInstance(&desc), nullptr);
        EXPECT_EQ(wgpu::CreateInstance(&desc), nullptr);
    }

    // Create a surface and configure it with the device so surface validation errors are reported
    // to the device.
    wgpu::SurfaceDescriptor emptySurfaceDesc = {};
    wgpu::Surface surface = instance.CreateSurface(&emptySurfaceDesc);
    ASSERT_NE(surface, nullptr);

    wgpu::SurfaceConfiguration config = {};
    config.device = device;
    config.format = wgpu::TextureFormat::RGBA8Unorm;
    config.usage = wgpu::TextureUsage::RenderAttachment;
    config.width = 4;
    config.height = 4;
    config.presentMode = wgpu::PresentMode::Fifo;
    config.alphaMode = wgpu::CompositeAlphaMode::Opaque;
    ASSERT_DEVICE_ERROR(surface.Configure(&config));

    wgpu::ChainedStructOut badOutChain = {};
    badOutChain.sType = wgpu::SType::AdapterPropertiesMemoryHeaps;
    wgpu::SurfaceCapabilities caps = {};
    caps.nextInChain = &badOutChain;
    ASSERT_DEVICE_ERROR(EXPECT_EQ(surface.GetCapabilities(adapter, &caps), wgpu::Status::Error),
                        HasSubstr("Unexpected chained struct"));
}

using RegularInstanceValidationTest = ValidationTest;

// On a regular instance the same YCbCrVkDescriptor chain is not rejected for stability: it only
// fails the feature check.
TEST_F(RegularInstanceValidationTest, SamplerChainIsNotCheckedForStability) {
    wgpu::YCbCrVkDescriptor ycbcr = {};
    wgpu::SamplerDescriptor samplerDesc = {};
    samplerDesc.nextInChain = &ycbcr;
    ASSERT_DEVICE_ERROR(device.CreateSampler(&samplerDesc),
                        Not(HasSubstr("allow_experimental_apis")));
}

}  // anonymous namespace
}  // namespace dawn
