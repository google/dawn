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

#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "dawn/common/Enumerator.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ScopedIgnoreValidationErrors.h"
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

    wgpu::ResourceTable MakeResourceTable(
        uint32_t size,
        std::vector<std::pair<uint32_t, wgpu::BindingResource>> resources = {}) {
        wgpu::ResourceTableDescriptor desc;
        desc.size = size;
        wgpu::ResourceTable table = device.CreateResourceTable(&desc);

        for (auto& [slot, resource] : resources) {
            EXPECT_EQ(wgpu::Status::Success, table.Update(slot, &resource));
        }

        return table;
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

    // Test that the `table`, has resources of `wgslType` in the `expected` slots.
    void TestHasResource(wgpu::ResourceTable table,
                         std::vector<bool> expected,
                         std::string wgslType = "texture_2d<f32>") {
        ASSERT_EQ(table.GetSize(), expected.size());

        // Create the test pipeline.
        wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_resource_table;

            @group(0) @binding(0) var<storage, read_write> results : array<u32>;
            var<immediate> resourceCount : u32;
            @compute @workgroup_size(1) fn main() {
                for (var i = 0u; i < resourceCount; i++) {
                    results[i] = u32(hasResource<)" + wgslType + R"(>(i));
                }
            }
        )");
        wgpu::ComputePipelineDescriptor csDesc = {.compute = {
                                                      .module = module,
                                                  }};
        wgpu::ComputePipeline testPipeline = device.CreateComputePipeline(&csDesc);

        // Create the result buffer.
        wgpu::BufferDescriptor bDesc = {
            .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
            .size = sizeof(uint32_t) * expected.size(),
        };
        wgpu::Buffer resultBuffer = device.CreateBuffer(&bDesc);
        wgpu::BindGroup resultBG =
            utils::MakeBindGroup(device, testPipeline.GetBindGroupLayout(0), {{0, resultBuffer}});
        uint32_t resourceCount = table.GetSize();

        // Run the test.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.SetResourceTable(table);
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetImmediates(0, &resourceCount, sizeof(resourceCount));
        pass.SetBindGroup(0, resultBG);
        pass.SetPipeline(testPipeline);
        pass.DispatchWorkgroups(1);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);

        // Check we have the expected results.
        std::vector<uint32_t> expectedU32;
        for (bool b : expected) {
            expectedU32.push_back(b ? 1u : 0u);
        }

        EXPECT_BUFFER_U32_RANGE_EQ(expectedU32.data(), resultBuffer, 0, expectedU32.size())
            << " for WGSL type " << wgslType;
    }

    void DoSomeWorkInSubmit() {
        wgpu::BufferDescriptor bufDesc = {
            .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc,
            .size = 4,
        };
        wgpu::Buffer src = device.CreateBuffer(&bufDesc);
        wgpu::Buffer dst = device.CreateBuffer(&bufDesc);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(src, 0, dst, 0, 4);

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    wgpu::TextureView MakePinnedU8View(uint8_t value) {
        // Create the texture.
        wgpu::TextureDescriptor tDesc{
            .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst,
            .size = {1, 1},
            .format = wgpu::TextureFormat::R8Uint,
        };
        wgpu::Texture tex = device.CreateTexture(&tDesc);

        // Write the u8
        wgpu::TexelCopyTextureInfo srcInfo = utils::CreateTexelCopyTextureInfo(tex);
        wgpu::TexelCopyBufferLayout dstInfo = {};
        wgpu::Extent3D copySize = {1, 1, 1};
        queue.WriteTexture(&srcInfo, &value, 1, &dstInfo, &copySize);

        // Return a view to the pinned texture.
        tex.Pin(wgpu::TextureUsage::TextureBinding);
        return tex.CreateView();
    }

    // Test that `table` has a texture_2d<u32> iff the `expected` has a value, and that the textures
    // have the expected value, if any.
    void TestHasU8Bindings(wgpu::ResourceTable table,
                           std::vector<std::optional<uint8_t>> expected) {
        ASSERT_EQ(table.GetSize(), expected.size());

        wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_resource_table;

            @group(0) @binding(0) var<storage, read_write> results : array<u32>;
            var<immediate> resourceCount : u32;
            @compute @workgroup_size(1) fn main() {
                for (var i = 0u; i < resourceCount; i++) {
                    if !hasResource<texture_2d<u32>>(i) {
                        results[i] = 0xBEEF;
                    } else {
                        results[i] = textureLoad(getResource<texture_2d<u32>>(i), vec2(0), 0).x;
                    }
                }
            }
        )");

        wgpu::ComputePipelineDescriptor csDesc = {.compute = {
                                                      .module = module,
                                                  }};
        wgpu::ComputePipeline testPipeline = device.CreateComputePipeline(&csDesc);

        // Create the result buffer.
        wgpu::BufferDescriptor bDesc = {
            .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
            .size = sizeof(uint32_t) * expected.size(),
        };
        wgpu::Buffer resultBuffer = device.CreateBuffer(&bDesc);
        wgpu::BindGroup resultBG =
            utils::MakeBindGroup(device, testPipeline.GetBindGroupLayout(0), {{0, resultBuffer}});
        uint32_t resourceCount = table.GetSize();

        // Run the test.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.SetResourceTable(table);
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetImmediates(0, &resourceCount, sizeof(resourceCount));
        pass.SetBindGroup(0, resultBG);
        pass.SetPipeline(testPipeline);
        pass.DispatchWorkgroups(1);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);

        // Check we have the expected results.
        std::vector<uint32_t> expectedU32;
        for (auto optValue : expected) {
            expectedU32.push_back(optValue ? *optValue : 0xBEEFu);
        }

        EXPECT_BUFFER_U32_RANGE_EQ(expectedU32.data(), resultBuffer, 0, expectedU32.size());
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

