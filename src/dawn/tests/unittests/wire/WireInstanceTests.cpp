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

#include "dawn/common/StringViewUtils.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/tests/StringViewMatchers.h"
#include "dawn/tests/unittests/wire/WireFutureTest.h"
#include "dawn/tests/unittests/wire/WireTest.h"

#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireServer.h"

#include "webgpu/webgpu_cpp.h"

namespace dawn::wire {
namespace {

using testing::_;
using testing::EmptySizedString;
using testing::Invoke;
using testing::InvokeWithoutArgs;
using testing::MockCallback;
using testing::NiceMock;
using testing::NonEmptySizedString;
using testing::NotNull;
using testing::Return;
using testing::SetArgPointee;
using testing::SizedString;
using testing::StrEq;
using testing::WithArg;

class WireInstanceBasicTest : public WireTest {};

// Test that an Instance can be reserved and injected into the wire.
TEST_F(WireInstanceBasicTest, ReserveAndInject) {
    auto reserved = GetWireClient()->ReserveInstance();
    wgpu::Instance instance = wgpu::Instance::Acquire(reserved.instance);

    WGPUInstance apiInstance = api.GetNewInstance();
    EXPECT_CALL(api, InstanceAddRef(apiInstance));
    EXPECT_TRUE(GetWireServer()->InjectInstance(apiInstance, reserved.handle));

    instance = nullptr;

    EXPECT_CALL(api, InstanceRelease(apiInstance));
    FlushClient();
}

using WireInstanceTestBase = WireFutureTestWithParams<WGPURequestAdapterCallback,
                                                      WGPURequestAdapterCallbackInfo,
                                                      wgpuInstanceRequestAdapter,
                                                      wgpuInstanceRequestAdapterF>;
class WireInstanceTests : public WireInstanceTestBase {
  protected:
    // Overriden version of wgpuInstanceRequestAdapter that defers to the API call based on the
    // test callback mode.
    void InstanceRequestAdapter(const wgpu::Instance& i,
                                wgpu::RequestAdapterOptions const* options,
                                void* userdata = nullptr) {
        CallImpl(userdata, i.Get(), reinterpret_cast<const WGPURequestAdapterOptions*>(options));
    }
};

DAWN_INSTANTIATE_WIRE_FUTURE_TEST_P(WireInstanceTests);

// Test that RequestAdapterOptions are passed from the client to the server.
TEST_P(WireInstanceTests, RequestAdapterPassesOptions) {
    for (auto powerPreference :
         {wgpu::PowerPreference::LowPower, wgpu::PowerPreference::HighPerformance}) {
        wgpu::RequestAdapterOptions options = {};
        options.powerPreference = powerPreference;

        InstanceRequestAdapter(instance, &options, nullptr);

        EXPECT_CALL(api, OnInstanceRequestAdapter(apiInstance, NotNull(), _))
            .WillOnce(WithArg<1>(Invoke([&](const WGPURequestAdapterOptions* apiOptions) {
                EXPECT_EQ(apiOptions->powerPreference,
                          static_cast<WGPUPowerPreference>(options.powerPreference));
                EXPECT_EQ(apiOptions->forceFallbackAdapter, options.forceFallbackAdapter);
                api.CallInstanceRequestAdapterCallback(apiInstance, WGPURequestAdapterStatus_Error,
                                                       nullptr, kEmptyOutputStringView);
            })));

        FlushClient();
        FlushFutures();
        ExpectWireCallbacksWhen([&](auto& mockCb) {
            EXPECT_CALL(mockCb, Call).Times(1);

            FlushCallbacks();
        });
    }
}

// Test that RequestAdapter forwards the adapter information to the client.
TEST_P(WireInstanceTests, RequestAdapterSuccess) {
    wgpu::RequestAdapterOptions options = {};
    InstanceRequestAdapter(instance, &options, nullptr);

    WGPUAdapterInfo fakeInfo = {};
    fakeInfo.vendor = ToOutputStringView("fake-vendor");
    fakeInfo.architecture = ToOutputStringView("fake-architecture");
    fakeInfo.device = ToOutputStringView("fake-device");
    fakeInfo.description = ToOutputStringView("fake-description");
    fakeInfo.backendType = WGPUBackendType_D3D12;
    fakeInfo.adapterType = WGPUAdapterType_IntegratedGPU;
    fakeInfo.vendorID = 0x134;
    fakeInfo.deviceID = 0x918;

    WGPUSupportedLimits fakeLimits = {};
    fakeLimits.nextInChain = nullptr;
    fakeLimits.limits.maxTextureDimension1D = 433;
    fakeLimits.limits.maxVertexAttributes = 1243;

    std::initializer_list<WGPUFeatureName> fakeFeatures = {
        WGPUFeatureName_Depth32FloatStencil8,
        WGPUFeatureName_TextureCompressionBC,
    };

    // Expect the server to receive the message. Then, mock a fake reply.
    WGPUAdapter apiAdapter = api.GetNewAdapter();
    EXPECT_CALL(api, OnInstanceRequestAdapter(apiInstance, NotNull(), _))
        .WillOnce(InvokeWithoutArgs([&] {
            EXPECT_CALL(api, AdapterHasFeature(apiAdapter, _)).WillRepeatedly(Return(false));

            EXPECT_CALL(api, AdapterGetInfo(apiAdapter, NotNull()))
                .WillOnce(WithArg<1>(Invoke([&](WGPUAdapterInfo* info) {
                    *info = fakeInfo;
                    return WGPUStatus_Success;
                })));

            EXPECT_CALL(api, AdapterGetLimits(apiAdapter, NotNull()))
                .WillOnce(WithArg<1>(Invoke([&](WGPUSupportedLimits* limits) {
                    *limits = fakeLimits;
                    return WGPUStatus_Success;
                })));

            EXPECT_CALL(api, AdapterEnumerateFeatures(apiAdapter, nullptr))
                .WillOnce(Return(fakeFeatures.size()));

            EXPECT_CALL(api, AdapterEnumerateFeatures(apiAdapter, NotNull()))
                .WillOnce(WithArg<1>(Invoke([&](WGPUFeatureName* features) {
                    for (WGPUFeatureName feature : fakeFeatures) {
                        *(features++) = feature;
                    }
                    return fakeFeatures.size();
                })));
            api.CallInstanceRequestAdapterCallback(apiInstance, WGPURequestAdapterStatus_Success,
                                                   apiAdapter, kEmptyOutputStringView);
        }));

    FlushClient();
    FlushFutures();

    // Expect the callback in the client and all the adapter information to match.
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb,
                    Call(WGPURequestAdapterStatus_Success, NotNull(), EmptySizedString(), nullptr))
            .WillOnce(WithArg<1>(Invoke([&](WGPUAdapter adapter) {
                WGPUAdapterInfo info = {};
                wgpuAdapterGetInfo(adapter, &info);
                EXPECT_NE(info.vendor.length, WGPU_STRLEN);
                EXPECT_EQ(info.vendor, fakeInfo.vendor);
                EXPECT_NE(info.architecture.length, WGPU_STRLEN);
                EXPECT_EQ(info.architecture, fakeInfo.architecture);
                EXPECT_NE(info.device.length, WGPU_STRLEN);
                EXPECT_EQ(info.device, fakeInfo.device);
                EXPECT_NE(info.description.length, WGPU_STRLEN);
                EXPECT_EQ(info.description, fakeInfo.description);
                EXPECT_EQ(info.backendType, fakeInfo.backendType);
                EXPECT_EQ(info.adapterType, fakeInfo.adapterType);
                EXPECT_EQ(info.vendorID, fakeInfo.vendorID);
                EXPECT_EQ(info.deviceID, fakeInfo.deviceID);

                WGPUSupportedLimits limits = {};
                EXPECT_EQ(wgpuAdapterGetLimits(adapter, &limits), WGPUStatus_Success);
                EXPECT_EQ(limits.limits.maxTextureDimension1D,
                          fakeLimits.limits.maxTextureDimension1D);
                EXPECT_EQ(limits.limits.maxVertexAttributes, fakeLimits.limits.maxVertexAttributes);

                std::vector<WGPUFeatureName> features;
                features.resize(wgpuAdapterEnumerateFeatures(adapter, nullptr));
                ASSERT_EQ(features.size(), fakeFeatures.size());
                EXPECT_EQ(wgpuAdapterEnumerateFeatures(adapter, &features[0]), features.size());

                std::unordered_set<WGPUFeatureName> featureSet(fakeFeatures);
                for (WGPUFeatureName feature : features) {
                    EXPECT_EQ(featureSet.erase(feature), 1u);
                }
            })));

