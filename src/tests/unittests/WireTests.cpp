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

using namespace testing;
using namespace nxt::wire;

class MockDeviceErrorCallback {
    public:
        MOCK_METHOD2(Call, void(const char* message, nxtCallbackUserdata userdata));
};

static MockDeviceErrorCallback* mockDeviceErrorCallback = nullptr;
static void ToMockDeviceErrorCallback(const char* message, nxtCallbackUserdata userdata) {
    mockDeviceErrorCallback->Call(message, userdata);
}

class MockBuilderErrorCallback {
    public:
        MOCK_METHOD4(Call, void(nxtBuilderErrorStatus status, const char* message, nxtCallbackUserdata userdata1, nxtCallbackUserdata userdata2));
};

static MockBuilderErrorCallback* mockBuilderErrorCallback = nullptr;
static void ToMockBuilderErrorCallback(nxtBuilderErrorStatus status, const char* message, nxtCallbackUserdata userdata1, nxtCallbackUserdata userdata2) {
    mockBuilderErrorCallback->Call(status, message, userdata1, userdata2);
}

class MockBufferMapReadCallback {
    public:
        MOCK_METHOD3(Call, void(nxtBufferMapReadStatus status, const uint32_t* ptr, nxtCallbackUserdata userdata));
};

static MockBufferMapReadCallback* mockBufferMapReadCallback = nullptr;
static void ToMockBufferMapReadCallback(nxtBufferMapReadStatus status, const void* ptr, nxtCallbackUserdata userdata) {
    // Assume the data is uint32_t to make writing matchers easier
    mockBufferMapReadCallback->Call(status, reinterpret_cast<const uint32_t*>(ptr), userdata);
}

class WireTestsBase : public Test {
    protected:
        WireTestsBase(bool ignoreSetCallbackCalls)
            : ignoreSetCallbackCalls(ignoreSetCallbackCalls) {
        }

        void SetUp() override {
            mockDeviceErrorCallback = new MockDeviceErrorCallback;
            mockBuilderErrorCallback = new MockBuilderErrorCallback;
            mockBufferMapReadCallback = new MockBufferMapReadCallback;

            nxtProcTable mockProcs;
            nxtDevice mockDevice;
            api.GetProcTableAndDevice(&mockProcs, &mockDevice);

            // This SetCallback call cannot be ignored because it is done as soon as we start the server
            EXPECT_CALL(api, OnDeviceSetErrorCallback(_, _, _)).Times(Exactly(1));
            if (ignoreSetCallbackCalls) {
                EXPECT_CALL(api, OnBuilderSetErrorCallback(_, _, _, _)).Times(AnyNumber());
            }
            EXPECT_CALL(api, DeviceTick(_)).Times(AnyNumber());

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
            delete mockDeviceErrorCallback;
            delete mockBuilderErrorCallback;
            delete mockBufferMapReadCallback;
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

// Test that the server doesn't forward calls to error objects or with error objects
// Also test that when GetResult is called on an error builder, the error callback is fired
TEST_F(WireTests, CallsSkippedAfterBuilderError) {
    nxtCommandBufferBuilder cmdBufBuilder = nxtDeviceCreateCommandBufferBuilder(device);
    nxtCommandBufferBuilderSetErrorCallback(cmdBufBuilder, ToMockBuilderErrorCallback, 1, 2);

    nxtBufferBuilder bufferBuilder = nxtDeviceCreateBufferBuilder(device);
    nxtBufferBuilderSetErrorCallback(bufferBuilder, ToMockBuilderErrorCallback, 3, 4);
    nxtBuffer buffer = nxtBufferBuilderGetResult(bufferBuilder); // Hey look an error!

    // These calls will be skipped because of the error
    nxtBufferTransitionUsage(buffer, NXT_BUFFER_USAGE_BIT_UNIFORM);
    nxtCommandBufferBuilderTransitionBufferUsage(cmdBufBuilder, buffer, NXT_BUFFER_USAGE_BIT_UNIFORM);
    nxtCommandBuffer cmdBuf = nxtCommandBufferBuilderGetResult(cmdBufBuilder);

    nxtCommandBufferBuilder apiCmdBufBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiCmdBufBuilder));

    nxtBufferBuilder apiBufferBuilder = api.GetNewBufferBuilder();
    EXPECT_CALL(api, DeviceCreateBufferBuilder(apiDevice))
        .WillOnce(Return(apiBufferBuilder));

