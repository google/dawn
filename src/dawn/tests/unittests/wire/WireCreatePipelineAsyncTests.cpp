// Copyright 2020 The Dawn & Tint Authors
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

#include "dawn/dawn_proc.h"
#include "dawn/native/DawnNative.h"
#include "dawn/tests/unittests/wire/WireTest.h"
#include "dawn/utils/TerribleCommandBuffer.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireServer.h"

namespace dawn::wire {
namespace {

using testing::_;
using testing::InvokeWithoutArgs;
using testing::Mock;
using testing::NotNull;
using testing::Return;
using testing::Sequence;
using testing::StrEq;
using testing::StrictMock;

// Mock class to add expectations on the wire calling callbacks
class MockCreateComputePipelineAsyncCallback {
  public:
    MOCK_METHOD(void,
                Call,
                (WGPUCreatePipelineAsyncStatus status,
                 WGPUComputePipeline pipeline,
                 const char* message,
                 void* userdata));
};

std::unique_ptr<StrictMock<MockCreateComputePipelineAsyncCallback>>
    mockCreateComputePipelineAsyncCallback;
void ToMockCreateComputePipelineAsyncCallback(WGPUCreatePipelineAsyncStatus status,
                                              WGPUComputePipeline pipeline,
                                              const char* message,
                                              void* userdata) {
    mockCreateComputePipelineAsyncCallback->Call(status, pipeline, message, userdata);
}

class MockCreateRenderPipelineAsyncCallback {
  public:
    MOCK_METHOD(void,
                Call,
                (WGPUCreatePipelineAsyncStatus status,
                 WGPURenderPipeline pipeline,
                 const char* message,
                 void* userdata));
};

std::unique_ptr<StrictMock<MockCreateRenderPipelineAsyncCallback>>
    mockCreateRenderPipelineAsyncCallback;
void ToMockCreateRenderPipelineAsyncCallback(WGPUCreatePipelineAsyncStatus status,
                                             WGPURenderPipeline pipeline,
                                             const char* message,
                                             void* userdata) {
    mockCreateRenderPipelineAsyncCallback->Call(status, pipeline, message, userdata);
}

class WireCreatePipelineAsyncTest : public WireTest {
  public:
    void SetUp() override {
        WireTest::SetUp();

        mockCreateComputePipelineAsyncCallback =
            std::make_unique<StrictMock<MockCreateComputePipelineAsyncCallback>>();
        mockCreateRenderPipelineAsyncCallback =
            std::make_unique<StrictMock<MockCreateRenderPipelineAsyncCallback>>();
    }

    void TearDown() override {
        WireTest::TearDown();

        // Delete mock so that expectations are checked
        mockCreateComputePipelineAsyncCallback = nullptr;
        mockCreateRenderPipelineAsyncCallback = nullptr;
    }

    void FlushClient() {
        WireTest::FlushClient();
        Mock::VerifyAndClearExpectations(&mockCreateComputePipelineAsyncCallback);
    }

    void FlushServer() {
        WireTest::FlushServer();
        Mock::VerifyAndClearExpectations(&mockCreateComputePipelineAsyncCallback);
    }
};

// Test when creating a compute pipeline with CreateComputePipelineAsync() successfully.
TEST_F(WireCreatePipelineAsyncTest, CreateComputePipelineAsyncSuccess) {
    WGPUShaderModuleDescriptor csDescriptor{};
    WGPUShaderModule csModule = wgpuDeviceCreateShaderModule(device, &csDescriptor);
    WGPUShaderModule apiCsModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiCsModule));

    WGPUComputePipelineDescriptor descriptor{};
    descriptor.compute.module = csModule;
    descriptor.compute.entryPoint = "main";

    wgpuDeviceCreateComputePipelineAsync(device, &descriptor,
                                         ToMockCreateComputePipelineAsyncCallback, this);

    EXPECT_CALL(api, OnDeviceCreateComputePipelineAsync(apiDevice, _, _, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallDeviceCreateComputePipelineAsyncCallback(
                apiDevice, WGPUCreatePipelineAsyncStatus_Success, nullptr, "");
        }));

    FlushClient();

    EXPECT_CALL(*mockCreateComputePipelineAsyncCallback,
                Call(WGPUCreatePipelineAsyncStatus_Success, NotNull(), StrEq(""), this))
        .Times(1);

    FlushServer();
}

