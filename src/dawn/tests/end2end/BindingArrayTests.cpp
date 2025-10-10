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

#include <string>
#include <unordered_set>
#include <vector>

#include "dawn/common/Enumerator.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class SizedBindingArrayTests : public DawnTest {
  public:
    void SetUp() override {
        DawnTest::SetUp();

        // TODO(https://issues.chromium.org/411573959) Fails using WARP but not on real hardware.
        // WARP 10.0.19031.4355 samples the wrong textures when indexing while WARP 1.0.12.0 fails
        // pipeline creation.
        DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsWARP());
    }

    // A 1x1 texture with a single value to check the correct binding is used.
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

    // Test textures that can be used that samplers correctly clamp or wrap.
    wgpu::Texture MakeTest2x1R8Texture(uint8_t left, uint8_t right) {
        wgpu::TextureDescriptor desc;
        desc.size = {2, 1};
        desc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
        desc.format = wgpu::TextureFormat::R8Unorm;
        wgpu::Texture texture = device.CreateTexture(&desc);

        wgpu::TexelCopyTextureInfo srcInfo = utils::CreateTexelCopyTextureInfo(texture);
        wgpu::TexelCopyBufferLayout dstInfo = {};
        wgpu::Extent3D copySize = {2, 1, 1};
        std::array<uint8_t, 2> data = {left, right};
        queue.WriteTexture(&srcInfo, &data, sizeof(data), &dstInfo, &copySize);

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
    // Compat should disallow dynamically uniform indexing of binding_array of textures.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    // TODO(https://issues.chromium.org/425328998): D3D11 exposes core when FXC cannot support
    // core's uniform indexing of binding_array<texture*>. We should make D3D11 expose only compat
    // by default.
    DAWN_SUPPRESS_TEST_IF(IsD3D11());

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
        std::array<wgpu::BindGroupLayoutEntry, 3> entries;

        entries[0].binding = 0;
        entries[0].visibility = wgpu::ShaderStage::Fragment;
        entries[0].texture.sampleType = wgpu::TextureSampleType::Float;

        entries[1].binding = 1;
        entries[1].visibility = wgpu::ShaderStage::Fragment;
        entries[1].bindingArraySize = 3;
        entries[1].texture.sampleType = wgpu::TextureSampleType::Float;

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
        @group(0) @binding(1) var ts : binding_array<texture_2d<f32>, 2>;
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
        wgpu::BindGroupLayoutEntry entry;

        entry.binding = 0;
        entry.visibility = wgpu::ShaderStage::Fragment;
        entry.bindingArraySize = 2;
        entry.texture.sampleType = wgpu::TextureSampleType::Float;

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

// Test passing sampled textures of binding_array as function arguments.
TEST_P(SizedBindingArrayTests, BindingArraySampledTextureAsFunctionArgument) {
    // TODO(https://issues.chromium.org/411573957) OgenGL requires that indices in arrays of sampler
    // be constants but even after the DirectVariableAccess transform, the index is a function
    // parameter (that's constant at the callsite) and not a constant.
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() || IsOpenGLES());

    // Make the test pipeline
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0.5, 0.5);
        }

        fn load(t : texture_2d<f32>) -> f32 {
            return textureLoad(t, vec2(0, 0), 0)[0];
        }

        @group(0) @binding(0) var ts : binding_array<texture_2d<f32>, 4>;
        @fragment fn fs() -> @location(0) vec4f {
            let r = load(ts[0]);
            let g = load(ts[1]);
            let b = load(ts[2]);
            let a = load(ts[3]);
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

// Test accessing a binding_array of sampled textures passed as function argument.
TEST_P(SizedBindingArrayTests, BindingArrayOfSampledTexturesPassedAsArgument) {
    // Crashes on the Intel Windows Vulkan shader compiler.
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsWindows() && IsIntel());

    // Make the test pipeline
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0.5, 0.5);
        }

        @group(0) @binding(0) var ts : binding_array<texture_2d<f32>, 4>;
        fn f(textures : binding_array<texture_2d<f32>, 4>) -> vec4f {
            let r = textureLoad(textures[0], vec2(0, 0), 0)[0];
            let g = textureLoad(textures[1], vec2(0, 0), 0)[0];
            let b = textureLoad(textures[2], vec2(0, 0), 0)[0];
            let a = textureLoad(textures[3], vec2(0, 0), 0)[0];
            return vec4(r, g, b, a);
        }

        @fragment fn fs() -> @location(0) vec4f {
            return f(ts);
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

// Test that multiple texture and sampler combinations are handled correctly (in particular on GL).
TEST_P(SizedBindingArrayTests, TextureAndSamplerCombination) {
    // Make the test pipeline
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0.5, 0.5);
        }

        @group(0) @binding(0) var ts : binding_array<texture_2d<f32>, 2>;
        @group(0) @binding(2) var samplerClamping : sampler;
        @group(0) @binding(3) var samplerWrapping : sampler;
        @fragment fn fs() -> @location(0) vec4f {
            let r = textureSample(ts[0], samplerWrapping, vec2(1.25, 0.5))[0];
            let g = textureSample(ts[0], samplerClamping, vec2(1.25, 0.5))[0];
            let b = textureSample(ts[1], samplerWrapping, vec2(1.25, 0.5))[0];
            let a = textureSample(ts[1], samplerClamping, vec2(1.25, 0.5))[0];
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
    wgpu::SamplerDescriptor sDesc = {};
    sDesc.addressModeU = wgpu::AddressMode::ClampToEdge;
    wgpu::Sampler samplerClamping = device.CreateSampler(&sDesc);
    sDesc.addressModeU = wgpu::AddressMode::Repeat;
    wgpu::Sampler samplerWrapping = device.CreateSampler(&sDesc);

    wgpu::BindGroup arrayGroup =
        utils::MakeBindGroup(device, testPipeline.GetBindGroupLayout(0),
                             {
                                 {0, MakeTest2x1R8Texture(3, 2).CreateView()},
                                 {1, MakeTest2x1R8Texture(1, 0).CreateView()},
                                 {2, samplerClamping},
                                 {3, samplerWrapping},
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

// Test that calling textureNumLevels on an element of a binding_array returns the correct value,
// this is targeted for the GL backend that has emulation of that builtin with a UBO.
TEST_P(SizedBindingArrayTests, TextureNumLevels) {
    // Make the test pipeline
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0.5, 0.5);
        }

        @group(0) @binding(0) var ts : binding_array<texture_2d<f32>, 3>;
        @fragment fn fs() -> @location(0) vec4f {
            let r = f32(textureNumLevels(ts[0])) / 255.0;
            let g = f32(textureNumLevels(ts[1])) / 255.0;
            let b = f32(textureNumLevels(ts[2])) / 255.0;
            let a : f32 = 0.0;
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

    // Create a bind group with textures views of decreasing numlevels.
    wgpu::TextureDescriptor tDesc;
    tDesc.size = {4, 4};
    tDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    tDesc.usage = wgpu::TextureUsage::TextureBinding;
    tDesc.mipLevelCount = 3;
    wgpu::TextureView view3 = device.CreateTexture(&tDesc).CreateView();
    tDesc.mipLevelCount = 2;
    wgpu::TextureView view2 = device.CreateTexture(&tDesc).CreateView();
    tDesc.mipLevelCount = 1;
    wgpu::TextureView view1 = device.CreateTexture(&tDesc).CreateView();

    wgpu::BindGroup arrayGroup = utils::MakeBindGroup(device, testPipeline.GetBindGroupLayout(0),
                                                      {
                                                          {0, view3},
                                                          {1, view2},
                                                          {2, view1},
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

// TODO(crbug.com/440123094): Implement nextInChain of dynamic binding in WebGPUBackend.
DAWN_INSTANTIATE_TEST(SizedBindingArrayTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

class DynamicBindingArrayTests : public DawnTest {
  public:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(
            !SupportsFeatures({wgpu::FeatureName::ChromiumExperimentalBindless}));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        if (SupportsFeatures({wgpu::FeatureName::ChromiumExperimentalBindless})) {
            return {wgpu::FeatureName::ChromiumExperimentalBindless};
        }
        return {};
    }

    // Helper similar to utils::MakeBindGroupLayout but that adds a dynamic array.
    wgpu::BindGroupLayout MakeBindGroupLayout(
        wgpu::DynamicBindingKind kind,
        uint32_t dynamicArrayStart = 0,
        std::initializer_list<utils::BindingLayoutEntryInitializationHelper> entriesInitializer =
            {}) {
        std::vector<wgpu::BindGroupLayoutEntry> entries;
        for (const utils::BindingLayoutEntryInitializationHelper& entry : entriesInitializer) {
            entries.push_back(entry);
        }

        wgpu::BindGroupLayoutDynamicBindingArray dynamic;
        dynamic.dynamicArray.kind = kind;
        dynamic.dynamicArray.start = dynamicArrayStart;

        wgpu::BindGroupLayoutDescriptor descriptor;
        descriptor.nextInChain = &dynamic;
        descriptor.entryCount = entries.size();
        descriptor.entries = entries.data();
        return device.CreateBindGroupLayout(&descriptor);
    }

    // Helper similar to utils::MakeBindGroup but that adds a dynamic array.
    wgpu::BindGroup MakeBindGroup(
        const wgpu::BindGroupLayout& layout,
        uint32_t dynamicArraySize,
        std::initializer_list<utils::BindingInitializationHelper> entriesInitializer) {
        std::vector<wgpu::BindGroupEntry> entries;
        for (const utils::BindingInitializationHelper& helper : entriesInitializer) {
            entries.push_back(helper.GetAsBinding());
        }

        wgpu::BindGroupDynamicBindingArray dynamic;
        dynamic.dynamicArraySize = dynamicArraySize;

        wgpu::BindGroupDescriptor descriptor;
        descriptor.nextInChain = &dynamic;
        descriptor.layout = layout;
        descriptor.entryCount = entries.size();
        descriptor.entries = entries.data();

        return device.CreateBindGroup(&descriptor);
    }

    // Test that `dynamicArray` (with layout `bgl` and `dynamicArrayStart`), has bindings of
    // `wgslType` in the `expected` slots.
    void TestHasBinding(wgpu::BindGroupLayout bgl,
                        wgpu::BindGroup dynamicArray,
                        std::vector<bool> expected,
                        uint32_t dynamicArrayStart = 0,
                        std::string wgslType = "texture_2d<f32>") {
        // Create the test pipeline.
        std::array<wgpu::BindGroupLayout, 2> bgls = {
            bgl,
            utils::MakeBindGroupLayout(
                device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}}),
        };
        wgpu::PipelineLayoutDescriptor plDesc = {
            .bindGroupLayoutCount = 2,
            .bindGroupLayouts = bgls.data(),
        };

        wgpu::ShaderModule module =
            utils::CreateShaderModule(device, R"(
            enable chromium_experimental_dynamic_binding;
            @group(0) @binding()" + std::to_string(dynamicArrayStart) +
                                                  R"() var bindings : resource_binding;
            @group(1) @binding(0) var<storage, read_write> results : array<u32>;

            @compute @workgroup_size(1) fn main() {
                for (var i = 0u; i < arrayLength(bindings); i++) {
                    results[i] = u32(hasBinding<)" +
                                                  wgslType + R"(>(bindings, i));
                }
            }
        )");

        wgpu::ComputePipelineDescriptor csDesc = {.layout = device.CreatePipelineLayout(&plDesc),
                                                  .compute = {
                                                      .module = module,
                                                  }};
        wgpu::ComputePipeline testPipeline = device.CreateComputePipeline(&csDesc);

        // Create the result buffer.
        wgpu::BufferDescriptor bDesc = {
            .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
            .size = sizeof(uint32_t) * expected.size(),
        };
        wgpu::Buffer resultBuffer = device.CreateBuffer(&bDesc);
        wgpu::BindGroup resultBG = utils::MakeBindGroup(device, bgls[1], {{0, resultBuffer}});

        // Run the test.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetBindGroup(0, dynamicArray);
        pass.SetBindGroup(1, resultBG);
        pass.SetPipeline(testPipeline);
        pass.DispatchWorkgroups(1);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);

        // Check we have the expected results.
        std::vector<uint32_t> expectedU32;
        for (bool b : expected) {
            expectedU32.push_back(b ? 1u : 0u);
        }

        EXPECT_BUFFER_U32_RANGE_EQ(expectedU32.data(), resultBuffer, 0, expectedU32.size())
            << " for WGSL type " << wgslType;
    }
};

// Tests that creating the bind group that's only a dynamic array doesn't crash in backends.
TEST_P(DynamicBindingArrayTests, BindGroupOnlyDynamicArray) {
    wgpu::BindGroupLayout bgl = MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture);

    wgpu::TextureDescriptor tDesc;
    tDesc.format = wgpu::TextureFormat::R32Float;
    tDesc.size = {1, 1};
    tDesc.usage = wgpu::TextureUsage::TextureBinding;
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    // Make a dense dynamic array of size 1.
    MakeBindGroup(bgl, 1, {{0, tex.CreateView()}});

    // Make a dense dynamic array of size 3.
    MakeBindGroup(bgl, 3,
                  {
                      {0, tex.CreateView()},
                      {1, tex.CreateView()},
                      {2, tex.CreateView()},
                  });

    // Make a sparse dynamic array.
    MakeBindGroup(bgl, 3, {{1, tex.CreateView()}});

    // Make an empty dynamic array.
    MakeBindGroup(bgl, 3, {});
}

// Tests that creating the bind group that has static bindings and a dynamic array doesn't crash in
// backends.
TEST_P(DynamicBindingArrayTests, BindGroupDynamicArrayWithStaticBindings) {
    wgpu::BindGroupLayout bgl = MakeBindGroupLayout(
        wgpu::DynamicBindingKind::SampledTexture, 4,
        {
            // Buffer sampler storage texture
            {0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform},
            {1, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering},
            {2, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float},
        });

    wgpu::TextureDescriptor tDesc;
    tDesc.format = wgpu::TextureFormat::R16Float;
    tDesc.size = {1, 1};
    tDesc.usage = wgpu::TextureUsage::TextureBinding;
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::BufferDescriptor bDesc;
    bDesc.size = 4;
    bDesc.usage = wgpu::BufferUsage::Uniform;
    wgpu::Buffer buffer = device.CreateBuffer(&bDesc);

    // Make a dense dynamic array of size 1.
    MakeBindGroup(bgl, 1,
                  {{0, buffer}, {1, sampler}, {2, tex.CreateView()}, {4, tex.CreateView()}});

    // Make a dense dynamic array of size 3.
    MakeBindGroup(bgl, 3,
                  {
                      {0, buffer},
                      {1, sampler},
                      {2, tex.CreateView()},
                      {4, tex.CreateView()},
                      {5, tex.CreateView()},
                      {6, tex.CreateView()},
                  });

    // Make a sparse dynamic array.
    MakeBindGroup(bgl, 3,
                  {{0, buffer}, {1, sampler}, {2, tex.CreateView()}, {5, tex.CreateView()}});

    // Make an empty dynamic array.
    MakeBindGroup(bgl, 3, {{0, buffer}, {1, sampler}, {2, tex.CreateView()}});
}

// Test that creating bind groups of different sizes doesn't end up reuse incorrectly sized
// allocations.
TEST_P(DynamicBindingArrayTests, RecyclingDoesntReuseTooSmallAllocation) {
    wgpu::BindGroupLayout bgl = MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture);

    for (uint32_t i = 0; i < 10; i++) {
        MakeBindGroup(bgl, i, {});

        // Wait to ensure some deallocation happens and has a chance to cause incorrect recycling.
        WaitForAllOperations();
    }
}

