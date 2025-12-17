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

#include <utility>
#include <vector>

#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/ScopedIgnoreValidationErrors.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class ResourceTableValidationTest : public ValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable};
    }

    wgpu::ResourceTable MakeResourceTable(uint32_t size) {
        wgpu::ResourceTableDescriptor desc;
        desc.size = size;
        return device.CreateResourceTable(&desc);
    }

    wgpu::ResourceTable MakeErrorResourceTable(uint32_t size) {
        wgpu::RenderPassMaxDrawCount maxDraw;
        maxDraw.maxDrawCount = 1000;
        wgpu::ResourceTableDescriptor desc{
            .nextInChain = &maxDraw,
            .size = size,
        };

        wgpu::ResourceTable table;
        ASSERT_DEVICE_ERROR(table = device.CreateResourceTable(&desc));
        return table;
    }

    enum class Mutator : uint8_t {
        Update,
        InsertBinding,
    };
    void TestMutator(Mutator mutator, const wgpu::BindingResource* resource, bool success) {
        wgpu::ResourceTable table = MakeResourceTable(1);

        switch (mutator) {
            case Mutator::Update: {
                wgpu::Status status;
                if (success) {
                    status = table.Update(0, resource);
                } else {
                    ASSERT_DEVICE_ERROR(status = table.Update(0, resource));
                }
                EXPECT_EQ(status, wgpu::Status::Success);
                break;
            }

            case Mutator::InsertBinding: {
                uint32_t slot = wgpu::kInvalidBinding;
                if (success) {
                    slot = table.InsertBinding(resource);
                } else {
                    ASSERT_DEVICE_ERROR(slot = table.InsertBinding(resource));
                }
                EXPECT_EQ(slot, 0u);
                break;
            }
        }
    }

    // Helper to make sure that the resource table is marked as used. Even if internally Dawn
    // doesn't track this, it makes tests more clearly correct.
    void UseResourceTableInSubmit(wgpu::ResourceTable table) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.SetResourceTable(table);
        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);
    }
};

class ResourceTableValidationTestDisabled : public ValidationTest {
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override { return {}; }
};

// Test that validates that the feature must be enabled
TEST_F(ResourceTableValidationTestDisabled, FeatureNotEnabled) {
    wgpu::ResourceTableDescriptor descriptor;
    ASSERT_DEVICE_ERROR(device.CreateResourceTable(&descriptor));
}

// Test that setting invalid size is an error
TEST_F(ResourceTableValidationTest, InvalidSize) {
    wgpu::ResourceTableDescriptor descriptor;

    // Size 0 is valid
    descriptor.size = 0u;
    device.CreateResourceTable(&descriptor);

    // Size of 1 is valid
    descriptor.size = 1u;
    device.CreateResourceTable(&descriptor);

    // Size of maxResourceTableSize is valid
    descriptor.size = kMaxResourceTableSize;
    device.CreateResourceTable(&descriptor);

    // Size > limits is invalid
    descriptor.size = kMaxResourceTableSize + 1u;
    ASSERT_DEVICE_ERROR(device.CreateResourceTable(&descriptor));
}

// Test that setting nextInChain to anything is an error
TEST_F(ResourceTableValidationTest, NextInChain) {
    // Control case, nextInChain = nullptr is valid.
    {
        wgpu::ResourceTableDescriptor descriptor{
            .nextInChain = nullptr,
            .size = 3,
        };
        device.CreateResourceTable(&descriptor);
    }

    // Control case, nextInChain = non null is invalid.
    {
        wgpu::RenderPassMaxDrawCount maxDraw;
        maxDraw.maxDrawCount = 1000;
        wgpu::ResourceTableDescriptor descriptor{
            .nextInChain = &maxDraw,
            .size = 3,
        };
        ASSERT_DEVICE_ERROR(device.CreateResourceTable(&descriptor));
    }
}

// Test the Destroy call on a ResourceTable
TEST_F(ResourceTableValidationTest, Destroy) {
    // TODO(435317394): Implemented bindless in the wire.
    if (UsesWire()) {
        GTEST_SKIP();
    }

    wgpu::ResourceTableDescriptor descriptor;
    descriptor.size = 1u;
    wgpu::ResourceTable resourceTable = device.CreateResourceTable(&descriptor);

    // Calling destroy is valid
    resourceTable.Destroy();

    // Calling it multiple times is valid
    resourceTable.Destroy();
}

// Control case where enabling use of a resource table with the feature enabled is valid.
TEST_F(ResourceTableValidationTest, PipelineLayoutCreation_SuccessWithFeatureEnabled) {
    wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
    pipelineLayoutDescriptor.bindGroupLayoutCount = 0;
    wgpu::PipelineLayoutResourceTable resourceTable;
    resourceTable.usesResourceTable = true;
    pipelineLayoutDescriptor.nextInChain = &resourceTable;
    device.CreatePipelineLayout(&pipelineLayoutDescriptor);
}

// Error case where enabling use of a resource table with the feature disabled is an error.
TEST_F(ResourceTableValidationTestDisabled, PipelineLayoutCreation_FailureWithFeatureDisabled) {
    wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
    pipelineLayoutDescriptor.bindGroupLayoutCount = 0;
    wgpu::PipelineLayoutResourceTable resourceTable;
    pipelineLayoutDescriptor.nextInChain = &resourceTable;

    // Failure case
    resourceTable.usesResourceTable = true;
    ASSERT_DEVICE_ERROR(device.CreatePipelineLayout(&pipelineLayoutDescriptor));

    // Success case
    resourceTable.usesResourceTable = false;
    device.CreatePipelineLayout(&pipelineLayoutDescriptor);
}

// Error case where compiling a shader using the resource table with the extension disabled is an
// error.
TEST_F(ResourceTableValidationTestDisabled, WGSLEnableNotAllowed) {
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;
        @compute @workgroup_size(1) fn main() {
            _ = hasResource<texture_2d<f32>>(0);
        }
    )"));
}

// Test that a shader using a resource table requires a layout with one.
TEST_F(ResourceTableValidationTest, PipelineCreation_ShaderRequiresLayoutWithResourceTable) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;
        @compute @workgroup_size(1) fn main() {
            _ = hasResource<texture_2d<f32>>(0);
        }
    )");

    wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
    pipelineLayoutDescriptor.bindGroupLayoutCount = 0;
    wgpu::PipelineLayoutResourceTable resourceTable;
    pipelineLayoutDescriptor.nextInChain = &resourceTable;

    // Success case, the layout uses a resource table
    resourceTable.usesResourceTable = true;
    csDesc.layout = device.CreatePipelineLayout(&pipelineLayoutDescriptor);
    device.CreateComputePipeline(&csDesc);

    // Failure case, the layout does not use a resource table
    resourceTable.usesResourceTable = false;
    csDesc.layout = device.CreatePipelineLayout(&pipelineLayoutDescriptor);
    ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&csDesc));
}

