// Copyright 2017 The Dawn Authors
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
#include "mock/mock_dawn.h"

#include "common/Assert.h"
#include "wire/TerribleCommandBuffer.h"
#include "wire/Wire.h"

using namespace testing;
using namespace dawn::wire;

// Definition of a "Lambda predicate matcher" for GMock to allow checking deep structures
// are passed correctly by the wire.

// Helper templates to extract the argument type of a lambda.
template<typename T>
struct MatcherMethodArgument;

template<typename Lambda, typename Arg>
struct MatcherMethodArgument<bool (Lambda::*)(Arg) const> {
    using Type = Arg;
};

template<typename Lambda>
using MatcherLambdaArgument = typename MatcherMethodArgument<decltype(&Lambda::operator())>::Type;

// The matcher itself, unfortunately it isn't able to return detailed information like other
// matchers do.
template <typename Lambda, typename Arg>
class LambdaMatcherImpl : public MatcherInterface<Arg> {
  public:
    explicit LambdaMatcherImpl(Lambda lambda) : mLambda(lambda) {}

    void DescribeTo(std::ostream* os) const override {
        *os << "with a custom matcher";
    }

    bool MatchAndExplain(Arg value, MatchResultListener* listener) const override {
        if (!mLambda(value)) {
            *listener << "which doesn't satisfy the custom predicate";
            return false;
        }
        return true;
    }

  private:
    Lambda mLambda;
};

// Use the MatchesLambda as follows:
//
//   EXPECT_CALL(foo, Bar(MatchesLambda([](ArgType arg) -> bool {
//       return CheckPredicateOnArg(arg);
//   })));
template <typename Lambda>
inline Matcher<MatcherLambdaArgument<Lambda>> MatchesLambda(Lambda lambda) {
    return MakeMatcher(new LambdaMatcherImpl<Lambda, MatcherLambdaArgument<Lambda>>(lambda));
}

// Mock classes to add expectations on the wire calling callbacks
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
        MOCK_METHOD3(Call, void(nxtBufferMapAsyncStatus status, const uint32_t* ptr, nxtCallbackUserdata userdata));
};

static MockBufferMapReadCallback* mockBufferMapReadCallback = nullptr;
static void ToMockBufferMapReadCallback(nxtBufferMapAsyncStatus status, const void* ptr, nxtCallbackUserdata userdata) {
    // Assume the data is uint32_t to make writing matchers easier
    mockBufferMapReadCallback->Call(status, static_cast<const uint32_t*>(ptr), userdata);
}

class MockBufferMapWriteCallback {
    public:
        MOCK_METHOD3(Call, void(nxtBufferMapAsyncStatus status, uint32_t* ptr, nxtCallbackUserdata userdata));
};

static MockBufferMapWriteCallback* mockBufferMapWriteCallback = nullptr;
uint32_t* lastMapWritePointer = nullptr;
static void ToMockBufferMapWriteCallback(nxtBufferMapAsyncStatus status, void* ptr, nxtCallbackUserdata userdata) {
    // Assume the data is uint32_t to make writing matchers easier
    lastMapWritePointer = static_cast<uint32_t*>(ptr);
    mockBufferMapWriteCallback->Call(status, lastMapWritePointer, userdata);
}

class WireTestsBase : public Test {
    protected:
        WireTestsBase(bool ignoreSetCallbackCalls)
            : mIgnoreSetCallbackCalls(ignoreSetCallbackCalls) {
        }

