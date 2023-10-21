// Copyright 2021 The Dawn & Tint Authors
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
#include <limits>
#include <string>
#include <vector>

#include "dawn/common/Math.h"
#include "dawn/common/Platform.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class MaxLimitTests : public DawnTest {
  public:
    wgpu::RequiredLimits GetRequiredLimits(const wgpu::SupportedLimits& supported) override {
        wgpu::RequiredLimits required = {};
        required.limits = supported.limits;
        return required;
    }
};

// Test using the maximum amount of workgroup memory works
TEST_P(MaxLimitTests, MaxComputeWorkgroupStorageSize) {
    uint32_t maxComputeWorkgroupStorageSize =
        GetSupportedLimits().limits.maxComputeWorkgroupStorageSize;

    std::string shader = R"(
        struct Dst {
            value0 : u32,
            value1 : u32,
        }

        @group(0) @binding(0) var<storage, read_write> dst : Dst;

        struct WGData {
          value0 : u32,
          // padding such that value0 and value1 are the first and last bytes of the memory.
          @size()" + std::to_string(maxComputeWorkgroupStorageSize / 4 - 2) +
                         R"() padding : u32,
          value1 : u32,
        }
        var<workgroup> wg_data : WGData;

        @compute @workgroup_size(2,1,1)
        fn main(@builtin(local_invocation_index) LocalInvocationIndex : u32) {
            if (LocalInvocationIndex == 0u) {
                // Put data into the first and last byte of workgroup memory.
                wg_data.value0 = 79u;
                wg_data.value1 = 42u;
            }

            workgroupBarrier();

            if (LocalInvocationIndex == 1u) {
                // Read data out of workgroup memory into a storage buffer.
                dst.value0 = wg_data.value0;
                dst.value1 = wg_data.value1;
            }
        }
    )";
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, shader.c_str());
    csDesc.compute.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Set up dst storage buffer
    wgpu::BufferDescriptor dstDesc;
    dstDesc.size = 8;
    dstDesc.usage =
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer dst = device.CreateBuffer(&dstDesc);

    // Set up bind group and issue dispatch
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, dst},
                                                     });

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(79, dst, 0);
    EXPECT_BUFFER_U32_EQ(42, dst, 4);
}

// Test using the maximum uniform/storage buffer binding size works
TEST_P(MaxLimitTests, MaxBufferBindingSize) {
    // The uniform buffer layout used in this test is not supported on ES.
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGLES());

    // TODO(crbug.com/dawn/1217): Remove this suppression.
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsVulkan() && IsNvidia());
    DAWN_SUPPRESS_TEST_IF(IsLinux() && IsVulkan() && IsNvidia());

    // TODO(crbug.com/dawn/1705): Use a zero buffer to clear buffers. Otherwise, the test
    // OOMs.
    DAWN_SUPPRESS_TEST_IF(IsD3D11());

    // TODO(dawn:1549) Fails on Qualcomm-based Android devices.
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsQualcomm());

    for (wgpu::BufferUsage usage : {wgpu::BufferUsage::Storage, wgpu::BufferUsage::Uniform}) {
        uint64_t maxBufferBindingSize;
        std::string shader;
        switch (usage) {
            case wgpu::BufferUsage::Storage:
                maxBufferBindingSize = GetSupportedLimits().limits.maxStorageBufferBindingSize;
                // TODO(crbug.com/dawn/1160): Usually can't actually allocate a buffer this large
                // because allocating the buffer for zero-initialization fails.
                maxBufferBindingSize =
                    std::min(maxBufferBindingSize, uint64_t(2) * 1024 * 1024 * 1024);
                // With WARP or on 32-bit platforms, such large buffer allocations often fail.
#if DAWN_PLATFORM_IS(32_BIT)
                if (IsWindows()) {
                    continue;
                }
#endif
                if (IsWARP()) {
                    maxBufferBindingSize =
                        std::min(maxBufferBindingSize, uint64_t(512) * 1024 * 1024);
                }
                maxBufferBindingSize = Align(maxBufferBindingSize - 3u, 4);
                shader = R"(
                  struct Buf {
                      values : array<u32>
                  }

                  struct Result {
                      value0 : u32,
                      value1 : u32,
                  }

                  @group(0) @binding(0) var<storage, read> buf : Buf;
                  @group(0) @binding(1) var<storage, read_write> result : Result;

                  @compute @workgroup_size(1,1,1)
                  fn main() {
                      result.value0 = buf.values[0];
                      result.value1 = buf.values[arrayLength(&buf.values) - 1u];
                  }
              )";
                break;
            case wgpu::BufferUsage::Uniform:
                maxBufferBindingSize = GetSupportedLimits().limits.maxUniformBufferBindingSize;

                // Clamp to not exceed the maximum i32 value for the WGSL @size(x) annotation.
                maxBufferBindingSize = std::min(maxBufferBindingSize,
                                                uint64_t(std::numeric_limits<int32_t>::max()) + 8);
                maxBufferBindingSize = Align(maxBufferBindingSize - 3u, 4);

                shader = R"(
                  struct Buf {
                      value0 : u32,
                      // padding such that value0 and value1 are the first and last bytes of the memory.
                      @size()" +
                         std::to_string(maxBufferBindingSize - 8) + R"() padding : u32,
                      value1 : u32,
                  }

                  struct Result {
                      value0 : u32,
                      value1 : u32,
                  }

                  @group(0) @binding(0) var<uniform> buf : Buf;
                  @group(0) @binding(1) var<storage, read_write> result : Result;

                  @compute @workgroup_size(1,1,1)
                  fn main() {
                      result.value0 = buf.value0;
                      result.value1 = buf.value1;
                  }
              )";
                break;
            default:
                DAWN_UNREACHABLE();
        }

        device.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);

        wgpu::BufferDescriptor bufDesc;
        bufDesc.size = maxBufferBindingSize;
        bufDesc.usage = usage | wgpu::BufferUsage::CopyDst;
        wgpu::Buffer buffer = device.CreateBuffer(&bufDesc);

        WGPUErrorType oomResult;
        device.PopErrorScope([](WGPUErrorType type, const char*,
                                void* userdata) { *static_cast<WGPUErrorType*>(userdata) = type; },
                             &oomResult);
        device.Tick();
        FlushWire();
        // Max buffer size is smaller than the max buffer binding size.
        DAWN_TEST_UNSUPPORTED_IF(oomResult == WGPUErrorType_OutOfMemory);

        wgpu::BufferDescriptor resultBufDesc;
        resultBufDesc.size = 8;
        resultBufDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
        wgpu::Buffer resultBuffer = device.CreateBuffer(&resultBufDesc);

        uint32_t value0 = 89234;
        queue.WriteBuffer(buffer, 0, &value0, sizeof(value0));

        uint32_t value1 = 234;
        uint64_t value1Offset = Align(maxBufferBindingSize - sizeof(value1), 4);
        queue.WriteBuffer(buffer, value1Offset, &value1, sizeof(value1));

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = utils::CreateShaderModule(device, shader.c_str());
        csDesc.compute.entryPoint = "main";
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, buffer}, {1, resultBuffer}});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER_U32_EQ(value0, resultBuffer, 0)
            << "maxBufferBindingSize=" << maxBufferBindingSize << "; offset=" << 0
            << "; usage=" << usage;
        EXPECT_BUFFER_U32_EQ(value1, resultBuffer, 4)
            << "maxBufferBindingSize=" << maxBufferBindingSize << "; offset=" << value1Offset
            << "; usage=" << usage;
    }
}