// Test that it is valid to have a layout specifying a resource table with a shader that
// doesn't have one.
TEST_F(ResourceTableValidationTest, PipelineCreation_ShaderNoResourceTableWithLayoutThatHasOne) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(1) fn main() {
        }
    )");

    wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
    pipelineLayoutDescriptor.bindGroupLayoutCount = 0;
    wgpu::PipelineLayoutResourceTable resourceTable;
    pipelineLayoutDescriptor.nextInChain = &resourceTable;

    resourceTable.usesResourceTable = true;
    csDesc.layout = device.CreatePipelineLayout(&pipelineLayoutDescriptor);
    device.CreateComputePipeline(&csDesc);
}

// Test that an defaulted pipeline layout with a shader that uses a resource table has a
// PipelineLayoutResourceTable with usesResourceTable == true.
TEST_F(ResourceTableValidationTest, PipelineCreation_DefaultedLayoutWithResourceTable) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;
        @compute @workgroup_size(1) fn main() {
            _ = hasResource<texture_2d<f32>>(0);
        }
    )");

    csDesc.layout = nullptr;  // Auto
    device.CreateComputePipeline(&csDesc);
}

// Test that an defaulted pipeline layout with a multi-stage shader where only one stage uses a
// resource table has a PipelineLayoutResourceTable with usesResourceTable == true.
TEST_F(ResourceTableValidationTest, PipelineCreation_OneShaderDefaultedLayoutWithResourceTable) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0.5, 0.5);
        }
        @compute @workgroup_size(1) fn compute_main() {
            _ = hasResource<texture_2d<f32>>(0);
        }
        @fragment fn fs() -> @location(0) vec4f {
            return vec4f(1.0, 0.0, 0.0, 1.0);
        }
    )");

    csDesc.layout = nullptr;  // Auto
    device.CreateComputePipeline(&csDesc);
}

// Test that a resource table uses up a BindGroupLayout slot
TEST_F(ResourceTableValidationTest, PipelineLayoutCreation_ResourceTableUsesBindGroupLayoutSlot) {
    // Control case: max bgls, no resource table
    {
        std::vector bgLayout(kMaxBindGroups, utils::MakeBindGroupLayout(device, {}));
        wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
        pipelineLayoutDescriptor.bindGroupLayoutCount = bgLayout.size();
        pipelineLayoutDescriptor.bindGroupLayouts = bgLayout.data();
        device.CreatePipelineLayout(&pipelineLayoutDescriptor);
    }

    // Failure case: not enough room for bgls and a resource table
    {
        std::vector bgLayout(kMaxBindGroups, utils::MakeBindGroupLayout(device, {}));
        wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
        pipelineLayoutDescriptor.bindGroupLayoutCount = bgLayout.size();
        pipelineLayoutDescriptor.bindGroupLayouts = bgLayout.data();
        wgpu::PipelineLayoutResourceTable resourceTable;
        resourceTable.usesResourceTable = true;
        pipelineLayoutDescriptor.nextInChain = &resourceTable;
        ASSERT_DEVICE_ERROR(device.CreatePipelineLayout(&pipelineLayoutDescriptor));
    }

    // Success case: enough room for bgls and a resource table
    {
        std::vector bgLayout(kMaxBindGroups - 1, utils::MakeBindGroupLayout(device, {}));
        wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
        pipelineLayoutDescriptor.bindGroupLayoutCount = bgLayout.size();
        pipelineLayoutDescriptor.bindGroupLayouts = bgLayout.data();
        wgpu::PipelineLayoutResourceTable resourceTable;
        resourceTable.usesResourceTable = true;
        pipelineLayoutDescriptor.nextInChain = &resourceTable;
        device.CreatePipelineLayout(&pipelineLayoutDescriptor);
    }
}

// Test that a resource table uses up a storage buffer binding
TEST_F(ResourceTableValidationTest, PipelineLayoutCreation_ResourceTableUsesOneStorageBuffer) {
    const uint32_t maxStorageBuffers = deviceLimits.maxStorageBuffersPerShaderStage;
    std::vector<wgpu::BindGroupLayoutEntry> storageBufferEntries(maxStorageBuffers);
    for (size_t i = 0; i < storageBufferEntries.size(); i++) {
        storageBufferEntries[i].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
        storageBufferEntries[i].visibility =
            wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Compute;
        storageBufferEntries[i].binding = i;
    }

    // Success case: exactly maxStorageBuffers are used (1 for the resource table, max - 1 for BGL
    // entries).
    {
        wgpu::BindGroupLayoutDescriptor bglDesc = {
            .entryCount = maxStorageBuffers - 1,
            .entries = storageBufferEntries.data(),
        };
        wgpu::BindGroupLayout bgl = device.CreateBindGroupLayout(&bglDesc);

        wgpu::PipelineLayoutResourceTable resourceTable;
        resourceTable.usesResourceTable = true;
        wgpu::PipelineLayoutDescriptor plDesc = {
            .nextInChain = &resourceTable,
            .bindGroupLayoutCount = 1,
            .bindGroupLayouts = &bgl,
        };
        device.CreatePipelineLayout(&plDesc);
    }

    // Error case: the resource table additional storage buffer make the layout go over the limit.
    {
        wgpu::BindGroupLayoutDescriptor bglDesc = {
            .entryCount = maxStorageBuffers,
            .entries = storageBufferEntries.data(),
        };
        wgpu::BindGroupLayout bgl = device.CreateBindGroupLayout(&bglDesc);

        wgpu::PipelineLayoutResourceTable resourceTable;
        resourceTable.usesResourceTable = true;
        wgpu::PipelineLayoutDescriptor plDesc = {
            .nextInChain = &resourceTable,
            .bindGroupLayoutCount = 1,
            .bindGroupLayouts = &bgl,
        };
        ASSERT_DEVICE_ERROR(device.CreatePipelineLayout(&plDesc));
    }
}

