// Copyright 2025 The Dawn & Tint Authors
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

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class SizedBindingArrayTests : public DawnTest {
  public:
    wgpu::Texture MakeTestR8Texture(uint8_t value) {
        wgpu::TextureDescriptor desc;
        desc.size = {1, 1};
        desc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
        desc.format = wgpu::TextureFormat::R8Unorm;
        wgpu::Texture texture = device.CreateTexture(&desc);

        wgpu::TexelCopyTextureInfo srcInfo = utils::CreateTexelCopyTextureInfo(texture);
        wgpu::TexelCopyBufferLayout dstInfo = {};
        wgpu::Extent3D copySize = {1, 1, 1};
        queue.WriteTexture(&srcInfo, &value, 1, &dstInfo, &copySize);

        return texture;
    }
};

// Test accessing a binding_array with constant values.
TEST_P(SizedBindingArrayTests, IndexingWithConstants) {
    // Make the test pipeline
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0.5, 0.5);
        }

        @group(0) @binding(0) var ts : binding_array<texture_2d<f32>, 4>;
        @fragment fn fs() -> @location(0) vec4f {
            let r = textureLoad(ts[0], vec2(0, 0), 0)[0];
            let g = textureLoad(ts[1], vec2(0, 0), 0)[0];
            let b = textureLoad(ts[2], vec2(0, 0), 0)[0];
            let a = textureLoad(ts[3], vec2(0, 0), 0)[0];
            return vec4(r, g, b, a);
        }
    )");

    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.vertex.module = module;
    pDesc.cFragment.module = module;
    pDesc.cFragment.targetCount = 1;
    pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    wgpu::RenderPipeline testPipeline = device.CreateRenderPipeline(&pDesc);

    // Create a bind group with textures of decreasing values
    wgpu::BindGroup arrayGroup = utils::MakeBindGroup(device, testPipeline.GetBindGroupLayout(0),
                                                      {
                                                          {3, MakeTestR8Texture(0).CreateView()},
                                                          {2, MakeTestR8Texture(1).CreateView()},
                                                          {1, MakeTestR8Texture(2).CreateView()},
                                                          {0, MakeTestR8Texture(3).CreateView()},
                                                      });

    // Run the test
    auto rp = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);

    pass.SetPipeline(testPipeline);
    pass.SetBindGroup(0, arrayGroup);
    pass.Draw(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(3, 2, 1, 0), rp.color, 0, 0);
}

// Test accessing a binding_array with dynamically uniform values.
TEST_P(SizedBindingArrayTests, IndexingWithDynamicallyUniformValues) {
    // Make the test pipeline
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0.5, 0.5);
        }

        @group(0) @binding(0) var<storage> indices : array<u32, 4>;
        @group(0) @binding(1) var ts : binding_array<texture_2d<f32>, 4>;
        @fragment fn fs() -> @location(0) vec4f {
            let r = textureLoad(ts[indices[0]], vec2(0, 0), 0)[0];
            let g = textureLoad(ts[indices[1]], vec2(0, 0), 0)[0];
            let b = textureLoad(ts[indices[2]], vec2(0, 0), 0)[0];
            let a = textureLoad(ts[indices[3]], vec2(0, 0), 0)[0];
            return vec4(r, g, b, a);
        }
    )");

    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.vertex.module = module;
    pDesc.cFragment.module = module;
    pDesc.cFragment.targetCount = 1;
    pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    wgpu::RenderPipeline testPipeline = device.CreateRenderPipeline(&pDesc);

    // Create a bind group with the indirection
    wgpu::Buffer indices =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Storage, {2, 3, 1, 0});

    wgpu::BindGroup arrayGroup = utils::MakeBindGroup(device, testPipeline.GetBindGroupLayout(0),
                                                      {
                                                          {0, indices},
                                                          {1, MakeTestR8Texture(0).CreateView()},
                                                          {2, MakeTestR8Texture(1).CreateView()},
                                                          {3, MakeTestR8Texture(2).CreateView()},
                                                          {4, MakeTestR8Texture(3).CreateView()},
                                                      });

    // Run the test
    auto rp = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);

    pass.SetPipeline(testPipeline);
    pass.SetBindGroup(0, arrayGroup);
    pass.Draw(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(2, 3, 1, 0), rp.color, 0, 0);
}

