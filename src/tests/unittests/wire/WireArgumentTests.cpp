// Copyright 2019 The Dawn Authors
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

#include "common/Constants.h"

using namespace testing;
using namespace dawn_wire;

class WireArgumentTests : public WireTest {
  public:
    WireArgumentTests() : WireTest(true) {
    }
    ~WireArgumentTests() override = default;
};

// Test that the wire is able to send numerical values
TEST_F(WireArgumentTests, ValueArgument) {
    dawnCommandEncoder encoder = dawnDeviceCreateCommandEncoder(device);
    dawnComputePassEncoder pass = dawnCommandEncoderBeginComputePass(encoder);
    dawnComputePassEncoderDispatch(pass, 1, 2, 3);

    dawnCommandEncoder apiEncoder = api.GetNewCommandEncoder();
    EXPECT_CALL(api, DeviceCreateCommandEncoder(apiDevice)).WillOnce(Return(apiEncoder));

    dawnComputePassEncoder apiPass = api.GetNewComputePassEncoder();
    EXPECT_CALL(api, CommandEncoderBeginComputePass(apiEncoder)).WillOnce(Return(apiPass));

    EXPECT_CALL(api, ComputePassEncoderDispatch(apiPass, 1, 2, 3)).Times(1);

    EXPECT_CALL(api, CommandEncoderRelease(apiEncoder));
    EXPECT_CALL(api, ComputePassEncoderRelease(apiPass));
    FlushClient();
}

// Test that the wire is able to send arrays of numerical values
static constexpr uint32_t testPushConstantValues[4] = {0, 42, 0xDEADBEEFu, 0xFFFFFFFFu};

bool CheckPushConstantValues(const uint32_t* values) {
    for (int i = 0; i < 4; ++i) {
        if (values[i] != testPushConstantValues[i]) {
            return false;
        }
    }
    return true;
}

TEST_F(WireArgumentTests, ValueArrayArgument) {
    dawnCommandEncoder encoder = dawnDeviceCreateCommandEncoder(device);
    dawnComputePassEncoder pass = dawnCommandEncoderBeginComputePass(encoder);
    dawnComputePassEncoderSetPushConstants(pass, DAWN_SHADER_STAGE_BIT_VERTEX, 0, 4,
                                           testPushConstantValues);

    dawnCommandEncoder apiEncoder = api.GetNewCommandEncoder();
    EXPECT_CALL(api, DeviceCreateCommandEncoder(apiDevice)).WillOnce(Return(apiEncoder));

    dawnComputePassEncoder apiPass = api.GetNewComputePassEncoder();
    EXPECT_CALL(api, CommandEncoderBeginComputePass(apiEncoder)).WillOnce(Return(apiPass));

    EXPECT_CALL(api,
                ComputePassEncoderSetPushConstants(apiPass, DAWN_SHADER_STAGE_BIT_VERTEX, 0, 4,
                                                   ResultOf(CheckPushConstantValues, Eq(true))));
    EXPECT_CALL(api, CommandEncoderRelease(apiEncoder));
    EXPECT_CALL(api, ComputePassEncoderRelease(apiPass));

    FlushClient();
}

