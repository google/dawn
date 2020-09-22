// Copyright 2017 The Dawn Authors
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

#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

#include <array>

constexpr static unsigned int kRTSize = 16;

class DrawQuad {
  public:
    DrawQuad() {
    }
    DrawQuad(wgpu::Device device, const char* vsSource, const char* fsSource) : device(device) {
        vsModule = utils::CreateShaderModule(device, utils::SingleShaderStage::Vertex, vsSource);
        fsModule = utils::CreateShaderModule(device, utils::SingleShaderStage::Fragment, fsSource);

        pipelineLayout = utils::MakeBasicPipelineLayout(device, nullptr);
    }

    void Draw(wgpu::RenderPassEncoder* pass) {
        utils::ComboRenderPipelineDescriptor descriptor(device);
        descriptor.layout = pipelineLayout;
        descriptor.vertexStage.module = vsModule;
        descriptor.cFragmentStage.module = fsModule;

        auto renderPipeline = device.CreateRenderPipeline(&descriptor);

        pass->SetPipeline(renderPipeline);
        pass->Draw(6, 1, 0, 0);
    }

  private:
    wgpu::Device device;
    wgpu::ShaderModule vsModule = {};
    wgpu::ShaderModule fsModule = {};
    wgpu::PipelineLayout pipelineLayout = {};
};

class RenderPassLoadOpTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.width = kRTSize;
        descriptor.size.height = kRTSize;
        descriptor.size.depth = 1;
        descriptor.sampleCount = 1;
        descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.mipLevelCount = 1;
        descriptor.usage = wgpu::TextureUsage::OutputAttachment | wgpu::TextureUsage::CopySrc;
        renderTarget = device.CreateTexture(&descriptor);

        renderTargetView = renderTarget.CreateView();

        std::fill(expectZero.begin(), expectZero.end(), RGBA8::kZero);

        std::fill(expectGreen.begin(), expectGreen.end(), RGBA8::kGreen);

        std::fill(expectBlue.begin(), expectBlue.end(), RGBA8::kBlue);

        // draws a blue quad on the right half of the screen
        const char* vsSource = R"(
                #version 450
                void main() {
                    const vec2 pos[6] = vec2[6](
                        vec2(0, -1), vec2(1, -1), vec2(0, 1),
                        vec2(0,  1), vec2(1, -1), vec2(1, 1));
                    gl_Position = vec4(pos[gl_VertexIndex], 0.f, 1.f);
                }
                )";
        const char* fsSource = R"(
                #version 450
                layout(location = 0) out vec4 color;
                void main() {
                    color = vec4(0.f, 0.f, 1.f, 1.f);
                }
                )";
        blueQuad = DrawQuad(device, vsSource, fsSource);
    }

    template <class T>
    void TestIntegerClearColor(wgpu::TextureFormat format,
                               const wgpu::Color& clearColor,
                               const std::array<T, 4>& expectedPixelValue) {
        constexpr wgpu::Extent3D kTextureSize = {1, 1, 1};

        wgpu::TextureDescriptor textureDescriptor;
        textureDescriptor.dimension = wgpu::TextureDimension::e2D;
        textureDescriptor.size = kTextureSize;
        textureDescriptor.usage =
            wgpu::TextureUsage::OutputAttachment | wgpu::TextureUsage::CopySrc;
        textureDescriptor.format = format;
        wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

        utils::ComboRenderPassDescriptor renderPassDescriptor({texture.CreateView()});
        renderPassDescriptor.cColorAttachments[0].clearColor = clearColor;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.EndPass();

        const uint64_t bufferSize = sizeof(T) * expectedPixelValue.size();
        wgpu::BufferDescriptor bufferDescriptor;
        bufferDescriptor.size = bufferSize;
        bufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

        wgpu::TextureCopyView textureCopyView = utils::CreateTextureCopyView(texture, 0, {0, 0, 0});
        wgpu::BufferCopyView bufferCopyView =
            utils::CreateBufferCopyView(buffer, 0, kTextureBytesPerRowAlignment, 0);
        encoder.CopyTextureToBuffer(&textureCopyView, &bufferCopyView, &kTextureSize);

        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);

        EXPECT_BUFFER_U32_RANGE_EQ(reinterpret_cast<const uint32_t*>(expectedPixelValue.data()),
                                   buffer, 0, bufferSize / sizeof(uint32_t));
    }

    wgpu::Texture renderTarget;
    wgpu::TextureView renderTargetView;

    std::array<RGBA8, kRTSize * kRTSize> expectZero;
    std::array<RGBA8, kRTSize * kRTSize> expectGreen;
    std::array<RGBA8, kRTSize * kRTSize> expectBlue;

    DrawQuad blueQuad = {};
};

