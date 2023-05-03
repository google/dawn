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

namespace {

using testing::Contains;
using testing::MockCallback;
using testing::NotNull;
using testing::SaveArg;
using testing::StrEq;

class InstanceToggleTest : public testing::Test {
  protected:
    void SetUp() override { dawnProcSetProcs(&dawn::native::GetProcs()); }

    void TearDown() override { dawnProcSetProcs(nullptr); }
};

// Test that instance toggles are set by requirement or default as expected.
TEST_F(InstanceToggleTest, InstanceTogglesSet) {
    auto validateInstanceToggles = [](const dawn::native::Instance* nativeInstance,
                                      std::initializer_list<const char*> enableToggles,
                                      std::initializer_list<const char*> disableToggles) {
        const dawn::native::InstanceBase* instance = dawn::native::FromAPI(nativeInstance->Get());
        const dawn::native::TogglesState& instanceTogglesState = instance->GetTogglesState();
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
        std::unique_ptr<dawn::native::Instance> instance;

        // Create an instance with default toggles, where DisallowUnsafeApis is enabled.
        instance = std::make_unique<dawn::native::Instance>();
        validateInstanceToggles(instance.get(), {"disallow_unsafe_apis"}, {});
    }

    // Create instance with empty toggles descriptor
    {
        std::unique_ptr<dawn::native::Instance> instance;

        // Make an instance descriptor chaining an empty toggles descriptor
        WGPUDawnTogglesDescriptor instanceTogglesDesc = {};
        instanceTogglesDesc.chain.sType = WGPUSType::WGPUSType_DawnTogglesDescriptor;

        WGPUInstanceDescriptor instanceDesc = {};
        instanceDesc.nextInChain = &instanceTogglesDesc.chain;

        // Create an instance with default toggles, where DisallowUnsafeApis is enabled.
        instance = std::make_unique<dawn::native::Instance>(&instanceDesc);
        validateInstanceToggles(instance.get(), {"disallow_unsafe_apis"}, {});
    }

    // Create instance with DisallowUnsafeAPIs explicitly enabled in toggles descriptor
    {
        std::unique_ptr<dawn::native::Instance> instance;

        const char* disallowUnsafeApisToggle = "disallow_unsafe_apis";
        WGPUDawnTogglesDescriptor instanceTogglesDesc = {};
        instanceTogglesDesc.chain.sType = WGPUSType::WGPUSType_DawnTogglesDescriptor;
        instanceTogglesDesc.enabledTogglesCount = 1;
        instanceTogglesDesc.enabledToggles = &disallowUnsafeApisToggle;

        WGPUInstanceDescriptor instanceDesc = {};
        instanceDesc.nextInChain = &instanceTogglesDesc.chain;

        // Create an instance with DisallowUnsafeApis explicitly enabled.
        instance = std::make_unique<dawn::native::Instance>(&instanceDesc);
        validateInstanceToggles(instance.get(), {disallowUnsafeApisToggle}, {});
    }

    // Create instance with DisallowUnsafeAPIs explicitly disabled in toggles descriptor
    {
        std::unique_ptr<dawn::native::Instance> instance;

        const char* disallowUnsafeApisToggle = "disallow_unsafe_apis";
        WGPUDawnTogglesDescriptor instanceTogglesDesc = {};
        instanceTogglesDesc.chain.sType = WGPUSType::WGPUSType_DawnTogglesDescriptor;
        instanceTogglesDesc.disabledTogglesCount = 1;
        instanceTogglesDesc.disabledToggles = &disallowUnsafeApisToggle;

        WGPUInstanceDescriptor instanceDesc = {};
        instanceDesc.nextInChain = &instanceTogglesDesc.chain;

        // Create an instance with DisallowUnsafeApis explicitly disabled.
        instance = std::make_unique<dawn::native::Instance>(&instanceDesc);
        validateInstanceToggles(instance.get(), {}, {disallowUnsafeApisToggle});
    }
}

// Test that instance toggles are inherited to the adapters and devices it creates.
TEST_F(InstanceToggleTest, InstanceTogglesInheritToAdapterAndDevice) {
    auto validateInstanceTogglesInheritedToAdapter = [&](dawn::native::Instance* nativeInstance) {
        dawn::native::InstanceBase* instance = dawn::native::FromAPI(nativeInstance->Get());
        const dawn::native::TogglesState& instanceTogglesState = instance->GetTogglesState();

        // Discover adapters with default toggles.
        instance->DiscoverDefaultAdapters();

        // Get the adapter created by instance with default toggles.
        dawn::native::AdapterBase* nullAdapter = nullptr;
        for (auto& adapter : instance->GetAdapters()) {
            if (adapter->GetPhysicalDevice()->GetBackendType() == wgpu::BackendType::Null) {
                nullAdapter = adapter.Get();
                break;
            }
        }
        ASSERT_NE(nullAdapter, nullptr);
        auto& adapterTogglesState = nullAdapter->GetTogglesState();

        // Creater a default device.
        dawn::native::DeviceBase* nullDevice = nullAdapter->APICreateDevice();

        // Check instance toggles are inherited by adapter and device.
        dawn::native::TogglesInfo togglesInfo;
        static_assert(std::is_same_v<std::underlying_type_t<dawn::native::Toggle>, int>);
        for (int i = 0; i < static_cast<int>(dawn::native::Toggle::EnumCount); i++) {
            dawn::native::Toggle toggle = static_cast<dawn::native::Toggle>(i);
            if (togglesInfo.GetToggleInfo(toggle)->stage != dawn::native::ToggleStage::Instance) {
                continue;
            }
            EXPECT_EQ(instanceTogglesState.IsSet(toggle), adapterTogglesState.IsSet(toggle));
            EXPECT_EQ(instanceTogglesState.IsEnabled(toggle),
                      adapterTogglesState.IsEnabled(toggle));
            EXPECT_EQ(instanceTogglesState.IsEnabled(toggle), nullDevice->IsToggleEnabled(toggle));
        }

        nullDevice->Release();
    };

    // Create instance with DisallowUnsafeAPIs explicitly enabled in toggles descriptor
    {
        std::unique_ptr<dawn::native::Instance> instance;

        const char* disallowUnsafeApisToggle = "disallow_unsafe_apis";
        WGPUDawnTogglesDescriptor instanceTogglesDesc = {};
        instanceTogglesDesc.chain.sType = WGPUSType::WGPUSType_DawnTogglesDescriptor;
        instanceTogglesDesc.enabledTogglesCount = 1;
        instanceTogglesDesc.enabledToggles = &disallowUnsafeApisToggle;

        WGPUInstanceDescriptor instanceDesc = {};
        instanceDesc.nextInChain = &instanceTogglesDesc.chain;

        // Create an instance with DisallowUnsafeApis explicitly enabled.
        instance = std::make_unique<dawn::native::Instance>(&instanceDesc);
        validateInstanceTogglesInheritedToAdapter(instance.get());
    }

    // Create instance with DisallowUnsafeAPIs explicitly disabled in toggles descriptor
    {
        std::unique_ptr<dawn::native::Instance> instance;

        const char* disallowUnsafeApisToggle = "disallow_unsafe_apis";
        WGPUDawnTogglesDescriptor instanceTogglesDesc = {};
        instanceTogglesDesc.chain.sType = WGPUSType::WGPUSType_DawnTogglesDescriptor;
        instanceTogglesDesc.disabledTogglesCount = 1;
        instanceTogglesDesc.disabledToggles = &disallowUnsafeApisToggle;

        WGPUInstanceDescriptor instanceDesc = {};
        instanceDesc.nextInChain = &instanceTogglesDesc.chain;

        // Create an instance with DisallowUnsafeApis explicitly enabled.
        instance = std::make_unique<dawn::native::Instance>(&instanceDesc);
        validateInstanceTogglesInheritedToAdapter(instance.get());
    }
}
}  // anonymous namespace
