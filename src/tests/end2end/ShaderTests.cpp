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

#include "tests/DawnTest.h"

#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

#include <vector>

class ShaderTests : public DawnTest {};

// Test that log2 is being properly calculated, base on crbug.com/1046622
TEST_P(ShaderTests, ComputeLog2) {
    uint32_t const kSteps = 19;
    std::vector<uint32_t> data(kSteps, 0);
    std::vector<uint32_t> expected{0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 32};
    uint64_t bufferSize = static_cast<uint64_t>(data.size() * sizeof(uint32_t));
    wgpu::Buffer buffer = utils::CreateBufferFromData(
        device, data.data(), bufferSize, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);

    std::string shader = R"(
[[block]] struct Buf {
    data : array<u32, 19>;
};

[[group(0), binding(0)]] var<storage, read_write> buf : Buf;

[[stage(compute), workgroup_size(1)]] fn main() {
    let factor : f32 = 1.0001;

    buf.data[0] = u32(log2(1.0 * factor));
    buf.data[1] = u32(log2(2.0 * factor));
    buf.data[2] = u32(log2(3.0 * factor));
    buf.data[3] = u32(log2(4.0 * factor));
    buf.data[4] = u32(log2(7.0 * factor));
    buf.data[5] = u32(log2(8.0 * factor));
    buf.data[6] = u32(log2(15.0 * factor));
    buf.data[7] = u32(log2(16.0 * factor));
    buf.data[8] = u32(log2(31.0 * factor));
    buf.data[9] = u32(log2(32.0 * factor));
    buf.data[10] = u32(log2(63.0 * factor));
    buf.data[11] = u32(log2(64.0 * factor));
    buf.data[12] = u32(log2(127.0 * factor));
    buf.data[13] = u32(log2(128.0 * factor));
    buf.data[14] = u32(log2(255.0 * factor));
    buf.data[15] = u32(log2(256.0 * factor));
    buf.data[16] = u32(log2(511.0 * factor));
    buf.data[17] = u32(log2(512.0 * factor));
    buf.data[18] = u32(log2(4294967295.0 * factor));
})";

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, shader.c_str());
    csDesc.compute.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Dispatch(1);
        pass.EndPass();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), buffer, 0, kSteps);
}

TEST_P(ShaderTests, BadWGSL) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    std::string shader = R"(
I am an invalid shader and should never pass validation!
})";
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, shader.c_str()));
}

// Tests that shaders using non-struct function parameters and return values for shader stage I/O
// can compile and link successfully.
TEST_P(ShaderTests, WGSLParamIO) {
    std::string vertexShader = R"(
[[stage(vertex)]]
fn main([[builtin(vertex_index)]] VertexIndex : u32) -> [[builtin(position)]] vec4<f32> {
    var pos = array<vec2<f32>, 3>(
        vec2<f32>(-1.0,  1.0),
        vec2<f32>( 1.0,  1.0),
        vec2<f32>( 0.0, -1.0));
    return vec4<f32>(pos[VertexIndex], 0.0, 1.0);
})";
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, vertexShader.c_str());

    std::string fragmentShader = R"(
[[stage(fragment)]]
fn main([[builtin(position)]] fragCoord : vec4<f32>) -> [[location(0)]] vec4<f32> {
    return vec4<f32>(fragCoord.xy, 0.0, 1.0);
})";
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, fragmentShader.c_str());

    utils::ComboRenderPipelineDescriptor rpDesc;
    rpDesc.vertex.module = vsModule;
    rpDesc.cFragment.module = fsModule;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);
}

// Tests that a vertex shader using struct function parameters and return values for shader stage
// I/O can compile and link successfully against a fragement shader using compatible non-struct I/O.
TEST_P(ShaderTests, WGSLMixedStructParamIO) {
    std::string vertexShader = R"(
struct VertexIn {
    [[location(0)]] position : vec3<f32>;
    [[location(1)]] color : vec4<f32>;
};

struct VertexOut {
    [[location(0)]] color : vec4<f32>;
    [[builtin(position)]] position : vec4<f32>;
};

[[stage(vertex)]]
fn main(input : VertexIn) -> VertexOut {
    var output : VertexOut;
    output.position = vec4<f32>(input.position, 1.0);
    output.color = input.color;
    return output;
})";
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, vertexShader.c_str());

    std::string fragmentShader = R"(