    // Hey look an error!
    EXPECT_CALL(api, BufferBuilderGetResult(apiBufferBuilder))
        .WillOnce(InvokeWithoutArgs([&]() -> nxtBuffer {
            api.CallBuilderErrorCallback(apiBufferBuilder, NXT_BUILDER_ERROR_STATUS_ERROR, "Error");
            return nullptr;
        }));

    EXPECT_CALL(api, BufferTransitionUsage(_, _)).Times(0);
    EXPECT_CALL(api, CommandBufferBuilderTransitionBufferUsage(_, _, _)).Times(0);
    EXPECT_CALL(api, CommandBufferBuilderGetResult(_)).Times(0);

    FlushClient();

    EXPECT_CALL(*mockBuilderErrorCallback, Call(NXT_BUILDER_ERROR_STATUS_ERROR, _, 1, 2)).Times(1);
    EXPECT_CALL(*mockBuilderErrorCallback, Call(NXT_BUILDER_ERROR_STATUS_ERROR, _, 3, 4)).Times(1);

    FlushServer();
}

// Test that we get a success builder error status when no error happens
TEST_F(WireTests, SuccessCallbackOnBuilderSuccess) {
    nxtBufferBuilder bufferBuilder = nxtDeviceCreateBufferBuilder(device);
    nxtBufferBuilderSetErrorCallback(bufferBuilder, ToMockBuilderErrorCallback, 1, 2);
    nxtBuffer buffer = nxtBufferBuilderGetResult(bufferBuilder);

    nxtBufferBuilder apiBufferBuilder = api.GetNewBufferBuilder();
    EXPECT_CALL(api, DeviceCreateBufferBuilder(apiDevice))
        .WillOnce(Return(apiBufferBuilder));

    nxtBuffer apiBuffer = api.GetNewBuffer();
    EXPECT_CALL(api, BufferBuilderGetResult(apiBufferBuilder))
        .WillOnce(InvokeWithoutArgs([&]() -> nxtBuffer {
            api.CallBuilderErrorCallback(apiBufferBuilder, NXT_BUILDER_ERROR_STATUS_SUCCESS, "I like cheese");
            return apiBuffer;
        }));

    FlushClient();

    EXPECT_CALL(*mockBuilderErrorCallback, Call(NXT_BUILDER_ERROR_STATUS_SUCCESS, _ , 1 ,2));

    FlushServer();
}

// Test that the client calls the builder callback with unknown when it HAS to fire the callback but can't
// know the status yet.
TEST_F(WireTests, UnknownBuilderErrorStatusCallback) {
    // The builder is destroyed before the object is built
    {
        nxtBufferBuilder bufferBuilder = nxtDeviceCreateBufferBuilder(device);
        nxtBufferBuilderSetErrorCallback(bufferBuilder, ToMockBuilderErrorCallback, 1, 2);

        EXPECT_CALL(*mockBuilderErrorCallback, Call(NXT_BUILDER_ERROR_STATUS_UNKNOWN, _ , 1 ,2)).Times(1);

        nxtBufferBuilderRelease(bufferBuilder);
    }

    // If the builder has been consumed, it doesn't fire the callback with unknown
    {
        nxtBufferBuilder bufferBuilder = nxtDeviceCreateBufferBuilder(device);
        nxtBufferBuilderSetErrorCallback(bufferBuilder, ToMockBuilderErrorCallback, 3, 4);
        nxtBuffer buffer = nxtBufferBuilderGetResult(bufferBuilder);

        EXPECT_CALL(*mockBuilderErrorCallback, Call(NXT_BUILDER_ERROR_STATUS_UNKNOWN, _ , 3, 4)).Times(0);

        nxtBufferBuilderRelease(bufferBuilder);
    }

    // If the builder has been consumed, and the object is destroyed before the result comes from the server,
    // then the callback is fired with unknown
    {
        nxtBufferBuilder bufferBuilder = nxtDeviceCreateBufferBuilder(device);
        nxtBufferBuilderSetErrorCallback(bufferBuilder, ToMockBuilderErrorCallback, 5, 6);
        nxtBuffer buffer = nxtBufferBuilderGetResult(bufferBuilder);

        EXPECT_CALL(*mockBuilderErrorCallback, Call(NXT_BUILDER_ERROR_STATUS_UNKNOWN, _ , 5, 6)).Times(1);

        nxtBufferRelease(buffer);
    }
}

