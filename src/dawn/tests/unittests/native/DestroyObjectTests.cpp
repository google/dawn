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

#include "dawn/native/Toggles.h"
#include "dawn/tests/DawnNativeTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"
#include "mocks/BindGroupLayoutMock.h"
#include "mocks/BindGroupMock.h"
#include "mocks/BufferMock.h"
#include "mocks/CommandBufferMock.h"
#include "mocks/ComputePipelineMock.h"
#include "mocks/DeviceMock.h"
#include "mocks/ExternalTextureMock.h"
#include "mocks/PipelineLayoutMock.h"
#include "mocks/QuerySetMock.h"
#include "mocks/RenderPipelineMock.h"
#include "mocks/SamplerMock.h"
#include "mocks/ShaderModuleMock.h"
#include "mocks/SwapChainMock.h"
#include "mocks/TextureMock.h"

namespace dawn::native { namespace {

    using ::testing::_;
    using ::testing::ByMove;
    using ::testing::InSequence;
    using ::testing::Return;
    using ::testing::Test;

    class DestroyObjectTests : public Test {
      public:
        DestroyObjectTests() : Test() {
            // Skipping validation on descriptors as coverage for validation is already present.
            mDevice.SetToggle(Toggle::SkipValidation, true);
        }

        Ref<TextureMock> GetTexture() {
            if (mTexture != nullptr) {
                return mTexture;
            }
            mTexture =
                AcquireRef(new TextureMock(&mDevice, TextureBase::TextureState::OwnedInternal));
            EXPECT_CALL(*mTexture.Get(), DestroyImpl).Times(1);
            return mTexture;
        }

        Ref<PipelineLayoutMock> GetPipelineLayout() {
            if (mPipelineLayout != nullptr) {
                return mPipelineLayout;
            }
            mPipelineLayout = AcquireRef(new PipelineLayoutMock(&mDevice));
            EXPECT_CALL(*mPipelineLayout.Get(), DestroyImpl).Times(1);
            return mPipelineLayout;
        }

        Ref<ShaderModuleMock> GetVertexShaderModule() {
            if (mVsModule != nullptr) {
                return mVsModule;
            }
            DAWN_TRY_ASSIGN_WITH_CLEANUP(
                mVsModule, ShaderModuleMock::Create(&mDevice, R"(
            @stage(vertex) fn main() -> @builtin(position) vec4<f32> {
                return vec4<f32>(0.0, 0.0, 0.0, 1.0);
            })"),
                { ASSERT(false); }, mVsModule);
            EXPECT_CALL(*mVsModule.Get(), DestroyImpl).Times(1);
            return mVsModule;
        }

        Ref<ShaderModuleMock> GetComputeShaderModule() {
            if (mCsModule != nullptr) {
                return mCsModule;
            }
            DAWN_TRY_ASSIGN_WITH_CLEANUP(
                mCsModule, ShaderModuleMock::Create(&mDevice, R"(
            @stage(compute) @workgroup_size(1) fn main() {
            })"),
                { ASSERT(false); }, mCsModule);
            EXPECT_CALL(*mCsModule.Get(), DestroyImpl).Times(1);
            return mCsModule;
        }

      protected:
        DeviceMock mDevice;

        // The following lazy-initialized objects are used to facilitate creation of dependent
        // objects under test.
        Ref<TextureMock> mTexture;
        Ref<PipelineLayoutMock> mPipelineLayout;
        Ref<ShaderModuleMock> mVsModule;
        Ref<ShaderModuleMock> mCsModule;
    };

    TEST_F(DestroyObjectTests, BindGroupExplicit) {
        BindGroupMock bindGroupMock(&mDevice);
        EXPECT_CALL(bindGroupMock, DestroyImpl).Times(1);

        EXPECT_TRUE(bindGroupMock.IsAlive());
        bindGroupMock.Destroy();
        EXPECT_FALSE(bindGroupMock.IsAlive());
    }

    // If the reference count on API objects reach 0, they should delete themselves. Note that GTest
    // will also complain if there is a memory leak.
    TEST_F(DestroyObjectTests, BindGroupImplicit) {
        BindGroupMock* bindGroupMock = new BindGroupMock(&mDevice);
        EXPECT_CALL(*bindGroupMock, DestroyImpl).Times(1);
        {
            BindGroupDescriptor desc = {};
            Ref<BindGroupBase> bindGroup;
            EXPECT_CALL(mDevice, CreateBindGroupImpl)
                .WillOnce(Return(ByMove(AcquireRef(bindGroupMock))));
            DAWN_ASSERT_AND_ASSIGN(bindGroup, mDevice.CreateBindGroup(&desc));

            EXPECT_TRUE(bindGroup->IsAlive());
        }
    }