// Tests that pinning / unpinning doesn't crash in backends. TextureVk has ASSERTs that the frontend
// ensures pinning / unpinning is balanced to help backends.
TEST_P(DynamicBindingArrayTests, PinningBalancedInBackends) {
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R16Float,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    // Frontend should skip that unpinning as the texture is not pinned.
    tex.Unpin();

    // Duplicate pinning should be skipped by the frontend.
    tex.Pin(wgpu::TextureUsage::TextureBinding);
    tex.Pin(wgpu::TextureUsage::TextureBinding);

    // Duplicate unpinning should be skipped by the frontend.
    tex.Unpin();
    tex.Unpin();

    // Force a queue submit to flush pending commands and potentially find more issues.
    queue.Submit(0, nullptr);
}

// TODO(https://crbug.com/435317394): Once we have support for running shader with dynamic binding
// arrays, add tests that pinning handles lazy clearing:
//  - Check that a newly created resource that's pinned samples as zeroes.
//  - Likewise for a texture written to, then discarded with a render pass.

// Test that the WGSL `arrayLength` builtin on dynamic binding arrays returns the correct length.
TEST_P(DynamicBindingArrayTests, ArrayLengthBuiltin) {
    // Create a compute pipeline that returns the array length of the dynamic binding arrays.
    // One of them has a static binding as well so as to check that it doesn't mess up the
    // computation of the array length.
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_dynamic_binding;

        @group(0) @binding(0) var<storage, read_write> result : array<u32, 2>;
        @group(0) @binding(1) var firstBindings : resource_binding;
        @group(1) @binding(0) var secondBindings : resource_binding;

        @compute @workgroup_size(1) fn getArrayLengths() {
            // Force the defaulted layout to wgpu::DynamicBindingKind::SampledTexture
            _ = hasBinding<texture_2d<f32>>(firstBindings, 0);
            _ = hasBinding<texture_2d<f32>>(secondBindings, 0);

            result[0] = arrayLength(firstBindings);
            result[1] = arrayLength(secondBindings);
        }
    )");
    wgpu::ComputePipelineDescriptor csDesc = {.compute = {
                                                  .module = module,
                                              }};
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Create the dynamic binding arrays and fetch their array length in a buffer.
    wgpu::BufferDescriptor bDesc = {
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        .size = 2 * sizeof(uint32_t),
    };
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bDesc);

    wgpu::BindGroup bg0 = MakeBindGroup(pipeline.GetBindGroupLayout(0), 17, {{0, resultBuffer}});
    wgpu::BindGroup bg1 = MakeBindGroup(pipeline.GetBindGroupLayout(1), 3, {});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetBindGroup(0, bg0);
    pass.SetBindGroup(1, bg1);
    pass.SetPipeline(pipeline);
    pass.DispatchWorkgroups(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    device.GetQueue().Submit(1, &commands);

    // The result buffer should contain the lengths of the dynamic binding arrays.
    EXPECT_BUFFER_U32_EQ(17, resultBuffer, 0);
    EXPECT_BUFFER_U32_EQ(3, resultBuffer, 4);
}

