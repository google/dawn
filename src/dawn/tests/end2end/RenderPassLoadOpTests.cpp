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
#include <limits>
#include <tuple>

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
            @vertex
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
            @fragment fn main() -> @location(0) vec4<f32> {
                return vec4<f32>(0.0, 0.0, 1.0, 1.0);
            })";
        blueQuad = DrawQuad(device, vsSource, fsSource);
    }

    template <class T>
    void TestClearColor(wgpu::TextureFormat format,
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

        EXPECT_BUFFER_U8_RANGE_EQ(reinterpret_cast<const uint8_t*>(expectedPixelValue.data()),
                                  buffer, 0, bufferSize / sizeof(uint8_t));
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
        TestClearColor<uint8_t>(wgpu::TextureFormat::RGBA8Uint, kClearColor, kExpectedPixelValue);
    }

    // RGBA8Sint
    {
        constexpr wgpu::Color kClearColor = {2.f, -3.3f, 126.8f, -128.0f};
        constexpr std::array<int8_t, 4> kExpectedPixelValue = {2, -3, 126, -128};
        TestClearColor<int8_t>(wgpu::TextureFormat::RGBA8Sint, kClearColor, kExpectedPixelValue);
    }

    // RGBA16Uint
    {
        constexpr wgpu::Color kClearColor = {2.f, 3.3f, 512.7f, 65535.f};
        constexpr std::array<uint16_t, 4> kExpectedPixelValue = {2, 3, 512, 65535u};
        TestClearColor<uint16_t>(wgpu::TextureFormat::RGBA16Uint, kClearColor, kExpectedPixelValue);
    }

    // RGBA16Sint
    {
        constexpr wgpu::Color kClearColor = {2.f, -3.3f, 32767.8f, -32768.0f};
        constexpr std::array<int16_t, 4> kExpectedPixelValue = {2, -3, 32767, -32768};
        TestClearColor<int16_t>(wgpu::TextureFormat::RGBA16Sint, kClearColor, kExpectedPixelValue);
    }

    // RGBA32Uint
    {
        constexpr wgpu::Color kClearColor = {2.f, 3.3f, 65534.8f, 65537.f};
        constexpr std::array<uint32_t, 4> kExpectedPixelValue = {2, 3, 65534, 65537};
        TestClearColor<uint32_t>(wgpu::TextureFormat::RGBA32Uint, kClearColor, kExpectedPixelValue);
    }

    // RGBA32Sint
    {
        constexpr wgpu::Color kClearColor = {2.f, -3.3f, 65534.8f, -65537.f};
        constexpr std::array<int32_t, 4> kExpectedPixelValue = {2, -3, 65534, -65537};
        TestClearColor<int32_t>(wgpu::TextureFormat::RGBA32Sint, kClearColor, kExpectedPixelValue);
    }
}

// This test verifies that input double values are being rendered correctly when clearing.
TEST_P(RenderPassLoadOpTests, LoadOpClearIntegerFormatsToLargeValues) {
    // TODO(http://crbug.com/dawn/537): Implemement a workaround to enable clearing integer formats
    // to large values on D3D12.
    DAWN_SUPPRESS_TEST_IF(IsD3D12());

    // TODO(crbug.com/dawn/1109): Re-enable once fixed on Mac Mini 8,1s w/ 11.5.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel() && IsMacOS(11, 5));

    // TODO(crbug.com/dawn/1463): Re-enable, might be the same as above just on
    // 12.4 instead of 11.5.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel() && IsMacOS(12, 4));

    constexpr double kUint32MaxDouble = 4294967295.0;
    constexpr uint32_t kUint32Max = static_cast<uint32_t>(kUint32MaxDouble);
    // RGBA32Uint for UINT32_MAX
    {
        constexpr wgpu::Color kClearColor = {kUint32MaxDouble, kUint32MaxDouble, kUint32MaxDouble,
                                             kUint32MaxDouble};
        constexpr std::array<uint32_t, 4> kExpectedPixelValue = {kUint32Max, kUint32Max, kUint32Max,
                                                                 kUint32Max};
        TestClearColor<uint32_t>(wgpu::TextureFormat::RGBA32Uint, kClearColor, kExpectedPixelValue);
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
        TestClearColor<int32_t>(wgpu::TextureFormat::RGBA32Sint, kClearColor, kExpectedPixelValue);
    }

    // RGBA32Sint for SINT32 lower bound.
    {
        constexpr wgpu::Color kClearColor = {kSint32MinDouble, kSint32MinDouble, kSint32MinDouble,
                                             kSint32MinDouble};
        constexpr std::array<int32_t, 4> kExpectedPixelValue = {kSint32Min, kSint32Min, kSint32Min,
                                                                kSint32Min};
        TestClearColor<int32_t>(wgpu::TextureFormat::RGBA32Sint, kClearColor, kExpectedPixelValue);
    }
}