// Test that the wire is able to send C strings
TEST_F(WireArgumentTests, CStringArgument) {
    // Create shader module
    dawnShaderModuleDescriptor vertexDescriptor;
    vertexDescriptor.nextInChain = nullptr;
    vertexDescriptor.codeSize = 0;
    dawnShaderModule vsModule = dawnDeviceCreateShaderModule(device, &vertexDescriptor);
    dawnShaderModule apiVsModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiVsModule));

    // Create the color state descriptor
    dawnBlendDescriptor blendDescriptor;
    blendDescriptor.operation = DAWN_BLEND_OPERATION_ADD;
    blendDescriptor.srcFactor = DAWN_BLEND_FACTOR_ONE;
    blendDescriptor.dstFactor = DAWN_BLEND_FACTOR_ONE;
    dawnColorStateDescriptor colorStateDescriptor;
    colorStateDescriptor.nextInChain = nullptr;
    colorStateDescriptor.format = DAWN_TEXTURE_FORMAT_R8_G8_B8_A8_UNORM;
    colorStateDescriptor.alphaBlend = blendDescriptor;
    colorStateDescriptor.colorBlend = blendDescriptor;
    colorStateDescriptor.colorWriteMask = DAWN_COLOR_WRITE_MASK_ALL;

    // Create the input state
    dawnInputStateBuilder inputStateBuilder = dawnDeviceCreateInputStateBuilder(device);
    dawnInputStateBuilder apiInputStateBuilder = api.GetNewInputStateBuilder();
    EXPECT_CALL(api, DeviceCreateInputStateBuilder(apiDevice))
        .WillOnce(Return(apiInputStateBuilder));

    dawnInputState inputState = dawnInputStateBuilderGetResult(inputStateBuilder);
    dawnInputState apiInputState = api.GetNewInputState();
    EXPECT_CALL(api, InputStateBuilderGetResult(apiInputStateBuilder))
        .WillOnce(Return(apiInputState));

    // Create the depth-stencil state
    dawnStencilStateFaceDescriptor stencilFace;
    stencilFace.compare = DAWN_COMPARE_FUNCTION_ALWAYS;
    stencilFace.failOp = DAWN_STENCIL_OPERATION_KEEP;
    stencilFace.depthFailOp = DAWN_STENCIL_OPERATION_KEEP;
    stencilFace.passOp = DAWN_STENCIL_OPERATION_KEEP;

    dawnDepthStencilStateDescriptor depthStencilState;
    depthStencilState.nextInChain = nullptr;
    depthStencilState.format = DAWN_TEXTURE_FORMAT_D32_FLOAT_S8_UINT;
    depthStencilState.depthWriteEnabled = false;
    depthStencilState.depthCompare = DAWN_COMPARE_FUNCTION_ALWAYS;
    depthStencilState.stencilBack = stencilFace;
    depthStencilState.stencilFront = stencilFace;
    depthStencilState.stencilReadMask = 0xff;
    depthStencilState.stencilWriteMask = 0xff;

    // Create the pipeline layout
    dawnPipelineLayoutDescriptor layoutDescriptor;
    layoutDescriptor.nextInChain = nullptr;
    layoutDescriptor.numBindGroupLayouts = 0;
    layoutDescriptor.bindGroupLayouts = nullptr;
    dawnPipelineLayout layout = dawnDeviceCreatePipelineLayout(device, &layoutDescriptor);
    dawnPipelineLayout apiLayout = api.GetNewPipelineLayout();
    EXPECT_CALL(api, DeviceCreatePipelineLayout(apiDevice, _)).WillOnce(Return(apiLayout));

    // Create pipeline
    dawnRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.nextInChain = nullptr;

    dawnPipelineStageDescriptor vertexStage;
    vertexStage.nextInChain = nullptr;
    vertexStage.module = vsModule;
    vertexStage.entryPoint = "main";
    pipelineDescriptor.vertexStage = &vertexStage;

    dawnPipelineStageDescriptor fragmentStage;
    fragmentStage.nextInChain = nullptr;
    fragmentStage.module = vsModule;
    fragmentStage.entryPoint = "main";
    pipelineDescriptor.fragmentStage = &fragmentStage;

    pipelineDescriptor.numColorStates = 1;
    pipelineDescriptor.colorStates = &colorStateDescriptor;

    pipelineDescriptor.sampleCount = 1;
    pipelineDescriptor.layout = layout;
    pipelineDescriptor.inputState = inputState;
    pipelineDescriptor.indexFormat = DAWN_INDEX_FORMAT_UINT32;
    pipelineDescriptor.primitiveTopology = DAWN_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineDescriptor.depthStencilState = &depthStencilState;

    dawnDeviceCreateRenderPipeline(device, &pipelineDescriptor);
    EXPECT_CALL(api,
                DeviceCreateRenderPipeline(
                    apiDevice, MatchesLambda([](const dawnRenderPipelineDescriptor* desc) -> bool {
                        return desc->vertexStage->entryPoint == std::string("main");
                    })))
        .WillOnce(Return(nullptr));
    EXPECT_CALL(api, ShaderModuleRelease(apiVsModule));
    EXPECT_CALL(api, InputStateBuilderRelease(apiInputStateBuilder));
    EXPECT_CALL(api, InputStateRelease(apiInputState));
    EXPECT_CALL(api, PipelineLayoutRelease(apiLayout));

    FlushClient();
}

