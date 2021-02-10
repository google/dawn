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

#include <d3d11.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

#include "dawn_native/D3D12Backend.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

using Microsoft::WRL::ComPtr;

namespace {
    class D3D12VideoViewsTests : public DawnTest {
      protected:
        void SetUp() override {
            DawnTest::SetUp();
            DAWN_SKIP_TEST_IF(UsesWire());
            DAWN_SKIP_TEST_IF(!IsMultiPlanarFormatsSupported());

            // Create the D3D11 device/contexts that will be used in subsequent tests
            ComPtr<ID3D12Device> d3d12Device = dawn_native::d3d12::GetD3D12Device(device.Get());

            const LUID adapterLuid = d3d12Device->GetAdapterLuid();

            ComPtr<IDXGIFactory4> dxgiFactory;
            HRESULT hr = ::CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory));
            ASSERT_EQ(hr, S_OK);

            ComPtr<IDXGIAdapter> dxgiAdapter;
            hr = dxgiFactory->EnumAdapterByLuid(adapterLuid, IID_PPV_ARGS(&dxgiAdapter));
            ASSERT_EQ(hr, S_OK);

            ComPtr<ID3D11Device> d3d11Device;
            D3D_FEATURE_LEVEL d3dFeatureLevel;
            ComPtr<ID3D11DeviceContext> d3d11DeviceContext;
            hr = ::D3D11CreateDevice(dxgiAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0,
                                     nullptr, 0, D3D11_SDK_VERSION, &d3d11Device, &d3dFeatureLevel,
                                     &d3d11DeviceContext);
            ASSERT_EQ(hr, S_OK);

