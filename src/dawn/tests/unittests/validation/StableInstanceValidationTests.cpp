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

WGPUStringView ToStringView(const char* s) {
    return {s, WGPU_STRLEN};
}

// The free functions of the stable table, resolved through GetStableProcAddress.
struct StableProcs {
    WGPUProcCreateInstance createInstance;
    WGPUProcGetInstanceFeatures getInstanceFeatures;
    WGPUProcHasInstanceFeature hasInstanceFeature;
    WGPUProcGetProcAddress getProcAddress;
};

StableProcs GetStableProcs() {
    StableProcs procs;
    procs.createInstance = reinterpret_cast<WGPUProcCreateInstance>(
        native::GetStableProcAddress(ToStringView("wgpuCreateInstance")));
    procs.getInstanceFeatures = reinterpret_cast<WGPUProcGetInstanceFeatures>(
        native::GetStableProcAddress(ToStringView("wgpuGetInstanceFeatures")));
    procs.hasInstanceFeature = reinterpret_cast<WGPUProcHasInstanceFeature>(
        native::GetStableProcAddress(ToStringView("wgpuHasInstanceFeature")));
    procs.getProcAddress = reinterpret_cast<WGPUProcGetProcAddress>(
        native::GetStableProcAddress(ToStringView("wgpuGetProcAddress")));
    return procs;
}

