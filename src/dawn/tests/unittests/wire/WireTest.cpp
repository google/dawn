// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/tests/unittests/wire/WireTest.h"

#include "dawn/dawn_proc.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/utils/TerribleCommandBuffer.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireServer.h"

using testing::_;
using testing::AnyNumber;
using testing::AtMost;
using testing::Exactly;
using testing::Invoke;
using testing::Mock;
using testing::MockCallback;
using testing::NotNull;
using testing::Return;
using testing::SaveArg;
using testing::WithArg;

WireTest::WireTest() {}

WireTest::~WireTest() {}

dawn::wire::client::MemoryTransferService* WireTest::GetClientMemoryTransferService() {
    return nullptr;
}

dawn::wire::server::MemoryTransferService* WireTest::GetServerMemoryTransferService() {
    return nullptr;
}

void WireTest::SetUp() {
    DawnProcTable mockProcs;
    api.GetProcTable(&mockProcs);
    SetupIgnoredCallExpectations();

    mS2cBuf = std::make_unique<dawn::utils::TerribleCommandBuffer>();
    mC2sBuf = std::make_unique<dawn::utils::TerribleCommandBuffer>(mWireServer.get());

    dawn::wire::WireServerDescriptor serverDesc = {};
    serverDesc.procs = &mockProcs;
    serverDesc.serializer = mS2cBuf.get();
    serverDesc.memoryTransferService = GetServerMemoryTransferService();

    mWireServer.reset(new dawn::wire::WireServer(serverDesc));
    mC2sBuf->SetHandler(mWireServer.get());

    dawn::wire::WireClientDescriptor clientDesc = {};
    clientDesc.serializer = mC2sBuf.get();
    clientDesc.memoryTransferService = GetClientMemoryTransferService();

    mWireClient.reset(new dawn::wire::WireClient(clientDesc));
    mS2cBuf->SetHandler(mWireClient.get());

    dawnProcSetProcs(&dawn::wire::client::GetProcs());

    auto reservedInstance = GetWireClient()->ReserveInstance();
    instance = wgpu::Instance::Acquire(reservedInstance.instance);
    apiInstance = api.GetNewInstance();
    EXPECT_CALL(api, InstanceAddRef(apiInstance));
    EXPECT_TRUE(GetWireServer()->InjectInstance(apiInstance, reservedInstance.handle));

    // Create the adapter for testing.
    apiAdapter = api.GetNewAdapter();
    MockCallback<void (*)(wgpu::RequestAdapterStatus, wgpu::Adapter, const char*, void*)> adapterCb;
    instance.RequestAdapter(nullptr, wgpu::CallbackMode::AllowSpontaneous, adapterCb.Callback(),
                            adapterCb.MakeUserdata(this));

    EXPECT_CALL(api, OnInstanceRequestAdapter2(apiInstance, _, _)).WillOnce([&]() {
        EXPECT_CALL(api, AdapterHasFeature(apiAdapter, _)).WillRepeatedly(Return(false));

        EXPECT_CALL(api, AdapterGetInfo(apiAdapter, NotNull()))
            .WillOnce(WithArg<1>(Invoke([&](WGPUAdapterInfo* info) {
                *info = {};
                info->vendor = "";
                info->architecture = "";
                info->device = "";
                info->description = "";
                return WGPUStatus_Success;
            })));

        EXPECT_CALL(api, AdapterGetLimits(apiAdapter, NotNull()))
            .WillOnce(WithArg<1>(Invoke([&](WGPUSupportedLimits* limits) {
                *limits = {};
                return WGPUStatus_Success;
            })));

        EXPECT_CALL(api, AdapterEnumerateFeatures(apiAdapter, nullptr))
            .WillOnce(Return(0))
            .WillOnce(Return(0));

        api.CallInstanceRequestAdapter2Callback(apiInstance, WGPURequestAdapterStatus_Success,
                                                apiAdapter, nullptr);
    });
    FlushClient();
    EXPECT_CALL(adapterCb, Call(wgpu::RequestAdapterStatus::Success, NotNull(), nullptr, this))
        .WillOnce(SaveArg<1>(&adapter));
    FlushServer();
    EXPECT_NE(adapter, nullptr);

    // Create the device for testing.
    apiDevice = api.GetNewDevice();
    wgpu::DeviceDescriptor deviceDesc = {};
    deviceDesc.deviceLostCallbackInfo = {nullptr, wgpu::CallbackMode::AllowSpontaneous,
                                         deviceLostCallback.Callback(),
                                         deviceLostCallback.MakeUserdata(this)};
    deviceDesc.uncapturedErrorCallbackInfo = {nullptr, uncapturedErrorCallback.Callback(),
                                              uncapturedErrorCallback.MakeUserdata(this)};
    EXPECT_CALL(deviceLostCallback, Call).Times(AtMost(1));

    MockCallback<void (*)(wgpu::RequestDeviceStatus, wgpu::Device, const char*, void*)> deviceCb;
    adapter.RequestDevice(&deviceDesc, wgpu::CallbackMode::AllowSpontaneous, deviceCb.Callback(),
                          deviceCb.MakeUserdata(this));
    EXPECT_CALL(api, OnAdapterRequestDevice2(apiAdapter, NotNull(), _))
        .WillOnce(WithArg<1>([&](const WGPUDeviceDescriptor* desc) {
            // Set on device creation to forward callbacks to the client.
            EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, NotNull(), NotNull())).Times(1);

            // The mock objects currently require us to manually set the callbacks because we
            // are no longer explicitly calling the setters anymore.
            ProcTableAsClass::Object* object =
                reinterpret_cast<ProcTableAsClass::Object*>(apiDevice);
            object->mDeviceLostCallback = desc->deviceLostCallbackInfo2.callback;
            object->mDeviceLostUserdata1 = desc->deviceLostCallbackInfo2.userdata1;
            object->mDeviceLostUserdata2 = desc->deviceLostCallbackInfo2.userdata2;
            object->mUncapturedErrorCallback = desc->uncapturedErrorCallbackInfo2.callback;
            object->mUncapturedErrorUserdata1 = desc->uncapturedErrorCallbackInfo2.userdata1;
            object->mUncapturedErrorUserdata2 = desc->uncapturedErrorCallbackInfo2.userdata2;

            EXPECT_CALL(api, DeviceGetLimits(apiDevice, NotNull()))
                .WillOnce(WithArg<1>(Invoke([&](WGPUSupportedLimits* limits) {
                    *limits = {};
                    return WGPUStatus_Success;
                })));

            EXPECT_CALL(api, DeviceEnumerateFeatures(apiDevice, nullptr))
                .WillOnce(Return(0))
                .WillOnce(Return(0));

            api.CallAdapterRequestDevice2Callback(apiAdapter, WGPURequestDeviceStatus_Success,
                                                  apiDevice, nullptr);
        }));
    FlushClient();
    EXPECT_CALL(deviceCb, Call(wgpu::RequestDeviceStatus::Success, NotNull(), nullptr, this))
        .WillOnce(SaveArg<1>(&device));
    FlushServer();
    EXPECT_NE(device, nullptr);

    // The GetQueue is done on WireClient startup so we expect it now.
    queue = device.GetQueue();
    apiQueue = api.GetNewQueue();
    EXPECT_CALL(api, DeviceGetQueue(apiDevice)).WillOnce(Return(apiQueue));
    FlushClient();

    cDevice = device.Get();
    cQueue = queue.Get();
}

