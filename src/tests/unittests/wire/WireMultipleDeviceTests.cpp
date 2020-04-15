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

#include "common/Assert.h"
#include "dawn/dawn_proc.h"
#include "dawn_wire/WireClient.h"
#include "dawn_wire/WireServer.h"
#include "tests/MockCallback.h"
#include "utils/TerribleCommandBuffer.h"

#include <array>

using namespace testing;
using namespace dawn_wire;

class WireMultipleDeviceTests : public testing::Test {
  protected:
    void SetUp() override {
        DawnProcTable procs = dawn_wire::WireClient::GetProcs();
        dawnProcSetProcs(&procs);
    }

    void TearDown() override {
        dawnProcSetProcs(nullptr);
    }

    class WireHolder {
      public:
        WireHolder() {
            DawnProcTable mockProcs;
            mApi.GetProcTableAndDevice(&mockProcs, &mServerDevice);

            // Ignore Tick()
            EXPECT_CALL(mApi, DeviceTick(_)).Times(AnyNumber());

            // This SetCallback call cannot be ignored because it is done as soon as we start the
            // server
            EXPECT_CALL(mApi, OnDeviceSetUncapturedErrorCallback(_, _, _)).Times(Exactly(1));
            EXPECT_CALL(mApi, OnDeviceSetDeviceLostCallback(_, _, _)).Times(Exactly(1));

            mS2cBuf = std::make_unique<utils::TerribleCommandBuffer>();
            mC2sBuf = std::make_unique<utils::TerribleCommandBuffer>();

            WireServerDescriptor serverDesc = {};
            serverDesc.device = mServerDevice;
            serverDesc.procs = &mockProcs;
            serverDesc.serializer = mS2cBuf.get();

            mWireServer.reset(new WireServer(serverDesc));
            mC2sBuf->SetHandler(mWireServer.get());

            WireClientDescriptor clientDesc = {};
            clientDesc.serializer = mC2sBuf.get();

            mWireClient.reset(new WireClient(clientDesc));
            mS2cBuf->SetHandler(mWireClient.get());

            mClientDevice = mWireClient->GetDevice();
        }

        ~WireHolder() {
            mApi.IgnoreAllReleaseCalls();
            mWireClient = nullptr;
            mWireServer = nullptr;
        }

        void FlushClient(bool success = true) {
            ASSERT_EQ(mC2sBuf->Flush(), success);
        }

        void FlushServer(bool success = true) {
            ASSERT_EQ(mS2cBuf->Flush(), success);
        }

        testing::StrictMock<MockProcTable>* Api() {
            return &mApi;
        }

        WGPUDevice ClientDevice() {
            return mClientDevice;
        }

        WGPUDevice ServerDevice() {
            return mServerDevice;
        }

      private:
        testing::StrictMock<MockProcTable> mApi;
        std::unique_ptr<dawn_wire::WireServer> mWireServer;
        std::unique_ptr<dawn_wire::WireClient> mWireClient;
        std::unique_ptr<utils::TerribleCommandBuffer> mS2cBuf;
        std::unique_ptr<utils::TerribleCommandBuffer> mC2sBuf;
        WGPUDevice mServerDevice;
        WGPUDevice mClientDevice;
    };

    void ExpectInjectedError(WireHolder* wire) {
        std::string errorMessage;
        EXPECT_CALL(*wire->Api(),
                    DeviceInjectError(wire->ServerDevice(), WGPUErrorType_Validation, _))
            .WillOnce(Invoke([&](WGPUDevice device, WGPUErrorType type, const char* message) {
                errorMessage = message;
                // Mock the call to the error callback.
                wire->Api()->CallDeviceErrorCallback(device, type, message);
            }));
        wire->FlushClient();

        // The error callback should be forwarded to the client.
        StrictMock<MockCallback<WGPUErrorCallback>> mockErrorCallback;
        wgpuDeviceSetUncapturedErrorCallback(wire->ClientDevice(), mockErrorCallback.Callback(),
                                             mockErrorCallback.MakeUserdata(this));

        EXPECT_CALL(mockErrorCallback, Call(WGPUErrorType_Validation, StrEq(errorMessage), this))
            .Times(1);
        wire->FlushServer();
    }
};

// Test that using objects from a different device is a validation error.
TEST_F(WireMultipleDeviceTests, ValidatesSameDevice) {
    WireHolder wireA;
    WireHolder wireB;

    // Create the objects
    WGPUQueue queueA = wgpuDeviceCreateQueue(wireA.ClientDevice());
    WGPUQueue queueB = wgpuDeviceCreateQueue(wireB.ClientDevice());

    WGPUFenceDescriptor desc = {};
    WGPUFence fenceA = wgpuQueueCreateFence(queueA, &desc);

    // Flush on wire B. We should see the queue created.
    EXPECT_CALL(*wireB.Api(), DeviceCreateQueue(wireB.ServerDevice()))
        .WillOnce(Return(wireB.Api()->GetNewQueue()));
    wireB.FlushClient();

    // Signal with a fence from a different wire.
    wgpuQueueSignal(queueB, fenceA, 1u);

    // We should inject an error into the server.
    ExpectInjectedError(&wireB);
}

