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

#include <numeric>
#include <string>
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

class ShaderTests : public DawnTest {
  public:
    wgpu::Buffer CreateBuffer(const uint32_t count) {
        std::vector<uint32_t> data(count, 0);
        uint64_t bufferSize = static_cast<uint64_t>(data.size() * sizeof(uint32_t));
        return utils::CreateBufferFromData(device, data.data(), bufferSize,
                                           wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);
    }
    wgpu::ComputePipeline CreateComputePipeline(
        const std::string& shader,
        const char* entryPoint,
        const std::vector<wgpu::ConstantEntry>* constants = nullptr) {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = utils::CreateShaderModule(device, shader.c_str());
        csDesc.compute.entryPoint = entryPoint;
        if (constants) {
            csDesc.compute.constants = constants->data();
            csDesc.compute.constantCount = constants->size();
        }
        return device.CreateComputePipeline(&csDesc);
    }
};

// Test that log2 is being properly calculated, base on crbug.com/1046622
TEST_P(ShaderTests, ComputeLog2) {
    uint32_t const kSteps = 19;
    std::vector<uint32_t> expected{0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 32};
    wgpu::Buffer buffer = CreateBuffer(kSteps);

    std::string shader = R"(
struct Buf {
    data : array<u32, 19>
}

@group(0) @binding(0) var<storage, read_write> buf : Buf;

@stage(compute) @workgroup_size(1) fn main() {
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

    wgpu::ComputePipeline pipeline = CreateComputePipeline(shader, "main");

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();

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
@stage(vertex)
fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4<f32> {
    var pos = array<vec2<f32>, 3>(
        vec2<f32>(-1.0,  1.0),
        vec2<f32>( 1.0,  1.0),
        vec2<f32>( 0.0, -1.0));
    return vec4<f32>(pos[VertexIndex], 0.0, 1.0);
})";
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, vertexShader.c_str());

    std::string fragmentShader = R"(
@stage(fragment)
fn main(@builtin(position) fragCoord : vec4<f32>) -> @location(0) vec4<f32> {
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
    @location(0) position : vec3<f32>,
    @location(1) color : vec4<f32>,
}

struct VertexOut {
    @location(0) color : vec4<f32>,
    @builtin(position) position : vec4<f32>,
}

@stage(vertex)
fn main(input : VertexIn) -> VertexOut {
    var output : VertexOut;
    output.position = vec4<f32>(input.position, 1.0);
    output.color = input.color;
    return output;
})";
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, vertexShader.c_str());

    std::string fragmentShader = R"(
@stage(fragment)
fn main(@location(0) color : vec4<f32>) -> @location(0) vec4<f32> {
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
    @location(0) position : vec3<f32>,
    @location(1) color : vec4<f32>,
}

struct VertexOut {
    @location(0) color : vec4<f32>,
    @builtin(position) position : vec4<f32>,
}

@stage(vertex)
fn main(input : VertexIn) -> VertexOut {
    var output : VertexOut;
    output.position = vec4<f32>(input.position, 1.0);
    output.color = input.color;
    return output;
})";
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, vertexShader.c_str());

    std::string fragmentShader = R"(
struct FragmentIn {
    @location(0) color : vec4<f32>,
    @builtin(position) fragCoord : vec4<f32>,
}

@stage(fragment)
fn main(input : FragmentIn) -> @location(0) vec4<f32> {
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
    @location(0) position : vec3<f32>,
    @location(1) color : vec4<f32>,
}

struct VertexOut {
    @builtin(position) position : vec4<f32>,
    @location(0) color : vec4<f32>,
}

@stage(vertex)
fn main(input : VertexIn) -> VertexOut {
    var output : VertexOut;
    output.position = vec4<f32>(input.position, 1.0);
    output.color = input.color;
    return output;
})";
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, vertexShader.c_str());

    std::string fragmentShader = R"(
struct FragmentIn {
    @location(0) color : vec4<f32>,
    @builtin(position) fragCoord : vec4<f32>,
}

