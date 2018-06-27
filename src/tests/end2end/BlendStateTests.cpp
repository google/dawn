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

#include <array>
#include <cmath>

#include "tests/NXTTest.h"

#include "common/Assert.h"
#include "common/Constants.h"
#include "utils/NXTHelpers.h"

constexpr static unsigned int kRTSize = 64;

class BlendStateTest : public NXTTest {
    protected:
        void SetUp() override {
            NXTTest::SetUp();

            vsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Vertex, R"(
                #version 450
                void main() {
                    const vec2 pos[3] = vec2[3](vec2(-1.f, -1.f), vec2(3.f, -1.f), vec2(-1.f, 3.f));
                    gl_Position = vec4(pos[gl_VertexIndex], 0.f, 1.f);
                }
            )");


            bindGroupLayout = device.CreateBindGroupLayoutBuilder()
                .SetBindingsType(nxt::ShaderStageBit::Fragment, nxt::BindingType::UniformBuffer, 0, 1)
                .GetResult();

            pipelineLayout = utils::MakeBasicPipelineLayout(device, &bindGroupLayout);

            renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
        }

        struct TriangleSpec {
            RGBA8 color;
            std::array<float, 4> blendFactor = {};
        };

        // Set up basePipeline and testPipeline. testPipeline has the given blend state on the first attachment. basePipeline has no blending
        void SetupSingleSourcePipelines(const nxt::BlendState &blendState) {
            nxt::ShaderModule fsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Fragment, R"(
                #version 450
                layout(set = 0, binding = 0) uniform myBlock {
                    vec4 color;
                } myUbo;

                layout(location = 0) out vec4 fragColor;

                void main() {
                    fragColor = myUbo.color;
                }
            )");

            basePipeline = device.CreateRenderPipelineBuilder()
                .SetColorAttachmentFormat(0, renderPass.colorFormat)
                .SetLayout(pipelineLayout)
                .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
                .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
                .GetResult();

            testPipeline = device.CreateRenderPipelineBuilder()
                .SetColorAttachmentFormat(0, renderPass.colorFormat)
                .SetLayout(pipelineLayout)
                .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
                .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
                .SetColorAttachmentBlendState(0, blendState)
                .GetResult();
        }

        // Create a bind group to set the colors as a uniform buffer
        template <size_t N>
        nxt::BindGroup MakeBindGroupForColors(std::array<RGBA8, N> colors) {
            std::array<float, 4 * N> data;
            for (unsigned int i = 0; i < N; ++i) {
                data[4 * i + 0] = static_cast<float>(colors[i].r) / 255.f;
                data[4 * i + 1] = static_cast<float>(colors[i].g) / 255.f;
                data[4 * i + 2] = static_cast<float>(colors[i].b) / 255.f;
                data[4 * i + 3] = static_cast<float>(colors[i].a) / 255.f;
            }

            uint32_t bufferSize = static_cast<uint32_t>(4 * N * sizeof(float));

            nxt::Buffer buffer = utils::CreateFrozenBufferFromData(device, &data, bufferSize, nxt::BufferUsageBit::Uniform);

            nxt::BufferView view = buffer.CreateBufferViewBuilder()
                .SetExtent(0, bufferSize)
                .GetResult();

            return device.CreateBindGroupBuilder()
                .SetLayout(bindGroupLayout)
                .SetUsage(nxt::BindGroupUsage::Frozen)
                .SetBufferViews(0, 1, &view)
                .GetResult();
        }

        // Test that after drawing a triangle with the base color, and then the given triangle spec, the color is as expected
        void DoSingleSourceTest(RGBA8 base, const TriangleSpec& triangle, const RGBA8& expected) {
            renderPass.color.TransitionUsage(nxt::TextureUsageBit::OutputAttachment);

            nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
                .BeginRenderPass(renderPass.renderPassInfo)
                    // First use the base pipeline to draw a triangle with no blending
                    .SetRenderPipeline(basePipeline)
                    .SetBindGroup(0, MakeBindGroupForColors(std::array<RGBA8, 1>({ { base } })))
                    .DrawArrays(3, 1, 0, 0)

                    // Then use the test pipeline to draw the test triangle with blending
                    .SetRenderPipeline(testPipeline)
                    .SetBindGroup(0, MakeBindGroupForColors(std::array<RGBA8, 1>({ { triangle.color } })))
                    .SetBlendColor(triangle.blendFactor[0], triangle.blendFactor[1], triangle.blendFactor[2], triangle.blendFactor[3])
                    .DrawArrays(3, 1, 0, 0)
                .EndRenderPass()
                .GetResult();


            queue.Submit(1, &commands);

            EXPECT_PIXEL_RGBA8_EQ(expected, renderPass.color, kRTSize / 2, kRTSize / 2);
        }

        // Given a vector of tests where each element is <testColor, expectedColor>, check that all expectations are true for the given blend operation
        void CheckBlendOperation(RGBA8 base, nxt::BlendOperation operation, std::vector<std::pair<RGBA8, RGBA8>> tests) {
            nxt::BlendState blendState = device.CreateBlendStateBuilder()
                .SetBlendEnabled(true)
                .SetColorBlend(operation, nxt::BlendFactor::One, nxt::BlendFactor::One)
                .SetAlphaBlend(operation, nxt::BlendFactor::One, nxt::BlendFactor::One)
                .GetResult();

            SetupSingleSourcePipelines(blendState);

            for (const auto& test : tests) {
                DoSingleSourceTest(base, { test.first }, test.second);
            }
        }

        // Given a vector of tests where each element is <testSpec, expectedColor>, check that all expectations are true for the given blend factors
        void CheckBlendFactor(RGBA8 base, nxt::BlendFactor colorSrcFactor, nxt::BlendFactor colorDstFactor, nxt::BlendFactor alphaSrcFactor, nxt::BlendFactor alphaDstFactor, std::vector<std::pair<TriangleSpec, RGBA8>> tests) {
            nxt::BlendState blendState = device.CreateBlendStateBuilder()
                .SetBlendEnabled(true)
                .SetColorBlend(nxt::BlendOperation::Add, colorSrcFactor, colorDstFactor)
                .SetAlphaBlend(nxt::BlendOperation::Add, alphaSrcFactor, alphaDstFactor)
                .GetResult();

            SetupSingleSourcePipelines(blendState);

            for (const auto& test : tests) {
                DoSingleSourceTest(base, test.first, test.second);
            }
        }

        void CheckSrcBlendFactor(RGBA8 base, nxt::BlendFactor colorFactor, nxt::BlendFactor alphaFactor, std::vector<std::pair<TriangleSpec, RGBA8>> tests) {
            CheckBlendFactor(base, colorFactor, nxt::BlendFactor::One, alphaFactor, nxt::BlendFactor::One, tests);
        }

        void CheckDstBlendFactor(RGBA8 base, nxt::BlendFactor colorFactor, nxt::BlendFactor alphaFactor, std::vector<std::pair<TriangleSpec, RGBA8>> tests) {
            CheckBlendFactor(base, nxt::BlendFactor::One, colorFactor, nxt::BlendFactor::One, alphaFactor, tests);
        }

        utils::BasicRenderPass renderPass;
        nxt::RenderPipeline basePipeline;
        nxt::RenderPipeline testPipeline;
        nxt::ShaderModule vsModule;
        nxt::BindGroupLayout bindGroupLayout;
        nxt::PipelineLayout pipelineLayout;
};