// Test that the wire is able to send objects as value arguments
TEST_F(WireArgumentTests, ObjectAsValueArgument) {
    // Create a RenderPassDescriptor
    dawnRenderPassDescriptorBuilder renderPassBuilder =
        dawnDeviceCreateRenderPassDescriptorBuilder(device);
    dawnRenderPassDescriptor renderPass =
        dawnRenderPassDescriptorBuilderGetResult(renderPassBuilder);

    dawnRenderPassDescriptorBuilder apiRenderPassBuilder = api.GetNewRenderPassDescriptorBuilder();
    EXPECT_CALL(api, DeviceCreateRenderPassDescriptorBuilder(apiDevice))
        .WillOnce(Return(apiRenderPassBuilder));
    dawnRenderPassDescriptor apiRenderPass = api.GetNewRenderPassDescriptor();
    EXPECT_CALL(api, RenderPassDescriptorBuilderGetResult(apiRenderPassBuilder))
        .WillOnce(Return(apiRenderPass));

    // Create command buffer encoder, setting render pass descriptor
    dawnCommandEncoder cmdBufEncoder = dawnDeviceCreateCommandEncoder(device);
    dawnCommandEncoderBeginRenderPass(cmdBufEncoder, renderPass);

    dawnCommandEncoder apiCmdBufEncoder = api.GetNewCommandEncoder();
    EXPECT_CALL(api, DeviceCreateCommandEncoder(apiDevice))
        .WillOnce(Return(apiCmdBufEncoder));

    EXPECT_CALL(api, CommandEncoderBeginRenderPass(apiCmdBufEncoder, apiRenderPass)).Times(1);

    EXPECT_CALL(api, CommandEncoderRelease(apiCmdBufEncoder));
    EXPECT_CALL(api, RenderPassDescriptorBuilderRelease(apiRenderPassBuilder));
    EXPECT_CALL(api, RenderPassDescriptorRelease(apiRenderPass));
    FlushClient();
}

// Test that the wire is able to send array of objects
TEST_F(WireArgumentTests, ObjectsAsPointerArgument) {
    dawnCommandBuffer cmdBufs[2];
    dawnCommandBuffer apiCmdBufs[2];

    // Create two command buffers we need to use a GMock sequence otherwise the order of the
    // CreateCommandEncoder might be swapped since they are equivalent in term of matchers
    Sequence s;
    for (int i = 0; i < 2; ++i) {
        dawnCommandEncoder cmdBufEncoder = dawnDeviceCreateCommandEncoder(device);
        cmdBufs[i] = dawnCommandEncoderFinish(cmdBufEncoder);

        dawnCommandEncoder apiCmdBufEncoder = api.GetNewCommandEncoder();
        EXPECT_CALL(api, DeviceCreateCommandEncoder(apiDevice))
            .InSequence(s)
            .WillOnce(Return(apiCmdBufEncoder));

        apiCmdBufs[i] = api.GetNewCommandBuffer();
        EXPECT_CALL(api, CommandEncoderFinish(apiCmdBufEncoder))
            .WillOnce(Return(apiCmdBufs[i]));
        EXPECT_CALL(api, CommandEncoderRelease(apiCmdBufEncoder));
        EXPECT_CALL(api, CommandBufferRelease(apiCmdBufs[i]));
    }

    // Create queue
    dawnQueue queue = dawnDeviceCreateQueue(device);
    dawnQueue apiQueue = api.GetNewQueue();
    EXPECT_CALL(api, DeviceCreateQueue(apiDevice)).WillOnce(Return(apiQueue));

    // Submit command buffer and check we got a call with both API-side command buffers
    dawnQueueSubmit(queue, 2, cmdBufs);

    EXPECT_CALL(
        api, QueueSubmit(apiQueue, 2, MatchesLambda([=](const dawnCommandBuffer* cmdBufs) -> bool {
                             return cmdBufs[0] == apiCmdBufs[0] && cmdBufs[1] == apiCmdBufs[1];
                         })));

    EXPECT_CALL(api, QueueRelease(apiQueue));
    FlushClient();
}

