// Copyright 2022 The Dawn Authors
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

#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

using RequestDP4aExtension = bool;
DAWN_TEST_PARAM_STRUCT(ExperimentalDP4aTestsParams, RequestDP4aExtension);

class ExperimentalDP4aTests : public DawnTestWithParams<ExperimentalDP4aTestsParams> {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        mIsDP4aSupportedOnAdapter = SupportsFeatures({wgpu::FeatureName::ChromiumExperimentalDp4a});
        if (!mIsDP4aSupportedOnAdapter) {
            return {};
        }

        if (!IsD3D12()) {
            mUseDxcEnabledOrNonD3D12 = true;
        } else {
            for (auto* enabledToggle : GetParam().forceEnabledWorkarounds) {
                if (strncmp(enabledToggle, "use_dxc", 7) == 0) {
                    mUseDxcEnabledOrNonD3D12 = true;
                    break;
                }
            }
        }

        if (GetParam().mRequestDP4aExtension && mUseDxcEnabledOrNonD3D12) {
            return {wgpu::FeatureName::ChromiumExperimentalDp4a};
        }

        return {};
    }

    bool IsDP4aSupportedOnAdapter() const { return mIsDP4aSupportedOnAdapter; }
    bool UseDxcEnabledOrNonD3D12() const { return mUseDxcEnabledOrNonD3D12; }

  private:
    bool mIsDP4aSupportedOnAdapter = false;
    bool mUseDxcEnabledOrNonD3D12 = false;
};

TEST_P(ExperimentalDP4aTests, BasicDP4aFeaturesTest) {
    const char* computeShader = R"(
        enable chromium_experimental_dp4a;

        struct Buf {
            data1 : i32,
            data2 : u32,
            data3 : i32,
            data4 : u32,
        }
        @group(0) @binding(0) var<storage, read_write> buf : Buf;

        @compute @workgroup_size(1)
        fn main() {
            var a = 0xFFFFFFFFu;
            var b = 0xFFFFFFFEu;
            var c = 0x01020304u;
            buf.data1 = dot4I8Packed(a, b);
            buf.data2 = dot4U8Packed(a, b);
            buf.data3 = dot4I8Packed(a, c);
            buf.data4 = dot4U8Packed(a, c);
        }
)";
    const bool shouldDP4AFeatureSupportedByDevice =
        // Required when creating device
        GetParam().mRequestDP4aExtension &&
        // Adapter support the feature
        IsDP4aSupportedOnAdapter() &&
        // Proper toggle, allow_unsafe_apis and use_dxc if d3d12
        // Note that "allow_unsafe_apis" is always enabled in
        // DawnTestEnvironment::CreateInstance.
        HasToggleEnabled("allow_unsafe_apis") && UseDxcEnabledOrNonD3D12();
    const bool deviceSupportDP4AFeature =
        device.HasFeature(wgpu::FeatureName::ChromiumExperimentalDp4a);
    EXPECT_EQ(deviceSupportDP4AFeature, shouldDP4AFeatureSupportedByDevice);

    if (!deviceSupportDP4AFeature) {
        ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, computeShader));
        return;
    }

    utils::CreateShaderModule(device, computeShader);

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 4 * sizeof(uint32_t);
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer bufferOut = device.CreateBuffer(&bufferDesc);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, computeShader);
    csDesc.compute.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, bufferOut},
                                                     });

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    uint32_t expected[] = {5, 259845, static_cast<uint32_t>(-10), 2550};
    EXPECT_BUFFER_U32_RANGE_EQ(expected, bufferOut, 0, 4);
}

// DawnTestBase::CreateDeviceImpl always enables allow_unsafe_apis toggle.
DAWN_INSTANTIATE_TEST_P(ExperimentalDP4aTests,
                        {
                            D3D12Backend(),
                            D3D12Backend({"use_dxc"}, {}),
                            VulkanBackend(),
                        },
                        {true, false});

}  // anonymous namespace
}  // namespace dawn
