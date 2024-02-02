// Copyright 2023 The Dawn & Tint Authors
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

#include <memory>
#include <optional>
#include <string>
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

class AdapterCreationTest : public ::testing::TestWithParam<std::optional<wgpu::CallbackMode>> {
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

    void RequestAdapter(const wgpu::Instance& inst,
                        const wgpu::RequestAdapterOptions* options,
                        WGPURequestAdapterCallback callback,
                        void* userdata) {
        if (GetParam() == std::nullopt) {
            // Legacy RequestAdapter. It should call the callback immediately.
            inst.RequestAdapter(options, callback, userdata);
            return;
        }

        wgpu::Future future =
            inst.RequestAdapter(options, {nullptr, *GetParam(), callback, userdata});
        switch (*GetParam()) {
            case wgpu::CallbackMode::WaitAnyOnly: {
                // Callback should complete as soon as poll once.
                wgpu::FutureWaitInfo waitInfo = {future};
                EXPECT_EQ(inst.WaitAny(1, &waitInfo, 0), wgpu::WaitStatus::Success);
                ASSERT_TRUE(waitInfo.completed);
                break;
            }
            case wgpu::CallbackMode::AllowSpontaneous:
                // Callback should already be called.
                break;
            case wgpu::CallbackMode::AllowProcessEvents:
                inst.ProcessEvents();
                break;
        }
    }

    wgpu::Instance instance;
    bool anyAdapterAvailable = false;
    bool swiftShaderAvailable = false;
    bool discreteGPUAvailable = false;
    bool integratedGPUAvailable = false;
};

INSTANTIATE_TEST_SUITE_P(
    ,
    AdapterCreationTest,
    ::testing::ValuesIn(std::initializer_list<std::optional<wgpu::CallbackMode>>{
        wgpu::CallbackMode::WaitAnyOnly, wgpu::CallbackMode::AllowProcessEvents,
        wgpu::CallbackMode::AllowSpontaneous, std::nullopt}));

// Test that requesting the default adapter works
TEST_P(AdapterCreationTest, DefaultAdapter) {
    wgpu::RequestAdapterOptions options = {};

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    RequestAdapter(instance, &options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
}

// Test that passing nullptr for the options gets the default adapter
TEST_P(AdapterCreationTest, NullGivesDefaultAdapter) {
    wgpu::RequestAdapterOptions options = {};

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    RequestAdapter(instance, &options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);

    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this + 1))
        .WillOnce(SaveArg<1>(&cAdapter));
    RequestAdapter(instance, nullptr, cb.Callback(), cb.MakeUserdata(this + 1));

    wgpu::Adapter adapter2 = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter2 != nullptr, anyAdapterAvailable);
}

