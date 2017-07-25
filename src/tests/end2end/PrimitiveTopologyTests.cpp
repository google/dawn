// Copyright 2017 The NXT Authors
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

#include "tests/NXTTest.h"

#include "common/Assert.h"
#include "utils/NXTHelpers.h"

// Primitive topology tests work by drawing the following vertices with all the different primitive topology states:
// -------------------------------------
// |                                   |
// |        1        2        5        |
// |                                   |
// |                                   |
// |                                   |
// |                                   |
// |        0        3        4        |
// |                                   |
// -------------------------------------
//
// Points: This case looks exactly like above
//
// Lines
// -------------------------------------
// |                                   |
// |        1        2        5        |
// |        |        |        |        |
// |        |        |        |        |
// |        |        |        |        |
// |        |        |        |        |
// |        0        3        4        |
// |                                   |
// -------------------------------------
//
// Line Strip
// -------------------------------------
// |                                   |
// |        1--------2        5        |
// |        |        |        |        |
// |        |        |        |        |
// |        |        |        |        |
// |        |        |        |        |
// |        0        3--------4        |
// |                                   |
// -------------------------------------
//
// Triangle
// -------------------------------------
// |                                   |
// |        1--------2        5        |
// |        |xxxxxxx         x|        |
// |        |xxxxx         xxx|        |
// |        |xxx         xxxxx|        |
// |        |x         xxxxxxx|        |
// |        0        3--------4        |
// |                                   |
// -------------------------------------
//
// Triangle Strip
// -------------------------------------
// |                                   |
// |        1--------2        5        |
// |        |xxxxxxxxx       x|        |
// |        |xxxxxxxxxxx   xxx|        |
// |        |xxx   xxxxxxxxxxx|        |
// |        |x      xxxxxxxxxx|        |
// |        0        3--------4        |
// |                                   |
// -------------------------------------
//
// Each of these different states is a superset of some of the previous states,
// so for every state, we check any new added test locations that are not contained in previous states
// We also check that the test locations of subsequent states are untouched

constexpr static unsigned int kRTSize = 32;

struct TestLocation {
    unsigned int x, y;
};

constexpr TestLocation GetMidpoint(const TestLocation& a, const TestLocation& b) noexcept {
    return { (a.x + b.x) / 2, (a.y + b.y) / 2 };
}

constexpr TestLocation GetCentroid(const TestLocation& a, const TestLocation& b, const TestLocation& c) noexcept {
    return { (a.x + b.x + c.x) / 3, (a.y + b.y + c.y) / 3 };
}

// Offset towards one corner to avoid x or y symmetry false positives
constexpr static unsigned int kOffset = kRTSize / 8;

constexpr static TestLocation kPointTestLocations[] = {
    { kRTSize * 1 / 4 + kOffset, kRTSize * 1 / 4 + kOffset },
    { kRTSize * 1 / 4 + kOffset, kRTSize * 3 / 4 + kOffset },
    { kRTSize * 2 / 4 + kOffset, kRTSize * 3 / 4 + kOffset },
    { kRTSize * 2 / 4 + kOffset, kRTSize * 1 / 4 + kOffset },
    { kRTSize * 3 / 4 + kOffset, kRTSize * 1 / 4 + kOffset },
    { kRTSize * 3 / 4 + kOffset, kRTSize * 3 / 4 + kOffset },
};

constexpr static TestLocation kLineTestLocations[] = {
    GetMidpoint(kPointTestLocations[0], kPointTestLocations[1]),
    GetMidpoint(kPointTestLocations[2], kPointTestLocations[3]),
    GetMidpoint(kPointTestLocations[4], kPointTestLocations[5]),
};

constexpr static TestLocation kLineStripTestLocations[] = {
    GetMidpoint(kPointTestLocations[1], kPointTestLocations[2]),
    GetMidpoint(kPointTestLocations[3], kPointTestLocations[4]),
};

constexpr static TestLocation kTriangleTestLocations[] = {
    GetCentroid(kPointTestLocations[0], kPointTestLocations[1], kPointTestLocations[2]),
    GetCentroid(kPointTestLocations[3], kPointTestLocations[4], kPointTestLocations[5]),
};