// Test using the maximum number of dynamic uniform and storage buffers
TEST_P(MaxLimitTests, MaxDynamicBuffers) {
    // TODO(https://anglebug.com/8177) Causes assertion failure in ANGLE.
    DAWN_SUPPRESS_TEST_IF(IsANGLE() && IsWindows());

    wgpu::Limits limits = GetSupportedLimits().limits;

    std::vector<wgpu::BindGroupLayoutEntry> bglEntries;
    std::vector<wgpu::BindGroupEntry> bgEntries;

    // Binding number counter which is bumped as we create bind group layout
    // entries.
    uint32_t bindingNumber = 1u;

    // Lambda to create a buffer. The binding number is written at an offset of
    // 256 bytes. The test binds at a 256-byte dynamic offset and checks that the
    // contents of the buffer are equal to the binding number.
    std::vector<uint32_t> bufferData(1 + 256 / sizeof(uint32_t));
    auto MakeBuffer = [&](wgpu::BufferUsage usage) {
        *bufferData.rbegin() = bindingNumber;
        return utils::CreateBufferFromData(device, bufferData.data(),
                                           sizeof(uint32_t) * bufferData.size(), usage);
    };

    // Create as many dynamic uniform buffers as the limits allow.
    for (uint32_t i = 0u; i < limits.maxDynamicUniformBuffersPerPipelineLayout &&
                          i < 2 * limits.maxUniformBuffersPerShaderStage;
         ++i) {
        wgpu::Buffer buffer = MakeBuffer(wgpu::BufferUsage::Uniform);

        bglEntries.push_back(utils::BindingLayoutEntryInitializationHelper{
            bindingNumber,
            // When we surpass the per-stage limit, switch to the fragment shader.
            i < limits.maxUniformBuffersPerShaderStage ? wgpu::ShaderStage::Vertex
                                                       : wgpu::ShaderStage::Fragment,
            wgpu::BufferBindingType::Uniform, true});
        bgEntries.push_back(
            utils::BindingInitializationHelper(bindingNumber, buffer, 0, sizeof(uint32_t))
                .GetAsBinding());

        ++bindingNumber;
    }

    // Create as many dynamic storage buffers as the limits allow.
    for (uint32_t i = 0; i < limits.maxDynamicStorageBuffersPerPipelineLayout &&
                         i < 2 * limits.maxStorageBuffersPerShaderStage;
         ++i) {
        wgpu::Buffer buffer = MakeBuffer(wgpu::BufferUsage::Storage);

        bglEntries.push_back(utils::BindingLayoutEntryInitializationHelper{
            bindingNumber,
            // When we surpass the per-stage limit, switch to the fragment shader.
            i < limits.maxStorageBuffersPerShaderStage ? wgpu::ShaderStage::Vertex
                                                       : wgpu::ShaderStage::Fragment,
            wgpu::BufferBindingType::ReadOnlyStorage, true});
        bgEntries.push_back(
            utils::BindingInitializationHelper(bindingNumber, buffer, 0, sizeof(uint32_t))
                .GetAsBinding());

        ++bindingNumber;
    }

    // Create the bind group layout.
    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.entryCount = bglEntries.size();
    bglDesc.entries = bglEntries.data();
    wgpu::BindGroupLayout bgl = device.CreateBindGroupLayout(&bglDesc);

    // Create the bind group.
    wgpu::BindGroupDescriptor bgDesc;
    bgDesc.layout = bgl;
    bgDesc.entryCount = bgEntries.size();
    bgDesc.entries = bgEntries.data();
    wgpu::BindGroup bindGroup = device.CreateBindGroup(&bgDesc);

    // Generate binding declarations at the top of the the shader.
    std::ostringstream wgslShader;
    for (const auto& binding : bglEntries) {
        if (binding.buffer.type == wgpu::BufferBindingType::Uniform) {
            wgslShader << "@group(0) @binding(" << binding.binding << ") var<uniform> b"
                       << binding.binding << ": u32;\n";
        } else if (binding.buffer.type == wgpu::BufferBindingType::ReadOnlyStorage) {
            wgslShader << "@group(0) @binding(" << binding.binding << ") var<storage, read> b"
                       << binding.binding << ": u32;\n";
        }
    }

    // Generate a vertex shader which rasterizes primitives outside the viewport
    // if the bound buffer contents are not expected.
    wgslShader << "@vertex fn vert_main() -> @builtin(position) vec4f {\n";
    for (const auto& binding : bglEntries) {
        if (binding.visibility == wgpu::ShaderStage::Vertex) {
            // If the value is not what is expected, return a vertex that will be clipped.
            wgslShader << "    if (b" << binding.binding << " != " << binding.binding
                       << "u) { return vec4f(10.0, 10.0, 10.0, 1.0); }\n";
        }
    }
    wgslShader << "    return vec4f(0.0, 0.0, 0.5, 1.0);\n";
    wgslShader << "}\n";

    // Generate a fragment shader which discards fragments if the bound buffer
    // contents are not expected.
    wgslShader << "@fragment fn frag_main() -> @location(0) u32 {\n";
    for (const auto& binding : bglEntries) {
        if (binding.visibility == wgpu::ShaderStage::Fragment) {
            // If the value is not what is expected, discard.
            wgslShader << "    if (b" << binding.binding << " != " << binding.binding
                       << "u) { discard; }\n";
        }
    }
    wgslShader << "    return 1u;\n";
    wgslShader << "}\n";

    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, wgslShader.str().c_str());

    // Create a render target. Its contents will be 1 if the test passes.
    wgpu::TextureDescriptor renderTargetDesc;
    renderTargetDesc.size = {1, 1};
    renderTargetDesc.format = wgpu::TextureFormat::R8Uint;
    renderTargetDesc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture renderTarget = device.CreateTexture(&renderTargetDesc);

    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.layout = utils::MakePipelineLayout(device, {bgl});
    pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    pipelineDesc.vertex.module = shaderModule;
    pipelineDesc.vertex.entryPoint = "vert_main";
    pipelineDesc.cFragment.module = shaderModule;
    pipelineDesc.cFragment.entryPoint = "frag_main";
    pipelineDesc.cTargets[0].format = renderTargetDesc.format;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

    utils::ComboRenderPassDescriptor rpDesc({renderTarget.CreateView()});
    rpDesc.cColorAttachments[0].clearValue = {};
    rpDesc.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
    rpDesc.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rpDesc);

    // Bind the bind group with all resources at a 256-byte dynamic offset, and draw.
    std::vector<uint32_t> dynamicOffsets(bglEntries.size(), 256u);
    pass.SetBindGroup(0, bindGroup, dynamicOffsets.size(), dynamicOffsets.data());
    pass.SetPipeline(pipeline);
    pass.Draw(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    uint32_t expected = 1u;
    EXPECT_TEXTURE_EQ(&expected, renderTarget, {0, 0}, {1, 1});
}

