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
#include "dawn_wire/Wire.h"
#include "utils/TerribleCommandBuffer.h"

#include <memory>

using namespace testing;
using namespace dawn_wire;

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
        MOCK_METHOD2(Call, void(const char* message, dawnCallbackUserdata userdata));
};

static std::unique_ptr<MockDeviceErrorCallback> mockDeviceErrorCallback;
static void ToMockDeviceErrorCallback(const char* message, dawnCallbackUserdata userdata) {
    mockDeviceErrorCallback->Call(message, userdata);
}

class MockBuilderErrorCallback {
    public:
        MOCK_METHOD4(Call, void(dawnBuilderErrorStatus status, const char* message, dawnCallbackUserdata userdata1, dawnCallbackUserdata userdata2));
};

static std::unique_ptr<MockBuilderErrorCallback> mockBuilderErrorCallback;
static void ToMockBuilderErrorCallback(dawnBuilderErrorStatus status, const char* message, dawnCallbackUserdata userdata1, dawnCallbackUserdata userdata2) {
    mockBuilderErrorCallback->Call(status, message, userdata1, userdata2);
}

class MockBufferMapReadCallback {
    public:
        MOCK_METHOD3(Call, void(dawnBufferMapAsyncStatus status, const uint32_t* ptr, dawnCallbackUserdata userdata));
};

static std::unique_ptr<MockBufferMapReadCallback> mockBufferMapReadCallback;
static void ToMockBufferMapReadCallback(dawnBufferMapAsyncStatus status, const void* ptr, dawnCallbackUserdata userdata) {
    // Assume the data is uint32_t to make writing matchers easier
    mockBufferMapReadCallback->Call(status, static_cast<const uint32_t*>(ptr), userdata);
}

class MockBufferMapWriteCallback {
    public:
        MOCK_METHOD3(Call, void(dawnBufferMapAsyncStatus status, uint32_t* ptr, dawnCallbackUserdata userdata));
};

static std::unique_ptr<MockBufferMapWriteCallback> mockBufferMapWriteCallback;
uint32_t* lastMapWritePointer = nullptr;
static void ToMockBufferMapWriteCallback(dawnBufferMapAsyncStatus status, void* ptr, dawnCallbackUserdata userdata) {
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
            mockDeviceErrorCallback = std::make_unique<MockDeviceErrorCallback>();
            mockBuilderErrorCallback = std::make_unique<MockBuilderErrorCallback>();
            mockBufferMapReadCallback = std::make_unique<MockBufferMapReadCallback>();
            mockBufferMapWriteCallback = std::make_unique<MockBufferMapWriteCallback>();

            dawnProcTable mockProcs;
            dawnDevice mockDevice;
            api.GetProcTableAndDevice(&mockProcs, &mockDevice);

            // This SetCallback call cannot be ignored because it is done as soon as we start the server
            EXPECT_CALL(api, OnDeviceSetErrorCallback(_, _, _)).Times(Exactly(1));
            if (mIgnoreSetCallbackCalls) {
                EXPECT_CALL(api, OnBuilderSetErrorCallback(_, _, _, _)).Times(AnyNumber());
            }
            EXPECT_CALL(api, DeviceTick(_)).Times(AnyNumber());

            mS2cBuf = std::make_unique<utils::TerribleCommandBuffer>();
            mC2sBuf = std::make_unique<utils::TerribleCommandBuffer>(mWireServer.get());

            mWireServer.reset(NewServerCommandHandler(mockDevice, mockProcs, mS2cBuf.get()));
            mC2sBuf->SetHandler(mWireServer.get());

            dawnProcTable clientProcs;
            mWireClient.reset(NewClientDevice(&clientProcs, &device, mC2sBuf.get()));
            dawnSetProcs(&clientProcs);
            mS2cBuf->SetHandler(mWireClient.get());

            apiDevice = mockDevice;
        }

        void TearDown() override {
            dawnSetProcs(nullptr);

            // Delete mocks so that expectations are checked
            mockDeviceErrorCallback = nullptr;
            mockBuilderErrorCallback = nullptr;
            mockBufferMapReadCallback = nullptr;
            mockBufferMapWriteCallback = nullptr;
        }

        void FlushClient() {
            ASSERT_TRUE(mC2sBuf->Flush());
        }

        void FlushServer() {
            ASSERT_TRUE(mS2cBuf->Flush());
        }

        MockProcTable api;
        dawnDevice apiDevice;
        dawnDevice device;

    private:
        bool mIgnoreSetCallbackCalls = false;

        std::unique_ptr<CommandHandler> mWireServer;
        std::unique_ptr<CommandHandler> mWireClient;
        std::unique_ptr<utils::TerribleCommandBuffer> mS2cBuf;
        std::unique_ptr<utils::TerribleCommandBuffer> mC2sBuf;
};

class WireTests : public WireTestsBase {
    public:
        WireTests() : WireTestsBase(true) {
        }
};

// One call gets forwarded correctly.
TEST_F(WireTests, CallForwarded) {
    dawnDeviceCreateCommandBufferBuilder(device);

    dawnCommandBufferBuilder apiCmdBufBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiCmdBufBuilder));

    FlushClient();
}

