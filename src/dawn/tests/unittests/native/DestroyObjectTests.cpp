// Copyright 2021 The Dawn Authors
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

#include <gtest/gtest.h>

#include <utility>
#include <vector>

#include "dawn/native/Toggles.h"
#include "dawn/native/utils/WGPUHelpers.h"
#include "dawn/tests/DawnNativeTest.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"
#include "dawn/webgpu_cpp.h"
#include "mocks/BindGroupLayoutMock.h"
#include "mocks/BindGroupMock.h"
#include "mocks/BufferMock.h"
#include "mocks/CommandBufferMock.h"
#include "mocks/ComputePipelineMock.h"
#include "mocks/DawnMockTest.h"
#include "mocks/DeviceMock.h"
#include "mocks/ExternalTextureMock.h"
#include "mocks/PipelineLayoutMock.h"
#include "mocks/QuerySetMock.h"
#include "mocks/RenderPipelineMock.h"
#include "mocks/SamplerMock.h"
#include "mocks/ShaderModuleMock.h"
#include "mocks/SwapChainMock.h"
#include "mocks/TextureMock.h"

namespace dawn::native {
namespace {

using ::testing::_;
using ::testing::ByMove;
using ::testing::InSequence;
using ::testing::Mock;
using testing::MockCallback;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::StrictMock;
using ::testing::Test;

static constexpr std::string_view kComputeShader = R"(
        @compute @workgroup_size(1) fn main() {}
    )";

static constexpr std::string_view kVertexShader = R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 0.0);
        }
    )";

static constexpr std::string_view kFragmentShader = R"(
        @fragment fn main() {}
    )";

// Stores and scopes a raw mock object ptr expectation. This is particularly useful on objects that
// are expected to be destroyed at the end of the scope. In most cases, when the validation in this
// class's destructor is ran, the pointer is probably already freed.
class ScopedRawPtrExpectation {
  public:
    explicit ScopedRawPtrExpectation(void* ptr) : mPtr(ptr) {}
    ~ScopedRawPtrExpectation() { Mock::VerifyAndClearExpectations(mPtr); }

  private:
    void* mPtr = nullptr;
};

class DestroyObjectTests : public DawnMockTest {
  public:
    DestroyObjectTests() : DawnMockTest() {
        // Skipping validation on descriptors as coverage for validation is already present.
        mDeviceMock->ForceSetToggleForTesting(Toggle::SkipValidation, true);
    }
};

TEST_F(DestroyObjectTests, BindGroupNativeExplicit) {
    BindGroupDescriptor desc = {};
    desc.layout = mDeviceMock->GetEmptyBindGroupLayout();
    desc.entryCount = 0;
    desc.entries = nullptr;

    Ref<BindGroupMock> bindGroupMock = AcquireRef(new BindGroupMock(mDeviceMock, &desc));
    EXPECT_CALL(*bindGroupMock.Get(), DestroyImpl).Times(1);

    EXPECT_TRUE(bindGroupMock->IsAlive());
    bindGroupMock->Destroy();
    EXPECT_FALSE(bindGroupMock->IsAlive());
}

// If the reference count on API objects reach 0, they should delete themselves. Note that GTest
// will also complain if there is a memory leak.
TEST_F(DestroyObjectTests, BindGroupImplicit) {
    BindGroupDescriptor desc = {};
    desc.layout = mDeviceMock->GetEmptyBindGroupLayout();
    desc.entryCount = 0;
    desc.entries = nullptr;

    Ref<BindGroupMock> bindGroupMock = AcquireRef(new BindGroupMock(mDeviceMock, &desc));
    EXPECT_CALL(*bindGroupMock.Get(), DestroyImpl).Times(1);
    {
        ScopedRawPtrExpectation scoped(bindGroupMock.Get());

        EXPECT_CALL(*mDeviceMock, CreateBindGroupImpl)
            .WillOnce(Return(ByMove(std::move(bindGroupMock))));
        wgpu::BindGroup bindGroup = device.CreateBindGroup(ToCppAPI(&desc));

        EXPECT_TRUE(FromAPI(bindGroup.Get())->IsAlive());
    }
}

TEST_F(DestroyObjectTests, BindGroupLayoutNativeExplicit) {
    // Use an non-empty bind group layout to avoid hitting the internal empty layout in the cache.
    BindGroupLayoutDescriptor desc = {};
    std::vector<BindGroupLayoutEntry> entries;
    entries.push_back(utils::BindingLayoutEntryInitializationHelper(
        0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform));
    desc.entryCount = entries.size();
    desc.entries = entries.data();

    Ref<BindGroupLayoutMock> bindGroupLayoutMock =
        AcquireRef(new BindGroupLayoutMock(mDeviceMock, &desc));
    EXPECT_CALL(*bindGroupLayoutMock.Get(), DestroyImpl).Times(1);

    EXPECT_TRUE(bindGroupLayoutMock->IsAlive());
    bindGroupLayoutMock->Destroy();
    EXPECT_FALSE(bindGroupLayoutMock->IsAlive());
}

// If the reference count on API objects reach 0, they should delete themselves. Note that GTest
// will also complain if there is a memory leak.
TEST_F(DestroyObjectTests, BindGroupLayoutImplicit) {
    // Use an non-empty bind group layout to avoid hitting the internal empty layout in the cache.
    BindGroupLayoutDescriptor desc = {};
    std::vector<BindGroupLayoutEntry> entries;
    entries.push_back(utils::BindingLayoutEntryInitializationHelper(
        0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform));
    desc.entryCount = entries.size();
    desc.entries = entries.data();

    Ref<BindGroupLayoutMock> bindGroupLayoutMock =
        AcquireRef(new BindGroupLayoutMock(mDeviceMock, &desc));
    EXPECT_CALL(*bindGroupLayoutMock.Get(), DestroyImpl).Times(1);
    {
        ScopedRawPtrExpectation scoped(bindGroupLayoutMock.Get());

        EXPECT_CALL(*mDeviceMock, CreateBindGroupLayoutImpl)
            .WillOnce(Return(ByMove(std::move(bindGroupLayoutMock))));
        wgpu::BindGroupLayout bindGroupLayout = device.CreateBindGroupLayout(ToCppAPI(&desc));

        EXPECT_TRUE(FromAPI(bindGroupLayout.Get())->IsAlive());
    }
}

