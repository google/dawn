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

#include <unordered_set>
#include <vector>

#include "dawn/tests/MockCallback.h"
#include "dawn/tests/unittests/wire/WireTest.h"

#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireServer.h"

#include "webgpu/webgpu_cpp.h"

namespace dawn::wire {
namespace {

using testing::Invoke;
using testing::InvokeWithoutArgs;
using testing::MockCallback;
using testing::NotNull;
using testing::Return;
using testing::SetArgPointee;
using testing::StrEq;
using testing::WithArg;

class WireInstanceBasicTest : public WireTest {};
class WireInstanceTests : public WireTest {
  protected:
    void SetUp() override {
        WireTest::SetUp();

        auto reservation = GetWireClient()->ReserveInstance();
        instance = wgpu::Instance::Acquire(reservation.instance);

        apiInstance = api.GetNewInstance();
        EXPECT_CALL(api, InstanceReference(apiInstance));
        EXPECT_TRUE(
            GetWireServer()->InjectInstance(apiInstance, reservation.id, reservation.generation));
    }

    void TearDown() override {
        instance = nullptr;
        WireTest::TearDown();
    }

    wgpu::Instance instance;
    WGPUInstance apiInstance;
};

// Test that an Instance can be reserved and injected into the wire.
TEST_F(WireInstanceBasicTest, ReserveAndInject) {
    auto reservation = GetWireClient()->ReserveInstance();
    wgpu::Instance instance = wgpu::Instance::Acquire(reservation.instance);

    WGPUInstance apiInstance = api.GetNewInstance();
    EXPECT_CALL(api, InstanceReference(apiInstance));
    EXPECT_TRUE(
        GetWireServer()->InjectInstance(apiInstance, reservation.id, reservation.generation));

    instance = nullptr;

    EXPECT_CALL(api, InstanceRelease(apiInstance));
    FlushClient();
}

// Test that RequestAdapterOptions are passed from the client to the server.
TEST_F(WireInstanceTests, RequestAdapterPassesOptions) {
    MockCallback<WGPURequestAdapterCallback> cb;
    auto* userdata = cb.MakeUserdata(this);

    for (wgpu::PowerPreference powerPreference :
         {wgpu::PowerPreference::LowPower, wgpu::PowerPreference::HighPerformance}) {
        wgpu::RequestAdapterOptions options = {};
        options.powerPreference = powerPreference;

        instance.RequestAdapter(&options, cb.Callback(), userdata);

        EXPECT_CALL(api, OnInstanceRequestAdapter(apiInstance, NotNull(), NotNull(), NotNull()))
            .WillOnce(WithArg<1>(Invoke([&](const WGPURequestAdapterOptions* apiOptions) {
                EXPECT_EQ(apiOptions->powerPreference,
                          static_cast<WGPUPowerPreference>(options.powerPreference));
                EXPECT_EQ(apiOptions->forceFallbackAdapter, options.forceFallbackAdapter);
            })));
        FlushClient();
    }

    // Delete the instance now, or it'll call the mock callback after it's deleted.
    instance = nullptr;
}

// Test that RequestAdapter forwards the adapter information to the client.
TEST_F(WireInstanceTests, RequestAdapterSuccess) {
    wgpu::RequestAdapterOptions options = {};
    MockCallback<WGPURequestAdapterCallback> cb;
    auto* userdata = cb.MakeUserdata(this);
    instance.RequestAdapter(&options, cb.Callback(), userdata);

    WGPUAdapterProperties fakeProperties = {};
    fakeProperties.vendorID = 0x134;
    fakeProperties.vendorName = "fake-vendor";
    fakeProperties.architecture = "fake-architecture";
    fakeProperties.deviceID = 0x918;
    fakeProperties.name = "fake adapter";
    fakeProperties.driverDescription = "hello world";
    fakeProperties.backendType = WGPUBackendType_D3D12;
    fakeProperties.adapterType = WGPUAdapterType_IntegratedGPU;

    wgpu::SupportedLimits fakeLimits = {};
    fakeLimits.limits.maxTextureDimension1D = 433;
    fakeLimits.limits.maxVertexAttributes = 1243;

    std::initializer_list<wgpu::FeatureName> fakeFeatures = {
        wgpu::FeatureName::Depth32FloatStencil8,
        wgpu::FeatureName::TextureCompressionBC,
    };

    // Expect the server to receive the message. Then, mock a fake reply.
    WGPUAdapter apiAdapter = api.GetNewAdapter();
    EXPECT_CALL(api, OnInstanceRequestAdapter(apiInstance, NotNull(), NotNull(), NotNull()))
        .WillOnce(InvokeWithoutArgs([&] {
            EXPECT_CALL(api, AdapterGetProperties(apiAdapter, NotNull()))
                .WillOnce(SetArgPointee<1>(fakeProperties));

            EXPECT_CALL(api, AdapterGetLimits(apiAdapter, NotNull()))
                .WillOnce(WithArg<1>(Invoke([&](WGPUSupportedLimits* limits) {
                    *reinterpret_cast<wgpu::SupportedLimits*>(limits) = fakeLimits;
                    return true;
                })));

            EXPECT_CALL(api, AdapterEnumerateFeatures(apiAdapter, nullptr))
                .WillOnce(Return(fakeFeatures.size()));

            EXPECT_CALL(api, AdapterEnumerateFeatures(apiAdapter, NotNull()))
                .WillOnce(WithArg<1>(Invoke([&](WGPUFeatureName* features) {
                    for (wgpu::FeatureName feature : fakeFeatures) {
                        *(features++) = static_cast<WGPUFeatureName>(feature);
                    }
                    return fakeFeatures.size();
                })));
            api.CallInstanceRequestAdapterCallback(apiInstance, WGPURequestAdapterStatus_Success,
                                                   apiAdapter, nullptr);
        }));
    FlushClient();

    // Expect the callback in the client and all the adapter information to match.
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, NotNull(), nullptr, this))
        .WillOnce(WithArg<1>(Invoke([&](WGPUAdapter cAdapter) {
            wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);

            wgpu::AdapterProperties properties;
            adapter.GetProperties(&properties);
            const auto& rhs = *reinterpret_cast<wgpu::AdapterProperties*>(&fakeProperties);
            EXPECT_EQ(properties.vendorID, rhs.vendorID);
            EXPECT_STREQ(properties.vendorName, rhs.vendorName);
            EXPECT_STREQ(properties.architecture, rhs.architecture);
            EXPECT_EQ(properties.deviceID, rhs.deviceID);
            EXPECT_STREQ(properties.name, rhs.name);
            EXPECT_STREQ(properties.driverDescription, rhs.driverDescription);
            EXPECT_EQ(properties.backendType, rhs.backendType);
            EXPECT_EQ(properties.adapterType, rhs.adapterType);

            wgpu::SupportedLimits limits;
            EXPECT_TRUE(adapter.GetLimits(&limits));
            EXPECT_EQ(limits.limits.maxTextureDimension1D, fakeLimits.limits.maxTextureDimension1D);
            EXPECT_EQ(limits.limits.maxVertexAttributes, fakeLimits.limits.maxVertexAttributes);

            std::vector<wgpu::FeatureName> features;
            features.resize(adapter.EnumerateFeatures(nullptr));
            ASSERT_EQ(features.size(), fakeFeatures.size());
            EXPECT_EQ(adapter.EnumerateFeatures(&features[0]), features.size());

            std::unordered_set<wgpu::FeatureName> featureSet(fakeFeatures);
            for (wgpu::FeatureName feature : features) {
                EXPECT_EQ(featureSet.erase(feature), 1u);
            }
        })));
    FlushServer();
}

