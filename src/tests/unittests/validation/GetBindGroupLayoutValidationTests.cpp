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
        wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
                [[stage(vertex)]] fn main() -> void {
                })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, shader);

        utils::ComboRenderPipelineDescriptor descriptor(device);
        descriptor.layout = nullptr;
        descriptor.vertexStage.module = vsModule;
        descriptor.cFragmentStage.module = fsModule;

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

    wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct S {
            [[offset(0)]] pos : vec4<f32>;
        };
        [[group(0), binding(0)]] var<uniform> uniform0 : S;
        [[group(1), binding(0)]] var<uniform> uniform1 : S;

        [[stage(vertex)]] fn main() -> void {
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct S2 {
            [[offset(0)]] pos : vec4<f32>;
        };
        [[group(2), binding(0)]] var<uniform> uniform2 : S2;

        [[block]] struct S3 {
            [[offset(0)]] pos : mat4x4<f32>;
        };
        [[group(3), binding(0)]] var<storage_buffer> storage3 : S3;

        [[stage(fragment)]] fn main() -> void {
        })");

    utils::ComboRenderPipelineDescriptor descriptor(device);
    descriptor.layout = nullptr;
    descriptor.vertexStage.module = vsModule;
    descriptor.cFragmentStage.module = fsModule;

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
            [[offset(0)]] pos : vec4<f32>;
        };
        [[group(0), binding(0)]] var<uniform> uniforms : S;

        [[stage(fragment)]] fn main() -> void {
        })");

    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.buffer.type = wgpu::BufferBindingType::Uniform;
    binding.buffer.minBindingSize = 4 * sizeof(float);

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    // Check that visibility and dynamic offsets match
    binding.buffer.hasDynamicOffset = false;
    binding.visibility = wgpu::ShaderStage::Fragment;
    EXPECT_EQ(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());

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