TEST_F(DestroyObjectTests, BufferNativeExplicit) {
    BufferDescriptor desc = {};
    desc.size = 16;
    desc.usage = wgpu::BufferUsage::Uniform;

    Ref<BufferMock> bufferMock = AcquireRef(new BufferMock(mDeviceMock, &desc));
    EXPECT_CALL(*bufferMock.Get(), DestroyImpl).Times(1);

    EXPECT_TRUE(bufferMock->IsAlive());
    bufferMock->Destroy();
    EXPECT_FALSE(bufferMock->IsAlive());
}

TEST_F(DestroyObjectTests, BufferApiExplicit) {
    BufferDescriptor desc = {};
    desc.size = 16;
    desc.usage = wgpu::BufferUsage::Uniform;

    Ref<BufferMock> bufferMock = AcquireRef(new BufferMock(mDeviceMock, &desc));
    EXPECT_CALL(*bufferMock.Get(), DestroyImpl).Times(1);

    EXPECT_CALL(*mDeviceMock, CreateBufferImpl).WillOnce(Return(ByMove(std::move(bufferMock))));
    wgpu::Buffer buffer = device.CreateBuffer(ToCppAPI(&desc));

    EXPECT_TRUE(FromAPI(buffer.Get())->IsAlive());
    buffer.Destroy();
    EXPECT_FALSE(FromAPI(buffer.Get())->IsAlive());
}

// If the reference count on API objects reach 0, they should delete themselves. Note that GTest
// will also complain if there is a memory leak.
TEST_F(DestroyObjectTests, BufferImplicit) {
    BufferDescriptor desc = {};
    desc.size = 16;
    desc.usage = wgpu::BufferUsage::Uniform;

    Ref<BufferMock> bufferMock = AcquireRef(new BufferMock(mDeviceMock, &desc));
    EXPECT_CALL(*bufferMock.Get(), DestroyImpl).Times(1);
    {
        ScopedRawPtrExpectation scoped(bufferMock.Get());

        EXPECT_CALL(*mDeviceMock, CreateBufferImpl).WillOnce(Return(ByMove(std::move(bufferMock))));
        wgpu::Buffer buffer = device.CreateBuffer(ToCppAPI(&desc));

        EXPECT_TRUE(FromAPI(buffer.Get())->IsAlive());
    }
}

TEST_F(DestroyObjectTests, MappedBufferApiExplicit) {
    BufferDescriptor desc = {};
    desc.size = 16;
    desc.usage = wgpu::BufferUsage::MapRead;

    StrictMock<MockCallback<wgpu::BufferMapCallback>> cb;
    EXPECT_CALL(cb, Call).Times(1);
    Ref<BufferMock> bufferMock = AcquireRef(new BufferMock(mDeviceMock, &desc));
    {
        InSequence seq;
        EXPECT_CALL(*bufferMock.Get(), MapAsyncImpl).WillOnce([]() -> MaybeError { return {}; });
        EXPECT_CALL(*bufferMock.Get(), DestroyImpl).Times(1);
        EXPECT_CALL(*bufferMock.Get(), UnmapImpl).Times(1);
    }
    {
        EXPECT_CALL(*mDeviceMock, CreateBufferImpl).WillOnce(Return(ByMove(std::move(bufferMock))));
        wgpu::Buffer buffer = device.CreateBuffer(ToCppAPI(&desc));
        buffer.MapAsync(wgpu::MapMode::Read, 0, 16, cb.Callback(), cb.MakeUserdata(this));
        device.Tick();

        EXPECT_TRUE(FromAPI(buffer.Get())->IsAlive());
        buffer.Destroy();
        EXPECT_FALSE(FromAPI(buffer.Get())->IsAlive());
    }
}

// If the reference count on API objects reach 0, they should delete themselves. Note that GTest
// will also complain if there is a memory leak.
TEST_F(DestroyObjectTests, MappedBufferImplicit) {
    BufferDescriptor desc = {};
    desc.size = 16;
    desc.usage = wgpu::BufferUsage::MapRead;

    StrictMock<MockCallback<wgpu::BufferMapCallback>> cb;
    EXPECT_CALL(cb, Call).Times(1);
    Ref<BufferMock> bufferMock = AcquireRef(new BufferMock(mDeviceMock, &desc));
    {
        InSequence seq;
        EXPECT_CALL(*bufferMock.Get(), MapAsyncImpl).WillOnce([]() -> MaybeError { return {}; });
        EXPECT_CALL(*bufferMock.Get(), DestroyImpl).Times(1);
        EXPECT_CALL(*bufferMock.Get(), UnmapImpl).Times(1);
    }
    {
        ScopedRawPtrExpectation scoped(bufferMock.Get());

        EXPECT_CALL(*mDeviceMock, CreateBufferImpl).WillOnce(Return(ByMove(std::move(bufferMock))));
        wgpu::Buffer buffer = device.CreateBuffer(ToCppAPI(&desc));
        buffer.MapAsync(wgpu::MapMode::Read, 0, 16, cb.Callback(), cb.MakeUserdata(this));
        device.Tick();

        EXPECT_TRUE(FromAPI(buffer.Get())->IsAlive());
    }
}

TEST_F(DestroyObjectTests, CommandBufferNativeExplicit) {
    CommandEncoderDescriptor commandEncoderDesc = {};
    Ref<CommandEncoder> commandEncoder = CommandEncoder::Create(mDeviceMock, &commandEncoderDesc);

    CommandBufferDescriptor commandBufferDesc = {};

    Ref<CommandBufferMock> commandBufferMock =
        AcquireRef(new CommandBufferMock(mDeviceMock, commandEncoder.Get(), &commandBufferDesc));
    EXPECT_CALL(*commandBufferMock.Get(), DestroyImpl).Times(1);

    EXPECT_TRUE(commandBufferMock->IsAlive());
    commandBufferMock->Destroy();
    EXPECT_FALSE(commandBufferMock->IsAlive());
}

// If the reference count on API objects reach 0, they should delete themselves. Note that GTest
// will also complain if there is a memory leak.
TEST_F(DestroyObjectTests, CommandBufferImplicit) {
    CommandEncoderDescriptor commandEncoderDesc = {};
    wgpu::CommandEncoder commandEncoder =
        device.CreateCommandEncoder(ToCppAPI(&commandEncoderDesc));

    CommandBufferDescriptor commandBufferDesc = {};

    Ref<CommandBufferMock> commandBufferMock = AcquireRef(
        new CommandBufferMock(mDeviceMock, FromAPI(commandEncoder.Get()), &commandBufferDesc));
    EXPECT_CALL(*commandBufferMock.Get(), DestroyImpl).Times(1);
    {
        ScopedRawPtrExpectation scoped(commandBufferMock.Get());

        EXPECT_CALL(*mDeviceMock, CreateCommandBuffer)
            .WillOnce(Return(ByMove(std::move(commandBufferMock))));
        wgpu::CommandBuffer commandBuffer = commandEncoder.Finish(ToCppAPI(&commandBufferDesc));

        EXPECT_TRUE(FromAPI(commandBuffer.Get())->IsAlive());
    }
}