// Test that creating resource tables of different sizes doesn't end up reusing incorrectly sized
// allocations.
TEST_P(ResourceTableTests, RecyclingDoesntReuseTooSmallAllocation) {
    for (uint32_t i = 0; i < 10; i++) {
        MakeResourceTable(i);

        // Wait to ensure some deallocation happens and has a chance to cause incorrect recycling.
        WaitForAllOperations();
    }
}

// Tests that pinning / unpinning doesn't crash in backends.
TEST_P(ResourceTableTests, PinningBalancedInBackends) {
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R16Float,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    // Frontend should skip that unpinning as the texture is not pinned.
    tex.Unpin();

    // Duplicate pinning should be skipped by the frontend.
    tex.Pin(wgpu::TextureUsage::TextureBinding);
    tex.Pin(wgpu::TextureUsage::TextureBinding);

    // Duplicate unpinning should be skipped by the frontend.
    tex.Unpin();
    tex.Unpin();

    // Force a queue submit to flush pending commands and potentially find more issues.
    queue.Submit(0, nullptr);
}

// Test WGSL `hasResource` reflects the state of the resource table.
TEST_P(ResourceTableTests, HasResourceOneTexturePinUnpin) {
    // TODO(https://issues.chromium.org/463925499): Remove suppression once supported.
    DAWN_SUPPRESS_TEST_IF(IsVulkan());

    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    wgpu::ResourceTable table = MakeResourceTable(3, {{1, {.textureView = tex.CreateView()}}});

    // Before pinning, the table has no valid entries.
    TestHasResource(table, {false, false, false});

    // After pinning it has the one valid entry valid.
    tex.Pin(wgpu::TextureUsage::TextureBinding);
    TestHasResource(table, {false, true, false});

    // After unpinning it has the no more valid entries.
    tex.Unpin();
    TestHasResource(table, {false, false, false});
}

// Test that calling texture.Destroy() implicitly unpins it.
TEST_P(ResourceTableTests, HasResourceOneTexturePinDestroy) {
    // TODO(https://issues.chromium.org/463925499): Remove suppression once supported.
    DAWN_SUPPRESS_TEST_IF(IsVulkan());

    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    wgpu::ResourceTable table = MakeResourceTable(3, {{1, {.textureView = tex.CreateView()}}});

    // Before pinning, the table has no valid entries.
    TestHasResource(table, {false, false, false});

    // After pinning it has the one valid entry valid.
    tex.Pin(wgpu::TextureUsage::TextureBinding);
    TestHasResource(table, {false, true, false});

    // After texture destruction it has the no more valid entries.
    tex.Destroy();
    TestHasResource(table, {false, false, false});
}

// Test that a texture used multiple times in the same table has its availability correctly updated.
TEST_P(ResourceTableTests, HasResourceSameTextureMultipleTimesPinUnpin) {
    // TODO(https://issues.chromium.org/463925499): Remove suppression once supported.
    DAWN_SUPPRESS_TEST_IF(IsVulkan());

    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    wgpu::ResourceTable table = MakeResourceTable(4, {
                                                         {1, {.textureView = tex.CreateView()}},
                                                         {3, {.textureView = tex.CreateView()}},
                                                     });

    // Before pinning, the table has no valid entries.
    TestHasResource(table, {false, false, false, false});

    // After pinning it has valid entries.
    tex.Pin(wgpu::TextureUsage::TextureBinding);
    TestHasResource(table, {false, true, false, true});

    // After unpinning it has the no more valid entries.
    tex.Unpin();
    TestHasResource(table, {false, false, false, false});
}

