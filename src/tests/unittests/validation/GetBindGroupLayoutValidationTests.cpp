// Copyright 2019 The Dawn Authors
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

#include "tests/unittests/validation/ValidationTest.h"

#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

class GetBindGroupLayoutTests : public ValidationTest {
  protected:
    wgpu::RenderPipeline RenderPipelineFromFragmentShader(const char* shader) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
                [[stage(vertex)]] fn main() -> [[builtin(position)]] vec4<f32> {
                    return vec4<f32>();
                })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, shader);

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.layout = nullptr;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

        return device.CreateRenderPipeline(&descriptor);
    }
};

// Test that GetBindGroupLayout returns the same object for the same index
// and for matching layouts.
TEST_F(GetBindGroupLayoutTests, SameObject) {
    // This test works assuming Dawn Native's object deduplication.
    // Getting the same pointer to equivalent bind group layouts is an implementation detail of Dawn
    // Native.
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        [[block]] struct S {
            pos : vec4<f32>;
        };
        [[group(0), binding(0)]] var<uniform> uniform0 : S;
        [[group(1), binding(0)]] var<uniform> uniform1 : S;

        [[stage(vertex)]] fn main() -> [[builtin(position)]] vec4<f32> {
            var pos : vec4<f32> = uniform0.pos;
            pos = uniform1.pos;
            return vec4<f32>();
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        [[block]] struct S2 {
            pos : vec4<f32>;
        };
        [[group(2), binding(0)]] var<uniform> uniform2 : S2;

        [[block]] struct S3 {
            pos : mat4x4<f32>;
        };
        [[group(3), binding(0)]] var<storage, read_write> storage3 : S3;

        [[stage(fragment)]] fn main() {
            var pos_u : vec4<f32> = uniform2.pos;
            var pos_s : mat4x4<f32> = storage3.pos;
        })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    // The same value is returned for the same index.
    EXPECT_EQ(pipeline.GetBindGroupLayout(0).Get(), pipeline.GetBindGroupLayout(0).Get());

    // Matching bind group layouts at different indices are the same object.
    EXPECT_EQ(pipeline.GetBindGroupLayout(0).Get(), pipeline.GetBindGroupLayout(1).Get());

    // BGLs with different bindings types are different objects.
    EXPECT_NE(pipeline.GetBindGroupLayout(2).Get(), pipeline.GetBindGroupLayout(3).Get());

    // BGLs with different visibilities are different objects.
    EXPECT_NE(pipeline.GetBindGroupLayout(0).Get(), pipeline.GetBindGroupLayout(2).Get());
}

// Test that default BindGroupLayouts cannot be used in the creation of a new PipelineLayout
TEST_F(GetBindGroupLayoutTests, DefaultBindGroupLayoutPipelineCompatibility) {
    wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
        [[block]] struct S {
            pos : vec4<f32>;
        };
        [[group(0), binding(0)]] var<uniform> uniforms : S;

        [[stage(fragment)]] fn main() {
            var pos : vec4<f32> = uniforms.pos;
        })");

    ASSERT_DEVICE_ERROR(utils::MakePipelineLayout(device, {pipeline.GetBindGroupLayout(0)}));
}

// Test that getBindGroupLayout defaults are correct
// - shader stage visibility is the stage that adds the binding.
// - dynamic offsets is false
TEST_F(GetBindGroupLayoutTests, DefaultShaderStageAndDynamicOffsets) {
    // This test works assuming Dawn Native's object deduplication.
    // Getting the same pointer to equivalent bind group layouts is an implementation detail of Dawn
    // Native.
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
        [[block]] struct S {
            pos : vec4<f32>;
        };
        [[group(0), binding(0)]] var<uniform> uniforms : S;

        [[stage(fragment)]] fn main() {
            var pos : vec4<f32> = uniforms.pos;
        })");

    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.buffer.type = wgpu::BufferBindingType::Uniform;
    binding.buffer.minBindingSize = 4 * sizeof(float);

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    // Check that an otherwise compatible bind group layout doesn't match one created as part of a
    // default pipeline layout.
    binding.buffer.hasDynamicOffset = false;
    binding.visibility = wgpu::ShaderStage::Fragment;
    EXPECT_NE(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());

    // Check that any change in visibility doesn't match.
    binding.visibility = wgpu::ShaderStage::Vertex;
    EXPECT_NE(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());

    binding.visibility = wgpu::ShaderStage::Compute;
    EXPECT_NE(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());

    // Check that any change in hasDynamicOffsets doesn't match.
    binding.buffer.hasDynamicOffset = true;
    binding.visibility = wgpu::ShaderStage::Fragment;
    EXPECT_NE(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());
}