        void SetUp() override {
            mockDeviceErrorCallback = new MockDeviceErrorCallback;
            mockBuilderErrorCallback = new MockBuilderErrorCallback;
            mockBufferMapReadCallback = new MockBufferMapReadCallback;
            mockBufferMapWriteCallback = new MockBufferMapWriteCallback;

            nxtProcTable mockProcs;
            nxtDevice mockDevice;
            api.GetProcTableAndDevice(&mockProcs, &mockDevice);

            // This SetCallback call cannot be ignored because it is done as soon as we start the server
            EXPECT_CALL(api, OnDeviceSetErrorCallback(_, _, _)).Times(Exactly(1));
            if (mIgnoreSetCallbackCalls) {
                EXPECT_CALL(api, OnBuilderSetErrorCallback(_, _, _, _)).Times(AnyNumber());
            }
            EXPECT_CALL(api, DeviceTick(_)).Times(AnyNumber());

            mS2cBuf = new TerribleCommandBuffer();
            mC2sBuf = new TerribleCommandBuffer(mWireServer);

            mWireServer = NewServerCommandHandler(mockDevice, mockProcs, mS2cBuf);
            mC2sBuf->SetHandler(mWireServer);

            nxtProcTable clientProcs;
            mWireClient = NewClientDevice(&clientProcs, &device, mC2sBuf);
            nxtSetProcs(&clientProcs);
            mS2cBuf->SetHandler(mWireClient);

            apiDevice = mockDevice;
        }

        void TearDown() override {
            nxtSetProcs(nullptr);
            delete mWireServer;
            delete mWireClient;
            delete mC2sBuf;
            delete mS2cBuf;
            delete mockDeviceErrorCallback;
            delete mockBuilderErrorCallback;
            delete mockBufferMapReadCallback;
            delete mockBufferMapWriteCallback;
        }

        void FlushClient() {
            ASSERT_TRUE(mC2sBuf->Flush());
        }

        void FlushServer() {
            ASSERT_TRUE(mS2cBuf->Flush());
        }

        MockProcTable api;
        nxtDevice apiDevice;
        nxtDevice device;

    private:
        bool mIgnoreSetCallbackCalls = false;

        CommandHandler* mWireServer = nullptr;
        CommandHandler* mWireClient = nullptr;
        TerribleCommandBuffer* mS2cBuf = nullptr;
        TerribleCommandBuffer* mC2sBuf = nullptr;
};

class WireTests : public WireTestsBase {
    public:
        WireTests() : WireTestsBase(true) {
        }
};

// One call gets forwarded correctly.
TEST_F(WireTests, CallForwarded) {
    nxtDeviceCreateCommandBufferBuilder(device);

    nxtCommandBufferBuilder apiCmdBufBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiCmdBufBuilder));

    FlushClient();
}

// Test that calling methods on a new object works as expected.
TEST_F(WireTests, CreateThenCall) {
    nxtCommandBufferBuilder builder = nxtDeviceCreateCommandBufferBuilder(device);
    nxtCommandBufferBuilderGetResult(builder);

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
    nxtCommandBufferBuilder builder = nxtDeviceCreateCommandBufferBuilder(device);
    nxtCommandBufferBuilderDispatch(builder, 1, 2, 3);

    nxtCommandBufferBuilder apiBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiBuilder));

    EXPECT_CALL(api, CommandBufferBuilderDispatch(apiBuilder, 1, 2, 3))
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
    nxtRenderPipelineBuilder pipelineBuilder = nxtDeviceCreateRenderPipelineBuilder(device);
    nxtRenderPipelineBuilderSetStage(pipelineBuilder, NXT_SHADER_STAGE_FRAGMENT, shaderModule, "my entry point");

    nxtRenderPipelineBuilder apiPipelineBuilder = api.GetNewRenderPipelineBuilder();
    EXPECT_CALL(api, DeviceCreateRenderPipelineBuilder(apiDevice))
        .WillOnce(Return(apiPipelineBuilder));

    EXPECT_CALL(api, RenderPipelineBuilderSetStage(apiPipelineBuilder, NXT_SHADER_STAGE_FRAGMENT, apiShaderModule, StrEq("my entry point")));

    FlushClient();
}