// Test that updating a table with an already destroyed texture works, but doesn't show that entry
// as available.
TEST_P(ResourceTableTests, HasResourceUpdateWithTextureAlreadyDestroyed) {
    // TODO(https://issues.chromium.org/463925499): Remove suppression once supported.
    DAWN_SUPPRESS_TEST_IF(IsVulkan());

    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);
    tex.Destroy();

    wgpu::ResourceTable table = MakeResourceTable(1, {{0, {.textureView = tex.CreateView()}}});

    // Before pinning, the table has no valid entries.
    TestHasResource(table, {false});
}

// Test that a texture used in multiple resource tables has its availability correctly updated.
TEST_P(ResourceTableTests, HasResourceSameTextureMultipleTables) {
    // TODO(https://issues.chromium.org/463925499): Remove suppression once supported.
    DAWN_SUPPRESS_TEST_IF(IsVulkan());

    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    wgpu::ResourceTable table1 = MakeResourceTable(3, {{1, {.textureView = tex.CreateView()}}});
    wgpu::ResourceTable table2 = MakeResourceTable(1, {{0, {.textureView = tex.CreateView()}}});

    // Before pinning, the tables have no valid entries.
    TestHasResource(table1, {false, false, false});
    TestHasResource(table2, {false});

    // After pinning the texture, they have valid entries.
    tex.Pin(wgpu::TextureUsage::TextureBinding);
    TestHasResource(table1, {false, true, false});
    TestHasResource(table2, {true});

    // After destroying one table, the other still has the texture available.
    table1.Destroy();
    TestHasResource(table2, {true});
}

// Test that texture availabililty is controlled per-texture.
TEST_P(ResourceTableTests, HasResourceMultipleTexturesTable) {
    // TODO(https://issues.chromium.org/463925499): Remove suppression once supported.
    DAWN_SUPPRESS_TEST_IF(IsVulkan());

    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex0 = device.CreateTexture(&tDesc);
    wgpu::Texture tex1 = device.CreateTexture(&tDesc);

    wgpu::ResourceTable table = MakeResourceTable(2, {
                                                         {0, {.textureView = tex0.CreateView()}},
                                                         {1, {.textureView = tex1.CreateView()}},
                                                     });

    // Before pinning, the table has no valid entries.
    TestHasResource(table, {false, false});

    // After pinning tex0 it has one valid entry.
    tex0.Pin(wgpu::TextureUsage::TextureBinding);
    TestHasResource(table, {true, false});

    // After pinning tex1 it has two valid entries.
    tex1.Pin(wgpu::TextureUsage::TextureBinding);
    TestHasResource(table, {true, true});

    // After unpinning tex0 it has only one valid entry.
    tex0.Unpin();
    TestHasResource(table, {false, true});
}

constexpr auto kWgslSampledTextureTypes = std::array{
    "texture_1d<f32>",
    "texture_1d<i32>",
    "texture_1d<u32>",
    "texture_2d<f32>",
    "texture_2d<i32>",
    "texture_2d<u32>",
    "texture_2d_array<f32>",
    "texture_2d_array<i32>",
    "texture_2d_array<u32>",
    "texture_cube<f32>",
    "texture_cube<i32>",
    "texture_cube<u32>",
    "texture_cube_array<f32>",
    "texture_cube_array<i32>",
    "texture_cube_array<u32>",
    "texture_3d<f32>",
    "texture_3d<i32>",
    "texture_3d<u32>",

    "texture_multisampled_2d<f32>",
    "texture_multisampled_2d<i32>",
    "texture_multisampled_2d<u32>",

    "texture_depth_2d",
    "texture_depth_2d_array",
    "texture_depth_cube",
    "texture_depth_cube_array",
    "texture_depth_multisampled_2d",
};

struct TextureDescForTypeIDCase {
    std::unordered_set<std::string_view> wgslTypes;
    wgpu::TextureFormat format;
    wgpu::TextureDimension dimension;
    wgpu::TextureViewDimension viewDimension = wgpu::TextureViewDimension::Undefined;
    uint32_t sampleCount = 1;
    wgpu::TextureAspect viewAspect = wgpu::TextureAspect::All;

    // Create a view for a pinned texture for this case.
    wgpu::TextureView CreateTestView(const wgpu::Device& device) {
        wgpu::TextureDescriptor tDesc = {
            .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc,
            .dimension = dimension,
            .size = {1, 1, 1},
            .format = format,
            .sampleCount = sampleCount,
        };
        if (viewDimension == wgpu::TextureViewDimension::Cube ||
            viewDimension == wgpu::TextureViewDimension::CubeArray) {
            tDesc.size.depthOrArrayLayers = 6;
        }
        if (sampleCount != 1) {
            tDesc.usage |= wgpu::TextureUsage::RenderAttachment;
        }

        wgpu::TextureViewDescriptor vDesc{
            .dimension = viewDimension,
            .aspect = viewAspect,
            .usage = wgpu::TextureUsage::TextureBinding,
        };

        wgpu::Texture texture = device.CreateTexture(&tDesc);
        texture.Pin(wgpu::TextureUsage::TextureBinding);
        return texture.CreateView(&vDesc);
    }
};