namespace {
    // Add two colors and clamp
    constexpr RGBA8 operator+(const RGBA8& col1, const RGBA8& col2) {
        int r = static_cast<int>(col1.r) + static_cast<int>(col2.r);
        int g = static_cast<int>(col1.g) + static_cast<int>(col2.g);
        int b = static_cast<int>(col1.b) + static_cast<int>(col2.b);
        int a = static_cast<int>(col1.a) + static_cast<int>(col2.a);
        r = (r > 255 ? 255 : (r < 0 ? 0 : r));
        g = (g > 255 ? 255 : (g < 0 ? 0 : g));
        b = (b > 255 ? 255 : (b < 0 ? 0 : b));
        a = (a > 255 ? 255 : (a < 0 ? 0 : a));

        return RGBA8(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b), static_cast<uint8_t>(a));
    }

    // Subtract two colors and clamp
    constexpr RGBA8 operator-(const RGBA8& col1, const RGBA8& col2) {
        int r = static_cast<int>(col1.r) - static_cast<int>(col2.r);
        int g = static_cast<int>(col1.g) - static_cast<int>(col2.g);
        int b = static_cast<int>(col1.b) - static_cast<int>(col2.b);
        int a = static_cast<int>(col1.a) - static_cast<int>(col2.a);
        r = (r > 255 ? 255 : (r < 0 ? 0 : r));
        g = (g > 255 ? 255 : (g < 0 ? 0 : g));
        b = (b > 255 ? 255 : (b < 0 ? 0 : b));
        a = (a > 255 ? 255 : (a < 0 ? 0 : a));

        return RGBA8(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b), static_cast<uint8_t>(a));
    }

    // Get the component-wise minimum of two colors
    RGBA8 min(const RGBA8& col1, const RGBA8& col2) {
        return RGBA8(
            std::min(col1.r, col2.r),
            std::min(col1.g, col2.g),
            std::min(col1.b, col2.b),
            std::min(col1.a, col2.a)
        );
    }

    // Get the component-wise maximum of two colors
    RGBA8 max(const RGBA8& col1, const RGBA8& col2) {
        return RGBA8(
            std::max(col1.r, col2.r),
            std::max(col1.g, col2.g),
            std::max(col1.b, col2.b),
            std::max(col1.a, col2.a)
        );
    }

    // Blend two RGBA8 color values parameterized by the provided factors in the range [0.f, 1.f]
    RGBA8 mix(const RGBA8& col1, const RGBA8& col2, std::array<float, 4> fac) {
        float r = static_cast<float>(col1.r) * (1.f - fac[0]) + static_cast<float>(col2.r) * fac[0];
        float g = static_cast<float>(col1.g) * (1.f - fac[1]) + static_cast<float>(col2.g) * fac[1];
        float b = static_cast<float>(col1.b) * (1.f - fac[2]) + static_cast<float>(col2.b) * fac[2];
        float a = static_cast<float>(col1.a) * (1.f - fac[3]) + static_cast<float>(col2.a) * fac[3];

        return RGBA8({ static_cast<uint8_t>(std::round(r)), static_cast<uint8_t>(std::round(g)), static_cast<uint8_t>(std::round(b)), static_cast<uint8_t>(std::round(a)) });
    }

    // Blend two RGBA8 color values parameterized by the provided RGBA8 factor
    RGBA8 mix(const RGBA8& col1, const RGBA8& col2, const RGBA8& fac) {
        std::array<float, 4> f = { {
            static_cast<float>(fac.r) / 255.f,
            static_cast<float>(fac.g) / 255.f,
            static_cast<float>(fac.b) / 255.f,
            static_cast<float>(fac.a) / 255.f,
        } };
        return mix(col1, col2, f);
    }

    constexpr std::array<RGBA8, 8> kColors = { {
        // check operations over multiple channels
        RGBA8(64,0,0,0),
        RGBA8(0,64,0,0),
        RGBA8(64,0,32,0),
        RGBA8(0,64,32,0),
        RGBA8(128,0,128,128),
        RGBA8(0,128,128,128),

        // check cases that may cause overflow
        RGBA8(0,0,0,0),
        RGBA8(255,255,255,255),
    } };
}