    TEST_F(DestroyObjectTests, BindGroupLayoutExplicit) {
        BindGroupLayoutMock bindGroupLayoutMock(&mDevice);
        EXPECT_CALL(bindGroupLayoutMock, DestroyImpl).Times(1);

        EXPECT_TRUE(bindGroupLayoutMock.IsAlive());
        bindGroupLayoutMock.Destroy();
        EXPECT_FALSE(bindGroupLayoutMock.IsAlive());
    }

    // If the reference count on API objects reach 0, they should delete themselves. Note that GTest
    // will also complain if there is a memory leak.
    TEST_F(DestroyObjectTests, BindGroupLayoutImplicit) {
        BindGroupLayoutMock* bindGroupLayoutMock = new BindGroupLayoutMock(&mDevice);
        EXPECT_CALL(*bindGroupLayoutMock, DestroyImpl).Times(1);
        {
            BindGroupLayoutDescriptor desc = {};
            Ref<BindGroupLayoutBase> bindGroupLayout;
            EXPECT_CALL(mDevice, CreateBindGroupLayoutImpl)
                .WillOnce(Return(ByMove(AcquireRef(bindGroupLayoutMock))));
            DAWN_ASSERT_AND_ASSIGN(bindGroupLayout, mDevice.CreateBindGroupLayout(&desc));

            EXPECT_TRUE(bindGroupLayout->IsAlive());
            EXPECT_TRUE(bindGroupLayout->IsCachedReference());
        }
    }

    TEST_F(DestroyObjectTests, BufferExplicit) {
        {
            BufferMock bufferMock(&mDevice, BufferBase::BufferState::Unmapped);
            EXPECT_CALL(bufferMock, DestroyImpl).Times(1);

            EXPECT_TRUE(bufferMock.IsAlive());
            bufferMock.Destroy();
            EXPECT_FALSE(bufferMock.IsAlive());
        }
        {
            BufferMock bufferMock(&mDevice, BufferBase::BufferState::Mapped);
            {
                InSequence seq;
                EXPECT_CALL(bufferMock, DestroyImpl).Times(1);
                EXPECT_CALL(bufferMock, UnmapImpl).Times(1);
            }

            EXPECT_TRUE(bufferMock.IsAlive());
            bufferMock.Destroy();
            EXPECT_FALSE(bufferMock.IsAlive());
        }
    }

    // If the reference count on API objects reach 0, they should delete themselves. Note that GTest
    // will also complain if there is a memory leak.
    TEST_F(DestroyObjectTests, BufferImplicit) {
        {
            BufferMock* bufferMock = new BufferMock(&mDevice, BufferBase::BufferState::Unmapped);
            EXPECT_CALL(*bufferMock, DestroyImpl).Times(1);
            {
                BufferDescriptor desc = {};
                Ref<BufferBase> buffer;
                EXPECT_CALL(mDevice, CreateBufferImpl)
                    .WillOnce(Return(ByMove(AcquireRef(bufferMock))));
                DAWN_ASSERT_AND_ASSIGN(buffer, mDevice.CreateBuffer(&desc));

                EXPECT_TRUE(buffer->IsAlive());
            }
        }
        {
            BufferMock* bufferMock = new BufferMock(&mDevice, BufferBase::BufferState::Mapped);
            {
                InSequence seq;
                EXPECT_CALL(*bufferMock, DestroyImpl).Times(1);
                EXPECT_CALL(*bufferMock, UnmapImpl).Times(1);
            }
            {
                BufferDescriptor desc = {};
                Ref<BufferBase> buffer;
                EXPECT_CALL(mDevice, CreateBufferImpl)
                    .WillOnce(Return(ByMove(AcquireRef(bufferMock))));
                DAWN_ASSERT_AND_ASSIGN(buffer, mDevice.CreateBuffer(&desc));

                EXPECT_TRUE(buffer->IsAlive());
            }
        }
    }

    TEST_F(DestroyObjectTests, CommandBufferExplicit) {
        CommandBufferMock commandBufferMock(&mDevice);
        EXPECT_CALL(commandBufferMock, DestroyImpl).Times(1);

        EXPECT_TRUE(commandBufferMock.IsAlive());
        commandBufferMock.Destroy();
        EXPECT_FALSE(commandBufferMock.IsAlive());
    }

    // If the reference count on API objects reach 0, they should delete themselves. Note that GTest
    // will also complain if there is a memory leak.
    TEST_F(DestroyObjectTests, CommandBufferImplicit) {
        CommandBufferMock* commandBufferMock = new CommandBufferMock(&mDevice);
        EXPECT_CALL(*commandBufferMock, DestroyImpl).Times(1);
        {
            CommandBufferDescriptor desc = {};
            Ref<CommandBufferBase> commandBuffer;
            EXPECT_CALL(mDevice, CreateCommandBuffer)
                .WillOnce(Return(ByMove(AcquireRef(commandBufferMock))));
            DAWN_ASSERT_AND_ASSIGN(commandBuffer, mDevice.CreateCommandBuffer(nullptr, &desc));

            EXPECT_TRUE(commandBuffer->IsAlive());
        }
    }