// Test clearing a color attachment on Uint8 formats (R8Uint, RG8Uint, RGBA8Uint) when the clear
// values are out of bound.
TEST_P(RenderPassLoadOpTests, LoadOpClearIntegerFormatsOutOfBound_Uint8) {
    constexpr uint16_t kUint8Max = std::numeric_limits<uint8_t>::max();

    using TestCase = std::tuple<wgpu::TextureFormat, wgpu::Color, std::array<uint8_t, 4>>;
    constexpr std::array<TestCase, 7> kTestCases = {{
        {wgpu::TextureFormat::R8Uint, {-1, 0, 0, 0}, {0, 0, 0, 0}},
        {wgpu::TextureFormat::R8Uint, {0, 0, 0, 0}, {0, 0, 0, 0}},
        {wgpu::TextureFormat::R8Uint, {kUint8Max, 0, 0, 0}, {kUint8Max, 0, 0, 0}},
        {wgpu::TextureFormat::R8Uint, {kUint8Max + 1, 0, 0, 0}, {kUint8Max, 0, 0, 0}},
        {wgpu::TextureFormat::RG8Uint, {0, kUint8Max, 0, 0}, {0, kUint8Max, 0, 0}},
        {wgpu::TextureFormat::RG8Uint, {-1, kUint8Max + 1, 0, 0}, {0, kUint8Max, 0, 0}},
        {wgpu::TextureFormat::RGBA8Uint,
         {-1, 0, kUint8Max, kUint8Max + 1},
         {0, 0, kUint8Max, kUint8Max}},
    }};

    for (const TestCase& testCase : kTestCases) {
        auto [format, clearColor, expectedPixelValue] = testCase;
        TestClearColor<uint8_t>(format, clearColor, expectedPixelValue);
    }
}

// Test clearing a color attachment on Sint8 formats (R8Sint, RG8Sint, RGBA8Sint) when the clear
// values are out of bound.
TEST_P(RenderPassLoadOpTests, LoadOpClearIntegerFormatsOutOfBound_Sint8) {
    constexpr int16_t kSint8Max = std::numeric_limits<int8_t>::max();
    constexpr int16_t kSint8Min = std::numeric_limits<int8_t>::min();

    using TestCase = std::tuple<wgpu::TextureFormat, wgpu::Color, std::array<int8_t, 4>>;
    constexpr std::array<TestCase, 7> kTestCases = {{
        {wgpu::TextureFormat::R8Sint, {kSint8Min - 1, 0, 0, 0}, {kSint8Min, 0, 0, 0}},
        {wgpu::TextureFormat::R8Sint, {kSint8Min, 0, 0, 0}, {kSint8Min, 0, 0, 0}},
        {wgpu::TextureFormat::R8Sint, {kSint8Max, 0, 0, 0}, {kSint8Max, 0, 0, 0}},
        {wgpu::TextureFormat::R8Sint, {kSint8Max + 1, 0, 0, 0}, {kSint8Max, 0, 0, 0}},
        {wgpu::TextureFormat::RG8Sint, {kSint8Min, kSint8Max, 0, 0}, {kSint8Min, kSint8Max, 0, 0}},
        {wgpu::TextureFormat::RG8Sint,
         {kSint8Min - 1, kSint8Max + 1, 0, 0},
         {kSint8Min, kSint8Max, 0, 0}},
        {wgpu::TextureFormat::RGBA8Sint,
         {kSint8Min - 1, kSint8Min, kSint8Max, kSint8Max + 1},
         {kSint8Min, kSint8Min, kSint8Max, kSint8Max}},
    }};

    for (const TestCase& testCase : kTestCases) {
        auto [format, clearColor, expectedPixelValue] = testCase;
        TestClearColor<int8_t>(format, clearColor, expectedPixelValue);
    }
}