// Test that calling methods on a new object works as expected.
TEST_F(WireTests, CreateThenCall) {
    dawnCommandBufferBuilder builder = dawnDeviceCreateCommandBufferBuilder(device);
    dawnCommandBufferBuilderGetResult(builder);

    dawnCommandBufferBuilder apiCmdBufBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiCmdBufBuilder));

    dawnCommandBuffer apiCmdBuf = api.GetNewCommandBuffer();
    EXPECT_CALL(api, CommandBufferBuilderGetResult(apiCmdBufBuilder))
        .WillOnce(Return(apiCmdBuf));

    FlushClient();
}

// Test that client reference/release do not call the backend API.
TEST_F(WireTests, RefCountKeptInClient) {
    dawnCommandBufferBuilder builder = dawnDeviceCreateCommandBufferBuilder(device);

    dawnCommandBufferBuilderReference(builder);
    dawnCommandBufferBuilderRelease(builder);

    dawnCommandBufferBuilder apiCmdBufBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiCmdBufBuilder));

    FlushClient();
}

// Test that client reference/release do not call the backend API.
TEST_F(WireTests, ReleaseCalledOnRefCount0) {
    dawnCommandBufferBuilder builder = dawnDeviceCreateCommandBufferBuilder(device);

    dawnCommandBufferBuilderRelease(builder);

    dawnCommandBufferBuilder apiCmdBufBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiCmdBufBuilder));

    EXPECT_CALL(api, CommandBufferBuilderRelease(apiCmdBufBuilder));

    FlushClient();
}

// Test that the wire is able to send numerical values
TEST_F(WireTests, ValueArgument) {
    dawnCommandBufferBuilder builder = dawnDeviceCreateCommandBufferBuilder(device);
    dawnComputePassEncoder pass = dawnCommandBufferBuilderBeginComputePass(builder);
    dawnComputePassEncoderDispatch(pass, 1, 2, 3);

    dawnCommandBufferBuilder apiBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiBuilder));

    dawnComputePassEncoder apiPass = api.GetNewComputePassEncoder();
    EXPECT_CALL(api, CommandBufferBuilderBeginComputePass(apiBuilder))
        .WillOnce(Return(apiPass));

    EXPECT_CALL(api, ComputePassEncoderDispatch(apiPass, 1, 2, 3))
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
    dawnCommandBufferBuilder builder = dawnDeviceCreateCommandBufferBuilder(device);
    dawnComputePassEncoder pass = dawnCommandBufferBuilderBeginComputePass(builder);
    dawnComputePassEncoderSetPushConstants(pass, DAWN_SHADER_STAGE_BIT_VERTEX, 0, 4, testPushConstantValues);

    dawnCommandBufferBuilder apiBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiBuilder));

    dawnComputePassEncoder apiPass = api.GetNewComputePassEncoder();
    EXPECT_CALL(api, CommandBufferBuilderBeginComputePass(apiBuilder))
        .WillOnce(Return(apiPass));

    EXPECT_CALL(api, ComputePassEncoderSetPushConstants(apiPass, DAWN_SHADER_STAGE_BIT_VERTEX, 0, 4, ResultOf(CheckPushConstantValues, Eq(true))));

    FlushClient();
}

// Test that the wire is able to send C strings
TEST_F(WireTests, CStringArgument) {
    // Create shader module
    dawnShaderModuleDescriptor descriptor;
    descriptor.nextInChain = nullptr;
    descriptor.codeSize = 0;
    dawnShaderModule shaderModule = dawnDeviceCreateShaderModule(device, &descriptor);

    dawnShaderModule apiShaderModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _))
        .WillOnce(Return(apiShaderModule));

    // Create pipeline
    dawnRenderPipelineBuilder pipelineBuilder = dawnDeviceCreateRenderPipelineBuilder(device);
    dawnRenderPipelineBuilderSetStage(pipelineBuilder, DAWN_SHADER_STAGE_FRAGMENT, shaderModule, "my entry point");

    dawnRenderPipelineBuilder apiPipelineBuilder = api.GetNewRenderPipelineBuilder();
    EXPECT_CALL(api, DeviceCreateRenderPipelineBuilder(apiDevice))
        .WillOnce(Return(apiPipelineBuilder));

    EXPECT_CALL(api, RenderPipelineBuilderSetStage(apiPipelineBuilder, DAWN_SHADER_STAGE_FRAGMENT, apiShaderModule, StrEq("my entry point")));

    FlushClient();
}

