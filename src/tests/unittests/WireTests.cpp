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

#include "wire/TerribleCommandBuffer.h"
#include "wire/Wire.h"

#include <iostream>

using namespace testing;
using namespace nxt::wire;

class WireTestsBase : public Test {
    protected:
        WireTestsBase(bool ignoreSetCallbackCalls)
            : ignoreSetCallbackCalls(ignoreSetCallbackCalls) {
        }

        void SetUp() override {
            nxtProcTable mockProcs;
            nxtDevice mockDevice;
            api.GetProcTableAndDevice(&mockProcs, &mockDevice);

            if (ignoreSetCallbackCalls) {
                EXPECT_CALL(api, OnDeviceSetErrorCallback(_, _, _)).Times(Exactly(1));
                EXPECT_CALL(api, OnBuilderSetErrorCallback(_, _, _, _)).Times(AnyNumber());
            }

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
        bool ignoreSetCallbackCalls = false;

        CommandHandler* wireServer = nullptr;
        CommandHandler* wireClient = nullptr;
        TerribleCommandBuffer* s2cBuf = nullptr;
        TerribleCommandBuffer* c2sBuf = nullptr;
};

class WireTests : public WireTestsBase {
    public:
        WireTests() : WireTestsBase(true) {
        }
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

// Test that the wire is able to send numerical values
TEST_F(WireTests, ValueArgument) {
    nxtSamplerBuilder builder = nxtDeviceCreateSamplerBuilder(device);
    nxtSamplerBuilderSetFilterMode(builder, NXT_FILTER_MODE_LINEAR, NXT_FILTER_MODE_LINEAR, NXT_FILTER_MODE_NEAREST);

    nxtSamplerBuilder apiBuilder = api.GetNewSamplerBuilder();
    EXPECT_CALL(api, DeviceCreateSamplerBuilder(apiDevice))
        .WillOnce(Return(apiBuilder));

    EXPECT_CALL(api, SamplerBuilderSetFilterMode(apiBuilder, NXT_FILTER_MODE_LINEAR, NXT_FILTER_MODE_LINEAR, NXT_FILTER_MODE_NEAREST))
        .Times(1);

    FlushClient();
}

// Test that the wire is able to send arrays of numerical values
static constexpr uint32_t testPushConstantValues[4] = {
    0,
    42,
    0xDEADBEEFu,
    0xFFFFFFFFu
};

bool CheckPushConstantValues(const uint32_t* values) {
    for (int i = 0; i < 4; ++i) {
        if (values[i] != testPushConstantValues[i]) {
            return false;
        }
    }
    return true;
}

TEST_F(WireTests, ValueArrayArgument) {
    nxtCommandBufferBuilder builder = nxtDeviceCreateCommandBufferBuilder(device);
    nxtCommandBufferBuilderSetPushConstants(builder, NXT_SHADER_STAGE_BIT_VERTEX, 0, 4, testPushConstantValues);

    nxtCommandBufferBuilder apiBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiBuilder));

    EXPECT_CALL(api, CommandBufferBuilderSetPushConstants(apiBuilder, NXT_SHADER_STAGE_BIT_VERTEX, 0, 4, ResultOf(CheckPushConstantValues, Eq(true))));

    FlushClient();
}