TEST_F(GetBindGroupLayoutTests, DefaultTextureSampleType) {
    // This test works assuming Dawn Native's object deduplication.
    // Getting the same pointer to equivalent bind group layouts is an implementation detail of Dawn
    // Native.
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::BindGroupLayout filteringBGL = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
                  wgpu::TextureSampleType::Float},
                 {1, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
                  wgpu::SamplerBindingType::Filtering}});

    wgpu::BindGroupLayout nonFilteringBGL = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
                  wgpu::TextureSampleType::UnfilterableFloat},
                 {1, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
                  wgpu::SamplerBindingType::Filtering}});

    wgpu::ShaderModule emptyVertexModule = utils::CreateShaderModule(device, R"(
        [[group(0), binding(0)]] var myTexture : texture_2d<f32>;
        [[group(0), binding(1)]] var mySampler : sampler;
        [[stage(vertex)]] fn main() -> [[builtin(position)]] vec4<f32> {
            _ = myTexture;
            _ = mySampler;
            return vec4<f32>();
        })");

    wgpu::ShaderModule textureLoadVertexModule = utils::CreateShaderModule(device, R"(
        [[group(0), binding(0)]] var myTexture : texture_2d<f32>;
        [[group(0), binding(1)]] var mySampler : sampler;
        [[stage(vertex)]] fn main() -> [[builtin(position)]] vec4<f32> {
            textureLoad(myTexture, vec2<i32>(), 0);
            _ = mySampler;
            return vec4<f32>();
        })");

    wgpu::ShaderModule textureSampleVertexModule = utils::CreateShaderModule(device, R"(
        [[group(0), binding(0)]] var myTexture : texture_2d<f32>;
        [[group(0), binding(1)]] var mySampler : sampler;
        [[stage(vertex)]] fn main() -> [[builtin(position)]] vec4<f32> {
            textureSampleLevel(myTexture, mySampler, vec2<f32>(), 0.0);
            return vec4<f32>();
        })");

    wgpu::ShaderModule unusedTextureFragmentModule = utils::CreateShaderModule(device, R"(
        [[group(0), binding(0)]] var myTexture : texture_2d<f32>;
        [[group(0), binding(1)]] var mySampler : sampler;
        [[stage(fragment)]] fn main() {
            _ = myTexture;
            _ = mySampler;
        })");

    wgpu::ShaderModule textureLoadFragmentModule = utils::CreateShaderModule(device, R"(
        [[group(0), binding(0)]] var myTexture : texture_2d<f32>;
        [[group(0), binding(1)]] var mySampler : sampler;
        [[stage(fragment)]] fn main() {
            textureLoad(myTexture, vec2<i32>(), 0);
            _ = mySampler;
        })");

    wgpu::ShaderModule textureSampleFragmentModule = utils::CreateShaderModule(device, R"(
        [[group(0), binding(0)]] var myTexture : texture_2d<f32>;
        [[group(0), binding(1)]] var mySampler : sampler;
        [[stage(fragment)]] fn main() {
            textureSample(myTexture, mySampler, vec2<f32>());
        })");

    auto BGLFromModules = [this](wgpu::ShaderModule vertexModule,
                                 wgpu::ShaderModule fragmentModule) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vertexModule;
        descriptor.cFragment.module = fragmentModule;
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        return device.CreateRenderPipeline(&descriptor).GetBindGroupLayout(0);
    };

    // Textures not used default to non-filtering
    EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
        BGLFromModules(emptyVertexModule, unusedTextureFragmentModule).Get(),
        nonFilteringBGL.Get()));
    EXPECT_FALSE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
        BGLFromModules(emptyVertexModule, unusedTextureFragmentModule).Get(), filteringBGL.Get()));

    // Textures used with textureLoad default to non-filtering
    EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
        BGLFromModules(emptyVertexModule, textureLoadFragmentModule).Get(), nonFilteringBGL.Get()));
    EXPECT_FALSE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
        BGLFromModules(emptyVertexModule, textureLoadFragmentModule).Get(), filteringBGL.Get()));

    // Textures used with textureLoad on both stages default to non-filtering
    EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
        BGLFromModules(textureLoadVertexModule, textureLoadFragmentModule).Get(),
        nonFilteringBGL.Get()));
    EXPECT_FALSE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
        BGLFromModules(textureLoadVertexModule, textureLoadFragmentModule).Get(),
        filteringBGL.Get()));

    // Textures used with textureSample default to filtering
    EXPECT_FALSE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
        BGLFromModules(emptyVertexModule, textureSampleFragmentModule).Get(),
        nonFilteringBGL.Get()));
    EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
        BGLFromModules(emptyVertexModule, textureSampleFragmentModule).Get(), filteringBGL.Get()));
    EXPECT_FALSE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
        BGLFromModules(textureSampleVertexModule, unusedTextureFragmentModule).Get(),
        nonFilteringBGL.Get()));
    EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
        BGLFromModules(textureSampleVertexModule, unusedTextureFragmentModule).Get(),
        filteringBGL.Get()));

    // Textures used with both textureLoad and textureSample default to filtering
    EXPECT_FALSE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
        BGLFromModules(textureLoadVertexModule, textureSampleFragmentModule).Get(),
        nonFilteringBGL.Get()));
    EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
        BGLFromModules(textureLoadVertexModule, textureSampleFragmentModule).Get(),
        filteringBGL.Get()));
    EXPECT_FALSE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
        BGLFromModules(textureSampleVertexModule, textureLoadFragmentModule).Get(),
        nonFilteringBGL.Get()));
    EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
        BGLFromModules(textureSampleVertexModule, textureLoadFragmentModule).Get(),
        filteringBGL.Get()));
}