// Test that the wire is able to send structures that contain pure values (non-objects)
TEST_F(WireArgumentTests, StructureOfValuesArgument) {
    dawnSamplerDescriptor descriptor;
    descriptor.nextInChain = nullptr;
    descriptor.magFilter = DAWN_FILTER_MODE_LINEAR;
    descriptor.minFilter = DAWN_FILTER_MODE_NEAREST;
    descriptor.mipmapFilter = DAWN_FILTER_MODE_LINEAR;
    descriptor.addressModeU = DAWN_ADDRESS_MODE_CLAMP_TO_EDGE;
    descriptor.addressModeV = DAWN_ADDRESS_MODE_REPEAT;
    descriptor.addressModeW = DAWN_ADDRESS_MODE_MIRRORED_REPEAT;
    descriptor.lodMinClamp = kLodMin;
    descriptor.lodMaxClamp = kLodMax;
    descriptor.compareFunction = DAWN_COMPARE_FUNCTION_NEVER;
    descriptor.borderColor = DAWN_BORDER_COLOR_TRANSPARENT_BLACK;

    dawnDeviceCreateSampler(device, &descriptor);
    EXPECT_CALL(api, DeviceCreateSampler(
                         apiDevice, MatchesLambda([](const dawnSamplerDescriptor* desc) -> bool {
                             return desc->nextInChain == nullptr &&
                                    desc->magFilter == DAWN_FILTER_MODE_LINEAR &&
                                    desc->minFilter == DAWN_FILTER_MODE_NEAREST &&
                                    desc->mipmapFilter == DAWN_FILTER_MODE_LINEAR &&
                                    desc->addressModeU == DAWN_ADDRESS_MODE_CLAMP_TO_EDGE &&
                                    desc->addressModeV == DAWN_ADDRESS_MODE_REPEAT &&
                                    desc->addressModeW == DAWN_ADDRESS_MODE_MIRRORED_REPEAT &&
                                    desc->compareFunction == DAWN_COMPARE_FUNCTION_NEVER &&
                                    desc->borderColor == DAWN_BORDER_COLOR_TRANSPARENT_BLACK &&
                                    desc->lodMinClamp == kLodMin && desc->lodMaxClamp == kLodMax;
                         })))
        .WillOnce(Return(nullptr));

    FlushClient();
}

// Test that the wire is able to send structures that contain objects
TEST_F(WireArgumentTests, StructureOfObjectArrayArgument) {
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
    EXPECT_CALL(api, DeviceCreatePipelineLayout(
                         apiDevice,
                         MatchesLambda([apiBgl](const dawnPipelineLayoutDescriptor* desc) -> bool {
                             return desc->nextInChain == nullptr &&
                                    desc->numBindGroupLayouts == 1 &&
                                    desc->bindGroupLayouts[0] == apiBgl;
                         })))
        .WillOnce(Return(nullptr));

    EXPECT_CALL(api, BindGroupLayoutRelease(apiBgl));
    FlushClient();
}

// Test that the wire is able to send structures that contain objects
TEST_F(WireArgumentTests, StructureOfStructureArrayArgument) {
    static constexpr int NUM_BINDINGS = 3;
    dawnBindGroupLayoutBinding bindings[NUM_BINDINGS]{
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

    EXPECT_CALL(api, BindGroupLayoutRelease(apiBgl));
    FlushClient();
}

// Test passing nullptr instead of objects - array of objects version
TEST_F(WireArgumentTests, DISABLED_NullptrInArray) {
    dawnBindGroupLayout nullBGL = nullptr;

    dawnPipelineLayoutDescriptor descriptor;
    descriptor.nextInChain = nullptr;
    descriptor.numBindGroupLayouts = 1;
    descriptor.bindGroupLayouts = &nullBGL;

    dawnDeviceCreatePipelineLayout(device, &descriptor);
    EXPECT_CALL(api,
                DeviceCreatePipelineLayout(
                    apiDevice, MatchesLambda([](const dawnPipelineLayoutDescriptor* desc) -> bool {
                        return desc->nextInChain == nullptr && desc->numBindGroupLayouts == 1 &&
                               desc->bindGroupLayouts[0] == nullptr;
                    })))
        .WillOnce(Return(nullptr));

    FlushClient();
}
