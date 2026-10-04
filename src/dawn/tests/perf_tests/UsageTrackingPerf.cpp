// Copyright 2026 The Dawn & Tint Authors
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

// Measures the CPU cost of Dawn's per-command resource usage tracking, i.e. the containers in
// PassResourceUsage.h / PassResourceUsageTracker.h that are populated while encoding commands and
// consumed during Queue::Submit validation. In particular, these tests attempt to measure the
// performance impact of using raw_ptr in these tracking containers.
//
// These tests are meant to be run on the Null backend so that no GPU work dilutes the measurement:
//   dawn_perf_tests --gtest_filter='*UsageTrackingPerf*' --backend=null

#include <vector>

#include "src/dawn/tests/perf_tests/DawnPerfTest.h"
#include "src/dawn/utils/ComboRenderPipelineDescriptor.h"
#include "src/dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

// Number of distinct resources cycled through, so the tracking containers actually grow instead
// of repeatedly hitting the same key.
constexpr uint32_t kNumResources = 64;
// Large enough for a 4x4 RGBA8 buffer->texture copy, which requires a 256-byte aligned
// bytesPerRow for each of the 4 rows.
constexpr uint64_t kBufferSize = 1024;

enum class Workload {
    // Top-level (outside of any pass) buffer copies. Exercises
    // CommandBufferResourceUsage::topLevelBuffers.
    BufferCopies,
    // Top-level buffer<->texture copies. Exercises topLevelBuffers and topLevelTextures.
    TextureCopies,
    // Compute passes binding many bind groups. Exercises
    // ComputePassResourceUsage::referencedBuffers.
    ComputeBindGroups,
    // Render passes with draws. Control: this path's containers do not use raw_ptr, as of this
    // writing.
    RenderDraws,
};

std::ostream& operator<<(std::ostream& ostream, Workload workload) {
    switch (workload) {
        case Workload::BufferCopies:
            ostream << "BufferCopies";
            break;
        case Workload::TextureCopies:
            ostream << "TextureCopies";
            break;
        case Workload::ComputeBindGroups:
            ostream << "ComputeBindGroups";
            break;
        case Workload::RenderDraws:
            ostream << "RenderDraws";
            break;
    }
    return ostream;
}

struct UsageTrackingParams : AdapterTestParam {
    UsageTrackingParams(const AdapterTestParam& param, Workload workloadIn, uint32_t commandCountIn)
        : AdapterTestParam(param), workload(workloadIn), commandCount(commandCountIn) {}
    Workload workload;
    uint32_t commandCount;
};

std::ostream& operator<<(std::ostream& ostream, const UsageTrackingParams& param) {
    ostream << static_cast<const AdapterTestParam&>(param);
    ostream << "_" << param.workload;
    ostream << "_commands_" << param.commandCount;
    return ostream;
}

class UsageTrackingPerf : public DawnPerfTestWithParams<UsageTrackingParams> {
  public:
    UsageTrackingPerf() : DawnPerfTestWithParams(GetParam().commandCount, 1) {}
    ~UsageTrackingPerf() override = default;

  protected:
    void SetUpPerfTest() override {
        const UsageTrackingParams& params = GetParam();

        switch (params.workload) {
            case Workload::BufferCopies:
                CreateBuffers(wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);
                break;

            case Workload::TextureCopies:
                CreateBuffers(wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);
                CreateTextures();
                break;

            case Workload::ComputeBindGroups: {
                CreateBuffers(wgpu::BufferUsage::Storage);

                wgpu::ComputePipelineDescriptor pipelineDesc;
                pipelineDesc.compute.module = utils::CreateShaderModule(device, R"(
                    @group(0) @binding(0) var<storage, read_write> data : array<u32>;
                    @compute @workgroup_size(1) fn main() {
                        data[0] = data[0] + 1u;
                    })");
                mComputePipeline = device.CreateComputePipeline(&pipelineDesc);

                for (uint32_t i = 0; i < kNumResources; ++i) {
                    mBindGroups.push_back(utils::MakeBindGroup(
                        device, mComputePipeline.GetBindGroupLayout(0), {{0, mBuffers[i]}}));
                }
                break;
            }

            case Workload::RenderDraws: {
                CreateTextures();

                utils::ComboRenderPipelineDescriptor pipelineDesc;
                pipelineDesc.vertex.module = utils::CreateShaderModule(device, R"(
                    @vertex fn main() -> @builtin(position) vec4f {
                        return vec4f(0.0, 0.0, 0.0, 1.0);
                    })");
                pipelineDesc.cFragment.module = utils::CreateShaderModule(device, R"(
                    @fragment fn main() -> @location(0) vec4f {
                        return vec4f(0.0, 1.0, 0.0, 1.0);
                    })");
                pipelineDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
                mRenderPipeline = device.CreateRenderPipeline(&pipelineDesc);
                break;
            }
        }
    }