    TEST_F(DestroyObjectTests, ComputePipelineExplicit) {
        ComputePipelineMock computePipelineMock(&mDevice);
        EXPECT_CALL(computePipelineMock, DestroyImpl).Times(1);

        EXPECT_TRUE(computePipelineMock.IsAlive());
        computePipelineMock.Destroy();
        EXPECT_FALSE(computePipelineMock.IsAlive());
    }

    // If the reference count on API objects reach 0, they should delete themselves. Note that GTest
    // will also complain if there is a memory leak.
    TEST_F(DestroyObjectTests, ComputePipelineImplicit) {
        // ComputePipelines usually set their hash values at construction, but the mock does not, so
        // we set it here.
        constexpr size_t hash = 0x12345;
        ComputePipelineMock* computePipelineMock = new ComputePipelineMock(&mDevice);
        computePipelineMock->SetContentHash(hash);
        ON_CALL(*computePipelineMock, ComputeContentHash).WillByDefault(Return(hash));

        // Compute pipelines are initialized during their creation via the device.
        EXPECT_CALL(*computePipelineMock, Initialize).Times(1);
        EXPECT_CALL(*computePipelineMock, DestroyImpl).Times(1);

        {
            ComputePipelineDescriptor desc = {};
            desc.layout = GetPipelineLayout().Get();
            desc.compute.module = GetComputeShaderModule().Get();

            Ref<ComputePipelineBase> computePipeline;
            EXPECT_CALL(mDevice, CreateUninitializedComputePipelineImpl)
                .WillOnce(Return(ByMove(AcquireRef(computePipelineMock))));
            DAWN_ASSERT_AND_ASSIGN(computePipeline, mDevice.CreateComputePipeline(&desc));

            EXPECT_TRUE(computePipeline->IsAlive());
            EXPECT_TRUE(computePipeline->IsCachedReference());
        }
    }

    TEST_F(DestroyObjectTests, ExternalTextureExplicit) {
        ExternalTextureMock externalTextureMock(&mDevice);
        EXPECT_CALL(externalTextureMock, DestroyImpl).Times(1);

        EXPECT_TRUE(externalTextureMock.IsAlive());
        externalTextureMock.Destroy();
        EXPECT_FALSE(externalTextureMock.IsAlive());
    }

    TEST_F(DestroyObjectTests, ExternalTextureImplicit) {
        ExternalTextureMock* externalTextureMock = new ExternalTextureMock(&mDevice);
        EXPECT_CALL(*externalTextureMock, DestroyImpl).Times(1);
        {
            ExternalTextureDescriptor desc = {};
            Ref<ExternalTextureBase> externalTexture;
            EXPECT_CALL(mDevice, CreateExternalTextureImpl)
                .WillOnce(Return(ByMove(AcquireRef(externalTextureMock))));
            DAWN_ASSERT_AND_ASSIGN(externalTexture, mDevice.CreateExternalTextureImpl(&desc));

            EXPECT_TRUE(externalTexture->IsAlive());
        }
    }

    TEST_F(DestroyObjectTests, PipelineLayoutExplicit) {
        PipelineLayoutMock pipelineLayoutMock(&mDevice);
        EXPECT_CALL(pipelineLayoutMock, DestroyImpl).Times(1);

        EXPECT_TRUE(pipelineLayoutMock.IsAlive());
        pipelineLayoutMock.Destroy();
        EXPECT_FALSE(pipelineLayoutMock.IsAlive());
    }

    // If the reference count on API objects reach 0, they should delete themselves. Note that GTest
    // will also complain if there is a memory leak.
    TEST_F(DestroyObjectTests, PipelineLayoutImplicit) {
        PipelineLayoutMock* pipelineLayoutMock = new PipelineLayoutMock(&mDevice);
        EXPECT_CALL(*pipelineLayoutMock, DestroyImpl).Times(1);
        {
            PipelineLayoutDescriptor desc = {};
            Ref<PipelineLayoutBase> pipelineLayout;
            EXPECT_CALL(mDevice, CreatePipelineLayoutImpl)
                .WillOnce(Return(ByMove(AcquireRef(pipelineLayoutMock))));
            DAWN_ASSERT_AND_ASSIGN(pipelineLayout, mDevice.CreatePipelineLayout(&desc));

            EXPECT_TRUE(pipelineLayout->IsAlive());
            EXPECT_TRUE(pipelineLayout->IsCachedReference());
        }
    }

