// Copyright 2017 The NXT Authors
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

#include "gtest/gtest.h"
#include "mock/mock_nxt.h"

#include "TerribleCommandBuffer.h"
#include "Wire.h"

using namespace testing;
using namespace nxt::wire;

class WireTests : public Test {
    protected:
        void SetUp() override {
            nxtProcTable mockProcs;
            nxtDevice mockDevice;
            api.GetProcTableAndDevice(&mockProcs, &mockDevice);

            s2cBuf = new TerribleCommandBuffer();
            c2sBuf = new TerribleCommandBuffer(wireServer);

            wireServer = NewServerCommandHandler(mockDevice, mockProcs, s2cBuf);
            c2sBuf->SetHandler(wireServer);

            nxtProcTable clientProcs;
            wireClient = NewClientDevice(&clientProcs, &device, c2sBuf);
            nxtSetProcs(&clientProcs);
            s2cBuf->SetHandler(wireClient);

            apiDevice = mockDevice;
        }

        void TearDown() override {
            nxtSetProcs(nullptr);
            delete wireServer;
            delete wireClient;
            delete c2sBuf;
            delete s2cBuf;
        }

        void FlushClient() {
            c2sBuf->Flush();
        }

        void FlushServer() {
            s2cBuf->Flush();
        }

        MockProcTable api;
        nxtDevice apiDevice;
        nxtDevice device;

    private:
        CommandHandler* wireServer = nullptr;
        CommandHandler* wireClient = nullptr;
        TerribleCommandBuffer* s2cBuf = nullptr;
        TerribleCommandBuffer* c2sBuf = nullptr;
};

// One call gets forwarded correctly.
TEST_F(WireTests, CallForwarded) {
    nxtCommandBufferBuilder builder = nxtDeviceCreateCommandBufferBuilder(device);

    nxtCommandBufferBuilder apiCmdBufBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiCmdBufBuilder));

    FlushClient();
}

// Test that calling methods on a new object works as expected.
TEST_F(WireTests, CreateThenCall) {
    nxtCommandBufferBuilder builder = nxtDeviceCreateCommandBufferBuilder(device);
    nxtCommandBuffer cmdBuf = nxtCommandBufferBuilderGetResult(builder);

    nxtCommandBufferBuilder apiCmdBufBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiCmdBufBuilder));

    nxtCommandBuffer apiCmdBuf = api.GetNewCommandBuffer();
    EXPECT_CALL(api, CommandBufferBuilderGetResult(apiCmdBufBuilder))
        .WillOnce(Return(apiCmdBuf));

    FlushClient();
}

// Test that client reference/release do not call the backend API.
TEST_F(WireTests, RefCountKeptInClient) {
    nxtCommandBufferBuilder builder = nxtDeviceCreateCommandBufferBuilder(device);

    nxtCommandBufferBuilderReference(builder);
    nxtCommandBufferBuilderRelease(builder);

    nxtCommandBufferBuilder apiCmdBufBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiCmdBufBuilder));

    FlushClient();
}

// Test that client reference/release do not call the backend API.
TEST_F(WireTests, ReleaseCalledOnRefCount0) {
    nxtCommandBufferBuilder builder = nxtDeviceCreateCommandBufferBuilder(device);

    nxtCommandBufferBuilderRelease(builder);

    nxtCommandBufferBuilder apiCmdBufBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiCmdBufBuilder));

    EXPECT_CALL(api, CommandBufferBuilderRelease(apiCmdBufBuilder));

    FlushClient();
}

TEST_F(WireTests, ObjectAsValueArgument) {
    // Create pipeline
    nxtPipelineBuilder pipelineBuilder = nxtDeviceCreatePipelineBuilder(device);
    nxtPipeline pipeline = nxtPipelineBuilderGetResult(pipelineBuilder);

    nxtPipelineBuilder apiPipelineBuilder = api.GetNewPipelineBuilder();
    EXPECT_CALL(api, DeviceCreatePipelineBuilder(apiDevice))
        .WillOnce(Return(apiPipelineBuilder));

    nxtPipeline apiPipeline = api.GetNewPipeline();
    EXPECT_CALL(api, PipelineBuilderGetResult(apiPipelineBuilder))
        .WillOnce(Return(apiPipeline));

    // Create command buffer builder, setting pipeline
    nxtCommandBufferBuilder cmdBufBuilder = nxtDeviceCreateCommandBufferBuilder(device);
    nxtCommandBufferBuilderSetPipeline(cmdBufBuilder, pipeline);

    nxtCommandBufferBuilder apiCmdBufBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiCmdBufBuilder));

    EXPECT_CALL(api, CommandBufferBuilderSetPipeline(apiCmdBufBuilder, apiPipeline));

    FlushClient();
}

TEST_F(WireTests, OneObjectAsPointerArgument) {
    // Create command buffer
    nxtCommandBufferBuilder cmdBufBuilder = nxtDeviceCreateCommandBufferBuilder(device);
    nxtCommandBuffer cmdBuf = nxtCommandBufferBuilderGetResult(cmdBufBuilder);

    nxtCommandBufferBuilder apiCmdBufBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiCmdBufBuilder));

    nxtCommandBuffer apiCmdBuf = api.GetNewCommandBuffer();
    EXPECT_CALL(api, CommandBufferBuilderGetResult(apiCmdBufBuilder))
        .WillOnce(Return(apiCmdBuf));

    // Create queue
    nxtQueueBuilder queueBuilder = nxtDeviceCreateQueueBuilder(device);
    nxtQueue queue = nxtQueueBuilderGetResult(queueBuilder);

    nxtQueueBuilder apiQueueBuilder = api.GetNewQueueBuilder();
    EXPECT_CALL(api, DeviceCreateQueueBuilder(apiDevice))
        .WillOnce(Return(apiQueueBuilder));

    nxtQueue apiQueue = api.GetNewQueue();
    EXPECT_CALL(api, QueueBuilderGetResult(apiQueueBuilder))
        .WillOnce(Return(apiQueue));

    // Submit command buffer
    nxtQueueSubmit(queue, 1, &cmdBuf);

    EXPECT_CALL(api, QueueSubmit(apiQueue, 1, Pointee(apiCmdBuf)));

    FlushClient();
}

// TODO
//  - Test values work
//  - Test multiple objects as value work
//  - Object creation, then calls do nothing after error on builder
//  - Object creation then error then create object, then should do nothing.
