// Copyright 2020 The Dawn & Tint Authors
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
using testing::Invoke;
using testing::NotNull;
using testing::Return;
using testing::Unused;

class WireExtensionTests : public WireTest {
  public:
    WireExtensionTests() {}
    ~WireExtensionTests() override = default;
};

// Serialize/Deserializes a chained struct correctly.
TEST_F(WireExtensionTests, ChainedStruct) {
    WGPUShaderModuleDescriptor shaderModuleDesc = {};
    WGPUShaderModule apiShaderModule = api.GetNewShaderModule();
    WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(device, &shaderModuleDesc);
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiShaderModule));
    FlushClient();

    WGPUPrimitiveDepthClipControl clientExt = {};
    clientExt.chain.sType = WGPUSType_PrimitiveDepthClipControl;
    clientExt.chain.next = nullptr;
    clientExt.unclippedDepth = true;

    WGPURenderPipelineDescriptor renderPipelineDesc = {};
    renderPipelineDesc.vertex.module = shaderModule;
    renderPipelineDesc.vertex.entryPoint = "main";
    renderPipelineDesc.primitive.nextInChain = &clientExt.chain;

    wgpuDeviceCreateRenderPipeline(device, &renderPipelineDesc);
    EXPECT_CALL(api, DeviceCreateRenderPipeline(apiDevice, NotNull()))
        .WillOnce(Invoke(
            [&](Unused, const WGPURenderPipelineDescriptor* serverDesc) -> WGPURenderPipeline {
                const auto* ext = reinterpret_cast<const WGPUPrimitiveDepthClipControl*>(
                    serverDesc->primitive.nextInChain);
                EXPECT_EQ(ext->chain.sType, clientExt.chain.sType);
                EXPECT_EQ(ext->unclippedDepth, true);
                EXPECT_EQ(ext->chain.next, nullptr);

                return api.GetNewRenderPipeline();
            }));
    FlushClient();
}

// Serialize/Deserializes multiple chained structs correctly.
TEST_F(WireExtensionTests, MutlipleChainedStructs) {
    WGPUShaderModuleDescriptor shaderModuleDesc = {};
    WGPUShaderModule apiShaderModule = api.GetNewShaderModule();
    WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(device, &shaderModuleDesc);
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiShaderModule));
    FlushClient();

    WGPUPrimitiveDepthClipControl clientExt2 = {};
    clientExt2.chain.sType = WGPUSType_PrimitiveDepthClipControl;
    clientExt2.chain.next = nullptr;
    clientExt2.unclippedDepth = false;

    WGPUPrimitiveDepthClipControl clientExt1 = {};
    clientExt1.chain.sType = WGPUSType_PrimitiveDepthClipControl;
    clientExt1.chain.next = &clientExt2.chain;
    clientExt1.unclippedDepth = true;

    WGPURenderPipelineDescriptor renderPipelineDesc = {};
    renderPipelineDesc.vertex.module = shaderModule;
    renderPipelineDesc.vertex.entryPoint = "main";
    renderPipelineDesc.primitive.nextInChain = &clientExt1.chain;

    wgpuDeviceCreateRenderPipeline(device, &renderPipelineDesc);
    EXPECT_CALL(api, DeviceCreateRenderPipeline(apiDevice, NotNull()))
        .WillOnce(Invoke(
            [&](Unused, const WGPURenderPipelineDescriptor* serverDesc) -> WGPURenderPipeline {
                const auto* ext1 = reinterpret_cast<const WGPUPrimitiveDepthClipControl*>(
                    serverDesc->primitive.nextInChain);
                EXPECT_EQ(ext1->chain.sType, clientExt1.chain.sType);
                EXPECT_EQ(ext1->unclippedDepth, true);

                const auto* ext2 =
                    reinterpret_cast<const WGPUPrimitiveDepthClipControl*>(ext1->chain.next);
                EXPECT_EQ(ext2->chain.sType, clientExt2.chain.sType);
                EXPECT_EQ(ext2->unclippedDepth, false);
                EXPECT_EQ(ext2->chain.next, nullptr);

                return api.GetNewRenderPipeline();
            }));
    FlushClient();

    // Swap the order of the chained structs.
    renderPipelineDesc.primitive.nextInChain = &clientExt2.chain;
    clientExt2.chain.next = &clientExt1.chain;
    clientExt1.chain.next = nullptr;

    wgpuDeviceCreateRenderPipeline(device, &renderPipelineDesc);
    EXPECT_CALL(api, DeviceCreateRenderPipeline(apiDevice, NotNull()))
        .WillOnce(Invoke(
            [&](Unused, const WGPURenderPipelineDescriptor* serverDesc) -> WGPURenderPipeline {
                const auto* ext2 = reinterpret_cast<const WGPUPrimitiveDepthClipControl*>(
                    serverDesc->primitive.nextInChain);
                EXPECT_EQ(ext2->chain.sType, clientExt2.chain.sType);
                EXPECT_EQ(ext2->unclippedDepth, false);

                const auto* ext1 =
                    reinterpret_cast<const WGPUPrimitiveDepthClipControl*>(ext2->chain.next);
                EXPECT_EQ(ext1->chain.sType, clientExt1.chain.sType);
                EXPECT_EQ(ext1->unclippedDepth, true);
                EXPECT_EQ(ext1->chain.next, nullptr);

                return api.GetNewRenderPipeline();
            }));
    FlushClient();
}