// Test compilation and usage of the fixture
TEST_P(BlendStateTest, Basic) {
    nxt::BlendState blendState = device.CreateBlendStateBuilder().GetResult();
    SetupSingleSourcePipelines(blendState);

    DoSingleSourceTest(RGBA8(0, 0, 0, 0), { RGBA8(255, 0, 0, 0) }, RGBA8(255, 0, 0, 0));
}

// The following tests check test that the blend operation works
TEST_P(BlendStateTest, BlendOperationAdd) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<RGBA8, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        return std::make_pair(color, base + color);
    });
    CheckBlendOperation(base, nxt::BlendOperation::Add, tests);
}

TEST_P(BlendStateTest, BlendOperationSubtract) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<RGBA8, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        return std::make_pair(color, color - base);
    });
    CheckBlendOperation(base, nxt::BlendOperation::Subtract, tests);
}

TEST_P(BlendStateTest, BlendOperationReverseSubtract) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<RGBA8, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        return std::make_pair(color, base - color);
    });
    CheckBlendOperation(base, nxt::BlendOperation::ReverseSubtract, tests);
}

TEST_P(BlendStateTest, BlendOperationMin) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<RGBA8, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        return std::make_pair(color, min(base, color));
    });
    CheckBlendOperation(base, nxt::BlendOperation::Min, tests);
}

TEST_P(BlendStateTest, BlendOperationMax) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<RGBA8, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        return std::make_pair(color, max(base, color));
    });
    CheckBlendOperation(base, nxt::BlendOperation::Max, tests);
}

// The following tests check that the Source blend factor works
TEST_P(BlendStateTest, SrcBlendFactorZero) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        return std::make_pair(TriangleSpec({ { color } }), base);
    });
    CheckSrcBlendFactor(base, nxt::BlendFactor::Zero, nxt::BlendFactor::Zero, tests);
}

TEST_P(BlendStateTest, SrcBlendFactorOne) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        return std::make_pair(TriangleSpec({ { color } }), base + color);
    });
    CheckSrcBlendFactor(base, nxt::BlendFactor::One, nxt::BlendFactor::One, tests);
}

TEST_P(BlendStateTest, SrcBlendFactorSrcColor) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        RGBA8 fac = color;
        fac.a = 0;
        RGBA8 expected = base + mix(RGBA8(0, 0, 0, 0), color, fac);
        return std::make_pair(TriangleSpec({ { color } }), expected);
    });
    CheckSrcBlendFactor(base, nxt::BlendFactor::SrcColor, nxt::BlendFactor::Zero, tests);
}

TEST_P(BlendStateTest, SrcBlendFactorOneMinusSrcColor) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        RGBA8 fac = RGBA8(255, 255, 255, 255) - color;
        fac.a = 0;
        RGBA8 expected = base + mix(RGBA8(0, 0, 0, 0), color, fac);
        return std::make_pair(TriangleSpec({ { color } }), expected);
    });
    CheckSrcBlendFactor(base, nxt::BlendFactor::OneMinusSrcColor, nxt::BlendFactor::Zero, tests);
}

TEST_P(BlendStateTest, SrcBlendFactorSrcAlpha) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        RGBA8 fac(color.a, color.a, color.a, color.a);
        RGBA8 expected = base + mix(RGBA8(0, 0, 0, 0), color, fac);
        return std::make_pair(TriangleSpec({ { color } }), expected);
    });
    CheckSrcBlendFactor(base, nxt::BlendFactor::SrcAlpha, nxt::BlendFactor::SrcAlpha, tests);
}

