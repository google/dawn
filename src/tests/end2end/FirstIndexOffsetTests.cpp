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

#include "tests/DawnTest.h"

#include <sstream>
#include <vector>

#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

constexpr uint32_t kRTSize = 1;

enum class DrawMode {
    NonIndexed,
    Indexed,
};

enum class CheckIndex : uint32_t {
    Vertex = 0x0000001,
    Instance = 0x0000002,
};

namespace wgpu {
    template <>
    struct IsDawnBitmask<CheckIndex> {
        static constexpr bool enable = true;
    };
}  // namespace wgpu

class FirstIndexOffsetTests : public DawnTest {
  public:
    void TestVertexIndex(DrawMode mode, uint32_t firstVertex);
    void TestInstanceIndex(DrawMode mode, uint32_t firstInstance);
    void TestBothIndices(DrawMode mode, uint32_t firstVertex, uint32_t firstInstance);

  protected:
    void SetUp() override {
        DawnTest::SetUp();

        // WGSL doesn't have the ability to tag attributes as "flat". "flat" is required on u32
        // attributes for correct runtime behavior under Vulkan and codegen under OpenGL(ES).
        // TODO(tint:451): Remove once resolved by spec/tint
        DAWN_SKIP_TEST_IF(IsVulkan() || IsOpenGL() || IsOpenGLES());
    }

  private:
    void TestImpl(DrawMode mode,
                  CheckIndex checkIndex,
                  uint32_t vertexIndex,
                  uint32_t instanceIndex);
};

void FirstIndexOffsetTests::TestVertexIndex(DrawMode mode, uint32_t firstVertex) {
    TestImpl(mode, CheckIndex::Vertex, firstVertex, 0);
}

void FirstIndexOffsetTests::TestInstanceIndex(DrawMode mode, uint32_t firstInstance) {
    TestImpl(mode, CheckIndex::Instance, 0, firstInstance);
}

void FirstIndexOffsetTests::TestBothIndices(DrawMode mode,
                                            uint32_t firstVertex,
                                            uint32_t firstInstance) {
    using wgpu::operator|;
    TestImpl(mode, CheckIndex::Vertex | CheckIndex::Instance, firstVertex, firstInstance);
}