// Test GetBindGroupLayout works with a compute pipeline
TEST_F(GetBindGroupLayoutTests, ComputePipeline) {
    // This test works assuming Dawn Native's object deduplication.
    // Getting the same pointer to equivalent bind group layouts is an implementation detail of Dawn
    // Native.
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::ShaderModule csModule = utils::CreateShaderModule(device, R"(
        [[block]] struct S {
            pos : vec4<f32>;
        };
        [[group(0), binding(0)]] var<uniform> uniforms : S;

        [[stage(compute), workgroup_size(1)]] fn main() {
            var pos : vec4<f32> = uniforms.pos;
        })");

    wgpu::ComputePipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.compute.module = csModule;
    descriptor.compute.entryPoint = "main";

    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&descriptor);

    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.buffer.type = wgpu::BufferBindingType::Uniform;
    binding.visibility = wgpu::ShaderStage::Compute;
    binding.buffer.hasDynamicOffset = false;
    binding.buffer.minBindingSize = 4 * sizeof(float);

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
        device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get()));
}

// Test that the binding type matches the shader.
TEST_F(GetBindGroupLayoutTests, BindingType) {
    // This test works assuming Dawn Native's object deduplication.
    // Getting the same pointer to equivalent bind group layouts is an implementation detail of Dawn
    // Native.
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.buffer.hasDynamicOffset = false;
    binding.buffer.minBindingSize = 4 * sizeof(float);
    binding.visibility = wgpu::ShaderStage::Fragment;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    {
        // Storage buffer binding is not supported in vertex shader.
        binding.visibility = wgpu::ShaderStage::Fragment;
        binding.buffer.type = wgpu::BufferBindingType::Storage;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[block]] struct S {
                pos : vec4<f32>;
            };
            [[group(0), binding(0)]] var<storage, read_write> ssbo : S;

            [[stage(fragment)]] fn main() {
                var pos : vec4<f32> = ssbo.pos;
            })");
        EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get()));
    }
    {
        binding.buffer.type = wgpu::BufferBindingType::Uniform;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[block]] struct S {
                pos : vec4<f32>;
            };
            [[group(0), binding(0)]] var<uniform> uniforms : S;

            [[stage(fragment)]] fn main() {
                var pos : vec4<f32> = uniforms.pos;
            })");
        EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get()));
    }

    {
        binding.buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[block]] struct S {
                pos : vec4<f32>;
            };
            [[group(0), binding(0)]] var<storage, read> ssbo : S;

            [[stage(fragment)]] fn main() {
                var pos : vec4<f32> = ssbo.pos;
            })");
        EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get()));
    }

    binding.buffer.type = wgpu::BufferBindingType::Undefined;
    binding.buffer.minBindingSize = 0;
    {
        binding.texture.sampleType = wgpu::TextureSampleType::UnfilterableFloat;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var myTexture : texture_2d<f32>;

            [[stage(fragment)]] fn main() {
                textureDimensions(myTexture);
            })");
        EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get()));
    }

    {
        binding.texture.multisampled = true;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var myTexture : texture_multisampled_2d<f32>;

            [[stage(fragment)]] fn main() {
                textureDimensions(myTexture);
            })");
        EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get()));
    }

    binding.texture.sampleType = wgpu::TextureSampleType::Undefined;
    {
        binding.sampler.type = wgpu::SamplerBindingType::Filtering;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var mySampler: sampler;

            [[stage(fragment)]] fn main() {
                _ = mySampler;
            })");
        EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get()));
    }
}

