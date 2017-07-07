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

#include "utils/NXTHelpers.h"

using nxt::InputStepMode;
using nxt::VertexFormat;

// Input state tests all work the same way: the test will render triangles in a grid up to 4x4. Each triangle
// is position in the grid such that X will correspond to the "triangle number" and the Y to the instance number.
// Each test will set up an input state and buffers, and the vertex shader will check that the vertex attributes
// corresponds to predetermined values. On success it outputs green, otherwise red.
//
// The predetermined values are "K * gl_VertexID + componentIndex" for vertex-indexed buffers, and
// "K * gl_InstanceID + componentIndex" for instance-indexed buffers.

constexpr static int kRTSize = 400;
constexpr static int kRTCellOffset = 50;
constexpr static int kRTCellSize = 100;

class InputStateTest : public NXTTest {
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
                .SetDimensions(640, 480)
                .GetResult();
        }

        bool ShouldComponentBeDefault(VertexFormat format, int component) {
            EXPECT_TRUE(component >= 0 && component < 4);
            switch (format) {
                case VertexFormat::FloatR32G32B32A32:
                    return component >= 4;
                case VertexFormat::FloatR32G32B32:
                    return component >= 3;
                case VertexFormat::FloatR32G32:
                    return component >= 2;
                case VertexFormat::FloatR32:
                    return component >= 1;
            }
        }

        struct ShaderTestSpec {
            uint32_t location;
            VertexFormat format;
            InputStepMode step;
        };
        nxt::Pipeline MakeTestPipeline(const nxt::InputState& inputState, int multiplier, std::vector<ShaderTestSpec> testSpec) {
            std::ostringstream vs;
            vs << "#version 450\n";

            // TODO(cwallez@chromium.org): this only handles float attributes, we should extend it to other types
            // Adds line of the form
            //    layout(location=1) in vec4 input1;
            for (const auto& input : testSpec) {
                vs << "layout(location=" << input.location << ") in vec4 input" << input.location << ";\n";
            }

            vs << "layout(location = 0) out vec4 color;\n";
            vs << "void main() {\n";

            // Hard code the triangle in the shader so that we don't have to add a vertex input for it.
            // Also this places the triangle in the grid based on its VertexID and InstanceID
            vs << "    const vec2 pos[3] = vec2[3](vec2(0.5f, 1.0f), vec2(0.0f, 0.0f), vec2(1.0f, 0.0f));\n";
            vs << "    vec2 offset = vec2(float(gl_VertexIndex / 3), 3 - float(gl_InstanceIndex));\n";
            vs << "    vec2 worldPos = pos[gl_VertexIndex % 3] + offset;\n";
            vs << "    gl_Position = vec4(worldPos / 2 - vec2(1.0f), 0.0f, 1.0f);\n";

            // Perform the checks by successively ANDing a boolean
            vs << "    bool success = true;\n";
            for (const auto& input : testSpec) {
                for (int component = 0; component < 4; ++component) {
                    vs << "    success = success && (input" << input.location << "[" << component << "] == ";
                    if (ShouldComponentBeDefault(input.format, component)) {
                        vs << (component == 3 ? "1.0f" : "0.0f");
                    } else {
                        if (input.step == InputStepMode::Vertex) {
                            vs << multiplier << " * gl_VertexIndex + " << component << ".0f";
                        } else {
                            vs << multiplier << " * gl_InstanceIndex + " << component << ".0f";
                        }
                    }
                    vs << ");\n";
                }
            }

            // Choose the color
            vs << "    if (success) {\n";
            vs << "        color = vec4(0.0f, 1.0f, 0.0f, 1.0f);\n";
            vs << "    } else {\n";
            vs << "        color = vec4(1.0f, 0.0f, 0.0f, 1.0f);\n";
            vs << "    }\n;";
            vs << "}\n";

            nxt::ShaderModule vsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Vertex, vs.str().c_str());
            nxt::ShaderModule fsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Fragment, R"(
                #version 450
                layout(location = 0) in vec4 color;
                layout(location = 0) out vec4 fragColor;
                void main() {
                    fragColor = color;
                })"
            );

            return device.CreatePipelineBuilder()
                .SetSubpass(renderpass, 0)
                .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
                .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
                .SetInputState(inputState)
                .GetResult();
        }

        struct InputSpec {
            uint32_t slot;
            uint32_t stride;
            InputStepMode step;
        };
        struct AttributeSpec {
            uint32_t location;
            uint32_t slot;
            uint32_t offset;
            VertexFormat format;
        };
        nxt::InputState MakeInputState(std::vector<InputSpec> inputs, std::vector<AttributeSpec> attributes) {
            nxt::InputStateBuilder builder = device.CreateInputStateBuilder();

            for (const auto& input : inputs) {
                builder.SetInput(input.slot, input.stride, input.step);
            }

            for (const auto& attribute : attributes) {
                builder.SetAttribute(attribute.location, attribute.slot, attribute.format, attribute.offset);
            }

            return builder.GetResult();
        }

        template<typename T>
        nxt::Buffer MakeVertexBuffer(std::vector<T> data) {
            return utils::CreateFrozenBufferFromData(device, data.data(), data.size() * sizeof(T), nxt::BufferUsageBit::Vertex);
        }

        struct DrawVertexBuffer {
            uint32_t location;
            nxt::Buffer* buffer;
        };
        void DoTestDraw(const nxt::Pipeline& pipeline, unsigned int triangles, unsigned int instances, std::vector<DrawVertexBuffer> vertexBuffers) {
            EXPECT_LE(triangles, 4);
            EXPECT_LE(instances, 4);

            nxt::CommandBufferBuilder builder = device.CreateCommandBufferBuilder();

            builder.BeginRenderPass(renderpass, framebuffer)
                .BeginRenderSubpass()
                .SetPipeline(pipeline);

            uint32_t zeroOffset = 0;
            for (const auto& buffer : vertexBuffers) {
                builder.SetVertexBuffers(buffer.location, 1, buffer.buffer, &zeroOffset);
            }

            nxt::CommandBuffer commands = builder
                .DrawArrays(triangles * 4, instances, 0, 0)
                .EndRenderSubpass()
                .EndRenderPass()
                .GetResult();

            queue.Submit(1, &commands);

            // Check that the center of each triangle is pure green, so that if a single vertex shader
            // instance fails, linear interpolation makes the pixel check fail.
            for (size_t triangle = 0; triangle < 4; triangle++) {
                for (size_t instance = 0; instance < 4; instance++) {
                    int x = kRTCellOffset + kRTCellSize * triangle;
                    int y = kRTCellOffset + kRTCellSize * instance;
                    if (triangle < triangles && instance < instances) {
                        EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 255, 0, 255), renderTarget, x, y);
                    } else {
                        EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 0, 0, 0), renderTarget, x, y);
                    }
                }
            }
        }

        nxt::RenderPass renderpass;
        nxt::Texture renderTarget;
        nxt::TextureView renderTargetView;
        nxt::Framebuffer framebuffer;
};