// Test WGSL `hasBinding` reflects the state of a dynamic binding array.
TEST_P(DynamicBindingArrayTests, HasBindingOneTexturePinUnpin) {
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    wgpu::BindGroupLayout bgl = MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture);
    wgpu::BindGroup bg = MakeBindGroup(bgl, 3, {{1, tex.CreateView()}});

    // Before pinning, the bind group has no valid entries.
    TestHasBinding(bgl, bg, {false, false, false});

    // After pinning it has the one valid entry valid.
    tex.Pin(wgpu::TextureUsage::TextureBinding);
    TestHasBinding(bgl, bg, {false, true, false});

    // After unpinning it has the no more valid entries.
    tex.Unpin();
    TestHasBinding(bgl, bg, {false, false, false});
}

// Test pin/unpin updating the availability takes into account the static bindings (so even if it
// doesn't start at BindingIndex 0, things still work)
TEST_P(DynamicBindingArrayTests, HasBindingOneTexturePinUnpinWithStaticBindings) {
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    wgpu::BindGroupLayout bgl = MakeBindGroupLayout(
        wgpu::DynamicBindingKind::SampledTexture, 4,
        {{0, wgpu::ShaderStage::Compute, wgpu::TextureSampleType::UnfilterableFloat}});
    wgpu::BindGroup bg = MakeBindGroup(bgl, 3, {{0, tex.CreateView()}, {5, tex.CreateView()}});

    // Before pinning, the bind group has no valid entries.
    TestHasBinding(bgl, bg, {false, false, false}, 4);

    // After pinning it has the one valid entry valid.
    tex.Pin(wgpu::TextureUsage::TextureBinding);
    TestHasBinding(bgl, bg, {false, true, false}, 4);

    // After unpinning it has the no more valid entries.
    tex.Unpin();
    TestHasBinding(bgl, bg, {false, false, false}, 4);
}

