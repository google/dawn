// Copyright 2025 The Dawn & Tint Authors
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

class ResourceTableTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(
            !SupportsFeatures({wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable}));

        // TODO(https://issues.chromium.org/435317394): The Subzero compiler used by Swiftshader
        // produces bad code and crashes on some VK_EXT_descriptor_indexing workloads. Skip tests on
        // it, but still run them with Swiftshader LLVM 10.0. On ARM64 the only supported compiler
        // is LLVM10.0 so use that signal to choose when Swiftshader can be tested.
        DAWN_SUPPRESS_TEST_IF(IsSwiftshader() && !DAWN_PLATFORM_IS(ARM64));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        if (SupportsFeatures({wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable})) {
            return {wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable};
        }
        return {};
    }

    wgpu::ResourceTable MakeResourceTable(uint32_t size) {
        wgpu::ResourceTableDescriptor desc;
        desc.size = size;
        return device.CreateResourceTable(&desc);
    }

    wgpu::PipelineLayout MakePipelineLayoutWithTable(std::vector<wgpu::BindGroupLayout> bgls = {},
                                                     uint32_t immediateSize = 0) {
        wgpu::PipelineLayoutResourceTable plTable;
        plTable.usesResourceTable = true;

        wgpu::PipelineLayoutDescriptor desc{
            .nextInChain = &plTable,
            .bindGroupLayoutCount = bgls.size(),
            .bindGroupLayouts = bgls.data(),
            .immediateSize = immediateSize,
        };

        return device.CreatePipelineLayout(&desc);
    }
};

// Test that creating resource tables doesn't crash in backends.
TEST_P(ResourceTableTests, ResourceTableCreation) {
    // Creating an empty resource table.
    MakeResourceTable(0);

    // Creating a resource table with a few entries.
    MakeResourceTable(36);

    // Creating a resource table with the maximum number of entries.
    MakeResourceTable(kMaxResourceTableSize);
}

// Test that creating pipeline layouts with resources tables doesn't crash in backends.
TEST_P(ResourceTableTests, PipelineLayoutWithResourceTableCreation) {
    // Make layouts with no BGLs with / without immediates.
    MakePipelineLayoutWithTable({}, 0);
    MakePipelineLayoutWithTable({}, 4);

    // Make layouts with one BGL, with / without immediates.
    wgpu::BindGroupLayout testBgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform}});
    MakePipelineLayoutWithTable({testBgl}, 0);
    MakePipelineLayoutWithTable({testBgl}, 4);

    // Make layouts with max BGLs (3 because the resource tables "consumes" one bind group), with /
    // without immediates.
    MakePipelineLayoutWithTable({testBgl, testBgl, testBgl}, 0);
    MakePipelineLayoutWithTable({testBgl, testBgl, testBgl}, 4);
}

// Test that creating pipelines that use resource tables doesn't crash in backends.
TEST_P(ResourceTableTests, ShaderWithResourceTableCreation) {
    wgpu::ComputePipelineDescriptor csDesc;

    // Test compiling a pipeline using only the resource table.
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;
        @compute @workgroup_size(1) fn main() {
            _ = hasResource<texture_2d<f32>>(0);
        }
    )");
    device.CreateComputePipeline(&csDesc);

    // Test compiling a pipeline using the resource table and a bindgroup.
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;
        @group(0) @binding(0) var t0 : texture_2d<f32>;
        @compute @workgroup_size(1) fn main() {
            _ = hasResource<texture_2d<f32>>(0);
            _ = t0;
        }
    )");
    device.CreateComputePipeline(&csDesc);

    // Test compiling a pipeline using the resource table and many bindgroup.
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;
        @group(0) @binding(0) var t0 : texture_2d<f32>;
        @group(1) @binding(0) var t1 : texture_2d<f32>;
        @group(2) @binding(0) var t2 : texture_2d<f32>;
        @compute @workgroup_size(1) fn main() {
            _ = hasResource<texture_2d<f32>>(0);
            _ = t0;
            _ = t1;
            _ = t2;
        }
    )");
    device.CreateComputePipeline(&csDesc);
}

DAWN_INSTANTIATE_TEST(ResourceTableTests, D3D12Backend(), MetalBackend(), VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