// Test that the wire is able to send objects as value arguments
TEST_F(WireTests, ObjectAsValueArgument) {
    // Create pipeline
    nxtRenderPipelineBuilder pipelineBuilder = nxtDeviceCreateRenderPipelineBuilder(device);
    nxtRenderPipeline pipeline = nxtRenderPipelineBuilderGetResult(pipelineBuilder);

    nxtRenderPipelineBuilder apiPipelineBuilder = api.GetNewRenderPipelineBuilder();
    EXPECT_CALL(api, DeviceCreateRenderPipelineBuilder(apiDevice))
        .WillOnce(Return(apiPipelineBuilder));

    nxtRenderPipeline apiPipeline = api.GetNewRenderPipeline();
    EXPECT_CALL(api, RenderPipelineBuilderGetResult(apiPipelineBuilder))
        .WillOnce(Return(apiPipeline));

    // Create command buffer builder, setting pipeline
    nxtCommandBufferBuilder cmdBufBuilder = nxtDeviceCreateCommandBufferBuilder(device);
    nxtCommandBufferBuilderSetRenderPipeline(cmdBufBuilder, pipeline);

    nxtCommandBufferBuilder apiCmdBufBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiCmdBufBuilder));

    EXPECT_CALL(api, CommandBufferBuilderSetRenderPipeline(apiCmdBufBuilder, apiPipeline));

    FlushClient();
}

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
    nxtQueue queue = nxtDeviceCreateQueue(device);
    nxtQueue apiQueue = api.GetNewQueue();
    EXPECT_CALL(api, DeviceCreateQueue(apiDevice))
        .WillOnce(Return(apiQueue));

    // Submit command buffer and check we got a call with both API-side command buffers
    nxtQueueSubmit(queue, 2, cmdBufs);

    EXPECT_CALL(api, QueueSubmit(apiQueue, 2, MatchesLambda([=](const nxtCommandBuffer* cmdBufs) -> bool {
        return cmdBufs[0] == apiCmdBufs[0] && cmdBufs[1] == apiCmdBufs[1];
    })));

    FlushClient();
}

// Test that the wire is able to send structures that contain pure values (non-objects)
TEST_F(WireTests, StructureOfValuesArgument) {
    nxtSamplerDescriptor descriptor;
    descriptor.nextInChain = nullptr;
    descriptor.magFilter = NXT_FILTER_MODE_LINEAR;
    descriptor.minFilter = NXT_FILTER_MODE_NEAREST;
    descriptor.mipmapFilter = NXT_FILTER_MODE_LINEAR;
    descriptor.addressModeU = NXT_ADDRESS_MODE_CLAMP_TO_EDGE;
    descriptor.addressModeV = NXT_ADDRESS_MODE_REPEAT;
    descriptor.addressModeW = NXT_ADDRESS_MODE_MIRRORED_REPEAT;

    nxtDeviceCreateSampler(device, &descriptor);
    EXPECT_CALL(api, DeviceCreateSampler(apiDevice, MatchesLambda([](const nxtSamplerDescriptor* desc) -> bool {
        return desc->nextInChain == nullptr &&
            desc->magFilter == NXT_FILTER_MODE_LINEAR &&
            desc->minFilter == NXT_FILTER_MODE_NEAREST &&
            desc->mipmapFilter == NXT_FILTER_MODE_LINEAR &&
            desc->addressModeU == NXT_ADDRESS_MODE_CLAMP_TO_EDGE &&
            desc->addressModeV == NXT_ADDRESS_MODE_REPEAT &&
            desc->addressModeW == NXT_ADDRESS_MODE_MIRRORED_REPEAT;
    })))
        .WillOnce(Return(nullptr));

    FlushClient();
}