// Test that the wire is able to send objects as value arguments
TEST_F(WireTests, ObjectAsValueArgument) {
    // Create a RenderPassDescriptor
    dawnRenderPassDescriptorBuilder renderPassBuilder = dawnDeviceCreateRenderPassDescriptorBuilder(device);
    dawnRenderPassDescriptor renderPass = dawnRenderPassDescriptorBuilderGetResult(renderPassBuilder);

    dawnRenderPassDescriptorBuilder apiRenderPassBuilder = api.GetNewRenderPassDescriptorBuilder();
    EXPECT_CALL(api, DeviceCreateRenderPassDescriptorBuilder(apiDevice))
        .WillOnce(Return(apiRenderPassBuilder));
    dawnRenderPassDescriptor apiRenderPass = api.GetNewRenderPassDescriptor();
    EXPECT_CALL(api, RenderPassDescriptorBuilderGetResult(apiRenderPassBuilder))
        .WillOnce(Return(apiRenderPass));

    // Create command buffer builder, setting render pass descriptor
    dawnCommandBufferBuilder cmdBufBuilder = dawnDeviceCreateCommandBufferBuilder(device);
    dawnCommandBufferBuilderBeginRenderPass(cmdBufBuilder, renderPass);

    dawnCommandBufferBuilder apiCmdBufBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiCmdBufBuilder));

    EXPECT_CALL(api, CommandBufferBuilderBeginRenderPass(apiCmdBufBuilder, apiRenderPass))
        .Times(1);

    FlushClient();
}

// Test that the wire is able to send array of objects
TEST_F(WireTests, ObjectsAsPointerArgument) {
    dawnCommandBuffer cmdBufs[2];
    dawnCommandBuffer apiCmdBufs[2];

    // Create two command buffers we need to use a GMock sequence otherwise the order of the
    // CreateCommandBufferBuilder might be swapped since they are equivalent in term of matchers
    Sequence s;
    for (int i = 0; i < 2; ++i) {
        dawnCommandBufferBuilder cmdBufBuilder = dawnDeviceCreateCommandBufferBuilder(device);
        cmdBufs[i] = dawnCommandBufferBuilderGetResult(cmdBufBuilder);

        dawnCommandBufferBuilder apiCmdBufBuilder = api.GetNewCommandBufferBuilder();
        EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
            .InSequence(s)
            .WillOnce(Return(apiCmdBufBuilder));

        apiCmdBufs[i] = api.GetNewCommandBuffer();
        EXPECT_CALL(api, CommandBufferBuilderGetResult(apiCmdBufBuilder))
            .WillOnce(Return(apiCmdBufs[i]));
    }

    // Create queue
    dawnQueue queue = dawnDeviceCreateQueue(device);
    dawnQueue apiQueue = api.GetNewQueue();
    EXPECT_CALL(api, DeviceCreateQueue(apiDevice))
        .WillOnce(Return(apiQueue));

    // Submit command buffer and check we got a call with both API-side command buffers
    dawnQueueSubmit(queue, 2, cmdBufs);

    EXPECT_CALL(api, QueueSubmit(apiQueue, 2, MatchesLambda([=](const dawnCommandBuffer* cmdBufs) -> bool {
        return cmdBufs[0] == apiCmdBufs[0] && cmdBufs[1] == apiCmdBufs[1];
    })));

    FlushClient();
}

// Test that the wire is able to send structures that contain pure values (non-objects)
TEST_F(WireTests, StructureOfValuesArgument) {
    dawnSamplerDescriptor descriptor;
    descriptor.nextInChain = nullptr;
    descriptor.magFilter = DAWN_FILTER_MODE_LINEAR;
    descriptor.minFilter = DAWN_FILTER_MODE_NEAREST;
    descriptor.mipmapFilter = DAWN_FILTER_MODE_LINEAR;
    descriptor.addressModeU = DAWN_ADDRESS_MODE_CLAMP_TO_EDGE;
    descriptor.addressModeV = DAWN_ADDRESS_MODE_REPEAT;
    descriptor.addressModeW = DAWN_ADDRESS_MODE_MIRRORED_REPEAT;

    dawnDeviceCreateSampler(device, &descriptor);
    EXPECT_CALL(api, DeviceCreateSampler(apiDevice, MatchesLambda([](const dawnSamplerDescriptor* desc) -> bool {
        return desc->nextInChain == nullptr &&
            desc->magFilter == DAWN_FILTER_MODE_LINEAR &&
            desc->minFilter == DAWN_FILTER_MODE_NEAREST &&
            desc->mipmapFilter == DAWN_FILTER_MODE_LINEAR &&
            desc->addressModeU == DAWN_ADDRESS_MODE_CLAMP_TO_EDGE &&
            desc->addressModeV == DAWN_ADDRESS_MODE_REPEAT &&
            desc->addressModeW == DAWN_ADDRESS_MODE_MIRRORED_REPEAT;
    })))
        .WillOnce(Return(nullptr));

    FlushClient();
}