std::vector<TextureDescForTypeIDCase> MakeTextureDescForTypeIDCases() {
    std::vector<TextureDescForTypeIDCase> cases;

    // TODO(https://crbug.com/435317394): Add tests of filterable vs. unfilterable floats when
    // get/hasBinding is able to make the difference.

    // Regular 1D textures.
    cases.push_back({
        .wgslTypes = {{"texture_1d<f32>"}},
        .format = wgpu::TextureFormat::RGBA32Float,
        .dimension = wgpu::TextureDimension::e1D,
    });
    cases.push_back({
        .wgslTypes = {{"texture_1d<i32>"}},
        .format = wgpu::TextureFormat::RGBA32Sint,
        .dimension = wgpu::TextureDimension::e1D,
    });
    cases.push_back({
        .wgslTypes = {{"texture_1d<u32>"}},
        .format = wgpu::TextureFormat::RGBA32Uint,
        .dimension = wgpu::TextureDimension::e1D,
    });

    // Regular 2D textures.
    cases.push_back({
        .wgslTypes = {{"texture_2d<f32>"}},
        .format = wgpu::TextureFormat::RGBA32Float,
        .dimension = wgpu::TextureDimension::e2D,
    });
    cases.push_back({
        .wgslTypes = {{"texture_2d<i32>"}},
        .format = wgpu::TextureFormat::RGBA32Sint,
        .dimension = wgpu::TextureDimension::e2D,
    });
    cases.push_back({
        .wgslTypes = {{"texture_2d<u32>"}},
        .format = wgpu::TextureFormat::RGBA32Uint,
        .dimension = wgpu::TextureDimension::e2D,
    });

    // Regular 2D array textures.
    cases.push_back({
        .wgslTypes = {{"texture_2d_array<f32>"}},
        .format = wgpu::TextureFormat::RGBA32Float,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::e2DArray,
    });
    cases.push_back({
        .wgslTypes = {{"texture_2d_array<i32>"}},
        .format = wgpu::TextureFormat::RGBA32Sint,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::e2DArray,
    });
    cases.push_back({
        .wgslTypes = {{"texture_2d_array<u32>"}},
        .format = wgpu::TextureFormat::RGBA32Uint,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::e2DArray,
    });

    // Regular cube textures.
    cases.push_back({
        .wgslTypes = {{"texture_cube<f32>"}},
        .format = wgpu::TextureFormat::RGBA32Float,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::Cube,
    });
    cases.push_back({
        .wgslTypes = {{"texture_cube<i32>"}},
        .format = wgpu::TextureFormat::RGBA32Sint,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::Cube,
    });
    cases.push_back({
        .wgslTypes = {{"texture_cube<u32>"}},
        .format = wgpu::TextureFormat::RGBA32Uint,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::Cube,
    });

    // Regular cube array textures.
    cases.push_back({
        .wgslTypes = {{"texture_cube_array<f32>"}},
        .format = wgpu::TextureFormat::RGBA32Float,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::CubeArray,
    });
    cases.push_back({
        .wgslTypes = {{"texture_cube_array<i32>"}},
        .format = wgpu::TextureFormat::RGBA32Sint,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::CubeArray,
    });
    cases.push_back({
        .wgslTypes = {{"texture_cube_array<u32>"}},
        .format = wgpu::TextureFormat::RGBA32Uint,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::CubeArray,
    });

    // Regular 3d textures.
    cases.push_back({
        .wgslTypes = {{"texture_3d<f32>"}},
        .format = wgpu::TextureFormat::RGBA32Float,
        .dimension = wgpu::TextureDimension::e3D,
    });
    cases.push_back({
        .wgslTypes = {{"texture_3d<i32>"}},
        .format = wgpu::TextureFormat::RGBA32Sint,
        .dimension = wgpu::TextureDimension::e3D,
    });
    cases.push_back({
        .wgslTypes = {{"texture_3d<u32>"}},
        .format = wgpu::TextureFormat::RGBA32Uint,
        .dimension = wgpu::TextureDimension::e3D,
    });

    // Color multisampled textures.
    cases.push_back({
        .wgslTypes = {{"texture_multisampled_2d<f32>"}},
        .format = wgpu::TextureFormat::RGBA16Float,
        .dimension = wgpu::TextureDimension::e2D,
        .sampleCount = 4,
    });
    cases.push_back({
        .wgslTypes = {{"texture_multisampled_2d<i32>"}},
        .format = wgpu::TextureFormat::RGBA16Sint,
        .dimension = wgpu::TextureDimension::e2D,
        .sampleCount = 4,
    });
    cases.push_back({
        .wgslTypes = {{"texture_multisampled_2d<u32>"}},
        .format = wgpu::TextureFormat::RGBA16Uint,
        .dimension = wgpu::TextureDimension::e2D,
        .sampleCount = 4,
    });

    // Depth textures (including multisampled).
    // TODO(https://crbug.com/435317394): In the future we should allow depth textures to be used as
    // texture_*<f32>.
    cases.push_back({
        .wgslTypes = {{"texture_depth_2d"}},
        .format = wgpu::TextureFormat::Depth32Float,
        .dimension = wgpu::TextureDimension::e2D,
    });
    cases.push_back({
        .wgslTypes = {{"texture_depth_2d_array"}},
        .format = wgpu::TextureFormat::Depth32Float,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::e2DArray,
    });
    cases.push_back({
        .wgslTypes = {{"texture_depth_cube"}},
        .format = wgpu::TextureFormat::Depth32Float,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::Cube,
    });
    cases.push_back({
        .wgslTypes = {{"texture_depth_cube_array"}},
        .format = wgpu::TextureFormat::Depth32Float,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::CubeArray,
    });
    cases.push_back({
        .wgslTypes = {{"texture_depth_multisampled_2d"}},
        .format = wgpu::TextureFormat::Depth32Float,
        .dimension = wgpu::TextureDimension::e2D,
        .sampleCount = 4,
    });

    // Stencil textures can be used as 2D.
    cases.push_back({
        .wgslTypes = {{"texture_2d<u32>"}},
        .format = wgpu::TextureFormat::Stencil8,
        .dimension = wgpu::TextureDimension::e2D,
    });
    cases.push_back({
        .wgslTypes = {{"texture_2d_array<u32>"}},
        .format = wgpu::TextureFormat::Stencil8,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::e2DArray,
    });
    cases.push_back({
        .wgslTypes = {{"texture_cube<u32>"}},
        .format = wgpu::TextureFormat::Stencil8,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::Cube,
    });
    cases.push_back({
        .wgslTypes = {{"texture_cube_array<u32>"}},
        .format = wgpu::TextureFormat::Stencil8,
        .dimension = wgpu::TextureDimension::e2D,
        .viewDimension = wgpu::TextureViewDimension::CubeArray,
    });

    // Depth-stencil textures with only one aspect selected.
    cases.push_back({
        .wgslTypes = {{"texture_depth_2d"}},
        .format = wgpu::TextureFormat::Depth24PlusStencil8,
        .dimension = wgpu::TextureDimension::e2D,
        .viewAspect = wgpu::TextureAspect::DepthOnly,
    });
    cases.push_back({
        .wgslTypes = {{"texture_2d<u32>"}},
        .format = wgpu::TextureFormat::Depth24PlusStencil8,
        .dimension = wgpu::TextureDimension::e2D,
        .viewAspect = wgpu::TextureAspect::StencilOnly,
    });

    return cases;
}