// Tests that the external texture binding type matches with a texture_external declared in the
// shader.
TEST_F(GetBindGroupLayoutTests, ExternalTextureBindingType) {
    // This test works assuming Dawn Native's object deduplication.
    // Getting the same pointer to equivalent bind group layouts is an implementation detail of Dawn
    // Native.
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.visibility = wgpu::ShaderStage::Fragment;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    binding.nextInChain = &utils::kExternalTextureBindingLayout;
    wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var myExternalTexture: texture_external;

            [[stage(fragment)]] fn main() {
               _ = myExternalTexture;
            })");
    EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
        device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get()));
}

// Test that texture view dimension matches the shader.
TEST_F(GetBindGroupLayoutTests, ViewDimension) {
    // This test works assuming Dawn Native's object deduplication.
    // Getting the same pointer to equivalent bind group layouts is an implementation detail of Dawn
    // Native.
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.visibility = wgpu::ShaderStage::Fragment;
    binding.texture.sampleType = wgpu::TextureSampleType::UnfilterableFloat;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    {
        binding.texture.viewDimension = wgpu::TextureViewDimension::e1D;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var myTexture : texture_1d<f32>;

            [[stage(fragment)]] fn main() {
                textureDimensions(myTexture);
            })");
        EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get()));
    }

    {
        binding.texture.viewDimension = wgpu::TextureViewDimension::e2D;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var myTexture : texture_2d<f32>;

            [[stage(fragment)]] fn main() {
                textureDimensions(myTexture);
            })");
        EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get()));
    }

    {
        binding.texture.viewDimension = wgpu::TextureViewDimension::e2DArray;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var myTexture : texture_2d_array<f32>;

            [[stage(fragment)]] fn main() {
                textureDimensions(myTexture);
            })");
        EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get()));
    }

    {
        binding.texture.viewDimension = wgpu::TextureViewDimension::e3D;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var myTexture : texture_3d<f32>;

            [[stage(fragment)]] fn main() {
                textureDimensions(myTexture);
            })");
        EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get()));
    }

    {
        binding.texture.viewDimension = wgpu::TextureViewDimension::Cube;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var myTexture : texture_cube<f32>;

            [[stage(fragment)]] fn main() {
                textureDimensions(myTexture);
            })");
        EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get()));
    }

    {
        binding.texture.viewDimension = wgpu::TextureViewDimension::CubeArray;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var myTexture : texture_cube_array<f32>;

            [[stage(fragment)]] fn main() {
                textureDimensions(myTexture);
            })");
        EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get()));
    }
}