    TEST_F(DestroyObjectTests, QuerySetExplicit) {
        QuerySetMock querySetMock(&mDevice);
        EXPECT_CALL(querySetMock, DestroyImpl).Times(1);

        EXPECT_TRUE(querySetMock.IsAlive());
        querySetMock.Destroy();
        EXPECT_FALSE(querySetMock.IsAlive());
    }

    // If the reference count on API objects reach 0, they should delete themselves. Note that GTest
    // will also complain if there is a memory leak.
    TEST_F(DestroyObjectTests, QuerySetImplicit) {
        QuerySetMock* querySetMock = new QuerySetMock(&mDevice);
        EXPECT_CALL(*querySetMock, DestroyImpl).Times(1);
        {
            QuerySetDescriptor desc = {};
            Ref<QuerySetBase> querySet;
            EXPECT_CALL(mDevice, CreateQuerySetImpl)
                .WillOnce(Return(ByMove(AcquireRef(querySetMock))));
            DAWN_ASSERT_AND_ASSIGN(querySet, mDevice.CreateQuerySet(&desc));

            EXPECT_TRUE(querySet->IsAlive());
        }
    }

    TEST_F(DestroyObjectTests, RenderPipelineExplicit) {
        RenderPipelineMock renderPipelineMock(&mDevice);
        EXPECT_CALL(renderPipelineMock, DestroyImpl).Times(1);

        EXPECT_TRUE(renderPipelineMock.IsAlive());
        renderPipelineMock.Destroy();
        EXPECT_FALSE(renderPipelineMock.IsAlive());
    }

    // If the reference count on API objects reach 0, they should delete themselves. Note that GTest
    // will also complain if there is a memory leak.
    TEST_F(DestroyObjectTests, RenderPipelineImplicit) {
        // RenderPipelines usually set their hash values at construction, but the mock does not, so
        // we set it here.
        constexpr size_t hash = 0x12345;
        RenderPipelineMock* renderPipelineMock = new RenderPipelineMock(&mDevice);
        renderPipelineMock->SetContentHash(hash);
        ON_CALL(*renderPipelineMock, ComputeContentHash).WillByDefault(Return(hash));

        // Render pipelines are initialized during their creation via the device.
        EXPECT_CALL(*renderPipelineMock, Initialize).Times(1);
        EXPECT_CALL(*renderPipelineMock, DestroyImpl).Times(1);

        {
            RenderPipelineDescriptor desc = {};
            desc.layout = GetPipelineLayout().Get();
            desc.vertex.module = GetVertexShaderModule().Get();

            Ref<RenderPipelineBase> renderPipeline;
            EXPECT_CALL(mDevice, CreateUninitializedRenderPipelineImpl)
                .WillOnce(Return(ByMove(AcquireRef(renderPipelineMock))));
            DAWN_ASSERT_AND_ASSIGN(renderPipeline, mDevice.CreateRenderPipeline(&desc));

            EXPECT_TRUE(renderPipeline->IsAlive());
            EXPECT_TRUE(renderPipeline->IsCachedReference());
        }
    }

    TEST_F(DestroyObjectTests, SamplerExplicit) {
        SamplerMock samplerMock(&mDevice);
        EXPECT_CALL(samplerMock, DestroyImpl).Times(1);

        EXPECT_TRUE(samplerMock.IsAlive());
        samplerMock.Destroy();
        EXPECT_FALSE(samplerMock.IsAlive());
    }

    // If the reference count on API objects reach 0, they should delete themselves. Note that GTest
    // will also complain if there is a memory leak.
    TEST_F(DestroyObjectTests, SamplerImplicit) {
        SamplerMock* samplerMock = new SamplerMock(&mDevice);
        EXPECT_CALL(*samplerMock, DestroyImpl).Times(1);
        {
            SamplerDescriptor desc = {};
            Ref<SamplerBase> sampler;
            EXPECT_CALL(mDevice, CreateSamplerImpl)
                .WillOnce(Return(ByMove(AcquireRef(samplerMock))));
            DAWN_ASSERT_AND_ASSIGN(sampler, mDevice.CreateSampler(&desc));

            EXPECT_TRUE(sampler->IsAlive());
            EXPECT_TRUE(sampler->IsCachedReference());
        }
    }

    TEST_F(DestroyObjectTests, ShaderModuleExplicit) {
        ShaderModuleMock shaderModuleMock(&mDevice);
        EXPECT_CALL(shaderModuleMock, DestroyImpl).Times(1);

        EXPECT_TRUE(shaderModuleMock.IsAlive());
        shaderModuleMock.Destroy();
        EXPECT_FALSE(shaderModuleMock.IsAlive());
    }