// Test that hasResource() works as expected for all supported types in WGSL.
TEST_P(ResourceTableTests, HasResourceTextureCompatibilityAllTypes) {
    // TODO(https://issues.chromium.org/463925499): Remove suppression once supported.
    DAWN_SUPPRESS_TEST_IF(IsVulkan());

    auto textureCases = MakeTextureDescForTypeIDCases();

    // Make a resource table with all of our test texture views.
    wgpu::ResourceTable table = MakeResourceTable(textureCases.size());
    for (auto [i, textureCase] : Enumerate(textureCases)) {
        wgpu::BindingResource resource = {.textureView = textureCase.CreateTestView(device)};
        table.Update(i, &resource);
    }

    // Test hasBinding returning for each of the supported WGSL types, against each texture.
    for (auto wgslType : kWgslSampledTextureTypes) {
        std::vector<bool> expected;
        for (auto textureCase : textureCases) {
            expected.push_back(textureCase.wgslTypes.contains(wgslType));
        }

        TestHasResource(table, expected, wgslType);
    }
}

// Test that calling hasResource() with values outside of the resource table size returns false.
TEST_P(ResourceTableTests, HasBindingOOBIsFalse) {
    // TODO(https://issues.chromium.org/463925499): Remove suppression once supported.
    DAWN_SUPPRESS_TEST_IF(IsVulkan());

    // Create the test pipeline
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;

        @group(0) @binding(0) var<storage, read_write> result : array<u32, 4>;
        var<immediate> resourceCount : u32;
        @compute @workgroup_size(1) fn getArrayLengths() {
            result[0] = u32(hasResource<texture_2d<f32>>(resourceCount - 1));
            result[1] = u32(hasResource<texture_2d<f32>>(resourceCount));

            // Check against all the slots where the default resources are.
            var result2 = 0u;
            for (var i = 1u; i < 100; i++) {
                result2 += u32(hasResource<texture_2d<f32>>(resourceCount + i));
            }
            result[2] = result2;

            result[3] = u32(hasResource<texture_2d<f32>>(resourceCount + 10000000));
        }
    )");
    wgpu::ComputePipelineDescriptor csDesc = {.compute = {
                                                  .module = module,
                                              }};
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Create the test resource table.
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);
    tex.Pin(wgpu::TextureUsage::TextureBinding);

    wgpu::ResourceTable table = MakeResourceTable(3, {
                                                         {0, {.textureView = tex.CreateView()}},
                                                         {1, {.textureView = tex.CreateView()}},
                                                         {2, {.textureView = tex.CreateView()}},
                                                     });

    // Create the other test resources.
    wgpu::BufferDescriptor bDesc = {
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        .size = 4 * sizeof(uint32_t),
    };
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bDesc);
    wgpu::BindGroup resultBG =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, resultBuffer}});
    uint32_t resourceCount = table.GetSize();

    // Run the test and check results are the expected ones.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.SetResourceTable(table);
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetImmediates(0, &resourceCount, sizeof(resourceCount));
    pass.SetBindGroup(0, resultBG);
    pass.SetPipeline(pipeline);
    pass.DispatchWorkgroups(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    device.GetQueue().Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(1, resultBuffer, 0);
    EXPECT_BUFFER_U32_EQ(0, resultBuffer, 4);
    EXPECT_BUFFER_U32_EQ(0, resultBuffer, 8);
    EXPECT_BUFFER_U32_EQ(0, resultBuffer, 12);
}

