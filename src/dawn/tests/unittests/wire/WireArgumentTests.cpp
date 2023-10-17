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

#include <array>
#include <string>

#include "dawn/common/Constants.h"
#include "dawn/tests/unittests/wire/WireTest.h"

namespace dawn::wire {
namespace {

using testing::_;
using testing::Return;
using testing::Sequence;

class WireArgumentTests : public WireTest {
  public:
    WireArgumentTests() {}
    ~WireArgumentTests() override = default;
};

// Test that the wire is able to send numerical values
TEST_F(WireArgumentTests, ValueArgument) {
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, nullptr);
    WGPUComputePassEncoder pass = wgpuCommandEncoderBeginComputePass(encoder, nullptr);
    wgpuComputePassEncoderDispatchWorkgroups(pass, 1, 2, 3);

    WGPUCommandEncoder apiEncoder = api.GetNewCommandEncoder();
    EXPECT_CALL(api, DeviceCreateCommandEncoder(apiDevice, nullptr)).WillOnce(Return(apiEncoder));

    WGPUComputePassEncoder apiPass = api.GetNewComputePassEncoder();
    EXPECT_CALL(api, CommandEncoderBeginComputePass(apiEncoder, nullptr)).WillOnce(Return(apiPass));

    EXPECT_CALL(api, ComputePassEncoderDispatchWorkgroups(apiPass, 1, 2, 3)).Times(1);

    FlushClient();
}

// Test that the wire is able to send arrays of numerical values
TEST_F(WireArgumentTests, ValueArrayArgument) {
    // Create a bindgroup.
    WGPUBindGroupLayoutDescriptor bglDescriptor = {};
    bglDescriptor.entryCount = 0;
    bglDescriptor.entries = nullptr;

    WGPUBindGroupLayout bgl = wgpuDeviceCreateBindGroupLayout(device, &bglDescriptor);
    WGPUBindGroupLayout apiBgl = api.GetNewBindGroupLayout();
    EXPECT_CALL(api, DeviceCreateBindGroupLayout(apiDevice, _)).WillOnce(Return(apiBgl));

    WGPUBindGroupDescriptor bindGroupDescriptor = {};
    bindGroupDescriptor.layout = bgl;
    bindGroupDescriptor.entryCount = 0;
    bindGroupDescriptor.entries = nullptr;

    WGPUBindGroup bindGroup = wgpuDeviceCreateBindGroup(device, &bindGroupDescriptor);
    WGPUBindGroup apiBindGroup = api.GetNewBindGroup();
    EXPECT_CALL(api, DeviceCreateBindGroup(apiDevice, _)).WillOnce(Return(apiBindGroup));

    // Use the bindgroup in SetBindGroup that takes an array of value offsets.
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, nullptr);
    WGPUComputePassEncoder pass = wgpuCommandEncoderBeginComputePass(encoder, nullptr);

    std::array<uint32_t, 4> testOffsets = {0, 42, 0xDEAD'BEEFu, 0xFFFF'FFFFu};
    wgpuComputePassEncoderSetBindGroup(pass, 0, bindGroup, testOffsets.size(), testOffsets.data());

    WGPUCommandEncoder apiEncoder = api.GetNewCommandEncoder();
    EXPECT_CALL(api, DeviceCreateCommandEncoder(apiDevice, nullptr)).WillOnce(Return(apiEncoder));

    WGPUComputePassEncoder apiPass = api.GetNewComputePassEncoder();
    EXPECT_CALL(api, CommandEncoderBeginComputePass(apiEncoder, nullptr)).WillOnce(Return(apiPass));

    EXPECT_CALL(api, ComputePassEncoderSetBindGroup(
                         apiPass, 0, apiBindGroup, testOffsets.size(),
                         MatchesLambda([testOffsets](const uint32_t* offsets) -> bool {
                             for (size_t i = 0; i < testOffsets.size(); i++) {
                                 if (offsets[i] != testOffsets[i]) {
                                     return false;
                                 }
                             }
                             return true;
                         })));

    FlushClient();
}

// Test that the wire is able to send C strings
TEST_F(WireArgumentTests, CStringArgument) {
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
    pipelineDescriptor.depthStencil = &depthStencilState;

    wgpuDeviceCreateRenderPipeline(device, &pipelineDescriptor);

    WGPURenderPipeline apiPlaceholderPipeline = api.GetNewRenderPipeline();
    EXPECT_CALL(api,
                DeviceCreateRenderPipeline(
                    apiDevice, MatchesLambda([](const WGPURenderPipelineDescriptor* desc) -> bool {
                        return desc->vertex.entryPoint == std::string("main");
                    })))
        .WillOnce(Return(apiPlaceholderPipeline));

    FlushClient();
}