  private:
    void CreateBuffers(wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor desc;
        desc.size = kBufferSize;
        desc.usage = usage;
        for (uint32_t i = 0; i < kNumResources; ++i) {
            mBuffers.push_back(device.CreateBuffer(&desc));
        }
    }

    void CreateTextures() {
        wgpu::TextureDescriptor desc;
        desc.dimension = wgpu::TextureDimension::e2D;
        desc.size = {4, 4, 1};
        desc.format = wgpu::TextureFormat::RGBA8Unorm;
        desc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst |
                     wgpu::TextureUsage::RenderAttachment;
        for (uint32_t i = 0; i < kNumResources; ++i) {
            mTextures.push_back(device.CreateTexture(&desc));
        }
    }

    void Step() override {
        const UsageTrackingParams& params = GetParam();
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        switch (params.workload) {
            case Workload::BufferCopies:
                for (uint32_t i = 0; i < params.commandCount; ++i) {
                    encoder.CopyBufferToBuffer(mBuffers[i % kNumResources], 0,
                                               mBuffers[(i + 1) % kNumResources], 0, kBufferSize);
                }
                break;

            case Workload::TextureCopies:
                for (uint32_t i = 0; i < params.commandCount; ++i) {
                    wgpu::TexelCopyBufferInfo src =
                        utils::CreateTexelCopyBufferInfo(mBuffers[i % kNumResources], 0, 256);
                    wgpu::TexelCopyTextureInfo dst =
                        utils::CreateTexelCopyTextureInfo(mTextures[i % kNumResources]);
                    wgpu::Extent3D copySize = {4, 4, 1};
                    encoder.CopyBufferToTexture(&src, &dst, &copySize);
                }
                break;

            case Workload::ComputeBindGroups: {
                wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
                pass.SetPipeline(mComputePipeline);
                for (uint32_t i = 0; i < params.commandCount; ++i) {
                    pass.SetBindGroup(0, mBindGroups[i % kNumResources]);
                    pass.DispatchWorkgroups(1);
                }
                pass.End();
                break;
            }

            case Workload::RenderDraws: {
                for (uint32_t i = 0; i < params.commandCount; ++i) {
                    utils::ComboRenderPassDescriptor renderPass(
                        {mTextures[i % kNumResources].CreateView()});
                    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
                    pass.SetPipeline(mRenderPipeline);
                    pass.Draw(3);
                    pass.End();
                }
                break;
            }
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    std::vector<wgpu::Buffer> mBuffers;
    std::vector<wgpu::Texture> mTextures;
    std::vector<wgpu::BindGroup> mBindGroups;
    wgpu::ComputePipeline mComputePipeline;
    wgpu::RenderPipeline mRenderPipeline;
};

TEST_P(UsageTrackingPerf, Run) {
    RunTest();
}

DAWN_INSTANTIATE_TEST_P(UsageTrackingPerf,
                        {D3D11Backend(), D3D12Backend(), MetalBackend(), NullBackend(),
                         OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
                        {Workload::BufferCopies, Workload::TextureCopies,
                         Workload::ComputeBindGroups, Workload::RenderDraws},
                        {500u});

}  // anonymous namespace
}  // namespace dawn