// Test that calling texture.Destroy() implicitly unpins it.
TEST_P(DynamicBindingArrayTests, HasBindingOneTexturePinDestroy) {
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    wgpu::BindGroupLayout bgl = MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture);
    wgpu::BindGroup bg = MakeBindGroup(bgl, 3, {{1, tex.CreateView()}});

    // Before pinning, the bind group has no valid entries.
    TestHasBinding(bgl, bg, {false, false, false});

    // After pinning it has the one valid entry valid.
    tex.Pin(wgpu::TextureUsage::TextureBinding);
    TestHasBinding(bgl, bg, {false, true, false});

    // After texture destruction it has the no more valid entries.
    tex.Destroy();
    TestHasBinding(bgl, bg, {false, false, false});
}

// Test that a texture used multiple times in the same dynamic binding array has its availability
// correctly updated.
TEST_P(DynamicBindingArrayTests, HasBindingSameTextureMultipleTimesPinUnpin) {
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    wgpu::BindGroupLayout bgl = MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture);
    wgpu::BindGroup bg = MakeBindGroup(bgl, 4, {{1, tex.CreateView()}, {3, tex.CreateView()}});

    // Before pinning, the bind group has no valid entries.
    TestHasBinding(bgl, bg, {false, false, false, false});

    // After pinning it has valid entries.
    tex.Pin(wgpu::TextureUsage::TextureBinding);
    TestHasBinding(bgl, bg, {false, true, false, true});

    // After unpinning it has the no more valid entries.
    tex.Unpin();
    TestHasBinding(bgl, bg, {false, false, false, false});
}