// The ValidationTest instance, adapter, and device are created with allow_experimental_apis (and
// allow_unsafe_apis) disabled. A regular instance, adapter, and device are created next to them
// for comparison.
class StableInstanceValidationTest : public ValidationTest {
  protected:
    void SetUp() override {
        ValidationTest::SetUp();
        // The tests check dawn::native objects (toggles, feature info, error objects) directly.
        DAWN_SKIP_TEST_IF(UsesWire());

        mStableProcs = GetStableProcs();
        ASSERT_NE(mStableProcs.createInstance, nullptr);
        ASSERT_NE(mStableProcs.getInstanceFeatures, nullptr);
        ASSERT_NE(mStableProcs.hasInstanceFeature, nullptr);
        ASSERT_NE(mStableProcs.getProcAddress, nullptr);

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

    // Creates an instance through the stable wgpuCreateInstance entry point.
    wgpu::Instance CreateStableInstance(const wgpu::InstanceDescriptor* desc = nullptr) const {
        return wgpu::Instance::Acquire(
            mStableProcs.createInstance(reinterpret_cast<const WGPUInstanceDescriptor*>(desc)));
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

    StableProcs mStableProcs;
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

// A regular instance keeps allow_experimental_apis enabled by default, allow_unsafe_apis
// disabled, and webview_instance unset, whereas a stable instance disables
// allow_experimental_apis and allow_unsafe_apis, enables webview_instance, and propagates that
// state to its adapters and devices.
TEST_F(StableInstanceValidationTest, ToggleDefaults) {
    // A stable instance disables allow_experimental_apis and allow_unsafe_apis and enables
    // webview_instance.
    wgpu::Instance stable = CreateStableInstance();
    ASSERT_NE(stable, nullptr);
    EXPECT_FALSE(HasToggle(stable, "allow_experimental_apis"));
    EXPECT_FALSE(HasToggle(stable, "allow_unsafe_apis"));
    EXPECT_TRUE(HasToggle(stable, "webview_instance"));

    // A regular instance enables allow_experimental_apis by default, keeps allow_unsafe_apis
    // disabled, and leaves webview_instance unset.
    EXPECT_TRUE(HasToggle(mRegularInstance, "allow_experimental_apis"));
    EXPECT_FALSE(HasToggle(mRegularInstance, "allow_unsafe_apis"));
    EXPECT_FALSE(HasToggle(mRegularInstance, "webview_instance"));

    // Adapters and devices inherit the toggle state from the stable instance.
    wgpu::Adapter stableAdapter = GetNullAdapter(stable);
    ASSERT_NE(stableAdapter, nullptr);
    EXPECT_FALSE(HasToggle(stableAdapter, "allow_experimental_apis"));
    EXPECT_FALSE(HasToggle(stableAdapter, "allow_unsafe_apis"));
    EXPECT_TRUE(HasToggle(stableAdapter, "webview_instance"));

    wgpu::Device stableDevice;
    wgpu::DeviceDescriptor deviceDesc = {};
    EXPECT_EQ(RequestDeviceMessage(stableAdapter, &deviceDesc, &stableDevice), "");
    ASSERT_NE(stableDevice, nullptr);
    EXPECT_FALSE(HasToggle(stableDevice, "allow_experimental_apis"));
    EXPECT_FALSE(HasToggle(stableDevice, "allow_unsafe_apis"));
    EXPECT_TRUE(HasToggle(stableDevice, "webview_instance"));
}

// Regular instances can opt in to webview_instance for testing temporary workarounds, whereas
// stable instances reject attempts to disable webview_instance at instance, adapter, or device
// creation.
TEST_F(StableInstanceValidationTest, WebViewInstanceToggle) {
    const char* kWebViewToggle = "webview_instance";

    // Opting in via DawnTogglesDescriptor on a regular instance propagates webview_instance to
    // adapters and devices.
    {
        wgpu::DawnTogglesDescriptor togglesDesc = {};
        togglesDesc.enabledToggleCount = 1;
        togglesDesc.enabledToggles = &kWebViewToggle;

        wgpu::InstanceDescriptor instanceDesc = {};
        instanceDesc.nextInChain = &togglesDesc;
        wgpu::Instance optInInstance = wgpu::CreateInstance(&instanceDesc);
        ASSERT_NE(optInInstance, nullptr);
        EXPECT_TRUE(HasToggle(optInInstance, "webview_instance"));

        wgpu::Adapter optInAdapter = GetNullAdapter(optInInstance);
        ASSERT_NE(optInAdapter, nullptr);
        EXPECT_TRUE(HasToggle(optInAdapter, "webview_instance"));

        wgpu::Device optInDevice;
        wgpu::DeviceDescriptor deviceDesc = {};
        EXPECT_EQ(RequestDeviceMessage(optInAdapter, &deviceDesc, &optInDevice), "");
        ASSERT_NE(optInDevice, nullptr);
        EXPECT_TRUE(HasToggle(optInDevice, "webview_instance"));
    }

    // Attempting to disable webview_instance on a stable instance, adapter, or device is rejected.
    {
        wgpu::DawnTogglesDescriptor disableDesc = {};
        disableDesc.disabledToggleCount = 1;
        disableDesc.disabledToggles = &kWebViewToggle;

        wgpu::InstanceDescriptor instanceDesc = {};
        instanceDesc.nextInChain = &disableDesc;
        EXPECT_EQ(CreateStableInstance(&instanceDesc), nullptr);

        wgpu::Instance stable = CreateStableInstance();
        ASSERT_NE(stable, nullptr);

        wgpu::RequestAdapterOptions adapterOpts = {};
        adapterOpts.backendType = wgpu::BackendType::Null;
        adapterOpts.nextInChain = &disableDesc;
        wgpu::Adapter rejectedAdapter;
        bool called = false;
        stable.RequestAdapter(
            &adapterOpts, wgpu::CallbackMode::AllowSpontaneous,
            [&](wgpu::RequestAdapterStatus status, wgpu::Adapter found, wgpu::StringView) {
                called = true;
                if (status == wgpu::RequestAdapterStatus::Success) {
                    rejectedAdapter = std::move(found);
                }
            });
        EXPECT_TRUE(called);
        EXPECT_EQ(rejectedAdapter, nullptr);

        wgpu::Adapter stableAdapter = GetNullAdapter(stable);
        ASSERT_NE(stableAdapter, nullptr);
        wgpu::DeviceDescriptor deviceDesc = {};
        deviceDesc.nextInChain = &disableDesc;
        EXPECT_NE(RequestDeviceMessage(stableAdapter, &deviceDesc), "");
    }
}

// Instance features: unsupported values are rejected everywhere; Experimental ones
// (ShaderSourceSPIRV) are rejected when allow_experimental_apis is disabled.
TEST_F(StableInstanceValidationTest, RequiredInstanceFeatures) {
    // Stable instance features (TimedWaitAny, MultipleDevicesPerAdapter) are accepted on both
    // stable and regular instances.
    {
        wgpu::InstanceFeatureName features[] = {
            wgpu::InstanceFeatureName::TimedWaitAny,
            wgpu::InstanceFeatureName::MultipleDevicesPerAdapter};
        wgpu::InstanceDescriptor desc = {};
        desc.requiredFeatureCount = 2;
        desc.requiredFeatures = features;
        EXPECT_NE(CreateStableInstance(&desc), nullptr);
        EXPECT_NE(wgpu::CreateInstance(&desc), nullptr);
    }
    // ShaderSourceSPIRV is a supported Dawn instance feature that is not part of the stable ABI,
    // so stable instances reject it while regular instances accept it.
    {
        wgpu::InstanceFeatureName unstableFeature = wgpu::InstanceFeatureName::ShaderSourceSPIRV;
        wgpu::InstanceDescriptor desc = {};
        desc.requiredFeatureCount = 1;
        desc.requiredFeatures = &unstableFeature;
        EXPECT_EQ(CreateStableInstance(&desc), nullptr);
        EXPECT_NE(wgpu::CreateInstance(&desc), nullptr);
    }
    // Unknown instance feature enum values are rejected on both stable and regular instances.
    {
        wgpu::InstanceFeatureName invalidFeature =
            static_cast<wgpu::InstanceFeatureName>(0x7FFFFFF0);
        wgpu::InstanceDescriptor desc = {};
        desc.requiredFeatureCount = 1;
        desc.requiredFeatures = &invalidFeature;
        EXPECT_EQ(CreateStableInstance(&desc), nullptr);
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

    // SAFETY: features always points to at least featureCount entries.
    std::span<const wgpu::FeatureName> DAWN_UNSAFE_BUFFERS(
        stableFeaturesSpan(stableFeatures.features, stableFeatures.featureCount));
    for (wgpu::FeatureName f : stableFeaturesSpan) {
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
    EXPECT_FALSE(HasToggle(disabledInstance, "webview_instance"));

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
        EXPECT_EQ(CreateStableInstance(&desc), nullptr);
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

// The stable proc map is a strict subset of the full proc map and resolves every entry to the
// same function as the full table (except for the stable free-function overrides).
TEST_F(StableInstanceValidationTest, StableProcMapIsSubsetOfProcMap) {
    std::vector<std::string_view> stable = native::GetStableProcMapNamesForTesting();
    EXPECT_FALSE(stable.empty());

    for (std::string_view name : stable) {
        WGPUStringView sv = {name.data(), name.size()};
        WGPUProc stableProc = native::GetStableProcAddress(sv);
        EXPECT_NE(stableProc, nullptr) << name;
        EXPECT_NE(native::GetProcs().getProcAddress(sv), nullptr) << name;
        EXPECT_EQ(stableProc, mStableProcs.getProcAddress(sv)) << name;
    }
    // Dawn-only entry points exist in the full table but not in the stable one.
    WGPUStringView dawnOnly = {"wgpuDeviceCreateErrorBuffer", WGPU_STRLEN};
    EXPECT_NE(native::GetProcs().getProcAddress(dawnOnly), nullptr);
    EXPECT_EQ(native::GetStableProcAddress(dawnOnly), nullptr);
}

// Entry points in Dawn's stable C header resolve through GetStableProcAddress; Dawn-only entry
// points, including Dawn-only methods on otherwise stable objects, resolve to nullptr.
TEST_F(StableInstanceValidationTest, StableProcMapContents) {
    const char* kStable[] = {
        "wgpuInstanceRequestAdapter", "wgpuAdapterRequestDevice",
        "wgpuDeviceCreateBuffer",     "wgpuQueueSubmit",
        "wgpuAdapterInfoFreeMembers",
    };
    for (const char* name : kStable) {
        EXPECT_NE(native::GetStableProcAddress(ToStringView(name)), nullptr) << name;
    }

    const char* kExperimental[] = {
        "wgpuDeviceTick",
        "wgpuDeviceCreateErrorBuffer",
    };
    for (const char* name : kExperimental) {
        EXPECT_NE(native::GetProcs().getProcAddress(ToStringView(name)), nullptr) << name;
        EXPECT_EQ(native::GetStableProcAddress(ToStringView(name)), nullptr) << name;
        EXPECT_EQ(mStableProcs.getProcAddress(ToStringView(name)), nullptr) << name;
    }

    // Invalid inputs and length-delimited string views.
    EXPECT_EQ(native::GetStableProcAddress({nullptr, WGPU_STRLEN}), nullptr);
    EXPECT_EQ(native::GetStableProcAddress({nullptr, 0}), nullptr);
    EXPECT_EQ(native::GetStableProcAddress(ToStringView("")), nullptr);
    EXPECT_EQ(native::GetStableProcAddress(ToStringView("wgpuDeviceDoSomething")), nullptr);
    EXPECT_EQ(native::GetStableProcAddress(ToStringView("zzzzzzz")), nullptr);
    EXPECT_EQ(native::GetStableProcAddress({"wgpuQueueSubmitExtraString", 15}),
              reinterpret_cast<WGPUProc>(native::GetProcs().queueSubmit));
    EXPECT_EQ(native::GetStableProcAddress({"wgpuQueueSubmit", 14}), nullptr);
}

// wgpuGetProcAddress resolves to the stable resolver itself, and wgpuCreateInstance /
// wgpuGetInstanceFeatures / wgpuHasInstanceFeature resolve to their stable variants.
TEST_F(StableInstanceValidationTest, StableFreeStandingFunctions) {
    WGPUProc stableGetProcAddress =
        native::GetStableProcAddress(ToStringView("wgpuGetProcAddress"));
    ASSERT_NE(stableGetProcAddress, nullptr);
    EXPECT_NE(stableGetProcAddress, reinterpret_cast<WGPUProc>(native::GetProcs().getProcAddress));
    EXPECT_EQ(mStableProcs.getProcAddress(ToStringView("wgpuGetProcAddress")),
              stableGetProcAddress);

    EXPECT_NE(reinterpret_cast<WGPUProc>(mStableProcs.createInstance),
              reinterpret_cast<WGPUProc>(native::GetProcs().createInstance));
    EXPECT_NE(reinterpret_cast<WGPUProc>(mStableProcs.getInstanceFeatures),
              reinterpret_cast<WGPUProc>(native::GetProcs().getInstanceFeatures));
    EXPECT_NE(reinterpret_cast<WGPUProc>(mStableProcs.hasInstanceFeature),
              reinterpret_cast<WGPUProc>(native::GetProcs().hasInstanceFeature));
}

// Stable instance feature queries only report features that are part of Dawn's stable ABI:
// ShaderSourceSPIRV is hidden while TimedWaitAny and MultipleDevicesPerAdapter are reported.
TEST_F(StableInstanceValidationTest, StableInstanceFeatureQueries) {
    EXPECT_TRUE(mStableProcs.hasInstanceFeature(WGPUInstanceFeatureName_TimedWaitAny));
    EXPECT_TRUE(mStableProcs.hasInstanceFeature(WGPUInstanceFeatureName_MultipleDevicesPerAdapter));
    EXPECT_FALSE(mStableProcs.hasInstanceFeature(WGPUInstanceFeatureName_ShaderSourceSPIRV));
    EXPECT_TRUE(native::GetProcs().hasInstanceFeature(WGPUInstanceFeatureName_ShaderSourceSPIRV));

    WGPUSupportedInstanceFeatures stableFeatures = {};
    mStableProcs.getInstanceFeatures(&stableFeatures);
    WGPUSupportedInstanceFeatures allFeatures = {};
    native::GetProcs().getInstanceFeatures(&allFeatures);
    EXPECT_LT(stableFeatures.featureCount, allFeatures.featureCount);

    // SAFETY: features always points to at least featureCount entries.
    std::span<const WGPUInstanceFeatureName> DAWN_UNSAFE_BUFFERS(
        stableFeaturesSpan(stableFeatures.features, stableFeatures.featureCount));
    for (WGPUInstanceFeatureName f : stableFeaturesSpan) {
        EXPECT_NE(f, WGPUInstanceFeatureName_ShaderSourceSPIRV);
        EXPECT_TRUE(mStableProcs.hasInstanceFeature(f));
        EXPECT_TRUE(native::GetProcs().hasInstanceFeature(f));
    }

    // SAFETY: features always points to at least featureCount entries.
    std::span<const WGPUInstanceFeatureName> DAWN_UNSAFE_BUFFERS(
        allFeaturesSpan(allFeatures.features, allFeatures.featureCount));
    for (WGPUInstanceFeatureName f : allFeaturesSpan) {
        const bool inStableList =
            std::ranges::find(stableFeaturesSpan, f) != stableFeaturesSpan.end();
        EXPECT_EQ(inStableList, static_cast<bool>(mStableProcs.hasInstanceFeature(f)));
    }
    native::GetProcs().supportedInstanceFeaturesFreeMembers(stableFeatures);
    native::GetProcs().supportedInstanceFeaturesFreeMembers(allFeatures);
}

// A stable instance rejects external chained structs on InstanceDescriptor (including
// DawnTogglesDescriptor, DawnInstanceDescriptor, and DawnWGSLBlocklist), while a regular instance
// continues accepting them.
TEST_F(StableInstanceValidationTest, CreateStableInstanceRejectsChainedStructs) {
    const char* kToggles[] = {"allow_experimental_apis", "allow_unsafe_apis"};
    wgpu::DawnTogglesDescriptor togglesDesc = {};
    togglesDesc.enabledToggleCount = 2;
    togglesDesc.enabledToggles = kToggles;

    // Chaining DawnTogglesDescriptor on InstanceDescriptor is rejected by CreateStableInstance.
    {
        wgpu::InstanceDescriptor desc = {};
        desc.nextInChain = &togglesDesc;
        EXPECT_EQ(CreateStableInstance(&desc), nullptr);

        wgpu::Instance regular = wgpu::CreateInstance(&desc);
        ASSERT_NE(regular, nullptr);
        EXPECT_TRUE(HasToggle(regular, "allow_experimental_apis"));
        EXPECT_TRUE(HasToggle(regular, "allow_unsafe_apis"));
    }
    // Chaining an empty DawnTogglesDescriptor on InstanceDescriptor is also rejected by
    // CreateStableInstance.
    {
        wgpu::DawnTogglesDescriptor emptyTogglesDesc = {};
        wgpu::InstanceDescriptor desc = {};
        desc.nextInChain = &emptyTogglesDesc;
        EXPECT_EQ(CreateStableInstance(&desc), nullptr);
        EXPECT_NE(wgpu::CreateInstance(&desc), nullptr);
    }
    // Chaining other Dawn-specific structs (DawnInstanceDescriptor, DawnWGSLBlocklist) on
    // InstanceDescriptor is also rejected by CreateStableInstance.
    {
        native::DawnInstanceDescriptor dawnDesc = {};
        wgpu::InstanceDescriptor desc = {};
        desc.nextInChain = &dawnDesc;
        EXPECT_EQ(CreateStableInstance(&desc), nullptr);
        EXPECT_NE(wgpu::CreateInstance(&desc), nullptr);
    }
    {
        wgpu::DawnWGSLBlocklist blocklist = {};
        wgpu::InstanceDescriptor desc = {};
        desc.nextInChain = &blocklist;
        EXPECT_EQ(CreateStableInstance(&desc), nullptr);
        EXPECT_NE(wgpu::CreateInstance(&desc), nullptr);
    }
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