// Test that an defaulted pipeline layout with a resource table uses up a BindGroupLayout slot
TEST_F(ResourceTableValidationTest,
       PipelineCreation_DefaultedLayoutWithResourceTableUsesBindGroupLayoutSlot) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.layout = nullptr;  // Auto

    // Control case: max bgls, no resource table
    {
        csDesc.compute.module = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_resource_table;
            @group(0) @binding(0) var<uniform> a : u32;
            @group(1) @binding(0) var<uniform> b : u32;
            @group(2) @binding(0) var<uniform> c : u32;
            @group(3) @binding(0) var<uniform> d : u32;
            @compute @workgroup_size(1) fn main() {
                // _ = hasResource<texture_2d<f32>>(0);
                _ = a;
                _ = b;
                _ = c;
                _ = d;
            }
        )");
        device.CreateComputePipeline(&csDesc);
    }

    // Failure case: not enough room for bgls and a resource table
    {
        csDesc.compute.module = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_resource_table;
            @group(0) @binding(0) var<uniform> a : u32;
            @group(1) @binding(0) var<uniform> b : u32;
            @group(2) @binding(0) var<uniform> c : u32;
            @group(3) @binding(0) var<uniform> d : u32;
            @compute @workgroup_size(1) fn main() {
                _ = hasResource<texture_2d<f32>>(0);
                _ = a;
                _ = b;
                _ = c;
                _ = d;
            }
        )");
        ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&csDesc));
    }

    // Success case: enough room for bgls and a resource table
    {
        csDesc.compute.module = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_resource_table;
            @group(0) @binding(0) var<uniform> a : u32;
            @group(1) @binding(0) var<uniform> b : u32;
            @group(2) @binding(0) var<uniform> c : u32;
            @compute @workgroup_size(1) fn main() {
                _ = hasResource<texture_2d<f32>>(0);
                _ = a;
                _ = b;
                _ = c;
            }
        )");
        device.CreateComputePipeline(&csDesc);
    }
}

// Test that GetBindGroupLayout is valid for one less BGL if resource tables are used.
TEST_F(ResourceTableValidationTest, GetBindGroupLayoutValidForOneLessIndex) {
    // Default behavior case: GetBGL is valid until kMaxBindGroups - 1 when no resource table is
    // used.
    {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.layout = nullptr;
        csDesc.compute.module = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_resource_table;
            @compute @workgroup_size(1) fn main() {
            }
        )");
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

        pipeline.GetBindGroupLayout(kMaxBindGroups - 1);
        ASSERT_DEVICE_ERROR(pipeline.GetBindGroupLayout(kMaxBindGroups));
    }

    // Resource table case: GetBGL is valid until kMaxBindGroups - 2.
    {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.layout = nullptr;
        csDesc.compute.module = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_resource_table;
            @compute @workgroup_size(1) fn main() {
                _ = hasResource<texture_2d<f32>>(0);
            }
        )");
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

        pipeline.GetBindGroupLayout(kMaxBindGroups - 2);
        ASSERT_DEVICE_ERROR(pipeline.GetBindGroupLayout(kMaxBindGroups - 1));
    }
}

// Tests calling CommandEncoder::SetResourceTable
TEST_F(ResourceTableValidationTest, CommandEncoder_SetResourceTable) {
    // Failure case: invalid encoder state
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.Finish();
        ASSERT_DEVICE_ERROR(encoder.SetResourceTable(nullptr));
    }

    // Failure case: invalid resource table
    {
        wgpu::ResourceTableDescriptor descriptor;
        descriptor.size = kMaxResourceTableSize + 1u;  // Invalid size
        wgpu::ResourceTable resourceTable;
        ASSERT_DEVICE_ERROR(resourceTable = device.CreateResourceTable(&descriptor));

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.SetResourceTable(resourceTable);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Success case: valid resource table
    {
        wgpu::ResourceTableDescriptor descriptor;
        descriptor.size = 1;
        wgpu::ResourceTable resourceTable = device.CreateResourceTable(&descriptor);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.SetResourceTable(resourceTable);
        encoder.Finish();
    }

    // Success case: null resource table
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.SetResourceTable(nullptr);
        encoder.Finish();
    }
}

// Tests calling CommandEncoder::SetResourceTable when the feature is disabled
TEST_F(ResourceTableValidationTestDisabled, CommandEncoder_SetResourceTable) {
    // Failure case: feature is disabled
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.SetResourceTable(nullptr);
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Tests that the resource table can be used in submit
TEST_F(ResourceTableValidationTest, Submit_CanUseInSubmit) {
    // TODO(435317394): Implemented bindless in the wire.
    if (UsesWire()) {
        GTEST_SKIP();
    }

    // Success case: resource table can be used in submit
    {
        wgpu::ResourceTableDescriptor descriptor;
        descriptor.size = 1u;
        wgpu::ResourceTable resourceTable = device.CreateResourceTable(&descriptor);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.SetResourceTable(resourceTable);
        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);
    }

    // Failure case: resource table has been destroyed
    {
        wgpu::ResourceTableDescriptor descriptor;
        descriptor.size = 1u;
        wgpu::ResourceTable resourceTable = device.CreateResourceTable(&descriptor);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.SetResourceTable(resourceTable);
        wgpu::CommandBuffer commands = encoder.Finish();
        resourceTable.Destroy();  // Destroy it
        ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
    }

    // Failure case: one of multiple resource tables has been destroyed
    {
        wgpu::ResourceTableDescriptor descriptor;
        descriptor.size = 1u;
        wgpu::ResourceTable resourceTable1 = device.CreateResourceTable(&descriptor);
        wgpu::ResourceTable resourceTable2 = device.CreateResourceTable(&descriptor);
        wgpu::ResourceTable resourceTable3 = device.CreateResourceTable(&descriptor);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.SetResourceTable(resourceTable1);
        encoder.SetResourceTable(resourceTable2);
        encoder.SetResourceTable(resourceTable3);
        wgpu::CommandBuffer commands = encoder.Finish();
        resourceTable2.Destroy();  // Destroy one
        ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
    }

    // Failure case: resource table must still be valid if set, then nullptr is set
    {
        wgpu::ResourceTableDescriptor descriptor;
        descriptor.size = 1u;
        wgpu::ResourceTable resourceTable = device.CreateResourceTable(&descriptor);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.SetResourceTable(resourceTable);
        encoder.SetResourceTable(nullptr);  // Clear it
        wgpu::CommandBuffer commands = encoder.Finish();
        resourceTable.Destroy();  // Destroy it
        ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
    }
}