// Test that texture component type matches the shader.
TEST_F(GetBindGroupLayoutTests, TextureComponentType) {
    // This test works assuming Dawn Native's object deduplication.
    // Getting the same pointer to equivalent bind group layouts is an implementation detail of Dawn
    // Native.
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.visibility = wgpu::ShaderStage::Fragment;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    {
        binding.texture.sampleType = wgpu::TextureSampleType::UnfilterableFloat;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var myTexture : texture_2d<f32>;

            [[stage(fragment)]] fn main() {
                textureDimensions(myTexture);
            })");
        EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get()));
    }

    {
        binding.texture.sampleType = wgpu::TextureSampleType::Sint;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var myTexture : texture_2d<i32>;

            [[stage(fragment)]] fn main() {
                textureDimensions(myTexture);
            })");
        EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get()));
    }

    {
        binding.texture.sampleType = wgpu::TextureSampleType::Uint;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var myTexture : texture_2d<u32>;

            [[stage(fragment)]] fn main() {
                textureDimensions(myTexture);
            })");
        EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get()));
    }
}

// Test that binding= indices match.
TEST_F(GetBindGroupLayoutTests, BindingIndices) {
    // This test works assuming Dawn Native's object deduplication.
    // Getting the same pointer to equivalent bind group layouts is an implementation detail of Dawn
    // Native.
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::BindGroupLayoutEntry binding = {};
    binding.visibility = wgpu::ShaderStage::Fragment;
    binding.buffer.type = wgpu::BufferBindingType::Uniform;
    binding.buffer.hasDynamicOffset = false;
    binding.buffer.minBindingSize = 4 * sizeof(float);

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    {
        binding.binding = 0;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[block]] struct S {
                pos : vec4<f32>;
            };
            [[group(0), binding(0)]] var<uniform> uniforms : S;

            [[stage(fragment)]] fn main() {
                var pos : vec4<f32> = uniforms.pos;
            })");
        EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get()));
    }

    {
        binding.binding = 1;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[block]] struct S {
                pos : vec4<f32>;
            };
            [[group(0), binding(1)]] var<uniform> uniforms : S;

            [[stage(fragment)]] fn main() {
                var pos : vec4<f32> = uniforms.pos;
            })");
        EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get()));
    }

    {
        binding.binding = 2;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[block]] struct S {
                pos : vec4<f32>;
            };
            [[group(0), binding(1)]] var<uniform> uniforms : S;

            [[stage(fragment)]] fn main() {
                var pos : vec4<f32> = uniforms.pos;
            })");
        EXPECT_FALSE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get()));
    }
}

// Test it is valid to have duplicate bindings in the shaders.
TEST_F(GetBindGroupLayoutTests, DuplicateBinding) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        [[block]] struct S {
            pos : vec4<f32>;
        };
        [[group(0), binding(0)]] var<uniform> uniform0 : S;
        [[group(1), binding(0)]] var<uniform> uniform1 : S;

        [[stage(vertex)]] fn main() -> [[builtin(position)]] vec4<f32> {
            var pos : vec4<f32> = uniform0.pos;
            pos = uniform1.pos;
            return vec4<f32>();
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        [[block]] struct S {
            pos : vec4<f32>;
        };
        [[group(1), binding(0)]] var<uniform> uniforms : S;

        [[stage(fragment)]] fn main() {
            var pos : vec4<f32> = uniforms.pos;
        })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

    device.CreateRenderPipeline(&descriptor);
}