// Test clearing a color attachment on Uint16 formats (R16Uint, RG16Uint, RGBA16Uint) when the clear
// values are out of bound.
TEST_P(RenderPassLoadOpTests, LoadOpClearIntegerFormatsOutOfBound_Uint16) {
    constexpr uint32_t kUint16Max = std::numeric_limits<uint16_t>::max();

    using TestCase = std::tuple<wgpu::TextureFormat, wgpu::Color, std::array<uint16_t, 4>>;
    constexpr std::array<TestCase, 7> kTestCases = {{
        {wgpu::TextureFormat::R16Uint, {-1, 0, 0, 0}, {0, 0, 0, 0}},
        {wgpu::TextureFormat::R16Uint, {0, 0, 0, 0}, {0, 0, 0, 0}},
        {wgpu::TextureFormat::R16Uint, {kUint16Max, 0, 0, 0}, {kUint16Max, 0, 0, 0}},
        {wgpu::TextureFormat::R16Uint, {kUint16Max + 1, 0, 0, 0}, {kUint16Max, 0, 0, 0}},
        {wgpu::TextureFormat::RG16Uint, {0, kUint16Max, 0, 0}, {0, kUint16Max, 0, 0}},
        {wgpu::TextureFormat::RG16Uint, {-1, kUint16Max + 1, 0, 0}, {0, kUint16Max, 0, 0}},
        {wgpu::TextureFormat::RGBA16Uint,
         {-1, 0, kUint16Max, kUint16Max + 1},
         {0, 0, kUint16Max, kUint16Max}},
    }};

    for (const TestCase& testCase : kTestCases) {
        auto [format, clearColor, expectedPixelValue] = testCase;
        TestClearColor<uint16_t>(format, clearColor, expectedPixelValue);
    }
}

// Test clearing a color attachment on Sint16 formats (R16Sint, RG16Sint, RGBA16Sint) when the clear
// values are out of bound.
TEST_P(RenderPassLoadOpTests, LoadOpClearIntegerFormatsOutOfBound_Sint16) {
    constexpr int32_t kSint16Max = std::numeric_limits<int16_t>::max();
    constexpr int32_t kSint16Min = std::numeric_limits<int16_t>::min();

    using TestCase = std::tuple<wgpu::TextureFormat, wgpu::Color, std::array<int16_t, 4>>;
    constexpr std::array<TestCase, 7> kTestCases = {{
        {wgpu::TextureFormat::R16Sint, {kSint16Min - 1, 0, 0, 0}, {kSint16Min, 0, 0, 0}},
        {wgpu::TextureFormat::R16Sint, {kSint16Min, 0, 0, 0}, {kSint16Min, 0, 0, 0}},
        {wgpu::TextureFormat::R16Sint, {kSint16Max, 0, 0, 0}, {kSint16Max, 0, 0, 0}},
        {wgpu::TextureFormat::R16Sint, {kSint16Max + 1, 0, 0, 0}, {kSint16Max, 0, 0, 0}},
        {wgpu::TextureFormat::RG16Sint,
         {kSint16Min, kSint16Max, 0, 0},
         {kSint16Min, kSint16Max, 0, 0}},
        {wgpu::TextureFormat::RG16Sint,
         {kSint16Min - 1, kSint16Max + 1, 0, 0},
         {kSint16Min, kSint16Max, 0, 0}},
        {wgpu::TextureFormat::RGBA16Sint,
         {kSint16Min - 1, kSint16Min, kSint16Max, kSint16Max + 1},
         {kSint16Min, kSint16Min, kSint16Max, kSint16Max}},
    }};

    for (const TestCase& testCase : kTestCases) {
        auto [format, clearColor, expectedPixelValue] = testCase;
        TestClearColor<int16_t>(format, clearColor, expectedPixelValue);
    }
}