constexpr static TestLocation kTriangleStripTestLocations[] = {
    GetCentroid(kPointTestLocations[1], kPointTestLocations[2], kPointTestLocations[3]),
    GetCentroid(kPointTestLocations[2], kPointTestLocations[3], kPointTestLocations[4]),
};

constexpr static float kRTSizef = static_cast<float>(kRTSize);
constexpr static float kVertices[] = {
    2.f * (kPointTestLocations[0].x + 0.5f) / kRTSizef - 1.f, 1.f - 2.f * (kPointTestLocations[0].y + 0.5f) / kRTSizef, 0.f, 1.f,
    2.f * (kPointTestLocations[1].x + 0.5f) / kRTSizef - 1.f, 1.f - 2.f * (kPointTestLocations[1].y + 0.5f) / kRTSizef, 0.f, 1.f,
    2.f * (kPointTestLocations[2].x + 0.5f) / kRTSizef - 1.f, 1.f - 2.f * (kPointTestLocations[2].y + 0.5f) / kRTSizef, 0.f, 1.f,
    2.f * (kPointTestLocations[3].x + 0.5f) / kRTSizef - 1.f, 1.f - 2.f * (kPointTestLocations[3].y + 0.5f) / kRTSizef, 0.f, 1.f,
    2.f * (kPointTestLocations[4].x + 0.5f) / kRTSizef - 1.f, 1.f - 2.f * (kPointTestLocations[4].y + 0.5f) / kRTSizef, 0.f, 1.f,
    2.f * (kPointTestLocations[5].x + 0.5f) / kRTSizef - 1.f, 1.f - 2.f * (kPointTestLocations[5].y + 0.5f) / kRTSizef, 0.f, 1.f,
};

class PrimitiveTopologyTest : public NXTTest {
    protected:
        void SetUp() override {
            NXTTest::SetUp();

            renderpass = device.CreateRenderPassBuilder()
                .SetAttachmentCount(1)
                .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
                .SetSubpassCount(1)
                .SubpassSetColorAttachment(0, 0, 0)
                .GetResult();

            renderTarget = device.CreateTextureBuilder()
                .SetDimension(nxt::TextureDimension::e2D)
                .SetExtent(kRTSize, kRTSize, 1)
                .SetFormat(nxt::TextureFormat::R8G8B8A8Unorm)
                .SetMipLevels(1)
                .SetAllowedUsage(nxt::TextureUsageBit::OutputAttachment | nxt::TextureUsageBit::TransferSrc)
                .SetInitialUsage(nxt::TextureUsageBit::OutputAttachment)
                .GetResult();

            renderTargetView = renderTarget.CreateTextureViewBuilder().GetResult();

            framebuffer = device.CreateFramebufferBuilder()
                .SetRenderPass(renderpass)
                .SetAttachment(0, renderTargetView)
                .SetDimensions(kRTSize, kRTSize)
                .GetResult();

            vsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Vertex, R"(
                #version 450
                layout(location = 0) in vec4 pos;
                void main() {
                    gl_Position = pos;
                })"
            );

            fsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Fragment, R"(
                #version 450
                out vec4 fragColor;
                void main() {
                    fragColor = vec4(0.0, 1.0, 0.0, 1.0);
                })"
            );

            inputState = device.CreateInputStateBuilder()
                .SetAttribute(0, 0, nxt::VertexFormat::FloatR32G32B32A32, 0)
                .SetInput(0, 4 * sizeof(float), nxt::InputStepMode::Vertex)
                .GetResult();

            vertexBuffer = utils::CreateFrozenBufferFromData(device, kVertices, sizeof(kVertices), nxt::BufferUsageBit::Vertex);
        }

        struct LocationSpec {
            const TestLocation* locations;
            size_t count;
            bool include;
        };

        template <std::size_t N>
        constexpr LocationSpec TestPoints(TestLocation const (&points)[N], bool include) noexcept {
            return { points, N, include };
        }

        // Draw the vertices with the given primitive topology and check the pixel values of the test locations
        void DoTest(nxt::PrimitiveTopology primitiveTopology, const std::vector<LocationSpec> &locationSpecs) {
            nxt::RenderPipeline pipeline = device.CreateRenderPipelineBuilder()
                .SetSubpass(renderpass, 0)
                .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
                .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
                .SetInputState(inputState)
                .SetPrimitiveTopology(primitiveTopology)
                .GetResult();

            renderTarget.TransitionUsage(nxt::TextureUsageBit::OutputAttachment);
            static const uint32_t zeroOffset = 0;
            nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
                .BeginRenderPass(renderpass, framebuffer)
                .BeginRenderSubpass()
                    .SetRenderPipeline(pipeline)
                    .SetVertexBuffers(0, 1, &vertexBuffer, &zeroOffset)
                    .DrawArrays(6, 1, 0, 0)
                .EndRenderSubpass()
                .EndRenderPass()
                .GetResult();

            queue.Submit(1, &commands);

            for (auto& locationSpec : locationSpecs) {
                for (size_t i = 0; i < locationSpec.count; ++i) {
                    // If this pixel is included, check that it is green. Otherwise, check that it is black
                    RGBA8 color = locationSpec.include ? RGBA8(0, 255, 0, 255) : RGBA8(0, 0, 0, 0);
                    EXPECT_PIXEL_RGBA8_EQ(color, renderTarget, locationSpec.locations[i].x, locationSpec.locations[i].y)
                        << "Expected (" << locationSpec.locations[i].x << ", " << locationSpec.locations[i].y << ") to be " << color;
                }
            }
        }

        nxt::RenderPass renderpass;
        nxt::Texture renderTarget;
        nxt::TextureView renderTargetView;
        nxt::Framebuffer framebuffer;
        nxt::ShaderModule vsModule;
        nxt::ShaderModule fsModule;
        nxt::InputState inputState;
        nxt::Buffer vertexBuffer;
};

