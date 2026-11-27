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

#include <algorithm>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "src/dawn/tests/DawnTest.h"
#include "src/dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

struct ShaderPoisonCase {
    const char* name;
    const char* expr;
    bool requiresF16 = false;
    bool isIndexExpr = false;

    bool operator==(const ShaderPoisonCase&) const = default;
};

std::ostream& operator<<(std::ostream& o, const ShaderPoisonCase& c) {
    return o << c.name;
}

DAWN_TEST_PARAM_STRUCT(ShaderPoisonParams, ShaderPoisonCase);

// These tests cover various cases where UB in the native shading language (e.g. Inf/NaN in a
// floating-point builtin) could cause a downstream compiler to make bad assumptions that in turn
// could be exploited to circumvent our security boundaries.
// See for example crbug.com/587021111 and crbug.com/587219560.
class ShaderPoisonTests : public DawnTestWithParams<ShaderPoisonParams> {
  protected:
    void SetUp() override {
        DawnTestWithParams<ShaderPoisonParams>::SetUp();

        // TODO(crbug.com/590145733): DXC fails to compile some cases.
        DAWN_SUPPRESS_TEST_IF((IsDXC() || IsWebGPUOn(wgpu::BackendType::D3D12)) &&
                              MatchesCase({"atanh_f32_inf_output", "pow_f32_inf_input"}));

        // TODO(crbug.com/590147040): Crashes on Android PowerVR OpenGLES for several cases.
        DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsImgTec() && IsOpenGLES() &&
                              MatchesCase({"cross_f32_nan_input", "determinant_2x2_nan_input",
                                           "fma_f32_inf_input", "mat2x2_mul_vec2_nan", "mod_inf"}));
    }

    bool MatchesCase(std::initializer_list<std::string_view> names) const {
        return std::find(names.begin(), names.end(), GetParam().mShaderPoisonCase.name) !=
               names.end();
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> features;
        if (SupportsFeatures({wgpu::FeatureName::ShaderF16})) {
            features.push_back(wgpu::FeatureName::ShaderF16);
        }
        return features;
    }

    wgpu::Buffer CreateBuffer(const std::vector<uint32_t>& data,
                              wgpu::BufferUsage usage = wgpu::BufferUsage::Storage |
                                                        wgpu::BufferUsage::CopySrc) {
        uint64_t bufferSize = static_cast<uint64_t>(data.size() * sizeof(uint32_t));
        return utils::CreateBufferFromData(device, data.data(), bufferSize, usage);
    }

    wgpu::Buffer CreateBuffer(const uint32_t count,
                              wgpu::BufferUsage usage = wgpu::BufferUsage::Storage |
                                                        wgpu::BufferUsage::CopySrc) {
        return CreateBuffer(std::vector<uint32_t>(count, 0), usage);
    }

    std::string BuildPoisonPreamble(const ShaderPoisonCase& testCase) {
        if (testCase.requiresF16) {
            return "enable f16;\n";
        }
        return "";
    }

    std::string BuildPoisonLocals(const ShaderPoisonCase& testCase) {
        std::string locals = R"(
  let zf = f32(U.x & 0u);
  let x = f32(U.x);
  let inf = 1.0f / zf;
  let nan = zf / zf;
)";
        if (testCase.requiresF16) {
            locals += R"(
  let zfh = f16(U.x & 0u);
  let xh = f16(U.x);
  let infh = 1.0h / zfh;
  let nanh = zfh / zfh;
)";
        }
        return locals;
    }

    std::string BuildPoisonIndexExpr(const ShaderPoisonCase& testCase) {
        if (testCase.isIndexExpr) {
            return testCase.expr;
        }
        return std::string("select(U.w, 0u, ") + testCase.expr + ")";
    }
};

