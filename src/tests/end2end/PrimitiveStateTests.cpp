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

#include "common/Assert.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

constexpr static unsigned int kRTSize = 1;

class DepthClampingTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({"depth-clamping"}));

        wgpu::TextureDescriptor renderTargetDescriptor;
        renderTargetDescriptor.size = {kRTSize, kRTSize};
        renderTargetDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        renderTargetDescriptor.usage =
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        renderTarget = device.CreateTexture(&renderTargetDescriptor);

        renderTargetView = renderTarget.CreateView();

        wgpu::TextureDescriptor depthDescriptor;
        depthDescriptor.dimension = wgpu::TextureDimension::e2D;
        depthDescriptor.size = {kRTSize, kRTSize};
        depthDescriptor.format = wgpu::TextureFormat::Depth24PlusStencil8;
        depthDescriptor.usage = wgpu::TextureUsage::RenderAttachment;
        depthTexture = device.CreateTexture(&depthDescriptor);

        depthTextureView = depthTexture.CreateView();

        vsModule = utils::CreateShaderModule(device, R"(
            [[block]] struct UBO {
                color : vec3<f32>;
                depth : f32;
            };
            [[group(0), binding(0)]] var<uniform> ubo : UBO;

            [[stage(vertex)]] fn main() -> [[builtin(position)]] vec4<f32> {
                return vec4<f32>(0.0, 0.0, ubo.depth, 1.0);
            })");

        fsModule = utils::CreateShaderModule(device, R"(
            [[block]] struct UBO {
                color : vec3<f32>;
                depth : f32;
            };
            [[group(0), binding(0)]] var<uniform> ubo : UBO;

            [[stage(fragment)]] fn main() -> [[location(0)]] vec4<f32> {
                return vec4<f32>(ubo.color, 1.0);
            })");
    }

    std::vector<const char*> GetRequiredFeatures() override {
        std::vector<const char*> requiredFeatures = {};
        if (SupportsFeatures({"depth-clamping"})) {
            requiredFeatures.push_back("depth-clamping");
        }
        return requiredFeatures;
    }

    struct TestSpec {
        wgpu::PrimitiveDepthClampingState* depthClampingState;
        RGBA8 color;
        float depth;
        wgpu::CompareFunction depthCompareFunction;
    };

    // Each test param represents a pair of triangles with a color, depth, stencil value, and
    // depthStencil state, one frontfacing, one backfacing Draw the triangles in order and check the
    // expected colors for the frontfaces and backfaces
    void DoTest(const std::vector<TestSpec>& testParams, const RGBA8& expected) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        struct TriangleData {
            float color[3];
            float depth;
        };

        utils::ComboRenderPassDescriptor renderPass({renderTargetView}, depthTextureView);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);

        for (size_t i = 0; i < testParams.size(); ++i) {
            const TestSpec& test = testParams[i];

            TriangleData data = {
                {static_cast<float>(test.color.r) / 255.f, static_cast<float>(test.color.g) / 255.f,
                 static_cast<float>(test.color.b) / 255.f},
                test.depth,
            };
            // Upload a buffer for each triangle's depth and color data
            wgpu::Buffer buffer = utils::CreateBufferFromData(device, &data, sizeof(TriangleData),
                                                              wgpu::BufferUsage::Uniform);

            // Create a pipeline for the triangles with the test spec's params.
            utils::ComboRenderPipelineDescriptor descriptor;
            descriptor.primitive.nextInChain = test.depthClampingState;
            descriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;
            descriptor.vertex.module = vsModule;
            descriptor.cFragment.module = fsModule;
            wgpu::DepthStencilState* depthStencil = descriptor.EnableDepthStencil();
            depthStencil->depthWriteEnabled = true;
            depthStencil->depthCompare = test.depthCompareFunction;
            depthStencil->format = wgpu::TextureFormat::Depth24PlusStencil8;

            wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

            // Create a bind group for the data
            wgpu::BindGroup bindGroup = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(0), {{0, buffer}});

            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(1);
        }
        pass.EndPass();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(expected, renderTarget,  0, 0) << "Pixel check failed";
    }

    wgpu::Texture renderTarget;
    wgpu::Texture depthTexture;
    wgpu::TextureView renderTargetView;
    wgpu::TextureView depthTextureView;
    wgpu::ShaderModule vsModule;
    wgpu::ShaderModule fsModule;
};