// Test clearing a color attachment on Uint32 formats (R32Uint, RG32Uint, RGBA32Uint) when the clear
// values are out of bound.
TEST_P(RenderPassLoadOpTests, LoadOpClearIntegerFormatsOutOfBound_Uint32) {
    // TODO(http://crbug.com/dawn/537): Implemement a workaround to enable clearing integer formats
    // to large values on D3D12.
    DAWN_SUPPRESS_TEST_IF(IsD3D12());

    // TODO(crbug.com/dawn/1109): Re-enable once fixed on Mac Mini 8,1s w/ 11.5.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel() && IsMacOS(11, 5));

    // TODO(crbug.com/dawn/1463): Re-enable, might be the same as above just on
    // 12.4 instead of 11.5.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel() && IsMacOS(12, 4));

    constexpr uint64_t kUint32Max = std::numeric_limits<uint32_t>::max();

    using TestCase = std::tuple<wgpu::TextureFormat, wgpu::Color, std::array<uint32_t, 4>>;
    constexpr std::array<TestCase, 7> kTestCases = {{
        {wgpu::TextureFormat::R32Uint, {-1, 0, 0, 0}, {0, 0, 0, 0}},
        {wgpu::TextureFormat::R32Uint, {0, 0, 0, 0}, {0, 0, 0, 0}},
        {wgpu::TextureFormat::R32Uint, {kUint32Max, 0, 0, 0}, {kUint32Max, 0, 0, 0}},
        {wgpu::TextureFormat::R32Uint, {kUint32Max + 1, 0, 0, 0}, {kUint32Max, 0, 0, 0}},
        {wgpu::TextureFormat::RG32Uint, {0, kUint32Max, 0, 0}, {0, kUint32Max, 0, 0}},
        {wgpu::TextureFormat::RG32Uint, {-1, kUint32Max + 1, 0, 0}, {0, kUint32Max, 0, 0}},
        {wgpu::TextureFormat::RGBA32Uint,
         {-1, 0, kUint32Max, kUint32Max + 1},
         {0, 0, kUint32Max, kUint32Max}},
    }};

    for (const TestCase& testCase : kTestCases) {
        auto [format, clearColor, expectedPixelValue] = testCase;
        TestClearColor<uint32_t>(format, clearColor, expectedPixelValue);
    }
}