TEST_F(DestroyObjectTests, ComputePipelineNativeExplicit) {
    Ref<ShaderModuleMock> csModuleMock =
        ShaderModuleMock::Create(mDeviceMock, kComputeShader.data());
    ComputePipelineDescriptor desc = {};
    desc.compute.module = csModuleMock.Get();
    desc.compute.entryPoint = "main";

    Ref<ComputePipelineMock> computePipelineMock = ComputePipelineMock::Create(mDeviceMock, &desc);
    EXPECT_CALL(*computePipelineMock.Get(), DestroyImpl).Times(1);

    EXPECT_TRUE(computePipelineMock->IsAlive());
    computePipelineMock->Destroy();
    EXPECT_FALSE(computePipelineMock->IsAlive());
}

// If the reference count on API objects reach 0, they should delete themselves. Note that GTest
// will also complain if there is a memory leak.
TEST_F(DestroyObjectTests, ComputePipelineImplicit) {
    Ref<ShaderModuleMock> csModuleMock =
        ShaderModuleMock::Create(mDeviceMock, kComputeShader.data());
    ComputePipelineDescriptor desc = {};
    desc.compute.module = csModuleMock.Get();
    desc.compute.entryPoint = "main";

    // Compute pipelines are initialized during their creation via the device.
    Ref<ComputePipelineMock> computePipelineMock = ComputePipelineMock::Create(mDeviceMock, &desc);
    EXPECT_CALL(*computePipelineMock.Get(), Initialize).Times(1);
    EXPECT_CALL(*computePipelineMock.Get(), DestroyImpl).Times(1);

    {
        ScopedRawPtrExpectation scoped(computePipelineMock.Get());

        EXPECT_CALL(*mDeviceMock, CreateUninitializedComputePipelineImpl)
            .WillOnce(Return(ByMove(std::move(computePipelineMock))));
        wgpu::ComputePipeline computePipeline = device.CreateComputePipeline(ToCppAPI(&desc));

        EXPECT_TRUE(FromAPI(computePipeline.Get())->IsAlive());
    }
}

TEST_F(DestroyObjectTests, ExternalTextureNativeExplicit) {
    TextureDescriptor textureDesc = {};
    textureDesc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
    textureDesc.size.width = 1;
    textureDesc.size.height = 1;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    Ref<TextureMock> textureMock = AcquireRef(new NiceMock<TextureMock>(mDeviceMock, &textureDesc));

    TextureViewDescriptor textureViewDesc = {};
    textureViewDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    Ref<TextureViewMock> textureViewMock =
        AcquireRef(new NiceMock<TextureViewMock>(textureMock.Get(), &textureViewDesc));

    ExternalTextureDescriptor desc = {};
    std::array<float, 12> placeholderConstantArray;
    desc.yuvToRgbConversionMatrix = placeholderConstantArray.data();
    desc.gamutConversionMatrix = placeholderConstantArray.data();
    desc.srcTransferFunctionParameters = placeholderConstantArray.data();
    desc.dstTransferFunctionParameters = placeholderConstantArray.data();
    desc.visibleSize = {1, 1};
    desc.plane0 = textureViewMock.Get();

    Ref<ExternalTextureMock> externalTextureMock = ExternalTextureMock::Create(mDeviceMock, &desc);
    EXPECT_CALL(*externalTextureMock.Get(), DestroyImpl).Times(1);

    EXPECT_TRUE(externalTextureMock->IsAlive());
    externalTextureMock->Destroy();
    EXPECT_FALSE(externalTextureMock->IsAlive());
}

TEST_F(DestroyObjectTests, ExternalTextureApiExplicit) {
    TextureDescriptor textureDesc = {};
    textureDesc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
    textureDesc.size.width = 1;
    textureDesc.size.height = 1;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    Ref<TextureMock> textureMock = AcquireRef(new NiceMock<TextureMock>(mDeviceMock, &textureDesc));

    TextureViewDescriptor textureViewDesc = {};
    textureViewDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    Ref<TextureViewMock> textureViewMock =
        AcquireRef(new NiceMock<TextureViewMock>(textureMock.Get(), &textureViewDesc));

    ExternalTextureDescriptor desc = {};
    std::array<float, 12> placeholderConstantArray;
    desc.yuvToRgbConversionMatrix = placeholderConstantArray.data();
    desc.gamutConversionMatrix = placeholderConstantArray.data();
    desc.srcTransferFunctionParameters = placeholderConstantArray.data();
    desc.dstTransferFunctionParameters = placeholderConstantArray.data();
    desc.visibleSize = {1, 1};
    desc.plane0 = textureViewMock.Get();

    Ref<ExternalTextureMock> externalTextureMock = ExternalTextureMock::Create(mDeviceMock, &desc);
    EXPECT_CALL(*externalTextureMock.Get(), DestroyImpl).Times(1);

    EXPECT_CALL(*mDeviceMock, CreateExternalTextureImpl)
        .WillOnce(Return(ByMove(std::move(externalTextureMock))));
    wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(ToCppAPI(&desc));

    EXPECT_TRUE(FromAPI(externalTexture.Get())->IsAlive());
    externalTexture.Destroy();
    EXPECT_FALSE(FromAPI(externalTexture.Get())->IsAlive());
}

TEST_F(DestroyObjectTests, ExternalTextureImplicit) {
    TextureDescriptor textureDesc = {};
    textureDesc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
    textureDesc.size.width = 1;
    textureDesc.size.height = 1;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    Ref<TextureMock> textureMock = AcquireRef(new NiceMock<TextureMock>(mDeviceMock, &textureDesc));

    TextureViewDescriptor textureViewDesc = {};
    textureViewDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    Ref<TextureViewMock> textureViewMock =
        AcquireRef(new NiceMock<TextureViewMock>(textureMock.Get(), &textureViewDesc));

    ExternalTextureDescriptor desc = {};
    std::array<float, 12> placeholderConstantArray;
    desc.yuvToRgbConversionMatrix = placeholderConstantArray.data();
    desc.gamutConversionMatrix = placeholderConstantArray.data();
    desc.srcTransferFunctionParameters = placeholderConstantArray.data();
    desc.dstTransferFunctionParameters = placeholderConstantArray.data();
    desc.visibleSize = {1, 1};
    desc.plane0 = textureViewMock.Get();

    Ref<ExternalTextureMock> externalTextureMock = ExternalTextureMock::Create(mDeviceMock, &desc);
    EXPECT_CALL(*externalTextureMock.Get(), DestroyImpl).Times(1);
    {
        ScopedRawPtrExpectation scoped(externalTextureMock.Get());

        EXPECT_CALL(*mDeviceMock, CreateExternalTextureImpl)
            .WillOnce(Return(ByMove(std::move(externalTextureMock))));
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(ToCppAPI(&desc));

        EXPECT_TRUE(FromAPI(externalTexture.Get())->IsAlive());
    }
}