class WireSetCallbackTests : public WireTestsBase {
    public:
        WireSetCallbackTests() : WireTestsBase(false) {
        }
};

// Test the return wire for device error callbacks
TEST_F(WireSetCallbackTests, DeviceErrorCallback) {
    uint64_t userdata = 3049785;
    nxtDeviceSetErrorCallback(device, ToMockDeviceErrorCallback, userdata);

    // Setting the error callback should stay on the client side and do nothing
    FlushClient();

    // Calling the callback on the server side will result in the callback being called on the client side
    api.CallDeviceErrorCallback(apiDevice, "Some error message");

    EXPECT_CALL(*mockDeviceErrorCallback, Call(StrEq("Some error message"), userdata))
        .Times(1);

    FlushServer();
}

// Test the return wire for device error callbacks
TEST_F(WireSetCallbackTests, BuilderErrorCallback) {
    uint64_t userdata1 = 982734;
    uint64_t userdata2 = 982734239028;

    // Create the buffer builder, the callback is set immediately on the server side
    nxtBufferBuilder bufferBuilder = nxtDeviceCreateBufferBuilder(device);

    nxtBufferBuilder apiBufferBuilder = api.GetNewBufferBuilder();
    EXPECT_CALL(api, DeviceCreateBufferBuilder(apiDevice))
        .WillOnce(Return(apiBufferBuilder));

    EXPECT_CALL(api, OnBuilderSetErrorCallback(apiBufferBuilder, _, _, _))
        .Times(1);

    FlushClient();

    // Setting the callback on the client side doesn't do anything on the server side
    nxtBufferBuilderSetErrorCallback(bufferBuilder, ToMockBuilderErrorCallback, userdata1, userdata2);
    FlushClient();

    // Create an object so that it is a valid case to call the error callback
    nxtBuffer buffer = nxtBufferBuilderGetResult(bufferBuilder);

    nxtBuffer apiBuffer = api.GetNewBuffer();
    EXPECT_CALL(api, BufferBuilderGetResult(apiBufferBuilder))
        .WillOnce(InvokeWithoutArgs([&]() -> nxtBuffer {
            api.CallBuilderErrorCallback(apiBufferBuilder, NXT_BUILDER_ERROR_STATUS_SUCCESS, "Success!");
            return apiBuffer;
        }));

    FlushClient();

    // The error callback gets called on the client side
    EXPECT_CALL(*mockBuilderErrorCallback, Call(NXT_BUILDER_ERROR_STATUS_SUCCESS, StrEq("Success!"), userdata1, userdata2))
        .Times(1);

    FlushServer();
}

class WireBufferMappingTests : public WireTestsBase {
    public:
        WireBufferMappingTests() : WireTestsBase(true) {
        }

        void SetUp() override {
            WireTestsBase::SetUp();

            {
                nxtBufferBuilder apiBufferBuilder = api.GetNewBufferBuilder();
                nxtBufferBuilder bufferBuilder = nxtDeviceCreateBufferBuilder(device);
                EXPECT_CALL(api, DeviceCreateBufferBuilder(apiDevice))
                    .WillOnce(Return(apiBufferBuilder))
                    .RetiresOnSaturation();

                apiBuffer = api.GetNewBuffer();
                buffer = nxtBufferBuilderGetResult(bufferBuilder);
                EXPECT_CALL(api, BufferBuilderGetResult(apiBufferBuilder))
                    .WillOnce(Return(apiBuffer))
                    .RetiresOnSaturation();
                FlushClient();
            }
            {
                nxtBufferBuilder apiBufferBuilder = api.GetNewBufferBuilder();
                nxtBufferBuilder bufferBuilder = nxtDeviceCreateBufferBuilder(device);
                EXPECT_CALL(api, DeviceCreateBufferBuilder(apiDevice))
                    .WillOnce(Return(apiBufferBuilder))
                    .RetiresOnSaturation();

                errorBuffer = nxtBufferBuilderGetResult(bufferBuilder);
                EXPECT_CALL(api, BufferBuilderGetResult(apiBufferBuilder))
                    .WillOnce(Return(nullptr))
                    .RetiresOnSaturation();
                FlushClient();
            }
        }

    protected:
        // A successfully created buffer
        nxtBuffer buffer;
        nxtBuffer apiBuffer;

