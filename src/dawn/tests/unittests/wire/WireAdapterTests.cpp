// Copyright 2021 The Dawn & Tint Authors
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

#include <unordered_set>
#include <vector>

#include "dawn/tests/MockCallback.h"
#include "dawn/tests/unittests/wire/WireFutureTest.h"
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

using WireAdapterTestBase = WireFutureTestWithParams<WGPURequestDeviceCallback,
                                                     WGPURequestDeviceCallbackInfo,
                                                     wgpuAdapterRequestDevice,
                                                     wgpuAdapterRequestDeviceF>;

class WireAdapterTests : public WireAdapterTestBase {
  protected:
    // Overriden version of wgpuAdapterRequestDevice that defers to the API call based on the
    // test callback mode.
    void AdapterRequestDevice(const wgpu::Adapter& a,
                              const wgpu::DeviceDescriptor* descriptor,
                              void* userdata = nullptr) {
        CallImpl(userdata, a.Get(), reinterpret_cast<WGPUDeviceDescriptor const*>(descriptor));
    }
};
DAWN_INSTANTIATE_WIRE_FUTURE_TEST_P(WireAdapterTests);

// Test that an empty DeviceDescriptor is passed from the client to the server.
TEST_P(WireAdapterTests, RequestDeviceEmptyDescriptor) {
    wgpu::DeviceDescriptor desc = {};
    AdapterRequestDevice(adapter, &desc);

    EXPECT_CALL(api, OnAdapterRequestDevice(apiAdapter, NotNull(), _))
        .WillOnce(WithArg<1>(Invoke([&](const WGPUDeviceDescriptor* apiDesc) {
            EXPECT_EQ(apiDesc->label.data, nullptr);
            EXPECT_EQ(apiDesc->requiredFeatureCount, 0u);
            EXPECT_EQ(apiDesc->requiredLimits, nullptr);

            // Call the callback so the test doesn't wait indefinitely.
            api.CallAdapterRequestDeviceCallback(apiAdapter, WGPURequestDeviceStatus_Error, nullptr,
                                                 nullptr);
        })));
    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call).Times(1);

        FlushCallbacks();
    });
}

// Test that a null DeviceDescriptor is passed from the client to the server as an empty one.
TEST_P(WireAdapterTests, RequestDeviceNullDescriptor) {
    AdapterRequestDevice(adapter, nullptr);

    EXPECT_CALL(api, OnAdapterRequestDevice(apiAdapter, NotNull(), _))
        .WillOnce(WithArg<1>(Invoke([&](const WGPUDeviceDescriptor* apiDesc) {
            EXPECT_EQ(apiDesc->label.data, nullptr);
            EXPECT_EQ(apiDesc->requiredFeatureCount, 0u);
            EXPECT_EQ(apiDesc->requiredLimits, nullptr);

            // Call the callback so the test doesn't wait indefinitely.
            api.CallAdapterRequestDeviceCallback(apiAdapter, WGPURequestDeviceStatus_Error, nullptr,
                                                 nullptr);
        })));
    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call).Times(1);

        FlushCallbacks();
    });
}

static void DeviceLostCallback(const wgpu::Device&,
                               wgpu::DeviceLostReason reason,
                               const char* message) {}

// Test that the DeviceDescriptor is not allowed to pass a device lost callback from the client to
// the server.
TEST_P(WireAdapterTests, RequestDeviceAssertsOnLostCallbackPointer) {
    wgpu::DeviceDescriptor desc = {};
    desc.SetDeviceLostCallback(wgpu::CallbackMode::AllowSpontaneous, DeviceLostCallback);

    AdapterRequestDevice(adapter, &desc);

    EXPECT_CALL(api, OnAdapterRequestDevice(apiAdapter, NotNull(), _))
        .WillOnce(WithArg<1>(Invoke([&](const WGPUDeviceDescriptor* apiDesc) {
            EXPECT_STREQ(apiDesc->label.data, desc.label.data);

            // The callback should not be passed through to the server, and it should be overridden.
            WGPUDeviceDescriptor& inputDesc = *reinterpret_cast<WGPUDeviceDescriptor*>(&desc);
            ASSERT_NE(apiDesc->deviceLostCallbackInfo2.callback,
                      inputDesc.deviceLostCallbackInfo2.callback);
            ASSERT_NE(apiDesc->deviceLostCallbackInfo2.callback, nullptr);
            ASSERT_NE(apiDesc->deviceLostCallbackInfo2.userdata1, nullptr);
            ASSERT_EQ(apiDesc->deviceLostCallbackInfo2.userdata2, nullptr);

            // Call the callback so the test doesn't wait indefinitely.
            api.CallAdapterRequestDeviceCallback(apiAdapter, WGPURequestDeviceStatus_Error, nullptr,
                                                 nullptr);
        })));
    FlushClient();
    FlushFutures();
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call).Times(1);

        FlushCallbacks();
    });
}