// Test that creating a dynamic binding array with an already destroyed texture works, but doesn't
// show that entry as available.
TEST_P(DynamicBindingArrayTests, HasBindingDynamicArrayCreatedWithTextureAlreadyDestroyed) {
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);
    tex.Destroy();

    wgpu::BindGroupLayout bgl = MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture);
    wgpu::BindGroup bg = MakeBindGroup(bgl, 1, {{0, tex.CreateView()}});

    // Before pinning, the bind group has no valid entries.
    TestHasBinding(bgl, bg, {false});
}

// Test that a texture used multiple times in the same dynamic binding array has its
// availability correctly updated.
TEST_P(DynamicBindingArrayTests, HasBindingSameTextureMultipleDynamicArrays) {
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    wgpu::BindGroupLayout bgl = MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture);
    wgpu::BindGroup bg1 = MakeBindGroup(bgl, 3, {{1, tex.CreateView()}});
    wgpu::BindGroup bg2 = MakeBindGroup(bgl, 1, {{0, tex.CreateView()}});

    // Before pinning, the bind group has no valid entries.
    TestHasBinding(bgl, bg1, {false, false, false});
    TestHasBinding(bgl, bg2, {false});

    // After pinning it has valid entries.
    tex.Pin(wgpu::TextureUsage::TextureBinding);
    TestHasBinding(bgl, bg1, {false, true, false});
    TestHasBinding(bgl, bg2, {true});

    // After destroying on dynamic binding array, the other still has the texture available.
    bg1.Destroy();
    TestHasBinding(bgl, bg2, {true});
}

// Test that texture availabililty is controlled per-texture.
TEST_P(DynamicBindingArrayTests, HasBindingMultipleTexturesInDynamicArray) {
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex0 = device.CreateTexture(&tDesc);
    wgpu::Texture tex1 = device.CreateTexture(&tDesc);

    wgpu::BindGroupLayout bgl = MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture);
    wgpu::BindGroup bg = MakeBindGroup(bgl, 2, {{0, tex0.CreateView()}, {1, tex1.CreateView()}});

    // Before pinning, the bind group has no valid entries.
    TestHasBinding(bgl, bg, {false, false});

    // After pinning tex0 it has one valid entry.
    tex0.Pin(wgpu::TextureUsage::TextureBinding);
    TestHasBinding(bgl, bg, {true, false});

    // After pinning tex1 it has two valid entries.
    tex1.Pin(wgpu::TextureUsage::TextureBinding);
    TestHasBinding(bgl, bg, {true, true});

    // After unpinning tex0 it has only one valid entry.
    tex0.Unpin();
    TestHasBinding(bgl, bg, {false, true});
}

constexpr auto kWgslSampledTextureTypes = std::array{
    "texture_1d<f32>",
    "texture_1d<i32>",
    "texture_1d<u32>",
    "texture_2d<f32>",
    "texture_2d<i32>",
    "texture_2d<u32>",
    "texture_2d_array<f32>",
    "texture_2d_array<i32>",
    "texture_2d_array<u32>",
    "texture_cube<f32>",
    "texture_cube<i32>",
    "texture_cube<u32>",
    "texture_cube_array<f32>",
    "texture_cube_array<i32>",
    "texture_cube_array<u32>",
    "texture_3d<f32>",
    "texture_3d<i32>",
    "texture_3d<u32>",

    "texture_multisampled_2d<f32>",
    "texture_multisampled_2d<i32>",
    "texture_multisampled_2d<u32>",

    "texture_depth_2d",
    "texture_depth_2d_array",
    "texture_depth_cube",
    "texture_depth_cube_array",
    "texture_depth_multisampled_2d",
};

struct TextureDescForTypeIDCase {
    std::unordered_set<std::string_view> wgslTypes;
    wgpu::TextureFormat format;
    wgpu::TextureDimension dimension;
    wgpu::TextureViewDimension viewDimension = wgpu::TextureViewDimension::Undefined;
    uint32_t sampleCount = 1;
    wgpu::TextureAspect viewAspect = wgpu::TextureAspect::All;

    // Create a view for a pinned texture for this case.
    wgpu::TextureView CreateTestView(const wgpu::Device& device) {
        wgpu::TextureDescriptor tDesc = {
            .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc,
            .dimension = dimension,
            .size = {1, 1, 1},
            .format = format,
            .sampleCount = sampleCount,
        };
        if (viewDimension == wgpu::TextureViewDimension::Cube ||
            viewDimension == wgpu::TextureViewDimension::CubeArray) {
            tDesc.size.depthOrArrayLayers = 6;
        }
        if (sampleCount != 1) {
            tDesc.usage |= wgpu::TextureUsage::RenderAttachment;
        }

        wgpu::TextureViewDescriptor vDesc{
            .dimension = viewDimension,
            .aspect = viewAspect,
            .usage = wgpu::TextureUsage::TextureBinding,
        };

        wgpu::Texture texture = device.CreateTexture(&tDesc);
        texture.Pin(wgpu::TextureUsage::TextureBinding);
        return texture.CreateView(&vDesc);
    }
};