// Test that the wire is able to send structures that contain objects
TEST_F(WireTests, StructureOfObjectArrayArgument) {
    nxtBindGroupLayoutDescriptor bglDescriptor;
    bglDescriptor.numBindings = 0;
    bglDescriptor.bindings = nullptr;

    nxtBindGroupLayout bgl = nxtDeviceCreateBindGroupLayout(device, &bglDescriptor);
    nxtBindGroupLayout apiBgl = api.GetNewBindGroupLayout();
    EXPECT_CALL(api, DeviceCreateBindGroupLayout(apiDevice, _)).WillOnce(Return(apiBgl));

    nxtPipelineLayoutDescriptor descriptor;
    descriptor.nextInChain = nullptr;
    descriptor.numBindGroupLayouts = 1;
    descriptor.bindGroupLayouts = &bgl;

    nxtDeviceCreatePipelineLayout(device, &descriptor);
    EXPECT_CALL(api, DeviceCreatePipelineLayout(apiDevice, MatchesLambda([apiBgl](const nxtPipelineLayoutDescriptor* desc) -> bool {
        return desc->nextInChain == nullptr &&
            desc->numBindGroupLayouts == 1 &&
            desc->bindGroupLayouts[0] == apiBgl;
    })))
        .WillOnce(Return(nullptr));

    FlushClient();
}

// Test that the wire is able to send structures that contain objects
TEST_F(WireTests, StructureOfStructureArrayArgument) {
    static constexpr int NUM_BINDINGS = 3;
    nxtBindGroupBinding bindings[NUM_BINDINGS]{
        {0, NXT_SHADER_STAGE_BIT_VERTEX, NXT_BINDING_TYPE_SAMPLER},
        {1, NXT_SHADER_STAGE_BIT_VERTEX, NXT_BINDING_TYPE_SAMPLED_TEXTURE},
        {2,
         static_cast<nxtShaderStageBit>(NXT_SHADER_STAGE_BIT_VERTEX |
                                        NXT_SHADER_STAGE_BIT_FRAGMENT),
         NXT_BINDING_TYPE_UNIFORM_BUFFER},
    };
    nxtBindGroupLayoutDescriptor bglDescriptor;
    bglDescriptor.numBindings = NUM_BINDINGS;
    bglDescriptor.bindings = bindings;

    nxtDeviceCreateBindGroupLayout(device, &bglDescriptor);
    nxtBindGroupLayout apiBgl = api.GetNewBindGroupLayout();
    EXPECT_CALL(
        api,
        DeviceCreateBindGroupLayout(
            apiDevice, MatchesLambda([bindings](const nxtBindGroupLayoutDescriptor* desc) -> bool {
                for (int i = 0; i < NUM_BINDINGS; ++i) {
                    const auto& a = desc->bindings[i];
                    const auto& b = bindings[i];
                    if (a.binding != b.binding || a.visibility != b.visibility ||
                        a.type != b.type) {
                        return false;
                    }
                }
                return desc->nextInChain == nullptr && desc->numBindings == 3;
            })))
        .WillOnce(Return(apiBgl));

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
    nxtBufferSetSubData(buffer, 0, 0, nullptr);
    nxtCommandBufferBuilderSetIndexBuffer(cmdBufBuilder, buffer, 0);
    nxtCommandBufferBuilderGetResult(cmdBufBuilder);

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

    EXPECT_CALL(api, BufferSetSubData(_, _, _, _)).Times(0);
    EXPECT_CALL(api, CommandBufferBuilderSetIndexBuffer(_, _, _)).Times(0);
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
    nxtBufferBuilderGetResult(bufferBuilder);

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
        nxtBufferBuilderGetResult(bufferBuilder);

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

// Test that a builder success status doesn't get forwarded to the device
TEST_F(WireTests, SuccessCallbackNotForwardedToDevice) {
    nxtDeviceSetErrorCallback(device, ToMockDeviceErrorCallback, 0);

    nxtBufferBuilder bufferBuilder = nxtDeviceCreateBufferBuilder(device);
    nxtBufferBuilderGetResult(bufferBuilder);

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
    FlushServer();
}

// Test that a builder error status gets forwarded to the device
TEST_F(WireTests, ErrorCallbackForwardedToDevice) {
    uint64_t userdata = 30495;
    nxtDeviceSetErrorCallback(device, ToMockDeviceErrorCallback, userdata);

    nxtBufferBuilder bufferBuilder = nxtDeviceCreateBufferBuilder(device);
    nxtBufferBuilderGetResult(bufferBuilder);

    nxtBufferBuilder apiBufferBuilder = api.GetNewBufferBuilder();
    EXPECT_CALL(api, DeviceCreateBufferBuilder(apiDevice))
        .WillOnce(Return(apiBufferBuilder));

    EXPECT_CALL(api, BufferBuilderGetResult(apiBufferBuilder))
        .WillOnce(InvokeWithoutArgs([&]() -> nxtBuffer {
            api.CallBuilderErrorCallback(apiBufferBuilder, NXT_BUILDER_ERROR_STATUS_ERROR, "Error :(");
            return nullptr;
        }));

    FlushClient();

    EXPECT_CALL(*mockDeviceErrorCallback, Call(_, userdata)).Times(1);

    FlushServer();
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
    nxtBufferBuilderGetResult(bufferBuilder);

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

// MapRead-specific tests

// Check mapping for reading a succesfully created buffer
TEST_F(WireBufferMappingTests, MappingForReadSuccessBuffer) {
    nxtCallbackUserdata userdata = 8653;
    nxtBufferMapReadAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);
    
    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapReadAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapReadCallback(apiBuffer, NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, &bufferContent);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapReadCallback, Call(NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Pointee(Eq(bufferContent)), userdata))
        .Times(1);

    FlushServer();

    nxtBufferUnmap(buffer);
    EXPECT_CALL(api, BufferUnmap(apiBuffer))
        .Times(1);

    FlushClient();
}