TEST_F(DestroyObjectTests, PipelineLayoutNativeExplicit) {
    PipelineLayoutDescriptor desc = {};
    std::vector<BindGroupLayoutBase*> bindGroupLayouts;
    bindGroupLayouts.push_back(mDeviceMock->GetEmptyBindGroupLayout());
    desc.bindGroupLayoutCount = bindGroupLayouts.size();
    desc.bindGroupLayouts = bindGroupLayouts.data();

    Ref<PipelineLayoutMock> pipelineLayoutMock =
        AcquireRef(new PipelineLayoutMock(mDeviceMock, &desc));
    EXPECT_CALL(*pipelineLayoutMock.Get(), DestroyImpl).Times(1);

    EXPECT_TRUE(pipelineLayoutMock->IsAlive());
    pipelineLayoutMock->Destroy();
    EXPECT_FALSE(pipelineLayoutMock->IsAlive());
}

// If the reference count on API objects reach 0, they should delete themselves. Note that GTest
// will also complain if there is a memory leak.
TEST_F(DestroyObjectTests, PipelineLayoutImplicit) {
    PipelineLayoutDescriptor desc = {};
    std::vector<BindGroupLayoutBase*> bindGroupLayouts;
    bindGroupLayouts.push_back(mDeviceMock->GetEmptyBindGroupLayout());
    desc.bindGroupLayoutCount = bindGroupLayouts.size();
    desc.bindGroupLayouts = bindGroupLayouts.data();

    Ref<PipelineLayoutMock> pipelineLayoutMock =
        AcquireRef(new PipelineLayoutMock(mDeviceMock, &desc));
    EXPECT_CALL(*pipelineLayoutMock.Get(), DestroyImpl).Times(1);
    {
        ScopedRawPtrExpectation scoped(pipelineLayoutMock.Get());

        EXPECT_CALL(*mDeviceMock, CreatePipelineLayoutImpl)
            .WillOnce(Return(ByMove(std::move(pipelineLayoutMock))));
        wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(ToCppAPI(&desc));

        EXPECT_TRUE(FromAPI(pipelineLayout.Get())->IsAlive());
    }
}

TEST_F(DestroyObjectTests, QuerySetNativeExplicit) {
    QuerySetDescriptor desc = {};
    desc.type = wgpu::QueryType::Occlusion;
    desc.count = 1;

    Ref<QuerySetMock> querySetMock = AcquireRef(new QuerySetMock(mDeviceMock, &desc));
    EXPECT_CALL(*querySetMock.Get(), DestroyImpl).Times(1);

    EXPECT_TRUE(querySetMock->IsAlive());
    querySetMock->Destroy();
    EXPECT_FALSE(querySetMock->IsAlive());
}

TEST_F(DestroyObjectTests, QuerySetApiExplicit) {
    QuerySetDescriptor desc = {};
    desc.type = wgpu::QueryType::Occlusion;
    desc.count = 1;

    Ref<QuerySetMock> querySetMock = AcquireRef(new QuerySetMock(mDeviceMock, &desc));
    EXPECT_CALL(*querySetMock.Get(), DestroyImpl).Times(1);

    EXPECT_CALL(*mDeviceMock, CreateQuerySetImpl).WillOnce(Return(ByMove(std::move(querySetMock))));
    wgpu::QuerySet querySet = device.CreateQuerySet(ToCppAPI(&desc));

    EXPECT_TRUE(FromAPI(querySet.Get())->IsAlive());
    querySet.Destroy();
    EXPECT_FALSE(FromAPI(querySet.Get())->IsAlive());
}

// If the reference count on API objects reach 0, they should delete themselves. Note that GTest
// will also complain if there is a memory leak.
TEST_F(DestroyObjectTests, QuerySetImplicit) {
    QuerySetDescriptor desc = {};
    desc.type = wgpu::QueryType::Occlusion;
    desc.count = 1;

    Ref<QuerySetMock> querySetMock = AcquireRef(new QuerySetMock(mDeviceMock, &desc));
    EXPECT_CALL(*querySetMock.Get(), DestroyImpl).Times(1);
    {
        ScopedRawPtrExpectation scoped(querySetMock.Get());

        EXPECT_CALL(*mDeviceMock, CreateQuerySetImpl)
            .WillOnce(Return(ByMove(std::move(querySetMock))));
        wgpu::QuerySet querySet = device.CreateQuerySet(ToCppAPI(&desc));

        EXPECT_TRUE(FromAPI(querySet.Get())->IsAlive());
    }
}

TEST_F(DestroyObjectTests, RenderPipelineNativeExplicit) {
    Ref<ShaderModuleMock> vsModuleMock =
        ShaderModuleMock::Create(mDeviceMock, kVertexShader.data());
    RenderPipelineDescriptor desc = {};
    desc.vertex.module = vsModuleMock.Get();
    desc.vertex.entryPoint = "main";

    Ref<RenderPipelineMock> renderPipelineMock = RenderPipelineMock::Create(mDeviceMock, &desc);
    EXPECT_CALL(*renderPipelineMock.Get(), DestroyImpl).Times(1);

    EXPECT_TRUE(renderPipelineMock->IsAlive());
    renderPipelineMock->Destroy();
    EXPECT_FALSE(renderPipelineMock->IsAlive());
}