// Test that the wire is able to send C strings
TEST_F(WireTests, CStringArgument) {
    // Create shader module
    nxtShaderModuleBuilder shaderModuleBuilder = nxtDeviceCreateShaderModuleBuilder(device);
    nxtShaderModule shaderModule = nxtShaderModuleBuilderGetResult(shaderModuleBuilder);

    nxtShaderModuleBuilder apiShaderModuleBuilder = api.GetNewShaderModuleBuilder();
    EXPECT_CALL(api, DeviceCreateShaderModuleBuilder(apiDevice))
        .WillOnce(Return(apiShaderModuleBuilder));

    nxtShaderModule apiShaderModule = api.GetNewShaderModule();
    EXPECT_CALL(api, ShaderModuleBuilderGetResult(apiShaderModuleBuilder))
        .WillOnce(Return(apiShaderModule));

    // Create pipeline
    nxtPipelineBuilder pipelineBuilder = nxtDeviceCreatePipelineBuilder(device);
    nxtPipelineBuilderSetStage(pipelineBuilder, NXT_SHADER_STAGE_FRAGMENT, shaderModule, "my entry point");

    nxtPipelineBuilder apiPipelineBuilder = api.GetNewPipelineBuilder();
    EXPECT_CALL(api, DeviceCreatePipelineBuilder(apiDevice))
        .WillOnce(Return(apiPipelineBuilder));

    EXPECT_CALL(api, PipelineBuilderSetStage(apiPipelineBuilder, NXT_SHADER_STAGE_FRAGMENT, apiShaderModule, StrEq("my entry point")));

    FlushClient();
}

// Test that the wire is able to send objects as value arguments
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

// GMock doesn't support lambdas in ResultOf, so we make a functor instead.
struct AreAPICmdBufs {
    using result_type = bool;
    using argument_type = const nxtCommandBuffer*;
    bool operator() (const nxtCommandBuffer* cmdBufs) const {
        return cmdBufs[0] == apiCmdBufs[0] && cmdBufs[1] == apiCmdBufs[1];
    }
    nxtCommandBuffer apiCmdBufs[2];
};

// Test that the wire is able to send array of objects
TEST_F(WireTests, ObjectsAsPointerArgument) {
    nxtCommandBuffer cmdBufs[2];
    nxtCommandBuffer apiCmdBufs[2];

    // Create two command buffers we need to use a GMock sequence otherwise the order of the
    // CreateCommandBufferBuilder might be swapped since they are equivalent in term of matchers
    Sequence s;
    for (int i = 0; i < 2; ++i) {
        nxtCommandBufferBuilder cmdBufBuilder = nxtDeviceCreateCommandBufferBuilder(device);
        cmdBufs[i] = nxtCommandBufferBuilderGetResult(cmdBufBuilder);

        nxtCommandBufferBuilder apiCmdBufBuilder = api.GetNewCommandBufferBuilder();
        EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
            .InSequence(s)
            .WillOnce(Return(apiCmdBufBuilder));

        apiCmdBufs[i] = api.GetNewCommandBuffer();
        EXPECT_CALL(api, CommandBufferBuilderGetResult(apiCmdBufBuilder))
            .WillOnce(Return(apiCmdBufs[i]));
    }

    // Create queue
    nxtQueueBuilder queueBuilder = nxtDeviceCreateQueueBuilder(device);
    nxtQueue queue = nxtQueueBuilderGetResult(queueBuilder);

    nxtQueueBuilder apiQueueBuilder = api.GetNewQueueBuilder();
    EXPECT_CALL(api, DeviceCreateQueueBuilder(apiDevice))
        .WillOnce(Return(apiQueueBuilder));

    nxtQueue apiQueue = api.GetNewQueue();
    EXPECT_CALL(api, QueueBuilderGetResult(apiQueueBuilder))
        .WillOnce(Return(apiQueue));

    // Submit command buffer and check we got a call with both API-side command buffers
    nxtQueueSubmit(queue, 2, cmdBufs);

    AreAPICmdBufs predicate;
    predicate.apiCmdBufs[0] = apiCmdBufs[0];
    predicate.apiCmdBufs[1] = apiCmdBufs[1];

    EXPECT_CALL(api, QueueSubmit(apiQueue, 2, ResultOf(predicate, Eq(true))));

    FlushClient();
}

// TODO
//  - Object creation, then calls do nothing after error on builder
//  - Object creation then error then create object, then should do nothing.
//  - Device error gets forwarded properly
//  - Builder error
//    - An error gets forwarded properly
//    - No other call to builder after error
//    - No call to object after error
//    - No error -> success
//    - Builder destroyed on client side -> gets unknown
//    - Same for getresult then destroyed object