// Check that things work correctly when a validation error happens when mapping the buffer for reading
TEST_F(WireBufferMappingTests, ErrorWhileMappingForRead) {
    nxtCallbackUserdata userdata = 8654;
    nxtBufferMapReadAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);
    
    EXPECT_CALL(api, OnBufferMapReadAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapReadCallback(apiBuffer, NXT_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapReadCallback, Call(NXT_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata))
        .Times(1);

    FlushServer();
}

// Check mapping for reading a buffer that didn't get created on the server side
TEST_F(WireBufferMappingTests, MappingForReadErrorBuffer) {
    nxtCallbackUserdata userdata = 8655;
    nxtBufferMapReadAsync(errorBuffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);

    FlushClient();

    EXPECT_CALL(*mockBufferMapReadCallback, Call(NXT_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata))
        .Times(1);

    FlushServer();

    nxtBufferUnmap(errorBuffer);

    FlushClient();
}

// Check that the map read callback is called with UNKNOWN when the buffer is destroyed before the request is finished
TEST_F(WireBufferMappingTests, DestroyBeforeReadRequestEnd) {
    nxtCallbackUserdata userdata = 8656;
    nxtBufferMapReadAsync(errorBuffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);

    EXPECT_CALL(*mockBufferMapReadCallback, Call(NXT_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr, userdata))
        .Times(1);

    nxtBufferRelease(errorBuffer);
}

// Check the map read callback is called with UNKNOWN when the map request would have worked, but Unmap was called
TEST_F(WireBufferMappingTests, UnmapCalledTooEarlyForRead) {
    nxtCallbackUserdata userdata = 8657;
    nxtBufferMapReadAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapReadAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapReadCallback(apiBuffer, NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, &bufferContent);
        }));

    FlushClient();

    // Oh no! We are calling Unmap too early!
    EXPECT_CALL(*mockBufferMapReadCallback, Call(NXT_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr, userdata))
        .Times(1);
    nxtBufferUnmap(buffer);

    // The callback shouldn't get called, even when the request succeeded on the server side
    FlushServer();
}

// Check that an error map read callback gets nullptr while a buffer is already mapped
TEST_F(WireBufferMappingTests, MappingForReadingErrorWhileAlreadyMappedGetsNullptr) {
    // Successful map
    nxtCallbackUserdata userdata = 34098;
    nxtBufferMapReadAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapReadAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapReadCallback(apiBuffer, NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, &bufferContent);
        }))
        .RetiresOnSaturation();

    FlushClient();

    EXPECT_CALL(*mockBufferMapReadCallback, Call(NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Pointee(Eq(bufferContent)), userdata))
        .Times(1);

    FlushServer();

    // Map failure while the buffer is already mapped
    userdata ++;
    nxtBufferMapReadAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);
    EXPECT_CALL(api, OnBufferMapReadAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapReadCallback(apiBuffer, NXT_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapReadCallback, Call(NXT_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata))
        .Times(1);

    FlushServer();
}