// Test compilation and usage of the fixture :)
TEST_P(InputStateTest, Basic) {
    nxt::InputState inputState = MakeInputState({
            {0, 4 * sizeof(float), InputStepMode::Vertex}
        }, {
            {0, 0, 0, VertexFormat::FloatR32G32B32A32}
        }
    );
    nxt::Pipeline pipeline = MakeTestPipeline(inputState, 1, {
        {0, VertexFormat::FloatR32G32B32A32, InputStepMode::Vertex}
    });

    nxt::Buffer buffer0 = MakeVertexBuffer<float>({
        0, 1, 2, 3,
        1, 2, 3, 4,
        2, 3, 4, 5
    });
    DoTestDraw(pipeline, 1, 1, {DrawVertexBuffer{0, &buffer0}});
}

// Test a stride of 0 works
TEST_P(InputStateTest, ZeroStride) {
    nxt::InputState inputState = MakeInputState({
            {0, 0, InputStepMode::Vertex}
        }, {
            {0, 0, 0, VertexFormat::FloatR32G32B32A32}
        }
    );
    nxt::Pipeline pipeline = MakeTestPipeline(inputState, 0, {
        {0, VertexFormat::FloatR32G32B32A32, InputStepMode::Vertex}
    });

    nxt::Buffer buffer0 = MakeVertexBuffer<float>({
        0, 1, 2, 3,
    });
    DoTestDraw(pipeline, 1, 1, {DrawVertexBuffer{0, &buffer0}});
}

