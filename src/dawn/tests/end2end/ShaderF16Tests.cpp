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
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace {
using RequireShaderF16Feature = bool;
DAWN_TEST_PARAM_STRUCT(ShaderF16TestsParams, RequireShaderF16Feature);

}  // anonymous namespace

class ShaderF16Tests : public DawnTestWithParams<ShaderF16TestsParams> {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        mIsShaderF16SupportedOnAdapter = SupportsFeatures({wgpu::FeatureName::ShaderF16});
        if (!mIsShaderF16SupportedOnAdapter) {
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

        if (GetParam().mRequireShaderF16Feature && mUseDxcEnabledOrNonD3D12) {
            return {wgpu::FeatureName::ShaderF16};
        }

        return {};
    }

    bool IsShaderF16SupportedOnAdapter() const { return mIsShaderF16SupportedOnAdapter; }
    bool UseDxcEnabledOrNonD3D12() const { return mUseDxcEnabledOrNonD3D12; }

  private:
    bool mIsShaderF16SupportedOnAdapter = false;
    bool mUseDxcEnabledOrNonD3D12 = false;
};

TEST_P(ShaderF16Tests, BasicShaderF16FeaturesTest) {
    const char* computeShader = R"(
        enable f16;

        struct Buf {
            v : f32,
        }
        @group(0) @binding(0) var<storage, read_write> buf : Buf;

        @compute @workgroup_size(1)
        fn CSMain() {
            let a : f16 = f16(buf.v) + 1.0h;
            buf.v = f32(a);
        }
    )";

    const bool shouldShaderF16FeatureSupportedByDevice =
        // Required when creating device
        GetParam().mRequireShaderF16Feature &&
        // Adapter support the feature
        IsShaderF16SupportedOnAdapter() &&
        // Proper toggle, disallow_unsafe_apis and use_dxc if d3d12
        // Note that "disallow_unsafe_apis" is always disabled in DawnTestBase::CreateDeviceImpl.
        !HasToggleEnabled("disallow_unsafe_apis") && UseDxcEnabledOrNonD3D12();
    const bool deviceSupportShaderF16Feature = device.HasFeature(wgpu::FeatureName::ShaderF16);
    EXPECT_EQ(deviceSupportShaderF16Feature, shouldShaderF16FeatureSupportedByDevice);

    if (!deviceSupportShaderF16Feature) {
        ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, computeShader));
        return;
    }

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 4u;
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer bufferOut = device.CreateBuffer(&bufferDesc);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, computeShader);
    csDesc.compute.entryPoint = "CSMain";
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

    uint32_t expected[] = {0x3f800000};  // 1.0f
    EXPECT_BUFFER_U32_RANGE_EQ(expected, bufferOut, 0, 1);
}

// DawnTestBase::CreateDeviceImpl always disable disallow_unsafe_apis toggle.
DAWN_INSTANTIATE_TEST_P(ShaderF16Tests,
                        {
                            D3D12Backend(),
                            D3D12Backend({"use_dxc"}),
                            VulkanBackend(),
                            MetalBackend(),
                            OpenGLBackend(),
                            OpenGLESBackend(),
                        },
                        {true, false});
