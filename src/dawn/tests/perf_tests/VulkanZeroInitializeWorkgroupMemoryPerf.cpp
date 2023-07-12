// Copyright 2023 The Dawn Authors
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

#include <string>
#include <vector>

#include "dawn/tests/perf_tests/DawnPerfTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr unsigned int kNumIterations = 50;
constexpr uint32_t kWorkgroupArraySize = 2048u;

struct ZeroInitializeWorkgroupMemoryParams : AdapterTestParam {
    ZeroInitializeWorkgroupMemoryParams(const AdapterTestParam& param, uint32_t workgroupSize)
        : AdapterTestParam(param), workgroupSize(workgroupSize) {}
    uint32_t workgroupSize;
};

std::ostream& operator<<(std::ostream& ostream, const ZeroInitializeWorkgroupMemoryParams& param) {
    ostream << static_cast<const AdapterTestParam&>(param);
    ostream << "_workgroupSize_" << param.workgroupSize;
    return ostream;
}

class VulkanZeroInitializeWorkgroupMemoryExtensionTest
    : public DawnPerfTestWithParams<ZeroInitializeWorkgroupMemoryParams> {
  public:
    VulkanZeroInitializeWorkgroupMemoryExtensionTest()
        : DawnPerfTestWithParams<ZeroInitializeWorkgroupMemoryParams>(kNumIterations, 1) {}

    ~VulkanZeroInitializeWorkgroupMemoryExtensionTest() override = default;

    void SetUp() override;

  private:
    void Step() override;

    wgpu::BindGroup mBindGroup;
    wgpu::ComputePipeline mPipeline;
};

void VulkanZeroInitializeWorkgroupMemoryExtensionTest::SetUp() {
    DawnPerfTestWithParams<ZeroInitializeWorkgroupMemoryParams>::SetUp();

    std::ostringstream ostream;
    ostream << R"(
        @group(0) @binding(0) var<storage, read_write> dst : array<f32>;

        const kWorkgroupSize = )"
            << GetParam().workgroupSize << R"(;
        const kWorkgroupArraySize = )"
            << kWorkgroupArraySize << R"(;
        const kLoopLength = kWorkgroupArraySize / kWorkgroupSize;

        var<workgroup> mm_Asub : array<f32, kWorkgroupArraySize>;
        @compute @workgroup_size(kWorkgroupSize)
        fn main(@builtin(local_invocation_id) LocalInvocationID : vec3u) {
            for (var k = 0u; k < kLoopLength; k = k + 1u) {
                var index = kLoopLength * LocalInvocationID.x + k;
                dst[index] = mm_Asub[index];
            }
        })";
    wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage, true}});

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.layout = utils::MakePipelineLayout(device, {bindGroupLayout});
    csDesc.compute.module = utils::CreateShaderModule(device, ostream.str().c_str());
    csDesc.compute.entryPoint = "main";
    mPipeline = device.CreateComputePipeline(&csDesc);

    std::array<float, kWorkgroupArraySize * kNumIterations> data;
    data.fill(1);
    wgpu::Buffer buffer = utils::CreateBufferFromData(
        device, data.data(), sizeof(data), wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage);

    mBindGroup = utils::MakeBindGroup(device, bindGroupLayout,
                                      {
                                          {0, buffer, 0, kWorkgroupArraySize},
                                      });
}

void VulkanZeroInitializeWorkgroupMemoryExtensionTest::Step() {
    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        if (SupportsTimestampQuery()) {
            RecordBeginTimestamp(encoder);
        }

        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(mPipeline);
        for (uint32_t i = 0; i < kNumIterations; ++i) {
            uint32_t dynamicBufferOffset = sizeof(float) * kWorkgroupArraySize * i;
            pass.SetBindGroup(0, mBindGroup, 1, &dynamicBufferOffset);
            pass.DispatchWorkgroups(1);
        }
        pass.End();

        if (SupportsTimestampQuery()) {
            RecordEndTimestampAndResolveQuerySet(encoder);
        }

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    if (SupportsTimestampQuery()) {
        ComputeGPUElapsedTime();
    }
}

TEST_P(VulkanZeroInitializeWorkgroupMemoryExtensionTest, Run) {
    RunTest();
}

DAWN_INSTANTIATE_TEST_P(
    VulkanZeroInitializeWorkgroupMemoryExtensionTest,
    {VulkanBackend(), VulkanBackend({}, {"use_vulkan_zero_initialize_workgroup_memory_extension"}),
     VulkanBackend({"disable_workgroup_init"}, {})},
    {64, 256});

}  // anonymous namespace
}  // namespace dawn