// Test that the wire is able to send structures that contain objects
TEST_F(WireTests, StructureOfObjectArrayArgument) {
    dawnBindGroupLayoutDescriptor bglDescriptor;
    bglDescriptor.numBindings = 0;
    bglDescriptor.bindings = nullptr;

    dawnBindGroupLayout bgl = dawnDeviceCreateBindGroupLayout(device, &bglDescriptor);
    dawnBindGroupLayout apiBgl = api.GetNewBindGroupLayout();
    EXPECT_CALL(api, DeviceCreateBindGroupLayout(apiDevice, _)).WillOnce(Return(apiBgl));

    dawnPipelineLayoutDescriptor descriptor;
    descriptor.nextInChain = nullptr;
    descriptor.numBindGroupLayouts = 1;
    descriptor.bindGroupLayouts = &bgl;

    dawnDeviceCreatePipelineLayout(device, &descriptor);
    EXPECT_CALL(api, DeviceCreatePipelineLayout(apiDevice, MatchesLambda([apiBgl](const dawnPipelineLayoutDescriptor* desc) -> bool {
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
    dawnBindGroupBinding bindings[NUM_BINDINGS]{
        {0, DAWN_SHADER_STAGE_BIT_VERTEX, DAWN_BINDING_TYPE_SAMPLER},
        {1, DAWN_SHADER_STAGE_BIT_VERTEX, DAWN_BINDING_TYPE_SAMPLED_TEXTURE},
        {2,
         static_cast<dawnShaderStageBit>(DAWN_SHADER_STAGE_BIT_VERTEX |
                                        DAWN_SHADER_STAGE_BIT_FRAGMENT),
         DAWN_BINDING_TYPE_UNIFORM_BUFFER},
    };
    dawnBindGroupLayoutDescriptor bglDescriptor;
    bglDescriptor.numBindings = NUM_BINDINGS;
    bglDescriptor.bindings = bindings;

    dawnDeviceCreateBindGroupLayout(device, &bglDescriptor);
    dawnBindGroupLayout apiBgl = api.GetNewBindGroupLayout();
    EXPECT_CALL(
        api,
        DeviceCreateBindGroupLayout(
            apiDevice, MatchesLambda([bindings](const dawnBindGroupLayoutDescriptor* desc) -> bool {
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

// Test passing nullptr instead of objects - object as value version
TEST_F(WireTests, DISABLED_NullptrAsValue) {
    dawnCommandBufferBuilder builder = dawnDeviceCreateCommandBufferBuilder(device);
    dawnComputePassEncoder pass = dawnCommandBufferBuilderBeginComputePass(builder);
    dawnComputePassEncoderSetComputePipeline(pass, nullptr);

    dawnCommandBufferBuilder apiBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiBuilder));

    dawnComputePassEncoder apiPass = api.GetNewComputePassEncoder();
    EXPECT_CALL(api, CommandBufferBuilderBeginComputePass(apiBuilder))
        .WillOnce(Return(apiPass));

    EXPECT_CALL(api, ComputePassEncoderSetComputePipeline(apiPass, nullptr))
        .Times(1);

    FlushClient();
}

// Test passing nullptr instead of objects - array of objects version
TEST_F(WireTests, DISABLED_NullptrInArray) {
    dawnBindGroupLayout nullBGL = nullptr;

    dawnPipelineLayoutDescriptor descriptor;
    descriptor.nextInChain = nullptr;
    descriptor.numBindGroupLayouts = 1;
    descriptor.bindGroupLayouts = &nullBGL;

    dawnDeviceCreatePipelineLayout(device, &descriptor);
    EXPECT_CALL(api, DeviceCreatePipelineLayout(apiDevice, MatchesLambda([](const dawnPipelineLayoutDescriptor* desc) -> bool {
        return desc->nextInChain == nullptr &&
            desc->numBindGroupLayouts == 1 &&
            desc->bindGroupLayouts[0] == nullptr;
    })))
        .WillOnce(Return(nullptr));

    FlushClient();
}

// Test that the server doesn't forward calls to error objects or with error objects
// Also test that when GetResult is called on an error builder, the error callback is fired
// TODO(cwallez@chromium.org): This test is disabled because the introduction of encoders breaks
// the assumptions of the "builder error" handling that a builder is self-contained. We need to
// revisit this once the new error handling is in place.
TEST_F(WireTests, DISABLED_CallsSkippedAfterBuilderError) {
    dawnCommandBufferBuilder cmdBufBuilder = dawnDeviceCreateCommandBufferBuilder(device);
    dawnCommandBufferBuilderSetErrorCallback(cmdBufBuilder, ToMockBuilderErrorCallback, 1, 2);

    dawnRenderPassEncoder pass = dawnCommandBufferBuilderBeginRenderPass(cmdBufBuilder, nullptr);

    dawnBufferBuilder bufferBuilder = dawnDeviceCreateBufferBuilderForTesting(device);
    dawnBufferBuilderSetErrorCallback(bufferBuilder, ToMockBuilderErrorCallback, 3, 4);
    dawnBuffer buffer = dawnBufferBuilderGetResult(bufferBuilder); // Hey look an error!

    // These calls will be skipped because of the error
    dawnBufferSetSubData(buffer, 0, 0, nullptr);
    dawnRenderPassEncoderSetIndexBuffer(pass, buffer, 0);
    dawnRenderPassEncoderEndPass(pass);
    dawnCommandBufferBuilderGetResult(cmdBufBuilder);

    dawnCommandBufferBuilder apiCmdBufBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiCmdBufBuilder));

    dawnRenderPassEncoder apiPass = api.GetNewRenderPassEncoder();
    EXPECT_CALL(api, CommandBufferBuilderBeginRenderPass(apiCmdBufBuilder, _))
        .WillOnce(Return(apiPass));

    dawnBufferBuilder apiBufferBuilder = api.GetNewBufferBuilder();
    EXPECT_CALL(api, DeviceCreateBufferBuilderForTesting(apiDevice))
        .WillOnce(Return(apiBufferBuilder));

    // Hey look an error!
    EXPECT_CALL(api, BufferBuilderGetResult(apiBufferBuilder))
        .WillOnce(InvokeWithoutArgs([&]() -> dawnBuffer {
            api.CallBuilderErrorCallback(apiBufferBuilder, DAWN_BUILDER_ERROR_STATUS_ERROR, "Error");
            return nullptr;
        }));

    EXPECT_CALL(api, BufferSetSubData(_, _, _, _)).Times(0);
    EXPECT_CALL(api, RenderPassEncoderSetIndexBuffer(_, _, _)).Times(0);
    EXPECT_CALL(api, CommandBufferBuilderGetResult(_)).Times(0);

    FlushClient();

    EXPECT_CALL(*mockBuilderErrorCallback, Call(DAWN_BUILDER_ERROR_STATUS_ERROR, _, 1, 2)).Times(1);
    EXPECT_CALL(*mockBuilderErrorCallback, Call(DAWN_BUILDER_ERROR_STATUS_ERROR, _, 3, 4)).Times(1);

    FlushServer();
}

// Test that we get a success builder error status when no error happens
TEST_F(WireTests, SuccessCallbackOnBuilderSuccess) {
    dawnBufferBuilder bufferBuilder = dawnDeviceCreateBufferBuilderForTesting(device);
    dawnBufferBuilderSetErrorCallback(bufferBuilder, ToMockBuilderErrorCallback, 1, 2);
    dawnBufferBuilderGetResult(bufferBuilder);

    dawnBufferBuilder apiBufferBuilder = api.GetNewBufferBuilder();
    EXPECT_CALL(api, DeviceCreateBufferBuilderForTesting(apiDevice))
        .WillOnce(Return(apiBufferBuilder));

    dawnBuffer apiBuffer = api.GetNewBuffer();
    EXPECT_CALL(api, BufferBuilderGetResult(apiBufferBuilder))
        .WillOnce(InvokeWithoutArgs([&]() -> dawnBuffer {
            api.CallBuilderErrorCallback(apiBufferBuilder, DAWN_BUILDER_ERROR_STATUS_SUCCESS, "I like cheese");
            return apiBuffer;
        }));

    FlushClient();

    EXPECT_CALL(*mockBuilderErrorCallback, Call(DAWN_BUILDER_ERROR_STATUS_SUCCESS, _ , 1 ,2));

    FlushServer();
}

// Test that the client calls the builder callback with unknown when it HAS to fire the callback but can't
// know the status yet.
TEST_F(WireTests, UnknownBuilderErrorStatusCallback) {
    // The builder is destroyed before the object is built
    {
        dawnBufferBuilder bufferBuilder = dawnDeviceCreateBufferBuilderForTesting(device);
        dawnBufferBuilderSetErrorCallback(bufferBuilder, ToMockBuilderErrorCallback, 1, 2);

        EXPECT_CALL(*mockBuilderErrorCallback, Call(DAWN_BUILDER_ERROR_STATUS_UNKNOWN, _ , 1 ,2)).Times(1);

        dawnBufferBuilderRelease(bufferBuilder);
    }

    // If the builder has been consumed, it doesn't fire the callback with unknown
    {
        dawnBufferBuilder bufferBuilder = dawnDeviceCreateBufferBuilderForTesting(device);
        dawnBufferBuilderSetErrorCallback(bufferBuilder, ToMockBuilderErrorCallback, 3, 4);
        dawnBufferBuilderGetResult(bufferBuilder);

        EXPECT_CALL(*mockBuilderErrorCallback, Call(DAWN_BUILDER_ERROR_STATUS_UNKNOWN, _ , 3, 4)).Times(0);

        dawnBufferBuilderRelease(bufferBuilder);
    }

    // If the builder has been consumed, and the object is destroyed before the result comes from the server,
    // then the callback is fired with unknown
    {
        dawnBufferBuilder bufferBuilder = dawnDeviceCreateBufferBuilderForTesting(device);
        dawnBufferBuilderSetErrorCallback(bufferBuilder, ToMockBuilderErrorCallback, 5, 6);
        dawnBuffer buffer = dawnBufferBuilderGetResult(bufferBuilder);

        EXPECT_CALL(*mockBuilderErrorCallback, Call(DAWN_BUILDER_ERROR_STATUS_UNKNOWN, _ , 5, 6)).Times(1);

        dawnBufferRelease(buffer);
    }
}

// Test that a builder success status doesn't get forwarded to the device
TEST_F(WireTests, SuccessCallbackNotForwardedToDevice) {
    dawnDeviceSetErrorCallback(device, ToMockDeviceErrorCallback, 0);

    dawnBufferBuilder bufferBuilder = dawnDeviceCreateBufferBuilderForTesting(device);
    dawnBufferBuilderGetResult(bufferBuilder);

    dawnBufferBuilder apiBufferBuilder = api.GetNewBufferBuilder();
    EXPECT_CALL(api, DeviceCreateBufferBuilderForTesting(apiDevice))
        .WillOnce(Return(apiBufferBuilder));

    dawnBuffer apiBuffer = api.GetNewBuffer();
    EXPECT_CALL(api, BufferBuilderGetResult(apiBufferBuilder))
        .WillOnce(InvokeWithoutArgs([&]() -> dawnBuffer {
            api.CallBuilderErrorCallback(apiBufferBuilder, DAWN_BUILDER_ERROR_STATUS_SUCCESS, "I like cheese");
            return apiBuffer;
        }));

    FlushClient();
    FlushServer();
}

// Test that a builder error status gets forwarded to the device
TEST_F(WireTests, ErrorCallbackForwardedToDevice) {
    uint64_t userdata = 30495;
    dawnDeviceSetErrorCallback(device, ToMockDeviceErrorCallback, userdata);

    dawnBufferBuilder bufferBuilder = dawnDeviceCreateBufferBuilderForTesting(device);
    dawnBufferBuilderGetResult(bufferBuilder);

    dawnBufferBuilder apiBufferBuilder = api.GetNewBufferBuilder();
    EXPECT_CALL(api, DeviceCreateBufferBuilderForTesting(apiDevice))
        .WillOnce(Return(apiBufferBuilder));

    EXPECT_CALL(api, BufferBuilderGetResult(apiBufferBuilder))
        .WillOnce(InvokeWithoutArgs([&]() -> dawnBuffer {
            api.CallBuilderErrorCallback(apiBufferBuilder, DAWN_BUILDER_ERROR_STATUS_ERROR, "Error :(");
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
    dawnDeviceSetErrorCallback(device, ToMockDeviceErrorCallback, userdata);

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
    dawnBufferBuilder bufferBuilder = dawnDeviceCreateBufferBuilderForTesting(device);

    dawnBufferBuilder apiBufferBuilder = api.GetNewBufferBuilder();
    EXPECT_CALL(api, DeviceCreateBufferBuilderForTesting(apiDevice))
        .WillOnce(Return(apiBufferBuilder));

    EXPECT_CALL(api, OnBuilderSetErrorCallback(apiBufferBuilder, _, _, _))
        .Times(1);

    FlushClient();

    // Setting the callback on the client side doesn't do anything on the server side
    dawnBufferBuilderSetErrorCallback(bufferBuilder, ToMockBuilderErrorCallback, userdata1, userdata2);
    FlushClient();

    // Create an object so that it is a valid case to call the error callback
    dawnBufferBuilderGetResult(bufferBuilder);

    dawnBuffer apiBuffer = api.GetNewBuffer();
    EXPECT_CALL(api, BufferBuilderGetResult(apiBufferBuilder))
        .WillOnce(InvokeWithoutArgs([&]() -> dawnBuffer {
            api.CallBuilderErrorCallback(apiBufferBuilder, DAWN_BUILDER_ERROR_STATUS_SUCCESS, "Success!");
            return apiBuffer;
        }));

    FlushClient();

    // The error callback gets called on the client side
    EXPECT_CALL(*mockBuilderErrorCallback, Call(DAWN_BUILDER_ERROR_STATUS_SUCCESS, StrEq("Success!"), userdata1, userdata2))
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
                dawnBufferDescriptor descriptor;
                descriptor.nextInChain = nullptr;

                apiBuffer = api.GetNewBuffer();
                buffer = dawnDeviceCreateBuffer(device, &descriptor);

                EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _))
                    .WillOnce(Return(apiBuffer))
                    .RetiresOnSaturation();
                FlushClient();
            }
            {
                dawnBufferDescriptor descriptor;
                descriptor.nextInChain = nullptr;

                errorBuffer = dawnDeviceCreateBuffer(device, &descriptor);

                EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _))
                    .WillOnce(Return(nullptr))
                    .RetiresOnSaturation();
                FlushClient();
            }
        }

    protected:
        // A successfully created buffer
        dawnBuffer buffer;
        dawnBuffer apiBuffer;

        // An buffer that wasn't created on the server side
        dawnBuffer errorBuffer;
};

// MapRead-specific tests

// Check mapping for reading a succesfully created buffer
TEST_F(WireBufferMappingTests, MappingForReadSuccessBuffer) {
    dawnCallbackUserdata userdata = 8653;
    dawnBufferMapReadAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);
    
    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapReadAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapReadCallback(apiBuffer, DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, &bufferContent);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapReadCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Pointee(Eq(bufferContent)), userdata))
        .Times(1);

    FlushServer();

    dawnBufferUnmap(buffer);
    EXPECT_CALL(api, BufferUnmap(apiBuffer))
        .Times(1);

    FlushClient();
}