[[stage(fragment)]]
fn main([[location(0)]] color : vec4<f32>) -> [[location(0)]] vec4<f32> {
    return color;
})";
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, fragmentShader.c_str());

    utils::ComboRenderPipelineDescriptor rpDesc;
    rpDesc.vertex.module = vsModule;
    rpDesc.cFragment.module = fsModule;
    rpDesc.vertex.bufferCount = 1;
    rpDesc.cBuffers[0].attributeCount = 2;
    rpDesc.cBuffers[0].arrayStride = 28;
    rpDesc.cAttributes[0].shaderLocation = 0;
    rpDesc.cAttributes[0].format = wgpu::VertexFormat::Float32x3;
    rpDesc.cAttributes[1].shaderLocation = 1;
    rpDesc.cAttributes[1].format = wgpu::VertexFormat::Float32x4;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);
}

// Tests that shaders using struct function parameters and return values for shader stage I/O
// can compile and link successfully.
TEST_P(ShaderTests, WGSLStructIO) {
    std::string vertexShader = R"(
struct VertexIn {
    [[location(0)]] position : vec3<f32>;
    [[location(1)]] color : vec4<f32>;
};

struct VertexOut {
    [[location(0)]] color : vec4<f32>;
    [[builtin(position)]] position : vec4<f32>;
};

[[stage(vertex)]]
fn main(input : VertexIn) -> VertexOut {
    var output : VertexOut;
    output.position = vec4<f32>(input.position, 1.0);
    output.color = input.color;
    return output;
})";
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, vertexShader.c_str());

    std::string fragmentShader = R"(
struct FragmentIn {
    [[location(0)]] color : vec4<f32>;
    [[builtin(position)]] fragCoord : vec4<f32>;
};

[[stage(fragment)]]
fn main(input : FragmentIn) -> [[location(0)]] vec4<f32> {
    return input.color * input.fragCoord;
})";
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, fragmentShader.c_str());

    utils::ComboRenderPipelineDescriptor rpDesc;
    rpDesc.vertex.module = vsModule;
    rpDesc.cFragment.module = fsModule;
    rpDesc.vertex.bufferCount = 1;
    rpDesc.cBuffers[0].attributeCount = 2;
    rpDesc.cBuffers[0].arrayStride = 28;
    rpDesc.cAttributes[0].shaderLocation = 0;
    rpDesc.cAttributes[0].format = wgpu::VertexFormat::Float32x3;
    rpDesc.cAttributes[1].shaderLocation = 1;
    rpDesc.cAttributes[1].format = wgpu::VertexFormat::Float32x4;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);
}

// Tests that shaders I/O structs that us compatible locations but are not sorted by hand can link.
TEST_P(ShaderTests, WGSLUnsortedStructIO) {
    std::string vertexShader = R"(
struct VertexIn {
    [[location(0)]] position : vec3<f32>;
    [[location(1)]] color : vec4<f32>;
};

struct VertexOut {
    [[builtin(position)]] position : vec4<f32>;
    [[location(0)]] color : vec4<f32>;
};

[[stage(vertex)]]
fn main(input : VertexIn) -> VertexOut {
    var output : VertexOut;
    output.position = vec4<f32>(input.position, 1.0);
    output.color = input.color;
    return output;
})";
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, vertexShader.c_str());

    std::string fragmentShader = R"(
struct FragmentIn {
    [[location(0)]] color : vec4<f32>;
    [[builtin(position)]] fragCoord : vec4<f32>;
};

[[stage(fragment)]]
fn main(input : FragmentIn) -> [[location(0)]] vec4<f32> {
    return input.color * input.fragCoord;
})";
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, fragmentShader.c_str());

    utils::ComboRenderPipelineDescriptor rpDesc;
    rpDesc.vertex.module = vsModule;
    rpDesc.cFragment.module = fsModule;
    rpDesc.vertex.bufferCount = 1;
    rpDesc.cBuffers[0].attributeCount = 2;
    rpDesc.cBuffers[0].arrayStride = 28;
    rpDesc.cAttributes[0].shaderLocation = 0;
    rpDesc.cAttributes[0].format = wgpu::VertexFormat::Float32x3;
    rpDesc.cAttributes[1].shaderLocation = 1;
    rpDesc.cAttributes[1].format = wgpu::VertexFormat::Float32x4;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);
}