        FlushCallbacks();
    });
}

// Test that RequestAdapter forwards all chained properties to the client.
TEST_P(WireInstanceTests, RequestAdapterPassesChainedProperties) {
    wgpu::RequestAdapterOptions options = {};
    InstanceRequestAdapter(instance, &options, nullptr);

    WGPUMemoryHeapInfo fakeHeapInfo[3] = {
        {WGPUHeapProperty_DeviceLocal, 64},
        {WGPUHeapProperty_DeviceLocal | WGPUHeapProperty_HostVisible, 136},
        {WGPUHeapProperty_HostCached | WGPUHeapProperty_HostVisible, 460},
    };

    WGPUAdapterPropertiesMemoryHeaps fakeMemoryHeapProperties = {};
    fakeMemoryHeapProperties.chain.sType = WGPUSType_AdapterPropertiesMemoryHeaps;
    fakeMemoryHeapProperties.heapCount = 3;
    fakeMemoryHeapProperties.heapInfo = fakeHeapInfo;

    WGPUAdapterPropertiesD3D fakeD3DProperties = {};
    fakeD3DProperties.chain.sType = WGPUSType_AdapterPropertiesD3D;
    fakeD3DProperties.shaderModel = 61;

    WGPUAdapterPropertiesVk fakeVkProperties = {};
    fakeVkProperties.chain.sType = WGPUSType_AdapterPropertiesVk;
    fakeVkProperties.driverVersion = 0x801F6000;

    std::initializer_list<WGPUFeatureName> fakeFeatures = {
        WGPUFeatureName_AdapterPropertiesMemoryHeaps,
        WGPUFeatureName_AdapterPropertiesD3D,
        WGPUFeatureName_AdapterPropertiesVk,
    };

    // Expect the server to receive the message. Then, mock a fake reply.
    WGPUAdapter apiAdapter = api.GetNewAdapter();
    EXPECT_CALL(api, OnInstanceRequestAdapter(apiInstance, NotNull(), _))
        .WillOnce(InvokeWithoutArgs([&] {
            for (WGPUFeatureName feature : fakeFeatures) {
                EXPECT_CALL(api, AdapterHasFeature(apiAdapter, feature)).WillOnce(Return(true));
            }

            EXPECT_CALL(api, AdapterGetInfo(apiAdapter, NotNull()))
                .WillOnce(WithArg<1>(Invoke([&](WGPUAdapterInfo* info) {
                    info->vendor = ToOutputStringView("fake-vendor");
                    info->architecture = ToOutputStringView("fake-architecture");
                    info->device = ToOutputStringView("fake-device");
                    info->description = ToOutputStringView("fake-description");

                    WGPUChainedStructOut* chain = info->nextInChain;
                    while (chain != nullptr) {
                        auto* next = chain->next;
                        switch (chain->sType) {
                            case WGPUSType_AdapterPropertiesMemoryHeaps:
                                *reinterpret_cast<WGPUAdapterPropertiesMemoryHeaps*>(chain) =
                                    fakeMemoryHeapProperties;
                                break;
                            case WGPUSType_AdapterPropertiesD3D:
                                *reinterpret_cast<WGPUAdapterPropertiesD3D*>(chain) =
                                    fakeD3DProperties;
                                break;
                            case WGPUSType_AdapterPropertiesVk:
                                *reinterpret_cast<WGPUAdapterPropertiesVk*>(chain) =
                                    fakeVkProperties;
                                break;
                            default:
                                ADD_FAILURE() << "Unexpected chain";
                                return WGPUStatus_Error;
                        }
                        // update next pointer back to the original since it would be overwritten
                        // in the switch statement
                        chain->next = next;

                        chain = next;
                    }
                    return WGPUStatus_Success;
                })));

            EXPECT_CALL(api, AdapterGetLimits(apiAdapter, NotNull()))
                .WillOnce(WithArg<1>(Invoke([&](WGPUSupportedLimits* limits) {
                    *limits = {};
                    return WGPUStatus_Success;
                })));

            EXPECT_CALL(api, AdapterEnumerateFeatures(apiAdapter, nullptr))
                .WillOnce(Return(fakeFeatures.size()));

            EXPECT_CALL(api, AdapterEnumerateFeatures(apiAdapter, NotNull()))
                .WillOnce(WithArg<1>(Invoke([&](WGPUFeatureName* features) {
                    for (WGPUFeatureName feature : fakeFeatures) {
                        *(features++) = feature;
                    }
                    return fakeFeatures.size();
                })));
            api.CallInstanceRequestAdapterCallback(apiInstance, WGPURequestAdapterStatus_Success,
                                                   apiAdapter, kEmptyOutputStringView);
        }));

    FlushClient();
    FlushFutures();

    // Expect the callback in the client and the adapter information to match.
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb,
                    Call(WGPURequestAdapterStatus_Success, NotNull(), EmptySizedString(), nullptr))
            .WillOnce(WithArg<1>(Invoke([&](WGPUAdapter adapter) {
                // Request info without a chained struct.
                // It should be nullptr.
                WGPUAdapterInfo info = {};
                wgpuAdapterGetInfo(adapter, &info);
                EXPECT_EQ(info.nextInChain, nullptr);

                // Request the memory heap properties.
                WGPUAdapterPropertiesMemoryHeaps memoryHeapProperties = {};
                memoryHeapProperties.chain.sType = WGPUSType_AdapterPropertiesMemoryHeaps;
                info.nextInChain = &memoryHeapProperties.chain;
                wgpuAdapterGetInfo(adapter, &info);

                // Expect everything matches the fake properties returned by the server.
                EXPECT_EQ(memoryHeapProperties.heapCount, fakeMemoryHeapProperties.heapCount);
                for (size_t i = 0; i < fakeMemoryHeapProperties.heapCount; ++i) {
                    EXPECT_EQ(memoryHeapProperties.heapInfo[i].properties,
                              fakeMemoryHeapProperties.heapInfo[i].properties);
                    EXPECT_EQ(memoryHeapProperties.heapInfo[i].size,
                              fakeMemoryHeapProperties.heapInfo[i].size);
                }

                // Get the D3D properties.
                WGPUAdapterPropertiesD3D d3dProperties = {};
                d3dProperties.chain.sType = WGPUSType_AdapterPropertiesD3D;
                info.nextInChain = &d3dProperties.chain;
                wgpuAdapterGetInfo(adapter, &info);
                // Expect them to match.
                EXPECT_EQ(d3dProperties.shaderModel, fakeD3DProperties.shaderModel);

                // Get the Vulkan properties.
                WGPUAdapterPropertiesVk vkProperties = {};
                vkProperties.chain.sType = WGPUSType_AdapterPropertiesVk;
                info.nextInChain = &vkProperties.chain;
                wgpuAdapterGetInfo(adapter, &info);
                // Expect them to match.
                EXPECT_EQ(vkProperties.driverVersion, fakeVkProperties.driverVersion);
            })));

        FlushCallbacks();
    });
}

