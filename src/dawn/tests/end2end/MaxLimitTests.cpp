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

#include <algorithm>
#include <limits>
#include <string>

#include "dawn/common/Math.h"
#include "dawn/common/Platform.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

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

        @group(0) @binding(0) var<storage, write> dst : Dst;

        struct WGData {
          value0 : u32,
          // padding such that value0 and value1 are the first and last bytes of the memory.
          @size()" + std::to_string(maxComputeWorkgroupStorageSize / 4 - 2) +
                         R"() padding : u32,
          value1 : u32,
        }
        var<workgroup> wg_data : WGData;

        @stage(compute) @workgroup_size(2,1,1)
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

    // TODO(crbug.com/dawn/1172)
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsVulkan() && IsIntel());

    // TODO(crbug.com/dawn/1217): Remove this suppression.
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsVulkan() && IsNvidia());

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
#ifdef DAWN_PLATFORM_32_BIT
                if (IsWindows()) {
                    continue;
                }
#endif
                if (IsWARP()) {
                    maxBufferBindingSize =
                        std::min(maxBufferBindingSize, uint64_t(512) * 1024 * 1024);
                }
                shader = R"(
                  struct Buf {
                      values : array<u32>
                  }

                  struct Result {
                      value0 : u32,
                      value1 : u32,
                  }

                  @group(0) @binding(0) var<storage, read> buf : Buf;
                  @group(0) @binding(1) var<storage, write> result : Result;

                  @stage(compute) @workgroup_size(1,1,1)
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
                  @group(0) @binding(1) var<storage, write> result : Result;

                  @stage(compute) @workgroup_size(1,1,1)
                  fn main() {
                      result.value0 = buf.value0;
                      result.value1 = buf.value1;
                  }
              )";
                break;
            default:
                UNREACHABLE();
        }

        device.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);

        wgpu::BufferDescriptor bufDesc;
        bufDesc.size = Align(maxBufferBindingSize, 4);
        bufDesc.usage = usage | wgpu::BufferUsage::CopyDst;
        wgpu::Buffer buffer = device.CreateBuffer(&bufDesc);

        WGPUErrorType oomResult;
        device.PopErrorScope([](WGPUErrorType type, const char*,
                                void* userdata) { *static_cast<WGPUErrorType*>(userdata) = type; },
                             &oomResult);
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

DAWN_INSTANTIATE_TEST(MaxLimitTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
