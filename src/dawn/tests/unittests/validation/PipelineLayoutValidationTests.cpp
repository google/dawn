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

#include <vector>

#include "src/dawn/common/Constants.h"
#include "src/dawn/tests/unittests/validation/ValidationTest.h"
#include "src/dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class PipelineLayoutValidationTest : public ValidationTest {};

// Test creating pipeline layout with null bind group layout works when unsafe APIs are allowed.
TEST_F(PipelineLayoutValidationTest, CreateWithNullBindGroupLayout) {
    for (uint32_t nullBGLIndex = 0; nullBGLIndex < 4; ++nullBGLIndex) {
        std::vector<wgpu::BindGroupLayout> bgls(4);
        for (uint32_t i = 0; i < 4; ++i) {
            if (i == nullBGLIndex) {
                continue;
            }
            bgls[i] = utils::MakeBindGroupLayout(
                device, {{0, wgpu::ShaderStage::Compute, wgpu::StorageTextureAccess::WriteOnly,
                          wgpu::TextureFormat::R32Float}});
        }
        utils::MakePipelineLayout(device, bgls);
    }
}

// Test the pipeline layout with null bind group layout must match the corresponding binding in
// shader.
TEST_F(PipelineLayoutValidationTest, ShaderMatchesPipelineLayoutWithNullBindGroupLayout) {
    for (uint32_t nullBGLIndex = 0; nullBGLIndex < 4; ++nullBGLIndex) {
        std::vector<wgpu::BindGroupLayout> bgls(4);
        for (uint32_t i = 0; i < 4; ++i) {
            if (i == nullBGLIndex) {
                continue;
            }
            bgls[i] = utils::MakeBindGroupLayout(
                device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});
        }
        wgpu::PipelineLayout pipelineLayout = utils::MakePipelineLayout(device, bgls);

        for (uint32_t missedGroupIndex = 0; missedGroupIndex < 4; ++missedGroupIndex) {
            std::ostringstream stream;
            for (uint32_t i = 0; i < 4; ++i) {
                if (i != missedGroupIndex) {
                    stream << "@group(" << i << ") @binding(0) var<storage, read_write> outputData"
                           << i << " : u32;\n";
                }
            }
            stream << "@compute @workgroup_size(1, 1) fn main() {\n";
            for (uint32_t i = 0; i < 4; ++i) {
                if (i != missedGroupIndex) {
                    stream << "outputData" << i << " = 1u;\n";
                }
            }
            stream << "};";
            wgpu::ComputePipelineDescriptor computePipelineDescriptor = {};
            computePipelineDescriptor.compute.module =
                utils::CreateShaderModule(device, stream.str());
            computePipelineDescriptor.layout = pipelineLayout;
            if (missedGroupIndex == nullBGLIndex) {
                device.CreateComputePipeline(&computePipelineDescriptor);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&computePipelineDescriptor));
            }
        }
    }
}

// Test the null or empty bind group layout in a pipeline layout should be ignored when we check the
// compatibility between the pipeline layout and the corresponding bind group.
TEST_F(PipelineLayoutValidationTest, BindGroupSlotWithEmptyLayoutIsNotValidated) {
    std::array<wgpu::BindGroupLayout, 2> nullOrEmptyBindGroupLayouts = {
        nullptr, utils::MakeBindGroupLayout(device, {})};

    wgpu::BindGroupLayout nonEmptyBGL = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});
    wgpu::BufferDescriptor bufferDescForNonEmptyBGL = {};
    bufferDescForNonEmptyBGL.size = 4;
    bufferDescForNonEmptyBGL.usage = wgpu::BufferUsage::Storage;
    wgpu::Buffer bufferForNonEmptyBGL = device.CreateBuffer(&bufferDescForNonEmptyBGL);
    wgpu::BindGroup nonEmptyBindGroup =
        utils::MakeBindGroup(device, nonEmptyBGL, {{0, bufferForNonEmptyBGL}});

    for (uint32_t nullBGLIndex = 0; nullBGLIndex < 4; ++nullBGLIndex) {
        for (wgpu::BindGroupLayout nullOrEmptyBindGroupLayout : nullOrEmptyBindGroupLayouts) {
            std::vector<wgpu::BindGroupLayout> bgls(4);
            std::vector<wgpu::BindGroup> bgs(4);

            // Create compute pipeline with null or empty bind group layout and the bind groups.
            // Note that the bind groups are all non-empty.
            for (uint32_t i = 0; i < 4; ++i) {
                if (i == nullBGLIndex) {
                    bgls[i] = nullOrEmptyBindGroupLayout;
                    bgs[i] = nonEmptyBindGroup;
                } else {
                    bgls[i] = utils::MakeBindGroupLayout(
                        device,
                        {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});

                    wgpu::BufferDescriptor bufferDesc = {};
                    bufferDesc.size = 4;
                    bufferDesc.usage = wgpu::BufferUsage::Storage;
                    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);
                    bgs[i] = utils::MakeBindGroup(device, bgls[i], {{0, buffer}});
                }
            }
            wgpu::PipelineLayout pipelineLayout = utils::MakePipelineLayout(device, bgls);

            std::ostringstream stream;
            for (uint32_t i = 0; i < 4; ++i) {
                if (i != nullBGLIndex) {
                    stream << "@group(" << i << ") @binding(0) var<storage, read_write> outputData"
                           << i << " : u32;\n";
                }
            }
            stream << "@compute @workgroup_size(1, 1) fn main() {\n";
            for (uint32_t i = 0; i < 4; ++i) {
                if (i != nullBGLIndex) {
                    stream << "outputData" << i << " = 1u;\n";
                }
            }
            stream << "};";
            wgpu::ComputePipelineDescriptor computePipelineDescriptor = {};
            computePipelineDescriptor.compute.module =
                utils::CreateShaderModule(device, stream.str());
            computePipelineDescriptor.layout = pipelineLayout;

            wgpu::ComputePipeline computePipeline =
                device.CreateComputePipeline(&computePipelineDescriptor);

            // Set pipeline and bind groups. The null or empty bind group layout in the pipeline
            // layout should be ignored in the check of the compatibility between pipeline layout
            // and the bind group when encoding `SetBindGroup()`.
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
            for (uint32_t i = 0; i < 4; ++i) {
                computePass.SetBindGroup(i, bgs[i]);
            }
            computePass.SetPipeline(computePipeline);
            computePass.DispatchWorkgroups(1);
            computePass.End();
            wgpu::CommandBuffer cmdbuf = encoder.Finish();
            device.GetQueue().Submit(1, &cmdbuf);
        }
    }
}

