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

using namespace testing;
using namespace dawn_wire;

class WireOptionalTests : public WireTest {
  public:
    WireOptionalTests() : WireTest(true) {
    }
    ~WireOptionalTests() override = default;
};

// Test passing nullptr instead of objects - object as value version
TEST_F(WireOptionalTests, OptionalObjectValue) {
    dawnBindGroupLayoutDescriptor bglDesc;
    bglDesc.nextInChain = nullptr;
    bglDesc.numBindings = 0;
    dawnBindGroupLayout bgl = dawnDeviceCreateBindGroupLayout(device, &bglDesc);

    dawnBindGroupLayout apiBindGroupLayout = api.GetNewBindGroupLayout();
    EXPECT_CALL(api, DeviceCreateBindGroupLayout(apiDevice, _))
        .WillOnce(Return(apiBindGroupLayout));

    // The `sampler`, `textureView` and `buffer` members of a binding are optional.
    dawnBindGroupBinding binding;
    binding.binding = 0;
    binding.sampler = nullptr;
    binding.textureView = nullptr;
    binding.buffer = nullptr;

    dawnBindGroupDescriptor bgDesc;
    bgDesc.nextInChain = nullptr;
    bgDesc.layout = bgl;
    bgDesc.numBindings = 1;
    bgDesc.bindings = &binding;

    dawnDeviceCreateBindGroup(device, &bgDesc);
    EXPECT_CALL(api, DeviceCreateBindGroup(
                         apiDevice, MatchesLambda([](const dawnBindGroupDescriptor* desc) -> bool {
                             return desc->nextInChain == nullptr && desc->numBindings == 1 &&
                                    desc->bindings[0].binding == 0 &&
                                    desc->bindings[0].sampler == nullptr &&
                                    desc->bindings[0].buffer == nullptr &&
                                    desc->bindings[0].textureView == nullptr;
                         })))
        .WillOnce(Return(nullptr));

    EXPECT_CALL(api, BindGroupLayoutRelease(apiBindGroupLayout));
    FlushClient();
}

// Test that the wire is able to send optional pointers to structures
TEST_F(WireOptionalTests, OptionalStructPointer) {
    // Create shader module
    dawnShaderModuleDescriptor vertexDescriptor;
    vertexDescriptor.nextInChain = nullptr;
    vertexDescriptor.codeSize = 0;
    dawnShaderModule vsModule = dawnDeviceCreateShaderModule(device, &vertexDescriptor);
    dawnShaderModule apiVsModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiVsModule));

    // Create the blend state descriptor
    dawnBlendDescriptor blendDescriptor;
    blendDescriptor.operation = DAWN_BLEND_OPERATION_ADD;
    blendDescriptor.srcFactor = DAWN_BLEND_FACTOR_ONE;
    blendDescriptor.dstFactor = DAWN_BLEND_FACTOR_ONE;
    dawnBlendStateDescriptor blendStateDescriptor;
    blendStateDescriptor.nextInChain = nullptr;
    blendStateDescriptor.alphaBlend = blendDescriptor;
    blendStateDescriptor.colorBlend = blendDescriptor;
    blendStateDescriptor.colorWriteMask = DAWN_COLOR_WRITE_MASK_ALL;

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

    dawnAttachmentsStateDescriptor attachmentsState;
    attachmentsState.nextInChain = nullptr;
    attachmentsState.numColorAttachments = 1;
    dawnAttachmentDescriptor colorAttachment = {nullptr, DAWN_TEXTURE_FORMAT_R8_G8_B8_A8_UNORM};
    dawnAttachmentDescriptor* colorAttachmentPtr[] = {&colorAttachment};
    attachmentsState.colorAttachments = colorAttachmentPtr;
    attachmentsState.hasDepthStencilAttachment = false;
    // Even with hasDepthStencilAttachment = false, depthStencilAttachment must point to valid
    // data because we don't have optional substructures yet.
    attachmentsState.depthStencilAttachment = &colorAttachment;
    pipelineDescriptor.attachmentsState = &attachmentsState;

    pipelineDescriptor.numBlendStates = 1;
    pipelineDescriptor.blendStates = &blendStateDescriptor;

    pipelineDescriptor.sampleCount = 1;
    pipelineDescriptor.layout = layout;
    pipelineDescriptor.inputState = inputState;
    pipelineDescriptor.indexFormat = DAWN_INDEX_FORMAT_UINT32;
    pipelineDescriptor.primitiveTopology = DAWN_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // First case: depthStencilState is not null.
    pipelineDescriptor.depthStencilState = &depthStencilState;
    dawnDeviceCreateRenderPipeline(device, &pipelineDescriptor);
    EXPECT_CALL(
        api,
        DeviceCreateRenderPipeline(
            apiDevice, MatchesLambda([](const dawnRenderPipelineDescriptor* desc) -> bool {
                return desc->depthStencilState != nullptr &&
                       desc->depthStencilState->nextInChain == nullptr &&
                       desc->depthStencilState->depthWriteEnabled == false &&
                       desc->depthStencilState->depthCompare == DAWN_COMPARE_FUNCTION_ALWAYS &&
                       desc->depthStencilState->stencilBack.compare ==
                           DAWN_COMPARE_FUNCTION_ALWAYS &&
                       desc->depthStencilState->stencilBack.failOp == DAWN_STENCIL_OPERATION_KEEP &&
                       desc->depthStencilState->stencilBack.depthFailOp ==
                           DAWN_STENCIL_OPERATION_KEEP &&
                       desc->depthStencilState->stencilBack.passOp == DAWN_STENCIL_OPERATION_KEEP &&
                       desc->depthStencilState->stencilFront.compare ==
                           DAWN_COMPARE_FUNCTION_ALWAYS &&
                       desc->depthStencilState->stencilFront.failOp ==
                           DAWN_STENCIL_OPERATION_KEEP &&
                       desc->depthStencilState->stencilFront.depthFailOp ==
                           DAWN_STENCIL_OPERATION_KEEP &&
                       desc->depthStencilState->stencilFront.passOp ==
                           DAWN_STENCIL_OPERATION_KEEP &&
                       desc->depthStencilState->stencilReadMask == 0xff &&
                       desc->depthStencilState->stencilWriteMask == 0xff;
            })))
        .WillOnce(Return(nullptr));

    FlushClient();

    // Second case: depthStencilState is null.
    pipelineDescriptor.depthStencilState = nullptr;
    dawnDeviceCreateRenderPipeline(device, &pipelineDescriptor);
    EXPECT_CALL(api,
                DeviceCreateRenderPipeline(
                    apiDevice, MatchesLambda([](const dawnRenderPipelineDescriptor* desc) -> bool {
                        return desc->depthStencilState == nullptr;
                    })))
        .WillOnce(Return(nullptr));

    EXPECT_CALL(api, ShaderModuleRelease(apiVsModule));
    EXPECT_CALL(api, InputStateBuilderRelease(apiInputStateBuilder));
    EXPECT_CALL(api, InputStateRelease(apiInputState));
    EXPECT_CALL(api, PipelineLayoutRelease(apiLayout));

    FlushClient();
}