            // Runtime of the created texture (D3D11 device) and OpenSharedHandle runtime (Dawn's
            // D3D12 device) must agree on resource sharing capability. For NV12 formats, D3D11
            // requires at-least D3D11_SHARED_RESOURCE_TIER_2 support.
            // https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_shared_resource_tier
            D3D11_FEATURE_DATA_D3D11_OPTIONS5 featureOptions5{};
            hr = d3d11Device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS5, &featureOptions5,
                                                  sizeof(featureOptions5));
            ASSERT_EQ(hr, S_OK);

            ASSERT_GE(featureOptions5.SharedResourceTier, D3D11_SHARED_RESOURCE_TIER_2);

            mD3d11Device = std::move(d3d11Device);
        }

        std::vector<const char*> GetRequiredExtensions() override {
            mIsMultiPlanarFormatsSupported = SupportsExtensions({"multiplanar_formats"});
            if (!mIsMultiPlanarFormatsSupported) {
                return {};
            }

            return {"multiplanar_formats"};
        }

        bool IsMultiPlanarFormatsSupported() const {
            return mIsMultiPlanarFormatsSupported;
        }

        static DXGI_FORMAT GetDXGITextureFormat(wgpu::TextureFormat format) {
            switch (format) {
                case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
                    return DXGI_FORMAT_NV12;
                default:
                    UNREACHABLE();
                    return DXGI_FORMAT_UNKNOWN;
            }
        }

        // Returns a pre-prepared multi-planar formatted texture
        // The encoded texture data represents a 4x4 converted image. When |isCheckerboard| is true,
        // the upper left and bottom right fill a 2x2 grey block, from RGB(128, 128, 128), while the
        // upper right and bottom left fill a 2x2 white block, from RGB(255, 255, 255). When
        // |isCheckerboard| is false, the image is converted from a solid grey 4x4 block.
        static std::vector<uint8_t> GetTestTextureData(wgpu::TextureFormat format,
                                                       bool isCheckerboard) {
            constexpr uint8_t Y1 = kGreyYUVColor[kYUVLumaPlaneIndex].r;
            constexpr uint8_t U1 = kGreyYUVColor[kYUVChromaPlaneIndex].r;
            constexpr uint8_t V1 = kGreyYUVColor[kYUVChromaPlaneIndex].g;

            switch (format) {
                // The first 16 bytes is the luma plane (Y), followed by the chroma plane (UV) which
                // is half the number of bytes (subsampled by 2) but same bytes per line as luma
                // plane.
                case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
                    if (isCheckerboard) {
                        constexpr uint8_t Y2 = kWhiteYUVColor[kYUVLumaPlaneIndex].r;
                        constexpr uint8_t U2 = kWhiteYUVColor[kYUVChromaPlaneIndex].r;
                        constexpr uint8_t V2 = kWhiteYUVColor[kYUVChromaPlaneIndex].g;
                        // clang-format off
                        return {
                            Y2, Y2, Y1, Y1, // plane 0, start + 0
                            Y2, Y2, Y1, Y1,
                            Y1, Y1, Y2, Y2,
                            Y1, Y1, Y2, Y2,
                            U1, V1, U2, V2, // plane 1, start + 16
                            U2, V2, U1, V1,
                        };
                        // clang-format on
                    } else {
                        // clang-format off
                        return {
                            Y1, Y1, Y1, Y1,  // plane 0, start + 0
                            Y1, Y1, Y1, Y1,
                            Y1, Y1, Y1, Y1,
                            Y1, Y1, Y1, Y1,
                            U1, V1, U1, V1,  // plane 1, start + 16
                            U1, V1, U1, V1,
                        };
                        // clang-format on
                    }
                default:
                    UNREACHABLE();
                    return {};
            }
        }

        wgpu::Texture CreateVideoTextureForTest(wgpu::TextureFormat format,
                                                wgpu::TextureUsage usage,
                                                bool isCheckerboard = false) {
            wgpu::TextureDescriptor textureDesc;
            textureDesc.format = format;
            textureDesc.dimension = wgpu::TextureDimension::e2D;
            textureDesc.usage = usage;
            textureDesc.size = {kYUVImageDataWidthInTexels, kYUVImageDataHeightInTexels, 1};

            // Create a DX11 texture with data then wrap it in a shared handle.
            D3D11_TEXTURE2D_DESC d3dDescriptor;
            d3dDescriptor.Width = kYUVImageDataWidthInTexels;
            d3dDescriptor.Height = kYUVImageDataHeightInTexels;
            d3dDescriptor.MipLevels = 1;
            d3dDescriptor.ArraySize = 1;
            d3dDescriptor.Format = GetDXGITextureFormat(format);
            d3dDescriptor.SampleDesc.Count = 1;
            d3dDescriptor.SampleDesc.Quality = 0;
            d3dDescriptor.Usage = D3D11_USAGE_DEFAULT;
            d3dDescriptor.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            d3dDescriptor.CPUAccessFlags = 0;
            d3dDescriptor.MiscFlags =
                D3D11_RESOURCE_MISC_SHARED_NTHANDLE | D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;

            std::vector<uint8_t> initialData = GetTestTextureData(format, isCheckerboard);

            D3D11_SUBRESOURCE_DATA subres;
            subres.pSysMem = initialData.data();
            subres.SysMemPitch = kYUVImageDataWidthInTexels;

            ComPtr<ID3D11Texture2D> d3d11Texture;
            HRESULT hr = mD3d11Device->CreateTexture2D(&d3dDescriptor, &subres, &d3d11Texture);
            EXPECT_EQ(hr, S_OK);

            ComPtr<IDXGIResource1> dxgiResource;
            hr = d3d11Texture.As(&dxgiResource);
            EXPECT_EQ(hr, S_OK);

            HANDLE sharedHandle;
            hr = dxgiResource->CreateSharedHandle(
                nullptr, DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE, nullptr,
                &sharedHandle);
            EXPECT_EQ(hr, S_OK);

            dawn_native::d3d12::ExternalImageDescriptorDXGISharedHandle externDesc;
            externDesc.cTextureDescriptor =
                reinterpret_cast<const WGPUTextureDescriptor*>(&textureDesc);
            externDesc.sharedHandle = sharedHandle;
            externDesc.acquireMutexKey = 1;
            externDesc.isInitialized = true;

            // DX11 texture should be initialized upon CreateTexture2D. However, if we do not
            // acquire/release the keyed mutex before using the wrapped WebGPU texture, the WebGPU
            // texture is left uninitialized. This is required for D3D11 and D3D12 interop.
            ComPtr<IDXGIKeyedMutex> dxgiKeyedMutex;
            hr = d3d11Texture.As(&dxgiKeyedMutex);
            EXPECT_EQ(hr, S_OK);

            hr = dxgiKeyedMutex->AcquireSync(0, INFINITE);
            EXPECT_EQ(hr, S_OK);

            hr = dxgiKeyedMutex->ReleaseSync(1);
            EXPECT_EQ(hr, S_OK);

            // Open the DX11 texture in Dawn from the shared handle and return it as a WebGPU
            // texture.
            wgpu::Texture wgpuTexture = wgpu::Texture::Acquire(
                dawn_native::d3d12::WrapSharedHandle(device.Get(), &externDesc));

            // Handle is no longer needed once resources are created.
            ::CloseHandle(sharedHandle);

            return wgpuTexture;
        }

        // Vertex shader used to render a sampled texture into a quad.
        wgpu::ShaderModule GetTestVertexShaderModule() const {
            return utils::CreateShaderModuleFromWGSL(device, R"(
                [[builtin(position)]] var<out> Position : vec4<f32>;
                [[location(0)]] var<out> texCoord : vec2 <f32>;

                [[builtin(vertex_index)]] var<in> VertexIndex : u32;

                [[stage(vertex)]] fn main() -> void {
                    const pos : array<vec2<f32>, 6> = array<vec2<f32>, 6>(
                        vec2<f32>(-1.0, 1.0),
                        vec2<f32>(-1.0, -1.0),
                        vec2<f32>(1.0, -1.0),
                        vec2<f32>(-1.0, 1.0),
                        vec2<f32>(1.0, -1.0),
                        vec2<f32>(1.0, 1.0)
                    );
                    Position = vec4<f32>(pos[VertexIndex], 0.0, 1.0);
                    texCoord = vec2<f32>(Position.xy * 0.5) + vec2<f32>(0.5, 0.5);
            })");
        }

        // The width and height in texels are 4 for all YUV formats.
        static constexpr uint32_t kYUVImageDataWidthInTexels = 4;
        static constexpr uint32_t kYUVImageDataHeightInTexels = 4;

        static constexpr size_t kYUVLumaPlaneIndex = 0;
        static constexpr size_t kYUVChromaPlaneIndex = 1;

        // RGB colors converted into YUV (per plane), for testing.
        static constexpr std::array<RGBA8, 2> kGreyYUVColor = {RGBA8{126, 0, 0, 0xFF},     // Y
                                                               RGBA8{128, 128, 0, 0xFF}};  // UV

        static constexpr std::array<RGBA8, 2> kWhiteYUVColor = {RGBA8{235, 0, 0, 0xFF},     // Y
                                                                RGBA8{128, 128, 0, 0xFF}};  // UV

        ComPtr<ID3D11Device> mD3d11Device;

        bool mIsMultiPlanarFormatsSupported = false;
    };
}  // namespace

