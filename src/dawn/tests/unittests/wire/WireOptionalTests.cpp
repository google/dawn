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

#include "dawn/tests/unittests/wire/WireTest.h"

namespace dawn::wire {

using testing::_;
using testing::Return;

class WireOptionalTests : public WireTest {
  public:
    WireOptionalTests() {}
    ~WireOptionalTests() override = default;
};

// Test passing nullptr instead of objects - object as value version
TEST_F(WireOptionalTests, OptionalObjectValue) {
    WGPUBindGroupLayoutDescriptor bglDesc = {};
    bglDesc.entryCount = 0;
    WGPUBindGroupLayout bgl = wgpuDeviceCreateBindGroupLayout(device, &bglDesc);

    WGPUBindGroupLayout apiBindGroupLayout = api.GetNewBindGroupLayout();
    EXPECT_CALL(api, DeviceCreateBindGroupLayout(apiDevice, _))
        .WillOnce(Return(apiBindGroupLayout));

    // The `sampler`, `textureView` and `buffer` members of a binding are optional.
    WGPUBindGroupEntry entry;
    entry.binding = 0;
    entry.sampler = nullptr;
    entry.textureView = nullptr;
    entry.buffer = nullptr;
    entry.nextInChain = nullptr;

    WGPUBindGroupDescriptor bgDesc = {};
    bgDesc.layout = bgl;
    bgDesc.entryCount = 1;
    bgDesc.entries = &entry;

    wgpuDeviceCreateBindGroup(device, &bgDesc);

    WGPUBindGroup apiPlaceholderBindGroup = api.GetNewBindGroup();
    EXPECT_CALL(api, DeviceCreateBindGroup(
                         apiDevice, MatchesLambda([](const WGPUBindGroupDescriptor* desc) -> bool {
                             return desc->nextInChain == nullptr && desc->entryCount == 1 &&
                                    desc->entries[0].binding == 0 &&
                                    desc->entries[0].sampler == nullptr &&
                                    desc->entries[0].buffer == nullptr &&
                                    desc->entries[0].textureView == nullptr;
                         })))
        .WillOnce(Return(apiPlaceholderBindGroup));

    FlushClient();
}

// Test that the wire is able to send optional pointers to structures
TEST_F(WireOptionalTests, OptionalStructPointer) {
    // Create shader module
    WGPUShaderModuleDescriptor vertexDescriptor = {};
    WGPUShaderModule vsModule = wgpuDeviceCreateShaderModule(device, &vertexDescriptor);
    WGPUShaderModule apiVsModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiVsModule));

    // Create the color state descriptor
    WGPUBlendComponent blendComponent = {};
    blendComponent.operation = WGPUBlendOperation_Add;
    blendComponent.srcFactor = WGPUBlendFactor_One;
    blendComponent.dstFactor = WGPUBlendFactor_One;
    WGPUBlendState blendState = {};
    blendState.alpha = blendComponent;
    blendState.color = blendComponent;
    WGPUColorTargetState colorTargetState = {};
    colorTargetState.format = WGPUTextureFormat_RGBA8Unorm;
    colorTargetState.blend = &blendState;
    colorTargetState.writeMask = WGPUColorWriteMask_All;

    // Create the depth-stencil state
    WGPUStencilFaceState stencilFace = {};
    stencilFace.compare = WGPUCompareFunction_Always;
    stencilFace.failOp = WGPUStencilOperation_Keep;
    stencilFace.depthFailOp = WGPUStencilOperation_Keep;
    stencilFace.passOp = WGPUStencilOperation_Keep;

    WGPUDepthStencilState depthStencilState = {};
    depthStencilState.format = WGPUTextureFormat_Depth24PlusStencil8;
    depthStencilState.depthWriteEnabled = false;
    depthStencilState.depthCompare = WGPUCompareFunction_Always;
    depthStencilState.stencilBack = stencilFace;
    depthStencilState.stencilFront = stencilFace;
    depthStencilState.stencilReadMask = 0xff;
    depthStencilState.stencilWriteMask = 0xff;
    depthStencilState.depthBias = 0;
    depthStencilState.depthBiasSlopeScale = 0.0;
    depthStencilState.depthBiasClamp = 0.0;

    // Create the pipeline layout
    WGPUPipelineLayoutDescriptor layoutDescriptor = {};
    layoutDescriptor.bindGroupLayoutCount = 0;
    layoutDescriptor.bindGroupLayouts = nullptr;
    WGPUPipelineLayout layout = wgpuDeviceCreatePipelineLayout(device, &layoutDescriptor);
    WGPUPipelineLayout apiLayout = api.GetNewPipelineLayout();
    EXPECT_CALL(api, DeviceCreatePipelineLayout(apiDevice, _)).WillOnce(Return(apiLayout));

    // Create pipeline
    WGPURenderPipelineDescriptor pipelineDescriptor = {};

    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.vertex.entryPoint = "main";
    pipelineDescriptor.vertex.bufferCount = 0;
    pipelineDescriptor.vertex.buffers = nullptr;

    WGPUFragmentState fragment = {};
    fragment.module = vsModule;
    fragment.entryPoint = "main";
    fragment.targetCount = 1;
    fragment.targets = &colorTargetState;
    pipelineDescriptor.fragment = &fragment;

    pipelineDescriptor.multisample.count = 1;
    pipelineDescriptor.multisample.mask = 0xFFFFFFFF;
    pipelineDescriptor.multisample.alphaToCoverageEnabled = false;
    pipelineDescriptor.layout = layout;
    pipelineDescriptor.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    pipelineDescriptor.primitive.frontFace = WGPUFrontFace_CCW;
    pipelineDescriptor.primitive.cullMode = WGPUCullMode_None;

    // First case: depthStencil is not null.
    pipelineDescriptor.depthStencil = &depthStencilState;
    wgpuDeviceCreateRenderPipeline(device, &pipelineDescriptor);

    WGPURenderPipeline apiPlaceholderPipeline = api.GetNewRenderPipeline();
    EXPECT_CALL(
        api,
        DeviceCreateRenderPipeline(
            apiDevice, MatchesLambda([](const WGPURenderPipelineDescriptor* desc) -> bool {
                return desc->depthStencil != nullptr &&
                       desc->depthStencil->nextInChain == nullptr &&
                       desc->depthStencil->depthWriteEnabled == false &&
                       desc->depthStencil->depthCompare == WGPUCompareFunction_Always &&
                       desc->depthStencil->stencilBack.compare == WGPUCompareFunction_Always &&
                       desc->depthStencil->stencilBack.failOp == WGPUStencilOperation_Keep &&
                       desc->depthStencil->stencilBack.depthFailOp == WGPUStencilOperation_Keep &&
                       desc->depthStencil->stencilBack.passOp == WGPUStencilOperation_Keep &&
                       desc->depthStencil->stencilFront.compare == WGPUCompareFunction_Always &&
                       desc->depthStencil->stencilFront.failOp == WGPUStencilOperation_Keep &&
                       desc->depthStencil->stencilFront.depthFailOp == WGPUStencilOperation_Keep &&
                       desc->depthStencil->stencilFront.passOp == WGPUStencilOperation_Keep &&
                       desc->depthStencil->stencilReadMask == 0xff &&
                       desc->depthStencil->stencilWriteMask == 0xff &&
                       desc->depthStencil->depthBias == 0 &&
                       desc->depthStencil->depthBiasSlopeScale == 0.0 &&
                       desc->depthStencil->depthBiasClamp == 0.0;
            })))
        .WillOnce(Return(apiPlaceholderPipeline));

    FlushClient();

    // Second case: depthStencil is null.
    pipelineDescriptor.depthStencil = nullptr;
    wgpuDeviceCreateRenderPipeline(device, &pipelineDescriptor);
    EXPECT_CALL(api,
                DeviceCreateRenderPipeline(
                    apiDevice, MatchesLambda([](const WGPURenderPipelineDescriptor* desc) -> bool {
                        return desc->depthStencil == nullptr;
                    })))
        .WillOnce(Return(apiPlaceholderPipeline));

    FlushClient();
}

}  // namespace dawn::wire