// Test that RequestDevice forwards the device information to the client.
TEST_P(WireAdapterTests, RequestDeviceSuccess) {
    wgpu::SupportedLimits fakeLimits = {};
    fakeLimits.limits.maxTextureDimension1D = 433;
    fakeLimits.limits.maxVertexAttributes = 1243;

    std::initializer_list<wgpu::FeatureName> fakeFeatures = {
        wgpu::FeatureName::Depth32FloatStencil8,
        wgpu::FeatureName::TextureCompressionBC,
    };

    wgpu::DeviceDescriptor desc = {};
    AdapterRequestDevice(adapter, &desc, this);

    // Expect the server to receive the message. Then, mock a fake reply.
    WGPUDevice apiDevice = api.GetNewDevice();
    // The backend device should not be known by the wire server.
    EXPECT_FALSE(GetWireServer()->IsDeviceKnown(apiDevice));

    EXPECT_CALL(api, OnAdapterRequestDevice(apiAdapter, NotNull(), _))
        .WillOnce(InvokeWithoutArgs([&] {
            // Set on device creation to forward callbacks to the client.
            EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, NotNull(), NotNull())).Times(1);

            EXPECT_CALL(api, DeviceGetLimits(apiDevice, NotNull()))
                .WillOnce(WithArg<1>(Invoke([&](WGPUSupportedLimits* limits) {
                    *reinterpret_cast<wgpu::SupportedLimits*>(limits) = fakeLimits;
                    return WGPUStatus_Success;
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

            // The backend device should still not be known by the wire server since the
            // callback has not been called yet.
            EXPECT_FALSE(GetWireServer()->IsDeviceKnown(apiDevice));
            api.CallAdapterRequestDeviceCallback(apiAdapter, WGPURequestDeviceStatus_Success,
                                                 apiDevice, nullptr);
            // After the callback is called, the backend device is now known by the server.
            EXPECT_TRUE(GetWireServer()->IsDeviceKnown(apiDevice));
        }));

    FlushClient();
    FlushFutures();

    wgpu::Device device;
    // Expect the callback in the client and all the device information to match.
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(WGPURequestDeviceStatus_Success, NotNull(), nullptr, this))
            .WillOnce(WithArg<1>(Invoke([&](WGPUDevice cDevice) {
                device = wgpu::Device::Acquire(cDevice);

                wgpu::SupportedLimits limits;
                EXPECT_EQ(device.GetLimits(&limits), wgpu::Status::Success);
                EXPECT_EQ(limits.limits.maxTextureDimension1D,
                          fakeLimits.limits.maxTextureDimension1D);
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
        FlushCallbacks();
    });

    EXPECT_EQ(device.GetAdapter().Get(), adapter.Get());

    device = nullptr;
    // Cleared when the device is destroyed.
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, DeviceRelease(apiDevice));

    // Server has not recevied the release yet, so the device should be known.
    EXPECT_TRUE(GetWireServer()->IsDeviceKnown(apiDevice));
    FlushClient();
    // After receiving the release call, the device is no longer known by the server.
    EXPECT_FALSE(GetWireServer()->IsDeviceKnown(apiDevice));
}

// Test that features requested that the implementation supports, but not the
// wire reject the callback.
TEST_P(WireAdapterTests, RequestFeatureUnsupportedByWire) {
    std::initializer_list<wgpu::FeatureName> fakeFeatures = {
        // Some value that is not a valid feature
        static_cast<wgpu::FeatureName>(-2),
        wgpu::FeatureName::TextureCompressionASTC,
    };

    wgpu::DeviceDescriptor desc = {};
    AdapterRequestDevice(adapter, &desc, this);

    // Expect the server to receive the message. Then, mock a fake reply.
    // The reply contains features that the device implementation supports, but the
    // wire does not.
    WGPUDevice apiDevice = api.GetNewDevice();
    EXPECT_CALL(api, OnAdapterRequestDevice(apiAdapter, NotNull(), _))
        .WillOnce(InvokeWithoutArgs([&] {
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
    FlushFutures();

    // Expect an error callback since the feature is not supported.
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(WGPURequestDeviceStatus_Error, nullptr, NotNull(), this)).Times(1);
        FlushCallbacks();
    });
}

// Test that RequestDevice errors forward to the client.
TEST_P(WireAdapterTests, RequestDeviceError) {
    wgpu::DeviceDescriptor desc = {};
    AdapterRequestDevice(adapter, &desc, this);

    // Expect the server to receive the message. Then, mock an error.
    EXPECT_CALL(api, OnAdapterRequestDevice(apiAdapter, NotNull(), _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallAdapterRequestDeviceCallback(apiAdapter, WGPURequestDeviceStatus_Error, nullptr,
                                                 "Request device failed");
        }));
    FlushClient();
    FlushFutures();

    // Expect the callback in the client.
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(WGPURequestDeviceStatus_Error, nullptr,
                                 StrEq("Request device failed"), this))
            .Times(1);
        FlushCallbacks();
    });
}