// Test clearing a color attachment on Sint32 formats (R32Sint, RG32Sint, RGBA32Sint) when the clear
// values are out of bound.
TEST_P(RenderPassLoadOpTests, LoadOpClearIntegerFormatsOutOfBound_Sint32) {
    // TODO(http://crbug.com/dawn/537): Implemement a workaround to enable clearing integer formats
    // to large values on D3D12.
    DAWN_SUPPRESS_TEST_IF(IsD3D12());

    // TODO(crbug.com/dawn/1109): Re-enable once fixed on Mac Mini 8,1s w/ 11.5.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel() && IsMacOS(11, 5));

    // TODO(crbug.com/dawn/1463): Re-enable, might be the same as above just on
    // 12.4 instead of 11.5.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel() && IsMacOS(12, 4));

    constexpr int64_t kSint32Max = std::numeric_limits<int32_t>::max();
    constexpr int64_t kSint32Min = std::numeric_limits<int32_t>::min();

    using TestCase = std::tuple<wgpu::TextureFormat, wgpu::Color, std::array<int32_t, 4>>;
    constexpr std::array<TestCase, 7> kTestCases = {{
        {wgpu::TextureFormat::R32Sint, {kSint32Min - 1, 0, 0, 0}, {kSint32Min, 0, 0, 0}},
        {wgpu::TextureFormat::R32Sint, {kSint32Min, 0, 0, 0}, {kSint32Min, 0, 0, 0}},
        {wgpu::TextureFormat::R32Sint, {kSint32Max, 0, 0, 0}, {kSint32Max, 0, 0, 0}},
        {wgpu::TextureFormat::R32Sint, {kSint32Max + 1, 0, 0, 0}, {kSint32Max, 0, 0, 0}},
        {wgpu::TextureFormat::RG32Sint,
         {kSint32Min, kSint32Max, 0, 0},
         {kSint32Min, kSint32Max, 0, 0}},
        {wgpu::TextureFormat::RG32Sint,
         {kSint32Min - 1, kSint32Max + 1, 0, 0},
         {kSint32Min, kSint32Max, 0, 0}},
        {wgpu::TextureFormat::RGBA32Sint,
         {kSint32Min - 1, kSint32Min, kSint32Max, kSint32Max + 1},
         {kSint32Min, kSint32Min, kSint32Max, kSint32Max}},
    }};

    for (const TestCase& testCase : kTestCases) {
        auto [format, clearColor, expectedPixelValue] = testCase;
        TestClearColor<int32_t>(format, clearColor, expectedPixelValue);
    }
}

// Test clearing a color attachment on normalized formats when the clear values are out of bound.
// Note that we don't test RGBA8Snorm because it doesn't support being used as render attachments in
// current WebGPU SPEC.
TEST_P(RenderPassLoadOpTests, LoadOpClearNormalizedFormatsOutOfBound) {
    // RGBA8Unorm
    {
        constexpr wgpu::Color kClearColor = {-0.1f, 0, 1, 1.1f};
        constexpr std::array<uint8_t, 4> kExpectedPixelValue = {0, 0, 255u, 255u};
        TestClearColor<uint8_t>(wgpu::TextureFormat::RGBA8Unorm, kClearColor, kExpectedPixelValue);
    }

    // RGB10A2Unorm - Test components RGB
    {
        constexpr wgpu::Color kClearColor = {-0.1f, 0, 1.1f, 1};
        constexpr std::array<uint8_t, 4> kExpectedPixelValue = {0, 0, 0xF0u, 0xFFu};
        TestClearColor<uint8_t>(wgpu::TextureFormat::RGB10A2Unorm, kClearColor,
                                kExpectedPixelValue);
    }

    // RGB10A2Unorm - Test component A < 0
    {
        constexpr wgpu::Color kClearColor = {0, 0, 0, -0.1f};
        constexpr std::array<uint8_t, 4> kExpectedPixelValue = {0, 0, 0, 0};
        TestClearColor<uint8_t>(wgpu::TextureFormat::RGB10A2Unorm, kClearColor,
                                kExpectedPixelValue);
    }

    // RGB10A2Unorm - Test component A > 1
    {
        constexpr wgpu::Color kClearColor = {0, 0, 0, 1.1f};
        constexpr std::array<uint8_t, 4> kExpectedPixelValue = {0, 0, 0, 0xC0u};
        TestClearColor<uint8_t>(wgpu::TextureFormat::RGB10A2Unorm, kClearColor,
                                kExpectedPixelValue);
    }
}