// If the reference count on API objects reach 0, they should delete themselves. Note that GTest
// will also complain if there is a memory leak.
TEST_F(DestroyObjectTests, RenderPipelineImplicit) {
    Ref<ShaderModuleMock> vsModuleMock =
        ShaderModuleMock::Create(mDeviceMock, kVertexShader.data());
    RenderPipelineDescriptor desc = {};
    desc.vertex.module = vsModuleMock.Get();
    desc.vertex.entryPoint = "main";

    // Render pipelines are initialized during their creation via the device.
    Ref<RenderPipelineMock> renderPipelineMock = RenderPipelineMock::Create(mDeviceMock, &desc);
    EXPECT_CALL(*renderPipelineMock.Get(), Initialize).Times(1);
    EXPECT_CALL(*renderPipelineMock.Get(), DestroyImpl).Times(1);

    {
        ScopedRawPtrExpectation scoped(renderPipelineMock.Get());

        EXPECT_CALL(*mDeviceMock, CreateUninitializedRenderPipelineImpl)
            .WillOnce(Return(ByMove(std::move(renderPipelineMock))));
        wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(ToCppAPI(&desc));

        EXPECT_TRUE(FromAPI(renderPipeline.Get())->IsAlive());
    }
}

TEST_F(DestroyObjectTests, SamplerNativeExplicit) {
    SamplerDescriptor desc = {};

    Ref<SamplerMock> samplerMock = AcquireRef(new SamplerMock(mDeviceMock, &desc));
    EXPECT_CALL(*samplerMock.Get(), DestroyImpl).Times(1);

    EXPECT_TRUE(samplerMock->IsAlive());
    samplerMock->Destroy();
    EXPECT_FALSE(samplerMock->IsAlive());
}

// If the reference count on API objects reach 0, they should delete themselves. Note that GTest
// will also complain if there is a memory leak.
TEST_F(DestroyObjectTests, SamplerImplicit) {
    SamplerDescriptor desc = {};

    Ref<SamplerMock> samplerMock = AcquireRef(new SamplerMock(mDeviceMock, &desc));
    EXPECT_CALL(*samplerMock.Get(), DestroyImpl).Times(1);
    {
        ScopedRawPtrExpectation scoped(samplerMock.Get());

        EXPECT_CALL(*mDeviceMock, CreateSamplerImpl)
            .WillOnce(Return(ByMove(std::move(samplerMock))));
        wgpu::Sampler sampler = device.CreateSampler(ToCppAPI(&desc));

        EXPECT_TRUE(FromAPI(sampler.Get())->IsAlive());
    }
}

TEST_F(DestroyObjectTests, ShaderModuleNativeExplicit) {
    Ref<ShaderModuleMock> shaderModuleMock =
        ShaderModuleMock::Create(mDeviceMock, kVertexShader.data());
    EXPECT_CALL(*shaderModuleMock.Get(), DestroyImpl).Times(1);

    EXPECT_TRUE(shaderModuleMock->IsAlive());
    shaderModuleMock->Destroy();
    EXPECT_FALSE(shaderModuleMock->IsAlive());
}

TEST_F(DestroyObjectTests, ShaderModuleImplicit) {
    ShaderModuleWGSLDescriptor wgslDesc = {};
    wgslDesc.code = kVertexShader.data();
    ShaderModuleDescriptor desc = {};
    desc.nextInChain = &wgslDesc;

    Ref<ShaderModuleMock> shaderModuleMock = ShaderModuleMock::Create(mDeviceMock, &desc);
    EXPECT_CALL(*shaderModuleMock.Get(), DestroyImpl).Times(1);
    {
        ScopedRawPtrExpectation scoped(shaderModuleMock.Get());

        EXPECT_CALL(*mDeviceMock, CreateShaderModuleImpl)
            .WillOnce(Return(ByMove(std::move(shaderModuleMock))));
        wgpu::ShaderModule shaderModule = device.CreateShaderModule(ToCppAPI(&desc));

        EXPECT_TRUE(FromAPI(shaderModule.Get())->IsAlive());
    }
}

TEST_F(DestroyObjectTests, TextureNativeExplicit) {
    TextureDescriptor desc = {};
    desc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
    desc.size.width = 1;
    desc.size.height = 1;
    desc.format = wgpu::TextureFormat::RGBA8Unorm;

    Ref<TextureMock> textureMock = AcquireRef(new TextureMock(mDeviceMock, &desc));
    EXPECT_CALL(*textureMock.Get(), DestroyImpl).Times(1);

    EXPECT_TRUE(textureMock->IsAlive());
    textureMock->Destroy();
    EXPECT_FALSE(textureMock->IsAlive());
}

TEST_F(DestroyObjectTests, TextureApiExplicit) {
    TextureDescriptor desc = {};
    desc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
    desc.size.width = 1;
    desc.size.height = 1;
    desc.format = wgpu::TextureFormat::RGBA8Unorm;

    Ref<TextureMock> textureMock = AcquireRef(new TextureMock(mDeviceMock, &desc));
    EXPECT_CALL(*textureMock.Get(), DestroyImpl).Times(1);

    EXPECT_CALL(*mDeviceMock, CreateTextureImpl).WillOnce(Return(ByMove(std::move(textureMock))));
    wgpu::Texture texture = device.CreateTexture(ToCppAPI(&desc));

    EXPECT_TRUE(FromAPI(texture.Get())->IsAlive());
    texture.Destroy();
    EXPECT_FALSE(FromAPI(texture.Get())->IsAlive());
}

// If the reference count on API objects reach 0, they should delete themselves. Note that GTest
// will also complain if there is a memory leak.
TEST_F(DestroyObjectTests, TextureImplicit) {
    TextureDescriptor desc = {};
    desc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
    desc.size.width = 1;
    desc.size.height = 1;
    desc.format = wgpu::TextureFormat::RGBA8Unorm;

    Ref<TextureMock> textureMock = AcquireRef(new TextureMock(mDeviceMock, &desc));
    EXPECT_CALL(*textureMock.Get(), DestroyImpl).Times(1);
    {
        ScopedRawPtrExpectation scoped(textureMock.Get());

        EXPECT_CALL(*mDeviceMock, CreateTextureImpl)
            .WillOnce(Return(ByMove(std::move(textureMock))));
        wgpu::Texture texture = device.CreateTexture(ToCppAPI(&desc));

        EXPECT_TRUE(FromAPI(texture.Get())->IsAlive());
    }
}

