// Copyright 2019 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/tests/unittests/wire/WireTest.h"

namespace dawn::wire {
namespace {

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

}  // anonymous namespace
}  // namespace dawn::wire