// Test that objects created from mixed devices are an error to use.
TEST_F(WireMultipleDeviceTests, DifferentDeviceObjectCreationIsError) {
    WireHolder wireA;
    WireHolder wireB;

    // Create a bind group layout on wire A.
    WGPUBindGroupLayoutDescriptor bglDesc = {};
    WGPUBindGroupLayout bglA = wgpuDeviceCreateBindGroupLayout(wireA.ClientDevice(), &bglDesc);
    EXPECT_CALL(*wireA.Api(), DeviceCreateBindGroupLayout(wireA.ServerDevice(), _))
        .WillOnce(Return(wireA.Api()->GetNewBindGroupLayout()));

    wireA.FlushClient();

    std::array<WGPUBindGroupBinding, 2> bindings = {};

    // Create a buffer on wire A.
    WGPUBufferDescriptor bufferDesc = {};
    bindings[0].buffer = wgpuDeviceCreateBuffer(wireA.ClientDevice(), &bufferDesc);
    EXPECT_CALL(*wireA.Api(), DeviceCreateBuffer(wireA.ServerDevice(), _))
        .WillOnce(Return(wireA.Api()->GetNewBuffer()));

    wireA.FlushClient();

    // Create a sampler on wire B.
    WGPUSamplerDescriptor samplerDesc = {};
    bindings[1].sampler = wgpuDeviceCreateSampler(wireB.ClientDevice(), &samplerDesc);
    EXPECT_CALL(*wireB.Api(), DeviceCreateSampler(wireB.ServerDevice(), _))
        .WillOnce(Return(wireB.Api()->GetNewSampler()));

    wireB.FlushClient();

    // Create a bind group on wire A using the bgl (A), buffer (A), and sampler (B).
    WGPUBindGroupDescriptor bgDesc = {};
    bgDesc.layout = bglA;
    bgDesc.bindingCount = bindings.size();
    bgDesc.bindings = bindings.data();
    WGPUBindGroup bindGroupA = wgpuDeviceCreateBindGroup(wireA.ClientDevice(), &bgDesc);

    // It should inject an error because the sampler is from a different device.
    ExpectInjectedError(&wireA);

    // The bind group was never created on a server because it failed device validation.
    // Any commands that use it should error.
    wgpuBindGroupRelease(bindGroupA);
    wireA.FlushClient(false);
}

// Test that using objects, included in an extension struct,
// from a difference device is a validation error.
TEST_F(WireMultipleDeviceTests, ValidatesSameDeviceInExtensionStruct) {
    WireHolder wireA;
    WireHolder wireB;

    WGPUShaderModuleDescriptor shaderModuleDesc = {};
    WGPUShaderModule shaderModuleA =
        wgpuDeviceCreateShaderModule(wireA.ClientDevice(), &shaderModuleDesc);

    // Flush on wire A. We should see the shader module created.
    EXPECT_CALL(*wireA.Api(), DeviceCreateShaderModule(wireA.ServerDevice(), _))
        .WillOnce(Return(wireA.Api()->GetNewShaderModule()));
    wireA.FlushClient();

    WGPURenderPipelineDescriptorDummyExtension extDesc = {};
    extDesc.chain.sType = WGPUSType_RenderPipelineDescriptorDummyExtension;
    extDesc.dummyStage.entryPoint = "main";
    extDesc.dummyStage.module = shaderModuleA;

    WGPURenderPipelineDescriptor pipelineDesc = {};
    pipelineDesc.nextInChain = &extDesc.chain;
    WGPURenderPipeline pipelineB =
        wgpuDeviceCreateRenderPipeline(wireB.ClientDevice(), &pipelineDesc);

    // We should inject an error into the server.
    ExpectInjectedError(&wireB);

    // The pipeline was never created on a server because it failed device validation.
    // Any commands that use it should error.
    wgpuRenderPipelineRelease(pipelineB);
    wireB.FlushClient(false);
}

// Test that using objects, included in a chained extension struct,
// from a different device is a validation error.
TEST_F(WireMultipleDeviceTests, ValidatesSameDeviceSecondInExtensionStructChain) {
    WireHolder wireA;
    WireHolder wireB;

    WGPUShaderModuleDescriptor shaderModuleDesc = {};
    WGPUShaderModule shaderModuleA =
        wgpuDeviceCreateShaderModule(wireA.ClientDevice(), &shaderModuleDesc);

    // Flush on wire A. We should see the shader module created.
    EXPECT_CALL(*wireA.Api(), DeviceCreateShaderModule(wireA.ServerDevice(), _))
        .WillOnce(Return(wireA.Api()->GetNewShaderModule()));
    wireA.FlushClient();

    WGPUShaderModule shaderModuleB =
        wgpuDeviceCreateShaderModule(wireB.ClientDevice(), &shaderModuleDesc);

    // Flush on wire B. We should see the shader module created.
    EXPECT_CALL(*wireB.Api(), DeviceCreateShaderModule(wireB.ServerDevice(), _))
        .WillOnce(Return(wireB.Api()->GetNewShaderModule()));
    wireB.FlushClient();

    WGPURenderPipelineDescriptorDummyExtension extDescA = {};
    extDescA.chain.sType = WGPUSType_RenderPipelineDescriptorDummyExtension;
    extDescA.dummyStage.entryPoint = "main";
    extDescA.dummyStage.module = shaderModuleA;

    WGPURenderPipelineDescriptorDummyExtension extDescB = {};
    extDescB.chain.sType = WGPUSType_RenderPipelineDescriptorDummyExtension;
    extDescB.chain.next = &extDescA.chain;
    extDescB.dummyStage.entryPoint = "main";
    extDescB.dummyStage.module = shaderModuleB;

    // The first extension struct is from Device B, and the second is from A.
    // We should validate the second struct, is from the same device.
    WGPURenderPipelineDescriptor pipelineDesc = {};
    pipelineDesc.nextInChain = &extDescB.chain;
    WGPURenderPipeline pipelineB =
        wgpuDeviceCreateRenderPipeline(wireB.ClientDevice(), &pipelineDesc);

    // We should inject an error into the server.
    ExpectInjectedError(&wireB);

    // The pipeline was never created on a server because it failed device validation.
    // Any commands that use it should error.
    wgpuRenderPipelineRelease(pipelineB);
    wireB.FlushClient(false);
}