TEST_F(DestroyObjectTests, TextureViewNativeExplicit) {
    TextureDescriptor textureDesc = {};
    textureDesc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
    textureDesc.size.width = 1;
    textureDesc.size.height = 1;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    Ref<TextureMock> textureMock = AcquireRef(new NiceMock<TextureMock>(mDeviceMock, &textureDesc));

    TextureViewDescriptor desc = {};
    desc.format = wgpu::TextureFormat::RGBA8Unorm;
    {
        // Explicitly destroy the texture view.
        Ref<TextureViewMock> textureViewMock =
            AcquireRef(new TextureViewMock(textureMock.Get(), &desc));
        EXPECT_CALL(*textureViewMock.Get(), DestroyImpl).Times(1);

        EXPECT_TRUE(textureViewMock->IsAlive());
        textureViewMock->Destroy();
        EXPECT_FALSE(textureViewMock->IsAlive());
    }
    {
        // Destroying the owning texture should cause the view to be destroyed as well.
        Ref<TextureViewMock> textureViewMock =
            AcquireRef(new TextureViewMock(textureMock.Get(), &desc));
        EXPECT_CALL(*textureViewMock.Get(), DestroyImpl).Times(1);

        EXPECT_TRUE(textureViewMock->IsAlive());
        textureMock->Destroy();
        EXPECT_FALSE(textureViewMock->IsAlive());
    }
}

TEST_F(DestroyObjectTests, TextureViewApiExplicit) {
    TextureDescriptor textureDesc = {};
    textureDesc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
    textureDesc.size.width = 1;
    textureDesc.size.height = 1;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    Ref<TextureMock> textureMock = AcquireRef(new NiceMock<TextureMock>(mDeviceMock, &textureDesc));

    TextureViewDescriptor viewDesc = {};
    viewDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    Ref<TextureViewMock> textureViewMock =
        AcquireRef(new TextureViewMock(textureMock.Get(), &viewDesc));
    EXPECT_CALL(*textureViewMock.Get(), DestroyImpl).Times(1);

    EXPECT_CALL(*mDeviceMock, CreateTextureViewImpl(textureMock.Get(), _))
        .WillOnce(Return(ByMove(std::move(textureViewMock))));
    EXPECT_CALL(*mDeviceMock, CreateTextureImpl).WillOnce(Return(ByMove(std::move(textureMock))));
    wgpu::Texture texture = device.CreateTexture(ToCppAPI(&textureDesc));
    wgpu::TextureView textureView = texture.CreateView(ToCppAPI(&viewDesc));

    EXPECT_TRUE(FromAPI(textureView.Get())->IsAlive());
    texture.Destroy();
    EXPECT_FALSE(FromAPI(textureView.Get())->IsAlive());
}

// If the reference count on API objects reach 0, they should delete themselves. Note that GTest
// will also complain if there is a memory leak.
TEST_F(DestroyObjectTests, TextureViewImplicit) {
    TextureDescriptor textureDesc = {};
    textureDesc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
    textureDesc.size.width = 1;
    textureDesc.size.height = 1;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    Ref<TextureMock> textureMock = AcquireRef(new NiceMock<TextureMock>(mDeviceMock, &textureDesc));

    TextureViewDescriptor viewDesc = {};
    viewDesc.format = wgpu::TextureFormat::RGBA8Unorm;

    Ref<TextureViewMock> textureViewMock =
        AcquireRef(new TextureViewMock(textureMock.Get(), &viewDesc));
    EXPECT_CALL(*textureViewMock.Get(), DestroyImpl).Times(1);
    {
        ScopedRawPtrExpectation scoped(textureViewMock.Get());

        EXPECT_CALL(*mDeviceMock, CreateTextureViewImpl(textureMock.Get(), _))
            .WillOnce(Return(ByMove(std::move(textureViewMock))));
        EXPECT_CALL(*mDeviceMock, CreateTextureImpl)
            .WillOnce(Return(ByMove(std::move(textureMock))));
        wgpu::Texture texture = device.CreateTexture(ToCppAPI(&textureDesc));
        wgpu::TextureView textureView = texture.CreateView(ToCppAPI(&viewDesc));

        EXPECT_TRUE(FromAPI(textureView.Get())->IsAlive());
    }
}