// Test the empty bind group layout returned by calling `getBindGroupLayout()` on a pipeline created
// with `auto` pipeline layout cannot be used to create other pipeline layouts.
TEST_F(PipelineLayoutValidationTest, ReuseEmptyBindGroupLayoutCreatedwithAutoPipelineLayout) {
    // The empty bind group layout comes from a pipeline created with an explicit pipeline layout.
    {
        wgpu::PipelineLayout pipelineLayout = utils::MakePipelineLayout(device, {});
        wgpu::ComputePipelineDescriptor computePipelineDescriptor = {};
        computePipelineDescriptor.compute.module = utils::CreateShaderModule(device, R"(
                @compute @workgroup_size(1, 1) fn main() {})");
        computePipelineDescriptor.layout = pipelineLayout;
        wgpu::ComputePipeline computePipeline =
            device.CreateComputePipeline(&computePipelineDescriptor);

        wgpu::BindGroupLayout emptyBindGroupLayout = computePipeline.GetBindGroupLayout(3);
        std::vector<wgpu::BindGroupLayout> bindGroupLayouts = {{emptyBindGroupLayout}};
        utils::MakePipelineLayout(device, bindGroupLayouts);
    }

    // The empty bind group layout comes from a pipeline created with an 'auto' pipeline layout.
    {
        wgpu::ComputePipelineDescriptor computePipelineDescriptor = {};
        computePipelineDescriptor.compute.module = utils::CreateShaderModule(device, R"(
                @compute @workgroup_size(1, 1) fn main() {})");
        wgpu::ComputePipeline computePipeline =
            device.CreateComputePipeline(&computePipelineDescriptor);

        wgpu::BindGroupLayout emptyBindGroupLayout = computePipeline.GetBindGroupLayout(3);
        std::vector<wgpu::BindGroupLayout> bindGroupLayouts = {{emptyBindGroupLayout}};
        ASSERT_DEVICE_ERROR(utils::MakePipelineLayout(device, bindGroupLayouts));
    }
}

// Check validation against maxBindGroups
TEST_F(PipelineLayoutValidationTest, MaxBindGroups) {
    std::array<wgpu::BindGroupLayout, kMaxBindGroups + 1> bgls;
    bgls.fill(utils::MakeBindGroupLayout(device, {}));

    // Control case: 1 BGL is allowed.
    {
        wgpu::PipelineLayoutDescriptor desc;
        desc.bindGroupLayoutCount = 1;
        desc.bindGroupLayouts = bgls.data();
        device.CreatePipelineLayout(&desc);
    }

    // Success case: 0 bindgroups is allowed.
    {
        wgpu::PipelineLayoutDescriptor desc;
        desc.bindGroupLayoutCount = 0;
        desc.bindGroupLayouts = nullptr;
        device.CreatePipelineLayout(&desc);
    }

    // Success case: maxBindGroups bindgroups are allowed.
    {
        wgpu::PipelineLayoutDescriptor desc;
        desc.bindGroupLayoutCount = kMaxBindGroups;
        desc.bindGroupLayouts = bgls.data();
        device.CreatePipelineLayout(&desc);
    }

    // Error case: maxBindGroups + 1 bindgroups is an error.
    {
        wgpu::PipelineLayoutDescriptor desc;
        desc.bindGroupLayoutCount = kMaxBindGroups + 1;
        desc.bindGroupLayouts = bgls.data();
        ASSERT_DEVICE_ERROR(device.CreatePipelineLayout(&desc));
    }
}

// Test that bindGroupLayoutCount is checked before dereferencing bindGroupLayouts.
TEST_F(PipelineLayoutValidationTest, BGLCountOverLimitsNotAccessed) {
    // This is a test for dawn::native only.
    if (UsesWire()) {
        GTEST_SKIP();
    }

    // Check that bindGroupLayouts is not accessed if their count is higher that the bind group
    // limit.
    {
        wgpu::PipelineLayoutDescriptor desc;
        desc.bindGroupLayoutCount = kMaxBindGroups + 1;
        desc.bindGroupLayouts = nullptr;
        ASSERT_DEVICE_ERROR(device.CreatePipelineLayout(&desc));
    }

#if DAWN_PLATFORM_IS(64_BIT)
    // Check that a bindGroupLayouts that would end up being BindGroupIndex{1} still causes a
    // validation error.
    {
        wgpu::PipelineLayoutDescriptor desc;
        desc.bindGroupLayoutCount = 0x1'000'0001;
        desc.bindGroupLayouts = nullptr;
        ASSERT_DEVICE_ERROR(device.CreatePipelineLayout(&desc));
    }
#endif
}

}  // anonymous namespace
}  // namespace dawn