// Test when creating a compute pipeline with CreateComputePipelineAsync() results in an error.
TEST_F(WireCreatePipelineAsyncTest, CreateComputePipelineAsyncError) {
    WGPUShaderModuleDescriptor csDescriptor{};
    WGPUShaderModule csModule = wgpuDeviceCreateShaderModule(device, &csDescriptor);
    WGPUShaderModule apiCsModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiCsModule));

    WGPUComputePipelineDescriptor descriptor{};
    descriptor.compute.module = csModule;
    descriptor.compute.entryPoint = "main";

    wgpuDeviceCreateComputePipelineAsync(device, &descriptor,
                                         ToMockCreateComputePipelineAsyncCallback, this);

    EXPECT_CALL(api, OnDeviceCreateComputePipelineAsync(apiDevice, _, _, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallDeviceCreateComputePipelineAsyncCallback(
                apiDevice, WGPUCreatePipelineAsyncStatus_ValidationError, nullptr,
                "Some error message");
        }));

    FlushClient();

    EXPECT_CALL(
        *mockCreateComputePipelineAsyncCallback,
        Call(WGPUCreatePipelineAsyncStatus_ValidationError, _, StrEq("Some error message"), this))
        .Times(1);

    FlushServer();
}

// Test when creating a render pipeline with CreateRenderPipelineAsync() successfully.
TEST_F(WireCreatePipelineAsyncTest, CreateRenderPipelineAsyncSuccess) {
    WGPUShaderModuleDescriptor vertexDescriptor = {};
    WGPUShaderModule vsModule = wgpuDeviceCreateShaderModule(device, &vertexDescriptor);
    WGPUShaderModule apiVsModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiVsModule));

    WGPURenderPipelineDescriptor pipelineDescriptor{};
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.vertex.entryPoint = "main";

    WGPUFragmentState fragment = {};
    fragment.module = vsModule;
    fragment.entryPoint = "main";
    pipelineDescriptor.fragment = &fragment;

    wgpuDeviceCreateRenderPipelineAsync(device, &pipelineDescriptor,
                                        ToMockCreateRenderPipelineAsyncCallback, this);
    EXPECT_CALL(api, OnDeviceCreateRenderPipelineAsync(apiDevice, _, _, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallDeviceCreateRenderPipelineAsyncCallback(
                apiDevice, WGPUCreatePipelineAsyncStatus_Success, nullptr, "");
        }));

    FlushClient();

    EXPECT_CALL(*mockCreateRenderPipelineAsyncCallback,
                Call(WGPUCreatePipelineAsyncStatus_Success, NotNull(), StrEq(""), this))
        .Times(1);

    FlushServer();
}

// Test when creating a render pipeline with CreateRenderPipelineAsync() results in an error.
TEST_F(WireCreatePipelineAsyncTest, CreateRenderPipelineAsyncError) {
    WGPUShaderModuleDescriptor vertexDescriptor = {};
    WGPUShaderModule vsModule = wgpuDeviceCreateShaderModule(device, &vertexDescriptor);
    WGPUShaderModule apiVsModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiVsModule));

    WGPURenderPipelineDescriptor pipelineDescriptor{};
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.vertex.entryPoint = "main";

    WGPUFragmentState fragment = {};
    fragment.module = vsModule;
    fragment.entryPoint = "main";
    pipelineDescriptor.fragment = &fragment;

    wgpuDeviceCreateRenderPipelineAsync(device, &pipelineDescriptor,
                                        ToMockCreateRenderPipelineAsyncCallback, this);
    EXPECT_CALL(api, OnDeviceCreateRenderPipelineAsync(apiDevice, _, _, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallDeviceCreateRenderPipelineAsyncCallback(
                apiDevice, WGPUCreatePipelineAsyncStatus_ValidationError, nullptr,
                "Some error message");
        }));

    FlushClient();

    EXPECT_CALL(
        *mockCreateRenderPipelineAsyncCallback,
        Call(WGPUCreatePipelineAsyncStatus_ValidationError, _, StrEq("Some error message"), this))
        .Times(1);

    FlushServer();
}

// Test that registering a callback then wire disconnect calls the callback with
// Success.
TEST_F(WireCreatePipelineAsyncTest, CreateRenderPipelineAsyncThenDisconnect) {
    WGPUShaderModuleDescriptor vertexDescriptor = {};
    WGPUShaderModule vsModule = wgpuDeviceCreateShaderModule(device, &vertexDescriptor);
    WGPUShaderModule apiVsModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiVsModule));

    WGPUFragmentState fragment = {};
    fragment.module = vsModule;
    fragment.entryPoint = "main";

    WGPURenderPipelineDescriptor pipelineDescriptor{};
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.vertex.entryPoint = "main";
    pipelineDescriptor.fragment = &fragment;

    wgpuDeviceCreateRenderPipelineAsync(device, &pipelineDescriptor,
                                        ToMockCreateRenderPipelineAsyncCallback, this);
    EXPECT_CALL(api, OnDeviceCreateRenderPipelineAsync(apiDevice, _, _, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallDeviceCreateRenderPipelineAsyncCallback(
                apiDevice, WGPUCreatePipelineAsyncStatus_Success, nullptr, "");
        }));

    FlushClient();

    EXPECT_CALL(*mockCreateRenderPipelineAsyncCallback,
                Call(WGPUCreatePipelineAsyncStatus_Success, NotNull(), StrEq(""), this))
        .Times(1);
    GetWireClient()->Disconnect();
}