void WireTest::TearDown() {
    instance = nullptr;
    adapter = nullptr;
    device = nullptr;
    queue = nullptr;
    dawnProcSetProcs(nullptr);

    // Derived classes should call the base TearDown() first. The client must
    // be reset before any mocks are deleted.
    // Incomplete client callbacks will be called on deletion, so the mocks
    // cannot be null.
    api.IgnoreAllReleaseCalls();
    mS2cBuf->SetHandler(nullptr);
    mWireClient = nullptr;

    if (mWireServer && apiDevice) {
        // These are called on server destruction to clear the callbacks. They must not be
        // called after the server is destroyed.
        EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, nullptr, nullptr)).Times(Exactly(1));
    }
    mC2sBuf->SetHandler(nullptr);
    mWireServer = nullptr;
}

// This should be called if |apiDevice| no longer exists on the wire.
// This signals that expectations in |TearDown| shouldn't be added.
void WireTest::DefaultApiDeviceWasReleased() {
    apiDevice = nullptr;
}

// This should be called if |apiAdapter| no longer exists on the wire.
// This signals that expectations in |TearDown| shouldn't be added.
void WireTest::DefaultApiAdapterWasReleased() {
    apiAdapter = nullptr;
}

void WireTest::FlushClient(bool success) {
    ASSERT_EQ(mC2sBuf->Flush(), success);

    Mock::VerifyAndClearExpectations(&api);
    SetupIgnoredCallExpectations();
}

void WireTest::FlushServer(bool success) {
    ASSERT_EQ(mS2cBuf->Flush(), success);
}

dawn::wire::WireServer* WireTest::GetWireServer() {
    return mWireServer.get();
}

dawn::wire::WireClient* WireTest::GetWireClient() {
    return mWireClient.get();
}

void WireTest::DeleteServer() {
    EXPECT_CALL(api, QueueRelease(apiQueue)).Times(1);
    EXPECT_CALL(api, DeviceRelease(apiDevice)).Times(1);
    EXPECT_CALL(api, AdapterRelease(apiAdapter)).Times(1);
    EXPECT_CALL(api, InstanceRelease(apiInstance)).Times(1);

    if (mWireServer) {
        // These are called on server destruction to clear the callbacks. They must not be
        // called after the server is destroyed.
        EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, nullptr, nullptr)).Times(Exactly(1));
    }
    mC2sBuf->SetHandler(nullptr);
    mWireServer = nullptr;
}

void WireTest::DeleteClient() {
    mS2cBuf->SetHandler(nullptr);
    mWireClient = nullptr;
}

void WireTest::SetupIgnoredCallExpectations() {
    EXPECT_CALL(api, InstanceProcessEvents(_)).Times(AnyNumber());
    EXPECT_CALL(api, DeviceTick(_)).Times(AnyNumber());
}
