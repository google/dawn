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

#include "tests/perf_tests/DawnPerfTest.h"

#include "tests/ParamGenerator.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

struct SubresourceTrackingParams : AdapterTestParam {
    SubresourceTrackingParams(const AdapterTestParam& param,
                              uint32_t arrayLayerCountIn,
                              uint32_t mipLevelCountIn)
        : AdapterTestParam(param),
          arrayLayerCount(arrayLayerCountIn),
          mipLevelCount(mipLevelCountIn) {
    }
    uint32_t arrayLayerCount;
    uint32_t mipLevelCount;
};

std::ostream& operator<<(std::ostream& ostream, const SubresourceTrackingParams& param) {
    ostream << static_cast<const AdapterTestParam&>(param);
    ostream << "_arrayLayer_" << param.arrayLayerCount;
    ostream << "_mipLevel_" << param.mipLevelCount;
    return ostream;
}

// Test the performance of Subresource usage and barrier tracking on a case that would generally be
// difficult. It uses a 2D array texture with mipmaps and updates one of the layers with data from
// another texture, then generates mipmaps for that layer. It is difficult because it requires
// tracking the state of individual subresources in the middle of the subresources of that texture.
class SubresourceTrackingPerf : public DawnPerfTestWithParams<SubresourceTrackingParams> {
  public:
    static constexpr unsigned int kNumIterations = 50;

    SubresourceTrackingPerf() : DawnPerfTestWithParams(kNumIterations, 1) {
    }
    ~SubresourceTrackingPerf() override = default;

    void SetUp() override {
        DawnPerfTestWithParams<SubresourceTrackingParams>::SetUp();
        const SubresourceTrackingParams& params = GetParam();

        wgpu::TextureDescriptor materialDesc;
        materialDesc.dimension = wgpu::TextureDimension::e2D;
        materialDesc.size = {1u << (params.mipLevelCount - 1), 1u << (params.mipLevelCount - 1),
                             params.arrayLayerCount};
        materialDesc.mipLevelCount = params.mipLevelCount;
        materialDesc.usage = wgpu::TextureUsage::Sampled | wgpu::TextureUsage::RenderAttachment |
                             wgpu::TextureUsage::CopyDst;
        materialDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        mMaterials = device.CreateTexture(&materialDesc);

        wgpu::TextureDescriptor uploadTexDesc = materialDesc;
        uploadTexDesc.size.depth = 1;
        uploadTexDesc.mipLevelCount = 1;
        uploadTexDesc.usage = wgpu::TextureUsage::CopySrc;
        mUploadTexture = device.CreateTexture(&uploadTexDesc);

        utils::ComboRenderPipelineDescriptor pipelineDesc(device);
        pipelineDesc.vertexStage.module = utils::CreateShaderModuleFromWGSL(device, R"(
            [[builtin(position)]] var<out> Position : vec4<f32>;
            [[stage(vertex)]] fn main() -> void {
                Position = vec4<f32>(1.0, 0.0, 0.0, 1.0);
            }
        )");
        pipelineDesc.cFragmentStage.module = utils::CreateShaderModuleFromWGSL(device, R"(
            [[location(0)]] var<out> FragColor : vec4<f32>;
            [[set(0), binding(0)]] var<uniform_constant> materials : texture_sampled_2d<f32>;
            [[stage(fragment)]] fn main() -> void {
                FragColor = vec4<f32>(1.0, 0.0, 0.0, 1.0);
            }
        )");
        mPipeline = device.CreateRenderPipeline(&pipelineDesc);
    }

  private:
    void Step() override {
        const SubresourceTrackingParams& params = GetParam();

        uint32_t layerUploaded = params.arrayLayerCount / 2;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        // Copy into the layer of the material array.
        {
            wgpu::TextureCopyView sourceView;
            sourceView.texture = mUploadTexture;

            wgpu::TextureCopyView destView;
            destView.texture = mMaterials;
            destView.origin.z = layerUploaded;

            wgpu::Extent3D copySize = {1u << (params.mipLevelCount - 1),
                                       1u << (params.mipLevelCount - 1), 1};

            encoder.CopyTextureToTexture(&sourceView, &destView, &copySize);
        }

        // Fake commands that would be used to create the mip levels.
        for (uint32_t level = 1; level < params.mipLevelCount; level++) {
            wgpu::TextureViewDescriptor rtViewDesc;
            rtViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            rtViewDesc.baseMipLevel = level;
            rtViewDesc.mipLevelCount = 1;
            rtViewDesc.baseArrayLayer = layerUploaded;
            rtViewDesc.arrayLayerCount = 1;
            wgpu::TextureView rtView = mMaterials.CreateView(&rtViewDesc);

            wgpu::TextureViewDescriptor sampleViewDesc = rtViewDesc;
            sampleViewDesc.baseMipLevel = level - 1;
            wgpu::TextureView sampleView = mMaterials.CreateView(&sampleViewDesc);

            wgpu::BindGroup bindgroup =
                utils::MakeBindGroup(device, mPipeline.GetBindGroupLayout(0), {{0, sampleView}});

            utils::ComboRenderPassDescriptor renderPass({rtView});
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
            pass.SetPipeline(mPipeline);
            pass.SetBindGroup(0, bindgroup);
            pass.Draw(3);
            pass.EndPass();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    wgpu::Texture mUploadTexture;
    wgpu::Texture mMaterials;
    wgpu::RenderPipeline mPipeline;
};

TEST_P(SubresourceTrackingPerf, Run) {
    RunTest();
}

DAWN_INSTANTIATE_PERF_TEST_SUITE_P(SubresourceTrackingPerf,
                                   {D3D12Backend(), MetalBackend(), OpenGLBackend(),
                                    VulkanBackend()},
                                   {1, 4, 16, 256},
                                   {2, 3, 8});
