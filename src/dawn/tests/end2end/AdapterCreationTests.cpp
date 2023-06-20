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
#include <utility>

#include "dawn/common/GPUInfo.h"
#include "dawn/dawn_proc.h"
#include "dawn/native/DawnNative.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/webgpu_cpp.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

using testing::_;
using testing::MockCallback;
using testing::SaveArg;

class AdapterCreationTest : public ::testing::Test {
  protected:
    void SetUp() override {
        dawnProcSetProcs(&native::GetProcs());

        {
            auto nativeInstance = std::make_unique<native::Instance>();
            for (native::Adapter& nativeAdapter : nativeInstance->EnumerateAdapters()) {
                anyAdapterAvailable = true;

                wgpu::AdapterProperties properties;
                nativeAdapter.GetProperties(&properties);
                if (properties.compatibilityMode) {
                    continue;
                }
                swiftShaderAvailable |=
                    gpu_info::IsGoogleSwiftshader(properties.vendorID, properties.deviceID);
                discreteGPUAvailable |= properties.adapterType == wgpu::AdapterType::DiscreteGPU;
                integratedGPUAvailable |=
                    properties.adapterType == wgpu::AdapterType::IntegratedGPU;
            }
        }

        instance = wgpu::CreateInstance();
    }

    void TearDown() override {
        instance = nullptr;
        dawnProcSetProcs(nullptr);
    }

    wgpu::Instance instance;
    bool anyAdapterAvailable = false;
    bool swiftShaderAvailable = false;
    bool discreteGPUAvailable = false;
    bool integratedGPUAvailable = false;
};

// Test that requesting the default adapter works
TEST_F(AdapterCreationTest, DefaultAdapter) {
    wgpu::RequestAdapterOptions options = {};

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    instance.RequestAdapter(&options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
}

// Test that passing nullptr for the options gets the default adapter
TEST_F(AdapterCreationTest, NullGivesDefaultAdapter) {
    wgpu::RequestAdapterOptions options = {};

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    instance.RequestAdapter(&options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);

    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this + 1))
        .WillOnce(SaveArg<1>(&cAdapter));
    instance.RequestAdapter(nullptr, cb.Callback(), cb.MakeUserdata(this + 1));

    wgpu::Adapter adapter2 = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter2 != nullptr, anyAdapterAvailable);
}

// Test that requesting the fallback adapter returns SwiftShader.
TEST_F(AdapterCreationTest, FallbackAdapter) {
    wgpu::RequestAdapterOptions options = {};
    options.forceFallbackAdapter = true;

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    if (swiftShaderAvailable) {
        EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
            .WillOnce(SaveArg<1>(&cAdapter));
    } else {
        EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Unavailable, nullptr, _, this))
            .WillOnce(SaveArg<1>(&cAdapter));
    }
    instance.RequestAdapter(&options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, swiftShaderAvailable);
    if (adapter != nullptr) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);

        EXPECT_EQ(properties.adapterType, wgpu::AdapterType::CPU);
        EXPECT_TRUE(gpu_info::IsGoogleSwiftshader(properties.vendorID, properties.deviceID));
    }
}

// Test that requesting a high performance GPU works
TEST_F(AdapterCreationTest, PreferHighPerformance) {
    wgpu::RequestAdapterOptions options = {};
    options.powerPreference = wgpu::PowerPreference::HighPerformance;

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    if (anyAdapterAvailable) {
        EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
            .WillOnce(SaveArg<1>(&cAdapter));
    } else {
        EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Unavailable, nullptr, _, this))
            .WillOnce(SaveArg<1>(&cAdapter));
    }
    instance.RequestAdapter(&options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
    if (discreteGPUAvailable) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);
        EXPECT_EQ(properties.adapterType, wgpu::AdapterType::DiscreteGPU);
    }
}

// Test that requesting a low power GPU works
TEST_F(AdapterCreationTest, PreferLowPower) {
    wgpu::RequestAdapterOptions options = {};
    options.powerPreference = wgpu::PowerPreference::LowPower;

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    if (anyAdapterAvailable) {
        EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
            .WillOnce(SaveArg<1>(&cAdapter));
    } else {
        EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Unavailable, nullptr, _, this))
            .WillOnce(SaveArg<1>(&cAdapter));
    }
    instance.RequestAdapter(&options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
    if (integratedGPUAvailable) {
        wgpu::AdapterProperties properties;
        adapter.GetProperties(&properties);
        EXPECT_EQ(properties.adapterType, wgpu::AdapterType::IntegratedGPU);
    }
}

// Test that requesting a Compatibility adapter is supported.
TEST_F(AdapterCreationTest, Compatibility) {
    wgpu::RequestAdapterOptions options = {};
    options.compatibilityMode = true;

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    instance.RequestAdapter(&options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);

    wgpu::AdapterProperties properties;
    adapter.GetProperties(&properties);
    EXPECT_TRUE(properties.compatibilityMode);
}

// Test that requesting a Non-Compatibility adapter is supported and is default.
TEST_F(AdapterCreationTest, NonCompatibility) {
    wgpu::RequestAdapterOptions options = {};

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    instance.RequestAdapter(&options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);

    wgpu::AdapterProperties properties;
    adapter.GetProperties(&properties);
    EXPECT_FALSE(properties.compatibilityMode);
}

// Test that GetInstance() returns the correct Instance.
TEST_F(AdapterCreationTest, GetInstance) {
    wgpu::RequestAdapterOptions options = {};

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    instance.RequestAdapter(&options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);

    EXPECT_EQ(adapter.GetInstance().Get(), instance.Get());
}

}  // anonymous namespace
}  // namespace dawn