TEST_P(BlendStateTest, SrcBlendFactorOneMinusSrcAlpha) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        RGBA8 fac = RGBA8(255, 255, 255, 255) - RGBA8(color.a, color.a, color.a, color.a);
        RGBA8 expected = base + mix(RGBA8(0, 0, 0, 0), color, fac);
        return std::make_pair(TriangleSpec({ { color } }), expected);
    });
    CheckSrcBlendFactor(base, nxt::BlendFactor::OneMinusSrcAlpha, nxt::BlendFactor::OneMinusSrcAlpha, tests);
}

TEST_P(BlendStateTest, SrcBlendFactorDstColor) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        RGBA8 fac = base;
        fac.a = 0;
        RGBA8 expected = base + mix(RGBA8(0, 0, 0, 0), color, fac);
        return std::make_pair(TriangleSpec({ { color } }), expected);
    });
    CheckSrcBlendFactor(base, nxt::BlendFactor::DstColor, nxt::BlendFactor::Zero, tests);
}

TEST_P(BlendStateTest, SrcBlendFactorOneMinusDstColor) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        RGBA8 fac = RGBA8(255, 255, 255, 255) - base;
        fac.a = 0;
        RGBA8 expected = base + mix(RGBA8(0, 0, 0, 0), color, fac);
        return std::make_pair(TriangleSpec({ { color } }), expected);
    });
    CheckSrcBlendFactor(base, nxt::BlendFactor::OneMinusDstColor, nxt::BlendFactor::Zero, tests);
}

TEST_P(BlendStateTest, SrcBlendFactorDstAlpha) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        RGBA8 fac(base.a, base.a, base.a, base.a);
        RGBA8 expected = base + mix(RGBA8(0, 0, 0, 0), color, fac);
        return std::make_pair(TriangleSpec({ { color } }), expected);
    });
    CheckSrcBlendFactor(base, nxt::BlendFactor::DstAlpha, nxt::BlendFactor::DstAlpha, tests);
}

TEST_P(BlendStateTest, SrcBlendFactorOneMinusDstAlpha) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        RGBA8 fac = RGBA8(255, 255, 255, 255) - RGBA8(base.a, base.a, base.a, base.a);
        RGBA8 expected = base + mix(RGBA8(0, 0, 0, 0), color, fac);
        return std::make_pair(TriangleSpec({ { color } }), expected);
    });
    CheckSrcBlendFactor(base, nxt::BlendFactor::OneMinusDstAlpha, nxt::BlendFactor::OneMinusDstAlpha, tests);
}

TEST_P(BlendStateTest, SrcBlendFactorSrcAlphaSaturated) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        uint8_t f = std::min(color.a, static_cast<uint8_t>(255 - base.a));
        RGBA8 fac(f, f, f, 255);
        RGBA8 expected = base + mix(RGBA8(0, 0, 0, 0), color, fac);
        return std::make_pair(TriangleSpec({ { color } }), expected);
    });
    CheckSrcBlendFactor(base, nxt::BlendFactor::SrcAlphaSaturated, nxt::BlendFactor::SrcAlphaSaturated, tests);
}

TEST_P(BlendStateTest, SrcBlendFactorBlendColor) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        auto triangleSpec = TriangleSpec({ { color }, {{ 0.2f, 0.4f, 0.6f, 0.8f }} });
        RGBA8 expected = base + mix(RGBA8(0, 0, 0, 0), color, triangleSpec.blendFactor);
        return std::make_pair(triangleSpec, expected);
    });
    CheckSrcBlendFactor(base, nxt::BlendFactor::BlendColor, nxt::BlendFactor::BlendColor, tests);
}

TEST_P(BlendStateTest, SrcBlendFactorOneMinusBlendColor) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        auto triangleSpec = TriangleSpec({ { color }, {{ 0.2f, 0.4f, 0.6f, 0.8f }} });
        std::array<float, 4> f = { { 0.8f, 0.6f, 0.4f, 0.2f } };
        RGBA8 expected = base + mix(RGBA8(0, 0, 0, 0), color, f);
        return std::make_pair(triangleSpec, expected);
    });
    CheckSrcBlendFactor(base, nxt::BlendFactor::OneMinusBlendColor, nxt::BlendFactor::OneMinusBlendColor, tests);
}

// The following tests check that the Destination blend factor works
TEST_P(BlendStateTest, DstBlendFactorZero) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        return std::make_pair(TriangleSpec({ { color } }), color);
    });
    CheckDstBlendFactor(base, nxt::BlendFactor::Zero, nxt::BlendFactor::Zero, tests);
}

TEST_P(BlendStateTest, DstBlendFactorOne) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        return std::make_pair(TriangleSpec({ { color } }), base + color);
    });
    CheckDstBlendFactor(base, nxt::BlendFactor::One, nxt::BlendFactor::One, tests);
}

TEST_P(BlendStateTest, DstBlendFactorSrcColor) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        RGBA8 fac = color;
        fac.a = 0;
        RGBA8 expected = color + mix(RGBA8(0, 0, 0, 0), base, fac);
        return std::make_pair(TriangleSpec({ { color } }), expected);
    });
    CheckDstBlendFactor(base, nxt::BlendFactor::SrcColor, nxt::BlendFactor::Zero, tests);
}