// Test clearing multiple color attachments with different big integers can still work correctly.
TEST_P(RenderPassLoadOpTests, LoadOpClearWithBigInt32ValuesOnMultipleColorAttachments) {
    // TODO(http://crbug.com/dawn/537): Implemement a workaround to enable clearing integer formats
    // to large values on D3D12.
    DAWN_SUPPRESS_TEST_IF(IsD3D12());

    // TODO(crbug.com/dawn/1109): Re-enable once fixed on Mac Mini 8,1s w/ 11.5.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel() && IsMacOS(11, 5));

    // TODO(crbug.com/dawn/1463): Re-enable, might be the same as above just on
    // 12.4 instead of 11.5.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel() && IsMacOS(12, 4));

    constexpr int32_t kMaxInt32RepresentableInFloat = 1 << std::numeric_limits<float>::digits;
    constexpr int32_t kMinInt32RepresentableInFloat = -kMaxInt32RepresentableInFloat;

    using TestCase = std::tuple<wgpu::TextureFormat, wgpu::Color, std::array<int32_t, 4>>;

    constexpr std::array<TestCase, kMaxColorAttachments> kTestCases = {{
        {wgpu::TextureFormat::R32Sint,
         {kMaxInt32RepresentableInFloat, 0, 0, 0},
         {kMaxInt32RepresentableInFloat, 0, 0, 0}},
        {wgpu::TextureFormat::R32Sint,
         {kMaxInt32RepresentableInFloat + 1, 0, 0, 0},
         {kMaxInt32RepresentableInFloat + 1, 0, 0, 0}},
        {wgpu::TextureFormat::R32Sint,
         {kMinInt32RepresentableInFloat, 0, 0, 0},
         {kMinInt32RepresentableInFloat, 0, 0, 0}},
        {wgpu::TextureFormat::R32Sint,
         {kMinInt32RepresentableInFloat - 1, 0, 0, 0},
         {kMinInt32RepresentableInFloat - 1, 0, 0, 0}},
        {wgpu::TextureFormat::RG32Sint,
         {kMaxInt32RepresentableInFloat, kMaxInt32RepresentableInFloat + 1, 0, 0},
         {kMaxInt32RepresentableInFloat, kMaxInt32RepresentableInFloat + 1, 0, 0}},
        {wgpu::TextureFormat::RG32Sint,
         {kMinInt32RepresentableInFloat, kMinInt32RepresentableInFloat - 1, 0, 0},
         {kMinInt32RepresentableInFloat, kMinInt32RepresentableInFloat - 1, 0, 0}},
        {wgpu::TextureFormat::RGBA32Sint,
         {kMaxInt32RepresentableInFloat, kMinInt32RepresentableInFloat,
          kMaxInt32RepresentableInFloat + 1, kMinInt32RepresentableInFloat - 1},
         {kMaxInt32RepresentableInFloat, kMinInt32RepresentableInFloat,
          kMaxInt32RepresentableInFloat + 1, kMinInt32RepresentableInFloat - 1}},
        {wgpu::TextureFormat::RGBA32Sint,
         {kMaxInt32RepresentableInFloat, kMinInt32RepresentableInFloat,
          kMaxInt32RepresentableInFloat - 1, kMinInt32RepresentableInFloat + 1},
         {kMaxInt32RepresentableInFloat, kMinInt32RepresentableInFloat,
          kMaxInt32RepresentableInFloat - 1, kMinInt32RepresentableInFloat + 1}},
    }};

    std::array<wgpu::Texture, kMaxColorAttachments> textures;

    wgpu::TextureDescriptor textureDescriptor = {};
    textureDescriptor.size = {1, 1, 1};
    textureDescriptor.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;

    std::array<wgpu::RenderPassColorAttachment, kMaxColorAttachments> colorAttachmentsInfo;
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
        textureDescriptor.format = std::get<0>(kTestCases[i]);
        textures[i] = device.CreateTexture(&textureDescriptor);

        colorAttachmentsInfo[i].view = textures[i].CreateView();
        colorAttachmentsInfo[i].loadOp = wgpu::LoadOp::Clear;
        colorAttachmentsInfo[i].storeOp = wgpu::StoreOp::Store;
        colorAttachmentsInfo[i].clearValue = std::get<1>(kTestCases[i]);
    }

    wgpu::RenderPassDescriptor renderPassDescriptor = {};
    renderPassDescriptor.colorAttachmentCount = kMaxColorAttachments;
    renderPassDescriptor.colorAttachments = colorAttachmentsInfo.data();
    wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
    renderPass.End();

    std::array<wgpu::Buffer, kMaxColorAttachments> outputBuffers;
    for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
        wgpu::BufferDescriptor bufferDescriptor = {};
        bufferDescriptor.size = sizeof(int32_t) * 4;
        bufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        outputBuffers[i] = device.CreateBuffer(&bufferDescriptor);

        wgpu::ImageCopyTexture imageCopyTexture =
            utils::CreateImageCopyTexture(textures[i], 0, {0, 0, 0});
        wgpu::ImageCopyBuffer imageCopyBuffer =
            utils::CreateImageCopyBuffer(outputBuffers[i], 0, kTextureBytesPerRowAlignment);
        encoder.CopyTextureToBuffer(&imageCopyTexture, &imageCopyBuffer, &textureDescriptor.size);
    }

    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
        const uint8_t* expected =
            reinterpret_cast<const uint8_t*>(std::get<2>(kTestCases[i]).data());
        EXPECT_BUFFER_U8_RANGE_EQ(expected, outputBuffers[i], 0,
                                  sizeof(std::get<2>(kTestCases[i])));
    }
}