// Check that poison does not directly cause a write to be optimized away.
// Failures for this test are not security issues on their own.
// Suppressions below do not necessarily have to be resolved, but instead indicate which downstream
// compilers are making optimizations that could potentially cause other problems in the future.
TEST_P(ShaderPoisonTests, PoisonedWrite) {
    const ShaderPoisonCase& testCase = GetParam().mShaderPoisonCase;
    DAWN_TEST_UNSUPPORTED_IF(testCase.requiresF16 &&
                             !device.HasFeature(wgpu::FeatureName::ShaderF16));

    // TODO(590225562): Fails on Android PowerVR Vulkan for several NaN-input cases.
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsImgTec() && IsVulkan() &&
                          MatchesCase({"mat2x2_mul_mat2x2_nan", "mat2x2_mul_scalar_nan"}));

    std::ostringstream shader;
    shader << BuildPoisonPreamble(testCase);
    shader << "@group(0) @binding(0) var<uniform> U : vec4<u32>;\n";
    shader << "@group(0) @binding(1) var<storage, read_write> output : array<u32, 16>;\n";
    shader << "@compute @workgroup_size(1) fn main() {\n";
    shader << "  let i = U.y;\n";
    shader << BuildPoisonLocals(testCase);
    shader << "  if (i > 15u) {\n";
    shader << "    output[" << BuildPoisonIndexExpr(testCase) << "] = 42u;\n";
    shader << "  }\n";
    shader << "}\n";

    wgpu::Buffer output = CreateBuffer(16);
    wgpu::Buffer bufU =
        CreateBuffer({5, 16, 0, 0}, wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, shader.str().c_str());
    csDesc.compute.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, bufU}, {1, output}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(42u, output, 0);
}

// Check that a write is not optimized away when it comes before poison that causes a dynamic error.
// Failures for this test are not necessarily security issues on their own.
// Suppressions below do not necessarily have to be resolved, but instead indicate which downstream
// compilers are making optimizations that could potentially cause other problems in the future.
// This is similar to PoisonedWrite, but fewer drivers fail this test so suppressions here are a
// potential indicator of more aggressive optimizations based on poisoned values.
TEST_P(ShaderPoisonTests, WriteBeforeDynamicError) {
    const ShaderPoisonCase& testCase = GetParam().mShaderPoisonCase;
    DAWN_TEST_UNSUPPORTED_IF(testCase.requiresF16 &&
                             !device.HasFeature(wgpu::FeatureName::ShaderF16));

    // TODO(590225562): Fails on Android PowerVR Vulkan for several NaN-input cases.
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsImgTec() && IsVulkan() &&
                          MatchesCase({"mat2x2_mul_mat2x2_nan", "mat2x2_mul_scalar_nan"}));

    std::ostringstream shader;
    shader << BuildPoisonPreamble(testCase);
    shader << "@group(0) @binding(0) var<uniform> U : vec4<u32>;\n";
    shader << "@group(0) @binding(1) var<storage, read_write> output : array<u32, 16>;\n";
    shader << "@group(0) @binding(2) var<storage, read_write> poison : array<u32, 16>;\n";
    shader << "@compute @workgroup_size(1) fn main() {\n";
    shader << "  let i = U.y;\n";
    shader << BuildPoisonLocals(testCase);
    shader << "  if (i > 15u) {\n";
    shader << "    output[0] = 42;\n";
    shader << "    poison[" << BuildPoisonIndexExpr(testCase) << "] = 42u;\n";
    shader << "  }\n";
    shader << "}\n";

    wgpu::Buffer output = CreateBuffer(16);
    wgpu::Buffer poison = CreateBuffer(16);
    wgpu::Buffer bufU =
        CreateBuffer({5, 16, 0, 0}, wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, shader.str().c_str());
    csDesc.compute.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {{0, bufU}, {1, output}, {2, poison}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(42u, output, 0);
}