// Samples the luminance (Y) plane from an imported NV12 texture into a single channel of an RGBA
// output attachment and checks for the expected pixel value in the rendered quad.
TEST_P(D3D12VideoViewsTests, NV12SampleYtoR) {
    wgpu::Texture wgpuTexture = CreateVideoTextureForTest(
        wgpu::TextureFormat::R8BG8Biplanar420Unorm, wgpu::TextureUsage::Sampled);

    wgpu::TextureViewDescriptor viewDesc;
    viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    wgpu::TextureView textureView = wgpuTexture.CreateView(&viewDesc);

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor(device);
    renderPipelineDescriptor.vertexStage.module = GetTestVertexShaderModule();

    renderPipelineDescriptor.cFragmentStage.module = utils::CreateShaderModuleFromWGSL(device, R"(
            [[set(0), binding(0)]] var sampler0 : sampler;
            [[set(0), binding(1)]] var texture : texture_2d<f32>;

            [[location(0)]] var<in> texCoord : vec2<f32>;
            [[location(0)]] var<out> fragColor : vec4<f32>;

            [[stage(fragment)]] fn main() -> void {
               var y : f32 = textureSample(texture, sampler0, texCoord).r;
               fragColor = vec4<f32>(y, 0.0, 0.0, 1.0);
            })");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(
        device, kYUVImageDataWidthInTexels, kYUVImageDataHeightInTexels);
    renderPipelineDescriptor.cColorStates[0].format = renderPass.colorFormat;
    renderPipelineDescriptor.primitiveTopology = wgpu::PrimitiveTopology::TriangleList;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);

    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetBindGroup(0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                                  {{0, sampler}, {1, textureView}}));
        pass.Draw(6);
        pass.EndPass();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Test the luma plane in the top left corner of grey RGB image.
    EXPECT_PIXEL_RGBA8_EQ(kGreyYUVColor[kYUVLumaPlaneIndex], renderPass.color, 0, 0);
}

