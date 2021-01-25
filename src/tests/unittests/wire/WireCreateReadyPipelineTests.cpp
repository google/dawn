// Copyright 2020 The Dawn Authors
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

#include "tests/unittests/wire/WireTest.h"

#include "dawn_wire/WireClient.h"

using namespace testing;
using namespace dawn_wire;

namespace {

    // Mock class to add expectations on the wire calling callbacks
    class MockCreateReadyComputePipelineCallback {
      public:
        MOCK_METHOD(void,
                    Call,
                    (WGPUCreateReadyPipelineStatus status,
                     WGPUComputePipeline pipeline,
                     const char* message,
                     void* userdata));
    };

    std::unique_ptr<StrictMock<MockCreateReadyComputePipelineCallback>>
        mockCreateReadyComputePipelineCallback;
    void ToMockCreateReadyComputePipelineCallback(WGPUCreateReadyPipelineStatus status,
                                                  WGPUComputePipeline pipeline,
                                                  const char* message,
                                                  void* userdata) {
        mockCreateReadyComputePipelineCallback->Call(status, pipeline, message, userdata);
    }

    class MockCreateReadyRenderPipelineCallback {
      public:
        MOCK_METHOD(void,
                    Call,
                    (WGPUCreateReadyPipelineStatus status,
                     WGPURenderPipeline pipeline,
                     const char* message,
                     void* userdata));
    };

    std::unique_ptr<StrictMock<MockCreateReadyRenderPipelineCallback>>
        mockCreateReadyRenderPipelineCallback;
    void ToMockCreateReadyRenderPipelineCallback(WGPUCreateReadyPipelineStatus status,
                                                 WGPURenderPipeline pipeline,
                                                 const char* message,
                                                 void* userdata) {
        mockCreateReadyRenderPipelineCallback->Call(status, pipeline, message, userdata);
    }

}  // anonymous namespace

class WireCreateReadyPipelineTest : public WireTest {
  public:
    void SetUp() override {
        WireTest::SetUp();

        mockCreateReadyComputePipelineCallback =
            std::make_unique<StrictMock<MockCreateReadyComputePipelineCallback>>();
        mockCreateReadyRenderPipelineCallback =
            std::make_unique<StrictMock<MockCreateReadyRenderPipelineCallback>>();
    }

    void TearDown() override {
        WireTest::TearDown();

        // Delete mock so that expectations are checked
        mockCreateReadyComputePipelineCallback = nullptr;
        mockCreateReadyRenderPipelineCallback = nullptr;
    }

    void FlushClient() {
        WireTest::FlushClient();
        Mock::VerifyAndClearExpectations(&mockCreateReadyComputePipelineCallback);
    }

    void FlushServer() {
        WireTest::FlushServer();
        Mock::VerifyAndClearExpectations(&mockCreateReadyComputePipelineCallback);
    }
};

// Test when creating a compute pipeline with CreateReadyComputePipeline() successfully.
TEST_F(WireCreateReadyPipelineTest, CreateReadyComputePipelineSuccess) {
    WGPUShaderModuleDescriptor csDescriptor{};
    WGPUShaderModule csModule = wgpuDeviceCreateShaderModule(device, &csDescriptor);
    WGPUShaderModule apiCsModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiCsModule));

    WGPUComputePipelineDescriptor descriptor{};
    descriptor.computeStage.module = csModule;
    descriptor.computeStage.entryPoint = "main";

    wgpuDeviceCreateReadyComputePipeline(device, &descriptor,
                                         ToMockCreateReadyComputePipelineCallback, this);

    EXPECT_CALL(api, OnDeviceCreateReadyComputePipeline(apiDevice, _, _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallDeviceCreateReadyComputePipelineCallback(
                apiDevice, WGPUCreateReadyPipelineStatus_Success, nullptr, "");
        }));

    FlushClient();

    EXPECT_CALL(*mockCreateReadyComputePipelineCallback,
                Call(WGPUCreateReadyPipelineStatus_Success, _, StrEq(""), this))
        .Times(1);

    FlushServer();
}

// Test when creating a compute pipeline with CreateReadyComputePipeline() results in an error.
TEST_F(WireCreateReadyPipelineTest, CreateReadyComputePipelineError) {
    WGPUShaderModuleDescriptor csDescriptor{};
    WGPUShaderModule csModule = wgpuDeviceCreateShaderModule(device, &csDescriptor);
    WGPUShaderModule apiCsModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiCsModule));

    WGPUComputePipelineDescriptor descriptor{};
    descriptor.computeStage.module = csModule;
    descriptor.computeStage.entryPoint = "main";

    wgpuDeviceCreateReadyComputePipeline(device, &descriptor,
                                         ToMockCreateReadyComputePipelineCallback, this);

    EXPECT_CALL(api, OnDeviceCreateReadyComputePipeline(apiDevice, _, _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallDeviceCreateReadyComputePipelineCallback(
                apiDevice, WGPUCreateReadyPipelineStatus_Error, nullptr, "Some error message");
        }));

    FlushClient();

    EXPECT_CALL(*mockCreateReadyComputePipelineCallback,
                Call(WGPUCreateReadyPipelineStatus_Error, _, StrEq("Some error message"), this))
        .Times(1);

    FlushServer();
}