// Tests that the resource table can be used in dispatch
TEST_F(ResourceTableValidationTest, Submit_DispatchRequiresResourceTable) {
    for (bool defaulted : {true, false}) {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_resource_table;
            @compute @workgroup_size(1) fn main() {
                _ = hasResource<texture_2d<f32>>(0);
            }
        )");

        wgpu::ComputePipeline pipeline;
        if (defaulted) {
            csDesc.layout = nullptr;
            pipeline = device.CreateComputePipeline(&csDesc);
        } else {
            wgpu::PipelineLayoutResourceTable plResourceTable;
            plResourceTable.usesResourceTable = true;

            wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
            pipelineLayoutDescriptor.bindGroupLayoutCount = 0;
            pipelineLayoutDescriptor.nextInChain = &plResourceTable;

            csDesc.layout = device.CreatePipelineLayout(&pipelineLayoutDescriptor);
            pipeline = device.CreateComputePipeline(&csDesc);
        }

        wgpu::ResourceTableDescriptor descriptor;
        descriptor.size = 1u;
        wgpu::ResourceTable resourceTable = device.CreateResourceTable(&descriptor);
        wgpu::ResourceTable resourceTable2 = device.CreateResourceTable(&descriptor);

        // Success case: `usesResourceTable` is enabled, and one has been set on the encoder
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.SetResourceTable(resourceTable);
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(pipeline);
            pass.DispatchWorkgroups(1);
            pass.End();
            wgpu::CommandBuffer commands = encoder.Finish();
            device.GetQueue().Submit(1, &commands);
        }

        // Failure case: `usesResourceTable` is enabled, but none has been set on the encoder
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(pipeline);
            pass.DispatchWorkgroups(1);
            pass.End();
            ASSERT_DEVICE_ERROR(wgpu::CommandBuffer commands = encoder.Finish());
        }

        // Failure case: `usesResourceTable` is enabled, one then nullptr set on the encoder
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.SetResourceTable(resourceTable);  // Set a valid one
            encoder.SetResourceTable(nullptr);        // Then clear it
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(pipeline);
            pass.DispatchWorkgroups(1);
            pass.End();
            ASSERT_DEVICE_ERROR(wgpu::CommandBuffer commands = encoder.Finish());
        }

        // Success case: `usesResourceTable` is enabled, one then nullptr then another set on the
        // encoder
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.SetResourceTable(resourceTable);   // Set a valid one
            encoder.SetResourceTable(nullptr);         // Then clear it
            encoder.SetResourceTable(resourceTable2);  // Then set another valid one
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(pipeline);
            pass.DispatchWorkgroups(1);
            pass.End();
            wgpu::CommandBuffer commands = encoder.Finish();
            device.GetQueue().Submit(1, &commands);
        }
    }
}

// Tests that the resource table can be used in draw
TEST_F(ResourceTableValidationTest, Submit_DrawRequiresResourceTable) {
    for (bool defaulted : {true, false}) {
        utils::ComboRenderPipelineDescriptor pDesc;
        pDesc.vertex.module = utils::CreateShaderModule(device, R"(
            @vertex fn vs() -> @builtin(position) vec4f {
                return vec4f();
            }
        )");
        pDesc.cFragment.module = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_resource_table;
            @fragment fn fs() -> @location(0) vec4f {
                _ = hasResource<texture_2d<f32>>(0);
                return vec4f();
            }
        )");

        wgpu::RenderPipeline pipeline;
        if (defaulted) {
            pDesc.layout = nullptr;
            pipeline = device.CreateRenderPipeline(&pDesc);
        } else {
            wgpu::PipelineLayoutResourceTable plResourceTable;
            plResourceTable.usesResourceTable = true;

            wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
            pipelineLayoutDescriptor.bindGroupLayoutCount = 0;
            pipelineLayoutDescriptor.nextInChain = &plResourceTable;

            pDesc.layout = device.CreatePipelineLayout(&pipelineLayoutDescriptor);
            pipeline = device.CreateRenderPipeline(&pDesc);
        }

        wgpu::ResourceTableDescriptor descriptor;
        descriptor.size = 1u;
        wgpu::ResourceTable resourceTable = device.CreateResourceTable(&descriptor);
        wgpu::ResourceTable resourceTable2 = device.CreateResourceTable(&descriptor);
        auto rp = utils::CreateBasicRenderPass(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);

        // Success case: `usesResourceTable` is enabled, and one has been set on the encoder
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.SetResourceTable(resourceTable);
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.Draw(1);
            pass.End();
            wgpu::CommandBuffer commands = encoder.Finish();
            device.GetQueue().Submit(1, &commands);
        }

        // Failure case: `usesResourceTable` is enabled, but none has been set on the encoder
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.Draw(1);
            pass.End();
            ASSERT_DEVICE_ERROR(wgpu::CommandBuffer commands = encoder.Finish());
        }

        // Failure case: `usesResourceTable` is enabled, one then nullptr set on the encoder
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.SetResourceTable(resourceTable);  // Set a valid one
            encoder.SetResourceTable(nullptr);        // Then clear it
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.Draw(1);
            pass.End();
            ASSERT_DEVICE_ERROR(wgpu::CommandBuffer commands = encoder.Finish());
        }

        // Success case: `usesResourceTable` is enabled, one then nullptr then another set on the
        // encoder
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.SetResourceTable(resourceTable);   // Set a valid one
            encoder.SetResourceTable(nullptr);         // Then clear it
            encoder.SetResourceTable(resourceTable2);  // Then set another valid one
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.Draw(1);
            pass.End();
            wgpu::CommandBuffer commands = encoder.Finish();
            device.GetQueue().Submit(1, &commands);
        }
    }
}

// Test that pinning / unpinning is valid for a simple case. This is a control for the test that
// errors are produced when the feature is not enabled.
TEST_F(ResourceTableValidationTest, PinUnpinTextureSuccess) {
    wgpu::TextureDescriptor desc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&desc);

    tex.Pin(wgpu::TextureUsage::TextureBinding);
    tex.Unpin();
}

// Test that calling pin/unpin is an error when the feature is not enabled.
TEST_F(ResourceTableValidationTestDisabled, PinUnpinTextureSuccess) {
    wgpu::TextureDescriptor desc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&desc);

    ASSERT_DEVICE_ERROR(tex.Pin(wgpu::TextureUsage::TextureBinding));
    ASSERT_DEVICE_ERROR(tex.Unpin());
}