// Samples the chrominance (UV) plane from an imported texture into two channels of an RGBA output
// attachment and checks for the expected pixel value in the rendered quad.
TEST_P(D3D12VideoViewsTests, NV12SampleUVtoRG) {
    wgpu::Texture wgpuTexture = CreateVideoTextureForTest(
        wgpu::TextureFormat::R8BG8Biplanar420Unorm, wgpu::TextureUsage::Sampled);

    wgpu::TextureViewDescriptor viewDesc;
    viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    wgpu::TextureView textureView = wgpuTexture.CreateView(&viewDesc);

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor(device);
    renderPipelineDescriptor.vertexStage.module = GetTestVertexShaderModule();

    renderPipelineDescriptor.cFragmentStage.module = utils::CreateShaderModuleFromWGSL(device, R"(
            [[set(0), binding(0)]] var sampler0 : sampler;
            [[set(0), binding(1)]] var texture : texture_2d<f32>;

            [[location(0)]] var<in> texCoord : vec2<f32>;
            [[location(0)]] var<out> fragColor : vec4<f32>;

            [[stage(fragment)]] fn main() -> void {
               var u : f32 = textureSample(texture, sampler0, texCoord).r;
               var v : f32 = textureSample(texture, sampler0, texCoord).g;
               fragColor = vec4<f32>(u, v, 0.0, 1.0);
            })");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(
        device, kYUVImageDataWidthInTexels, kYUVImageDataHeightInTexels);
    renderPipelineDescriptor.cColorStates[0].format = renderPass.colorFormat;
    renderPipelineDescriptor.primitiveTopology = wgpu::PrimitiveTopology::TriangleList;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);

    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetBindGroup(0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                                  {{0, sampler}, {1, textureView}}));
        pass.Draw(6);
        pass.EndPass();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Test the chroma plane in the top left corner of grey RGB image.
    EXPECT_PIXEL_RGBA8_EQ(kGreyYUVColor[kYUVChromaPlaneIndex], renderPass.color, 0, 0);
}

// Renders a NV12 "checkerboard" texture into a RGB quad then checks the color at specific
// points to ensure the image has not been flipped.
TEST_P(D3D12VideoViewsTests, NV12SampleYUVtoRGB) {
    wgpu::Texture wgpuTexture = CreateVideoTextureForTest(
        wgpu::TextureFormat::R8BG8Biplanar420Unorm, wgpu::TextureUsage::Sampled, true);

    wgpu::TextureViewDescriptor lumaViewDesc;
    lumaViewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    wgpu::TextureView lumaTextureView = wgpuTexture.CreateView(&lumaViewDesc);

    wgpu::TextureViewDescriptor chromaViewDesc;
    chromaViewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    wgpu::TextureView chromaTextureView = wgpuTexture.CreateView(&chromaViewDesc);

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor(device);
    renderPipelineDescriptor.vertexStage.module = GetTestVertexShaderModule();

    renderPipelineDescriptor.cFragmentStage.module = utils::CreateShaderModuleFromWGSL(device, R"(
            [[set(0), binding(0)]] var sampler0 : sampler;
            [[set(0), binding(1)]] var lumaTexture : texture_2d<f32>;
            [[set(0), binding(2)]] var chromaTexture : texture_2d<f32>;

            [[location(0)]] var<in> texCoord : vec2<f32>;
            [[location(0)]] var<out> fragColor : vec4<f32>;

            [[stage(fragment)]] fn main() -> void {
               var y : f32 = textureSample(lumaTexture, sampler0, texCoord).r;
               var u : f32 = textureSample(chromaTexture, sampler0, texCoord).r;
               var v : f32 = textureSample(chromaTexture, sampler0, texCoord).g;
               fragColor = vec4<f32>(y, u, v, 1.0);
            })");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(
        device, kYUVImageDataWidthInTexels, kYUVImageDataHeightInTexels);
    renderPipelineDescriptor.cColorStates[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);

    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetBindGroup(
            0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                    {{0, sampler}, {1, lumaTextureView}, {2, chromaTextureView}}));
        pass.Draw(6);
        pass.EndPass();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Test four corners of the grey-white checkerboard image (YUV color space).
    RGBA8 greyYUV(kGreyYUVColor[kYUVLumaPlaneIndex].r, kGreyYUVColor[kYUVChromaPlaneIndex].r,
                  kGreyYUVColor[kYUVChromaPlaneIndex].g, 0xFF);
    EXPECT_PIXEL_RGBA8_EQ(greyYUV, renderPass.color, 0, 0);  // top left
    EXPECT_PIXEL_RGBA8_EQ(greyYUV, renderPass.color, kYUVImageDataWidthInTexels - 1,
                          kYUVImageDataHeightInTexels - 1);  // bottom right

    RGBA8 whiteYUV(kWhiteYUVColor[kYUVLumaPlaneIndex].r, kWhiteYUVColor[kYUVChromaPlaneIndex].r,
                   kWhiteYUVColor[kYUVChromaPlaneIndex].g, 0xFF);

    EXPECT_PIXEL_RGBA8_EQ(whiteYUV, renderPass.color, kYUVImageDataWidthInTexels - 1,
                          0);  // top right
    EXPECT_PIXEL_RGBA8_EQ(whiteYUV, renderPass.color, 0,
                          kYUVImageDataHeightInTexels - 1);  // bottom left
}

DAWN_INSTANTIATE_TEST(D3D12VideoViewsTests, D3D12Backend());