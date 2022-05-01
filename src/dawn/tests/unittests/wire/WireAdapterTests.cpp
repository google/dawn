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

using testing::_;
using testing::Invoke;
using testing::InvokeWithoutArgs;
using testing::MockCallback;
using testing::NotNull;
using testing::Return;
using testing::SaveArg;
using testing::StrEq;
using testing::WithArg;

class WireAdapterTests : public WireTest {
  protected:
    // Bootstrap the tests and create a fake adapter.
    void SetUp() override {
        WireTest::SetUp();

        auto reservation = GetWireClient()->ReserveInstance();
        instance = wgpu::Instance::Acquire(reservation.instance);

        WGPUInstance apiInstance = api.GetNewInstance();
        EXPECT_CALL(api, InstanceReference(apiInstance));
        EXPECT_TRUE(
            GetWireServer()->InjectInstance(apiInstance, reservation.id, reservation.generation));

        wgpu::RequestAdapterOptions options = {};
        MockCallback<WGPURequestAdapterCallback> cb;
        auto* userdata = cb.MakeUserdata(this);
        instance.RequestAdapter(&options, cb.Callback(), userdata);

        // Expect the server to receive the message. Then, mock a fake reply.
        apiAdapter = api.GetNewAdapter();
        EXPECT_CALL(api, OnInstanceRequestAdapter(apiInstance, NotNull(), NotNull(), NotNull()))
            .WillOnce(InvokeWithoutArgs([&]() {
                EXPECT_CALL(api, AdapterGetProperties(apiAdapter, NotNull()))
                    .WillOnce(WithArg<1>(Invoke([&](WGPUAdapterProperties* properties) {
                        *properties = {};
                        properties->name = "";
                        properties->driverDescription = "";
                    })));

                EXPECT_CALL(api, AdapterGetLimits(apiAdapter, NotNull()))
                    .WillOnce(WithArg<1>(Invoke([&](WGPUSupportedLimits* limits) {
                        *limits = {};
                        return true;
                    })));

                EXPECT_CALL(api, AdapterEnumerateFeatures(apiAdapter, nullptr))
                    .WillOnce(Return(0))
                    .WillOnce(Return(0));
                api.CallInstanceRequestAdapterCallback(
                    apiInstance, WGPURequestAdapterStatus_Success, apiAdapter, nullptr);
            }));
        FlushClient();

        // Expect the callback in the client.
        WGPUAdapter cAdapter;
        EXPECT_CALL(cb, Call(WGPURequestAdapterStatus_Success, NotNull(), nullptr, this))
            .WillOnce(SaveArg<1>(&cAdapter));
        FlushServer();

        EXPECT_NE(cAdapter, nullptr);
        adapter = wgpu::Adapter::Acquire(cAdapter);
    }

    void TearDown() override {
        adapter = nullptr;
        instance = nullptr;
        WireTest::TearDown();
    }

    WGPUAdapter apiAdapter;
    wgpu::Instance instance;
    wgpu::Adapter adapter;
};

// Test that the DeviceDescriptor is passed from the client to the server.
TEST_F(WireAdapterTests, RequestDevicePassesDescriptor) {
    MockCallback<WGPURequestDeviceCallback> cb;
    auto* userdata = cb.MakeUserdata(this);

    // Test an empty descriptor
    {
        wgpu::DeviceDescriptor desc = {};
        adapter.RequestDevice(&desc, cb.Callback(), userdata);

        EXPECT_CALL(api, OnAdapterRequestDevice(apiAdapter, NotNull(), NotNull(), NotNull()))
            .WillOnce(WithArg<1>(Invoke([](const WGPUDeviceDescriptor* apiDesc) {
                EXPECT_EQ(apiDesc->label, nullptr);
                EXPECT_EQ(apiDesc->requiredFeaturesCount, 0u);
                EXPECT_EQ(apiDesc->requiredLimits, nullptr);
            })));
        FlushClient();
    }

    // Test a non-empty descriptor
    {
        wgpu::RequiredLimits limits = {};
        limits.limits.maxStorageTexturesPerShaderStage = 5;

        std::vector<wgpu::FeatureName> features = {wgpu::FeatureName::TextureCompressionETC2,
                                                   wgpu::FeatureName::TextureCompressionASTC};

        wgpu::DeviceDescriptor desc = {};
        desc.label = "hello device";
        desc.requiredLimits = &limits;
        desc.requiredFeaturesCount = features.size();
        desc.requiredFeatures = features.data();

        adapter.RequestDevice(&desc, cb.Callback(), userdata);

        EXPECT_CALL(api, OnAdapterRequestDevice(apiAdapter, NotNull(), NotNull(), NotNull()))
            .WillOnce(WithArg<1>(Invoke([&](const WGPUDeviceDescriptor* apiDesc) {
                EXPECT_STREQ(apiDesc->label, desc.label);

                ASSERT_EQ(apiDesc->requiredFeaturesCount, features.size());
                for (uint32_t i = 0; i < features.size(); ++i) {
                    EXPECT_EQ(apiDesc->requiredFeatures[i],
                              static_cast<WGPUFeatureName>(features[i]));
                }

                ASSERT_NE(apiDesc->requiredLimits, nullptr);
                EXPECT_EQ(apiDesc->requiredLimits->nextInChain, nullptr);
                EXPECT_EQ(apiDesc->requiredLimits->limits.maxStorageTexturesPerShaderStage,
                          limits.limits.maxStorageTexturesPerShaderStage);
            })));
        FlushClient();
    }

    // Delete the adapter now, or it'll call the mock callback after it's deleted.
    adapter = nullptr;
}