// Test attributes defaults to (0, 0, 0, 1) if the input state doesn't have all components
TEST_P(InputStateTest, AttributeExpanding) {
    // R32F case
    {
        nxt::InputState inputState = MakeInputState({
                {0, 0, InputStepMode::Vertex}
            }, {
                {0, 0, 0, VertexFormat::FloatR32}
            }
        );
        nxt::Pipeline pipeline = MakeTestPipeline(inputState, 0, {
            {0, VertexFormat::FloatR32, InputStepMode::Vertex}
        });

        nxt::Buffer buffer0 = MakeVertexBuffer<float>({
            0, 1, 2, 3
        });
        DoTestDraw(pipeline, 1, 1, {DrawVertexBuffer{0, &buffer0}});
    }
    // RG32F case
    {
        nxt::InputState inputState = MakeInputState({
                {0, 0, InputStepMode::Vertex}
            }, {
                {0, 0, 0, VertexFormat::FloatR32G32}
            }
        );
        nxt::Pipeline pipeline = MakeTestPipeline(inputState, 0, {
            {0, VertexFormat::FloatR32G32, InputStepMode::Vertex}
        });

        nxt::Buffer buffer0 = MakeVertexBuffer<float>({
            0, 1, 2, 3
        });
        DoTestDraw(pipeline, 1, 1, {DrawVertexBuffer{0, &buffer0}});
    }
    // RGB32F case
    {
        nxt::InputState inputState = MakeInputState({
                {0, 0, InputStepMode::Vertex}
            }, {
                {0, 0, 0, VertexFormat::FloatR32G32B32}
            }
        );
        nxt::Pipeline pipeline = MakeTestPipeline(inputState, 0, {
            {0, VertexFormat::FloatR32G32B32, InputStepMode::Vertex}
        });

        nxt::Buffer buffer0 = MakeVertexBuffer<float>({
            0, 1, 2, 3
        });
        DoTestDraw(pipeline, 1, 1, {DrawVertexBuffer{0, &buffer0}});
    }
}

// Test a stride larger than the attributes
TEST_P(InputStateTest, StrideLargerThanAttributes) {
    nxt::InputState inputState = MakeInputState({
            {0, 8 * sizeof(float), InputStepMode::Vertex}
        }, {
            {0, 0, 0, VertexFormat::FloatR32G32B32A32}
        }
    );
    nxt::Pipeline pipeline = MakeTestPipeline(inputState, 1, {
        {0, VertexFormat::FloatR32G32B32A32, InputStepMode::Vertex}
    });

    nxt::Buffer buffer0 = MakeVertexBuffer<float>({
        0, 1, 2, 3, 0, 0, 0, 0,
        1, 2, 3, 4, 0, 0, 0, 0,
        2, 3, 4, 5, 0, 0, 0, 0,
    });
    DoTestDraw(pipeline, 1, 1, {DrawVertexBuffer{0, &buffer0}});
}

// Test two attributes at an offset, vertex version
TEST_P(InputStateTest, TwoAttributesAtAnOffsetVertex) {
    nxt::InputState inputState = MakeInputState({
            {0, 8 * sizeof(float), InputStepMode::Vertex}
        }, {
            {0, 0, 0, VertexFormat::FloatR32G32B32A32},
            {1, 0, 4  * sizeof(float), VertexFormat::FloatR32G32B32A32}
        }
    );
    nxt::Pipeline pipeline = MakeTestPipeline(inputState, 1, {
        {0, VertexFormat::FloatR32G32B32A32, InputStepMode::Vertex}
    });

    nxt::Buffer buffer0 = MakeVertexBuffer<float>({
        0, 1, 2, 3, 0, 1, 2, 3,
        1, 2, 3, 4, 1, 2, 3, 4,
        2, 3, 4, 5, 2, 3, 4, 5,
    });
    DoTestDraw(pipeline, 1, 1, {DrawVertexBuffer{0, &buffer0}});
}

