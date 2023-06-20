// Copyright 2023 The Dawn Authors
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
#include "dawn/native/Adapter.h"
#include "dawn/native/DawnNative.h"
#include "dawn/native/Device.h"
#include "dawn/native/Instance.h"
#include "dawn/native/Toggles.h"
#include "dawn/native/dawn_platform.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/utils/SystemUtils.h"
#include "dawn/utils/WGPUHelpers.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

using testing::Contains;
using testing::MockCallback;
using testing::NotNull;
using testing::SaveArg;
using testing::StrEq;

class InstanceToggleTest : public testing::Test {
  protected:
    void SetUp() override { dawnProcSetProcs(&native::GetProcs()); }

    void TearDown() override { dawnProcSetProcs(nullptr); }
};

// Test that instance toggles are set by requirement or default as expected.
TEST_F(InstanceToggleTest, InstanceTogglesSet) {
    auto validateInstanceToggles = [](const native::Instance* nativeInstance,
                                      std::initializer_list<const char*> enableToggles,
                                      std::initializer_list<const char*> disableToggles) {
        const native::InstanceBase* instance = native::FromAPI(nativeInstance->Get());
        const native::TogglesState& instanceTogglesState = instance->GetTogglesState();
        std::vector<const char*> enabledToggles = instanceTogglesState.GetEnabledToggleNames();
        std::vector<const char*> disabledToggles = instanceTogglesState.GetDisabledToggleNames();
        EXPECT_EQ(disabledToggles.size(), disableToggles.size());
        EXPECT_EQ(enabledToggles.size(), enableToggles.size());
        for (auto* enableToggle : enableToggles) {
            EXPECT_THAT(enabledToggles, Contains(StrEq(enableToggle)));
        }
        for (auto* disableToggle : disableToggles) {
            EXPECT_THAT(disabledToggles, Contains(StrEq(disableToggle)));
        }
    };

    // Create instance with no toggles descriptor
    {
        std::unique_ptr<native::Instance> instance;

        // Create an instance with default toggles, where AllowUnsafeAPIs is disabled.
        instance = std::make_unique<native::Instance>();
        validateInstanceToggles(instance.get(), {}, {"allow_unsafe_apis"});
    }

    // Create instance with empty toggles descriptor
    {
        std::unique_ptr<native::Instance> instance;

        // Make an instance descriptor chaining an empty toggles descriptor
        WGPUDawnTogglesDescriptor instanceTogglesDesc = {};
        instanceTogglesDesc.chain.sType = WGPUSType::WGPUSType_DawnTogglesDescriptor;

        WGPUInstanceDescriptor instanceDesc = {};
        instanceDesc.nextInChain = &instanceTogglesDesc.chain;

        // Create an instance with default toggles, where AllowUnsafeAPIs is disabled and
        // DisallowUnsafeAPIs is enabled.
        instance = std::make_unique<native::Instance>(&instanceDesc);
        validateInstanceToggles(instance.get(), {}, {"allow_unsafe_apis"});
    }

    // Create instance with AllowUnsafeAPIs explicitly enabled in toggles descriptor
    {
        std::unique_ptr<native::Instance> instance;

        const char* allowUnsafeApisToggle = "allow_unsafe_apis";
        WGPUDawnTogglesDescriptor instanceTogglesDesc = {};
        instanceTogglesDesc.chain.sType = WGPUSType::WGPUSType_DawnTogglesDescriptor;
        instanceTogglesDesc.enabledTogglesCount = 1;
        instanceTogglesDesc.enabledToggles = &allowUnsafeApisToggle;

        WGPUInstanceDescriptor instanceDesc = {};
        instanceDesc.nextInChain = &instanceTogglesDesc.chain;

        // Create an instance with AllowUnsafeAPIs explicitly enabled.
        instance = std::make_unique<native::Instance>(&instanceDesc);
        validateInstanceToggles(instance.get(), {allowUnsafeApisToggle}, {});
    }

    // Create instance with AllowUnsafeAPIs explicitly disabled in toggles descriptor
    {
        std::unique_ptr<native::Instance> instance;

        const char* allowUnsafeApisToggle = "allow_unsafe_apis";
        WGPUDawnTogglesDescriptor instanceTogglesDesc = {};
        instanceTogglesDesc.chain.sType = WGPUSType::WGPUSType_DawnTogglesDescriptor;
        instanceTogglesDesc.disabledTogglesCount = 1;
        instanceTogglesDesc.disabledToggles = &allowUnsafeApisToggle;

        WGPUInstanceDescriptor instanceDesc = {};
        instanceDesc.nextInChain = &instanceTogglesDesc.chain;

        // Create an instance with AllowUnsafeAPIs explicitly disabled.
        instance = std::make_unique<native::Instance>(&instanceDesc);
        validateInstanceToggles(instance.get(), {}, {allowUnsafeApisToggle});
    }
}