// Test that minBufferSize is set on the BGL and that the max of the min buffer sizes is used.
TEST_F(GetBindGroupLayoutTests, MinBufferSize) {
    // This test works assuming Dawn Native's object deduplication.
    // Getting the same pointer to equivalent bind group layouts is an implementation detail of Dawn
    // Native.
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::ShaderModule vsModule4 = utils::CreateShaderModule(device, R"(
        [[block]] struct S {
            pos : f32;
        };
        [[group(0), binding(0)]] var<uniform> uniforms : S;

        [[stage(vertex)]] fn main() -> [[builtin(position)]] vec4<f32> {
            var pos : f32 = uniforms.pos;
            return vec4<f32>();
        })");

    wgpu::ShaderModule vsModule64 = utils::CreateShaderModule(device, R"(
        [[block]] struct S {
            pos : mat4x4<f32>;
        };
        [[group(0), binding(0)]] var<uniform> uniforms : S;

        [[stage(vertex)]] fn main() -> [[builtin(position)]] vec4<f32> {
            var pos : mat4x4<f32> = uniforms.pos;
            return vec4<f32>();
        })");

    wgpu::ShaderModule fsModule4 = utils::CreateShaderModule(device, R"(
        [[block]] struct S {
            pos : f32;
        };
        [[group(0), binding(0)]] var<uniform> uniforms : S;

        [[stage(fragment)]] fn main() {
            var pos : f32 = uniforms.pos;
        })");

    wgpu::ShaderModule fsModule64 = utils::CreateShaderModule(device, R"(
        [[block]] struct S {
            pos : mat4x4<f32>;
        };
        [[group(0), binding(0)]] var<uniform> uniforms : S;

        [[stage(fragment)]] fn main() {
            var pos : mat4x4<f32> = uniforms.pos;
        })");

    // Create BGLs with minBufferBindingSize 4 and 64.
    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.buffer.type = wgpu::BufferBindingType::Uniform;
    binding.visibility = wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Vertex;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    binding.buffer.minBindingSize = 4;
    wgpu::BindGroupLayout bgl4 = device.CreateBindGroupLayout(&desc);
    binding.buffer.minBindingSize = 64;
    wgpu::BindGroupLayout bgl64 = device.CreateBindGroupLayout(&desc);

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

    // Check with both stages using 4 bytes.
    {
        descriptor.vertex.module = vsModule4;
        descriptor.cFragment.module = fsModule4;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);
        EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            pipeline.GetBindGroupLayout(0).Get(), bgl4.Get()));
    }

    // Check that the max is taken between 4 and 64.
    {
        descriptor.vertex.module = vsModule64;
        descriptor.cFragment.module = fsModule4;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);
        EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            pipeline.GetBindGroupLayout(0).Get(), bgl64.Get()));
    }

    // Check that the order doesn't change that the max is taken.
    {
        descriptor.vertex.module = vsModule4;
        descriptor.cFragment.module = fsModule64;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);
        EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            pipeline.GetBindGroupLayout(0).Get(), bgl64.Get()));
    }
}

// Test that the visibility is correctly aggregated if two stages have the exact same binding.
TEST_F(GetBindGroupLayoutTests, StageAggregation) {
    // This test works assuming Dawn Native's object deduplication.
    // Getting the same pointer to equivalent bind group layouts is an implementation detail of Dawn
    // Native.
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::ShaderModule vsModuleNoSampler = utils::CreateShaderModule(device, R"(
        [[stage(vertex)]] fn main() -> [[builtin(position)]] vec4<f32> {
            return vec4<f32>();
        })");

    wgpu::ShaderModule vsModuleSampler = utils::CreateShaderModule(device, R"(
        [[group(0), binding(0)]] var mySampler: sampler;
        [[stage(vertex)]] fn main() -> [[builtin(position)]] vec4<f32> {
            _ = mySampler;
            return vec4<f32>();
        })");

    wgpu::ShaderModule fsModuleNoSampler = utils::CreateShaderModule(device, R"(
        [[stage(fragment)]] fn main() {
        })");

    wgpu::ShaderModule fsModuleSampler = utils::CreateShaderModule(device, R"(
        [[group(0), binding(0)]] var mySampler: sampler;
        [[stage(fragment)]] fn main() {
            _ = mySampler;
        })");

    // Create BGLs with minBufferBindingSize 4 and 64.
    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.sampler.type = wgpu::SamplerBindingType::Filtering;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

    // Check with only the vertex shader using the sampler
    {
        descriptor.vertex.module = vsModuleSampler;
        descriptor.cFragment.module = fsModuleNoSampler;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        binding.visibility = wgpu::ShaderStage::Vertex;
        EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            pipeline.GetBindGroupLayout(0).Get(), device.CreateBindGroupLayout(&desc).Get()));
    }

    // Check with only the fragment shader using the sampler
    {
        descriptor.vertex.module = vsModuleNoSampler;
        descriptor.cFragment.module = fsModuleSampler;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        binding.visibility = wgpu::ShaderStage::Fragment;
        EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            pipeline.GetBindGroupLayout(0).Get(), device.CreateBindGroupLayout(&desc).Get()));
    }

    // Check with both shaders using the sampler
    {
        descriptor.vertex.module = vsModuleSampler;
        descriptor.cFragment.module = fsModuleSampler;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        binding.visibility = wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Vertex;
        EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
            pipeline.GetBindGroupLayout(0).Get(), device.CreateBindGroupLayout(&desc).Get()));
    }
}