// Tests clearing, loading, and drawing into color attachments
TEST_P(RenderPassLoadOpTests, ColorClearThenLoadAndDraw) {
    // Part 1: clear once, check to make sure it's cleared
    utils::ComboRenderPassDescriptor renderPassClearZero({renderTargetView});
    auto commandsClearZeroEncoder = device.CreateCommandEncoder();
    auto clearZeroPass = commandsClearZeroEncoder.BeginRenderPass(&renderPassClearZero);
    clearZeroPass.EndPass();
    auto commandsClearZero = commandsClearZeroEncoder.Finish();

    utils::ComboRenderPassDescriptor renderPassClearGreen({renderTargetView});
    renderPassClearGreen.cColorAttachments[0].clearColor = {0.0f, 1.0f, 0.0f, 1.0f};
    auto commandsClearGreenEncoder = device.CreateCommandEncoder();
    auto clearGreenPass = commandsClearGreenEncoder.BeginRenderPass(&renderPassClearGreen);
    clearGreenPass.EndPass();
    auto commandsClearGreen = commandsClearGreenEncoder.Finish();

    queue.Submit(1, &commandsClearZero);
    EXPECT_TEXTURE_RGBA8_EQ(expectZero.data(), renderTarget, 0, 0, kRTSize, kRTSize, 0, 0);

    queue.Submit(1, &commandsClearGreen);
    EXPECT_TEXTURE_RGBA8_EQ(expectGreen.data(), renderTarget, 0, 0, kRTSize, kRTSize, 0, 0);

    // Part 2: draw a blue quad into the right half of the render target, and check result
    utils::ComboRenderPassDescriptor renderPassLoad({renderTargetView});
    renderPassLoad.cColorAttachments[0].loadOp = wgpu::LoadOp::Load;
    wgpu::CommandBuffer commandsLoad;
    {
        auto encoder = device.CreateCommandEncoder();
        auto pass = encoder.BeginRenderPass(&renderPassLoad);
        blueQuad.Draw(&pass);
        pass.EndPass();
        commandsLoad = encoder.Finish();
    }

    queue.Submit(1, &commandsLoad);
    // Left half should still be green
    EXPECT_TEXTURE_RGBA8_EQ(expectGreen.data(), renderTarget, 0, 0, kRTSize / 2, kRTSize, 0, 0);
    // Right half should now be blue
    EXPECT_TEXTURE_RGBA8_EQ(expectBlue.data(), renderTarget, kRTSize / 2, 0, kRTSize / 2, kRTSize,
                            0, 0);
}

// Test clearing a color attachment with signed and unsigned integer formats.
TEST_P(RenderPassLoadOpTests, LoadOpClearOnIntegerFormats) {
    // RGBA8Uint
    {
        constexpr wgpu::Color kClearColor = {2.f, 3.3f, 254.8f, 255.0f};
        constexpr std::array<uint8_t, 4> kExpectedPixelValue = {2, 3, 254, 255};
        TestIntegerClearColor<uint8_t>(wgpu::TextureFormat::RGBA8Uint, kClearColor,
                                       kExpectedPixelValue);
    }

    // RGBA8Sint
    {
        constexpr wgpu::Color kClearColor = {2.f, -3.3f, 126.8f, -128.0f};
        constexpr std::array<int8_t, 4> kExpectedPixelValue = {2, -3, 126, -128};
        TestIntegerClearColor<int8_t>(wgpu::TextureFormat::RGBA8Sint, kClearColor,
                                      kExpectedPixelValue);
    }

    // RGBA16Uint
    {
        constexpr wgpu::Color kClearColor = {2.f, 3.3f, 512.7f, 65535.f};
        constexpr std::array<uint16_t, 4> kExpectedPixelValue = {2, 3, 512, 65535u};
        TestIntegerClearColor<uint16_t>(wgpu::TextureFormat::RGBA16Uint, kClearColor,
                                        kExpectedPixelValue);
    }

    // RGBA16Sint
    {
        constexpr wgpu::Color kClearColor = {2.f, -3.3f, 32767.8f, -32768.0f};
        constexpr std::array<int16_t, 4> kExpectedPixelValue = {2, -3, 32767, -32768};
        TestIntegerClearColor<int16_t>(wgpu::TextureFormat::RGBA16Sint, kClearColor,
                                       kExpectedPixelValue);
    }

    // RGBA32Uint
    {
        constexpr wgpu::Color kClearColor = {2.f, 3.3f, 65534.8f, 65537.f};
        constexpr std::array<uint32_t, 4> kExpectedPixelValue = {2, 3, 65534, 65537};
        TestIntegerClearColor<uint32_t>(wgpu::TextureFormat::RGBA32Uint, kClearColor,
                                        kExpectedPixelValue);
    }

    // RGBA32Sint
    {
        constexpr wgpu::Color kClearColor = {2.f, -3.3f, 65534.8f, -65537.f};
        constexpr std::array<int32_t, 4> kExpectedPixelValue = {2, -3, 65534, -65537};
        TestIntegerClearColor<int32_t>(wgpu::TextureFormat::RGBA32Sint, kClearColor,
                                       kExpectedPixelValue);
    }
}

// This test verifies that input double values are being rounded to floats internally when
// clearing.
TEST_P(RenderPassLoadOpTests, LoadOpClearLargeIntegerValueRounding) {
    // Intel GPUs fail when we attempt to clear to a value that exceeds 2147483647 on a RGBA32Uint
    // texture.
    // Bug: dawn:530
    DAWN_SKIP_TEST_IF(IsIntel() && IsD3D12());

    // RGBA32Uint
    {
        constexpr wgpu::Color kClearColor = {4194966911.0, 3555555555.0, 2555555555.0,
                                             1555555555.0};
        constexpr std::array<uint32_t, 4> kExpectedPixelValue = {4194966784, 3555555584, 2555555584,
                                                                 1555555584};
        TestIntegerClearColor<uint32_t>(wgpu::TextureFormat::RGBA32Uint, kClearColor,
                                        kExpectedPixelValue);
    }

    // RGBA32Sint
    {
        constexpr wgpu::Color kClearColor = {2147483447.0, -2147483447.0, 1000000555.0,
                                             -1000000555.0};
        constexpr std::array<int32_t, 4> kExpectedPixelValue = {2147483392, -2147483392, 1000000576,
                                                                -1000000576};
        TestIntegerClearColor<int32_t>(wgpu::TextureFormat::RGBA32Sint, kClearColor,
                                       kExpectedPixelValue);
    }
}

DAWN_INSTANTIATE_TEST(RenderPassLoadOpTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      VulkanBackend());