// Check that things work correctly when a validation error happens when mapping the buffer for reading
TEST_F(WireBufferMappingTests, ErrorWhileMappingForRead) {
    dawnCallbackUserdata userdata = 8654;
    dawnBufferMapReadAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);
    
    EXPECT_CALL(api, OnBufferMapReadAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapReadCallback(apiBuffer, DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapReadCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata))
        .Times(1);

    FlushServer();
}

// Check mapping for reading a buffer that didn't get created on the server side
TEST_F(WireBufferMappingTests, MappingForReadErrorBuffer) {
    dawnCallbackUserdata userdata = 8655;
    dawnBufferMapReadAsync(errorBuffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);

    FlushClient();

    EXPECT_CALL(*mockBufferMapReadCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata))
        .Times(1);

    FlushServer();

    dawnBufferUnmap(errorBuffer);

    FlushClient();
}

// Check that the map read callback is called with UNKNOWN when the buffer is destroyed before the request is finished
TEST_F(WireBufferMappingTests, DestroyBeforeReadRequestEnd) {
    dawnCallbackUserdata userdata = 8656;
    dawnBufferMapReadAsync(errorBuffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);

    EXPECT_CALL(*mockBufferMapReadCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr, userdata))
        .Times(1);

    dawnBufferRelease(errorBuffer);
}