    // If the reference count on API objects reach 0, they should delete themselves. Note that GTest
    // will also complain if there is a memory leak.
    TEST_F(DestroyObjectTests, ShaderModuleImplicit) {
        ShaderModuleMock* shaderModuleMock = new ShaderModuleMock(&mDevice);
        EXPECT_CALL(*shaderModuleMock, DestroyImpl).Times(1);
        {
            ShaderModuleWGSLDescriptor wgslDesc;
            wgslDesc.source = R"(
                @stage(compute) @workgroup_size(1) fn main() {
                }
            )";
            ShaderModuleDescriptor desc = {};
            desc.nextInChain = &wgslDesc;
            Ref<ShaderModuleBase> shaderModule;
            EXPECT_CALL(mDevice, CreateShaderModuleImpl)
                .WillOnce(Return(ByMove(AcquireRef(shaderModuleMock))));
            DAWN_ASSERT_AND_ASSIGN(shaderModule, mDevice.CreateShaderModule(&desc));

            EXPECT_TRUE(shaderModule->IsAlive());
            EXPECT_TRUE(shaderModule->IsCachedReference());
        }
    }

    TEST_F(DestroyObjectTests, SwapChainExplicit) {
        SwapChainMock swapChainMock(&mDevice);
        EXPECT_CALL(swapChainMock, DestroyImpl).Times(1);

        EXPECT_TRUE(swapChainMock.IsAlive());
        swapChainMock.Destroy();
        EXPECT_FALSE(swapChainMock.IsAlive());
    }

    // If the reference count on API objects reach 0, they should delete themselves. Note that GTest
    // will also complain if there is a memory leak.
    TEST_F(DestroyObjectTests, SwapChainImplicit) {
        SwapChainMock* swapChainMock = new SwapChainMock(&mDevice);
        EXPECT_CALL(*swapChainMock, DestroyImpl).Times(1);
        {
            SwapChainDescriptor desc = {};
            Ref<SwapChainBase> swapChain;
            EXPECT_CALL(mDevice, CreateSwapChainImpl(_))
                .WillOnce(Return(ByMove(AcquireRef(swapChainMock))));
            DAWN_ASSERT_AND_ASSIGN(swapChain, mDevice.CreateSwapChain(nullptr, &desc));

            EXPECT_TRUE(swapChain->IsAlive());
        }
    }

    TEST_F(DestroyObjectTests, TextureExplicit) {
        {
            TextureMock textureMock(&mDevice, TextureBase::TextureState::OwnedInternal);
            EXPECT_CALL(textureMock, DestroyImpl).Times(1);

            EXPECT_TRUE(textureMock.IsAlive());
            textureMock.Destroy();
            EXPECT_FALSE(textureMock.IsAlive());
        }
        {
            TextureMock textureMock(&mDevice, TextureBase::TextureState::OwnedExternal);
            EXPECT_CALL(textureMock, DestroyImpl).Times(1);

            EXPECT_TRUE(textureMock.IsAlive());
            textureMock.Destroy();
            EXPECT_FALSE(textureMock.IsAlive());
        }
    }

    // If the reference count on API objects reach 0, they should delete themselves. Note that GTest
    // will also complain if there is a memory leak.
    TEST_F(DestroyObjectTests, TextureImplicit) {
        {
            TextureMock* textureMock =
                new TextureMock(&mDevice, TextureBase::TextureState::OwnedInternal);
            EXPECT_CALL(*textureMock, DestroyImpl).Times(1);
            {
                TextureDescriptor desc = {};
                Ref<TextureBase> texture;
                EXPECT_CALL(mDevice, CreateTextureImpl)
                    .WillOnce(Return(ByMove(AcquireRef(textureMock))));
                DAWN_ASSERT_AND_ASSIGN(texture, mDevice.CreateTexture(&desc));

                EXPECT_TRUE(texture->IsAlive());
            }
        }
        {
            TextureMock* textureMock =
                new TextureMock(&mDevice, TextureBase::TextureState::OwnedExternal);
            EXPECT_CALL(*textureMock, DestroyImpl).Times(1);
            {
                TextureDescriptor desc = {};
                Ref<TextureBase> texture;
                EXPECT_CALL(mDevice, CreateTextureImpl)
                    .WillOnce(Return(ByMove(AcquireRef(textureMock))));
                DAWN_ASSERT_AND_ASSIGN(texture, mDevice.CreateTexture(&desc));

                EXPECT_TRUE(texture->IsAlive());
            }
        }
    }

    TEST_F(DestroyObjectTests, TextureViewExplicit) {
        TextureViewMock textureViewMock(GetTexture().Get());
        EXPECT_CALL(textureViewMock, DestroyImpl).Times(1);

        EXPECT_TRUE(textureViewMock.IsAlive());
        textureViewMock.Destroy();
        EXPECT_FALSE(textureViewMock.IsAlive());
    }