// Test using the maximum number of storage buffers per shader stage.
TEST_P(MaxLimitTests, MaxStorageBuffersPerShaderStage) {
    // TODO(dawn:2162): Triage this failure.
    DAWN_SUPPRESS_TEST_IF(IsANGLE() && IsWindows());

    wgpu::Limits limits = GetSupportedLimits().limits;

    std::vector<wgpu::BindGroupLayoutEntry> bglEntries;
    std::vector<wgpu::BindGroupEntry> bgEntries;

    // Binding number counter which is bumped as we create bind group layout
    // entries.
    uint32_t bindingNumber = 1u;

    // Lambda to create a buffer.
    // The test binds checks that the contents of the buffer are equal to the binding number.
    auto MakeBuffer = [&]() {
        uint32_t bufferData = bindingNumber;
        return utils::CreateBufferFromData(device, &bufferData, sizeof(bufferData),
                                           wgpu::BufferUsage::Storage);
    };

    // Create as many storage buffers as the limits allow.
    for (uint32_t i = 0; i < 2 * limits.maxStorageBuffersPerShaderStage; ++i) {
        wgpu::Buffer buffer = MakeBuffer();

        bglEntries.push_back(utils::BindingLayoutEntryInitializationHelper{
            bindingNumber,
            // When we surpass the per-stage limit, switch to the fragment shader.
            i < limits.maxStorageBuffersPerShaderStage ? wgpu::ShaderStage::Vertex
                                                       : wgpu::ShaderStage::Fragment,
            wgpu::BufferBindingType::ReadOnlyStorage});
        bgEntries.push_back(
            utils::BindingInitializationHelper(bindingNumber, buffer, 0, sizeof(uint32_t))
                .GetAsBinding());
        ++bindingNumber;
    }

    // Create the bind group layout.
    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.entryCount = bglEntries.size();
    bglDesc.entries = bglEntries.data();
    wgpu::BindGroupLayout bgl = device.CreateBindGroupLayout(&bglDesc);

    // Create the bind group.
    wgpu::BindGroupDescriptor bgDesc;
    bgDesc.layout = bgl;
    bgDesc.entryCount = bgEntries.size();
    bgDesc.entries = bgEntries.data();
    wgpu::BindGroup bindGroup = device.CreateBindGroup(&bgDesc);

    // Generate binding declarations at the top of the the shader.
    std::ostringstream wgslShader;
    for (const auto& binding : bglEntries) {
        wgslShader << "@group(0) @binding(" << binding.binding << ") var<storage, read> b"
                   << binding.binding << ": u32;\n";
    }

    // Generate a vertex shader which rasterizes primitives outside the viewport
    // if the bound buffer contents are not expected.
    wgslShader << "@vertex fn vert_main() -> @builtin(position) vec4f {\n";
    for (const auto& binding : bglEntries) {
        if (binding.visibility == wgpu::ShaderStage::Vertex) {
            // If the value is not what is expected, return a vertex that will be clipped.
            wgslShader << "    if (b" << binding.binding << " != " << binding.binding
                       << "u) { return vec4f(10.0, 10.0, 10.0, 1.0); }\n";
        }
    }
    wgslShader << "    return vec4f(0.0, 0.0, 0.5, 1.0);\n";
    wgslShader << "}\n";

    // Generate a fragment shader which discards fragments if the bound buffer
    // contents are not expected.
    wgslShader << "@fragment fn frag_main() -> @location(0) u32 {\n";
    for (const auto& binding : bglEntries) {
        if (binding.visibility == wgpu::ShaderStage::Fragment) {
            // If the value is not what is expected, discard.
            wgslShader << "    if (b" << binding.binding << " != " << binding.binding
                       << "u) { discard; }\n";
        }
    }
    wgslShader << "    return 1u;\n";
    wgslShader << "}\n";

    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, wgslShader.str().c_str());

    // Create a render target. Its contents will be 1 if the test passes.
    wgpu::TextureDescriptor renderTargetDesc;
    renderTargetDesc.size = {1, 1};
    renderTargetDesc.format = wgpu::TextureFormat::R8Uint;
    renderTargetDesc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture renderTarget = device.CreateTexture(&renderTargetDesc);

    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.layout = utils::MakePipelineLayout(device, {bgl});
    pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    pipelineDesc.vertex.module = shaderModule;
    pipelineDesc.vertex.entryPoint = "vert_main";
    pipelineDesc.cFragment.module = shaderModule;
    pipelineDesc.cFragment.entryPoint = "frag_main";
    pipelineDesc.cTargets[0].format = renderTargetDesc.format;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

    utils::ComboRenderPassDescriptor rpDesc({renderTarget.CreateView()});
    rpDesc.cColorAttachments[0].clearValue = {};
    rpDesc.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
    rpDesc.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rpDesc);

    pass.SetBindGroup(0, bindGroup);
    pass.SetPipeline(pipeline);
    pass.Draw(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    uint32_t expected = 1u;
    EXPECT_TEXTURE_EQ(&expected, renderTarget, {0, 0}, {1, 1});
}