// Check that the default bindings are of size 1 and filled with zeroes. This is not an exhaustive
// test (that's for the CTS) but tries to check a few different interesting cases (MS, DS, Cube, 2D
// array).
TEST_P(ResourceTableTests, DefaultBindingsAreZeroAndSizeOne) {
    // TODO(https://issues.chromium.org/463925499): Remove suppression once supported.
    DAWN_SUPPRESS_TEST_IF(IsVulkan());

    // Create the test pipeline
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;

        @group(0) @binding(0) var<storage, read_write> error : u32;
        @group(0) @binding(1) var s : sampler;

        var<private> checkIndex = 0u;
        fn check(b : bool) {
            if (!b && error == 0) {
                error = 1 + checkIndex;
            }
            checkIndex++;
        }

        @compute @workgroup_size(1) fn checkDefault() {
            // Default texture_2d<f32>
            check(!hasResource<texture_2d<f32>>(0));
            check(all(textureDimensions(getResource<texture_2d<f32>>(0)) == vec2(1)));
            check(textureNumLevels(getResource<texture_2d<f32>>(0)) == 1);
            check(all(textureLoad(getResource<texture_2d<f32>>(0), vec2(0), 0) == vec4(0, 0, 0, 1)));

            // Default texture_multisampled_2d
            check(!hasResource<texture_multisampled_2d<u32>>(0));
            check(all(textureDimensions(getResource<texture_multisampled_2d<u32>>(0)) == vec2(1)));
            check(textureNumSamples(getResource<texture_multisampled_2d<u32>>(0)) == 4);
            check(all(textureLoad(getResource<texture_multisampled_2d<u32>>(0), vec2(0), 0) == vec4(0, 0, 0, 1)));

            // Default texture_depth_cube
            check(!hasResource<texture_depth_cube>(0));
            check(all(textureDimensions(getResource<texture_depth_cube>(0)) == vec2(1)));
            check(textureNumLevels(getResource<texture_depth_cube>(0)) == 1);
            check(textureSampleLevel(getResource<texture_depth_cube>(0), s, vec3(0), 0) == 0);

            // Default texture_2d_array<i32>
            check(!hasResource<texture_2d_array<i32>>(0));
            check(all(textureDimensions(getResource<texture_2d_array<i32>>(0)) == vec2(1)));
            check(textureNumLevels(getResource<texture_2d_array<i32>>(0)) == 1);
            check(textureNumLayers(getResource<texture_2d_array<i32>>(0)) == 1);
            check(all(textureLoad(getResource<texture_2d_array<i32>>(0), vec2(0), 0, 0) == vec4(0, 0, 0, 1)));
        }
    )");
    wgpu::ComputePipelineDescriptor csDesc = {.compute = {
                                                  .module = module,
                                              }};
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Create the test resources.
    wgpu::BufferDescriptor bDesc = {
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        .size = sizeof(uint32_t),
    };
    wgpu::Buffer errorBuffer = device.CreateBuffer(&bDesc);

    wgpu::BindGroup bg = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                              {
                                                  {0, errorBuffer},
                                                  {1, device.CreateSampler()},
                                              });
    wgpu::ResourceTable table = MakeResourceTable(0);

    // Run the test and check results are the expected ones.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.SetResourceTable(table);
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetBindGroup(0, bg);
    pass.SetPipeline(pipeline);
    pass.DispatchWorkgroups(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    device.GetQueue().Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(0, errorBuffer, 0);
}