// Test the validation of the usage parameter of Pin.
TEST_F(ResourceTableValidationTest, PinUnpinTextureUsageConstraint) {
    wgpu::TextureDescriptor desc{
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };

    desc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc |
                 wgpu::TextureUsage::StorageBinding;
    wgpu::Texture testTexture = device.CreateTexture(&desc);

    desc.usage = wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture renderOnlyTexture = device.CreateTexture(&desc);

    // Control case, pinning the sampled texture to TextureBinding is valid.
    testTexture.Pin(wgpu::TextureUsage::TextureBinding);

    // Error case, pinning to a usage not in the texture is invalid.
    ASSERT_DEVICE_ERROR(renderOnlyTexture.Pin(wgpu::TextureUsage::TextureBinding));

    // Error case, pinning to an invalid usage is invalid.
    ASSERT_DEVICE_ERROR(testTexture.Pin(static_cast<wgpu::TextureUsage>(0x8000'0000)));

    // Error case, pinning must be to a shader usage.
    ASSERT_DEVICE_ERROR(testTexture.Pin(wgpu::TextureUsage::CopySrc));

    // Error case, pinning must be to a shader usage.
    // TODO(https://crbug.com/435317394): Lift this constraint and allow other shader usages.
    ASSERT_DEVICE_ERROR(testTexture.Pin(wgpu::TextureUsage::StorageBinding));
}

// Test that pinning / unpinning don't need to be balanced.
TEST_F(ResourceTableValidationTest, PinUnpinUnbalancedIsValid) {
    wgpu::TextureDescriptor desc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&desc);

    // Pinning right after creation is valid.
    tex.Unpin();

    // Pinning twice is valid.
    tex.Pin(wgpu::TextureUsage::TextureBinding);
    // TODO(https://crbug.com/435317394): Use a different usage here when another is valid.
    tex.Pin(wgpu::TextureUsage::TextureBinding);

    // Unpinning twice (plus one more to make sure we are unbalanced) is valid.
    tex.Unpin();
    tex.Unpin();
    tex.Unpin();
}

// Test that pinning is not allowed on a destroyed texture.
TEST_F(ResourceTableValidationTest, PinDestroyedTextureInvalid) {
    wgpu::TextureDescriptor desc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&desc);

    // Success case, pinning before Destroy() is valid.
    tex.Pin(wgpu::TextureUsage::TextureBinding);
    tex.Unpin();

    // Error case, pinning a destroyed texture is not allowed.
    tex.Destroy();
    ASSERT_DEVICE_ERROR(tex.Pin(wgpu::TextureUsage::TextureBinding));
}

enum class TestPinState { Default, Pinned, Unpinned };
std::array<TestPinState, 3> kAllTestPinStates = {TestPinState::Default, TestPinState::Pinned,
                                                 TestPinState::Unpinned};
wgpu::Texture CreateTextureWithPinState(const wgpu::Device& device,
                                        TestPinState pin,
                                        wgpu::TextureUsage usage) {
    wgpu::TextureDescriptor desc{
        .usage = usage,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&desc);

    switch (pin) {
        case TestPinState::Default:
            break;
        case TestPinState::Pinned:
            tex.Pin(wgpu::TextureUsage::TextureBinding);
            break;
        case TestPinState::Unpinned:
            tex.Pin(wgpu::TextureUsage::TextureBinding);
            tex.Unpin();
            break;
    }

    return tex;
}

// Test that pinning prevents usage in WriteTexture
TEST_F(ResourceTableValidationTest, PinValidationUsageWriteTexture) {
    for (auto pin : kAllTestPinStates) {
        wgpu::Texture tex = CreateTextureWithPinState(
            device, pin, wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst);

        wgpu::TexelCopyTextureInfo dst = {
            .texture = tex,
        };
        wgpu::TexelCopyBufferLayout dataLayout = {};
        wgpu::Extent3D copySize = {0, 0, 0};

        if (pin == TestPinState::Pinned) {
            ASSERT_DEVICE_ERROR(
                device.GetQueue().WriteTexture(&dst, nullptr, 0, &dataLayout, &copySize));
        } else {
            device.GetQueue().WriteTexture(&dst, nullptr, 0, &dataLayout, &copySize);
        }
    }
}

// Test that pinning prevents usage in an encoder copy command
TEST_F(ResourceTableValidationTest, PinValidationUsageEncoderCopy) {
    wgpu::TextureDescriptor desc{
        .usage = wgpu::TextureUsage::CopyDst,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture texDst = device.CreateTexture(&desc);

    for (auto pin : kAllTestPinStates) {
        wgpu::Texture tex = CreateTextureWithPinState(
            device, pin, wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc);

        wgpu::TexelCopyTextureInfo src = {
            .texture = tex,
        };
        wgpu::TexelCopyTextureInfo dst = {
            .texture = texDst,
        };
        wgpu::Extent3D copySize = {0, 0, 0};

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&src, &dst, &copySize);
        wgpu::CommandBuffer commands = encoder.Finish();

        if (pin == TestPinState::Pinned) {
            ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
        } else {
            device.GetQueue().Submit(1, &commands);
        }
    }
}

// Test that pinning prevents usage in a dispatch if it is not the pinned usage.
TEST_F(ResourceTableValidationTest, PinValidationUsageDispatch) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var t_sampled : texture_2d<f32>;
        @compute @workgroup_size(1) fn sample() {
            _ = t_sampled;
        }

        @group(0) @binding(0) var t_ro_storage : texture_storage_2d<r32float, read>;
        @compute @workgroup_size(1) fn ro_storage() {
            _ = t_ro_storage;
        }
    )");

    csDesc.compute.entryPoint = "sample";
    wgpu::ComputePipeline samplePipeline = device.CreateComputePipeline(&csDesc);
    csDesc.compute.entryPoint = "ro_storage";
    wgpu::ComputePipeline storagePipeline = device.CreateComputePipeline(&csDesc);

    for (auto pin : kAllTestPinStates) {
        wgpu::Texture tex = CreateTextureWithPinState(
            device, pin, wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding);

        for (bool sample : {false, true}) {
            wgpu::ComputePipeline pipeline = sample ? samplePipeline : storagePipeline;
            wgpu::BindGroup bg = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                      {
                                                          {0, tex.CreateView()},
                                                      });

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bg);
            pass.DispatchWorkgroups(1);
            pass.End();
            wgpu::CommandBuffer commands = encoder.Finish();

            if (pin == TestPinState::Pinned && !sample) {
                ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
            } else {
                device.GetQueue().Submit(1, &commands);
            }
        }
    }
}