    // If the reference count on API objects reach 0, they should delete themselves. Note that GTest
    // will also complain if there is a memory leak.
    TEST_F(DestroyObjectTests, TextureViewImplicit) {
        TextureViewMock* textureViewMock = new TextureViewMock(GetTexture().Get());
        EXPECT_CALL(*textureViewMock, DestroyImpl).Times(1);
        {
            TextureViewDescriptor desc = {};
            Ref<TextureViewBase> textureView;
            EXPECT_CALL(mDevice, CreateTextureViewImpl)
                .WillOnce(Return(ByMove(AcquireRef(textureViewMock))));
            DAWN_ASSERT_AND_ASSIGN(textureView,
                                   mDevice.CreateTextureView(GetTexture().Get(), &desc));

            EXPECT_TRUE(textureView->IsAlive());
        }
    }

    // Destroying the objects on the mDevice should result in all created objects being destroyed in
    // order.
    TEST_F(DestroyObjectTests, DestroyObjects) {
        BindGroupMock* bindGroupMock = new BindGroupMock(&mDevice);
        BindGroupLayoutMock* bindGroupLayoutMock = new BindGroupLayoutMock(&mDevice);
        BufferMock* bufferMock = new BufferMock(&mDevice, BufferBase::BufferState::Unmapped);
        CommandBufferMock* commandBufferMock = new CommandBufferMock(&mDevice);
        ComputePipelineMock* computePipelineMock = new ComputePipelineMock(&mDevice);
        ExternalTextureMock* externalTextureMock = new ExternalTextureMock(&mDevice);
        PipelineLayoutMock* pipelineLayoutMock = new PipelineLayoutMock(&mDevice);
        QuerySetMock* querySetMock = new QuerySetMock(&mDevice);
        RenderPipelineMock* renderPipelineMock = new RenderPipelineMock(&mDevice);
        SamplerMock* samplerMock = new SamplerMock(&mDevice);
        ShaderModuleMock* shaderModuleMock = new ShaderModuleMock(&mDevice);
        SwapChainMock* swapChainMock = new SwapChainMock(&mDevice);
        TextureMock* textureMock =
            new TextureMock(&mDevice, TextureBase::TextureState::OwnedInternal);
        TextureViewMock* textureViewMock = new TextureViewMock(GetTexture().Get());
        {
            InSequence seq;
            EXPECT_CALL(*commandBufferMock, DestroyImpl).Times(1);
            EXPECT_CALL(*renderPipelineMock, DestroyImpl).Times(1);
            EXPECT_CALL(*computePipelineMock, DestroyImpl).Times(1);
            EXPECT_CALL(*pipelineLayoutMock, DestroyImpl).Times(1);
            EXPECT_CALL(*swapChainMock, DestroyImpl).Times(1);
            EXPECT_CALL(*bindGroupMock, DestroyImpl).Times(1);
            EXPECT_CALL(*bindGroupLayoutMock, DestroyImpl).Times(1);
            EXPECT_CALL(*shaderModuleMock, DestroyImpl).Times(1);
            EXPECT_CALL(*externalTextureMock, DestroyImpl).Times(1);
            EXPECT_CALL(*textureViewMock, DestroyImpl).Times(1);
            EXPECT_CALL(*textureMock, DestroyImpl).Times(1);
            EXPECT_CALL(*querySetMock, DestroyImpl).Times(1);
            EXPECT_CALL(*samplerMock, DestroyImpl).Times(1);
            EXPECT_CALL(*bufferMock, DestroyImpl).Times(1);
        }

        Ref<BindGroupBase> bindGroup;
        {
            BindGroupDescriptor desc = {};
            EXPECT_CALL(mDevice, CreateBindGroupImpl)
                .WillOnce(Return(ByMove(AcquireRef(bindGroupMock))));
            DAWN_ASSERT_AND_ASSIGN(bindGroup, mDevice.CreateBindGroup(&desc));
            EXPECT_TRUE(bindGroup->IsAlive());
        }

        Ref<BindGroupLayoutBase> bindGroupLayout;
        {
            BindGroupLayoutDescriptor desc = {};
            EXPECT_CALL(mDevice, CreateBindGroupLayoutImpl)
                .WillOnce(Return(ByMove(AcquireRef(bindGroupLayoutMock))));
            DAWN_ASSERT_AND_ASSIGN(bindGroupLayout, mDevice.CreateBindGroupLayout(&desc));
            EXPECT_TRUE(bindGroupLayout->IsAlive());
            EXPECT_TRUE(bindGroupLayout->IsCachedReference());
        }

        Ref<BufferBase> buffer;
        {
            BufferDescriptor desc = {};
            EXPECT_CALL(mDevice, CreateBufferImpl).WillOnce(Return(ByMove(AcquireRef(bufferMock))));
            DAWN_ASSERT_AND_ASSIGN(buffer, mDevice.CreateBuffer(&desc));
            EXPECT_TRUE(buffer->IsAlive());
        }

        Ref<CommandBufferBase> commandBuffer;
        {
            CommandBufferDescriptor desc = {};
            EXPECT_CALL(mDevice, CreateCommandBuffer)
                .WillOnce(Return(ByMove(AcquireRef(commandBufferMock))));
            DAWN_ASSERT_AND_ASSIGN(commandBuffer, mDevice.CreateCommandBuffer(nullptr, &desc));
            EXPECT_TRUE(commandBuffer->IsAlive());
        }

        Ref<ComputePipelineBase> computePipeline;
        {
            // Compute pipelines usually set their hash values at construction, but the mock does
            // not, so we set it here.
            constexpr size_t hash = 0x12345;
            computePipelineMock->SetContentHash(hash);
            ON_CALL(*computePipelineMock, ComputeContentHash).WillByDefault(Return(hash));

            // Compute pipelines are initialized during their creation via the device.
            EXPECT_CALL(*computePipelineMock, Initialize).Times(1);

            ComputePipelineDescriptor desc = {};
            desc.layout = GetPipelineLayout().Get();
            desc.compute.module = GetComputeShaderModule().Get();
            EXPECT_CALL(mDevice, CreateUninitializedComputePipelineImpl)
                .WillOnce(Return(ByMove(AcquireRef(computePipelineMock))));
            DAWN_ASSERT_AND_ASSIGN(computePipeline, mDevice.CreateComputePipeline(&desc));
            EXPECT_TRUE(computePipeline->IsAlive());
            EXPECT_TRUE(computePipeline->IsCachedReference());
        }

        Ref<ExternalTextureBase> externalTexture;
        {
            ExternalTextureDescriptor desc = {};
            EXPECT_CALL(mDevice, CreateExternalTextureImpl)
                .WillOnce(Return(ByMove(AcquireRef(externalTextureMock))));
            DAWN_ASSERT_AND_ASSIGN(externalTexture, mDevice.CreateExternalTextureImpl(&desc));
            EXPECT_TRUE(externalTexture->IsAlive());
        }

        Ref<PipelineLayoutBase> pipelineLayout;
        {
            PipelineLayoutDescriptor desc = {};
            EXPECT_CALL(mDevice, CreatePipelineLayoutImpl)
                .WillOnce(Return(ByMove(AcquireRef(pipelineLayoutMock))));
            DAWN_ASSERT_AND_ASSIGN(pipelineLayout, mDevice.CreatePipelineLayout(&desc));
            EXPECT_TRUE(pipelineLayout->IsAlive());
            EXPECT_TRUE(pipelineLayout->IsCachedReference());
        }

        Ref<QuerySetBase> querySet;
        {
            QuerySetDescriptor desc = {};
            EXPECT_CALL(mDevice, CreateQuerySetImpl)
                .WillOnce(Return(ByMove(AcquireRef(querySetMock))));
            DAWN_ASSERT_AND_ASSIGN(querySet, mDevice.CreateQuerySet(&desc));
            EXPECT_TRUE(querySet->IsAlive());
        }

        Ref<RenderPipelineBase> renderPipeline;
        {
            // Render pipelines usually set their hash values at construction, but the mock does
            // not, so we set it here.
            constexpr size_t hash = 0x12345;
            renderPipelineMock->SetContentHash(hash);
            ON_CALL(*renderPipelineMock, ComputeContentHash).WillByDefault(Return(hash));

            // Render pipelines are initialized during their creation via the device.
            EXPECT_CALL(*renderPipelineMock, Initialize).Times(1);

            RenderPipelineDescriptor desc = {};
            desc.layout = GetPipelineLayout().Get();
            desc.vertex.module = GetVertexShaderModule().Get();
            EXPECT_CALL(mDevice, CreateUninitializedRenderPipelineImpl)
                .WillOnce(Return(ByMove(AcquireRef(renderPipelineMock))));
            DAWN_ASSERT_AND_ASSIGN(renderPipeline, mDevice.CreateRenderPipeline(&desc));
            EXPECT_TRUE(renderPipeline->IsAlive());
            EXPECT_TRUE(renderPipeline->IsCachedReference());
        }

        Ref<SamplerBase> sampler;
        {
            SamplerDescriptor desc = {};
            EXPECT_CALL(mDevice, CreateSamplerImpl)
                .WillOnce(Return(ByMove(AcquireRef(samplerMock))));
            DAWN_ASSERT_AND_ASSIGN(sampler, mDevice.CreateSampler(&desc));
            EXPECT_TRUE(sampler->IsAlive());
            EXPECT_TRUE(sampler->IsCachedReference());
        }

        Ref<ShaderModuleBase> shaderModule;
        {
            ShaderModuleWGSLDescriptor wgslDesc;
            wgslDesc.source = R"(
                @stage(compute) @workgroup_size(1) fn main() {
                }
            )";
            ShaderModuleDescriptor desc = {};
            desc.nextInChain = &wgslDesc;

            EXPECT_CALL(mDevice, CreateShaderModuleImpl)
                .WillOnce(Return(ByMove(AcquireRef(shaderModuleMock))));
            DAWN_ASSERT_AND_ASSIGN(shaderModule, mDevice.CreateShaderModule(&desc));
            EXPECT_TRUE(shaderModule->IsAlive());
            EXPECT_TRUE(shaderModule->IsCachedReference());
        }