TEST_P(ShaderPoisonTests, OutOfBoundsStorageBufferWrite) {
    const ShaderPoisonCase& testCase = GetParam().mShaderPoisonCase;
    DAWN_TEST_UNSUPPORTED_IF(testCase.requiresF16 &&
                             !device.HasFeature(wgpu::FeatureName::ShaderF16));

    constexpr uint32_t kWriteIndex = 16;
    constexpr uint32_t kWriteVal = 0xE7E7E7E7u;

    std::ostringstream shader;
    shader << BuildPoisonPreamble(testCase);
    shader << "@group(0) @binding(0) var<uniform> U : vec4<u32>;\n";
    shader << "@group(0) @binding(1) var<storage, read_write> output : array<u32, 16>;\n";
    shader << "@group(0) @binding(2) var<storage, read_write> poison : array<u32, 16>;\n";
    shader << "@compute @workgroup_size(1) fn main() {\n";
    shader << "  let i = U.y;\n";
    shader << BuildPoisonLocals(testCase);
    shader << "  if (i > 15u) {\n";
    shader << "    poison[" << BuildPoisonIndexExpr(testCase) << "] = 42u;\n";
    shader << "  }\n";
    shader << "  output[i] = U.z;\n";
    shader << "}\n";

    // Output buffer: 128 bytes (32 u32s), filled with zeros. Only the first 64 bytes (16 u32s) are
    // bound to the shader.
    wgpu::Buffer output = CreateBuffer(
        32, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc);
    wgpu::Buffer poison = CreateBuffer(16);

    wgpu::Buffer bufU = CreateBuffer({5, kWriteIndex, kWriteVal, 0},
                                     wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, shader.str().c_str());
    csDesc.compute.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                             {{0, bufU}, {1, output, 0, 16 * sizeof(uint32_t)}, {2, poison}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // With robustness enabled, the unbound portion of `output` must remain untouched.
    std::vector<uint32_t> expectedUnbound(16, 0);
    EXPECT_BUFFER_U32_RANGE_EQ(expectedUnbound.data(), output, 16 * sizeof(uint32_t), 16);
}

TEST_P(ShaderPoisonTests, OutOfBoundsWorkgroupMemoryRead) {
    const ShaderPoisonCase& testCase = GetParam().mShaderPoisonCase;
    DAWN_TEST_UNSUPPORTED_IF(testCase.requiresF16 &&
                             !device.HasFeature(wgpu::FeatureName::ShaderF16));

    // TODO(590225562): Fails on Android PowerVR Vulkan for several NaN-input cases.
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsImgTec() && IsVulkan() &&
                          MatchesCase({"mat2x2_mul_mat2x2_nan", "mat2x2_mul_scalar_nan"}));

    // These tests are exceptionally slow on Swiftshader, even with just a single workgroup.
    DAWN_TEST_UNSUPPORTED_IF(IsSwiftshader());

    constexpr uint32_t kReadIndex = 1000;
    constexpr uint32_t kNumWorkgroups = 1024;

    // Fill 16 KiB (the default maxComputeWorkgroupStorageSize) across all compute units.
    const char* fillShader = R"(
const kWorkgroupSize = 128u;

@group(0) @binding(0) var<storage, read_write> scratch : array<u32>;
var<workgroup> wg : array<u32, 4096>;
@compute @workgroup_size(kWorkgroupSize) fn main(@builtin(local_invocation_index) idx : u32,
                                      @builtin(workgroup_id) wg_id : vec3<u32>) {
  for (var k = idx; k < 4096u; k += kWorkgroupSize) {
    wg[k] = 0xDEADBEEFu;
  }
  workgroupBarrier();
  if (idx == 0u) {
    scratch[wg_id.x] = wg[0];
  }
}
)";

    std::ostringstream shader;
    shader << BuildPoisonPreamble(testCase);
    shader << "@group(0) @binding(0) var<uniform> U : vec4<u32>;\n";
    shader << "@group(0) @binding(1) var<storage, read_write> output : array<u32>;\n";
    shader << "@group(0) @binding(2) var<storage, read_write> poison : array<u32, 16>;\n";
    shader << "var<workgroup> wg : array<u32, 16>;\n";
    shader << "@compute @workgroup_size(1) fn main(@builtin(workgroup_id) wg_id : vec3<u32>) {\n";
    shader << "  let i = U.y;\n";
    shader << BuildPoisonLocals(testCase);
    shader << "  if (i > 15u) {\n";
    shader << "    poison[" << BuildPoisonIndexExpr(testCase) << "] = 42u;\n";
    shader << "  }\n";
    shader << "  output[wg_id.x] = wg[i];\n";
    shader << "}\n";

    wgpu::Buffer scratch = CreateBuffer(kNumWorkgroups);
    wgpu::Buffer output = CreateBuffer(kNumWorkgroups);
    wgpu::Buffer poison = CreateBuffer(16);
    wgpu::Buffer bufU = CreateBuffer({5, kReadIndex, 0, 0},
                                     wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst);

    wgpu::ComputePipelineDescriptor fillDesc;
    fillDesc.compute.module = utils::CreateShaderModule(device, fillShader);
    fillDesc.compute.entryPoint = "main";
    wgpu::ComputePipeline fillPipeline = device.CreateComputePipeline(&fillDesc);
    wgpu::BindGroup fillBindGroup =
        utils::MakeBindGroup(device, fillPipeline.GetBindGroupLayout(0), {{0, scratch}});

    wgpu::ComputePipelineDescriptor readDesc;
    readDesc.compute.module = utils::CreateShaderModule(device, shader.str().c_str());
    readDesc.compute.entryPoint = "main";
    wgpu::ComputePipeline readPipeline = device.CreateComputePipeline(&readDesc);
    wgpu::BindGroup readBindGroup = utils::MakeBindGroup(device, readPipeline.GetBindGroupLayout(0),
                                                         {{0, bufU}, {1, output}, {2, poison}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(fillPipeline);
        pass.SetBindGroup(0, fillBindGroup);
        pass.DispatchWorkgroups(kNumWorkgroups);
        pass.End();
    }
    {
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(readPipeline);
        pass.SetBindGroup(0, readBindGroup);
        pass.DispatchWorkgroups(kNumWorkgroups);
        pass.End();
    }
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint32_t> expected(kNumWorkgroups, 0u);
    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), output, 0, kNumWorkgroups);
}