// Test that pinning prevents usage in a render pass if it is not the pinned usage.
TEST_F(ResourceTableValidationTest, PinValidationUsageRenderPass) {
    wgpu::BindGroupLayout sampleLayout = utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::UnfilterableFloat},
                });
    wgpu::BindGroupLayout storageLayout = utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Fragment, wgpu::StorageTextureAccess::ReadOnly,
                     wgpu::TextureFormat::R32Float},
                });

    for (auto pin : kAllTestPinStates) {
        wgpu::Texture tex = CreateTextureWithPinState(
            device, pin, wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding);

        for (bool sample : {false, true}) {
            wgpu::BindGroupLayout bgl = sample ? sampleLayout : storageLayout;
            wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl,
                                                      {
                                                          {0, tex.CreateView()},
                                                      });

            utils::BasicRenderPass rp = utils::CreateBasicRenderPass(device, 1, 1);

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass.SetBindGroup(0, bg);
            pass.End();
            wgpu::CommandBuffer commands = encoder.Finish();

            if (pin == TestPinState::Pinned && !sample) {
                ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
            } else {
                device.GetQueue().Submit(1, &commands);
            }
        }
    }
}

// Checks that only texture views are allowed as resources in mutators for SamplingResourceTable.
// TODO(https://issues.chromium.org/435317394): Support samplers in SamplingResourceTable.
TEST_F(ResourceTableValidationTest, MutatorBindingKindValidation) {
    // TODO(435317394): Implemented bindless in the wire.
    if (UsesWire()) {
        GTEST_SKIP();
    }

    // Create the texture to put in the table.
    wgpu::TextureDescriptor tDesc = {
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::Texture texture = device.CreateTexture(&tDesc);

    // Create the buffer to put in the table.
    wgpu::BufferDescriptor bDesc{
        .usage = wgpu::BufferUsage::Storage,
        .size = 4,
    };
    wgpu::Buffer buffer = device.CreateBuffer(&bDesc);

    // Create the sampler to put in the table.
    wgpu::Sampler sampler = device.CreateSampler();

    for (auto mutator : {Mutator::Update, Mutator::InsertBinding}) {
        // Control case: putting only a texture is valid.
        {
            wgpu::BindingResource resource = {.textureView = texture.CreateView()};
            TestMutator(mutator, &resource, true);
        }

        // Error case: a buffer is an error.
        {
            wgpu::BindingResource resource = {.buffer = buffer};
            TestMutator(mutator, &resource, false);
        }

        // Error case: a sampler is an error.
        // TODO(https://issues.chromium.org/435317394): Support samplers in SamplingResourceTable.
        {
            wgpu::BindingResource resource = {.sampler = sampler};
            TestMutator(mutator, &resource, false);
        }

        // Error case: both a sampler and a texture at the same time is an error.
        {
            wgpu::BindingResource resource = {.sampler = sampler,
                                              .textureView = texture.CreateView()};
            TestMutator(mutator, &resource, false);
        }
    }
}

// Check that the view must have only the TextureBinding usage for SamplingResourceTable.
// TODO(https://issues.chromium.org/435317394): Support storage textures in FullResourceTable
// TODO(https://issues.chromium.org/382544164): Support texel buffers in FullResourceTable
TEST_F(ResourceTableValidationTest, MutatorTextureViewMustBeOnlyTextureBinding) {
    // TODO(435317394): Implemented bindless in the wire.
    if (UsesWire()) {
        GTEST_SKIP();
    }

    // Create the texture to put in the table.
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding |
                 wgpu::TextureUsage::StorageBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Uint,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    for (auto mutator : {Mutator::Update, Mutator::InsertBinding}) {
        // Control case: limiting the usage to TextureBinding is valid.
        {
            wgpu::TextureViewDescriptor vDesc{
                .usage = wgpu::TextureUsage::TextureBinding,
            };
            wgpu::BindingResource resource = {.textureView = tex.CreateView(&vDesc)};
            TestMutator(mutator, &resource, true);
        }

        // Error case: having unrelated usages in the view is not allowed. RenderAttachment case.
        {
            wgpu::TextureViewDescriptor vDesc{
                .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment,
            };
            wgpu::BindingResource resource = {.textureView = tex.CreateView(&vDesc)};
            TestMutator(mutator, &resource, false);
        }

        // Error case: having unrelated usages in the view is not allowed. StorageBinding case.
        {
            wgpu::TextureViewDescriptor vDesc{
                .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding,
            };
            wgpu::BindingResource resource = {.textureView = tex.CreateView(&vDesc)};
            TestMutator(mutator, &resource, false);
        }

        // Error case: the defaulted texture usages don't contain TextureBinding.
        {
            wgpu::TextureDescriptor tDesc2 = tDesc;
            tDesc2.usage = wgpu::TextureUsage::CopyDst;
            wgpu::BindingResource resource = {.textureView =
                                                  device.CreateTexture(&tDesc2).CreateView()};
            TestMutator(mutator, &resource, false);
        }
    }
}

// Check that the texture view must have a single aspect for mutators.
TEST_F(ResourceTableValidationTest, MutatorTextureViewMustBeSingleAspect) {
    // TODO(435317394): Implemented bindless in the wire.
    if (UsesWire()) {
        GTEST_SKIP();
    }

    // Create the texture to put in the table.
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::Depth24PlusStencil8,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    for (auto mutator : {Mutator::Update, Mutator::InsertBinding}) {
        // Success case, only the depth aspect is selected.
        {
            wgpu::TextureViewDescriptor vDesc{
                .aspect = wgpu::TextureAspect::DepthOnly,
            };
            wgpu::BindingResource resource = {.textureView = tex.CreateView(&vDesc)};
            TestMutator(mutator, &resource, true);
        }

        // Success case, only the stencil aspect is selected.
        {
            wgpu::TextureViewDescriptor vDesc{
                .aspect = wgpu::TextureAspect::StencilOnly,
            };
            wgpu::BindingResource resource = {.textureView = tex.CreateView(&vDesc)};
            TestMutator(mutator, &resource, true);
        }

        // Error case: both aspects are selected.
        {
            wgpu::BindingResource resource = {.textureView = tex.CreateView()};
            TestMutator(mutator, &resource, false);
        }
    }
}

// Test that it is not allowed to call Update, RemoveBinding or InsertBinding after the table is
// destroyed.
TEST_F(ResourceTableValidationTest, MutatorsAfterDestroy) {
    // TODO(435317394): Implemented bindless in the wire.
    if (UsesWire()) {
        GTEST_SKIP();
    }

    // Create the texture to put in the table.
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::BindingResource resource = {.textureView = device.CreateTexture(&tDesc).CreateView()};

    // This is "content timeline" validation so it works the same on error tables and valid tables,
    // and we ignore device-timeline validation errors, they are not what we are testing here.
    for (auto table : {MakeResourceTable(7), MakeErrorResourceTable(7)}) {
        utils::ScopedIgnoreValidationErrors ignoreErrors(device);

        // Add a few bindings just to test RemoveBinding
        EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));
        EXPECT_EQ(wgpu::Status::Success, table.Update(1, &resource));

        // Success cases, calling mutators before destroying is valid.
        EXPECT_EQ(wgpu::Status::Success, table.Update(2, &resource));
        EXPECT_NE(wgpu::kInvalidBinding, table.InsertBinding(&resource));
        EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(0));

        // Error case, after destruction all mutators return errors.
        table.Destroy();
        EXPECT_EQ(wgpu::Status::Error, table.Update(6, &resource));
        EXPECT_EQ(wgpu::kInvalidBinding, table.InsertBinding(&resource));
        EXPECT_EQ(wgpu::Status::Error, table.RemoveBinding(1));
    }
}

