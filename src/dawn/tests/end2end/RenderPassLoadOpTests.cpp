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

#include <array>

#include "dawn/tests/DawnTest.h"

#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

constexpr static unsigned int kRTSize = 16;

class DrawQuad {
  public:
    DrawQuad() {}
    DrawQuad(wgpu::Device device, const char* vsSource, const char* fsSource) : device(device) {
        vsModule = utils::CreateShaderModule(device, vsSource);
        fsModule = utils::CreateShaderModule(device, fsSource);

        pipelineLayout = utils::MakeBasicPipelineLayout(device, nullptr);
    }

    void Draw(wgpu::RenderPassEncoder* pass) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.layout = pipelineLayout;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;

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
        descriptor.size.depthOrArrayLayers = 1;
        descriptor.sampleCount = 1;
        descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.mipLevelCount = 1;
        descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        renderTarget = device.CreateTexture(&descriptor);

        renderTargetView = renderTarget.CreateView();

        std::fill(expectZero.begin(), expectZero.end(), RGBA8::kZero);

        std::fill(expectGreen.begin(), expectGreen.end(), RGBA8::kGreen);

        std::fill(expectBlue.begin(), expectBlue.end(), RGBA8::kBlue);

        // draws a blue quad on the right half of the screen
        const char* vsSource = R"(
            @stage(vertex)
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4<f32> {
                var pos = array<vec2<f32>, 6>(
                    vec2<f32>( 0.0, -1.0),
                    vec2<f32>( 1.0, -1.0),
                    vec2<f32>( 0.0,  1.0),
                    vec2<f32>( 0.0,  1.0),
                    vec2<f32>( 1.0, -1.0),
                    vec2<f32>( 1.0,  1.0));

                return vec4<f32>(pos[VertexIndex], 0.0, 1.0);
            })";

        const char* fsSource = R"(
            @stage(fragment) fn main() -> @location(0) vec4<f32> {
                return vec4<f32>(0.0, 0.0, 1.0, 1.0);
            })";
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
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        textureDescriptor.format = format;
        wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

        utils::ComboRenderPassDescriptor renderPassDescriptor({texture.CreateView()});
        renderPassDescriptor.cColorAttachments[0].clearValue = clearColor;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.End();

        const uint64_t bufferSize = sizeof(T) * expectedPixelValue.size();
        wgpu::BufferDescriptor bufferDescriptor;
        bufferDescriptor.size = bufferSize;
        bufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

        wgpu::ImageCopyTexture imageCopyTexture =
            utils::CreateImageCopyTexture(texture, 0, {0, 0, 0});
        wgpu::ImageCopyBuffer imageCopyBuffer =
            utils::CreateImageCopyBuffer(buffer, 0, kTextureBytesPerRowAlignment);
        encoder.CopyTextureToBuffer(&imageCopyTexture, &imageCopyBuffer, &kTextureSize);

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
    clearZeroPass.End();
    auto commandsClearZero = commandsClearZeroEncoder.Finish();

    utils::ComboRenderPassDescriptor renderPassClearGreen({renderTargetView});
    renderPassClearGreen.cColorAttachments[0].clearValue = {0.0f, 1.0f, 0.0f, 1.0f};
    auto commandsClearGreenEncoder = device.CreateCommandEncoder();
    auto clearGreenPass = commandsClearGreenEncoder.BeginRenderPass(&renderPassClearGreen);
    clearGreenPass.End();
    auto commandsClearGreen = commandsClearGreenEncoder.Finish();

    queue.Submit(1, &commandsClearZero);
    EXPECT_TEXTURE_EQ(expectZero.data(), renderTarget, {0, 0}, {kRTSize, kRTSize});

    queue.Submit(1, &commandsClearGreen);
    EXPECT_TEXTURE_EQ(expectGreen.data(), renderTarget, {0, 0}, {kRTSize, kRTSize});

    // Part 2: draw a blue quad into the right half of the render target, and check result
    utils::ComboRenderPassDescriptor renderPassLoad({renderTargetView});
    renderPassLoad.cColorAttachments[0].loadOp = wgpu::LoadOp::Load;
    wgpu::CommandBuffer commandsLoad;
    {
        auto encoder = device.CreateCommandEncoder();
        auto pass = encoder.BeginRenderPass(&renderPassLoad);
        blueQuad.Draw(&pass);
        pass.End();
        commandsLoad = encoder.Finish();
    }

    queue.Submit(1, &commandsLoad);
    // Left half should still be green
    EXPECT_TEXTURE_EQ(expectGreen.data(), renderTarget, {0, 0}, {kRTSize / 2, kRTSize});
    // Right half should now be blue
    EXPECT_TEXTURE_EQ(expectBlue.data(), renderTarget, {kRTSize / 2, 0}, {kRTSize / 2, kRTSize});
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

// This test verifies that input double values are being rendered correctly when clearing.
TEST_P(RenderPassLoadOpTests, LoadOpClearIntegerFormatsToLargeValues) {
    // TODO(http://crbug.com/dawn/537): Implemement a workaround to enable clearing integer formats
    // to large values on D3D12.
    DAWN_SUPPRESS_TEST_IF(IsD3D12());

    // TODO(crbug.com/dawn/1109): Re-enable once fixed on Mac Mini 8,1s w/ 11.5.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel() && IsMacOS(11, 5));

    constexpr double kUint32MaxDouble = 4294967295.0;
    constexpr uint32_t kUint32Max = static_cast<uint32_t>(kUint32MaxDouble);
    // RGBA32Uint for UINT32_MAX
    {
        constexpr wgpu::Color kClearColor = {kUint32MaxDouble, kUint32MaxDouble, kUint32MaxDouble,
                                             kUint32MaxDouble};
        constexpr std::array<uint32_t, 4> kExpectedPixelValue = {kUint32Max, kUint32Max, kUint32Max,
                                                                 kUint32Max};
        TestIntegerClearColor<uint32_t>(wgpu::TextureFormat::RGBA32Uint, kClearColor,
                                        kExpectedPixelValue);
    }

    constexpr double kSint32MaxDouble = 2147483647.0;
    constexpr int32_t kSint32Max = static_cast<int32_t>(kSint32MaxDouble);

    constexpr double kSint32MinDouble = -2147483648.0;
    constexpr int32_t kSint32Min = static_cast<int32_t>(kSint32MinDouble);

    // RGBA32Sint for SINT32 upper bound.
    {
        constexpr wgpu::Color kClearColor = {kSint32MaxDouble, kSint32MaxDouble, kSint32MaxDouble,
                                             kSint32MaxDouble};
        constexpr std::array<int32_t, 4> kExpectedPixelValue = {kSint32Max, kSint32Max, kSint32Max,
                                                                kSint32Max};
        TestIntegerClearColor<int32_t>(wgpu::TextureFormat::RGBA32Sint, kClearColor,
                                       kExpectedPixelValue);
    }

    // RGBA32Sint for SINT32 lower bound.
    {
        constexpr wgpu::Color kClearColor = {kSint32MinDouble, kSint32MinDouble, kSint32MinDouble,
                                             kSint32MinDouble};
        constexpr std::array<int32_t, 4> kExpectedPixelValue = {kSint32Min, kSint32Min, kSint32Min,
                                                                kSint32Min};
        TestIntegerClearColor<int32_t>(wgpu::TextureFormat::RGBA32Sint, kClearColor,
                                       kExpectedPixelValue);
    }
}

DAWN_INSTANTIATE_TEST(RenderPassLoadOpTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
