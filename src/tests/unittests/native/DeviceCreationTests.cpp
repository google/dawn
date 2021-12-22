// Copyright 2021 The Dawn Authors
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

#include "dawn/dawn_proc.h"
#include "dawn_native/DawnNative.h"
#include "tests/MockCallback.h"
#include "utils/SystemUtils.h"
#include "utils/WGPUHelpers.h"

#include <gtest/gtest.h>

namespace {

    using namespace testing;

    class DeviceCreationTest : public testing::Test {
      protected:
        void SetUp() override {
            dawnProcSetProcs(&dawn_native::GetProcs());

            instance = std::make_unique<dawn_native::Instance>();
            instance->DiscoverDefaultAdapters();
            for (dawn_native::Adapter& nativeAdapter : instance->GetAdapters()) {
                wgpu::AdapterProperties properties;
                nativeAdapter.GetProperties(&properties);

                if (properties.backendType == wgpu::BackendType::Null) {
                    adapter = wgpu::Adapter(nativeAdapter.Get());
                    break;
                }
            }
            ASSERT_NE(adapter, nullptr);
        }

        void TearDown() override {
            adapter = nullptr;
            instance = nullptr;
            dawnProcSetProcs(nullptr);
        }

        std::unique_ptr<dawn_native::Instance> instance;
        wgpu::Adapter adapter;
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
        wgpu::DawnTogglesDeviceDescriptor togglesDesc = {};
        desc.nextInChain = &togglesDesc;

        const char* toggle = "skip_validation";
        togglesDesc.forceEnabledToggles = &toggle;
        togglesDesc.forceEnabledTogglesCount = 1;

        wgpu::Device device = adapter.CreateDevice(&desc);
        EXPECT_NE(device, nullptr);

        auto toggles = dawn_native::GetTogglesUsed(device.Get());
        EXPECT_THAT(toggles, testing::Contains(testing::StrEq(toggle)));
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

    // Test failing call to RequestDevice with invalid feature
    TEST_F(DeviceCreationTest, RequestDeviceFailure) {
        MockCallback<WGPURequestDeviceCallback> cb;
        EXPECT_CALL(cb, Call(WGPURequestDeviceStatus_Error, nullptr, NotNull(), this)).Times(1);

        wgpu::DeviceDescriptor desc = {};
        wgpu::FeatureName invalidFeature = static_cast<wgpu::FeatureName>(WGPUFeatureName_Force32);
        desc.requiredFeatures = &invalidFeature;
        desc.requiredFeaturesCount = 1;

        adapter.RequestDevice(&desc, cb.Callback(), cb.MakeUserdata(this));
    }

}  // anonymous namespace