// Test that features returned by the implementation that aren't supported in the wire are not
// exposed.
TEST_P(WireInstanceTests, RequestAdapterWireLacksFeatureSupport) {
    wgpu::RequestAdapterOptions options = {};
    InstanceRequestAdapter(instance, &options, nullptr);

    std::initializer_list<WGPUFeatureName> fakeFeatures = {
        WGPUFeatureName_Depth32FloatStencil8,
        // Some value that is not a valid feature
        static_cast<WGPUFeatureName>(-2),
    };

    // Expect the server to receive the message. Then, mock a fake reply.
    WGPUAdapter apiAdapter = api.GetNewAdapter();
    EXPECT_CALL(api, OnInstanceRequestAdapter(apiInstance, NotNull(), _))
        .WillOnce(InvokeWithoutArgs([&] {
            EXPECT_CALL(api, AdapterHasFeature(apiAdapter, _)).WillRepeatedly(Return(false));

            EXPECT_CALL(api, AdapterGetInfo(apiAdapter, NotNull()))
                .WillOnce(WithArg<1>(Invoke([&](WGPUAdapterInfo* info) {
                    info->vendor = ToOutputStringView("fake-vendor");
                    info->architecture = ToOutputStringView("fake-architecture");
                    info->device = ToOutputStringView("fake-device");
                    info->description = ToOutputStringView("fake-description");
                    return WGPUStatus_Success;
                })));

            EXPECT_CALL(api, AdapterGetLimits(apiAdapter, NotNull()))
                .WillOnce(WithArg<1>(Invoke([&](WGPUSupportedLimits* limits) {
                    *limits = {};
                    return WGPUStatus_Success;
                })));

            EXPECT_CALL(api, AdapterEnumerateFeatures(apiAdapter, nullptr))
                .WillOnce(Return(fakeFeatures.size()));

            EXPECT_CALL(api, AdapterEnumerateFeatures(apiAdapter, NotNull()))
                .WillOnce(WithArg<1>(Invoke([&](WGPUFeatureName* features) {
                    for (WGPUFeatureName feature : fakeFeatures) {
                        *(features++) = feature;
                    }
                    return fakeFeatures.size();
                })));
            api.CallInstanceRequestAdapterCallback(apiInstance, WGPURequestAdapterStatus_Success,
                                                   apiAdapter, kEmptyOutputStringView);
        }));

    FlushClient();
    FlushFutures();

    // Expect the callback in the client and all the adapter information to match.
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb,
                    Call(WGPURequestAdapterStatus_Success, NotNull(), EmptySizedString(), nullptr))
            .WillOnce(WithArg<1>(Invoke([&](WGPUAdapter adapter) {
                WGPUFeatureName feature;
                ASSERT_EQ(wgpuAdapterEnumerateFeatures(adapter, nullptr), 1u);
                wgpuAdapterEnumerateFeatures(adapter, &feature);
                EXPECT_EQ(feature, WGPUFeatureName_Depth32FloatStencil8);
            })));

        FlushCallbacks();
    });
}