// Test that creating a large bind group, with each binding type at the max count, works and can be
// used correctly. The test loads a different value from each binding, and writes 1 to a storage
// buffer if all values are correct.
TEST_P(MaxLimitTests, ReallyLargeBindGroup) {
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES());
    wgpu::Limits limits = GetSupportedLimits().limits;

    std::ostringstream interface;
    std::ostringstream body;
    uint32_t binding = 0;
    uint32_t expectedValue = 42;

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

    auto CreateTextureWithRedData = [&](wgpu::TextureFormat format, uint32_t value,
                                        wgpu::TextureUsage usage) {
        wgpu::TextureDescriptor textureDesc = {};
        textureDesc.usage = wgpu::TextureUsage::CopyDst | usage;
        textureDesc.size = {1, 1, 1};
        textureDesc.format = format;
        wgpu::Texture texture = device.CreateTexture(&textureDesc);

        if (format == wgpu::TextureFormat::R8Unorm) {
            DAWN_ASSERT(expectedValue < 255u);
        }
        wgpu::Buffer textureData =
            utils::CreateBufferFromData(device, wgpu::BufferUsage::CopySrc, {value});

        wgpu::ImageCopyBuffer imageCopyBuffer = {};
        imageCopyBuffer.buffer = textureData;
        imageCopyBuffer.layout.bytesPerRow = 256;

        wgpu::ImageCopyTexture imageCopyTexture = {};
        imageCopyTexture.texture = texture;

        wgpu::Extent3D copySize = {1, 1, 1};

        commandEncoder.CopyBufferToTexture(&imageCopyBuffer, &imageCopyTexture, &copySize);
        return texture;
    };

    std::vector<wgpu::BindGroupEntry> bgEntries;
    for (uint32_t i = 0;
         i < std::min(limits.maxSampledTexturesPerShaderStage, limits.maxSamplersPerShaderStage);
         ++i) {
        wgpu::Texture texture = CreateTextureWithRedData(
            wgpu::TextureFormat::R8Unorm, expectedValue, wgpu::TextureUsage::TextureBinding);
        bgEntries.push_back({nullptr, binding, nullptr, 0, 0, nullptr, texture.CreateView()});

        interface << "@group(0) @binding(" << binding++ << ") "
                  << "var tex" << i << " : texture_2d<f32>;\n";

        bgEntries.push_back({nullptr, binding, nullptr, 0, 0, device.CreateSampler(), nullptr});

        interface << "@group(0) @binding(" << binding++ << ")"
                  << "var samp" << i << " : sampler;\n";

        body << "if (abs(textureSampleLevel(tex" << i << ", samp" << i
             << ", vec2f(0.5, 0.5), 0.0).r - " << expectedValue++ << ".0 / 255.0) > 0.0001) {\n";
        body << "    return;\n";
        body << "}\n";
    }
    for (uint32_t i = 0; i < limits.maxStorageTexturesPerShaderStage; ++i) {
        wgpu::Texture texture = CreateTextureWithRedData(
            wgpu::TextureFormat::R32Uint, expectedValue, wgpu::TextureUsage::StorageBinding);
        bgEntries.push_back({nullptr, binding, nullptr, 0, 0, nullptr, texture.CreateView()});

        interface << "@group(0) @binding(" << binding++ << ") "
                  << "var image" << i << " : texture_storage_2d<r32uint, write>;\n";

        body << "_ = image" << i << ";";
    }

    for (uint32_t i = 0; i < limits.maxUniformBuffersPerShaderStage; ++i) {
        wgpu::Buffer buffer = utils::CreateBufferFromData<uint32_t>(
            device, wgpu::BufferUsage::Uniform, {expectedValue, 0, 0, 0});
        bgEntries.push_back({nullptr, binding, buffer, 0, 4 * sizeof(uint32_t), nullptr, nullptr});

        interface << "struct UniformBuffer" << i << R"({
                value : u32
            }
        )";
        interface << "@group(0) @binding(" << binding++ << ") "
                  << "var<uniform> ubuf" << i << " : UniformBuffer" << i << ";\n";

        body << "if (ubuf" << i << ".value != " << expectedValue++ << "u) {\n";
        body << "    return;\n";
        body << "}\n";
    }
    // Save one storage buffer for writing the result
    for (uint32_t i = 0; i < limits.maxStorageBuffersPerShaderStage - 1; ++i) {
        wgpu::Buffer buffer = utils::CreateBufferFromData<uint32_t>(
            device, wgpu::BufferUsage::Storage, {expectedValue});
        bgEntries.push_back({nullptr, binding, buffer, 0, sizeof(uint32_t), nullptr, nullptr});

        interface << "struct ReadOnlyStorageBuffer" << i << R"({
                value : u32
            }
        )";
        interface << "@group(0) @binding(" << binding++ << ") "
                  << "var<storage, read> sbuf" << i << " : ReadOnlyStorageBuffer" << i << ";\n";

        body << "if (sbuf" << i << ".value != " << expectedValue++ << "u) {\n";
        body << "    return;\n";
        body << "}\n";
    }

    wgpu::Buffer result = utils::CreateBufferFromData<uint32_t>(
        device, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc, {0});
    bgEntries.push_back({nullptr, binding, result, 0, sizeof(uint32_t), nullptr, nullptr});

    interface << R"(struct ReadWriteStorageBuffer{
            value : u32
        }
    )";
    interface << "@group(0) @binding(" << binding++ << ") "
              << "var<storage, read_write> result : ReadWriteStorageBuffer;\n";

    body << "result.value = 1u;\n";

    std::string shader =
        interface.str() + "@compute @workgroup_size(1) fn main() {\n" + body.str() + "}\n";
    wgpu::ComputePipelineDescriptor cpDesc;
    cpDesc.compute.module = utils::CreateShaderModule(device, shader.c_str());
    cpDesc.compute.entryPoint = "main";
    wgpu::ComputePipeline cp = device.CreateComputePipeline(&cpDesc);

    wgpu::BindGroupDescriptor bgDesc = {};
    bgDesc.layout = cp.GetBindGroupLayout(0);
    bgDesc.entryCount = bgEntries.size();
    bgDesc.entries = bgEntries.data();

    wgpu::BindGroup bg = device.CreateBindGroup(&bgDesc);

    wgpu::ComputePassEncoder pass = commandEncoder.BeginComputePass();
    pass.SetPipeline(cp);
    pass.SetBindGroup(0, bg);
    pass.DispatchWorkgroups(1, 1, 1);
    pass.End();

    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(1, result, 0);
}