// Test clearing multiple color attachments with different big unsigned integers can still work
// correctly.
TEST_P(RenderPassLoadOpTests, LoadOpClearWithBigUInt32ValuesOnMultipleColorAttachments) {
    // TODO(http://crbug.com/dawn/537): Implemement a workaround to enable clearing integer formats
    // to large values on D3D12.
    DAWN_SUPPRESS_TEST_IF(IsD3D12());

    // TODO(crbug.com/dawn/1109): Re-enable once fixed on Mac Mini 8,1s w/ 11.5.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel() && IsMacOS(11, 5));

    // TODO(crbug.com/dawn/1463): Re-enable, might be the same as above just on
    // 12.4 instead of 11.5.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel() && IsMacOS(12, 4));

    constexpr int32_t kMaxUInt32RepresentableInFloat = 1 << std::numeric_limits<float>::digits;

    using TestCase = std::tuple<wgpu::TextureFormat, wgpu::Color, std::array<uint32_t, 4>>;

    std::array<float, 4> testColorForRGBA32Float = {
        kMaxUInt32RepresentableInFloat, kMaxUInt32RepresentableInFloat - 1,
        kMaxUInt32RepresentableInFloat - 2, kMaxUInt32RepresentableInFloat - 3};
    std::array<uint32_t, 4> expectedDataForRGBA32Float;
    for (uint32_t i = 0; i < expectedDataForRGBA32Float.size(); ++i) {
        expectedDataForRGBA32Float[i] = *(reinterpret_cast<uint32_t*>(&testColorForRGBA32Float[i]));
    }

    const std::array<TestCase, kMaxColorAttachments> kTestCases = {{
        {wgpu::TextureFormat::R32Uint,
         {kMaxUInt32RepresentableInFloat, 0, 0, 0},
         {kMaxUInt32RepresentableInFloat, 0, 0, 0}},
        {wgpu::TextureFormat::R32Uint,
         {kMaxUInt32RepresentableInFloat + 1, 0, 0, 0},
         {kMaxUInt32RepresentableInFloat + 1, 0, 0, 0}},
        {wgpu::TextureFormat::RG32Uint,
         {kMaxUInt32RepresentableInFloat, kMaxUInt32RepresentableInFloat, 0, 0},
         {kMaxUInt32RepresentableInFloat, kMaxUInt32RepresentableInFloat, 0, 0}},
        {wgpu::TextureFormat::RG32Uint,
         {kMaxUInt32RepresentableInFloat, kMaxUInt32RepresentableInFloat + 1, 0, 0},
         {kMaxUInt32RepresentableInFloat, kMaxUInt32RepresentableInFloat + 1, 0, 0}},
        {wgpu::TextureFormat::RGBA32Uint,
         {kMaxUInt32RepresentableInFloat, kMaxUInt32RepresentableInFloat + 1,
          kMaxUInt32RepresentableInFloat - 1, kMaxUInt32RepresentableInFloat - 2},
         {kMaxUInt32RepresentableInFloat, kMaxUInt32RepresentableInFloat + 1,
          kMaxUInt32RepresentableInFloat - 1, kMaxUInt32RepresentableInFloat - 2}},
        {wgpu::TextureFormat::RGBA32Sint,
         {kMaxUInt32RepresentableInFloat, kMaxUInt32RepresentableInFloat - 1,
          kMaxUInt32RepresentableInFloat - 2, kMaxUInt32RepresentableInFloat - 3},
         {static_cast<int32_t>(kMaxUInt32RepresentableInFloat),
          static_cast<int32_t>(kMaxUInt32RepresentableInFloat - 1),
          static_cast<int32_t>(kMaxUInt32RepresentableInFloat - 2),
          static_cast<int32_t>(kMaxUInt32RepresentableInFloat - 3)}},
        {wgpu::TextureFormat::RGBA32Float,
         {kMaxUInt32RepresentableInFloat, kMaxUInt32RepresentableInFloat - 1,
          kMaxUInt32RepresentableInFloat - 2, kMaxUInt32RepresentableInFloat - 3},
         expectedDataForRGBA32Float},
        {wgpu::TextureFormat::Undefined,
         {kMaxUInt32RepresentableInFloat + 1, kMaxUInt32RepresentableInFloat + 1, 0, 0},
         {0, 0, 0, 0}},
    }};

    std::array<wgpu::Texture, kMaxColorAttachments> textures;

    wgpu::TextureDescriptor textureDescriptor = {};
    textureDescriptor.size = {1, 1, 1};
    textureDescriptor.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;

    std::array<wgpu::RenderPassColorAttachment, kMaxColorAttachments> colorAttachmentsInfo;
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
        wgpu::TextureFormat format = std::get<0>(kTestCases[i]);
        if (format == wgpu::TextureFormat::Undefined) {
            textures[i] = nullptr;
            colorAttachmentsInfo[i].view = nullptr;
            continue;
        }

        textureDescriptor.format = format;
        textures[i] = device.CreateTexture(&textureDescriptor);

        colorAttachmentsInfo[i].view = textures[i].CreateView();
        colorAttachmentsInfo[i].loadOp = wgpu::LoadOp::Clear;
        colorAttachmentsInfo[i].storeOp = wgpu::StoreOp::Store;
        colorAttachmentsInfo[i].clearValue = std::get<1>(kTestCases[i]);
    }

    wgpu::RenderPassDescriptor renderPassDescriptor = {};
    renderPassDescriptor.colorAttachmentCount = kMaxColorAttachments;
    renderPassDescriptor.colorAttachments = colorAttachmentsInfo.data();
    wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
    renderPass.End();

    std::array<wgpu::Buffer, kMaxColorAttachments> outputBuffers;
    for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
        wgpu::TextureFormat format = std::get<0>(kTestCases[i]);
        if (format == wgpu::TextureFormat::Undefined) {
            continue;
        }

        wgpu::BufferDescriptor bufferDescriptor = {};
        bufferDescriptor.size = sizeof(int32_t) * 4;
        bufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        outputBuffers[i] = device.CreateBuffer(&bufferDescriptor);

        wgpu::ImageCopyTexture imageCopyTexture =
            utils::CreateImageCopyTexture(textures[i], 0, {0, 0, 0});
        wgpu::ImageCopyBuffer imageCopyBuffer =
            utils::CreateImageCopyBuffer(outputBuffers[i], 0, kTextureBytesPerRowAlignment);
        encoder.CopyTextureToBuffer(&imageCopyTexture, &imageCopyBuffer, &textureDescriptor.size);
    }

    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    for (uint32_t i = 0; i < kMaxColorAttachments - 1; ++i) {
        const uint8_t* expected =
            reinterpret_cast<const uint8_t*>(std::get<2>(kTestCases[i]).data());
        EXPECT_BUFFER_U8_RANGE_EQ(expected, outputBuffers[i], 0,
                                  sizeof(std::get<2>(kTestCases[i])));
    }
}

DAWN_INSTANTIATE_TEST(RenderPassLoadOpTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