// Check the map read callback is called with UNKNOWN when the map request would have worked, but Unmap was called
TEST_F(WireBufferMappingTests, UnmapCalledTooEarlyForRead) {
    dawnCallbackUserdata userdata = 8657;
    dawnBufferMapReadAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapReadAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapReadCallback(apiBuffer, DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, &bufferContent);
        }));

    FlushClient();

    // Oh no! We are calling Unmap too early!
    EXPECT_CALL(*mockBufferMapReadCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr, userdata))
        .Times(1);
    dawnBufferUnmap(buffer);

    // The callback shouldn't get called, even when the request succeeded on the server side
    FlushServer();
}

// Check that an error map read callback gets nullptr while a buffer is already mapped
TEST_F(WireBufferMappingTests, MappingForReadingErrorWhileAlreadyMappedGetsNullptr) {
    // Successful map
    dawnCallbackUserdata userdata = 34098;
    dawnBufferMapReadAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapReadAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapReadCallback(apiBuffer, DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, &bufferContent);
        }))
        .RetiresOnSaturation();

    FlushClient();

    EXPECT_CALL(*mockBufferMapReadCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Pointee(Eq(bufferContent)), userdata))
        .Times(1);

    FlushServer();

    // Map failure while the buffer is already mapped
    userdata ++;
    dawnBufferMapReadAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);
    EXPECT_CALL(api, OnBufferMapReadAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapReadCallback(apiBuffer, DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapReadCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata))
        .Times(1);

    FlushServer();
}