// Test GetBindGroupLayout works with a compute pipeline
TEST_F(GetBindGroupLayoutTests, ComputePipeline) {
    // This test works assuming Dawn Native's object deduplication.
    // Getting the same pointer to equivalent bind group layouts is an implementation detail of Dawn
    // Native.
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::ShaderModule csModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct S {
            [[offset(0)]] pos : vec4<f32>;
        };
        [[group(0), binding(0)]] var<uniform> uniforms : S;

        [[stage(compute)]] fn main() -> void {
        })");

    wgpu::ComputePipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.computeStage.module = csModule;
    descriptor.computeStage.entryPoint = "main";

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

    EXPECT_EQ(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());
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
                [[offset(0)]] pos : vec4<f32>;
            };
            [[group(0), binding(0)]] var<storage_buffer> ssbo : S;

            [[stage(fragment)]] fn main() -> void {
            })");
        EXPECT_EQ(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());
    }
    {
        binding.buffer.type = wgpu::BufferBindingType::Uniform;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[block]] struct S {
                [[offset(0)]] pos : vec4<f32>;
            };
            [[group(0), binding(0)]] var<uniform> uniforms : S;

            [[stage(fragment)]] fn main() -> void {
            })");
        EXPECT_EQ(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());
    }

    {
        binding.buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[block]] struct S {
                [[offset(0)]] pos : vec4<f32>;
            };
            [[group(0), binding(0)]] var<storage_buffer> ssbo : [[access(read)]] S;

            [[stage(fragment)]] fn main() -> void {
            })");
        EXPECT_EQ(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());
    }

    binding.buffer.type = wgpu::BufferBindingType::Undefined;
    binding.buffer.minBindingSize = 0;
    {
        binding.texture.sampleType = wgpu::TextureSampleType::Float;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var<uniform_constant> myTexture : texture_2d<f32>;

            [[stage(fragment)]] fn main() -> void {
            })");
        EXPECT_EQ(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());
    }

    {
        binding.texture.multisampled = true;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var<uniform_constant> myTexture : texture_multisampled_2d<f32>;

            [[stage(fragment)]] fn main() -> void {
            })");
        EXPECT_EQ(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());
    }

    binding.texture.sampleType = wgpu::TextureSampleType::Undefined;
    {
        binding.sampler.type = wgpu::SamplerBindingType::Filtering;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var<uniform_constant> mySampler: sampler;

            [[stage(fragment)]] fn main() -> void {
            })");
        EXPECT_EQ(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());
    }
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
    binding.texture.sampleType = wgpu::TextureSampleType::Float;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    {
        binding.texture.viewDimension = wgpu::TextureViewDimension::e1D;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var<uniform_constant> myTexture : texture_1d<f32>;

            [[stage(fragment)]] fn main() -> void {
            })");
        EXPECT_EQ(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());
    }

    {
        binding.texture.viewDimension = wgpu::TextureViewDimension::e2D;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var<uniform_constant> myTexture : texture_2d<f32>;

            [[stage(fragment)]] fn main() -> void {
            })");
        EXPECT_EQ(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());
    }

    {
        binding.texture.viewDimension = wgpu::TextureViewDimension::e2DArray;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var<uniform_constant> myTexture : texture_2d_array<f32>;

            [[stage(fragment)]] fn main() -> void {
            })");
        EXPECT_EQ(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());
    }

    {
        binding.texture.viewDimension = wgpu::TextureViewDimension::e3D;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var<uniform_constant> myTexture : texture_3d<f32>;

            [[stage(fragment)]] fn main() -> void {
            })");
        EXPECT_EQ(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());
    }

    {
        binding.texture.viewDimension = wgpu::TextureViewDimension::Cube;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var<uniform_constant> myTexture : texture_cube<f32>;

            [[stage(fragment)]] fn main() -> void {
            })");
        EXPECT_EQ(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());
    }

    {
        binding.texture.viewDimension = wgpu::TextureViewDimension::CubeArray;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var<uniform_constant> myTexture : texture_cube_array<f32>;

            [[stage(fragment)]] fn main() -> void {
            })");
        EXPECT_EQ(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());
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
        binding.texture.sampleType = wgpu::TextureSampleType::Float;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var<uniform_constant> myTexture : texture_2d<f32>;

            [[stage(fragment)]] fn main() -> void {
            })");
        EXPECT_EQ(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());
    }

    {
        binding.texture.sampleType = wgpu::TextureSampleType::Sint;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var<uniform_constant> myTexture : texture_2d<i32>;

            [[stage(fragment)]] fn main() -> void {
            })");
        EXPECT_EQ(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());
    }

    {
        binding.texture.sampleType = wgpu::TextureSampleType::Uint;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[group(0), binding(0)]] var<uniform_constant> myTexture : texture_2d<u32>;

            [[stage(fragment)]] fn main() -> void {
            })");
        EXPECT_EQ(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());
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
                [[offset(0)]] pos : vec4<f32>;
            };
            [[group(0), binding(0)]] var<uniform> uniforms : S;

            [[stage(fragment)]] fn main() -> void {
            })");
        EXPECT_EQ(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());
    }

    {
        binding.binding = 1;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[block]] struct S {
                [[offset(0)]] pos : vec4<f32>;
            };
            [[group(0), binding(1)]] var<uniform> uniforms : S;

            [[stage(fragment)]] fn main() -> void {
            })");
        EXPECT_EQ(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());
    }

    {
        binding.binding = 2;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            [[block]] struct S {
                [[offset(0)]] pos : vec4<f32>;
            };
            [[group(0), binding(1)]] var<uniform> uniforms : S;

            [[stage(fragment)]] fn main() -> void {
            })");
        EXPECT_NE(device.CreateBindGroupLayout(&desc).Get(), pipeline.GetBindGroupLayout(0).Get());
    }
}