// Destroying the objects on the device explicitly should result in all created objects being
// destroyed in order.
TEST_F(DestroyObjectTests, DestroyObjectsApiExplicit) {
    Ref<BindGroupMock> bindGroupMock;
    wgpu::BindGroup bindGroup;
    {
        BindGroupDescriptor desc = {};
        desc.layout = mDeviceMock->GetEmptyBindGroupLayout();
        desc.entryCount = 0;
        desc.entries = nullptr;

        ScopedRawPtrExpectation scoped(mDeviceMock);
        bindGroupMock = AcquireRef(new BindGroupMock(mDeviceMock, &desc));
        EXPECT_CALL(*mDeviceMock, CreateBindGroupImpl).WillOnce(Return(bindGroupMock));
        bindGroup = device.CreateBindGroup(ToCppAPI(&desc));
    }

    Ref<BindGroupLayoutMock> bindGroupLayoutMock;
    wgpu::BindGroupLayout bindGroupLayout;
    {
        // Use an non-empty bind group layout to avoid hitting the internal empty layout in the
        // cache.
        BindGroupLayoutDescriptor desc = {};
        std::vector<BindGroupLayoutEntry> entries;
        entries.push_back(utils::BindingLayoutEntryInitializationHelper(
            0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform));
        desc.entryCount = entries.size();
        desc.entries = entries.data();

        ScopedRawPtrExpectation scoped(mDeviceMock);
        bindGroupLayoutMock = AcquireRef(new BindGroupLayoutMock(mDeviceMock, &desc));
        EXPECT_CALL(*mDeviceMock, CreateBindGroupLayoutImpl).WillOnce(Return(bindGroupLayoutMock));
        bindGroupLayout = device.CreateBindGroupLayout(ToCppAPI(&desc));
    }

    Ref<ShaderModuleMock> csModuleMock;
    wgpu::ShaderModule csModule;
    {
        ShaderModuleWGSLDescriptor wgslDesc = {};
        wgslDesc.code = kComputeShader.data();
        ShaderModuleDescriptor desc = {};
        desc.nextInChain = &wgslDesc;

        ScopedRawPtrExpectation scoped(mDeviceMock);
        csModuleMock = ShaderModuleMock::Create(mDeviceMock, &desc);
        EXPECT_CALL(*mDeviceMock, CreateShaderModuleImpl).WillOnce(Return(csModuleMock));
        csModule = device.CreateShaderModule(ToCppAPI(&desc));
    }

    Ref<ShaderModuleMock> vsModuleMock;
    wgpu::ShaderModule vsModule;
    {
        ShaderModuleWGSLDescriptor wgslDesc = {};
        wgslDesc.code = kVertexShader.data();
        ShaderModuleDescriptor desc = {};
        desc.nextInChain = &wgslDesc;

        ScopedRawPtrExpectation scoped(mDeviceMock);
        vsModuleMock = ShaderModuleMock::Create(mDeviceMock, &desc);
        EXPECT_CALL(*mDeviceMock, CreateShaderModuleImpl).WillOnce(Return(vsModuleMock));
        vsModule = device.CreateShaderModule(ToCppAPI(&desc));
    }

    Ref<BufferMock> bufferMock;
    wgpu::Buffer buffer;
    {
        BufferDescriptor desc = {};
        desc.size = 16;
        desc.usage = wgpu::BufferUsage::Uniform;

        ScopedRawPtrExpectation scoped(mDeviceMock);
        bufferMock = AcquireRef(new BufferMock(mDeviceMock, &desc));
        EXPECT_CALL(*mDeviceMock, CreateBufferImpl).WillOnce(Return(bufferMock));
        buffer = device.CreateBuffer(ToCppAPI(&desc));
    }

    Ref<CommandBufferMock> commandBufferMock;
    wgpu::CommandBuffer commandBuffer;
    {
        CommandEncoderDescriptor commandEncoderDesc = {};
        wgpu::CommandEncoder commandEncoder =
            device.CreateCommandEncoder(ToCppAPI(&commandEncoderDesc));

        CommandBufferDescriptor commandBufferDesc = {};

        ScopedRawPtrExpectation scoped(mDeviceMock);
        commandBufferMock = AcquireRef(
            new CommandBufferMock(mDeviceMock, FromAPI(commandEncoder.Get()), &commandBufferDesc));
        EXPECT_CALL(*mDeviceMock, CreateCommandBuffer).WillOnce(Return(commandBufferMock));
        commandBuffer = commandEncoder.Finish(ToCppAPI(&commandBufferDesc));
    }

    Ref<ComputePipelineMock> computePipelineMock;
    wgpu::ComputePipeline computePipeline;
    {
        ComputePipelineDescriptor desc = {};
        desc.compute.module = csModuleMock.Get();
        desc.compute.entryPoint = "main";

        ScopedRawPtrExpectation scoped(mDeviceMock);
        computePipelineMock = ComputePipelineMock::Create(mDeviceMock, &desc);
        EXPECT_CALL(*computePipelineMock.Get(), Initialize).Times(1);
        EXPECT_CALL(*mDeviceMock, CreateUninitializedComputePipelineImpl)
            .WillOnce(Return(computePipelineMock));
        computePipeline = device.CreateComputePipeline(ToCppAPI(&desc));
    }

    Ref<PipelineLayoutMock> pipelineLayoutMock;
    wgpu::PipelineLayout pipelineLayout;
    {
        PipelineLayoutDescriptor desc = {};
        std::vector<BindGroupLayoutBase*> bindGroupLayouts;
        bindGroupLayouts.push_back(mDeviceMock->GetEmptyBindGroupLayout());
        desc.bindGroupLayoutCount = bindGroupLayouts.size();
        desc.bindGroupLayouts = bindGroupLayouts.data();

        ScopedRawPtrExpectation scoped(mDeviceMock);
        pipelineLayoutMock = AcquireRef(new PipelineLayoutMock(mDeviceMock, &desc));
        EXPECT_CALL(*mDeviceMock, CreatePipelineLayoutImpl).WillOnce(Return(pipelineLayoutMock));
        pipelineLayout = device.CreatePipelineLayout(ToCppAPI(&desc));
    }

    Ref<QuerySetMock> querySetMock;
    wgpu::QuerySet querySet;
    {
        QuerySetDescriptor desc = {};
        desc.type = wgpu::QueryType::Occlusion;
        desc.count = 1;

        ScopedRawPtrExpectation scoped(mDeviceMock);
        querySetMock = AcquireRef(new QuerySetMock(mDeviceMock, &desc));
        EXPECT_CALL(*mDeviceMock, CreateQuerySetImpl).WillOnce(Return(querySetMock));
        querySet = device.CreateQuerySet(ToCppAPI(&desc));
    }

    Ref<RenderPipelineMock> renderPipelineMock;
    wgpu::RenderPipeline renderPipeline;
    {
        RenderPipelineDescriptor desc = {};
        desc.vertex.module = vsModuleMock.Get();
        desc.vertex.entryPoint = "main";

        ScopedRawPtrExpectation scoped(mDeviceMock);
        renderPipelineMock = RenderPipelineMock::Create(mDeviceMock, &desc);
        EXPECT_CALL(*renderPipelineMock.Get(), Initialize).Times(1);
        EXPECT_CALL(*mDeviceMock, CreateUninitializedRenderPipelineImpl)
            .WillOnce(Return(renderPipelineMock));
        renderPipeline = device.CreateRenderPipeline(ToCppAPI(&desc));
    }

    Ref<SamplerMock> samplerMock;
    wgpu::Sampler sampler;
    {
        SamplerDescriptor desc = {};

        ScopedRawPtrExpectation scoped(mDeviceMock);
        samplerMock = AcquireRef(new SamplerMock(mDeviceMock, &desc));
        EXPECT_CALL(*mDeviceMock, CreateSamplerImpl).WillOnce(Return(samplerMock));
        sampler = device.CreateSampler(ToCppAPI(&desc));
    }

    Ref<TextureMock> textureMock;
    wgpu::Texture texture;
    {
        TextureDescriptor desc = {};
        desc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
        desc.size.width = 1;
        desc.size.height = 1;
        desc.format = wgpu::TextureFormat::RGBA8Unorm;

        ScopedRawPtrExpectation scoped(mDeviceMock);
        textureMock = AcquireRef(new NiceMock<TextureMock>(mDeviceMock, &desc));
        EXPECT_CALL(*mDeviceMock, CreateTextureImpl).WillOnce(Return(textureMock));
        texture = device.CreateTexture(ToCppAPI(&desc));
    }

    Ref<TextureViewMock> textureViewMock;
    wgpu::TextureView textureView;
    {
        TextureViewDescriptor desc = {};
        desc.format = wgpu::TextureFormat::RGBA8Unorm;

        ScopedRawPtrExpectation scoped(mDeviceMock);
        textureViewMock = AcquireRef(new TextureViewMock(textureMock.Get(), &desc));
        EXPECT_CALL(*mDeviceMock, CreateTextureViewImpl).WillOnce(Return(textureViewMock));
        textureView = texture.CreateView(ToCppAPI(&desc));
    }

    Ref<ExternalTextureMock> externalTextureMock;
    wgpu::ExternalTexture externalTexture;
    {
        ExternalTextureDescriptor desc = {};
        std::array<float, 12> placeholderConstantArray;
        desc.yuvToRgbConversionMatrix = placeholderConstantArray.data();
        desc.gamutConversionMatrix = placeholderConstantArray.data();
        desc.srcTransferFunctionParameters = placeholderConstantArray.data();
        desc.dstTransferFunctionParameters = placeholderConstantArray.data();
        desc.visibleSize = {1, 1};
        desc.plane0 = textureViewMock.Get();

        ScopedRawPtrExpectation scoped(mDeviceMock);
        externalTextureMock = ExternalTextureMock::Create(mDeviceMock, &desc);
        EXPECT_CALL(*mDeviceMock, CreateExternalTextureImpl).WillOnce(Return(externalTextureMock));
        externalTexture = device.CreateExternalTexture(ToCppAPI(&desc));
    }

    {
        InSequence seq;
        EXPECT_CALL(*commandBufferMock.Get(), DestroyImpl).Times(1);
        EXPECT_CALL(*renderPipelineMock.Get(), DestroyImpl).Times(1);
        EXPECT_CALL(*computePipelineMock.Get(), DestroyImpl).Times(1);
        EXPECT_CALL(*pipelineLayoutMock.Get(), DestroyImpl).Times(1);
        EXPECT_CALL(*bindGroupMock.Get(), DestroyImpl).Times(1);
        EXPECT_CALL(*bindGroupLayoutMock.Get(), DestroyImpl).Times(1);
        EXPECT_CALL(*vsModuleMock.Get(), DestroyImpl).Times(1);
        EXPECT_CALL(*csModuleMock.Get(), DestroyImpl).Times(1);
        EXPECT_CALL(*externalTextureMock.Get(), DestroyImpl).Times(1);
        EXPECT_CALL(*textureMock.Get(), DestroyImpl).Times(1);
        EXPECT_CALL(*textureViewMock.Get(), DestroyImpl).Times(1);
        EXPECT_CALL(*querySetMock.Get(), DestroyImpl).Times(1);
        EXPECT_CALL(*samplerMock.Get(), DestroyImpl).Times(1);
        EXPECT_CALL(*bufferMock.Get(), DestroyImpl).Times(1);
    }

    EXPECT_TRUE(FromAPI(bindGroup.Get())->IsAlive());
    EXPECT_TRUE(FromAPI(bindGroupLayout.Get())->IsAlive());
    EXPECT_TRUE(FromAPI(buffer.Get())->IsAlive());
    EXPECT_TRUE(FromAPI(commandBuffer.Get())->IsAlive());
    EXPECT_TRUE(FromAPI(computePipeline.Get())->IsAlive());
    EXPECT_TRUE(FromAPI(externalTexture.Get())->IsAlive());
    EXPECT_TRUE(FromAPI(pipelineLayout.Get())->IsAlive());
    EXPECT_TRUE(FromAPI(querySet.Get())->IsAlive());
    EXPECT_TRUE(FromAPI(renderPipeline.Get())->IsAlive());
    EXPECT_TRUE(FromAPI(sampler.Get())->IsAlive());
    EXPECT_TRUE(FromAPI(vsModule.Get())->IsAlive());
    EXPECT_TRUE(FromAPI(csModule.Get())->IsAlive());
    EXPECT_TRUE(FromAPI(texture.Get())->IsAlive());
    EXPECT_TRUE(FromAPI(textureView.Get())->IsAlive());
    device.Destroy();
    EXPECT_FALSE(FromAPI(bindGroup.Get())->IsAlive());
    EXPECT_FALSE(FromAPI(bindGroupLayout.Get())->IsAlive());
    EXPECT_FALSE(FromAPI(buffer.Get())->IsAlive());
    EXPECT_FALSE(FromAPI(commandBuffer.Get())->IsAlive());
    EXPECT_FALSE(FromAPI(computePipeline.Get())->IsAlive());
    EXPECT_FALSE(FromAPI(externalTexture.Get())->IsAlive());
    EXPECT_FALSE(FromAPI(pipelineLayout.Get())->IsAlive());
    EXPECT_FALSE(FromAPI(querySet.Get())->IsAlive());
    EXPECT_FALSE(FromAPI(renderPipeline.Get())->IsAlive());
    EXPECT_FALSE(FromAPI(sampler.Get())->IsAlive());
    EXPECT_FALSE(FromAPI(vsModule.Get())->IsAlive());
    EXPECT_FALSE(FromAPI(csModule.Get())->IsAlive());
    EXPECT_FALSE(FromAPI(texture.Get())->IsAlive());
    EXPECT_FALSE(FromAPI(textureView.Get())->IsAlive());
}