// Test that features returned by the implementation that aren't supported
// in the wire are not exposed.
TEST_F(WireInstanceTests, RequestAdapterWireLacksFeatureSupport) {
    wgpu::RequestAdapterOptions options = {};
    MockCallback<WGPURequestAdapterCallback> cb;
    auto* userdata = cb.MakeUserdata(this);
    instance.RequestAdapter(&options, cb.Callback(), userdata);

    std::initializer_list<wgpu::FeatureName> fakeFeatures = {
        wgpu::FeatureName::Depth32FloatStencil8,
        // Some value that is not a valid feature
        static_cast<wgpu::FeatureName>(-2),
    };

    // Expect the server to receive the message. Then, mock a fake reply.
    WGPUAdapter apiAdapter = api.GetNewAdapter();
    EXPECT_CALL(api, OnInstanceRequestAdapter(apiInstance, NotNull(), NotNull(), NotNull()))
        .WillOnce(InvokeWithoutArgs([&] {
            EXPECT_CALL(api, AdapterGetProperties(apiAdapter, NotNull()))
                .WillOnce(WithArg<1>(Invoke([&](WGPUAdapterProperties* properties) {
                    *properties = {};
                    properties->vendorName = "";
                    properties->architecture = "";
                    properties->name = "";
                    properties->driverDescription = "";
                })));

            EXPECT_CALL(api, AdapterGetLimits(apiAdapter, NotNull()))
                .WillOnce(WithArg<1>(Invoke([&](WGPUSupportedLimits* limits) {
                    *limits = {};
                    return true;
                })));

            EXPECT_CALL(api, AdapterEnumerateFeatures(apiAdapter, nullptr))
                .WillOnce(Return(fakeFeatures.size()));

            EXPECT_CALL(api, AdapterEnumerateFeatures(apiAdapter, NotNull()))
                .WillOnce(WithArg<1>(Invoke([&](WGPUFeatureName* features) {
                    for (wgpu::FeatureName feature : fakeFeatures) {
                        *(features++) = static_cast<WGPUFeatureName>(feature);
                    }
                    return fakeFeatures.size();
                })));
            api.CallInstanceRequestAdapterCallback(apiInstance, WGPURequestAdapterStatus_Success,
                                                   apiAdapter, nullptr);
        }));
    FlushClient();

    // Expect the callback in the client and all the adapter information to match.
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, NotNull(), nullptr, this))
        .WillOnce(WithArg<1>(Invoke([&](WGPUAdapter cAdapter) {
            wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);

            wgpu::FeatureName feature;
            ASSERT_EQ(adapter.EnumerateFeatures(nullptr), 1u);
            adapter.EnumerateFeatures(&feature);

            EXPECT_EQ(feature, wgpu::FeatureName::Depth32FloatStencil8);
        })));
    FlushServer();
}