// Test it is valid to have duplicate bindings in the shaders.
TEST_F(GetBindGroupLayoutTests, DuplicateBinding) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct S {
            [[offset(0)]] pos : vec4<f32>;
        };
        [[group(0), binding(0)]] var<uniform> uniform0 : S;
        [[group(1), binding(0)]] var<uniform> uniform1 : S;

        [[stage(vertex)]] fn main() -> void {
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct S {
            [[offset(0)]] pos : vec4<f32>;
        };
        [[group(1), binding(0)]] var<uniform> uniforms : S;

        [[stage(fragment)]] fn main() -> void {
        })");

    utils::ComboRenderPipelineDescriptor descriptor(device);
    descriptor.layout = nullptr;
    descriptor.vertexStage.module = vsModule;
    descriptor.cFragmentStage.module = fsModule;

    device.CreateRenderPipeline(&descriptor);
}

// Test that minBufferSize is set on the BGL and that the max of the min buffer sizes is used.
TEST_F(GetBindGroupLayoutTests, MinBufferSize) {
    // This test works assuming Dawn Native's object deduplication.
    // Getting the same pointer to equivalent bind group layouts is an implementation detail of Dawn
    // Native.
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::ShaderModule vsModule4 = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct S {
            [[offset(0)]] pos : f32;
        };
        [[group(0), binding(0)]] var<uniform> uniforms : S;

        [[stage(vertex)]] fn main() -> void {
        })");

    wgpu::ShaderModule vsModule64 = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct S {
            [[offset(0)]] pos : mat4x4<f32>;
        };
        [[group(0), binding(0)]] var<uniform> uniforms : S;

        [[stage(vertex)]] fn main() -> void {
        })");

    wgpu::ShaderModule fsModule4 = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct S {
            [[offset(0)]] pos : f32;
        };
        [[group(0), binding(0)]] var<uniform> uniforms : S;

        [[stage(fragment)]] fn main() -> void {
        })");

    wgpu::ShaderModule fsModule64 = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct S {
            [[offset(0)]] pos : mat4x4<f32>;
        };
        [[group(0), binding(0)]] var<uniform> uniforms : S;

        [[stage(fragment)]] fn main() -> void {
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

    utils::ComboRenderPipelineDescriptor descriptor(device);
    descriptor.layout = nullptr;

    // Check with both stages using 4 bytes.
    {
        descriptor.vertexStage.module = vsModule4;
        descriptor.cFragmentStage.module = fsModule4;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);
        EXPECT_EQ(pipeline.GetBindGroupLayout(0).Get(), bgl4.Get());
    }

    // Check that the max is taken between 4 and 64.
    {
        descriptor.vertexStage.module = vsModule64;
        descriptor.cFragmentStage.module = fsModule4;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);
        EXPECT_EQ(pipeline.GetBindGroupLayout(0).Get(), bgl64.Get());
    }

    // Check that the order doesn't change that the max is taken.
    {
        descriptor.vertexStage.module = vsModule4;
        descriptor.cFragmentStage.module = fsModule64;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);
        EXPECT_EQ(pipeline.GetBindGroupLayout(0).Get(), bgl64.Get());
    }
}