TEST_P(BlendStateTest, DstBlendFactorOneMinusSrcColor) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        RGBA8 fac = RGBA8(255, 255, 255, 255) - color;
        fac.a = 0;
        RGBA8 expected = color + mix(RGBA8(0, 0, 0, 0), base, fac);
        return std::make_pair(TriangleSpec({ { color } }), expected);
    });
    CheckDstBlendFactor(base, nxt::BlendFactor::OneMinusSrcColor, nxt::BlendFactor::Zero, tests);
}

TEST_P(BlendStateTest, DstBlendFactorSrcAlpha) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        RGBA8 fac(color.a, color.a, color.a, color.a);
        RGBA8 expected = color + mix(RGBA8(0, 0, 0, 0), base, fac);
        return std::make_pair(TriangleSpec({ { color } }), expected);
    });
    CheckDstBlendFactor(base, nxt::BlendFactor::SrcAlpha, nxt::BlendFactor::SrcAlpha, tests);
}

TEST_P(BlendStateTest, DstBlendFactorOneMinusSrcAlpha) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        RGBA8 fac = RGBA8(255, 255, 255, 255) - RGBA8(color.a, color.a, color.a, color.a);
        RGBA8 expected = color + mix(RGBA8(0, 0, 0, 0), base, fac);
        return std::make_pair(TriangleSpec({ { color } }), expected);
    });
    CheckDstBlendFactor(base, nxt::BlendFactor::OneMinusSrcAlpha, nxt::BlendFactor::OneMinusSrcAlpha, tests);
}

TEST_P(BlendStateTest, DstBlendFactorDstColor) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        RGBA8 fac = base;
        fac.a = 0;
        RGBA8 expected = color + mix(RGBA8(0, 0, 0, 0), base, fac);
        return std::make_pair(TriangleSpec({ { color } }), expected);
    });
    CheckDstBlendFactor(base, nxt::BlendFactor::DstColor, nxt::BlendFactor::Zero, tests);
}

TEST_P(BlendStateTest, DstBlendFactorOneMinusDstColor) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        RGBA8 fac = RGBA8(255, 255, 255, 255) - base;
        fac.a = 0;
        RGBA8 expected = color + mix(RGBA8(0, 0, 0, 0), base, fac);
        return std::make_pair(TriangleSpec({ { color } }), expected);
    });
    CheckDstBlendFactor(base, nxt::BlendFactor::OneMinusDstColor, nxt::BlendFactor::Zero, tests);
}

TEST_P(BlendStateTest, DstBlendFactorDstAlpha) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        RGBA8 fac(base.a, base.a, base.a, base.a);
        RGBA8 expected = color + mix(RGBA8(0, 0, 0, 0), base, fac);
        return std::make_pair(TriangleSpec({ { color } }), expected);
    });
    CheckDstBlendFactor(base, nxt::BlendFactor::DstAlpha, nxt::BlendFactor::DstAlpha, tests);
}

TEST_P(BlendStateTest, DstBlendFactorOneMinusDstAlpha) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        RGBA8 fac = RGBA8(255, 255, 255, 255) - RGBA8(base.a, base.a, base.a, base.a);
        RGBA8 expected = color + mix(RGBA8(0, 0, 0, 0), base, fac);
        return std::make_pair(TriangleSpec({ { color } }), expected);
    });
    CheckDstBlendFactor(base, nxt::BlendFactor::OneMinusDstAlpha, nxt::BlendFactor::OneMinusDstAlpha, tests);
}

TEST_P(BlendStateTest, DstBlendFactorSrcAlphaSaturated) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        uint8_t f = std::min(color.a, static_cast<uint8_t>(255 - base.a));
        RGBA8 fac(f, f, f, 255);
        RGBA8 expected = color + mix(RGBA8(0, 0, 0, 0), base, fac);
        return std::make_pair(TriangleSpec({ { color } }), expected);
    });
    CheckDstBlendFactor(base, nxt::BlendFactor::SrcAlphaSaturated, nxt::BlendFactor::SrcAlphaSaturated, tests);
}

TEST_P(BlendStateTest, DstBlendFactorBlendColor) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        auto triangleSpec = TriangleSpec({ { color }, {{ 0.2f, 0.4f, 0.6f, 0.8f }} });
        RGBA8 expected = color + mix(RGBA8(0, 0, 0, 0), base, triangleSpec.blendFactor);
        return std::make_pair(triangleSpec, expected);
    });
    CheckDstBlendFactor(base, nxt::BlendFactor::BlendColor, nxt::BlendFactor::BlendColor, tests);
}

TEST_P(BlendStateTest, DstBlendFactorOneMinusBlendColor) {
    RGBA8 base(32, 64, 128, 192);
    std::vector<std::pair<TriangleSpec, RGBA8>> tests;
    std::transform(kColors.begin(), kColors.end(), std::back_inserter(tests), [&](const RGBA8& color) {
        auto triangleSpec = TriangleSpec({ { color }, {{ 0.2f, 0.4f, 0.6f, 0.8f }} });
        std::array<float, 4> f = { { 0.8f, 0.6f, 0.4f, 0.2f } };
        RGBA8 expected = color + mix(RGBA8(0, 0, 0, 0), base, f);
        return std::make_pair(triangleSpec, expected);
    });
    CheckDstBlendFactor(base, nxt::BlendFactor::OneMinusBlendColor, nxt::BlendFactor::OneMinusBlendColor, tests);
}