// Test it is invalid to have conflicting binding types in the shaders.
TEST_F(GetBindGroupLayoutTests, ConflictingBindingType) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        [[block]] struct S {
            pos : vec4<f32>;
        };
        [[group(0), binding(0)]] var<uniform> ubo : S;

        [[stage(vertex)]] fn main() -> [[builtin(position)]] vec4<f32> {
            var pos : vec4<f32> = ubo.pos;
            return vec4<f32>();
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        [[block]] struct S {
            pos : vec4<f32>;
        };
        [[group(0), binding(0)]] var<storage, read_write> ssbo : S;

        [[stage(fragment)]] fn main() {
            var pos : vec4<f32> = ssbo.pos;
        })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;

    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

// Test it is invalid to have conflicting binding texture multisampling in the shaders.
TEST_F(GetBindGroupLayoutTests, ConflictingBindingTextureMultisampling) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        [[group(0), binding(0)]] var myTexture : texture_2d<f32>;

        [[stage(vertex)]] fn main() -> [[builtin(position)]] vec4<f32> {
            textureDimensions(myTexture);
            return vec4<f32>();
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        [[group(0), binding(0)]] var myTexture : texture_multisampled_2d<f32>;

        [[stage(fragment)]] fn main() {
            textureDimensions(myTexture);
        })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;

    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

// Test it is invalid to have conflicting binding texture dimension in the shaders.
TEST_F(GetBindGroupLayoutTests, ConflictingBindingViewDimension) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        [[group(0), binding(0)]] var myTexture : texture_2d<f32>;

        [[stage(vertex)]] fn main() -> [[builtin(position)]] vec4<f32> {
            textureDimensions(myTexture);
            return vec4<f32>();
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        [[group(0), binding(0)]] var myTexture : texture_3d<f32>;

        [[stage(fragment)]] fn main() {
            textureDimensions(myTexture);
        })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;

    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

// Test it is invalid to have conflicting binding texture component type in the shaders.
TEST_F(GetBindGroupLayoutTests, ConflictingBindingTextureComponentType) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        [[group(0), binding(0)]] var myTexture : texture_2d<f32>;

        [[stage(vertex)]] fn main() -> [[builtin(position)]] vec4<f32> {
            textureDimensions(myTexture);
            return vec4<f32>();
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        [[group(0), binding(0)]] var myTexture : texture_2d<i32>;

        [[stage(fragment)]] fn main() {
            textureDimensions(myTexture);
        })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;

    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

// Test it is an error to query an out of range bind group layout.
TEST_F(GetBindGroupLayoutTests, OutOfRangeIndex) {
    ASSERT_DEVICE_ERROR(RenderPipelineFromFragmentShader(R"(
        [[stage(fragment)]] fn main() {
        })")
                            .GetBindGroupLayout(kMaxBindGroups));

    ASSERT_DEVICE_ERROR(RenderPipelineFromFragmentShader(R"(
        [[stage(fragment)]] fn main() {
        })")
                            .GetBindGroupLayout(kMaxBindGroups + 1));
}

// Test that unused indices return the empty bind group layout.
TEST_F(GetBindGroupLayoutTests, UnusedIndex) {
    // This test works assuming Dawn Native's object deduplication.
    // Getting the same pointer to equivalent bind group layouts is an implementation detail of Dawn
    // Native.
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
        [[block]] struct S {
            pos : vec4<f32>;
        };
        [[group(0), binding(0)]] var<uniform> uniforms0 : S;
        [[group(2), binding(0)]] var<uniform> uniforms2 : S;

        [[stage(fragment)]] fn main() {
            var pos : vec4<f32> = uniforms0.pos;
            pos = uniforms2.pos;
        })");

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 0;
    desc.entries = nullptr;

    wgpu::BindGroupLayout emptyBindGroupLayout = device.CreateBindGroupLayout(&desc);

    EXPECT_FALSE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
        pipeline.GetBindGroupLayout(0).Get(), emptyBindGroupLayout.Get()));  // Used
    EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
        pipeline.GetBindGroupLayout(1).Get(), emptyBindGroupLayout.Get()));  // Not Used.
    EXPECT_FALSE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
        pipeline.GetBindGroupLayout(2).Get(), emptyBindGroupLayout.Get()));  // Used.
    EXPECT_TRUE(dawn_native::BindGroupLayoutBindingsEqualForTesting(
        pipeline.GetBindGroupLayout(3).Get(), emptyBindGroupLayout.Get()));  // Not used
}