// Test that the visibility is correctly aggregated if two stages have the exact same binding.
TEST_F(GetBindGroupLayoutTests, StageAggregation) {
    // This test works assuming Dawn Native's object deduplication.
    // Getting the same pointer to equivalent bind group layouts is an implementation detail of Dawn
    // Native.
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::ShaderModule vsModuleNoSampler = utils::CreateShaderModuleFromWGSL(device, R"(
        [[stage(vertex)]] fn main() -> void {
        })");

    wgpu::ShaderModule vsModuleSampler = utils::CreateShaderModuleFromWGSL(device, R"(
        [[group(0), binding(0)]] var<uniform_constant> mySampler: sampler;
        [[stage(vertex)]] fn main() -> void {
        })");

    wgpu::ShaderModule fsModuleNoSampler = utils::CreateShaderModuleFromWGSL(device, R"(
        [[stage(fragment)]] fn main() -> void {
        })");

    wgpu::ShaderModule fsModuleSampler = utils::CreateShaderModuleFromWGSL(device, R"(
        [[group(0), binding(0)]] var<uniform_constant> mySampler: sampler;
        [[stage(fragment)]] fn main() -> void {
        })");

    // Create BGLs with minBufferBindingSize 4 and 64.
    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.sampler.type = wgpu::SamplerBindingType::Filtering;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    utils::ComboRenderPipelineDescriptor descriptor(device);
    descriptor.layout = nullptr;

    // Check with only the vertex shader using the sampler
    {
        descriptor.vertexStage.module = vsModuleSampler;
        descriptor.cFragmentStage.module = fsModuleNoSampler;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        binding.visibility = wgpu::ShaderStage::Vertex;
        EXPECT_EQ(pipeline.GetBindGroupLayout(0).Get(), device.CreateBindGroupLayout(&desc).Get());
    }

    // Check with only the fragment shader using the sampler
    {
        descriptor.vertexStage.module = vsModuleNoSampler;
        descriptor.cFragmentStage.module = fsModuleSampler;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        binding.visibility = wgpu::ShaderStage::Fragment;
        EXPECT_EQ(pipeline.GetBindGroupLayout(0).Get(), device.CreateBindGroupLayout(&desc).Get());
    }

    // Check with both shaders using the sampler
    {
        descriptor.vertexStage.module = vsModuleSampler;
        descriptor.cFragmentStage.module = fsModuleSampler;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        binding.visibility = wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Vertex;
        EXPECT_EQ(pipeline.GetBindGroupLayout(0).Get(), device.CreateBindGroupLayout(&desc).Get());
    }
}

// Test it is invalid to have conflicting binding types in the shaders.
TEST_F(GetBindGroupLayoutTests, ConflictingBindingType) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct S {
            [[offset(0)]] pos : vec4<f32>;
        };
        [[group(0), binding(0)]] var<uniform> ubo : S;

        [[stage(vertex)]] fn main() -> void {
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct S {
            [[offset(0)]] pos : vec4<f32>;
        };
        [[group(0), binding(0)]] var<storage_buffer> ssbo : S;

        [[stage(fragment)]] fn main() -> void {
        })");

    utils::ComboRenderPipelineDescriptor descriptor(device);
    descriptor.layout = nullptr;
    descriptor.vertexStage.module = vsModule;
    descriptor.cFragmentStage.module = fsModule;

    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

// Test it is invalid to have conflicting binding texture multisampling in the shaders.
TEST_F(GetBindGroupLayoutTests, ConflictingBindingTextureMultisampling) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[group(0), binding(0)]] var<uniform_constant> myTexture : texture_2d<f32>;

        [[stage(vertex)]] fn main() -> void {
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[group(0), binding(0)]] var<uniform_constant> myTexture : texture_multisampled_2d<f32>;

        [[stage(fragment)]] fn main() -> void {
        })");

    utils::ComboRenderPipelineDescriptor descriptor(device);
    descriptor.layout = nullptr;
    descriptor.vertexStage.module = vsModule;
    descriptor.cFragmentStage.module = fsModule;

    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

// Test it is invalid to have conflicting binding texture dimension in the shaders.
TEST_F(GetBindGroupLayoutTests, ConflictingBindingViewDimension) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[group(0), binding(0)]] var<uniform_constant> myTexture : texture_2d<f32>;

        [[stage(vertex)]] fn main() -> void {
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[group(0), binding(0)]] var<uniform_constant> myTexture : texture_3d<f32>;

        [[stage(fragment)]] fn main() -> void {
        })");

    utils::ComboRenderPipelineDescriptor descriptor(device);
    descriptor.layout = nullptr;
    descriptor.vertexStage.module = vsModule;
    descriptor.cFragmentStage.module = fsModule;

    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