// Test that RequestDevice forwards the device information to the client.
TEST_F(WireAdapterTests, RequestDeviceSuccess) {
    MockCallback<WGPURequestDeviceCallback> cb;
    auto* userdata = cb.MakeUserdata(this);

    wgpu::SupportedLimits fakeLimits = {};
    fakeLimits.limits.maxTextureDimension1D = 433;
    fakeLimits.limits.maxVertexAttributes = 1243;

    std::initializer_list<wgpu::FeatureName> fakeFeatures = {
        wgpu::FeatureName::Depth32FloatStencil8,
        wgpu::FeatureName::TextureCompressionBC,
    };

    wgpu::DeviceDescriptor desc = {};
    adapter.RequestDevice(&desc, cb.Callback(), userdata);

    // Expect the server to receive the message. Then, mock a fake reply.
    WGPUDevice apiDevice = api.GetNewDevice();
    EXPECT_CALL(api, OnAdapterRequestDevice(apiAdapter, NotNull(), NotNull(), NotNull()))
        .WillOnce(InvokeWithoutArgs([&]() {
            // Set on device creation to forward callbacks to the client.
            EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(apiDevice, NotNull(), NotNull()))
                .Times(1);
            EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, NotNull(), NotNull())).Times(1);
            EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(apiDevice, NotNull(), NotNull()))
                .Times(1);

            EXPECT_CALL(api, DeviceGetLimits(apiDevice, NotNull()))
                .WillOnce(WithArg<1>(Invoke([&](WGPUSupportedLimits* limits) {
                    *reinterpret_cast<wgpu::SupportedLimits*>(limits) = fakeLimits;
                    return true;
                })));

            EXPECT_CALL(api, DeviceEnumerateFeatures(apiDevice, nullptr))
                .WillOnce(Return(fakeFeatures.size()));

            EXPECT_CALL(api, DeviceEnumerateFeatures(apiDevice, NotNull()))
                .WillOnce(WithArg<1>(Invoke([&](WGPUFeatureName* features) {
                    for (wgpu::FeatureName feature : fakeFeatures) {
                        *(features++) = static_cast<WGPUFeatureName>(feature);
                    }
                    return fakeFeatures.size();
                })));

            api.CallAdapterRequestDeviceCallback(apiAdapter, WGPURequestDeviceStatus_Success,
                                                 apiDevice, nullptr);
        }));
    FlushClient();

    // Expect the callback in the client and all the device information to match.
    EXPECT_CALL(cb, Call(WGPURequestDeviceStatus_Success, NotNull(), nullptr, this))
        .WillOnce(WithArg<1>(Invoke([&](WGPUDevice cDevice) {
            wgpu::Device device = wgpu::Device::Acquire(cDevice);

            wgpu::SupportedLimits limits;
            EXPECT_TRUE(device.GetLimits(&limits));
            EXPECT_EQ(limits.limits.maxTextureDimension1D, fakeLimits.limits.maxTextureDimension1D);
            EXPECT_EQ(limits.limits.maxVertexAttributes, fakeLimits.limits.maxVertexAttributes);

            std::vector<wgpu::FeatureName> features;
            features.resize(device.EnumerateFeatures(nullptr));
            ASSERT_EQ(features.size(), fakeFeatures.size());
            EXPECT_EQ(device.EnumerateFeatures(&features[0]), features.size());

            std::unordered_set<wgpu::FeatureName> featureSet(fakeFeatures);
            for (wgpu::FeatureName feature : features) {
                EXPECT_EQ(featureSet.erase(feature), 1u);
            }
        })));
    FlushServer();

    // Cleared when the device is destroyed.
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(apiDevice, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(apiDevice, nullptr, nullptr)).Times(1);
}