class DestroyObjectRegressionTests : public DawnNativeTest {};

// LastRefInCommand* tests are regression test(s) for https://crbug.com/chromium/1318792. The
// regression tests here are not exhuastive. In order to have an exhuastive test case for this
// class of failures, we should test every possible command with the commands holding the last
// references (or as last as possible) of their needed objects. For now, including simple cases
// including a stripped-down case from the original bug.

// Tests that when a RenderPipeline's last reference is held in a command in an unfinished
// CommandEncoder, that destroying the device still works as expected (and does not cause
// double-free).
TEST_F(DestroyObjectRegressionTests, LastRefInCommandRenderPipeline) {
    ::dawn::utils::BasicRenderPass pass = ::dawn::utils::CreateBasicRenderPass(device, 1, 1);

    ::dawn::utils::ComboRenderPassDescriptor passDesc{};
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderEncoder = encoder.BeginRenderPass(&pass.renderPassInfo);

    ::dawn::utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
    pipelineDesc.vertex.module = ::dawn::utils::CreateShaderModule(device, kVertexShader.data());
    pipelineDesc.vertex.entryPoint = "main";
    pipelineDesc.cFragment.module =
        ::dawn::utils::CreateShaderModule(device, kFragmentShader.data());
    pipelineDesc.cFragment.entryPoint = "main";
    renderEncoder.SetPipeline(device.CreateRenderPipeline(&pipelineDesc));

    device.Destroy();
}

// Tests that when a ComputePipelines's last reference is held in a command in an unfinished
// CommandEncoder, that destroying the device still works as expected (and does not cause
// double-free).
TEST_F(DestroyObjectRegressionTests, LastRefInCommandComputePipeline) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder computeEncoder = encoder.BeginComputePass();

    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.compute.module = ::dawn::utils::CreateShaderModule(device, kComputeShader.data());
    pipelineDesc.compute.entryPoint = "main";
    computeEncoder.SetPipeline(device.CreateComputePipeline(&pipelineDesc));

    device.Destroy();
}

}  // anonymous namespace
}  // namespace dawn::native