@stage(fragment)
fn main(input : FragmentIn) -> @location(0) vec4<f32> {
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
    std::string shader = R"(
struct VertexIn {
    @location(0) position : vec3<f32>,
    @location(1) color : vec4<f32>,
}

struct VertexOut {
    @location(0) color : vec4<f32>,
    @builtin(position) position : vec4<f32>,
}

@stage(vertex)
fn vertexMain(input : VertexIn) -> VertexOut {
    var output : VertexOut;
    output.position = vec4<f32>(input.position, 1.0);
    output.color = input.color;
    return output;
}

@stage(fragment)
fn fragmentMain(input : VertexOut) -> @location(0) vec4<f32> {
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

// This is a regression test for an issue caused by the FirstIndexOffset transfrom being done before
// the BindingRemapper, causing an intermediate AST to be invalid (and fail the overall
// compilation).
TEST_P(ShaderTests, FirstIndexOffsetRegisterConflictInHLSLTransforms) {
    // TODO(crbug.com/dawn/658): Crashes on bots because there are two entrypoints in the shader.
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() || IsOpenGLES());

    const char* shader = R"(
// Dumped WGSL:

struct Inputs {
  @location(1) attrib1 : u32,
  // The extra register added to handle base_vertex for vertex_index conflicts with [1]
  @builtin(vertex_index) vertexIndex: u32,
}

// [1] a binding point that conflicts with the regitster
struct S1 { data : array<vec4<u32>, 20> }
@group(0) @binding(1) var<uniform> providedData1 : S1;

@stage(vertex) fn vsMain(input : Inputs) -> @builtin(position) vec4<f32> {
  _ = providedData1.data[input.vertexIndex][0];
  return vec4<f32>();
}

@stage(fragment) fn fsMain() -> @location(0) vec4<f32> {
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

// Test that WGSL built-in variable @sample_index can be used in fragment shaders.
TEST_P(ShaderTests, SampleIndex) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
@stage(vertex)
fn main(@location(0) pos : vec4<f32>) -> @builtin(position) vec4<f32> {
    return pos;
})");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
@stage(fragment) fn main(@builtin(sample_index) sampleIndex : u32)
    -> @location(0) vec4<f32> {
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

// Test overridable constants without numeric identifiers
TEST_P(ShaderTests, OverridableConstants) {
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGL());
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGLES());

    uint32_t const kCount = 11;
    std::vector<uint32_t> expected(kCount);
    std::iota(expected.begin(), expected.end(), 0);
    wgpu::Buffer buffer = CreateBuffer(kCount);

    std::string shader = R"(
override c0: bool;              // type: bool
override c1: bool = false;      // default override
override c2: f32;               // type: float32
override c3: f32 = 0.0;         // default override
override c4: f32 = 4.0;         // default
override c5: i32;               // type: int32
override c6: i32 = 0;           // default override
override c7: i32 = 7;           // default
override c8: u32;               // type: uint32
override c9: u32 = 0u;          // default override
override c10: u32 = 10u;        // default

struct Buf {
    data : array<u32, 11>
}

@group(0) @binding(0) var<storage, read_write> buf : Buf;

@stage(compute) @workgroup_size(1) fn main() {
    buf.data[0] = u32(c0);
    buf.data[1] = u32(c1);
    buf.data[2] = u32(c2);
    buf.data[3] = u32(c3);
    buf.data[4] = u32(c4);
    buf.data[5] = u32(c5);
    buf.data[6] = u32(c6);
    buf.data[7] = u32(c7);
    buf.data[8] = u32(c8);
    buf.data[9] = u32(c9);
    buf.data[10] = u32(c10);
})";

    std::vector<wgpu::ConstantEntry> constants;
    constants.push_back({nullptr, "c0", 0});
    constants.push_back({nullptr, "c1", 1});
    constants.push_back({nullptr, "c2", 2});
    constants.push_back({nullptr, "c3", 3});
    // c4 is not assigned, testing default value
    constants.push_back({nullptr, "c5", 5});
    constants.push_back({nullptr, "c6", 6});
    // c7 is not assigned, testing default value
    constants.push_back({nullptr, "c8", 8});
    constants.push_back({nullptr, "c9", 9});
    // c10 is not assigned, testing default value

    wgpu::ComputePipeline pipeline = CreateComputePipeline(shader, "main", &constants);

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), buffer, 0, kCount);
}