// Test that registering a callback then wire disconnect calls the callback with
// Success.
TEST_F(WireCreatePipelineAsyncTest, CreateComputePipelineAsyncThenDisconnect) {
    WGPUShaderModuleDescriptor csDescriptor{};
    WGPUShaderModule csModule = wgpuDeviceCreateShaderModule(device, &csDescriptor);
    WGPUShaderModule apiCsModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiCsModule));

    WGPUComputePipelineDescriptor descriptor{};
    descriptor.compute.module = csModule;
    descriptor.compute.entryPoint = "main";

    wgpuDeviceCreateComputePipelineAsync(device, &descriptor,
                                         ToMockCreateComputePipelineAsyncCallback, this);
    EXPECT_CALL(api, OnDeviceCreateComputePipelineAsync(apiDevice, _, _, _))
        .WillOnce(InvokeWithoutArgs([&] {
            api.CallDeviceCreateComputePipelineAsyncCallback(
                apiDevice, WGPUCreatePipelineAsyncStatus_Success, nullptr, "");
        }));

    FlushClient();

    EXPECT_CALL(*mockCreateComputePipelineAsyncCallback,
                Call(WGPUCreatePipelineAsyncStatus_Success, NotNull(), StrEq(""), this))
        .Times(1);
    GetWireClient()->Disconnect();
}

// Test that registering a callback after wire disconnect calls the callback with
// Success.
TEST_F(WireCreatePipelineAsyncTest, CreateRenderPipelineAsyncAfterDisconnect) {
    WGPUShaderModuleDescriptor vertexDescriptor = {};
    WGPUShaderModule vsModule = wgpuDeviceCreateShaderModule(device, &vertexDescriptor);
    WGPUShaderModule apiVsModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiVsModule));

    WGPUFragmentState fragment = {};
    fragment.module = vsModule;
    fragment.entryPoint = "main";

    WGPURenderPipelineDescriptor pipelineDescriptor{};
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.vertex.entryPoint = "main";
    pipelineDescriptor.fragment = &fragment;

    FlushClient();

    GetWireClient()->Disconnect();

    EXPECT_CALL(*mockCreateRenderPipelineAsyncCallback,
                Call(WGPUCreatePipelineAsyncStatus_Success, NotNull(), StrEq(""), this))
        .Times(1);
    wgpuDeviceCreateRenderPipelineAsync(device, &pipelineDescriptor,
                                        ToMockCreateRenderPipelineAsyncCallback, this);
}

// Test that registering a callback after wire disconnect calls the callback with
// Success.
TEST_F(WireCreatePipelineAsyncTest, CreateComputePipelineAsyncAfterDisconnect) {
    WGPUShaderModuleDescriptor csDescriptor{};
    WGPUShaderModule csModule = wgpuDeviceCreateShaderModule(device, &csDescriptor);
    WGPUShaderModule apiCsModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiCsModule));

    WGPUComputePipelineDescriptor descriptor{};
    descriptor.compute.module = csModule;
    descriptor.compute.entryPoint = "main";

    FlushClient();

    GetWireClient()->Disconnect();

    EXPECT_CALL(*mockCreateComputePipelineAsyncCallback,
                Call(WGPUCreatePipelineAsyncStatus_Success, NotNull(), StrEq(""), this))
        .Times(1);

    wgpuDeviceCreateComputePipelineAsync(device, &descriptor,
                                         ToMockCreateComputePipelineAsyncCallback, this);
}

TEST_F(WireCreatePipelineAsyncTest, DeviceDeletedBeforeCallback) {
    WGPUShaderModuleDescriptor vertexDescriptor = {};
    WGPUShaderModule module = wgpuDeviceCreateShaderModule(device, &vertexDescriptor);
    WGPUShaderModule apiModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiModule));

    WGPURenderPipelineDescriptor pipelineDescriptor{};
    pipelineDescriptor.vertex.module = module;
    pipelineDescriptor.vertex.entryPoint = "main";

    WGPUFragmentState fragment = {};
    fragment.module = module;
    fragment.entryPoint = "main";
    pipelineDescriptor.fragment = &fragment;

    wgpuDeviceCreateRenderPipelineAsync(device, &pipelineDescriptor,
                                        ToMockCreateRenderPipelineAsyncCallback, this);

    EXPECT_CALL(api, OnDeviceCreateRenderPipelineAsync(apiDevice, _, _, _));
    FlushClient();

    EXPECT_CALL(*mockCreateRenderPipelineAsyncCallback,
                Call(WGPUCreatePipelineAsyncStatus_Success, NotNull(), StrEq(""), this))
        .Times(1);

    wgpuDeviceRelease(device);

    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(apiDevice, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetLoggingCallback(apiDevice, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(apiDevice, nullptr, nullptr)).Times(1);
    EXPECT_CALL(api, DeviceRelease(apiDevice)).Times(1);

    FlushClient();
    DefaultApiDeviceWasReleased();
}