// Test that after explicitly creating a pipeline with a pipeline layout, calling
// GetBindGroupLayout reflects the same bind group layouts.
TEST_F(GetBindGroupLayoutTests, Reflection) {
    // This test works assuming Dawn Native's object deduplication.
    // Getting the same pointer to equivalent bind group layouts is an implementation detail of Dawn
    // Native.
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.buffer.type = wgpu::BufferBindingType::Uniform;
    binding.visibility = wgpu::ShaderStage::Vertex;

    wgpu::BindGroupLayoutDescriptor bglDesc = {};
    bglDesc.entryCount = 1;
    bglDesc.entries = &binding;

    wgpu::BindGroupLayout bindGroupLayout = device.CreateBindGroupLayout(&bglDesc);

    wgpu::PipelineLayoutDescriptor pipelineLayoutDesc = {};
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = &bindGroupLayout;

    wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&pipelineLayoutDesc);

    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        [[block]] struct S {
            pos : vec4<f32>;
        };
        [[group(0), binding(0)]] var<uniform> uniforms : S;

        [[stage(vertex)]] fn main() -> [[builtin(position)]] vec4<f32> {
            var pos : vec4<f32> = uniforms.pos;
            return vec4<f32>();
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        [[stage(fragment)]] fn main() {
        })");

    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.layout = pipelineLayout;
    pipelineDesc.vertex.module = vsModule;
    pipelineDesc.cFragment.module = fsModule;
    pipelineDesc.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

    EXPECT_EQ(pipeline.GetBindGroupLayout(0).Get(), bindGroupLayout.Get());

    {
        wgpu::BindGroupLayoutDescriptor emptyDesc = {};
        emptyDesc.entryCount = 0;
        emptyDesc.entries = nullptr;

        wgpu::BindGroupLayout emptyBindGroupLayout = device.CreateBindGroupLayout(&emptyDesc);

        // Check that the rest of the bind group layouts reflect the empty one.
        EXPECT_EQ(pipeline.GetBindGroupLayout(1).Get(), emptyBindGroupLayout.Get());
        EXPECT_EQ(pipeline.GetBindGroupLayout(2).Get(), emptyBindGroupLayout.Get());
        EXPECT_EQ(pipeline.GetBindGroupLayout(3).Get(), emptyBindGroupLayout.Get());
    }
}

// Test that fragment output validation is for the correct entryPoint
TEST_F(GetBindGroupLayoutTests, FromCorrectEntryPoint) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        [[block]] struct Data {
            data : f32;
        };
        [[group(0), binding(0)]] var<storage, read_write> data0 : Data;
        [[group(0), binding(1)]] var<storage, read_write> data1 : Data;

        [[stage(compute), workgroup_size(1)]] fn compute0() {
            data0.data = 0.0;
        }

        [[stage(compute), workgroup_size(1)]] fn compute1() {
            data1.data = 0.0;
        }
    )");

    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.compute.module = module;

    // Get each entryPoint's BGL.
    pipelineDesc.compute.entryPoint = "compute0";
    wgpu::ComputePipeline pipeline0 = device.CreateComputePipeline(&pipelineDesc);
    wgpu::BindGroupLayout bgl0 = pipeline0.GetBindGroupLayout(0);

    pipelineDesc.compute.entryPoint = "compute1";
    wgpu::ComputePipeline pipeline1 = device.CreateComputePipeline(&pipelineDesc);
    wgpu::BindGroupLayout bgl1 = pipeline1.GetBindGroupLayout(0);

    // Create the buffer used in the bindgroups.
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 4;
    bufferDesc.usage = wgpu::BufferUsage::Storage;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    // Success case, the BGL matches the descriptor for the bindgroup.
    utils::MakeBindGroup(device, bgl0, {{0, buffer}});
    utils::MakeBindGroup(device, bgl1, {{1, buffer}});

    // Error case, the BGL doesn't match the descriptor for the bindgroup.
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl0, {{1, buffer}}));
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl1, {{0, buffer}}));
}