// Check that Pin forces zero-initialization of the resources.
TEST_P(ResourceTableTests, PinDoesZeroInit) {
    // TODO(https://issues.chromium.org/463925499): Remove suppression once supported.
    DAWN_SUPPRESS_TEST_IF(IsVulkan());

    // Create the pipeline reading back from the texture.
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;

        @group(0) @binding(0) var<storage, read_write> result : u32;

        @compute @workgroup_size(1) fn readbackPixel() {
            let errorIfNotPresent = u32(!hasResource<texture_2d<u32>>(0));
            let texel = textureLoad(getResource<texture_2d<u32>>(0), vec2u(0), 0).r;
            result = errorIfNotPresent + texel;
        }
    )");

    wgpu::ComputePipelineDescriptor csDesc = {.compute = {
                                                  .module = module,
                                              }};
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Create the test resource table.
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Uint,
    };
    wgpu::TextureViewDescriptor vDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    wgpu::ResourceTable table =
        MakeResourceTable(1, {{0, {.textureView = tex.CreateView(&vDesc)}}});

    // Create the other test resources.
    wgpu::BufferDescriptor bDesc = {
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        .size = sizeof(uint32_t),
    };
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bDesc);

    wgpu::BindGroup bg = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                              {
                                                  {0, resultBuffer},
                                              });

    // Check that Pin does the initial zero init.
    {
        tex.Pin(wgpu::TextureUsage::TextureBinding);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.SetResourceTable(table);
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetBindGroup(0, bg);
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroups(1);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);

        EXPECT_BUFFER_U32_EQ(0, resultBuffer, 0);
    }

    // Use a render pass discard to mark the texture as uninitialized again. Use a LoadOp::Clear to
    // set some non-zero value in the texture which hopefully would tell us if the lazy clear didn't
    // happen.
    {
        tex.Unpin();

        wgpu::RenderPassColorAttachment attachment = {
            .view = tex.CreateView(),
            .loadOp = wgpu::LoadOp::Clear,
            .storeOp = wgpu::StoreOp::Discard,
            .clearValue = {.r = 1.0, .g = 0.0, .b = 0.0, .a = 0.0},
        };
        wgpu::RenderPassDescriptor rpDesc = {
            .colorAttachmentCount = 1,
            .colorAttachments = &attachment,
        };

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rpDesc);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);
    }

    // Check that Pin does the zero init after a discard.
    {
        tex.Pin(wgpu::TextureUsage::TextureBinding);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.SetResourceTable(table);
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetBindGroup(0, bg);
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroups(1);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);
        tex.Unpin();

        EXPECT_BUFFER_U32_EQ(0, resultBuffer, 0);
    }
}

// Check that a resource table slot can be updated only after all commands submitted prior to
// RemoveBinding are completed.
TEST_P(ResourceTableTests, UpdateAfterRemoveRequiresGPUIsFinished) {
    // TODO(435317394): Implemented bindless in the wire.
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Uint,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);
    wgpu::BindingResource resource{.textureView = tex.CreateView()};

    wgpu::ResourceTable table = MakeResourceTable(1);
    EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));

    // Removing while the table is still potentially in used by the GPU is an error. But immediately
    // after we know that the GPU is finished, it is valid.
    bool updateValid = false;
    DoSomeWorkInSubmit();
    queue.OnSubmittedWorkDone(
        wgpu::CallbackMode::AllowSpontaneous,
        [&](wgpu::QueueWorkDoneStatus, wgpu::StringView) { updateValid = true; });
    EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(0));

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

// Check that a resource table slot can be updated only after all commands submitted prior to
// RemoveBinding are completed.
TEST_P(ResourceTableTests, UpdateAfterRemoveRequiresGPUIsFinished_ErrorBindGroup) {
    // TODO(435317394): Implemented bindless in the wire.
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Uint,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);
    wgpu::BindingResource resource{.textureView = tex.CreateView()};

    // Make an error resource table.
    wgpu::RenderPassMaxDrawCount maxDraw;
    maxDraw.maxDrawCount = 1000;
    wgpu::ResourceTableDescriptor desc{
        .nextInChain = &maxDraw,
        .size = 1,
    };
    wgpu::ResourceTable table;
    ASSERT_DEVICE_ERROR(table = device.CreateResourceTable(&desc));

    {
        // Ignore all validation errors for this test as they are tested in other places, and we're
        // checking immediate validation returned as a wgpu::Status and supposed to be the same for
        // valid and invalid objects.
        utils::ScopedIgnoreValidationErrors ignoreErrors(device);

        EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));

        // Removing while the table is still potentially in used by the GPU is an error. But
        // immediately after we know that the GPU is finished, it is valid.
        bool updateValid = false;
        DoSomeWorkInSubmit();
        queue.OnSubmittedWorkDone(
            wgpu::CallbackMode::AllowSpontaneous,
            [&](wgpu::QueueWorkDoneStatus, wgpu::StringView) { updateValid = true; });
        EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(0));

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