// Test Point primitive topology
TEST_P(PrimitiveTopologyTest, Point) {
    DoTest(nxt::PrimitiveTopology::Point, {
        // Check that the points are drawn
        TestPoints(kPointTestLocations, true),

        // Check that line and triangle locations are untouched
        TestPoints(kLineTestLocations, false),
        TestPoints(kLineStripTestLocations, false),
        TestPoints(kTriangleTestLocations, false),
        TestPoints(kTriangleStripTestLocations, false),
    });
}

// Test Line primitive topology
TEST_P(PrimitiveTopologyTest, Line) {
    DoTest(nxt::PrimitiveTopology::Line, {
        // Check that lines are drawn
        TestPoints(kLineTestLocations, true),

        // Check that line strip and triangle locations are untouched
        TestPoints(kLineStripTestLocations, false),
        TestPoints(kTriangleTestLocations, false),
        TestPoints(kTriangleStripTestLocations, false),
    });
}

// Test LineStrip primitive topology
TEST_P(PrimitiveTopologyTest, LineStrip) {
    DoTest(nxt::PrimitiveTopology::LineStrip, {
        // Check that lines are drawn
        TestPoints(kLineTestLocations, true),
        TestPoints(kLineStripTestLocations, true),

        // Check that triangle locations are untouched
        TestPoints(kTriangleTestLocations, false),
        TestPoints(kTriangleStripTestLocations, false),
    });
}

// Test Triangle primitive topology
TEST_P(PrimitiveTopologyTest, Triangle) {
    DoTest(nxt::PrimitiveTopology::Triangle, {
        // Check that triangles are drawn
        TestPoints(kTriangleTestLocations, true),

        // Check that triangle strip locations are untouched
        TestPoints(kTriangleStripTestLocations, false),
    });
}

// Test TriangleStrip primitive topology
TEST_P(PrimitiveTopologyTest, TriangleStrip) {
    DoTest(nxt::PrimitiveTopology::TriangleStrip, {
        TestPoints(kTriangleTestLocations, true),
        TestPoints(kTriangleStripTestLocations, true),
    });
}

NXT_INSTANTIATE_TEST(PrimitiveTopologyTest, D3D12Backend, MetalBackend)