// Test that requesting the fallback adapter returns SwiftShader.
TEST_P(AdapterCreationTest, FallbackAdapter) {
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
    RequestAdapter(instance, &options, cb.Callback(), cb.MakeUserdata(this));

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
TEST_P(AdapterCreationTest, PreferHighPerformance) {
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
    RequestAdapter(instance, &options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
    if (discreteGPUAvailable) {
        wgpu::AdapterProperties properties;
        wgpu::DawnAdapterPropertiesPowerPreference powerPreferenceProperties;
        properties.nextInChain = &powerPreferenceProperties;
        adapter.GetProperties(&properties);
        EXPECT_EQ(properties.adapterType, wgpu::AdapterType::DiscreteGPU);
        EXPECT_EQ(powerPreferenceProperties.powerPreference, options.powerPreference);
    }
}

// Test that requesting a low power GPU works
TEST_P(AdapterCreationTest, PreferLowPower) {
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
    RequestAdapter(instance, &options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
    if (integratedGPUAvailable) {
        wgpu::AdapterProperties properties;
        wgpu::DawnAdapterPropertiesPowerPreference powerPreferenceProperties;
        properties.nextInChain = &powerPreferenceProperties;
        adapter.GetProperties(&properties);
        EXPECT_EQ(properties.adapterType, wgpu::AdapterType::IntegratedGPU);
        EXPECT_EQ(powerPreferenceProperties.powerPreference, options.powerPreference);
    }
}

// Test that requesting a Compatibility adapter is supported.
TEST_P(AdapterCreationTest, Compatibility) {
    wgpu::RequestAdapterOptions options = {};
    options.compatibilityMode = true;

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    RequestAdapter(instance, &options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);

    wgpu::AdapterProperties properties;
    adapter.GetProperties(&properties);
    EXPECT_TRUE(properties.compatibilityMode);
}

// Test that requesting a Non-Compatibility adapter is supported and is default.
TEST_P(AdapterCreationTest, NonCompatibility) {
    wgpu::RequestAdapterOptions options = {};

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    RequestAdapter(instance, &options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);

    wgpu::AdapterProperties properties;
    adapter.GetProperties(&properties);
    EXPECT_FALSE(properties.compatibilityMode);
}

// Test that GetInstance() returns the correct Instance.
TEST_P(AdapterCreationTest, GetInstance) {
    wgpu::RequestAdapterOptions options = {};

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    RequestAdapter(instance, &options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);

    EXPECT_EQ(adapter.GetInstance().Get(), instance.Get());
}

// Test that calling AdapterGetProperties returns separate allocations for strings.
// However, the string contents are equivalent.
TEST_P(AdapterCreationTest, PropertiesUnique) {
    wgpu::RequestAdapterOptions options = {};

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    RequestAdapter(instance, &options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
    if (!adapter) {
        return;
    }

    wgpu::AdapterProperties properties1;
    wgpu::AdapterProperties properties2;
    adapter.GetProperties(&properties1);
    adapter.GetProperties(&properties2);

    EXPECT_NE(properties1.vendorName, properties2.vendorName);
    EXPECT_STREQ(properties1.vendorName, properties2.vendorName);
    EXPECT_NE(properties1.architecture, properties2.architecture);
    EXPECT_STREQ(properties1.architecture, properties2.architecture);
    EXPECT_NE(properties1.name, properties2.name);
    EXPECT_STREQ(properties1.name, properties2.name);
    EXPECT_NE(properties1.driverDescription, properties2.driverDescription);
    EXPECT_STREQ(properties1.driverDescription, properties2.driverDescription);
}

// Test move assignment of the adapter properties.
TEST_P(AdapterCreationTest, PropertiesMoveAssign) {
    wgpu::RequestAdapterOptions options = {};

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    RequestAdapter(instance, &options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
    if (!adapter) {
        return;
    }

    wgpu::AdapterProperties properties1;
    wgpu::AdapterProperties properties2;
    adapter.GetProperties(&properties1);
    adapter.GetProperties(&properties2);

    uint32_t vendorID = properties1.vendorID;
    std::string vendorName = properties1.vendorName;
    std::string architecture = properties1.architecture;
    uint32_t deviceID = properties1.deviceID;
    std::string name = properties1.name;
    std::string driverDescription = properties1.driverDescription;
    wgpu::AdapterType adapterType = properties1.adapterType;
    wgpu::BackendType backendType = properties1.backendType;
    bool compatibilityMode = properties1.compatibilityMode;

    properties2 = std::move(properties1);

    // Expect properties2 to have properties1's old contents.
    EXPECT_EQ(properties2.vendorID, vendorID);
    EXPECT_STREQ(properties2.vendorName, vendorName.c_str());
    EXPECT_STREQ(properties2.architecture, architecture.c_str());
    EXPECT_EQ(properties2.deviceID, deviceID);
    EXPECT_STREQ(properties2.name, name.c_str());
    EXPECT_STREQ(properties2.driverDescription, driverDescription.c_str());
    EXPECT_EQ(properties2.adapterType, adapterType);
    EXPECT_EQ(properties2.backendType, backendType);
    EXPECT_EQ(properties2.compatibilityMode, compatibilityMode);

    // Expect properties1 to be empty.
    EXPECT_EQ(properties1.vendorID, 0u);
    EXPECT_EQ(properties1.vendorName, nullptr);
    EXPECT_EQ(properties1.architecture, nullptr);
    EXPECT_EQ(properties1.deviceID, 0u);
    EXPECT_EQ(properties1.name, nullptr);
    EXPECT_EQ(properties1.driverDescription, nullptr);
    EXPECT_EQ(properties1.adapterType, static_cast<wgpu::AdapterType>(0));
    EXPECT_EQ(properties1.backendType, static_cast<wgpu::BackendType>(0));
    EXPECT_EQ(properties1.compatibilityMode, false);
}

// Test move construction of the adapter properties.
TEST_P(AdapterCreationTest, PropertiesMoveConstruct) {
    wgpu::RequestAdapterOptions options = {};

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    RequestAdapter(instance, &options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
    if (!adapter) {
        return;
    }

    wgpu::AdapterProperties properties1;
    adapter.GetProperties(&properties1);

    uint32_t vendorID = properties1.vendorID;
    std::string vendorName = properties1.vendorName;
    std::string architecture = properties1.architecture;
    uint32_t deviceID = properties1.deviceID;
    std::string name = properties1.name;
    std::string driverDescription = properties1.driverDescription;
    wgpu::AdapterType adapterType = properties1.adapterType;
    wgpu::BackendType backendType = properties1.backendType;
    bool compatibilityMode = properties1.compatibilityMode;

    wgpu::AdapterProperties properties2(std::move(properties1));

    // Expect properties2 to have properties1's old contents.
    EXPECT_EQ(properties2.vendorID, vendorID);
    EXPECT_STREQ(properties2.vendorName, vendorName.c_str());
    EXPECT_STREQ(properties2.architecture, architecture.c_str());
    EXPECT_EQ(properties2.deviceID, deviceID);
    EXPECT_STREQ(properties2.name, name.c_str());
    EXPECT_STREQ(properties2.driverDescription, driverDescription.c_str());
    EXPECT_EQ(properties2.adapterType, adapterType);
    EXPECT_EQ(properties2.backendType, backendType);
    EXPECT_EQ(properties2.compatibilityMode, compatibilityMode);

    // Expect properties1 to be empty.
    EXPECT_EQ(properties1.vendorID, 0u);
    EXPECT_EQ(properties1.vendorName, nullptr);
    EXPECT_EQ(properties1.architecture, nullptr);
    EXPECT_EQ(properties1.deviceID, 0u);
    EXPECT_EQ(properties1.name, nullptr);
    EXPECT_EQ(properties1.driverDescription, nullptr);
    EXPECT_EQ(properties1.adapterType, static_cast<wgpu::AdapterType>(0));
    EXPECT_EQ(properties1.backendType, static_cast<wgpu::BackendType>(0));
    EXPECT_EQ(properties1.compatibilityMode, false);
}

// Test that the adapter properties can outlive the adapter.
TEST_P(AdapterCreationTest, PropertiesOutliveAdapter) {
    wgpu::RequestAdapterOptions options = {};

    MockCallback<WGPURequestAdapterCallback> cb;

    WGPUAdapter cAdapter = nullptr;
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, _, nullptr, this))
        .WillOnce(SaveArg<1>(&cAdapter));
    RequestAdapter(instance, &options, cb.Callback(), cb.MakeUserdata(this));

    wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
    EXPECT_EQ(adapter != nullptr, anyAdapterAvailable);
    if (!adapter) {
        return;
    }

    wgpu::AdapterProperties properties;
    adapter.GetProperties(&properties);

    // Release the adapter.
    adapter = nullptr;

    // Copy the properties to std::string, this should not be a use-after-free.
    std::string vendorName = properties.vendorName;
    std::string architecture = properties.architecture;
    std::string name = properties.name;
    std::string driverDescription = properties.driverDescription;
}

}  // anonymous namespace
}  // namespace dawn