// Check that the color write mask works
TEST_P(BlendStateTest, ColorWriteMask) {
    {
        // Test single channel color write
        nxt::BlendState blendState = device.CreateBlendStateBuilder()
            .SetBlendEnabled(true)
            .SetColorBlend(nxt::BlendOperation::Add, nxt::BlendFactor::One, nxt::BlendFactor::One)
            .SetAlphaBlend(nxt::BlendOperation::Add, nxt::BlendFactor::One, nxt::BlendFactor::One)
            .SetColorWriteMask(nxt::ColorWriteMask::Red)
            .GetResult();
        SetupSingleSourcePipelines(blendState);

        RGBA8 base(32, 64, 128, 192);
        for (auto& color : kColors) {
            RGBA8 expected = base + RGBA8(color.r, 0, 0, 0);
            DoSingleSourceTest(base, { color }, expected);
        }
    }

    {
        // Test multi channel color write
        nxt::BlendState blendState = device.CreateBlendStateBuilder()
            .SetBlendEnabled(true)
            .SetColorBlend(nxt::BlendOperation::Add, nxt::BlendFactor::One, nxt::BlendFactor::One)
            .SetAlphaBlend(nxt::BlendOperation::Add, nxt::BlendFactor::One, nxt::BlendFactor::One)
            .SetColorWriteMask(nxt::ColorWriteMask::Green | nxt::ColorWriteMask::Alpha)
            .GetResult();
        SetupSingleSourcePipelines(blendState);

        RGBA8 base(32, 64, 128, 192);
        for (auto& color : kColors) {
            RGBA8 expected = base + RGBA8(0, color.g, 0, color.a);
            DoSingleSourceTest(base, { color }, expected);
        }
    }

    {
        // Test no channel color write
        nxt::BlendState blendState = device.CreateBlendStateBuilder()
            .SetBlendEnabled(true)
            .SetColorBlend(nxt::BlendOperation::Add, nxt::BlendFactor::One, nxt::BlendFactor::One)
            .SetAlphaBlend(nxt::BlendOperation::Add, nxt::BlendFactor::One, nxt::BlendFactor::One)
            .SetColorWriteMask(nxt::ColorWriteMask::None)
            .GetResult();
        SetupSingleSourcePipelines(blendState);

        RGBA8 base(32, 64, 128, 192);
        for (auto& color : kColors) {
            DoSingleSourceTest(base, { color }, base);
        }
    }
}

// Check that the color write mask works when blending is disabled
TEST_P(BlendStateTest, ColorWriteMaskBlendingDisabled) {
    {
        nxt::BlendState blendState = device.CreateBlendStateBuilder()
            .SetBlendEnabled(false)
            .SetColorWriteMask(nxt::ColorWriteMask::Red)
            .GetResult();
        SetupSingleSourcePipelines(blendState);

        RGBA8 base(32, 64, 128, 192);
        RGBA8 expected(32, 0, 0, 0);
        nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
            .BeginRenderPass(renderPass.renderPassInfo)
                .SetRenderPipeline(testPipeline)
                .SetBindGroup(0, MakeBindGroupForColors(std::array<RGBA8, 1>({ { base } })))
                .DrawArrays(3, 1, 0, 0)
            .EndRenderPass()
            .GetResult();

        queue.Submit(1, &commands);
        EXPECT_PIXEL_RGBA8_EQ(expected, renderPass.color, kRTSize / 2, kRTSize / 2);
    }
}