// Test that the MapReadCallback isn't fired twice when unmap() is called inside the callback
TEST_F(WireBufferMappingTests, UnmapInsideMapReadCallback) {
    nxtCallbackUserdata userdata = 2039;
    nxtBufferMapReadAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapReadAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapReadCallback(apiBuffer, NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, &bufferContent);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapReadCallback, Call(NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Pointee(Eq(bufferContent)), userdata))
        .WillOnce(InvokeWithoutArgs([&]() {
            nxtBufferUnmap(buffer);
        }));

    FlushServer();

    EXPECT_CALL(api, BufferUnmap(apiBuffer))
        .Times(1);

    FlushClient();
}

// Test that the MapReadCallback isn't fired twice the buffer external refcount reaches 0 in the callback
TEST_F(WireBufferMappingTests, DestroyInsideMapReadCallback) {
    nxtCallbackUserdata userdata = 2039;
    nxtBufferMapReadAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapReadAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapReadCallback(apiBuffer, NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, &bufferContent);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapReadCallback, Call(NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Pointee(Eq(bufferContent)), userdata))
        .WillOnce(InvokeWithoutArgs([&]() {
            nxtBufferRelease(buffer);
        }));

    FlushServer();

    EXPECT_CALL(api, BufferRelease(apiBuffer))
        .Times(1);

    FlushClient();
}

// MapWrite-specific tests

// Check mapping for writing a succesfully created buffer
TEST_F(WireBufferMappingTests, MappingForWriteSuccessBuffer) {
    nxtCallbackUserdata userdata = 8653;
    nxtBufferMapWriteAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapWriteCallback, userdata);

    uint32_t serverBufferContent = 31337;
    uint32_t updatedContent = 4242;
    uint32_t zero = 0;

    EXPECT_CALL(api, OnBufferMapWriteAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapWriteCallback(apiBuffer, NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, &serverBufferContent);
        }));

    FlushClient();

    // The map write callback always gets a buffer full of zeroes.
    EXPECT_CALL(*mockBufferMapWriteCallback, Call(NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Pointee(Eq(zero)), userdata))
        .Times(1);

    FlushServer();

    // Write something to the mapped pointer
    *lastMapWritePointer = updatedContent;

    nxtBufferUnmap(buffer);
    EXPECT_CALL(api, BufferUnmap(apiBuffer))
        .Times(1);

    FlushClient();

    // After the buffer is unmapped, the content of the buffer is updated on the server
    ASSERT_EQ(serverBufferContent, updatedContent);
}

// Check that things work correctly when a validation error happens when mapping the buffer for writing
TEST_F(WireBufferMappingTests, ErrorWhileMappingForWrite) {
    nxtCallbackUserdata userdata = 8654;
    nxtBufferMapWriteAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapWriteCallback, userdata);

    EXPECT_CALL(api, OnBufferMapWriteAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapWriteCallback(apiBuffer, NXT_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapWriteCallback, Call(NXT_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata))
        .Times(1);

    FlushServer();
}

// Check mapping for writing a buffer that didn't get created on the server side
TEST_F(WireBufferMappingTests, MappingForWriteErrorBuffer) {
    nxtCallbackUserdata userdata = 8655;
    nxtBufferMapWriteAsync(errorBuffer, 40, sizeof(uint32_t), ToMockBufferMapWriteCallback, userdata);

    FlushClient();

    EXPECT_CALL(*mockBufferMapWriteCallback, Call(NXT_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata))
        .Times(1);

    FlushServer();

    nxtBufferUnmap(errorBuffer);

    FlushClient();
}