// Test it is invalid to have conflicting binding texture component type in the shaders.
TEST_F(GetBindGroupLayoutTests, ConflictingBindingTextureComponentType) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[group(0), binding(0)]] var<uniform_constant> myTexture : texture_2d<f32>;

        [[stage(vertex)]] fn main() -> void {
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[group(0), binding(0)]] var<uniform_constant> myTexture : texture_2d<i32>;

        [[stage(fragment)]] fn main() -> void {
        })");

    utils::ComboRenderPipelineDescriptor descriptor(device);
    descriptor.layout = nullptr;
    descriptor.vertexStage.module = vsModule;
    descriptor.cFragmentStage.module = fsModule;

    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

// Test it is an error to query an out of range bind group layout.
TEST_F(GetBindGroupLayoutTests, OutOfRangeIndex) {
    ASSERT_DEVICE_ERROR(RenderPipelineFromFragmentShader(R"(
        [[stage(fragment)]] fn main() -> void {
        })")
                            .GetBindGroupLayout(kMaxBindGroups));

    ASSERT_DEVICE_ERROR(RenderPipelineFromFragmentShader(R"(
        [[stage(fragment)]] fn main() -> void {
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
            [[offset(0)]] pos : vec4<f32>;
        };
        [[group(0), binding(0)]] var<uniform> uniforms0 : S;
        [[group(2), binding(0)]] var<uniform> uniforms2 : S;

        [[stage(fragment)]] fn main() -> void {
        })");

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 0;
    desc.entries = nullptr;

    wgpu::BindGroupLayout emptyBindGroupLayout = device.CreateBindGroupLayout(&desc);

    EXPECT_NE(pipeline.GetBindGroupLayout(0).Get(), emptyBindGroupLayout.Get());  // Used
    EXPECT_EQ(pipeline.GetBindGroupLayout(1).Get(), emptyBindGroupLayout.Get());  // Not Used.
    EXPECT_NE(pipeline.GetBindGroupLayout(2).Get(), emptyBindGroupLayout.Get());  // Used.
    EXPECT_EQ(pipeline.GetBindGroupLayout(3).Get(), emptyBindGroupLayout.Get());  // Not used
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

    wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct S {
            [[offset(0)]] pos : vec4<f32>;
        };
        [[group(0), binding(0)]] var<uniform> uniforms : S;

        [[stage(vertex)]] fn main() -> void {
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, R"(
        [[stage(fragment)]] fn main() -> void {
        })");

    utils::ComboRenderPipelineDescriptor pipelineDesc(device);
    pipelineDesc.layout = pipelineLayout;
    pipelineDesc.vertexStage.module = vsModule;
    pipelineDesc.cFragmentStage.module = fsModule;

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
// TODO(dawn:216): Re-enable when we correctly reflect which bindings are used for an entryPoint.
TEST_F(GetBindGroupLayoutTests, DISABLED_FromCorrectEntryPoint) {
    wgpu::ShaderModule module = utils::CreateShaderModuleFromWGSL(device, R"(
        [[block]] struct Data {
            [[offset 0]] data : f32;
        };
        [[binding 0, set 0]] var<storage_buffer> data0 : Data;
        [[binding 1, set 0]] var<storage_buffer> data1 : Data;

        fn compute0() -> void {
            data0.data = 0.0;
            return;
        }
        fn compute1() -> void {
            data1.data = 0.0;
            return;
        }
        entry_point compute = compute0;
        entry_point compute = compute1;
    )");

    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.computeStage.module = module;

    // Get each entryPoint's BGL.
    pipelineDesc.computeStage.entryPoint = "compute0";
    wgpu::ComputePipeline pipeline0 = device.CreateComputePipeline(&pipelineDesc);
    wgpu::BindGroupLayout bgl0 = pipeline0.GetBindGroupLayout(0);

    pipelineDesc.computeStage.entryPoint = "compute1";
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