// Test two attributes at an offset, instance version
TEST_P(InputStateTest, TwoAttributesAtAnOffsetInstance) {
    nxt::InputState inputState = MakeInputState({
            {0, 8 * sizeof(float), InputStepMode::Instance}
        }, {
            {0, 0, 0, VertexFormat::FloatR32G32B32A32},
            {1, 0, 4  * sizeof(float), VertexFormat::FloatR32G32B32A32}
        }
    );
    nxt::Pipeline pipeline = MakeTestPipeline(inputState, 1, {
        {0, VertexFormat::FloatR32G32B32A32, InputStepMode::Instance}
    });

    nxt::Buffer buffer0 = MakeVertexBuffer<float>({
        0, 1, 2, 3, 0, 1, 2, 3,
        1, 2, 3, 4, 1, 2, 3, 4,
        2, 3, 4, 5, 2, 3, 4, 5,
    });
    DoTestDraw(pipeline, 1, 1, {DrawVertexBuffer{0, &buffer0}});
}

// Test a pure-instance input state
TEST_P(InputStateTest, PureInstance) {
    nxt::InputState inputState = MakeInputState({
            {0, 4 * sizeof(float), InputStepMode::Instance}
        }, {
            {0, 0, 0, VertexFormat::FloatR32G32B32A32}
        }
    );
    nxt::Pipeline pipeline = MakeTestPipeline(inputState, 1, {
        {0, VertexFormat::FloatR32G32B32A32, InputStepMode::Instance}
    });

    nxt::Buffer buffer0 = MakeVertexBuffer<float>({
        0, 1, 2, 3,
        1, 2, 3, 4,
        2, 3, 4, 5,
        3, 4, 5, 6,
    });
    DoTestDraw(pipeline, 1, 4, {DrawVertexBuffer{0, &buffer0}});
}

// Test with mixed everything, vertex vs. instance, different stride and offsets
// different attribute types
TEST_P(InputStateTest, MixedEverything) {
    nxt::InputState inputState = MakeInputState({
            {0, 12 * sizeof(float), InputStepMode::Vertex},
            {1, 10 * sizeof(float), InputStepMode::Instance},
        }, {
            {0, 0, 0, VertexFormat::FloatR32},
            {1, 0, 6  * sizeof(float), VertexFormat::FloatR32G32},
            {2, 1, 0, VertexFormat::FloatR32G32B32},
            {3, 1, 5  * sizeof(float), VertexFormat::FloatR32G32B32A32}
        }
    );
    nxt::Pipeline pipeline = MakeTestPipeline(inputState, 1, {
        {0, VertexFormat::FloatR32, InputStepMode::Vertex},
        {1, VertexFormat::FloatR32G32, InputStepMode::Vertex},
        {2, VertexFormat::FloatR32G32B32, InputStepMode::Instance},
        {3, VertexFormat::FloatR32G32B32A32, InputStepMode::Instance}
    });

    nxt::Buffer buffer0 = MakeVertexBuffer<float>({
        0, 1, 2, 3, 0, 0, 0, 1, 2, 3, 0, 0,
        1, 2, 3, 4, 0, 0, 1, 2, 3, 4, 0, 0,
        2, 3, 4, 5, 0, 0, 2, 3, 4, 5, 0, 0,
        3, 4, 5, 6, 0, 0, 3, 4, 5, 6, 0, 0,
    });
    nxt::Buffer buffer1 = MakeVertexBuffer<float>({
        0, 1, 2, 3, 0, 0, 1, 2, 3, 0,
        1, 2, 3, 4, 0, 1, 2, 3, 4, 0,
        2, 3, 4, 5, 0, 2, 3, 4, 5, 0,
        3, 4, 5, 6, 0, 3, 4, 5, 6, 0,
    });
    DoTestDraw(pipeline, 1, 1, {{0, &buffer0}, {1, &buffer1}});
}

NXT_INSTANTIATE_TEST(InputStateTest, MetalBackend)

// TODO for the input state:
//  - Add more vertex formats
//  - Add checks that the stride is enough to contain all attributes
//  - Add checks stride less than some limit
//  - Add checks for alignement of vertex buffers and attributes if needed
//  - Check for attribute narrowing
//  - Check that the input state and the pipeline vertex input types match