// Check that the map write callback is called with UNKNOWN when the buffer is destroyed before the request is finished
TEST_F(WireBufferMappingTests, DestroyBeforeWriteRequestEnd) {
    nxtCallbackUserdata userdata = 8656;
    nxtBufferMapWriteAsync(errorBuffer, 40, sizeof(uint32_t), ToMockBufferMapWriteCallback, userdata);

    EXPECT_CALL(*mockBufferMapWriteCallback, Call(NXT_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr, userdata))
        .Times(1);

    nxtBufferRelease(errorBuffer);
}

// Check the map read callback is called with UNKNOWN when the map request would have worked, but Unmap was called
TEST_F(WireBufferMappingTests, UnmapCalledTooEarlyForWrite) {
    nxtCallbackUserdata userdata = 8657;
    nxtBufferMapWriteAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapWriteCallback, userdata);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapWriteAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapWriteCallback(apiBuffer, NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, &bufferContent);
        }));

    FlushClient();

    // Oh no! We are calling Unmap too early!
    EXPECT_CALL(*mockBufferMapWriteCallback, Call(NXT_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr, userdata))
        .Times(1);
    nxtBufferUnmap(buffer);

    // The callback shouldn't get called, even when the request succeeded on the server side
    FlushServer();
}

// Check that an error map read callback gets nullptr while a buffer is already mapped
TEST_F(WireBufferMappingTests, MappingForWritingErrorWhileAlreadyMappedGetsNullptr) {
    // Successful map
    nxtCallbackUserdata userdata = 34098;
    nxtBufferMapWriteAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapWriteCallback, userdata);

    uint32_t bufferContent = 31337;
    uint32_t zero = 0;
    EXPECT_CALL(api, OnBufferMapWriteAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapWriteCallback(apiBuffer, NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, &bufferContent);
        }))
        .RetiresOnSaturation();

    FlushClient();

    EXPECT_CALL(*mockBufferMapWriteCallback, Call(NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Pointee(Eq(zero)), userdata))
        .Times(1);

    FlushServer();

    // Map failure while the buffer is already mapped
    userdata ++;
    nxtBufferMapWriteAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapWriteCallback, userdata);
    EXPECT_CALL(api, OnBufferMapWriteAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapWriteCallback(apiBuffer, NXT_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapWriteCallback, Call(NXT_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata))
        .Times(1);

    FlushServer();
}

// Test that the MapWriteCallback isn't fired twice when unmap() is called inside the callback
TEST_F(WireBufferMappingTests, UnmapInsideMapWriteCallback) {
    nxtCallbackUserdata userdata = 2039;
    nxtBufferMapWriteAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapWriteCallback, userdata);

    uint32_t bufferContent = 31337;
    uint32_t zero = 0;
    EXPECT_CALL(api, OnBufferMapWriteAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapWriteCallback(apiBuffer, NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, &bufferContent);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapWriteCallback, Call(NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Pointee(Eq(zero)), userdata))
        .WillOnce(InvokeWithoutArgs([&]() {
            nxtBufferUnmap(buffer);
        }));

    FlushServer();

    EXPECT_CALL(api, BufferUnmap(apiBuffer))
        .Times(1);

    FlushClient();
}

// Test that the MapWriteCallback isn't fired twice the buffer external refcount reaches 0 in the callback
TEST_F(WireBufferMappingTests, DestroyInsideMapWriteCallback) {
    nxtCallbackUserdata userdata = 2039;
    nxtBufferMapWriteAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapWriteCallback, userdata);

    uint32_t bufferContent = 31337;
    uint32_t zero = 0;
    EXPECT_CALL(api, OnBufferMapWriteAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapWriteCallback(apiBuffer, NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, &bufferContent);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapWriteCallback, Call(NXT_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Pointee(Eq(zero)), userdata))
        .WillOnce(InvokeWithoutArgs([&]() {
            nxtBufferRelease(buffer);
        }));

    FlushServer();

    EXPECT_CALL(api, BufferRelease(apiBuffer))
        .Times(1);

    FlushClient();
}