// Test that if the server is deleted before the callback, it forces the
// callback to complete.
TEST(WireCreatePipelineAsyncTestNullBackend, ServerDeletedBeforeCallback) {
    // This test sets up its own wire facilities, because unlike the other
    // tests which use mocks, this test needs the null backend and the
    // threadpool which automatically pushes async pipeline compilation
    // to completion. With mocks, we need to explicitly trigger callbacks,
    // but this test depends on triggering the async compilation from
    // *within* the wire server destructor.
    auto c2sBuf = std::make_unique<dawn::utils::TerribleCommandBuffer>();
    auto s2cBuf = std::make_unique<dawn::utils::TerribleCommandBuffer>();

    dawn::wire::WireServerDescriptor serverDesc = {};
    serverDesc.procs = &dawn::native::GetProcs();
    serverDesc.serializer = s2cBuf.get();

    auto wireServer = std::make_unique<dawn::wire::WireServer>(serverDesc);
    c2sBuf->SetHandler(wireServer.get());

    dawn::wire::WireClientDescriptor clientDesc = {};
    clientDesc.serializer = c2sBuf.get();

    auto wireClient = std::make_unique<dawn::wire::WireClient>(clientDesc);
    s2cBuf->SetHandler(wireClient.get());

    dawnProcSetProcs(&dawn::wire::client::GetProcs());

    auto reservation = wireClient->ReserveInstance();
    WGPUInstance instance = reservation.instance;
    wireServer->InjectInstance(dawn::native::GetProcs().createInstance(nullptr), reservation.id,
                               reservation.generation);

    WGPURequestAdapterOptions adapterOptions = {};
    adapterOptions.backendType = WGPUBackendType_Null;

    WGPUAdapter adapter;
    wgpuInstanceRequestAdapter(
        instance, &adapterOptions,
        [](WGPURequestAdapterStatus status, WGPUAdapter adapter, const char*, void* userdata) {
            *static_cast<WGPUAdapter*>(userdata) = adapter;
        },
        &adapter);
    ASSERT_TRUE(c2sBuf->Flush());
    ASSERT_TRUE(s2cBuf->Flush());

    WGPUDeviceDescriptor deviceDesc = {};
    WGPUDevice device;
    wgpuAdapterRequestDevice(
        adapter, &deviceDesc,
        [](WGPURequestDeviceStatus status, WGPUDevice device, const char*, void* userdata) {
            *static_cast<WGPUDevice*>(userdata) = device;
        },
        &device);
    ASSERT_TRUE(c2sBuf->Flush());
    ASSERT_TRUE(s2cBuf->Flush());

    WGPUShaderModuleWGSLDescriptor wgslDesc = {};
    wgslDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
    wgslDesc.code = "@compute @workgroup_size(64) fn main() {}";

    WGPUShaderModuleDescriptor smDesc = {};
    smDesc.nextInChain = &wgslDesc.chain;

    WGPUShaderModule sm = wgpuDeviceCreateShaderModule(device, &smDesc);

    WGPUComputePipelineDescriptor computeDesc = {};
    computeDesc.compute.module = sm;
    computeDesc.compute.entryPoint = "main";

    WGPUComputePipeline pipeline = nullptr;
    wgpuDeviceCreateComputePipelineAsync(
        device, &computeDesc,
        [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline pipeline, const char* message,
           void* userdata) { *static_cast<WGPUComputePipeline*>(userdata) = pipeline; },
        &pipeline);

    ASSERT_TRUE(c2sBuf->Flush());

    // Delete the server. It should force async work to complete.
    wireServer.reset();

    ASSERT_TRUE(s2cBuf->Flush());
    ASSERT_NE(pipeline, nullptr);

    wgpuComputePipelineRelease(pipeline);
    wgpuShaderModuleRelease(sm);
    wgpuDeviceRelease(device);
    wgpuAdapterRelease(adapter);
    wgpuInstanceRelease(instance);
}

}  // anonymous namespace
}  // namespace dawn::wire