// Check that Update and InsertBinding make the new binding visible in the resource table.
TEST_P(ResourceTableTests, UpdateAndInsertBindingMakeBindingVisible) {
    // TODO(https://issues.chromium.org/463925499): Remove suppression once supported.
    DAWN_SUPPRESS_TEST_IF(IsVulkan());

    // TODO(435317394): Implemented bindless in the wire.
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    wgpu::ResourceTable table = MakeResourceTable(2);

    // Before we do anything, the table has no valid entries.
    TestHasU8Bindings(table, {{}, {}});

    // Update makes the entry visible.
    wgpu::BindingResource resource0 = {.textureView = MakePinnedU8View(17)};
    EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource0));
    TestHasU8Bindings(table, {{17}, {}});

    // InsertBinding makes the entry visible.
    wgpu::BindingResource resource1 = {.textureView = MakePinnedU8View(42)};
    EXPECT_EQ(1u, table.InsertBinding(&resource1));
    TestHasU8Bindings(table, {{17}, {42}});
}

// Check that RemoveBinding instantly makes the binding not visible, both for entries added with
// Update and InsertBinding.
TEST_P(ResourceTableTests, RemoveBindingMakeBindingInvalid) {
    // TODO(https://issues.chromium.org/463925499): Remove suppression once supported.
    DAWN_SUPPRESS_TEST_IF(IsVulkan());

    // TODO(435317394): Implemented bindless in the wire.
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    // Fill a resource table with both Update and InsertBinding.
    wgpu::ResourceTable table = MakeResourceTable(2);

    wgpu::BindingResource resource0 = {.textureView = MakePinnedU8View(100)};
    EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource0));

    wgpu::BindingResource resource1 = {.textureView = MakePinnedU8View(101)};
    EXPECT_EQ(1u, table.InsertBinding(&resource1));

    // Before we remove bindings, they are all valid.
    TestHasU8Bindings(table, {{100}, {101}});

    // RemoveBinding immediately makes bindings invalid.
    EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(1));
    TestHasU8Bindings(table, {{100}, {}});
    EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(0));
    TestHasU8Bindings(table, {{}, {}});
}

// Check that removing a binding and adding a different one works.
TEST_P(ResourceTableTests, ReplaceBinding) {
    // TODO(https://issues.chromium.org/463925499): Remove suppression once supported.
    DAWN_SUPPRESS_TEST_IF(IsVulkan());

    // TODO(435317394): Implemented bindless in the wire.
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    // Create the test resource table.
    wgpu::ResourceTable table = MakeResourceTable(1);
    wgpu::BindingResource resource = {.textureView = MakePinnedU8View(19)};
    EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));

    // Test removing a binding that was previously there.
    TestHasU8Bindings(table, {{19}});
    EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(0));
    TestHasU8Bindings(table, {{}});

    /// Add it back a new entry, the shader should be seeing the updated entry.
    WaitForAllOperations();

    wgpu::BindingResource newResource = {.textureView = MakePinnedU8View(23)};
    EXPECT_EQ(wgpu::Status::Success, table.Update(0, &newResource));
    TestHasU8Bindings(table, {{23}});
}

// Check that removing a binding and adding it back works.
TEST_P(ResourceTableTests, ReplaceWithSameBinding) {
    // TODO(https://issues.chromium.org/463925499): Remove suppression once supported.
    DAWN_SUPPRESS_TEST_IF(IsVulkan());

    // TODO(435317394): Implemented bindless in the wire.
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    // Create the test resource table.
    wgpu::ResourceTable table = MakeResourceTable(1);
    wgpu::BindingResource resource = {.textureView = MakePinnedU8View(19)};
    EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));

    // Test removing a binding that was previously there.
    TestHasU8Bindings(table, {{19}});
    EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(0));
    TestHasU8Bindings(table, {{}});

    /// Add it back a new entry, the shader should be seeing the updated entry.
    WaitForAllOperations();

    EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));
    TestHasU8Bindings(table, {{19}});
}

DAWN_INSTANTIATE_TEST(ResourceTableTests, D3D12Backend(), MetalBackend(), VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