// Test that fragments beyond the far plane are clamped to 1.0 if depth clamping is enabled.
TEST_P(DepthClampingTest, ClampOnBeyondFarPlane) {
    wgpu::PrimitiveDepthClampingState clampingState;
    clampingState.clampDepth = true;

    DoTest(
        {
            // Draw a red triangle at depth 1.
            {
                nullptr, /* depthClampingState */
                RGBA8(255, 0, 0, 255), /* color */
                1.f, /* depth */
                wgpu::CompareFunction::Always,
            },
            // Draw a green triangle at depth 2 which should get clamped to 1.
            {
                &clampingState,
                RGBA8(0, 255, 0, 255), /* color */
                2.f, /* depth */
                wgpu::CompareFunction::Equal,
            },
        },
        // Since we draw the green triangle with an "equal" depth compare function, the resulting
        // fragment should be green.
        RGBA8(0, 255, 0, 255));
}

// Test that fragments beyond the near plane are clamped to 0.0 if depth clamping is enabled.
TEST_P(DepthClampingTest, ClampOnBeyondNearPlane) {
    wgpu::PrimitiveDepthClampingState clampingState;
    clampingState.clampDepth = true;

    DoTest(
        {
            // Draw a red triangle at depth 0.
            {
                nullptr, /* depthClampingState */
                RGBA8(255, 0, 0, 255), /* color */
                0.f, /* depth */
                wgpu::CompareFunction::Always,
            },
            // Draw a green triangle at depth -1 which should get clamped to 0.
            {
                &clampingState,
                RGBA8(0, 255, 0, 255), /* color */
                -1.f, /* depth */
                wgpu::CompareFunction::Equal,
            },
        },
        // Since we draw the green triangle with an "equal" depth compare function, the resulting
        // fragment should be green.
        RGBA8(0, 255, 0, 255));
}

// Test that fragments inside the view frustum are unaffected by depth clamping.
TEST_P(DepthClampingTest, ClampOnInsideViewFrustum) {
    wgpu::PrimitiveDepthClampingState clampingState;
    clampingState.clampDepth = true;

    DoTest(
        {
            {
                &clampingState,
                RGBA8(0, 255, 0, 255), /* color */
                0.5f, /* depth */
                wgpu::CompareFunction::Always,
            },
        },
        RGBA8(0, 255, 0, 255));
}


// Test that fragments outside the view frustum are clipped if depth clamping is disabled.
TEST_P(DepthClampingTest, ClampOffOutsideViewFrustum) {
    wgpu::PrimitiveDepthClampingState clampingState;
    clampingState.clampDepth = false;

    DoTest(
        {
            {
                &clampingState,
                RGBA8(0, 255, 0, 255), /* color */
                2.f, /* depth */
                wgpu::CompareFunction::Always,
            },
            {
                &clampingState,
                RGBA8(0, 255, 0, 255), /* color */
                -1.f, /* depth */
                wgpu::CompareFunction::Always,
            },
        },
        RGBA8(0, 0, 0, 0));
}

// Test that fragments outside the view frustum are clipped if clampDepth is left unspecified.
TEST_P(DepthClampingTest, ClampUnspecifiedOutsideViewFrustum) {
    DoTest(
        {
            {
                nullptr, /* depthClampingState */
                RGBA8(0, 255, 0, 255), /* color */
                -1.f, /* depth */
                wgpu::CompareFunction::Always,
            },
            {
                nullptr, /* depthClampingState */
                RGBA8(0, 255, 0, 255), /* color */
                2.f, /* depth */
                wgpu::CompareFunction::Always,
            },
        },
        RGBA8(0, 0, 0, 0));
}

// Test that fragments are properly clipped or clamped if multiple render pipelines are used
// within the same render pass with differing clampDepth values.
TEST_P(DepthClampingTest, MultipleRenderPipelines) {
    wgpu::PrimitiveDepthClampingState clampingState;
    clampingState.clampDepth = true;

    wgpu::PrimitiveDepthClampingState clippingState;
    clippingState.clampDepth = false;

    DoTest(
        {
            // Draw green with clamping
            {
                &clampingState,
                RGBA8(0, 255, 0, 255), /* color */
                2.f, /* depth */
                wgpu::CompareFunction::Always,
            },
            // Draw red with clipping
            {
                &clippingState,
                RGBA8(255, 0, 0, 255), /* color */
                2.f, /* depth */
                wgpu::CompareFunction::Always,
            },
        },
        RGBA8(0, 255, 0, 255)); // Result should be green
}

DAWN_INSTANTIATE_TEST(DepthClampingTest,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