std::vector<TextureDescForTypeIDCase> MakeTextureDescForTypeIDCases() {
    std::vector<TextureDescForTypeIDCase> cases;

    // TODO(https://crbug.com/435317394): Add tests of filterable vs. unfilterable floats when
    // get/hasBinding is able to make the difference.

    // Regular 1D textures.
    cases.push_back({
        .wgslTypes = {{"texture_1d<f32>"}},
        .format = wgpu::TextureFormat::RGBA32Float,
        .dimension = wgpu::TextureDimension::e1D,
    });
    cases.push_back({
        .wgslTypes = {{"texture_1d<i32>"}},
        .format = wgpu::TextureFormat::RGBA32Sint,
        .dimension = wgpu::TextureDimension::e1D,
    });
    cases.push_back({
        .wgslTypes = {{"texture_1d<u32>"}},
        .format = wgpu::TextureFormat::RGBA32Uint,
        .dimension = wgpu::TextureDimension::e1D,
    });

    // Regular 2D textures.
    cases.push_back({
        .wgslTypes = {{"texture_2d<f32>"}},
        .format = wgpu::TextureFormat::RGBA32Float,
        .dimension = wgpu::TextureDimension::e2D,
    });
    cases.push_back({
        .wgslTypes = {{"texture_2d<i32>"}},
        .format = wgpu::TextureFormat::RGBA32Sint,
        .dimension = wgpu::TextureDimension::e2D,
    });
    cases.push_back({
        .wgslTypes = {{"texture_2d<u32>"}},
        .format = wgpu::TextureFormat::RGBA32Uint,
        .dimension = wgpu::TextureDimension::e2D,
    });

    // Regular 2D array textures.
    cases.push_back({
        .wgslTypes = {{"texture_2d_array<f32>"}},
        .format = wgpu::TextureFormat::RGBA32Float,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::e2DArray,
    });
    cases.push_back({
        .wgslTypes = {{"texture_2d_array<i32>"}},
        .format = wgpu::TextureFormat::RGBA32Sint,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::e2DArray,
    });
    cases.push_back({
        .wgslTypes = {{"texture_2d_array<u32>"}},
        .format = wgpu::TextureFormat::RGBA32Uint,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::e2DArray,
    });

    // Regular cube textures.
    cases.push_back({
        .wgslTypes = {{"texture_cube<f32>"}},
        .format = wgpu::TextureFormat::RGBA32Float,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::Cube,
    });
    cases.push_back({
        .wgslTypes = {{"texture_cube<i32>"}},
        .format = wgpu::TextureFormat::RGBA32Sint,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::Cube,
    });
    cases.push_back({
        .wgslTypes = {{"texture_cube<u32>"}},
        .format = wgpu::TextureFormat::RGBA32Uint,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::Cube,
    });

    // Regular cube array textures.
    cases.push_back({
        .wgslTypes = {{"texture_cube_array<f32>"}},
        .format = wgpu::TextureFormat::RGBA32Float,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::CubeArray,
    });
    cases.push_back({
        .wgslTypes = {{"texture_cube_array<i32>"}},
        .format = wgpu::TextureFormat::RGBA32Sint,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::CubeArray,
    });
    cases.push_back({
        .wgslTypes = {{"texture_cube_array<u32>"}},
        .format = wgpu::TextureFormat::RGBA32Uint,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::CubeArray,
    });

    // Regular 3d textures.
    cases.push_back({
        .wgslTypes = {{"texture_3d<f32>"}},
        .format = wgpu::TextureFormat::RGBA32Float,
        .dimension = wgpu::TextureDimension::e3D,
    });
    cases.push_back({
        .wgslTypes = {{"texture_3d<i32>"}},
        .format = wgpu::TextureFormat::RGBA32Sint,
        .dimension = wgpu::TextureDimension::e3D,
    });
    cases.push_back({
        .wgslTypes = {{"texture_3d<u32>"}},
        .format = wgpu::TextureFormat::RGBA32Uint,
        .dimension = wgpu::TextureDimension::e3D,
    });

    // Color multisampled textures.
    cases.push_back({
        .wgslTypes = {{"texture_multisampled_2d<f32>"}},
        .format = wgpu::TextureFormat::RGBA16Float,
        .dimension = wgpu::TextureDimension::e2D,
        .sampleCount = 4,
    });
    cases.push_back({
        .wgslTypes = {{"texture_multisampled_2d<i32>"}},
        .format = wgpu::TextureFormat::RGBA16Sint,
        .dimension = wgpu::TextureDimension::e2D,
        .sampleCount = 4,
    });
    cases.push_back({
        .wgslTypes = {{"texture_multisampled_2d<u32>"}},
        .format = wgpu::TextureFormat::RGBA16Uint,
        .dimension = wgpu::TextureDimension::e2D,
        .sampleCount = 4,
    });

    // Depth textures (including multisampled).
    // TODO(https://crbug.com/435317394): In the future we should allow depth textures to be used as
    // texture_*<f32>.
    cases.push_back({
        .wgslTypes = {{"texture_depth_2d"}},
        .format = wgpu::TextureFormat::Depth32Float,
        .dimension = wgpu::TextureDimension::e2D,
    });
    cases.push_back({
        .wgslTypes = {{"texture_depth_2d_array"}},
        .format = wgpu::TextureFormat::Depth32Float,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::e2DArray,
    });
    cases.push_back({
        .wgslTypes = {{"texture_depth_cube"}},
        .format = wgpu::TextureFormat::Depth32Float,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::Cube,
    });
    cases.push_back({
        .wgslTypes = {{"texture_depth_cube_array"}},
        .format = wgpu::TextureFormat::Depth32Float,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::CubeArray,
    });
    cases.push_back({
        .wgslTypes = {{"texture_depth_multisampled_2d"}},
        .format = wgpu::TextureFormat::Depth32Float,
        .dimension = wgpu::TextureDimension::e2D,
        .sampleCount = 4,
    });

    // Stencil textures can be used as 2D.
    cases.push_back({
        .wgslTypes = {{"texture_2d<u32>"}},
        .format = wgpu::TextureFormat::Stencil8,
        .dimension = wgpu::TextureDimension::e2D,
    });
    cases.push_back({
        .wgslTypes = {{"texture_2d_array<u32>"}},
        .format = wgpu::TextureFormat::Stencil8,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::e2DArray,
    });
    cases.push_back({
        .wgslTypes = {{"texture_cube<u32>"}},
        .format = wgpu::TextureFormat::Stencil8,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::Cube,
    });
    cases.push_back({
        .wgslTypes = {{"texture_cube_array<u32>"}},
        .format = wgpu::TextureFormat::Stencil8,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::CubeArray,
    });

    // Depth-stencil textures with only one aspect selected.
    cases.push_back({
        .wgslTypes = {{"texture_depth_2d"}},
        .format = wgpu::TextureFormat::Depth24PlusStencil8,
        .dimension = wgpu::TextureDimension::e2D,
        .viewAspect = wgpu::TextureAspect::DepthOnly,
    });
    cases.push_back({
        .wgslTypes = {{"texture_2d<u32>"}},
        .format = wgpu::TextureFormat::Depth24PlusStencil8,
        .dimension = wgpu::TextureDimension::e2D,
        .viewAspect = wgpu::TextureAspect::StencilOnly,
    });

    return cases;
}