// Verifies that devices can write to the limits specified for fragment outputs. This test
// exercises an internal Vulkan maxFragmentCombinedOutputResources limit and makes sure that the
// sub parts of the limit work as intended.
TEST_P(MaxLimitTests, WriteToMaxFragmentCombinedOutputResources) {
    // TODO(dawn:1692) Currently does not work on GL and GLES.
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() || IsOpenGLES());

    // Compute the number of each resource type (storage buffers and storage textures) such that
    // there is at least one color attachment, and as many of the buffer/textures as possible,
    // splitting a shared remaining count between the two resources if they are not separately
    // defined, or exceed the combined limit.
    wgpu::Limits limits = GetSupportedLimits().limits;
    uint32_t attachmentCount = limits.maxColorAttachments;
    uint32_t storageBuffers = limits.maxStorageBuffersPerShaderStage;
    uint32_t storageTextures = limits.maxStorageTexturesPerShaderStage;

    // Create a shader to write out to all the resources.
    auto CreateShader = [&]() -> wgpu::ShaderModule {
        // Header to declare storage buffer struct.
        std::ostringstream bufferBindings;
        std::ostringstream bufferOutputs;
        for (uint32_t i = 0; i < storageBuffers; i++) {
            bufferBindings << "@group(0) @binding(" << i << ") var<storage, read_write> b" << i
                           << ": u32;\n";
            bufferOutputs << "    b" << i << " = " << i << "u + 1u;\n";
        }

        std::ostringstream textureBindings;
        std::ostringstream textureOutputs;
        for (uint32_t i = 0; i < storageTextures; i++) {
            textureBindings << "@group(1) @binding(" << i << ") var t" << i
                            << ": texture_storage_2d<rgba8uint, write>;\n";
            textureOutputs << "    textureStore(t" << i << ", vec2u(0, 0), vec4u(" << i
                           << "u + 1u));\n";
        }

        std::ostringstream targetBindings;
        std::ostringstream targetOutputs;
        for (size_t i = 0; i < attachmentCount; i++) {
            targetBindings << "@location(" << i << ") o" << i << " : u32, ";
            targetOutputs << i << "u + 1u, ";
        }

        std::ostringstream fsShader;
        fsShader << bufferBindings.str();
        fsShader << textureBindings.str();
        fsShader << "struct Outputs { " << targetBindings.str() << "}\n";
        fsShader << "@fragment fn main() -> Outputs {\n";
        fsShader << bufferOutputs.str();
        fsShader << textureOutputs.str();
        fsShader << "    return Outputs(" << targetOutputs.str() << ");\n";
        fsShader << "}";
        return utils::CreateShaderModule(device, fsShader.str().c_str());
    };

    // Constants used for the render pipeline.
    wgpu::ColorTargetState kColorTargetState = {};
    kColorTargetState.format = wgpu::TextureFormat::R8Uint;

    // Create the render pipeline.
    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");
    pipelineDesc.vertex.entryPoint = "main";
    pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    pipelineDesc.cFragment.module = CreateShader();
    pipelineDesc.cFragment.entryPoint = "main";
    pipelineDesc.cTargets.fill(kColorTargetState);
    pipelineDesc.cFragment.targetCount = attachmentCount;
    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&pipelineDesc);

    // Create all the resources and bindings for them.
    std::vector<wgpu::Buffer> buffers;
    std::vector<wgpu::BindGroupEntry> bufferEntries;
    wgpu::BufferDescriptor bufferDesc = {};
    bufferDesc.size = 4;
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    for (uint32_t i = 0; i < storageBuffers; i++) {
        buffers.push_back(device.CreateBuffer(&bufferDesc));
        bufferEntries.push_back(utils::BindingInitializationHelper(i, buffers[i]).GetAsBinding());
    }
    wgpu::BindGroupDescriptor bufferBindGroupDesc = {};
    bufferBindGroupDesc.layout = renderPipeline.GetBindGroupLayout(0);
    bufferBindGroupDesc.entryCount = storageBuffers;
    bufferBindGroupDesc.entries = bufferEntries.data();
    wgpu::BindGroup bufferBindGroup = device.CreateBindGroup(&bufferBindGroupDesc);

    std::vector<wgpu::Texture> textures;
    std::vector<wgpu::BindGroupEntry> textureEntries;
    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.size.width = 1;
    textureDesc.size.height = 1;
    textureDesc.format = wgpu::TextureFormat::RGBA8Uint;
    textureDesc.usage = wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc;
    for (uint32_t i = 0; i < storageTextures; i++) {
        textures.push_back(device.CreateTexture(&textureDesc));
        textureEntries.push_back(
            utils::BindingInitializationHelper(i, textures[i].CreateView()).GetAsBinding());
    }
    wgpu::BindGroupDescriptor textureBindGroupDesc = {};
    textureBindGroupDesc.layout = renderPipeline.GetBindGroupLayout(1);
    textureBindGroupDesc.entryCount = storageTextures;
    textureBindGroupDesc.entries = textureEntries.data();
    wgpu::BindGroup textureBindGroup = device.CreateBindGroup(&textureBindGroupDesc);

    std::vector<wgpu::Texture> attachments;
    std::vector<wgpu::TextureView> attachmentViews;
    wgpu::TextureDescriptor attachmentDesc = {};
    attachmentDesc.size = {1, 1};
    attachmentDesc.format = wgpu::TextureFormat::R8Uint;
    attachmentDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    for (size_t i = 0; i < attachmentCount; i++) {
        attachments.push_back(device.CreateTexture(&attachmentDesc));
        attachmentViews.push_back(attachments[i].CreateView());
    }

    // Execute the pipeline.
    utils::ComboRenderPassDescriptor passDesc(attachmentViews);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDesc);
    pass.SetBindGroup(0, bufferBindGroup);
    pass.SetBindGroup(1, textureBindGroup);
    pass.SetPipeline(renderPipeline);
    pass.Draw(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Verify the results.
    for (uint32_t i = 0; i < storageBuffers; i++) {
        EXPECT_BUFFER_U32_EQ(i + 1, buffers[i], 0);
    }
    for (uint32_t i = 0; i < storageTextures; i++) {
        const uint32_t res = i + 1;
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(res, res, res, res), textures[i], 0, 0);
    }
    for (uint32_t i = 0; i < attachmentCount; i++) {
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(i + 1, 0, 0, 0), attachments[i], 0, 0);
    }
}