// Test that it is not allowed to call Update, RemoveBinding with slots past the end.
TEST_F(ResourceTableValidationTest, MutatorsAfterTableEnd) {
    // TODO(435317394): Implemented bindless in the wire.
    if (UsesWire()) {
        GTEST_SKIP();
    }

    // Create the texture to put in the table.
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::BindingResource resource = {.textureView = device.CreateTexture(&tDesc).CreateView()};

    // This is "content timeline" validation so it works the same on error tables and valid tables,
    // and we ignore device-timeline validation errors, they are not what we are testing here.
    for (auto table : {MakeResourceTable(42), MakeErrorResourceTable(42)}) {
        utils::ScopedIgnoreValidationErrors ignoreErrors(device);

        // Success cases, calling mutators with slots in bounds.
        EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));
        EXPECT_EQ(wgpu::Status::Success, table.Update(41, &resource));
        EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(0));
        EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(41));

        // Error case, calling mutators with out of bounds slots.
        EXPECT_EQ(wgpu::Status::Error, table.Update(42, &resource));
        EXPECT_EQ(wgpu::Status::Error, table.RemoveBinding(42));
    }
}

// Test that Update/RemoveBinding return success but generates a validation error when used on an
// invalid table.
TEST_F(ResourceTableValidationTest, MutatorsOnInvalidTable) {
    // TODO(435317394): Implemented bindless in the wire.
    if (UsesWire()) {
        GTEST_SKIP();
    }

    // Create the texture to put in the table.
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::BindingResource resource = {.textureView = device.CreateTexture(&tDesc).CreateView()};

    // Test on a valid table.
    {
        wgpu::ResourceTable table = MakeResourceTable(3);

        EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));
        EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(0));
        EXPECT_NE(wgpu::kInvalidBinding, table.InsertBinding(&resource));
    }

    // Test on an invalid table.
    {
        wgpu::ResourceTable table = MakeErrorResourceTable(3);

        ASSERT_DEVICE_ERROR(EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource)));
        ASSERT_DEVICE_ERROR(EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(0)));
        ASSERT_DEVICE_ERROR(EXPECT_NE(wgpu::kInvalidBinding, table.InsertBinding(&resource)));
    }

    // Test on an invalid table due to being too large.
    {
        wgpu::ResourceTable table;
        ASSERT_DEVICE_ERROR(table = MakeResourceTable(kMaxResourceTableSize + 1));

        EXPECT_EQ(wgpu::Status::Error, table.Update(0, &resource));
        EXPECT_EQ(wgpu::Status::Error, table.RemoveBinding(0));
        EXPECT_EQ(wgpu::kInvalidBinding, table.InsertBinding(&resource));
    }
}

// Test that Update() can be called on a table slot if it has never been used before.
TEST_F(ResourceTableValidationTest, UpdateBindingWhenNeverUsed) {
    // TODO(435317394): Implemented bindless in the wire.
    if (UsesWire()) {
        GTEST_SKIP();
    }

    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::BindingResource resource = {.textureView = device.CreateTexture(&tDesc).CreateView()};

    // This is "content timeline" validation so it works the same on error tables and valid tables,
    // and we ignore device-timeline validation errors, they are not what we are testing here.
    for (auto table : {MakeResourceTable(3), MakeErrorResourceTable(3)}) {
        utils::ScopedIgnoreValidationErrors ignoreErrors(device);

        // Updating slot 0 when it has never been used is valid, but a second time is an error.
        EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));
        EXPECT_EQ(wgpu::Status::Error, table.Update(0, &resource));

        // Even after using the table, a previously unused entry is valid to update.
        UseResourceTableInSubmit(table);
        EXPECT_EQ(wgpu::Status::Success, table.Update(1, &resource));
    }
}

// Test that Remove() can be called on a table slot even when it was never used.
TEST_F(ResourceTableValidationTest, RemoveBindingWhenNeverUsed) {
    // TODO(435317394): Implemented bindless in the wire.
    if (UsesWire()) {
        GTEST_SKIP();
    }

    // This is "content timeline" validation so it works the same on error tables and valid tables,
    // and we ignore device-timeline validation errors, they are not what we are testing here.
    for (auto table : {MakeResourceTable(3), MakeErrorResourceTable(3)}) {
        utils::ScopedIgnoreValidationErrors ignoreErrors(device);
        EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(0));
    }
}

// Check that a table slot can be updated only after all commands submitted prior to RemoveBinding
// are completed.
TEST_F(ResourceTableValidationTest, UpdateAfterRemoveRequiresGPUIsFinished) {
    // TODO(435317394): Implemented bindless in the wire.
    if (UsesWire()) {
        GTEST_SKIP();
    }

    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::BindingResource resource = {.textureView = device.CreateTexture(&tDesc).CreateView()};

    // This is "content timeline" validation so it works the same on error tables and valid tables,
    // and we ignore device-timeline validation errors, they are not what we are testing here.
    for (auto table : {MakeResourceTable(1), MakeErrorResourceTable(1)}) {
        utils::ScopedIgnoreValidationErrors ignoreErrors(device);

        // Removing while the table is still potentially in used by the GPU is an error. But
        // immediately after we know that the GPU is finished, it is valid.
        EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));

        bool updateValid = false;
        UseResourceTableInSubmit(table);
        device.GetQueue().OnSubmittedWorkDone(
            wgpu::CallbackMode::AllowSpontaneous,
            [&](wgpu::QueueWorkDoneStatus, wgpu::StringView) { updateValid = true; });
        EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(0));

        // The null backend happens to call OnSubmittedWorkDone immediately because commands take 0
        // time. This test is duplicated in the end2end tests where OnSubmittedWorkDone won't fire
        // immediately.
        // TODO(435317394): Actually duplicate in the end2end tests.
        if (updateValid) {
            EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));
            updateValid = false;
        } else {
            EXPECT_EQ(wgpu::Status::Error, table.Update(0, &resource));
        }

        WaitForAllOperations();

        if (updateValid) {
            EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));
        } else {
            EXPECT_EQ(wgpu::Status::Error, table.Update(0, &resource));
        }
    }
}