        Ref<SwapChainBase> swapChain;
        {
            SwapChainDescriptor desc = {};
            EXPECT_CALL(mDevice, CreateSwapChainImpl(_))
                .WillOnce(Return(ByMove(AcquireRef(swapChainMock))));
            DAWN_ASSERT_AND_ASSIGN(swapChain, mDevice.CreateSwapChain(nullptr, &desc));
            EXPECT_TRUE(swapChain->IsAlive());
        }

        Ref<TextureBase> texture;
        {
            TextureDescriptor desc = {};
            EXPECT_CALL(mDevice, CreateTextureImpl)
                .WillOnce(Return(ByMove(AcquireRef(textureMock))));
            DAWN_ASSERT_AND_ASSIGN(texture, mDevice.CreateTexture(&desc));
            EXPECT_TRUE(texture->IsAlive());
        }

        Ref<TextureViewBase> textureView;
        {
            TextureViewDescriptor desc = {};
            EXPECT_CALL(mDevice, CreateTextureViewImpl)
                .WillOnce(Return(ByMove(AcquireRef(textureViewMock))));
            DAWN_ASSERT_AND_ASSIGN(textureView,
                                   mDevice.CreateTextureView(GetTexture().Get(), &desc));
            EXPECT_TRUE(textureView->IsAlive());
        }