// Test that independent blend states on render targets works
TEST_P(BlendStateTest, IndependentBlendState) {

    std::array<nxt::Texture, 4> renderTargets;
    std::array<nxt::TextureView, 4> renderTargetViews;

    for (uint32_t i = 0; i < 4; ++i) {
        renderTargets[i] = device.CreateTextureBuilder()
            .SetDimension(nxt::TextureDimension::e2D)
            .SetExtent(kRTSize, kRTSize, 1)
            .SetFormat(nxt::TextureFormat::R8G8B8A8Unorm)
            .SetMipLevels(1)
            .SetAllowedUsage(nxt::TextureUsageBit::OutputAttachment | nxt::TextureUsageBit::TransferSrc)
            .SetInitialUsage(nxt::TextureUsageBit::OutputAttachment)
            .GetResult();
        renderTargetViews[i] = renderTargets[i].CreateTextureViewBuilder().GetResult();
    }

    nxt::RenderPassDescriptor renderpass = device.CreateRenderPassDescriptorBuilder()
        .SetColorAttachment(0, renderTargetViews[0], nxt::LoadOp::Clear)
        .SetColorAttachment(1, renderTargetViews[1], nxt::LoadOp::Clear)
        .SetColorAttachment(2, renderTargetViews[2], nxt::LoadOp::Clear)
        .SetColorAttachment(3, renderTargetViews[3], nxt::LoadOp::Clear)
        .GetResult();

    nxt::ShaderModule fsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Fragment, R"(
        #version 450
        layout(set = 0, binding = 0) uniform myBlock {
            vec4 color0;
            vec4 color1;
            vec4 color2;
            vec4 color3;
        } myUbo;

        layout(location = 0) out vec4 fragColor0;
        layout(location = 1) out vec4 fragColor1;
        layout(location = 2) out vec4 fragColor2;
        layout(location = 3) out vec4 fragColor3;

        void main() {
            fragColor0 = myUbo.color0;
            fragColor1 = myUbo.color1;
            fragColor2 = myUbo.color2;
            fragColor3 = myUbo.color3;
        }
    )");

    std::array<nxt::BlendState, 3> blendStates = { {
        device.CreateBlendStateBuilder()
            .SetBlendEnabled(true)
            .SetColorBlend(nxt::BlendOperation::Add, nxt::BlendFactor::One, nxt::BlendFactor::One)
            .SetAlphaBlend(nxt::BlendOperation::Add, nxt::BlendFactor::One, nxt::BlendFactor::One)
            .GetResult(),
        device.CreateBlendStateBuilder()
            .SetBlendEnabled(true)
            .SetColorBlend(nxt::BlendOperation::Subtract, nxt::BlendFactor::One, nxt::BlendFactor::One)
            .SetAlphaBlend(nxt::BlendOperation::Subtract, nxt::BlendFactor::One, nxt::BlendFactor::One)
            .GetResult(),
        device.CreateBlendStateBuilder()
            .SetBlendEnabled(true)
            .SetColorBlend(nxt::BlendOperation::Min, nxt::BlendFactor::One, nxt::BlendFactor::One)
            .SetAlphaBlend(nxt::BlendOperation::Min, nxt::BlendFactor::One, nxt::BlendFactor::One)
            .GetResult(),
    } };

    basePipeline = device.CreateRenderPipelineBuilder()
        .SetColorAttachmentFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        .SetColorAttachmentFormat(1, nxt::TextureFormat::R8G8B8A8Unorm)
        .SetColorAttachmentFormat(2, nxt::TextureFormat::R8G8B8A8Unorm)
        .SetColorAttachmentFormat(3, nxt::TextureFormat::R8G8B8A8Unorm)
        .SetLayout(pipelineLayout)
        .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
        .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
        .GetResult();

    testPipeline = device.CreateRenderPipelineBuilder()
        .SetColorAttachmentFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        .SetColorAttachmentFormat(1, nxt::TextureFormat::R8G8B8A8Unorm)
        .SetColorAttachmentFormat(2, nxt::TextureFormat::R8G8B8A8Unorm)
        .SetColorAttachmentFormat(3, nxt::TextureFormat::R8G8B8A8Unorm)
        .SetLayout(pipelineLayout)
        .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
        .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
        .SetColorAttachmentBlendState(0, blendStates[0])
        .SetColorAttachmentBlendState(1, blendStates[1])
        // Blend state not set on third color attachment. It should be default
        .SetColorAttachmentBlendState(3, blendStates[2])
        .GetResult();


    for (unsigned int c = 0; c < kColors.size(); ++c) {
        RGBA8 base = kColors[((c + 31) * 29) % kColors.size()];
        RGBA8 color0 = kColors[((c + 19) * 13) % kColors.size()];
        RGBA8 color1 = kColors[((c + 11) * 43) % kColors.size()];
        RGBA8 color2 = kColors[((c + 7) * 3) % kColors.size()];
        RGBA8 color3 = kColors[((c + 13) * 71) % kColors.size()];

        RGBA8 expected0 = color0 + base;
        RGBA8 expected1 = color1 - base;
        RGBA8 expected2 = color2;
        RGBA8 expected3 = min(color3, base);

        renderTargets[0].TransitionUsage(nxt::TextureUsageBit::OutputAttachment);
        renderTargets[1].TransitionUsage(nxt::TextureUsageBit::OutputAttachment);
        renderTargets[2].TransitionUsage(nxt::TextureUsageBit::OutputAttachment);
        renderTargets[3].TransitionUsage(nxt::TextureUsageBit::OutputAttachment);

        nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
            .BeginRenderPass(renderpass)
                .SetRenderPipeline(basePipeline)
                .SetBindGroup(0, MakeBindGroupForColors(std::array<RGBA8, 4>({ { base, base, base, base } })))
                .DrawArrays(3, 1, 0, 0)

                .SetRenderPipeline(testPipeline)
                .SetBindGroup(0, MakeBindGroupForColors(std::array<RGBA8, 4>({ { color0, color1, color2, color3 } })))
                .DrawArrays(3, 1, 0, 0)
            .EndRenderPass()
            .GetResult();

        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(expected0, renderTargets[0], kRTSize / 2, kRTSize / 2) << "Attachment slot 0 should have been " << color0 << " + " << base << " = " << expected0;
        EXPECT_PIXEL_RGBA8_EQ(expected1, renderTargets[1], kRTSize / 2, kRTSize / 2) << "Attachment slot 1 should have been " << color1 << " - " << base << " = " << expected1;
        EXPECT_PIXEL_RGBA8_EQ(expected2, renderTargets[2], kRTSize / 2, kRTSize / 2) << "Attachment slot 2 should have been " << color2 << " = " << expected2 << "(no blending)";
        EXPECT_PIXEL_RGBA8_EQ(expected3, renderTargets[3], kRTSize / 2, kRTSize / 2) << "Attachment slot 3 should have been min(" << color3 << ", " << base << ") = " << expected3;
    }
}