// Test that RequestAdapter errors forward to the client.
TEST_P(WireInstanceTests, RequestAdapterError) {
    wgpu::RequestAdapterOptions options = {};
    InstanceRequestAdapter(instance, &options, nullptr);

    // Expect the server to receive the message. Then, mock an error.
    EXPECT_CALL(api, OnInstanceRequestAdapter(apiInstance, NotNull(), _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallInstanceRequestAdapterCallback(apiInstance, WGPURequestAdapterStatus_Error,
                                                   nullptr, ToOutputStringView("Some error"));
        }));

    FlushClient();
    FlushFutures();

    // Expect the callback in the client.
    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(WGPURequestAdapterStatus_Error, nullptr, SizedString("Some error"),
                                 nullptr))
            .Times(1);

        FlushCallbacks();
    });
}

// Test that RequestAdapter receives unknown status if the instance is deleted before the callback
// happens.
TEST_P(WireInstanceTests, DISABLED_RequestAdapterInstanceDestroyedBeforeCallback) {
    // TODO(crbug.com/dawn/2061) This test does not currently pass because the callbacks aren't
    // actually triggered by the destruction of the instance at the moment. Once we move the
    // EventManager to be per-Instance, this test should pass.
    wgpu::RequestAdapterOptions options = {};
    InstanceRequestAdapter(instance, &options, nullptr);

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb,
                    Call(WGPURequestAdapterStatus_Unknown, nullptr, NonEmptySizedString(), nullptr))
            .Times(1);

        instance = nullptr;
    });
}

// Test that RequestAdapter receives unknown status if the wire is disconnected
// before the callback happens.
TEST_P(WireInstanceTests, RequestAdapterWireDisconnectBeforeCallback) {
    wgpu::RequestAdapterOptions options = {};
    InstanceRequestAdapter(instance, &options, nullptr);

    ExpectWireCallbacksWhen([&](auto& mockCb) {
        EXPECT_CALL(mockCb, Call(WGPURequestAdapterStatus_InstanceDropped, nullptr,
                                 NonEmptySizedString(), nullptr))
            .Times(1);

        GetWireClient()->Disconnect();
    });
}

}  // anonymous namespace
}  // namespace dawn::wire