TEST_P(ShaderPoisonTests, OutOfBoundsFunctionVarRead) {
    const ShaderPoisonCase& testCase = GetParam().mShaderPoisonCase;
    DAWN_TEST_UNSUPPORTED_IF(testCase.requiresF16 &&
                             !device.HasFeature(wgpu::FeatureName::ShaderF16));

    constexpr uint32_t kReadIndex = 1000;

    std::ostringstream shader;
    shader << BuildPoisonPreamble(testCase);
    shader << "@group(0) @binding(0) var<uniform> U : vec4<u32>;\n";
    shader << "@group(0) @binding(1) var<storage, read_write> output : array<u32, 16>;\n";
    shader << "@group(0) @binding(2) var<storage, read_write> poison : array<u32, 16>;\n";
    shader << "@compute @workgroup_size(64) fn main() {\n";
    shader << "  var local_arr : array<u32, 16>;\n";
    shader << "  let i = U.y;\n";
    shader << BuildPoisonLocals(testCase);
    shader << "  if (i > 15u) {\n";
    shader << "    poison[" << BuildPoisonIndexExpr(testCase) << "] = 42u;\n";
    shader << "  }\n";
    shader << "  if (local_arr[i] != 0u) {\n";
    shader << "    output[0] = 1u;\n";
    shader << "  }\n";
    shader << "}\n";

    wgpu::Buffer output = CreateBuffer(16);
    wgpu::Buffer poison = CreateBuffer(16);
    wgpu::Buffer bufU = CreateBuffer({5, kReadIndex, 0, 0},
                                     wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, shader.str().c_str());
    csDesc.compute.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {{0, bufU}, {1, output}, {2, poison}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(64);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(0u, output, 0);
}

DAWN_INSTANTIATE_TEST_P(
    ShaderPoisonTests,
    {
        D3D11Backend(),
        D3D12Backend(),
        D3D12Backend({}, {"use_dxc"}),
        MetalBackend(),
        OpenGLBackend(),
        OpenGLESBackend(),
        VulkanBackend(),
        VulkanBackend({"spirv_float_controls_32bit", "spirv_float_controls_16bit"},
                      {"spirv_float_controls2"}),
        WebGPUBackend(),
    },
    {
        // Floating point scalar arithmetic.
        ShaderPoisonCase{"add_inf_neg_inf", "(inf + (-inf)) == 123.0f"},
        ShaderPoisonCase{"div_zero", "(1.0f / zf) == 123.0f"},
        ShaderPoisonCase{"div_zero_over_zero", "(zf / zf) == 123.0f"},
        ShaderPoisonCase{"div_inf_by_inf", "(inf / inf) == 123.0f"},
        ShaderPoisonCase{"mod_zero", "(x % zf) == 123.0f"},
        ShaderPoisonCase{"mod_inf", "(inf % x) == 123.0f"},
        ShaderPoisonCase{"mul_overflow", "(3.0e38f * (2.0f + zf)) == 123.0f"},
        ShaderPoisonCase{"mul_inf_zero", "(inf * zf) == 123.0f"},
        ShaderPoisonCase{"sub_inf_inf", "(inf - inf) == 123.0f"},

        // Floating point matrix/vector arithmetic.
        ShaderPoisonCase{"mat2x2_add_mat2x2",
                         "(mat2x2<f32>(inf, 1.0f, 2.0f, 3.0f) + mat2x2<f32>(x, 1.0f, 1.0f, "
                         "1.0f))[0].x == 123.0f"},
        ShaderPoisonCase{"mat2x2_mul_mat2x2",
                         "(mat2x2<f32>(inf, x, x, x) * mat2x2<f32>(x, x, x, x))[0].x == 123.0f"},
        ShaderPoisonCase{"mat2x2_mul_mat2x2_nan",
                         "(mat2x2<f32>(nan, x, x, x) * mat2x2<f32>(x, x, x, x))[0].x == 123.0f"},
        ShaderPoisonCase{"mat2x2_mul_mat2x2_nan_2",
                         "(mat2x2<f32>(x, x, x, nan) * mat2x2<f32>(x, x, x, x))[1].y == 123.0f"},
        ShaderPoisonCase{"mat4x4_mul_mat4x4_f16_nan",
                         "(mat4x4<f16>(nanh, xh, xh, xh, xh, xh, xh, xh, xh, xh, xh, xh, xh, "
                         "xh, xh, xh) * mat4x4<f16>(xh, xh, xh, xh, xh, xh, xh, xh, xh, xh, xh, "
                         "xh, xh, xh, xh, xh))[0].x == 123.0h",
                         true},
        ShaderPoisonCase{"mat2x2_mul_scalar",
                         "(mat2x2<f32>(inf, 1.0f, 2.0f, 3.0f) * x)[0].x == 123.0f"},
        ShaderPoisonCase{"mat2x2_mul_scalar_nan",
                         "(mat2x2<f32>(nan, 1.0f, 2.0f, 3.0f) * x)[0].x == 123.0f"},
        ShaderPoisonCase{"mat2x2_mul_scalar_nan_2",
                         "(mat2x2<f32>(1.0f, 1.0f, 2.0f, nan) * x)[1].y == 123.0f"},
        ShaderPoisonCase{"mat2x2_mul_scalar_nan_3",
                         "(mat2x2<f32>(1.0f, nan, 2.0f, 3.0f) * x)[0].y == 123.0f"},
        ShaderPoisonCase{"mat2x2_mul_scalar_nan_4",
                         "(mat2x2<f32>(1.0f, 1.0f, nan, 3.0f) * x)[1].x == 123.0f"},
        ShaderPoisonCase{"scalar_mul_mat2x2",
                         "(x * mat2x2<f32>(inf, 1.0f, 2.0f, 3.0f))[0].x == 123.0f"},
        ShaderPoisonCase{"mat2x2_mul_vec2",
                         "(mat2x2<f32>(inf, x, x, x) * vec2<f32>(x, x)).x == 123.0f"},
        ShaderPoisonCase{"mat2x2_mul_vec2_nan",
                         "(mat2x2<f32>(nan, x, x, x) * vec2<f32>(x, x)).x == 123.0f"},
        ShaderPoisonCase{
            "mat3x3_mul_vec3",
            "(mat3x3<f32>(inf, x, x, x, x, x, x, x, x) * vec3<f32>(x, x, x)).x == 123.0f"},
        ShaderPoisonCase{"mat4x4_mul_vec4",
                         "(mat4x4<f32>(inf, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x) * "
                         "vec4<f32>(x, x, x, x)).x == 123.0f"},
        ShaderPoisonCase{"mat2x2_sub_mat2x2",
                         "(mat2x2<f32>(inf, 1.0f, 2.0f, 3.0f) - mat2x2<f32>(x, 1.0f, 1.0f, "
                         "1.0f))[0].x == 123.0f"},
        ShaderPoisonCase{"mat2x2_sub_mat2x2_nan",
                         "(mat2x2<f32>(nan, 1.0f, 2.0f, 3.0f) - mat2x2<f32>(x, 1.0f, 1.0f, "
                         "1.0f))[0].x == 123.0f"},
        ShaderPoisonCase{"mat4x3_sub_mat4x3",
                         "(mat4x3<f32>(inf, x, x, x, x, x, x, x, x, x, x, x) - "
                         "mat4x3<f32>(x, x, x, x, x, x, x, x, x, x, x, x))[0].x == 123.0f"},
        ShaderPoisonCase{"vec_mul_scalar", "(vec2<f32>(inf, x) * x).x == 123.0f"},

        // Floating point builtins functions.
        ShaderPoisonCase{"acosh_f32_nan_output", "acosh(0.5f + zf) == 123.0f"},
        ShaderPoisonCase{"asin_f32_nan_output", "asin(2.0f + zf) == 123.0f"},
        ShaderPoisonCase{"atan2_f32_inf_input", "atan2(inf, x) == 123.0f"},
        ShaderPoisonCase{"atanh_f32_inf_output", "atanh(1.0f + zf) == 123.0f"},
        ShaderPoisonCase{"atanh_f32_nan_output", "atanh(2.0f + zf) == 123.0f"},
        ShaderPoisonCase{"clamp_f32_inf_input", "clamp(inf, 0.0f, x) == 123.0f"},
        ShaderPoisonCase{"cross_f32_inf_input",
                         "cross(vec3<f32>(x, inf, x), vec3<f32>(x, x, x)).x == 123.0f"},
        ShaderPoisonCase{"cross_f32_inf_output",
                         "cross(vec3<f32>(3.0e38f + zf, 1.0f, 1.0f), vec3<f32>(1.0f, 3.0f, "
                         "1.0f)).z == 123.0f"},
        ShaderPoisonCase{"cross_f32_nan_input",
                         "cross(vec3<f32>(x, nan, x), vec3<f32>(x, x, x)).x == 123.0f"},
        ShaderPoisonCase{"determinant_2x2_inf_input",
                         "determinant(mat2x2<f32>(inf, x, x, x)) == 123.0f"},
        ShaderPoisonCase{"determinant_2x2_inf_output",
                         "determinant(mat2x2<f32>(3.0e38f + zf, 1.0f, 1.0f, 3.0f)) == 123.0f"},
        ShaderPoisonCase{"determinant_2x2_nan_input",
                         "determinant(mat2x2<f32>(nan, x, x, x)) == 123.0f"},
        ShaderPoisonCase{"determinant_3x3_inf_input",
                         "determinant(mat3x3<f32>(inf, x, x, x, x, x, x, x, x)) == 123.0f"},
        ShaderPoisonCase{
            "determinant_4x4_inf_input",
            "determinant(mat4x4<f32>(inf, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x)) == 123.0f"},
        ShaderPoisonCase{"distance_vec2_inf_input",
                         "distance(vec2<f32>(inf, x), vec2<f32>(x, x)) == 123.0f"},
        ShaderPoisonCase{
            "distance_vec2_inf_output",
            "distance(vec2<f32>(3.0e38f + zf, 0.0f), vec2<f32>(-3.0e38f, 0.0f)) == 123.0f"},
        ShaderPoisonCase{"dot_f32_vec2_inf_input",
                         "dot(vec2<f32>(inf, x), vec2<f32>(x, x)) == 123.0f"},
        ShaderPoisonCase{"dot_f32_vec2_inf_output",
                         "dot(vec2<f32>(3.0e38f + zf, x), vec2<f32>(3.0f, x)) == 123.0f"},
        ShaderPoisonCase{"dot_f32_vec2_nan_input",
                         "dot(vec2<f32>(nan, x), vec2<f32>(x, x)) == 123.0f"},
        ShaderPoisonCase{
            "dot_f32_vec2_nan_output",
            "dot(vec2<f32>(3.0e38f + zf, 3.0e38f + zf), vec2<f32>(3.0f, -3.0f)) == 123.0f"},
        ShaderPoisonCase{"dot_f32_vec3_inf_input",
                         "dot(vec3<f32>(inf, x, x), vec3<f32>(x, x, x)) == 123.0f"},
        ShaderPoisonCase{"dot_f32_vec4_inf_input",
                         "dot(vec4<f32>(inf, x, x, x), vec4<f32>(x, x, x, x)) == 123.0f"},
        ShaderPoisonCase{"dot_f16_vec2_inf_input",
                         "dot(vec2<f16>(infh, xh), vec2<f16>(xh, xh)) == 123.0h", true},
        ShaderPoisonCase{"dot_f16_vec2_nan_input",
                         "dot(vec2<f16>(nanh, xh), vec2<f16>(xh, xh)) == 123.0h", true},
        ShaderPoisonCase{"dot_f16_vec4_inf_input",
                         "dot(vec4<f16>(infh, xh, xh, xh), vec4<f16>(xh, xh, xh, xh)) == 123.0h",
                         true},
        ShaderPoisonCase{"exp_f32_inf_output", "exp(1000.0f + zf) == 123.0f"},
        ShaderPoisonCase{"exp2_f32_inf_output", "exp2(1000.0f + zf) == 123.0f"},
        ShaderPoisonCase{
            "faceForward_vec2_inf_input",
            "faceForward(vec2<f32>(inf, x), vec2<f32>(x, x), vec2<f32>(x, x)).x == 123.0f"},
        ShaderPoisonCase{"faceForward_vec2_inf_output",
                         "faceForward(vec2<f32>(3.0e38f + zf, 0.0f), vec2<f32>(3.0e38f + zf, "
                         "0.0f), vec2<f32>(3.0f, 0.0f)).x == 123.0f"},
        ShaderPoisonCase{"fma_f32_inf_input", "fma(inf, x, x) == 123.0f"},
        ShaderPoisonCase{"fma_f32_inf_output", "fma(3.0e38f + zf, 2.0f, 1.0f) == 123.0f"},
        ShaderPoisonCase{"inverseSqrt_f32_inf_output", "inverseSqrt(zf) == 123.0f"},
        ShaderPoisonCase{"inverseSqrt_f32_nan_output", "inverseSqrt(-1.0f + zf) == 123.0f"},
        ShaderPoisonCase{"ldexp_f32_inf_output", "ldexp(1.0f + zf, 200 + i32(U.x & 0u)) == 123.0f"},
        ShaderPoisonCase{"length_vec2_inf_input", "length(vec2<f32>(inf, x)) == 123.0f"},
        ShaderPoisonCase{"length_vec2_inf_output",
                         "length(vec2<f32>(3.0e38f + zf, 3.0e38f)) == 123.0f"},
        ShaderPoisonCase{"log_f32_inf_output", "log(zf) == 123.0f"},
        ShaderPoisonCase{"log_f32_nan_output", "log(-1.0f + zf) == 123.0f"},
        ShaderPoisonCase{"log2_f32_inf_output", "log2(zf) == 123.0f"},
        ShaderPoisonCase{"log2_f32_nan_output", "log2(-1.0f + zf) == 123.0f"},
        ShaderPoisonCase{"max_f32_inf_input", "max(inf, x) == 123.0f"},
        ShaderPoisonCase{"min_f32_inf_input", "min(inf, x) == 123.0f"},
        ShaderPoisonCase{"mix_f32_inf_input", "mix(inf, x, x) == 123.0f"},
        ShaderPoisonCase{"mix_f32_nan_input", "mix(nan, x, x) == 123.0f"},
        ShaderPoisonCase{"normalize_vec2_inf_input", "normalize(vec2<f32>(inf, x)).x == 123.0f"},
        ShaderPoisonCase{"normalize_vec2_nan_input", "normalize(vec2<f32>(nan, x)).x == 123.0f"},
        ShaderPoisonCase{"normalize_vec2_nan_output", "normalize(vec2<f32>(zf, zf)).x == 123.0f"},
        ShaderPoisonCase{"pow_f32_inf_input", "pow(inf, x) == 123.0f"},
        ShaderPoisonCase{"pow_f32_inf_output", "pow(2.0f + zf, 200.0f) == 123.0f"},
        ShaderPoisonCase{"pow_f32_nan_input", "pow(nan, x) == 123.0f"},
        ShaderPoisonCase{"pow_f32_nan_output", "pow(-2.0f + zf, 0.5f) == 123.0f"},
        ShaderPoisonCase{"reflect_vec2_inf_input",
                         "reflect(vec2<f32>(inf, x), vec2<f32>(x, x)).x == 123.0f"},
        ShaderPoisonCase{"reflect_vec2_inf_output",
                         "reflect(vec2<f32>(3.0e38f + zf, x), vec2<f32>(3.0f, x)).x == 123.0f"},
        ShaderPoisonCase{"reflect_vec2_nan_input",
                         "reflect(vec2<f32>(nan, x), vec2<f32>(x, x)).x == 123.0f"},
        ShaderPoisonCase{"refract_vec2_f32_inf_input",
                         "refract(vec2<f32>(inf, x), vec2<f32>(x, x), x).x == 123.0f"},
        ShaderPoisonCase{"refract_vec2_f32_inf_output",
                         "refract(vec2<f32>(3.0e38f + zf, x), vec2<f32>(3.0f, x), x).x == 123.0f"},
        ShaderPoisonCase{"refract_vec2_f32_nan_input",
                         "refract(vec2<f32>(nan, x), vec2<f32>(x, x), x).x == 123.0f"},
        ShaderPoisonCase{"refract_vec2_f16_inf_input",
                         "refract(vec2<f16>(infh, xh), vec2<f16>(xh, xh), xh).x == 123.0h", true},
        ShaderPoisonCase{"sin_f32_inf_input", "sin(inf) == 123.0f"},
        ShaderPoisonCase{"smoothstep_f32_inf_input", "smoothstep(0.0f, inf, x) == 123.0f"},
        ShaderPoisonCase{"sqrt_f32_nan_output", "sqrt(-1.0f - zf) == 123.0f"},
        ShaderPoisonCase{"step_f32_inf_input", "step(inf, x) == 123.0f"},

        // Misc other cases.
        // Apple compiler bug https://crbug.com/587021111
        ShaderPoisonCase{"extract_bits_u32", "extractBits(U.x, 32u, 0u)", /*requiresF16=*/false,
                         /*isIndexExpr=*/true},
    });

}  // anonymous namespace
}  // namespace dawn