// Test that RequestDevice can complete successfully even if the adapter is deleted
// before the callback happens.
TEST_P(WireAdapterTests, RequestDeviceAdapterDestroyedBeforeCallback) {
    wgpu::DeviceDescriptor desc = {};
    AdapterRequestDevice(adapter, &desc, this);
    adapter = nullptr;

    wgpu::SupportedLimits fakeLimits = {};
    fakeLimits.limits.maxTextureDimension1D = 433;
    fakeLimits.limits.maxVertexAttributes = 1243;

    std::initializer_list<wgpu::FeatureName> fakeFeatures = {
        wgpu::FeatureName::Depth32FloatStencil8,
        wgpu::FeatureName::TextureCompressionBC,
    };

    // Mock a reply from the server.
    WGPUDevice apiDevice = api.GetNewDevice();
    EXPECT_CALL(api, OnAdapterRequestDevice(apiAdapter, NotNull(), _))
        .WillOnce(InvokeWithoutArgs([&] {
            // Set on device creation to forward callbacks to the client.
            EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, NotNull(), NotNull())).Times(1);

            EXPECT_CALL(api, DeviceGetLimits(apiDevice, NotNull()))
                .WillOnce(WithArg<1>(Invoke([&](WGPUSupportedLimits* limits) {
                    *reinterpret_cast<wgpu::SupportedLimits*>(limits) = fakeLimits;
                    return WGPUStatus_Success;
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
    FlushFutures();

    wgpu::Device device;
    // Expect the callback in the client.
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(WGPURequestDeviceStatus_Success, NotNull(), nullptr, this))
            .WillOnce(WithArg<1>(
                Invoke([&](WGPUDevice cDevice) { device = wgpu::Device::Acquire(cDevice); })));
        FlushCallbacks();
    });

    device = nullptr;
    // Cleared when the device is destroyed.
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, DeviceRelease(apiDevice));
    FlushClient();
}

// Test that RequestDevice receives unknown status if the wire is disconnected
// before the callback happens.
TEST_P(WireAdapterTests, RequestDeviceWireDisconnectedBeforeCallback) {
    wgpu::DeviceDescriptor desc = {};
    AdapterRequestDevice(adapter, &desc, this);

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(WGPURequestDeviceStatus_InstanceDropped, nullptr, NotNull(), this))
            .Times(1);

        GetWireClient()->Disconnect();
    });
}

}  // anonymous namespace
}  // namespace dawn::wire
