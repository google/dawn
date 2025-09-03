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
#include <vector>

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

        EXPECT_BUFFER_U32_RANGE_EQ(expectedU32.data(), resultBuffer, 0, expectedU32.size());
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

// TODO(https://crbug.com/435317394): When wgpu::BindGroup::Update() or equivalent is added, test
// that availability is updated when entries in the dynamic binding array are updated.
// TODO(https://crbug.com/435317394): Add tests that hasBinding() works as expected for all support
// types in WGSL.

DAWN_INSTANTIATE_TEST(DynamicBindingArrayTests, D3D12Backend(), MetalBackend(), VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