// TODO(https://crbug.com/435317394): When wgpu::BindGroup::Update() or equivalent is added, test
// that availability is updated when entries in the dynamic binding array are updated.

// Test that hasBinding() works as expected for all support types in WGSL.
TEST_P(DynamicBindingArrayTests, HasBindingTextureCompatibilityAllTypes) {
    auto textureCases = MakeTextureDescForTypeIDCases();

    // Make a dynamic binding array with all of our test textures.
    std::vector<wgpu::BindGroupEntry> entries;
    for (auto [i, textureCase] : Enumerate(textureCases)) {
        wgpu::BindGroupEntry entry{
            .binding = uint32_t(i),
            .textureView = textureCase.CreateTestView(device),
        };
        entries.push_back(entry);
    }

    wgpu::BindGroupLayout bgl = MakeBindGroupLayout(wgpu::DynamicBindingKind::SampledTexture);

    wgpu::BindGroupDynamicBindingArray dynamic;
    dynamic.dynamicArraySize = entries.size();

    wgpu::BindGroupDescriptor bgDesc = {
        .nextInChain = &dynamic,
        .layout = bgl,
        .entryCount = entries.size(),
        .entries = entries.data(),
    };
    wgpu::BindGroup bg = device.CreateBindGroup(&bgDesc);

    // Test hasBinding returning for each of the supported WGSL types, against each texture.
    for (auto wgslType : kWgslSampledTextureTypes) {
        std::vector<bool> expected;
        for (auto textureCase : textureCases) {
            expected.push_back(textureCase.wgslTypes.contains(wgslType));
        }

        TestHasBinding(bgl, bg, expected, 0, wgslType);
    }
}

// Test that calling hasBinding() with values outside of [0, arrayLength) returns 0.
TEST_P(DynamicBindingArrayTests, HasBindingOOBIsFalse) {
    // Create the test pipeline
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_dynamic_binding;

        @group(0) @binding(0) var<storage, read_write> result : array<u32, 4>;
        @group(0) @binding(1) var a : resource_binding;

        @compute @workgroup_size(1) fn getArrayLengths() {
            result[0] = u32(hasBinding<texture_2d<f32>>(a, arrayLength(a) - 1));
            result[1] = u32(hasBinding<texture_2d<f32>>(a, arrayLength(a)));
            result[2] = u32(hasBinding<texture_2d<f32>>(a, arrayLength(a) + 1)) +
                        u32(hasBinding<texture_2d<f32>>(a, arrayLength(a) + 2)) +
                        u32(hasBinding<texture_2d<f32>>(a, arrayLength(a) + 3)) +
                        u32(hasBinding<texture_2d<f32>>(a, arrayLength(a) + 4));
            result[3] = u32(hasBinding<texture_2d<f32>>(a, arrayLength(a) + 10000000));
        }
    )");
    wgpu::ComputePipelineDescriptor csDesc = {.compute = {
                                                  .module = module,
                                              }};
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Create the test resources.
    wgpu::BufferDescriptor bDesc = {
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        .size = 4 * sizeof(uint32_t),
    };
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bDesc);

    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);
    tex.Pin(wgpu::TextureUsage::TextureBinding);

    wgpu::BindGroup bg = MakeBindGroup(pipeline.GetBindGroupLayout(0), 3,
                                       {
                                           {0, resultBuffer},
                                           {1, tex.CreateView()},
                                           {2, tex.CreateView()},
                                           {3, tex.CreateView()},
                                       });

    // Run the test and check results are the expected ones.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetBindGroup(0, bg);
    pass.SetPipeline(pipeline);
    pass.DispatchWorkgroups(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    device.GetQueue().Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(1, resultBuffer, 0);
    EXPECT_BUFFER_U32_EQ(0, resultBuffer, 4);
    EXPECT_BUFFER_U32_EQ(0, resultBuffer, 8);
    EXPECT_BUFFER_U32_EQ(0, resultBuffer, 12);
}