// Test that the wire is able to send objects as value arguments
TEST_F(WireArgumentTests, ObjectAsValueArgument) {
    WGPUCommandEncoder cmdBufEncoder = wgpuDeviceCreateCommandEncoder(device, nullptr);
    WGPUCommandEncoder apiEncoder = api.GetNewCommandEncoder();
    EXPECT_CALL(api, DeviceCreateCommandEncoder(apiDevice, nullptr)).WillOnce(Return(apiEncoder));

    WGPUBufferDescriptor descriptor = {};
    descriptor.size = 8;
    descriptor.usage =
        static_cast<WGPUBufferUsage>(WGPUBufferUsage_CopySrc | WGPUBufferUsage_CopyDst);

    WGPUBuffer buffer = wgpuDeviceCreateBuffer(device, &descriptor);
    WGPUBuffer apiBuffer = api.GetNewBuffer();
    EXPECT_CALL(api, DeviceCreateBuffer(apiDevice, _))
        .WillOnce(Return(apiBuffer))
        .RetiresOnSaturation();

    wgpuCommandEncoderCopyBufferToBuffer(cmdBufEncoder, buffer, 0, buffer, 4, 4);
    EXPECT_CALL(api, CommandEncoderCopyBufferToBuffer(apiEncoder, apiBuffer, 0, apiBuffer, 4, 4));

    FlushClient();
}

// Test that the wire is able to send array of objects
TEST_F(WireArgumentTests, ObjectsAsPointerArgument) {
    WGPUCommandBuffer cmdBufs[2];
    WGPUCommandBuffer apiCmdBufs[2];

    // Create two command buffers we need to use a GMock sequence otherwise the order of the
    // CreateCommandEncoder might be swapped since they are equivalent in term of matchers
    Sequence s;
    for (int i = 0; i < 2; ++i) {
        WGPUCommandEncoder cmdBufEncoder = wgpuDeviceCreateCommandEncoder(device, nullptr);
        cmdBufs[i] = wgpuCommandEncoderFinish(cmdBufEncoder, nullptr);

        WGPUCommandEncoder apiCmdBufEncoder = api.GetNewCommandEncoder();
        EXPECT_CALL(api, DeviceCreateCommandEncoder(apiDevice, nullptr))
            .InSequence(s)
            .WillOnce(Return(apiCmdBufEncoder));

        apiCmdBufs[i] = api.GetNewCommandBuffer();
        EXPECT_CALL(api, CommandEncoderFinish(apiCmdBufEncoder, nullptr))
            .WillOnce(Return(apiCmdBufs[i]));
    }

    // Submit command buffer and check we got a call with both API-side command buffers
    wgpuQueueSubmit(queue, 2, cmdBufs);

    EXPECT_CALL(
        api, QueueSubmit(apiQueue, 2, MatchesLambda([=](const WGPUCommandBuffer* cmdBufs) -> bool {
                             return cmdBufs[0] == apiCmdBufs[0] && cmdBufs[1] == apiCmdBufs[1];
                         })));

    FlushClient();
}

// Test that the wire is able to send structures that contain pure values (non-objects)
TEST_F(WireArgumentTests, StructureOfValuesArgument) {
    WGPUSamplerDescriptor descriptor = {};
    descriptor.magFilter = WGPUFilterMode_Linear;
    descriptor.minFilter = WGPUFilterMode_Nearest;
    descriptor.mipmapFilter = WGPUMipmapFilterMode_Linear;
    descriptor.addressModeU = WGPUAddressMode_ClampToEdge;
    descriptor.addressModeV = WGPUAddressMode_Repeat;
    descriptor.addressModeW = WGPUAddressMode_MirrorRepeat;
    descriptor.lodMinClamp = kLodMin;
    descriptor.lodMaxClamp = kLodMax;
    descriptor.compare = WGPUCompareFunction_Never;

    wgpuDeviceCreateSampler(device, &descriptor);

    WGPUSampler apiPlaceholderSampler = api.GetNewSampler();
    EXPECT_CALL(api, DeviceCreateSampler(
                         apiDevice, MatchesLambda([](const WGPUSamplerDescriptor* desc) -> bool {
                             return desc->nextInChain == nullptr &&
                                    desc->magFilter == WGPUFilterMode_Linear &&
                                    desc->minFilter == WGPUFilterMode_Nearest &&
                                    desc->mipmapFilter == WGPUMipmapFilterMode_Linear &&
                                    desc->addressModeU == WGPUAddressMode_ClampToEdge &&
                                    desc->addressModeV == WGPUAddressMode_Repeat &&
                                    desc->addressModeW == WGPUAddressMode_MirrorRepeat &&
                                    desc->compare == WGPUCompareFunction_Never &&
                                    desc->lodMinClamp == kLodMin && desc->lodMaxClamp == kLodMax;
                         })))
        .WillOnce(Return(apiPlaceholderSampler));

    FlushClient();
}