// Test that RequestAdapter errors forward to the client.
TEST_F(WireInstanceTests, RequestAdapterError) {
    wgpu::RequestAdapterOptions options = {};
    MockCallback<WGPURequestAdapterCallback> cb;
    auto* userdata = cb.MakeUserdata(this);
    instance.RequestAdapter(&options, cb.Callback(), userdata);

    // Expect the server to receive the message. Then, mock an error.
    EXPECT_CALL(api, OnInstanceRequestAdapter(apiInstance, NotNull(), NotNull(), NotNull()))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallInstanceRequestAdapterCallback(apiInstance, WGPURequestAdapterStatus_Error,
                                                   nullptr, "Some error");
        }));
    FlushClient();

    // Expect the callback in the client.
    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Error, nullptr, StrEq("Some error"), this))
        .Times(1);
    FlushServer();
}

// Test that RequestAdapter receives unknown status if the instance is deleted
// before the callback happens.
TEST_F(WireInstanceTests, RequestAdapterInstanceDestroyedBeforeCallback) {
    wgpu::RequestAdapterOptions options = {};
    MockCallback<WGPURequestAdapterCallback> cb;
    auto* userdata = cb.MakeUserdata(this);
    instance.RequestAdapter(&options, cb.Callback(), userdata);

    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Unknown, nullptr, NotNull(), this)).Times(1);
    instance = nullptr;
}

// Test that RequestAdapter receives unknown status if the wire is disconnected
// before the callback happens.
TEST_F(WireInstanceTests, RequestAdapterWireDisconnectBeforeCallback) {
    wgpu::RequestAdapterOptions options = {};
    MockCallback<WGPURequestAdapterCallback> cb;
    auto* userdata = cb.MakeUserdata(this);
    instance.RequestAdapter(&options, cb.Callback(), userdata);

    EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Unknown, nullptr, NotNull(), this)).Times(1);
    GetWireClient()->Disconnect();
}

}  // anonymous namespace
}  // namespace dawn::wire