        // An buffer that wasn't created on the server side
        nxtBuffer errorBuffer;
};

// Check mapping a succesfully created buffer
TEST_F(WireBufferMappingTests, MappingSuccessBuffer) {
    nxtCallbackUserdata userdata = 8653;
    nxtBufferMapReadAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);
    
    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapReadAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapReadCallback(apiBuffer, NXT_BUFFER_MAP_READ_STATUS_SUCCESS, &bufferContent);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapReadCallback, Call(NXT_BUFFER_MAP_READ_STATUS_SUCCESS, Pointee(Eq(bufferContent)), userdata))
        .Times(1);

    FlushServer();

    nxtBufferUnmap(buffer);
    EXPECT_CALL(api, BufferUnmap(apiBuffer))
        .Times(1);

    FlushClient();
}

// Check that things work correctly when a validation error happens when mapping the buffer
TEST_F(WireBufferMappingTests, ErrorWhileMapping) {
    nxtCallbackUserdata userdata = 8654;
    nxtBufferMapReadAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);
    
    EXPECT_CALL(api, OnBufferMapReadAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapReadCallback(apiBuffer, NXT_BUFFER_MAP_READ_STATUS_ERROR, nullptr);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapReadCallback, Call(NXT_BUFFER_MAP_READ_STATUS_ERROR, nullptr, userdata))
        .Times(1);

    FlushServer();
}

// Check mapping a buffer that didn't get created on the server side
TEST_F(WireBufferMappingTests, MappingErrorBuffer) {
    nxtCallbackUserdata userdata = 8655;
    nxtBufferMapReadAsync(errorBuffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);

    FlushClient();

    EXPECT_CALL(*mockBufferMapReadCallback, Call(NXT_BUFFER_MAP_READ_STATUS_ERROR, nullptr, userdata))
        .Times(1);

    FlushServer();

    nxtBufferUnmap(errorBuffer);

    FlushClient();
}

// Check that the callback is called with UNKNOWN when the buffer is destroyed before the request is finished
TEST_F(WireBufferMappingTests, DestroyBeforeRequestEnd) {
    nxtCallbackUserdata userdata = 8656;
    nxtBufferMapReadAsync(errorBuffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);

    EXPECT_CALL(*mockBufferMapReadCallback, Call(NXT_BUFFER_MAP_READ_STATUS_UNKNOWN, nullptr, userdata))
        .Times(1);

    nxtBufferRelease(errorBuffer);
}

// Check the callback is called with UNKNOWN when the map request would have worked, but Unmap was called
TEST_F(WireBufferMappingTests, UnmapCalledTooEarly) {
    nxtCallbackUserdata userdata = 8657;
    nxtBufferMapReadAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);
    
    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapReadAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapReadCallback(apiBuffer, NXT_BUFFER_MAP_READ_STATUS_SUCCESS, &bufferContent);
        }));

    FlushClient();

    // Oh no! We are calling Unmap too early!
    EXPECT_CALL(*mockBufferMapReadCallback, Call(NXT_BUFFER_MAP_READ_STATUS_UNKNOWN, nullptr, userdata))
        .Times(1);
    nxtBufferUnmap(buffer);

    // The callback shouldn't get called, even when the request succeeded on the server side
    FlushServer();
}

// Check that an error callback gets nullptr while a buffer is already mapped
TEST_F(WireBufferMappingTests, MappingErrorWhileAlreadyMappedGetsNullptr) {
    // Successful map
    nxtCallbackUserdata userdata = 34098;
    nxtBufferMapReadAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapReadAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapReadCallback(apiBuffer, NXT_BUFFER_MAP_READ_STATUS_SUCCESS, &bufferContent);
        }))
        .RetiresOnSaturation();

    FlushClient();

    EXPECT_CALL(*mockBufferMapReadCallback, Call(NXT_BUFFER_MAP_READ_STATUS_SUCCESS, Pointee(Eq(bufferContent)), userdata))
        .Times(1);

    FlushServer();

    // Map failure while the buffer is already mapped
    userdata ++;
    nxtBufferMapReadAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);
    EXPECT_CALL(api, OnBufferMapReadAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapReadCallback(apiBuffer, NXT_BUFFER_MAP_READ_STATUS_ERROR, nullptr);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapReadCallback, Call(NXT_BUFFER_MAP_READ_STATUS_ERROR, nullptr, userdata))
        .Times(1);

    FlushServer();
}