// Test overridable constants with numeric identifiers
TEST_P(ShaderTests, OverridableConstantsNumericIdentifiers) {
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGL());
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGLES());

    uint32_t const kCount = 4;
    std::vector<uint32_t> expected{1u, 2u, 3u, 0u};
    wgpu::Buffer buffer = CreateBuffer(kCount);

    std::string shader = R"(
@id(1001) override c1: u32;            // some big numeric id
@id(1) override c2: u32 = 0u;          // id == 1 might collide with some generated constant id
@id(1003) override c3: u32 = 3u;       // default
@id(1004) override c4: u32;            // default unspecified

struct Buf {
    data : array<u32, 4>
}

@group(0) @binding(0) var<storage, read_write> buf : Buf;

@stage(compute) @workgroup_size(1) fn main() {
    buf.data[0] = c1;
    buf.data[1] = c2;
    buf.data[2] = c3;
    buf.data[3] = c4;
})";

    std::vector<wgpu::ConstantEntry> constants;
    constants.push_back({nullptr, "1001", 1});
    constants.push_back({nullptr, "1", 2});
    // c3 is not assigned, testing default value
    constants.push_back({nullptr, "1004", 0});

    wgpu::ComputePipeline pipeline = CreateComputePipeline(shader, "main", &constants);

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), buffer, 0, kCount);
}

// Test overridable constants precision
// D3D12 HLSL shader uses defines so we want float number to have enough precision
TEST_P(ShaderTests, OverridableConstantsPrecision) {
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGL());
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGLES());

    uint32_t const kCount = 2;
    float const kValue1 = 3.14159;
    float const kValue2 = 3.141592653589793238;
    std::vector<float> expected{kValue1, kValue2};
    wgpu::Buffer buffer = CreateBuffer(kCount);

    std::string shader = R"(
@id(1001) override c1: f32;
@id(1002) override c2: f32;

struct Buf {
    data : array<f32, 2>
}

@group(0) @binding(0) var<storage, read_write> buf : Buf;

@stage(compute) @workgroup_size(1) fn main() {
    buf.data[0] = c1;
    buf.data[1] = c2;
})";

    std::vector<wgpu::ConstantEntry> constants;
    constants.push_back({nullptr, "1001", kValue1});
    constants.push_back({nullptr, "1002", kValue2});
    wgpu::ComputePipeline pipeline = CreateComputePipeline(shader, "main", &constants);

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    EXPECT_BUFFER_FLOAT_RANGE_EQ(expected.data(), buffer, 0, kCount);
}

// Test overridable constants for different entry points
TEST_P(ShaderTests, OverridableConstantsMultipleEntryPoints) {
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGL());
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGLES());

    uint32_t const kCount = 1;
    std::vector<uint32_t> expected1{1u};
    std::vector<uint32_t> expected2{2u};
    std::vector<uint32_t> expected3{3u};

    wgpu::Buffer buffer1 = CreateBuffer(kCount);
    wgpu::Buffer buffer2 = CreateBuffer(kCount);
    wgpu::Buffer buffer3 = CreateBuffer(kCount);

    std::string shader = R"(
@id(1001) override c1: u32;
@id(1002) override c2: u32;

struct Buf {
    data : array<u32, 1>
}

@group(0) @binding(0) var<storage, read_write> buf : Buf;

@stage(compute) @workgroup_size(1) fn main1() {
    buf.data[0] = c1;
}

@stage(compute) @workgroup_size(1) fn main2() {
    buf.data[0] = c2;
}