// Conditionally tests if first/baseVertex and/or firstInstance have been correctly passed to the
// vertex shader. Since vertex shaders can't write to storage buffers, we pass vertex/instance
// indices to a fragment shader via u32 attributes. The fragment shader runs once and writes the
// values to a storage buffer. If vertex index is used, the vertex buffer is padded with 0s.
void FirstIndexOffsetTests::TestImpl(DrawMode mode,
                                     CheckIndex checkIndex,
                                     uint32_t firstVertex,
                                     uint32_t firstInstance) {
    using wgpu::operator&;
    std::stringstream vertexShader;
    std::stringstream fragmentShader;

    if ((checkIndex & CheckIndex::Vertex) != 0) {
        vertexShader << R"(
        [[builtin(vertex_idx)]] var<in> vertex_index : u32;
        [[location(1)]] var<out> out_vertex_index : u32;
        )";
        fragmentShader << R"(
        [[location(1)]] var<in> in_vertex_index : u32;
    )";
    }
    if ((checkIndex & CheckIndex::Instance) != 0) {
        vertexShader << R"(
            [[builtin(instance_idx)]] var<in> instance_index : u32;
            [[location(2)]] var<out> out_instance_index : u32;
            )";
        fragmentShader << R"(
            [[location(2)]] var<in> in_instance_index : u32;
        )";
    }

    vertexShader << R"(
        [[builtin(position)]] var<out> position : vec4<f32>;
        [[location(0)]] var<in> pos : vec4<f32>;

        [[stage(vertex)]] fn main() -> void {)";
    fragmentShader << R"(
         [[block]] struct IndexVals {
             [[offset(0)]] vertex_index : u32;
             [[offset(4)]] instance_index : u32;
         };

        [[set(0), binding(0)]] var<storage_buffer> idx_vals : [[access(read_write)]] IndexVals;

        [[stage(fragment)]] fn main() -> void  {
        )";

    if ((checkIndex & CheckIndex::Vertex) != 0) {
        vertexShader << R"(
            out_vertex_index = vertex_index;
            )";
        fragmentShader << R"(
            idx_vals.vertex_index = in_vertex_index;
            )";
    }
    if ((checkIndex & CheckIndex::Instance) != 0) {
        vertexShader << R"(
            out_instance_index = instance_index;
            )";
        fragmentShader << R"(
            idx_vals.instance_index = in_instance_index;
            )";
    }

    vertexShader << R"(
            position = pos;
            return;
        })";

    fragmentShader << R"(
            return;
        })";

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    constexpr uint32_t kComponentsPerVertex = 4;

    utils::ComboRenderPipelineDescriptor pipelineDesc(device);
    pipelineDesc.vertexStage.module =
        utils::CreateShaderModuleFromWGSL(device, vertexShader.str().c_str());
    pipelineDesc.cFragmentStage.module =
        utils::CreateShaderModuleFromWGSL(device, fragmentShader.str().c_str());
    pipelineDesc.primitiveTopology = wgpu::PrimitiveTopology::PointList;
    pipelineDesc.cVertexState.vertexBufferCount = 1;
    pipelineDesc.cVertexState.cVertexBuffers[0].arrayStride = kComponentsPerVertex * sizeof(float);
    pipelineDesc.cVertexState.cVertexBuffers[0].attributeCount = 1;
    pipelineDesc.cVertexState.cAttributes[0].format = wgpu::VertexFormat::Float4;
    pipelineDesc.cColorStates[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

    std::vector<float> vertexData(firstVertex * kComponentsPerVertex);
    vertexData.insert(vertexData.end(), {0, 0, 0, 1});
    wgpu::Buffer vertices = utils::CreateBufferFromData(
        device, vertexData.data(), vertexData.size() * sizeof(float), wgpu::BufferUsage::Vertex);
    wgpu::Buffer indices =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, {0});
    wgpu::Buffer buffer = utils::CreateBufferFromData(
        device, wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage, {0u, 0u});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetVertexBuffer(0, vertices);
    pass.SetBindGroup(0,
                      utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer}}));
    if (mode == DrawMode::Indexed) {
        pass.SetIndexBuffer(indices, wgpu::IndexFormat::Uint32);
        pass.DrawIndexed(1, 1, 0, firstVertex, firstInstance);

    } else {
        pass.Draw(1, 1, firstVertex, firstInstance);
    }
    pass.EndPass();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    std::array<uint32_t, 2> expected = {firstVertex, firstInstance};
    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), buffer, 0, expected.size());
}

// Test that vertex_index starts at 7 when drawn using Draw()
TEST_P(FirstIndexOffsetTests, NonIndexedVertexOffset) {
    TestVertexIndex(DrawMode::NonIndexed, 7);
}

// Test that instance_index starts at 11 when drawn using Draw()
TEST_P(FirstIndexOffsetTests, NonIndexedInstanceOffset) {
    TestInstanceIndex(DrawMode::NonIndexed, 11);
}

// Test that vertex_index and instance_index start at 7 and 11 respectively when drawn using Draw()
TEST_P(FirstIndexOffsetTests, NonIndexedBothOffset) {
    TestBothIndices(DrawMode::NonIndexed, 7, 11);
}

// Test that vertex_index starts at 7 when drawn using DrawIndexed()
TEST_P(FirstIndexOffsetTests, IndexedVertex) {
    TestVertexIndex(DrawMode::Indexed, 7);
}

// Test that instance_index starts at 11 when drawn using DrawIndexed()
TEST_P(FirstIndexOffsetTests, IndexedInstance) {
    TestInstanceIndex(DrawMode::Indexed, 11);
}

// Test that vertex_index and instance_index start at 7 and 11 respectively when drawn using
// DrawIndexed()
TEST_P(FirstIndexOffsetTests, IndexedBothOffset) {
    TestBothIndices(DrawMode::Indexed, 7, 11);
}

DAWN_INSTANTIATE_TEST(FirstIndexOffsetTests,
                      D3D12Backend({"use_tint_generator"}),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