// Tests that shaders I/O structs can be shared between vertex and fragment shaders.
TEST_P(ShaderTests, WGSLSharedStructIO) {
    // TODO(tint:714): Not yet implemeneted in tint yet, but intended to work.
    DAWN_SUPPRESS_TEST_IF(IsD3D12() || IsVulkan() || IsMetal() || IsOpenGL() || IsOpenGLES());

    std::string shader = R"(
struct VertexIn {
    [[location(0)]] position : vec3<f32>;
    [[location(1)]] color : vec4<f32>;
};

struct VertexOut {
    [[location(0)]] color : vec4<f32>;
    [[builtin(position)]] position : vec4<f32>;
};

[[stage(vertex)]]
fn vertexMain(input : VertexIn) -> VertexOut {
    var output : VertexOut;
    output.position = vec4<f32>(input.position, 1.0);
    output.color = input.color;
    return output;
}

[[stage(fragment)]]
fn fragmentMain(input : VertexOut) -> [[location(0)]] vec4<f32> {
    return input.color;
})";
    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, shader.c_str());

    utils::ComboRenderPipelineDescriptor rpDesc;
    rpDesc.vertex.module = shaderModule;
    rpDesc.vertex.entryPoint = "vertexMain";
    rpDesc.cFragment.module = shaderModule;
    rpDesc.cFragment.entryPoint = "fragmentMain";
    rpDesc.vertex.bufferCount = 1;
    rpDesc.cBuffers[0].attributeCount = 2;
    rpDesc.cBuffers[0].arrayStride = 28;
    rpDesc.cAttributes[0].shaderLocation = 0;
    rpDesc.cAttributes[0].format = wgpu::VertexFormat::Float32x3;
    rpDesc.cAttributes[1].shaderLocation = 1;
    rpDesc.cAttributes[1].format = wgpu::VertexFormat::Float32x4;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&rpDesc);
}

// Feature currently not implemented in Tint, so should fail validation.
TEST_P(ShaderTests, PipelineOverridableUsed) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));
    DAWN_TEST_UNSUPPORTED_IF(!HasToggleEnabled("use_tint_generator"));

    std::string shader = R"(
[[override]] let foo : f32;

[[stage(compute), workgroup_size(1)]]
fn ep_func() {
  var local_foo : f32;
  local_foo = foo;
  return;
})";
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, shader.c_str()));
}

// This is a regression test for an issue caused by the FirstIndexOffset transfrom being done before
// the BindingRemapper, causing an intermediate AST to be invalid (and fail the overall
// compilation).
TEST_P(ShaderTests, FirstIndexOffsetRegisterConflictInHLSLTransforms) {
    // TODO(crbug.com/dawn/658): Crashes on bots because there are two entrypoints in the shader.
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() || IsOpenGLES());

    const char* shader = R"(
// Dumped WGSL:

struct Inputs {
  [[location(1)]] attrib1 : u32;
  // The extra register added to handle base_vertex for vertex_index conflicts with [1]
  [[builtin(vertex_index)]] vertexIndex: u32;
};

// [1] a binding point that conflicts with the regitster
[[block]] struct S1 { data : array<vec4<u32>, 20>; };
[[group(0), binding(1)]] var<uniform> providedData1 : S1;

[[stage(vertex)]] fn vsMain(input : Inputs) -> [[builtin(position)]] vec4<f32> {
  ignore(providedData1.data[input.vertexIndex][0]);
  return vec4<f32>();
}

[[stage(fragment)]] fn fsMain() -> [[location(0)]] vec4<f32> {
  return vec4<f32>();
}
    )";
    auto module = utils::CreateShaderModule(device, shader);

    utils::ComboRenderPipelineDescriptor rpDesc;
    rpDesc.vertex.module = module;
    rpDesc.vertex.entryPoint = "vsMain";
    rpDesc.cFragment.module = module;
    rpDesc.cFragment.entryPoint = "fsMain";
    rpDesc.vertex.bufferCount = 1;
    rpDesc.cBuffers[0].attributeCount = 1;
    rpDesc.cBuffers[0].arrayStride = 16;
    rpDesc.cAttributes[0].shaderLocation = 1;
    rpDesc.cAttributes[0].format = wgpu::VertexFormat::Uint8x2;
    device.CreateRenderPipeline(&rpDesc);
}

// Test that WGSL built-in variable [[sample_index]] can be used in fragment shaders.
TEST_P(ShaderTests, SampleIndex) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
[[stage(vertex)]]
fn main([[location(0)]] pos : vec4<f32>) -> [[builtin(position)]] vec4<f32> {
    return pos;
})");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
[[stage(fragment)]] fn main([[builtin(sample_index)]] sampleIndex : u32)
    -> [[location(0)]] vec4<f32> {
    return vec4<f32>(f32(sampleIndex), 1.0, 0.0, 1.0);
})");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    descriptor.vertex.bufferCount = 1;
    descriptor.cBuffers[0].arrayStride = 4 * sizeof(float);
    descriptor.cBuffers[0].attributeCount = 1;
    descriptor.cAttributes[0].format = wgpu::VertexFormat::Float32x4;
    descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

    device.CreateRenderPipeline(&descriptor);
}

DAWN_INSTANTIATE_TEST(ShaderTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