// Test that the MapReadCallback isn't fired twice when unmap() is called inside the callback
TEST_F(WireBufferMappingTests, UnmapInsideMapReadCallback) {
    dawnCallbackUserdata userdata = 2039;
    dawnBufferMapReadAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapReadAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapReadCallback(apiBuffer, DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, &bufferContent);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapReadCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Pointee(Eq(bufferContent)), userdata))
        .WillOnce(InvokeWithoutArgs([&]() {
            dawnBufferUnmap(buffer);
        }));

    FlushServer();

    EXPECT_CALL(api, BufferUnmap(apiBuffer))
        .Times(1);

    FlushClient();
}

// Test that the MapReadCallback isn't fired twice the buffer external refcount reaches 0 in the callback
TEST_F(WireBufferMappingTests, DestroyInsideMapReadCallback) {
    dawnCallbackUserdata userdata = 2039;
    dawnBufferMapReadAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapReadCallback, userdata);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapReadAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapReadCallback(apiBuffer, DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, &bufferContent);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapReadCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Pointee(Eq(bufferContent)), userdata))
        .WillOnce(InvokeWithoutArgs([&]() {
            dawnBufferRelease(buffer);
        }));

    FlushServer();

    EXPECT_CALL(api, BufferRelease(apiBuffer))
        .Times(1);

    FlushClient();
}

// MapWrite-specific tests

// Check mapping for writing a succesfully created buffer
TEST_F(WireBufferMappingTests, MappingForWriteSuccessBuffer) {
    dawnCallbackUserdata userdata = 8653;
    dawnBufferMapWriteAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapWriteCallback, userdata);

    uint32_t serverBufferContent = 31337;
    uint32_t updatedContent = 4242;
    uint32_t zero = 0;

    EXPECT_CALL(api, OnBufferMapWriteAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapWriteCallback(apiBuffer, DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, &serverBufferContent);
        }));

    FlushClient();

    // The map write callback always gets a buffer full of zeroes.
    EXPECT_CALL(*mockBufferMapWriteCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Pointee(Eq(zero)), userdata))
        .Times(1);

    FlushServer();

    // Write something to the mapped pointer
    *lastMapWritePointer = updatedContent;

    dawnBufferUnmap(buffer);
    EXPECT_CALL(api, BufferUnmap(apiBuffer))
        .Times(1);

    FlushClient();

    // After the buffer is unmapped, the content of the buffer is updated on the server
    ASSERT_EQ(serverBufferContent, updatedContent);
}

// Check that things work correctly when a validation error happens when mapping the buffer for writing
TEST_F(WireBufferMappingTests, ErrorWhileMappingForWrite) {
    dawnCallbackUserdata userdata = 8654;
    dawnBufferMapWriteAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapWriteCallback, userdata);

    EXPECT_CALL(api, OnBufferMapWriteAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapWriteCallback(apiBuffer, DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapWriteCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata))
        .Times(1);

    FlushServer();
}