// Test when creating a render pipeline with CreateReadyRenderPipeline() successfully.
TEST_F(WireCreateReadyPipelineTest, CreateReadyRenderPipelineSuccess) {
    WGPUShaderModuleDescriptor vertexDescriptor = {};
    WGPUShaderModule vsModule = wgpuDeviceCreateShaderModule(device, &vertexDescriptor);
    WGPUShaderModule apiVsModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiVsModule));

    WGPURenderPipelineDescriptor pipelineDescriptor{};
    pipelineDescriptor.vertexStage.module = vsModule;
    pipelineDescriptor.vertexStage.entryPoint = "main";

    WGPUProgrammableStageDescriptor fragmentStage = {};
    fragmentStage.module = vsModule;
    fragmentStage.entryPoint = "main";
    pipelineDescriptor.fragmentStage = &fragmentStage;

    wgpuDeviceCreateReadyRenderPipeline(device, &pipelineDescriptor,
                                        ToMockCreateReadyRenderPipelineCallback, this);
    EXPECT_CALL(api, OnDeviceCreateReadyRenderPipeline(apiDevice, _, _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallDeviceCreateReadyRenderPipelineCallback(
                apiDevice, WGPUCreateReadyPipelineStatus_Success, nullptr, "");
        }));

    FlushClient();

    EXPECT_CALL(*mockCreateReadyRenderPipelineCallback,
                Call(WGPUCreateReadyPipelineStatus_Success, _, StrEq(""), this))
        .Times(1);

    FlushServer();
}

// Test when creating a render pipeline with CreateReadyRenderPipeline() results in an error.
TEST_F(WireCreateReadyPipelineTest, CreateReadyRenderPipelineError) {
    WGPUShaderModuleDescriptor vertexDescriptor = {};
    WGPUShaderModule vsModule = wgpuDeviceCreateShaderModule(device, &vertexDescriptor);
    WGPUShaderModule apiVsModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiVsModule));

    WGPURenderPipelineDescriptor pipelineDescriptor{};
    pipelineDescriptor.vertexStage.module = vsModule;
    pipelineDescriptor.vertexStage.entryPoint = "main";

    WGPUProgrammableStageDescriptor fragmentStage = {};
    fragmentStage.module = vsModule;
    fragmentStage.entryPoint = "main";
    pipelineDescriptor.fragmentStage = &fragmentStage;

    wgpuDeviceCreateReadyRenderPipeline(device, &pipelineDescriptor,
                                        ToMockCreateReadyRenderPipelineCallback, this);
    EXPECT_CALL(api, OnDeviceCreateReadyRenderPipeline(apiDevice, _, _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallDeviceCreateReadyRenderPipelineCallback(
                apiDevice, WGPUCreateReadyPipelineStatus_Error, nullptr, "Some error message");
        }));

    FlushClient();

    EXPECT_CALL(*mockCreateReadyRenderPipelineCallback,
                Call(WGPUCreateReadyPipelineStatus_Error, _, StrEq("Some error message"), this))
        .Times(1);

    FlushServer();
}

// Test that registering a callback then wire disconnect calls the callback with
// DeviceLost.
TEST_F(WireCreateReadyPipelineTest, CreateReadyRenderPipelineThenDisconnect) {
    WGPUShaderModuleDescriptor vertexDescriptor = {};
    WGPUShaderModule vsModule = wgpuDeviceCreateShaderModule(device, &vertexDescriptor);
    WGPUShaderModule apiVsModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiVsModule));

    WGPUProgrammableStageDescriptor fragmentStage = {};
    fragmentStage.module = vsModule;
    fragmentStage.entryPoint = "main";

    WGPURenderPipelineDescriptor pipelineDescriptor{};
    pipelineDescriptor.vertexStage.module = vsModule;
    pipelineDescriptor.vertexStage.entryPoint = "main";
    pipelineDescriptor.fragmentStage = &fragmentStage;

    wgpuDeviceCreateReadyRenderPipeline(device, &pipelineDescriptor,
                                        ToMockCreateReadyRenderPipelineCallback, this);
    EXPECT_CALL(api, OnDeviceCreateReadyRenderPipeline(apiDevice, _, _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallDeviceCreateReadyRenderPipelineCallback(
                apiDevice, WGPUCreateReadyPipelineStatus_Success, nullptr, "");
        }));

    FlushClient();

    EXPECT_CALL(*mockCreateReadyRenderPipelineCallback,
                Call(WGPUCreateReadyPipelineStatus_DeviceLost, _, _, this))
        .Times(1);
    GetWireClient()->Disconnect();
}