@stage(compute) @workgroup_size(1) fn main3() {
    buf.data[0] = 3u;
}
)";

    std::vector<wgpu::ConstantEntry> constants1;
    constants1.push_back({nullptr, "1001", 1});
    std::vector<wgpu::ConstantEntry> constants2;
    constants2.push_back({nullptr, "1002", 2});

    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, shader.c_str());

    wgpu::ComputePipelineDescriptor csDesc1;
    csDesc1.compute.module = shaderModule;
    csDesc1.compute.entryPoint = "main1";
    csDesc1.compute.constants = constants1.data();
    csDesc1.compute.constantCount = constants1.size();
    wgpu::ComputePipeline pipeline1 = device.CreateComputePipeline(&csDesc1);

    wgpu::ComputePipelineDescriptor csDesc2;
    csDesc2.compute.module = shaderModule;
    csDesc2.compute.entryPoint = "main2";
    csDesc2.compute.constants = constants2.data();
    csDesc2.compute.constantCount = constants2.size();
    wgpu::ComputePipeline pipeline2 = device.CreateComputePipeline(&csDesc2);

    wgpu::ComputePipelineDescriptor csDesc3;
    csDesc3.compute.module = shaderModule;
    csDesc3.compute.entryPoint = "main3";
    wgpu::ComputePipeline pipeline3 = device.CreateComputePipeline(&csDesc3);

    wgpu::BindGroup bindGroup1 =
        utils::MakeBindGroup(device, pipeline1.GetBindGroupLayout(0), {{0, buffer1}});
    wgpu::BindGroup bindGroup2 =
        utils::MakeBindGroup(device, pipeline2.GetBindGroupLayout(0), {{0, buffer2}});
    wgpu::BindGroup bindGroup3 =
        utils::MakeBindGroup(device, pipeline3.GetBindGroupLayout(0), {{0, buffer3}});

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline1);
        pass.SetBindGroup(0, bindGroup1);
        pass.DispatchWorkgroups(1);

        pass.SetPipeline(pipeline2);
        pass.SetBindGroup(0, bindGroup2);
        pass.DispatchWorkgroups(1);

        pass.SetPipeline(pipeline3);
        pass.SetBindGroup(0, bindGroup3);
        pass.DispatchWorkgroups(1);

        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected1.data(), buffer1, 0, kCount);
    EXPECT_BUFFER_U32_RANGE_EQ(expected2.data(), buffer2, 0, kCount);
    EXPECT_BUFFER_U32_RANGE_EQ(expected3.data(), buffer3, 0, kCount);
}

// Test overridable constants with render pipeline
// Draw a triangle covering the render target, with vertex position and color values from
// overridable constants
TEST_P(ShaderTests, OverridableConstantsRenderPipeline) {
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGL());
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGLES());

    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
@id(1111) override xright: f32;
@id(2222) override ytop: f32;
@stage(vertex)
fn main(@builtin(vertex_index) VertexIndex : u32)
     -> @builtin(position) vec4<f32> {
  var pos = array<vec2<f32>, 3>(
      vec2<f32>(-1.0, ytop),
      vec2<f32>(-1.0, -ytop),
      vec2<f32>(xright, 0.0));

  return vec4<f32>(pos[VertexIndex], 0.0, 1.0);
})");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
@id(1000) override intensity: f32 = 0.0;
@stage(fragment) fn main()
    -> @location(0) vec4<f32> {
    return vec4<f32>(intensity, intensity, intensity, 1.0);
})");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    descriptor.cTargets[0].format = renderPass.colorFormat;

    std::vector<wgpu::ConstantEntry> vertexConstants;
    vertexConstants.push_back({nullptr, "1111", 3.0});  // x right
    vertexConstants.push_back({nullptr, "2222", 3.0});  // y top
    descriptor.vertex.constants = vertexConstants.data();
    descriptor.vertex.constantCount = vertexConstants.size();
    std::vector<wgpu::ConstantEntry> fragmentConstants;
    fragmentConstants.push_back({nullptr, "1000", 1.0});  // color intensity
    descriptor.cFragment.constants = fragmentConstants.data();
    descriptor.cFragment.constantCount = fragmentConstants.size();

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.Draw(3);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(RGBA8(255, 255, 255, 255), renderPass.color, 0, 0);
}

// TODO(tint:1155): Test overridable constants used for workgroup size

DAWN_INSTANTIATE_TEST(ShaderTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