// Check mapping for writing a buffer that didn't get created on the server side
TEST_F(WireBufferMappingTests, MappingForWriteErrorBuffer) {
    dawnCallbackUserdata userdata = 8655;
    dawnBufferMapWriteAsync(errorBuffer, 40, sizeof(uint32_t), ToMockBufferMapWriteCallback, userdata);

    FlushClient();

    EXPECT_CALL(*mockBufferMapWriteCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata))
        .Times(1);

    FlushServer();

    dawnBufferUnmap(errorBuffer);

    FlushClient();
}

// Check that the map write callback is called with UNKNOWN when the buffer is destroyed before the request is finished
TEST_F(WireBufferMappingTests, DestroyBeforeWriteRequestEnd) {
    dawnCallbackUserdata userdata = 8656;
    dawnBufferMapWriteAsync(errorBuffer, 40, sizeof(uint32_t), ToMockBufferMapWriteCallback, userdata);

    EXPECT_CALL(*mockBufferMapWriteCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr, userdata))
        .Times(1);

    dawnBufferRelease(errorBuffer);
}

// Check the map read callback is called with UNKNOWN when the map request would have worked, but Unmap was called
TEST_F(WireBufferMappingTests, UnmapCalledTooEarlyForWrite) {
    dawnCallbackUserdata userdata = 8657;
    dawnBufferMapWriteAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapWriteCallback, userdata);

    uint32_t bufferContent = 31337;
    EXPECT_CALL(api, OnBufferMapWriteAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapWriteCallback(apiBuffer, DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, &bufferContent);
        }));

    FlushClient();

    // Oh no! We are calling Unmap too early!
    EXPECT_CALL(*mockBufferMapWriteCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_UNKNOWN, nullptr, userdata))
        .Times(1);
    dawnBufferUnmap(buffer);

    // The callback shouldn't get called, even when the request succeeded on the server side
    FlushServer();
}

// Check that an error map read callback gets nullptr while a buffer is already mapped
TEST_F(WireBufferMappingTests, MappingForWritingErrorWhileAlreadyMappedGetsNullptr) {
    // Successful map
    dawnCallbackUserdata userdata = 34098;
    dawnBufferMapWriteAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapWriteCallback, userdata);

    uint32_t bufferContent = 31337;
    uint32_t zero = 0;
    EXPECT_CALL(api, OnBufferMapWriteAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapWriteCallback(apiBuffer, DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, &bufferContent);
        }))
        .RetiresOnSaturation();

    FlushClient();

    EXPECT_CALL(*mockBufferMapWriteCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Pointee(Eq(zero)), userdata))
        .Times(1);

    FlushServer();

    // Map failure while the buffer is already mapped
    userdata ++;
    dawnBufferMapWriteAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapWriteCallback, userdata);
    EXPECT_CALL(api, OnBufferMapWriteAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapWriteCallback(apiBuffer, DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapWriteCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_ERROR, nullptr, userdata))
        .Times(1);

    FlushServer();
}

// Test that the MapWriteCallback isn't fired twice when unmap() is called inside the callback
TEST_F(WireBufferMappingTests, UnmapInsideMapWriteCallback) {
    dawnCallbackUserdata userdata = 2039;
    dawnBufferMapWriteAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapWriteCallback, userdata);

    uint32_t bufferContent = 31337;
    uint32_t zero = 0;
    EXPECT_CALL(api, OnBufferMapWriteAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapWriteCallback(apiBuffer, DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, &bufferContent);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapWriteCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Pointee(Eq(zero)), userdata))
        .WillOnce(InvokeWithoutArgs([&]() {
            dawnBufferUnmap(buffer);
        }));

    FlushServer();

    EXPECT_CALL(api, BufferUnmap(apiBuffer))
        .Times(1);

    FlushClient();
}

// Test that the MapWriteCallback isn't fired twice the buffer external refcount reaches 0 in the callback
TEST_F(WireBufferMappingTests, DestroyInsideMapWriteCallback) {
    dawnCallbackUserdata userdata = 2039;
    dawnBufferMapWriteAsync(buffer, 40, sizeof(uint32_t), ToMockBufferMapWriteCallback, userdata);

    uint32_t bufferContent = 31337;
    uint32_t zero = 0;
    EXPECT_CALL(api, OnBufferMapWriteAsyncCallback(apiBuffer, 40, sizeof(uint32_t), _, _))
        .WillOnce(InvokeWithoutArgs([&]() {
            api.CallMapWriteCallback(apiBuffer, DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, &bufferContent);
        }));

    FlushClient();

    EXPECT_CALL(*mockBufferMapWriteCallback, Call(DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, Pointee(Eq(zero)), userdata))
        .WillOnce(InvokeWithoutArgs([&]() {
            dawnBufferRelease(buffer);
        }));

    FlushServer();

    EXPECT_CALL(api, BufferRelease(apiBuffer))
        .Times(1);

    FlushClient();
}