// Test that registering a callback then wire disconnect calls the callback with
// DeviceLost.
TEST_F(WireCreateReadyPipelineTest, CreateReadyComputePipelineThenDisconnect) {
    WGPUShaderModuleDescriptor csDescriptor{};
    WGPUShaderModule csModule = wgpuDeviceCreateShaderModule(device, &csDescriptor);
    WGPUShaderModule apiCsModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiCsModule));

    WGPUComputePipelineDescriptor descriptor{};
    descriptor.computeStage.module = csModule;
    descriptor.computeStage.entryPoint = "main";

    wgpuDeviceCreateReadyComputePipeline(device, &descriptor,
                                         ToMockCreateReadyComputePipelineCallback, this);
    EXPECT_CALL(api, OnDeviceCreateReadyComputePipeline(apiDevice, _, _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallDeviceCreateReadyComputePipelineCallback(
                apiDevice, WGPUCreateReadyPipelineStatus_Success, nullptr, "");
        }));

    FlushClient();

    EXPECT_CALL(*mockCreateReadyComputePipelineCallback,
                Call(WGPUCreateReadyPipelineStatus_DeviceLost, _, _, this))
        .Times(1);
    GetWireClient()->Disconnect();
}

// Test that registering a callback after wire disconnect calls the callback with
// DeviceLost.
TEST_F(WireCreateReadyPipelineTest, CreateReadyRenderPipelineAfterDisconnect) {
    WGPUShaderModuleDescriptor vertexDescriptor = {};
    WGPUShaderModule vsModule = wgpuDeviceCreateShaderModule(device, &vertexDescriptor);
    WGPUShaderModule apiVsModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiVsModule));

    WGPUProgrammableStageDescriptor fragmentStage = {};
    fragmentStage.module = vsModule;
    fragmentStage.entryPoint = "main";

    WGPURenderPipelineDescriptor pipelineDescriptor{};
    pipelineDescriptor.vertexStage.module = vsModule;
    pipelineDescriptor.vertexStage.entryPoint = "main";
    pipelineDescriptor.fragmentStage = &fragmentStage;

    FlushClient();

    GetWireClient()->Disconnect();

    EXPECT_CALL(*mockCreateReadyRenderPipelineCallback,
                Call(WGPUCreateReadyPipelineStatus_DeviceLost, nullptr, _, this))
        .Times(1);
    wgpuDeviceCreateReadyRenderPipeline(device, &pipelineDescriptor,
                                        ToMockCreateReadyRenderPipelineCallback, this);
}

// Test that registering a callback after wire disconnect calls the callback with
// DeviceLost.
TEST_F(WireCreateReadyPipelineTest, CreateReadyComputePipelineAfterDisconnect) {
    WGPUShaderModuleDescriptor csDescriptor{};
    WGPUShaderModule csModule = wgpuDeviceCreateShaderModule(device, &csDescriptor);
    WGPUShaderModule apiCsModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiCsModule));

    WGPUComputePipelineDescriptor descriptor{};
    descriptor.computeStage.module = csModule;
    descriptor.computeStage.entryPoint = "main";

    FlushClient();

    GetWireClient()->Disconnect();

    EXPECT_CALL(*mockCreateReadyComputePipelineCallback,
                Call(WGPUCreateReadyPipelineStatus_DeviceLost, nullptr, _, this))
        .Times(1);

    wgpuDeviceCreateReadyComputePipeline(device, &descriptor,
                                         ToMockCreateReadyComputePipelineCallback, this);
}

TEST_F(WireCreateReadyPipelineTest, DeviceDeletedBeforeCallback) {
    WGPUShaderModuleDescriptor vertexDescriptor = {};
    WGPUShaderModule module = wgpuDeviceCreateShaderModule(device, &vertexDescriptor);
    WGPUShaderModule apiModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiModule));

    WGPURenderPipelineDescriptor pipelineDescriptor{};
    pipelineDescriptor.vertexStage.module = module;
    pipelineDescriptor.vertexStage.entryPoint = "main";

    WGPUProgrammableStageDescriptor fragmentStage = {};
    fragmentStage.module = module;
    fragmentStage.entryPoint = "main";
    pipelineDescriptor.fragmentStage = &fragmentStage;

    wgpuDeviceCreateReadyRenderPipeline(device, &pipelineDescriptor,
                                        ToMockCreateReadyRenderPipelineCallback, this);

    EXPECT_CALL(api, OnDeviceCreateReadyRenderPipeline(apiDevice, _, _, _));
    FlushClient();

    EXPECT_CALL(*mockCreateReadyRenderPipelineCallback,
                Call(WGPUCreateReadyPipelineStatus_DeviceDestroyed, nullptr, _, this))
        .Times(1);

    wgpuDeviceRelease(device);

    // Expect release on all objects created by the client.
    Sequence s1, s2;
    EXPECT_CALL(api, QueueRelease(apiQueue)).Times(1).InSequence(s1);
    EXPECT_CALL(api, ShaderModuleRelease(apiModule)).Times(1).InSequence(s2);
    EXPECT_CALL(api, OnDeviceSetUncapturedErrorCallback(apiDevice, nullptr, nullptr))
        .Times(1)
        .InSequence(s1, s2);
    EXPECT_CALL(api, OnDeviceSetDeviceLostCallback(apiDevice, nullptr, nullptr))
        .Times(1)
        .InSequence(s1, s2);
    EXPECT_CALL(api, DeviceRelease(apiDevice)).Times(1).InSequence(s1, s2);

    FlushClient();
    DefaultApiDeviceWasReleased();
}