// Verifies that supported buffer limits do not exceed maxBufferSize.
TEST_P(MaxLimitTests, MaxBufferSizes) {
    // Base limits without tiering.
    wgpu::Limits baseLimits = GetAdapterLimits().limits;
    EXPECT_LE(baseLimits.maxStorageBufferBindingSize, baseLimits.maxBufferSize);
    EXPECT_LE(baseLimits.maxUniformBufferBindingSize, baseLimits.maxBufferSize);

    // Base limits with tiering.
    GetAdapter().SetUseTieredLimits(true);
    wgpu::Limits tieredLimits = GetAdapterLimits().limits;
    EXPECT_LE(tieredLimits.maxStorageBufferBindingSize, tieredLimits.maxBufferSize);
    EXPECT_LE(tieredLimits.maxUniformBufferBindingSize, tieredLimits.maxBufferSize);

    // Unset tiered limit usage to avoid affecting other tests.
    GetAdapter().SetUseTieredLimits(false);
}

DAWN_INSTANTIATE_TEST(MaxLimitTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

// Verifies the limits maxInterStageShaderVariables and maxInterStageShaderComponents work correctly
class MaxInterStageLimitTests : public MaxLimitTests {
  public:
    struct MaxInterStageLimitTestsSpec {
        bool renderPointLists;
        bool hasSampleMask;
        bool hasSampleIndex;
        bool hasFrontFacing;
    };

    void DoTest(const MaxInterStageLimitTestsSpec& spec) {
        wgpu::RenderPipeline pipeline = CreateRenderPipeline(spec);
        EXPECT_NE(nullptr, pipeline.Get());
    }

  private:
    struct InterStageVariableAllocation {
        uint32_t variableSize;
        uint32_t variableCount;
    };
    using InterStageVariableAllocations = std::array<InterStageVariableAllocation, 2>;

    // Allocate the inter-stage shader variables that consume all inter-stage shader variables and
    // components
    InterStageVariableAllocations AllocateInterStageVariables(
        const MaxInterStageLimitTestsSpec& spec) {
        wgpu::Limits baseLimits = GetAdapterLimits().limits;

        uint32_t interStageVariableCount = baseLimits.maxInterStageShaderVariables;

        uint32_t builtinCount = static_cast<uint32_t>(spec.renderPointLists) +
                                static_cast<uint32_t>(spec.hasFrontFacing) +
                                static_cast<uint32_t>(spec.hasSampleIndex) +
                                static_cast<uint32_t>(spec.hasSampleMask);
        uint32_t userDefinedInterStageComponents =
            baseLimits.maxInterStageShaderComponents - builtinCount;

        InterStageVariableAllocations allocation;

        // Allocate `userDefinedInterStageComponents` inter-stage shader components to fill
        // `interStageVariableCount` inter-stage shader variables.
        // For example, assuming we are neither rendering point lists and nor using built-ins other
        // than @builtin(position):
        // 1. on D3D12, maxInterStageShaderVariables == 30 and maxInterStageShaderComponents == 120,
        //    so allocation[1].variableCount = 0 (allocation[1].variableSize == 5 is not used) and
        //    allocation[0].variableCount = 30.
        // 2. on Vulkan. maxInterStageShaderVariables can be 16 and maxInterStageShaderComponents
        //    can be 60, so the inter-stage variables are 12 vec4fs and 4 vec3fs.
        allocation[0].variableSize = userDefinedInterStageComponents / interStageVariableCount;
        allocation[1].variableSize = allocation[0].variableSize + 1;
        allocation[1].variableCount =
            userDefinedInterStageComponents - interStageVariableCount * allocation[0].variableSize;
        allocation[0].variableCount = interStageVariableCount - allocation[1].variableCount;
        DAWN_ASSERT(userDefinedInterStageComponents ==
                    allocation[0].variableSize * allocation[0].variableCount +
                        allocation[1].variableSize * allocation[1].variableCount);

        return allocation;
    }

    std::string GetWGSLTypeFromVariableSize(uint32_t variableSize) {
        switch (variableSize) {
            case 1:
                return "f32";
            case 2:
                return "vec2f";
            case 3:
                return "vec3f";
            case 4:
                return "vec4f";
            // It's OK to return an empty string for variableSize == 5 as when variableSizes[i] ==
            // 0, variableSizesCount[i] must be 5, thus the returned empty string won't be used.
            case 5:
                return "";
            default:
                DAWN_UNREACHABLE();
                return "";
        }
    }

    std::string GetInterStageVariableDeclarations(const InterStageVariableAllocations& allocation) {
        std::stringstream stream;

        stream << "struct VertexOut {" << std::endl;

        uint32_t interStageVariableCount =
            allocation[0].variableCount + allocation[1].variableCount;
        for (uint32_t location = 0; location < interStageVariableCount; ++location) {
            uint32_t variableIndexInAllocation = (location < allocation[0].variableCount) ? 0 : 1;
            std::string wgslType =
                GetWGSLTypeFromVariableSize(allocation[variableIndexInAllocation].variableSize);
            stream << "@location(" << location << ") color" << location << " : " << wgslType << ", "
                   << std::endl;
        }

        stream << "@builtin(position) pos : vec4f" << std::endl << "}" << std::endl;

        return stream.str();
    }

    wgpu::ShaderModule GetShaderModuleForTest(const MaxInterStageLimitTestsSpec& spec) {
        std::stringstream stream;

        InterStageVariableAllocations allocation = AllocateInterStageVariables(spec);
        stream << GetInterStageVariableDeclarations(allocation) << std::endl
               << GetVertexShaderForTest(allocation) << std::endl
               << GetFragmentShaderForTest(allocation, spec) << std::endl;
        return utils::CreateShaderModule(device, stream.str().c_str());
    }

    std::string GetVertexShaderForTest(const InterStageVariableAllocations& allocation) {
        std::stringstream stream;
        stream << R"(
        @vertex
        fn vs_main(@builtin(vertex_index) vertexIndex : u32) -> VertexOut {
            var pos = array<vec2f, 3>(
                vec2f(-1.0, -1.0),
                vec2f( 2.0,  0.0),
                vec2f( 0.0,  2.0));
            var output : VertexOut;
            output.pos = vec4f(pos[vertexIndex], 0.0, 1.0);
            var vertexIndexFloat  = f32(vertexIndex);
)";
        // Ensure every inter-stage shader variable is used instead of being optimized out.
        uint32_t interStageVariableCount =
            allocation[0].variableCount + allocation[1].variableCount;
        for (uint32_t location = 0; location < interStageVariableCount; ++location) {
            uint32_t variableIndexInAllocation = (location < allocation[0].variableCount) ? 0 : 1;
            std::string wgslType =
                GetWGSLTypeFromVariableSize(allocation[variableIndexInAllocation].variableSize);
            stream << "output.color" << location << " = " << wgslType << "(";
            for (uint32_t index = 0; index < allocation[variableIndexInAllocation].variableSize;
                 ++index) {
                stream << "vertexIndexFloat / "
                       << location * allocation[variableIndexInAllocation].variableCount + index +
                              1;
                if (index != allocation[variableIndexInAllocation].variableSize - 1) {
                    stream << ", ";
                }
            }
            stream << ");" << std::endl;
        }
        stream << "return output;" << std::endl << "}" << std::endl;
        return stream.str();
    }

    std::string GetInterStageVariableToVec4f(uint32_t location, uint32_t variableSize) {
        std::stringstream stream;

        switch (variableSize) {
            case 1:
                stream << "vec4f(input.color" << location << ", 0, 0, 1)";
                break;
            case 2:
                stream << "vec4f(input.color" << location << ", 0, 1)";
                break;
            case 3:
                stream << "vec4f(input.color" << location << ", 1)";
                break;
            case 4:
                stream << "input.color" << location;
                break;
            default:
                DAWN_UNREACHABLE();
                break;
        }

        return stream.str();
    }

    std::string GetFragmentShaderForTest(const InterStageVariableAllocations& allocation,
                                         const MaxInterStageLimitTestsSpec& spec) {
        std::stringstream stream;

        stream << "@fragment fn fs_main(input: VertexOut";
        if (spec.hasFrontFacing) {
            stream << ", @builtin(front_facing) isFront : bool";
        }
        if (spec.hasSampleIndex) {
            stream << ", @builtin(sample_index) sampleIndex : u32";
        }
        if (spec.hasSampleMask) {
            stream << ", @builtin(sample_mask) sampleMask : u32";
        }
        // Ensure every inter-stage shader variable and built-in variable is used instead of being
        // optimized out.
        stream << ") -> @location(0) vec4f {" << std::endl << "return input.pos";
        if (spec.hasFrontFacing) {
            stream << " + vec4f(f32(isFront), 0, 0, 1)";
        }
        if (spec.hasSampleIndex) {
            stream << " + vec4f(f32(sampleIndex), 0, 0, 1)";
        }
        if (spec.hasSampleMask) {
            stream << " + vec4f(f32(sampleMask), 0, 0, 1)";
        }
        uint32_t interStageVariableCount =
            allocation[0].variableCount + allocation[1].variableCount;
        for (uint32_t location = 0; location < interStageVariableCount; ++location) {
            uint32_t variableIndexInAllocation = (location < allocation[0].variableCount) ? 0 : 1;
            stream << " + "
                   << GetInterStageVariableToVec4f(
                          location, allocation[variableIndexInAllocation].variableSize);
        }
        stream << ";}";
        return stream.str();
    }

    wgpu::RenderPipeline CreateRenderPipeline(const MaxInterStageLimitTestsSpec& spec) {
        wgpu::ShaderModule shaderModule = GetShaderModuleForTest(spec);
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = shaderModule;
        descriptor.vertex.entryPoint = "vs_main";
        descriptor.cFragment.module = shaderModule;
        descriptor.cFragment.entryPoint = "fs_main";
        descriptor.vertex.bufferCount = 0;
        descriptor.cBuffers[0].attributeCount = 0;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.primitive.topology = spec.renderPointLists
                                            ? wgpu::PrimitiveTopology::PointList
                                            : wgpu::PrimitiveTopology::TriangleList;
        if (spec.hasSampleIndex || spec.hasSampleMask) {
            descriptor.multisample.count = 4;
        }
        return device.CreateRenderPipeline(&descriptor);
    }
};

// Tests that both maxInterStageShaderComponents and maxInterStageShaderVariables work for a render
// pipeline with no built-in variables.
TEST_P(MaxInterStageLimitTests, NoBuiltins) {
    MaxInterStageLimitTestsSpec spec = {};
    DoTest(spec);
}

// Tests that both maxInterStageShaderComponents and maxInterStageShaderVariables work for a render
// pipeline with @builtin(sample_mask). On D3D SV_Coverage doesn't consume an independent float4
// register.
TEST_P(MaxInterStageLimitTests, SampleMask) {
    MaxInterStageLimitTestsSpec spec = {};
    spec.hasSampleMask = true;
    DoTest(spec);
}

// Tests that both maxInterStageShaderComponents and maxInterStageShaderVariables work for a render
// pipeline with @builtin(sample_index). On D3D SV_SampleIndex consumes an independent float4
// register.
TEST_P(MaxInterStageLimitTests, SampleIndex) {
    MaxInterStageLimitTestsSpec spec = {};
    spec.hasSampleIndex = true;
    DoTest(spec);
}

// Tests that both maxInterStageShaderComponents and maxInterStageShaderVariables work for a render
// pipeline with @builtin(front_facing). On D3D SV_IsFrontFace consumes an independent float4
// register.
TEST_P(MaxInterStageLimitTests, FrontFacing) {
    MaxInterStageLimitTestsSpec spec = {};
    spec.hasFrontFacing = true;
    DoTest(spec);
}

// Tests that both maxInterStageShaderComponents and maxInterStageShaderVariables work for a render
// pipeline with @builtin(front_facing). On D3D SV_IsFrontFace and SV_SampleIndex consume one
// independent float4 register.
TEST_P(MaxInterStageLimitTests, SampleIndex_FrontFacing) {
    MaxInterStageLimitTestsSpec spec = {};
    spec.hasSampleIndex = true;
    spec.hasFrontFacing = true;
    DoTest(spec);
}

// Tests that both maxInterStageShaderComponents and maxInterStageShaderVariables work for a render
// pipeline with @builtin(sample_mask),
// @builtin(sample_index) and @builtin(front_facing).
TEST_P(MaxInterStageLimitTests, SampleMask_SampleIndex_FrontFacing) {
    MaxInterStageLimitTestsSpec spec = {};
    spec.hasSampleMask = true;
    spec.hasSampleIndex = true;
    spec.hasFrontFacing = true;
    DoTest(spec);
}

// Tests that both maxInterStageShaderComponents and maxInterStageShaderVariables work for a render
// pipeline with PointList primitive topology. On Vulkan when the primitive topology is PointList,
// the SPIR-V builtin PointSize must be declared in vertex shader, which will consume 1 inter-stage
// shader component.
TEST_P(MaxInterStageLimitTests, RenderPointList) {
    MaxInterStageLimitTestsSpec spec = {};
    spec.renderPointLists = true;
    DoTest(spec);
}

// Tests that both maxInterStageShaderComponents and maxInterStageShaderVariables work for a render
// pipeline with PointList primitive topology, @builtin(sample_mask),
// @builtin(sample_index) and @builtin(front_facing).
TEST_P(MaxInterStageLimitTests, RenderPointList_SampleMask_SampleIndex_FrontFacing) {
    MaxInterStageLimitTestsSpec spec = {};
    spec.renderPointLists = true;
    spec.hasSampleMask = true;
    spec.hasSampleIndex = true;
    spec.hasFrontFacing = true;
    DoTest(spec);
}

DAWN_INSTANTIATE_TEST(MaxInterStageLimitTests,
                      D3D11Backend(),
                      D3D12Backend({}, {"use_dxc"}),
                      D3D12Backend({"use_dxc"}),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