// Check that the default bindings are of size 1 and filled with zeroes. This is not an exhaustive
// test (that's for the CTS) but tries to check a few different interesting cases (MS, DS, Cube, 2D
// array).
TEST_P(DynamicBindingArrayTests, DefaultBindingsAreZeroAndSizeOne) {
    // Create the test pipeline
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_dynamic_binding;

        @group(0) @binding(0) var<storage, read_write> error : u32;
        @group(0) @binding(1) var s : sampler;
        @group(0) @binding(2) var a : resource_binding;

        var<private> checkIndex = 0u;
        fn check(b : bool) {
            if (!b && error == 0) {
                error = 1 + checkIndex;
            }
            checkIndex++;
        }

        @compute @workgroup_size(1) fn checkDefault() {
            // Default texture_2d<f32>
            check(!hasBinding<texture_2d<f32>>(a, 0));
            check(all(textureDimensions(getBinding<texture_2d<f32>>(a, 0)) == vec2(1)));
            check(textureNumLevels(getBinding<texture_2d<f32>>(a, 0)) == 1);
            check(all(textureLoad(getBinding<texture_2d<f32>>(a, 0), vec2(0), 0) == vec4(0, 0, 0, 1)));

            // Default texture_multisampled_2d
            check(!hasBinding<texture_multisampled_2d<u32>>(a, 0));
            check(all(textureDimensions(getBinding<texture_multisampled_2d<u32>>(a, 0)) == vec2(1)));
            check(textureNumSamples(getBinding<texture_multisampled_2d<u32>>(a, 0)) == 4);
            check(all(textureLoad(getBinding<texture_multisampled_2d<u32>>(a, 0), vec2(0), 0) == vec4(0, 0, 0, 1)));

            // Default texture_depth_cube
            check(!hasBinding<texture_depth_cube>(a, 0));
            check(all(textureDimensions(getBinding<texture_depth_cube>(a, 0)) == vec2(1)));
            check(textureNumLevels(getBinding<texture_depth_cube>(a, 0)) == 1);
            check(textureSampleLevel(getBinding<texture_depth_cube>(a, 0), s, vec3(0), 0) == 0);

            // Default texture_2d_array<i32>
            check(!hasBinding<texture_2d_array<i32>>(a, 0));
            check(all(textureDimensions(getBinding<texture_2d_array<i32>>(a, 0)) == vec2(1)));
            check(textureNumLevels(getBinding<texture_2d_array<i32>>(a, 0)) == 1);
            check(textureNumLayers(getBinding<texture_2d_array<i32>>(a, 0)) == 1);
            check(all(textureLoad(getBinding<texture_2d_array<i32>>(a, 0), vec2(0), 0, 0) == vec4(0, 0, 0, 1)));
        }
    )");
    wgpu::ComputePipelineDescriptor csDesc = {.compute = {
                                                  .module = module,
                                              }};
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Create the test resources.
    wgpu::BufferDescriptor bDesc = {
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        .size = sizeof(uint32_t),
    };
    wgpu::Buffer errorBuffer = device.CreateBuffer(&bDesc);

    wgpu::BindGroup bg = MakeBindGroup(pipeline.GetBindGroupLayout(0), 1,
                                       {
                                           {0, errorBuffer},
                                           {1, device.CreateSampler()},
                                       });

    // Run the test and check results are the expected ones.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetBindGroup(0, bg);
    pass.SetPipeline(pipeline);
    pass.DispatchWorkgroups(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    device.GetQueue().Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(0, errorBuffer, 0);
}

// Check that Pin forces zero-initialization of the resources.
TEST_P(DynamicBindingArrayTests, PinDoesZeroInit) {
    // Create the pipeline reading back from the texture.
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_dynamic_binding;

        @group(0) @binding(0) var<storage, read_write> result : u32;
        @group(0) @binding(1) var a : resource_binding;

        @compute @workgroup_size(1) fn readbackPixel() {
            let errorIfNotPresent = u32(!hasBinding<texture_2d<u32>>(a, 0));
            let texel = textureLoad(getBinding<texture_2d<u32>>(a, 0), vec2u(0), 0).r;
            result = errorIfNotPresent + texel;
        }
    )");

    wgpu::ComputePipelineDescriptor csDesc = {.compute = {
                                                  .module = module,
                                              }};
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Create the test resources.
    wgpu::BufferDescriptor bDesc = {
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        .size = sizeof(uint32_t),
    };
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bDesc);

    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Uint,
    };
    wgpu::TextureViewDescriptor vDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    wgpu::BindGroup bg = MakeBindGroup(pipeline.GetBindGroupLayout(0), 3,
                                       {
                                           {0, resultBuffer},
                                           {1, tex.CreateView(&vDesc)},
                                       });

    // Check that Pin does the initial zero init.
    {
        tex.Pin(wgpu::TextureUsage::TextureBinding);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetBindGroup(0, bg);
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroups(1);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);

        EXPECT_BUFFER_U32_EQ(0, resultBuffer, 0);
    }

    // Use a render pass discard to mark the texture as uninitialized again. Use a LoadOp::Clear to
    // set some non-zero value in the texture which hopefully would tell us if the lazy clear didn't
    // happen.
    {
        tex.Unpin();

        wgpu::RenderPassColorAttachment attachment = {
            .view = tex.CreateView(),
            .loadOp = wgpu::LoadOp::Clear,
            .storeOp = wgpu::StoreOp::Discard,
            .clearValue = {.r = 1.0, .g = 0.0, .b = 0.0, .a = 0.0},
        };
        wgpu::RenderPassDescriptor rpDesc = {
            .colorAttachmentCount = 1,
            .colorAttachments = &attachment,
        };

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rpDesc);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);
    }

    // Check that Pin does the zero init after a discard.
    {
        tex.Pin(wgpu::TextureUsage::TextureBinding);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetBindGroup(0, bg);
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroups(1);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);
        tex.Unpin();

        EXPECT_BUFFER_U32_EQ(0, resultBuffer, 0);
    }
}

DAWN_INSTANTIATE_TEST(DynamicBindingArrayTests, D3D12Backend(), MetalBackend(), VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