// Test that instance toggles are inherited to the adapters and devices it creates.
TEST_F(InstanceToggleTest, InstanceTogglesInheritToAdapterAndDevice) {
    auto validateInstanceTogglesInheritedToAdapter = [&](native::Instance* nativeInstance) {
        native::InstanceBase* instance = native::FromAPI(nativeInstance->Get());
        const native::TogglesState& instanceTogglesState = instance->GetTogglesState();

        native::RequestAdapterOptionsBackendType backendTypeOptions = {};
        backendTypeOptions.backendType = wgpu::BackendType::Null;

        native::RequestAdapterOptions options = {};
        options.nextInChain = &backendTypeOptions;

        // Get the null adapter with default toggles.
        Ref<native::AdapterBase> nullAdapter = instance->EnumerateAdapters(&options)[0];

        auto& adapterTogglesState = nullAdapter->GetTogglesState();

        // Creater a default device.
        native::DeviceBase* nullDevice = nullAdapter->APICreateDevice();

        // Check instance toggles are inherited by adapter and device.
        native::TogglesInfo togglesInfo;
        static_assert(std::is_same_v<std::underlying_type_t<native::Toggle>, int>);
        for (int i = 0; i < static_cast<int>(native::Toggle::EnumCount); i++) {
            native::Toggle toggle = static_cast<native::Toggle>(i);
            if (togglesInfo.GetToggleInfo(toggle)->stage != native::ToggleStage::Instance) {
                continue;
            }
            EXPECT_EQ(instanceTogglesState.IsSet(toggle), adapterTogglesState.IsSet(toggle));
            EXPECT_EQ(instanceTogglesState.IsEnabled(toggle),
                      adapterTogglesState.IsEnabled(toggle));
            EXPECT_EQ(instanceTogglesState.IsEnabled(toggle), nullDevice->IsToggleEnabled(toggle));
        }

        nullDevice->Release();
    };

    // Create instance with AllowUnsafeAPIs explicitly enabled in toggles descriptor
    {
        std::unique_ptr<native::Instance> instance;

        const char* allowUnsafeApisToggle = "allow_unsafe_apis";
        WGPUDawnTogglesDescriptor instanceTogglesDesc = {};
        instanceTogglesDesc.chain.sType = WGPUSType::WGPUSType_DawnTogglesDescriptor;
        instanceTogglesDesc.enabledTogglesCount = 1;
        instanceTogglesDesc.enabledToggles = &allowUnsafeApisToggle;

        WGPUInstanceDescriptor instanceDesc = {};
        instanceDesc.nextInChain = &instanceTogglesDesc.chain;

        // Create an instance with DisallowUnsafeApis explicitly enabled.
        instance = std::make_unique<native::Instance>(&instanceDesc);
        validateInstanceTogglesInheritedToAdapter(instance.get());
    }

    // Create instance with AllowUnsafeAPIs explicitly disabled in toggles descriptor
    {
        std::unique_ptr<native::Instance> instance;

        const char* allowUnsafeApisToggle = "allow_unsafe_apis";
        WGPUDawnTogglesDescriptor instanceTogglesDesc = {};
        instanceTogglesDesc.chain.sType = WGPUSType::WGPUSType_DawnTogglesDescriptor;
        instanceTogglesDesc.disabledTogglesCount = 1;
        instanceTogglesDesc.disabledToggles = &allowUnsafeApisToggle;

        WGPUInstanceDescriptor instanceDesc = {};
        instanceDesc.nextInChain = &instanceTogglesDesc.chain;

        // Create an instance with DisallowUnsafeApis explicitly enabled.
        instance = std::make_unique<native::Instance>(&instanceDesc);
        validateInstanceTogglesInheritedToAdapter(instance.get());
    }
}

}  // anonymous namespace
}  // namespace dawn
