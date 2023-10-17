// Copyright 2022 The Dawn & Tint Authors
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
            bool useDxcDisabled = false;
            for (auto* disabledToggle : GetParam().forceDisabledWorkarounds) {
                if (strncmp(disabledToggle, "use_dxc", strlen("use_dxc")) == 0) {
                    useDxcDisabled = true;
                    break;
                }
            }
            mUseDxcEnabledOrNonD3D12 = !useDxcDisabled;
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
                            D3D12Backend({}, {"use_dxc"}),
                            MetalBackend(),
                            VulkanBackend(),
                        },
                        {true, false});

}  // anonymous namespace
}  // namespace dawn