        mDevice.DestroyObjects();
        EXPECT_FALSE(bindGroup->IsAlive());
        EXPECT_FALSE(bindGroupLayout->IsAlive());
        EXPECT_FALSE(buffer->IsAlive());
        EXPECT_FALSE(commandBuffer->IsAlive());
        EXPECT_FALSE(computePipeline->IsAlive());
        EXPECT_FALSE(externalTexture->IsAlive());
        EXPECT_FALSE(pipelineLayout->IsAlive());
        EXPECT_FALSE(querySet->IsAlive());
        EXPECT_FALSE(renderPipeline->IsAlive());
        EXPECT_FALSE(sampler->IsAlive());
        EXPECT_FALSE(shaderModule->IsAlive());
        EXPECT_FALSE(swapChain->IsAlive());
        EXPECT_FALSE(texture->IsAlive());
        EXPECT_FALSE(textureView->IsAlive());
    }

    static constexpr std::string_view kComputeShader = R"(
        @stage(compute) @workgroup_size(1) fn main() {}
    )";

    static constexpr std::string_view kVertexShader = R"(
        @stage(vertex) fn main() -> @builtin(position) vec4<f32> {
            return vec4<f32>(0.0, 0.0, 0.0, 0.0);
        }
    )";

    static constexpr std::string_view kFragmentShader = R"(
        @stage(fragment) fn main() {}
    )";

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
        utils::BasicRenderPass pass = utils::CreateBasicRenderPass(device, 1, 1);

        utils::ComboRenderPassDescriptor passDesc{};
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderEncoder = encoder.BeginRenderPass(&pass.renderPassInfo);

        utils::ComboRenderPipelineDescriptor pipelineDesc;
        pipelineDesc.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        pipelineDesc.vertex.module = utils::CreateShaderModule(device, kVertexShader.data());
        pipelineDesc.vertex.entryPoint = "main";
        pipelineDesc.cFragment.module = utils::CreateShaderModule(device, kFragmentShader.data());
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
        pipelineDesc.compute.module = utils::CreateShaderModule(device, kComputeShader.data());
        pipelineDesc.compute.entryPoint = "main";
        computeEncoder.SetPipeline(device.CreateComputePipeline(&pipelineDesc));

        device.Destroy();
    }

    // TODO(https://crbug.com/dawn/1381) Remove when namespaces are not indented.
    // NOLINTNEXTLINE(readability/namespace)
}}  // namespace dawn::native::