// Test that features requested that the implementation supports, but not the
// wire reject the callback.
TEST_F(WireAdapterTests, RequestFeatureUnsupportedByWire) {
    MockCallback<WGPURequestDeviceCallback> cb;
    auto* userdata = cb.MakeUserdata(this);

    std::initializer_list<wgpu::FeatureName> fakeFeatures = {
        // Some value that is not a valid feature
        static_cast<wgpu::FeatureName>(-2),
        wgpu::FeatureName::TextureCompressionASTC,
    };

    wgpu::DeviceDescriptor desc = {};
    adapter.RequestDevice(&desc, cb.Callback(), userdata);

    // Expect the server to receive the message. Then, mock a fake reply.
    // The reply contains features that the device implementation supports, but the
    // wire does not.
    WGPUDevice apiDevice = api.GetNewDevice();
    EXPECT_CALL(api, OnAdapterRequestDevice(apiAdapter, NotNull(), NotNull(), NotNull()))
        .WillOnce(InvokeWithoutArgs([&]() {
            EXPECT_CALL(api, DeviceEnumerateFeatures(apiDevice, nullptr))
                .WillOnce(Return(fakeFeatures.size()));

            EXPECT_CALL(api, DeviceEnumerateFeatures(apiDevice, NotNull()))
                .WillOnce(WithArg<1>(Invoke([&](WGPUFeatureName* features) {
                    for (wgpu::FeatureName feature : fakeFeatures) {
                        *(features++) = static_cast<WGPUFeatureName>(feature);
                    }
                    return fakeFeatures.size();
                })));

            // The device was actually created, but the wire didn't support its features.
            // Expect it to be released.
            EXPECT_CALL(api, DeviceRelease(apiDevice));

            // Fake successful creation. The client still receives a failure due to
            // unsupported features.
            api.CallAdapterRequestDeviceCallback(apiAdapter, WGPURequestDeviceStatus_Success,
                                                 apiDevice, nullptr);
        }));
    FlushClient();

    // Expect an error callback since the feature is not supported.
    EXPECT_CALL(cb, Call(WGPURequestDeviceStatus_Error, nullptr, NotNull(), this)).Times(1);
    FlushServer();
}

// Test that RequestDevice errors forward to the client.
TEST_F(WireAdapterTests, RequestDeviceError) {
    MockCallback<WGPURequestDeviceCallback> cb;
    auto* userdata = cb.MakeUserdata(this);

    wgpu::DeviceDescriptor desc = {};
    adapter.RequestDevice(&desc, cb.Callback(), userdata);

    // Expect the server to receive the message. Then, mock an error.
    EXPECT_CALL(api, OnAdapterRequestDevice(apiAdapter, NotNull(), NotNull(), NotNull()))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallAdapterRequestDeviceCallback(apiAdapter, WGPURequestDeviceStatus_Error, nullptr,
                                                 "Request device failed");
        }));
    FlushClient();

    // Expect the callback in the client.
    EXPECT_CALL(cb,
                Call(WGPURequestDeviceStatus_Error, nullptr, StrEq("Request device failed"), this))
        .Times(1);
    FlushServer();
}

// Test that RequestDevice receives unknown status if the adapter is deleted
// before the callback happens.
TEST_F(WireAdapterTests, RequestDeviceAdapterDestroyedBeforeCallback) {
    MockCallback<WGPURequestDeviceCallback> cb;
    auto* userdata = cb.MakeUserdata(this);

    wgpu::DeviceDescriptor desc = {};
    adapter.RequestDevice(&desc, cb.Callback(), userdata);

    EXPECT_CALL(cb, Call(WGPURequestDeviceStatus_Unknown, nullptr, NotNull(), this)).Times(1);
    adapter = nullptr;
}

// Test that RequestDevice receives unknown status if the wire is disconnected
// before the callback happens.
TEST_F(WireAdapterTests, RequestDeviceWireDisconnectedBeforeCallback) {
    MockCallback<WGPURequestDeviceCallback> cb;
    auto* userdata = cb.MakeUserdata(this);

    wgpu::DeviceDescriptor desc = {};
    adapter.RequestDevice(&desc, cb.Callback(), userdata);

    EXPECT_CALL(cb, Call(WGPURequestDeviceStatus_Unknown, nullptr, NotNull(), this)).Times(1);
    GetWireClient()->Disconnect();
}

// TODO(https://crbug.com/dawn/1381) Remove when namespaces are not indented.
// NOLINTNEXTLINE(readability/namespace)
}  // namespace
}  // namespace dawn::wire