// Test that a chained struct with Invalid sType passes through as Invalid.
TEST_F(WireExtensionTests, InvalidSType) {
    WGPUShaderModuleDescriptor shaderModuleDesc = {};
    WGPUShaderModule apiShaderModule = api.GetNewShaderModule();
    WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(device, &shaderModuleDesc);
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiShaderModule));
    FlushClient();

    WGPUPrimitiveDepthClipControl clientExt = {};
    clientExt.chain.sType = WGPUSType_Invalid;
    clientExt.chain.next = nullptr;

    WGPURenderPipelineDescriptor renderPipelineDesc = {};
    renderPipelineDesc.vertex.module = shaderModule;
    renderPipelineDesc.vertex.entryPoint = "main";
    renderPipelineDesc.primitive.nextInChain = &clientExt.chain;

    wgpuDeviceCreateRenderPipeline(device, &renderPipelineDesc);
    EXPECT_CALL(api, DeviceCreateRenderPipeline(apiDevice, NotNull()))
        .WillOnce(Invoke(
            [&](Unused, const WGPURenderPipelineDescriptor* serverDesc) -> WGPURenderPipeline {
                EXPECT_EQ(serverDesc->primitive.nextInChain->sType, WGPUSType_Invalid);
                EXPECT_EQ(serverDesc->primitive.nextInChain->next, nullptr);
                return api.GetNewRenderPipeline();
            }));
    FlushClient();
}

// Test that a chained struct with unknown sType passes through as Invalid.
TEST_F(WireExtensionTests, UnknownSType) {
    WGPUShaderModuleDescriptor shaderModuleDesc = {};
    WGPUShaderModule apiShaderModule = api.GetNewShaderModule();
    WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(device, &shaderModuleDesc);
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiShaderModule));
    FlushClient();

    WGPUPrimitiveDepthClipControl clientExt = {};
    clientExt.chain.sType = static_cast<WGPUSType>(-1);
    clientExt.chain.next = nullptr;

    WGPURenderPipelineDescriptor renderPipelineDesc = {};
    renderPipelineDesc.vertex.module = shaderModule;
    renderPipelineDesc.vertex.entryPoint = "main";
    renderPipelineDesc.primitive.nextInChain = &clientExt.chain;

    wgpuDeviceCreateRenderPipeline(device, &renderPipelineDesc);
    EXPECT_CALL(api, DeviceCreateRenderPipeline(apiDevice, NotNull()))
        .WillOnce(Invoke(
            [&](Unused, const WGPURenderPipelineDescriptor* serverDesc) -> WGPURenderPipeline {
                EXPECT_EQ(serverDesc->primitive.nextInChain->sType, WGPUSType_Invalid);
                EXPECT_EQ(serverDesc->primitive.nextInChain->next, nullptr);
                return api.GetNewRenderPipeline();
            }));
    FlushClient();
}

// Test that if both an invalid and valid stype are passed on the chain, only the invalid
// sType passes through as Invalid.
TEST_F(WireExtensionTests, ValidAndInvalidSTypeInChain) {
    WGPUShaderModuleDescriptor shaderModuleDesc = {};
    WGPUShaderModule apiShaderModule = api.GetNewShaderModule();
    WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(device, &shaderModuleDesc);
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiShaderModule));
    FlushClient();

    WGPUPrimitiveDepthClipControl clientExt2 = {};
    clientExt2.chain.sType = WGPUSType_Invalid;
    clientExt2.chain.next = nullptr;

    WGPUPrimitiveDepthClipControl clientExt1 = {};
    clientExt1.chain.sType = WGPUSType_PrimitiveDepthClipControl;
    clientExt1.chain.next = &clientExt2.chain;
    clientExt1.unclippedDepth = true;

    WGPURenderPipelineDescriptor renderPipelineDesc = {};
    renderPipelineDesc.vertex.module = shaderModule;
    renderPipelineDesc.vertex.entryPoint = "main";
    renderPipelineDesc.primitive.nextInChain = &clientExt1.chain;

    wgpuDeviceCreateRenderPipeline(device, &renderPipelineDesc);
    EXPECT_CALL(api, DeviceCreateRenderPipeline(apiDevice, NotNull()))
        .WillOnce(Invoke(
            [&](Unused, const WGPURenderPipelineDescriptor* serverDesc) -> WGPURenderPipeline {
                const auto* ext = reinterpret_cast<const WGPUPrimitiveDepthClipControl*>(
                    serverDesc->primitive.nextInChain);
                EXPECT_EQ(ext->chain.sType, clientExt1.chain.sType);
                EXPECT_EQ(ext->unclippedDepth, true);

                EXPECT_EQ(ext->chain.next->sType, WGPUSType_Invalid);
                EXPECT_EQ(ext->chain.next->next, nullptr);
                return api.GetNewRenderPipeline();
            }));
    FlushClient();

    // Swap the order of the chained structs.
    renderPipelineDesc.primitive.nextInChain = &clientExt2.chain;
    clientExt2.chain.next = &clientExt1.chain;
    clientExt1.chain.next = nullptr;

    wgpuDeviceCreateRenderPipeline(device, &renderPipelineDesc);
    EXPECT_CALL(api, DeviceCreateRenderPipeline(apiDevice, NotNull()))
        .WillOnce(Invoke(
            [&](Unused, const WGPURenderPipelineDescriptor* serverDesc) -> WGPURenderPipeline {
                EXPECT_EQ(serverDesc->primitive.nextInChain->sType, WGPUSType_Invalid);

                const auto* ext = reinterpret_cast<const WGPUPrimitiveDepthClipControl*>(
                    serverDesc->primitive.nextInChain->next);
                EXPECT_EQ(ext->chain.sType, clientExt1.chain.sType);
                EXPECT_EQ(ext->unclippedDepth, true);
                EXPECT_EQ(ext->chain.next, nullptr);

                return api.GetNewRenderPipeline();
            }));
    FlushClient();
}

}  // anonymous namespace
}  // namespace dawn::wire