// Test that the default blend color is correctly set at the beginning of every subpass
TEST_P(BlendStateTest, DefaultBlendColor) {
    nxt::BlendState blendState = device.CreateBlendStateBuilder()
        .SetBlendEnabled(true)
        .SetColorBlend(nxt::BlendOperation::Add, nxt::BlendFactor::BlendColor, nxt::BlendFactor::One)
        .SetAlphaBlend(nxt::BlendOperation::Add, nxt::BlendFactor::BlendColor, nxt::BlendFactor::One)
        .GetResult();

    nxt::ShaderModule fsModule = utils::CreateShaderModule(device, nxt::ShaderStage::Fragment, R"(
        #version 450
        layout(set = 0, binding = 0) uniform myBlock {
            vec4 color;
        } myUbo;

        layout(location = 0) out vec4 fragColor;

        void main() {
            fragColor = myUbo.color;
        }
    )");

    basePipeline = device.CreateRenderPipelineBuilder()
        .SetColorAttachmentFormat(0, renderPass.colorFormat)
        .SetLayout(pipelineLayout)
        .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
        .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
        .GetResult();

    testPipeline = device.CreateRenderPipelineBuilder()
        .SetColorAttachmentFormat(0, renderPass.colorFormat)
        .SetLayout(pipelineLayout)
        .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
        .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
        .SetColorAttachmentBlendState(0, blendState)
        .GetResult();

    // Check that the initial blend color is (0,0,0,0)
    {
        nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
            .BeginRenderPass(renderPass.renderPassInfo)
                .SetRenderPipeline(basePipeline)
                .SetBindGroup(0, MakeBindGroupForColors(std::array<RGBA8, 1>({ { RGBA8(0, 0, 0, 0) } })))
                .DrawArrays(3, 1, 0, 0)
                .SetRenderPipeline(testPipeline)
                .SetBindGroup(0, MakeBindGroupForColors(std::array<RGBA8, 1>({ { RGBA8(255, 255, 255, 255) } })))
                .DrawArrays(3, 1, 0, 0)
            .EndRenderPass()
            .GetResult();

        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 0, 0, 0), renderPass.color, kRTSize / 2, kRTSize / 2);
    }

    // Check that setting the blend color works
    {
        nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
            .BeginRenderPass(renderPass.renderPassInfo)
                .SetRenderPipeline(basePipeline)
                .SetBindGroup(0, MakeBindGroupForColors(std::array<RGBA8, 1>({ { RGBA8(0, 0, 0, 0) } })))
                .DrawArrays(3, 1, 0, 0)
                .SetRenderPipeline(testPipeline)
                .SetBlendColor(1, 1, 1, 1)
                .SetBindGroup(0, MakeBindGroupForColors(std::array<RGBA8, 1>({ { RGBA8(255, 255, 255, 255) } })))
                .DrawArrays(3, 1, 0, 0)
            .EndRenderPass()
            .GetResult();

        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(RGBA8(255, 255, 255, 255), renderPass.color, kRTSize / 2, kRTSize / 2);
    }

    // Check that the blend color is not inherited between render passes
    {
        nxt::CommandBuffer commands = device.CreateCommandBufferBuilder()
            .BeginRenderPass(renderPass.renderPassInfo)
                .SetRenderPipeline(basePipeline)
                .SetBindGroup(0, MakeBindGroupForColors(std::array<RGBA8, 1>({ { RGBA8(0, 0, 0, 0) } })))
                .DrawArrays(3, 1, 0, 0)
                .SetRenderPipeline(testPipeline)
                .SetBlendColor(1, 1, 1, 1)
                .SetBindGroup(0, MakeBindGroupForColors(std::array<RGBA8, 1>({ { RGBA8(255, 255, 255, 255) } })))
                .DrawArrays(3, 1, 0, 0)
            .EndRenderPass()
            .BeginRenderPass(renderPass.renderPassInfo)
                .SetRenderPipeline(basePipeline)
                .SetBindGroup(0, MakeBindGroupForColors(std::array<RGBA8, 1>({ { RGBA8(0, 0, 0, 0) } })))
                .DrawArrays(3, 1, 0, 0)
                .SetRenderPipeline(testPipeline)
                .SetBindGroup(0, MakeBindGroupForColors(std::array<RGBA8, 1>({ { RGBA8(255, 255, 255, 255) } })))
                .DrawArrays(3, 1, 0, 0)
            .EndRenderPass()
            .GetResult();

        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(RGBA8(0, 0, 0, 0), renderPass.color, kRTSize / 2, kRTSize / 2);
    }
}

NXT_INSTANTIATE_TEST(BlendStateTest, D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend)