// Test accessing a binding_array surrounded with other bindings in the layout.
TEST_P(SizedBindingArrayTests, ArrayEntryOversizedAndSurrounded) {
    // Create the BGL with an array larger than the declaration in the shader such the trailing
    // binding will be unused.
    wgpu::BindGroupLayout bgl;
    {
        wgpu::BindGroupLayoutEntryArraySize arraySize;
        arraySize.arraySize = 3;

        std::array<wgpu::BindGroupLayoutEntry, 3> entries;

        entries[0].binding = 0;
        entries[0].visibility = wgpu::ShaderStage::Fragment;
        entries[0].texture.sampleType = wgpu::TextureSampleType::Float;

        entries[1].binding = 1;
        entries[1].visibility = wgpu::ShaderStage::Fragment;
        entries[1].texture.sampleType = wgpu::TextureSampleType::Float;
        entries[1].nextInChain = &arraySize;

        entries[2].binding = 4;
        entries[2].visibility = wgpu::ShaderStage::Fragment;
        entries[2].texture.sampleType = wgpu::TextureSampleType::Float;

        wgpu::BindGroupLayoutDescriptor bglDesc;
        bglDesc.entryCount = entries.size();
        bglDesc.entries = entries.data();
        bgl = device.CreateBindGroupLayout(&bglDesc);
    }

    // Make the test pipeline
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0.5, 0.5);
        }

        @group(0) @binding(0) var t0 : texture_2d<f32>;
        // Note that the layout will contain one extra entry using @binding(3)
        @group(0) @binding(1) var ts : binding_array<texture_2d<f32>, 3>;
        @group(0) @binding(4) var t3 : texture_2d<f32>;
        @fragment fn fs() -> @location(0) vec4f {
            let r = textureLoad(t0, vec2(0, 0), 0)[0];
            let g = textureLoad(ts[0], vec2(0, 0), 0)[0];
            let b = textureLoad(ts[1], vec2(0, 0), 0)[0];
            let a = textureLoad(t3, vec2(0, 0), 0)[0];
            return vec4(r, g, b, a);
        }
    )");

    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.layout = utils::MakePipelineLayout(device, {bgl});
    pDesc.vertex.module = module;
    pDesc.cFragment.module = module;
    pDesc.cFragment.targetCount = 1;
    pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    wgpu::RenderPipeline testPipeline = device.CreateRenderPipeline(&pDesc);

    // Create a bind group with the explicit layout and the extra array element
    wgpu::BindGroup arrayGroup = utils::MakeBindGroup(device, bgl,
                                                      {
                                                          {0, MakeTestR8Texture(0).CreateView()},
                                                          {1, MakeTestR8Texture(1).CreateView()},
                                                          {2, MakeTestR8Texture(2).CreateView()},
                                                          {3, MakeTestR8Texture(42).CreateView()},
                                                          {4, MakeTestR8Texture(3).CreateView()},
                                                      });

    // Run the test
    auto rp = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);

    pass.SetPipeline(testPipeline);
    pass.SetBindGroup(0, arrayGroup);
    pass.Draw(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(0, 1, 2, 3), rp.color, 0, 0);
}

// Test that an arrayed BGLEntry of size 2 is compatible with a non-arrayed binding.
TEST_P(SizedBindingArrayTests, NonArrayedBindingCompatibleWithArrayedBGLEntry) {
    // Create the BGL with an oversized array
    wgpu::BindGroupLayout bgl;
    {
        wgpu::BindGroupLayoutEntryArraySize arraySize;
        arraySize.arraySize = 2;

        wgpu::BindGroupLayoutEntry entry;

        entry.binding = 0;
        entry.visibility = wgpu::ShaderStage::Fragment;
        entry.texture.sampleType = wgpu::TextureSampleType::Float;
        entry.nextInChain = &arraySize;

        wgpu::BindGroupLayoutDescriptor bglDesc;
        bglDesc.entryCount = 1;
        bglDesc.entries = &entry;
        bgl = device.CreateBindGroupLayout(&bglDesc);
    }

    // Make the test pipeline
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0.5, 0.5);
        }

        @group(0) @binding(0) var t : texture_2d<f32>;
        @fragment fn fs() -> @location(0) vec4f {
            return vec4f(textureLoad(t, vec2(0, 0), 0).r, 0, 0, 0);
        }
    )");

    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.layout = utils::MakePipelineLayout(device, {bgl});
    pDesc.vertex.module = module;
    pDesc.cFragment.module = module;
    pDesc.cFragment.targetCount = 1;
    pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    wgpu::RenderPipeline testPipeline = device.CreateRenderPipeline(&pDesc);

    // Run the test
    wgpu::BindGroup arrayGroup = utils::MakeBindGroup(device, bgl,
                                                      {
                                                          {0, MakeTestR8Texture(33).CreateView()},
                                                          {1, MakeTestR8Texture(34).CreateView()},
                                                      });

    auto rp = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);

    pass.SetPipeline(testPipeline);
    pass.SetBindGroup(0, arrayGroup);
    pass.Draw(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(33, 0, 0, 0), rp.color, 0, 0);
}

// Test that binding_array<T, 1> is compatible with a non-arrayed layout.
TEST_P(SizedBindingArrayTests, BindingArraySize1CompatibleWithNonArrayedBGL) {
    // Make the test pipeline
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0.5, 0.5);
        }

        @group(0) @binding(0) var ts : binding_array<texture_2d<f32>, 1>;
        @fragment fn fs() -> @location(0) vec4f {
            return vec4f(textureLoad(ts[0], vec2(0, 0), 0).r, 0, 0, 0);
        }
    )");

    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}});

    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.layout = utils::MakePipelineLayout(device, {bgl});
    pDesc.vertex.module = module;
    pDesc.cFragment.module = module;
    pDesc.cFragment.targetCount = 1;
    pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    wgpu::RenderPipeline testPipeline = device.CreateRenderPipeline(&pDesc);

    // Run the test
    wgpu::BindGroup arrayGroup = utils::MakeBindGroup(device, bgl,
                                                      {
                                                          {0, MakeTestR8Texture(42).CreateView()},
                                                      });

    auto rp = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);

    pass.SetPipeline(testPipeline);
    pass.SetBindGroup(0, arrayGroup);
    pass.Draw(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(42, 0, 0, 0), rp.color, 0, 0);
}

DAWN_INSTANTIATE_TEST(SizedBindingArrayTests, MetalBackend());

}  // anonymous namespace
}  // namespace dawn