// Test that the wire is able to send structures that contain objects
TEST_F(WireArgumentTests, StructureOfObjectArrayArgument) {
    WGPUBindGroupLayoutDescriptor bglDescriptor = {};
    bglDescriptor.entryCount = 0;
    bglDescriptor.entries = nullptr;

    WGPUBindGroupLayout bgl = wgpuDeviceCreateBindGroupLayout(device, &bglDescriptor);
    WGPUBindGroupLayout apiBgl = api.GetNewBindGroupLayout();
    EXPECT_CALL(api, DeviceCreateBindGroupLayout(apiDevice, _)).WillOnce(Return(apiBgl));

    WGPUPipelineLayoutDescriptor descriptor = {};
    descriptor.bindGroupLayoutCount = 1;
    descriptor.bindGroupLayouts = &bgl;

    wgpuDeviceCreatePipelineLayout(device, &descriptor);

    WGPUPipelineLayout apiPlaceholderLayout = api.GetNewPipelineLayout();
    EXPECT_CALL(api, DeviceCreatePipelineLayout(
                         apiDevice,
                         MatchesLambda([apiBgl](const WGPUPipelineLayoutDescriptor* desc) -> bool {
                             return desc->nextInChain == nullptr &&
                                    desc->bindGroupLayoutCount == 1 &&
                                    desc->bindGroupLayouts[0] == apiBgl;
                         })))
        .WillOnce(Return(apiPlaceholderLayout));

    FlushClient();
}

// Test that the wire is able to send structures that contain objects
TEST_F(WireArgumentTests, StructureOfStructureArrayArgument) {
    static constexpr int NUM_BINDINGS = 3;
    WGPUBindGroupLayoutEntry entries[NUM_BINDINGS]{
        {nullptr,
         0,
         WGPUShaderStage_Vertex,
         {},
         {nullptr, WGPUSamplerBindingType_Filtering},
         {},
         {}},
        {nullptr,
         1,
         WGPUShaderStage_Vertex,
         {},
         {},
         {nullptr, WGPUTextureSampleType_Float, WGPUTextureViewDimension_2D, false},
         {}},
        {nullptr,
         2,
         static_cast<WGPUShaderStage>(WGPUShaderStage_Vertex | WGPUShaderStage_Fragment),
         {nullptr, WGPUBufferBindingType_Uniform, false, 0},
         {},
         {},
         {}},
    };
    WGPUBindGroupLayoutDescriptor bglDescriptor = {};
    bglDescriptor.entryCount = NUM_BINDINGS;
    bglDescriptor.entries = entries;

    wgpuDeviceCreateBindGroupLayout(device, &bglDescriptor);
    WGPUBindGroupLayout apiBgl = api.GetNewBindGroupLayout();
    EXPECT_CALL(
        api,
        DeviceCreateBindGroupLayout(
            apiDevice, MatchesLambda([entries](const WGPUBindGroupLayoutDescriptor* desc) -> bool {
                for (int i = 0; i < NUM_BINDINGS; ++i) {
                    const auto& a = desc->entries[i];
                    const auto& b = entries[i];
                    if (a.binding != b.binding || a.visibility != b.visibility ||
                        a.buffer.type != b.buffer.type || a.sampler.type != b.sampler.type ||
                        a.texture.sampleType != b.texture.sampleType) {
                        return false;
                    }
                }
                return desc->nextInChain == nullptr && desc->entryCount == 3;
            })))
        .WillOnce(Return(apiBgl));

    FlushClient();
}

// Test passing nullptr instead of objects - array of objects version
TEST_F(WireArgumentTests, DISABLED_NullptrInArray) {
    WGPUBindGroupLayout nullBGL = nullptr;

    WGPUPipelineLayoutDescriptor descriptor = {};
    descriptor.bindGroupLayoutCount = 1;
    descriptor.bindGroupLayouts = &nullBGL;

    wgpuDeviceCreatePipelineLayout(device, &descriptor);
    EXPECT_CALL(api,
                DeviceCreatePipelineLayout(
                    apiDevice, MatchesLambda([](const WGPUPipelineLayoutDescriptor* desc) -> bool {
                        return desc->nextInChain == nullptr && desc->bindGroupLayoutCount == 1 &&
                               desc->bindGroupLayouts[0] == nullptr;
                    })))
        .WillOnce(Return(nullptr));

    FlushClient();
}

}  // anonymous namespace
}  // namespace dawn::wire