// Check that trying to insert bindings fail when no more are available.
TEST_F(ResourceTableValidationTest, InsertBindingFailWhenNoMoreSpace) {
    // TODO(435317394): Implemented bindless in the wire.
    if (UsesWire()) {
        GTEST_SKIP();
    }

    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::BindingResource resource = {.textureView = device.CreateTexture(&tDesc).CreateView()};

    // This is "content timeline" validation so it works the same on error tables and valid tables,
    // and we ignore device-timeline validation errors, they are not what we are testing here.
    for (auto table : {MakeResourceTable(3), MakeErrorResourceTable(3)}) {
        utils::ScopedIgnoreValidationErrors ignoreErrors(device);

        // There is space for only three resources.
        EXPECT_EQ(0u, table.InsertBinding(&resource));
        EXPECT_EQ(1u, table.InsertBinding(&resource));
        EXPECT_EQ(2u, table.InsertBinding(&resource));
        EXPECT_EQ(wgpu::kInvalidBinding, table.InsertBinding(&resource));

        // Remove one binding (and wait for it to be recycled), it will be available for
        // InsertBinding after which a new InsertBinding will still run out of space.
        EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(1));
        UseResourceTableInSubmit(table);
        WaitForAllOperations();

        EXPECT_EQ(1u, table.InsertBinding(&resource));
        EXPECT_EQ(wgpu::kInvalidBinding, table.InsertBinding(&resource));
    }
}

// Check that bindings that are inserted are unavailable for Update() until RemoveBinding.
TEST_F(ResourceTableValidationTest, InsertBindingPreventsUpdate) {
    // TODO(435317394): Implemented bindless in the wire.
    if (UsesWire()) {
        GTEST_SKIP();
    }

    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::BindingResource resource = {.textureView = device.CreateTexture(&tDesc).CreateView()};

    // This is "content timeline" validation so it works the same on error tables and valid tables,
    // and we ignore device-timeline validation errors, they are not what we are testing here.
    for (auto table : {MakeResourceTable(1), MakeErrorResourceTable(1)}) {
        utils::ScopedIgnoreValidationErrors ignoreErrors(device);

        EXPECT_EQ(0u, table.InsertBinding(&resource));
        EXPECT_EQ(wgpu::Status::Error, table.Update(0, &resource));

        // Remove one binding (and wait for it to be recycled), it will be available for Update.
        EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(0));
        UseResourceTableInSubmit(table);
        WaitForAllOperations();

        EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));
    }
}

// Check that InsertBinding skips over used slots.
TEST_F(ResourceTableValidationTest, InsertBindingSkipsOverUsedSlots) {
    // TODO(435317394): Implemented bindless in the wire.
    if (UsesWire()) {
        GTEST_SKIP();
    }

    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::BindingResource resource = {.textureView = device.CreateTexture(&tDesc).CreateView()};

    // This is "content timeline" validation so it works the same on error tables and valid tables,
    // and we ignore device-timeline validation errors, they are not what we are testing here.
    for (auto table : {MakeResourceTable(5), MakeErrorResourceTable(5)}) {
        utils::ScopedIgnoreValidationErrors ignoreErrors(device);
        EXPECT_EQ(wgpu::Status::Success, table.Update(1, &resource));
        EXPECT_EQ(wgpu::Status::Success, table.Update(3, &resource));

        // InsertBinding skips over entries used by Update()
        EXPECT_EQ(0u, table.InsertBinding(&resource));
        EXPECT_EQ(2u, table.InsertBinding(&resource));
        EXPECT_EQ(4u, table.InsertBinding(&resource));

        // Remove bindings in inverse order.
        for (uint32_t i : {4, 3, 2}) {
            EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(i));
        }
        UseResourceTableInSubmit(table);
        WaitForAllOperations();

        // InsertBinding should still return the min available slot.
        EXPECT_EQ(2u, table.InsertBinding(&resource));
        EXPECT_EQ(3u, table.InsertBinding(&resource));
        EXPECT_EQ(4u, table.InsertBinding(&resource));
    }
}

// Test the value returned by GetSize right after creating the table.
TEST_F(ResourceTableValidationTest, GetSizeAfterCreation) {
    // TODO(435317394): Implemented bindless in the wire.
    if (UsesWire()) {
        GTEST_SKIP();
    }

    // Valid resource tables of varying size.
    {
        EXPECT_EQ(0u, MakeResourceTable(0).GetSize());
        EXPECT_EQ(42u, MakeResourceTable(42).GetSize());
        EXPECT_EQ(kMaxResourceTableSize, MakeResourceTable(kMaxResourceTableSize).GetSize());
    }

    // Invalid resource tables of varying size under the limit.
    {
        EXPECT_EQ(0u, MakeErrorResourceTable(0).GetSize());
        EXPECT_EQ(42u, MakeErrorResourceTable(42).GetSize());
        EXPECT_EQ(kMaxResourceTableSize, MakeErrorResourceTable(kMaxResourceTableSize).GetSize());
    }

    // Invalid resource table with a size above the limit is a special case that doesn't allocate
    // state tracking.
    {
        wgpu::ResourceTable table;
        ASSERT_DEVICE_ERROR(table = MakeResourceTable(kMaxResourceTableSize + 1));
        EXPECT_EQ(0u, table.GetSize());
    }
}

// Test the value returned by GetSize after calling Destroy() should return the same value.
TEST_F(ResourceTableValidationTest, GetSizeAfterDestroy) {
    // TODO(435317394): Implemented bindless in the wire.
    if (UsesWire()) {
        GTEST_SKIP();
    }

    // Valid resource table.
    {
        wgpu::ResourceTable table = MakeResourceTable(42);
        EXPECT_EQ(42u, table.GetSize());
        table.Destroy();
        EXPECT_EQ(42u, table.GetSize());
    }

    // Invalid resource table.
    {
        wgpu::ResourceTable table = MakeResourceTable(42);
        EXPECT_EQ(42u, table.GetSize());
        table.Destroy();
        EXPECT_EQ(42u, table.GetSize());
    }
}

}  // namespace
}  // namespace dawn
