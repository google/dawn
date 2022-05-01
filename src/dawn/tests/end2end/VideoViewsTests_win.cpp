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

#include <d3d11.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

#include <memory>
#include <utility>
#include <vector>

#include "VideoViewsTests.h"
#include "dawn/common/Assert.h"
#include "dawn/native/D3D12Backend.h"

using Microsoft::WRL::ComPtr;

class PlatformTextureWin : public VideoViewsTestBackend::PlatformTexture {
  public:
    explicit PlatformTextureWin(wgpu::Texture&& texture) : PlatformTexture(std::move(texture)) {}
    ~PlatformTextureWin() override = default;

    bool CanWrapAsWGPUTexture() override { return true; }
};

class VideoViewsTestBackendWin : public VideoViewsTestBackend {
  public:
    ~VideoViewsTestBackendWin() override = default;

    void OnSetUp(WGPUDevice device) override {
        mWGPUDevice = device;

        // Create the D3D11 device/contexts that will be used in subsequent tests
        ComPtr<ID3D12Device> d3d12Device = dawn::native::d3d12::GetD3D12Device(device);

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
        hr = ::D3D11CreateDevice(dxgiAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0, nullptr, 0,
                                 D3D11_SDK_VERSION, &d3d11Device, &d3dFeatureLevel,
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

  protected:
    static DXGI_FORMAT GetDXGITextureFormat(wgpu::TextureFormat format) {
        switch (format) {
            case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
                return DXGI_FORMAT_NV12;
            default:
                UNREACHABLE();
        }
    }

    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> CreateVideoTextureForTest(
        wgpu::TextureFormat format,
        wgpu::TextureUsage usage,
        bool isCheckerboard) override {
        wgpu::TextureDescriptor textureDesc;
        textureDesc.format = format;
        textureDesc.dimension = wgpu::TextureDimension::e2D;
        textureDesc.usage = usage;
        textureDesc.size = {VideoViewsTests::kYUVImageDataWidthInTexels,
                            VideoViewsTests::kYUVImageDataHeightInTexels, 1};

        // Create a DX11 texture with data then wrap it in a shared handle.
        D3D11_TEXTURE2D_DESC d3dDescriptor;
        d3dDescriptor.Width = VideoViewsTests::kYUVImageDataWidthInTexels;
        d3dDescriptor.Height = VideoViewsTests::kYUVImageDataHeightInTexels;
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

        std::vector<uint8_t> initialData =
            VideoViewsTests::GetTestTextureData(format, isCheckerboard);

        D3D11_SUBRESOURCE_DATA subres;
        subres.pSysMem = initialData.data();
        subres.SysMemPitch = VideoViewsTests::kYUVImageDataWidthInTexels;

        ComPtr<ID3D11Texture2D> d3d11Texture;
        HRESULT hr = mD3d11Device->CreateTexture2D(&d3dDescriptor, &subres, &d3d11Texture);
        ASSERT(hr == S_OK);

        ComPtr<IDXGIResource1> dxgiResource;
        hr = d3d11Texture.As(&dxgiResource);
        ASSERT(hr == S_OK);

        HANDLE sharedHandle;
        hr = dxgiResource->CreateSharedHandle(
            nullptr, DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE, nullptr,
            &sharedHandle);
        ASSERT(hr == S_OK);

        // DX11 texture should be initialized upon CreateTexture2D. However, if we do not
        // acquire/release the keyed mutex before using the wrapped WebGPU texture, the WebGPU
        // texture is left uninitialized. This is required for D3D11 and D3D12 interop.
        ComPtr<IDXGIKeyedMutex> dxgiKeyedMutex;
        hr = d3d11Texture.As(&dxgiKeyedMutex);
        ASSERT(hr == S_OK);

        using dawn::native::d3d12::kDXGIKeyedMutexAcquireReleaseKey;
        hr = dxgiKeyedMutex->AcquireSync(kDXGIKeyedMutexAcquireReleaseKey, INFINITE);
        ASSERT(hr == S_OK);

        hr = dxgiKeyedMutex->ReleaseSync(kDXGIKeyedMutexAcquireReleaseKey);
        ASSERT(hr == S_OK);

        // Open the DX11 texture in Dawn from the shared handle and return it as a WebGPU
        // texture.
        dawn::native::d3d12::ExternalImageDescriptorDXGISharedHandle externalImageDesc;
        externalImageDesc.cTextureDescriptor =
            reinterpret_cast<const WGPUTextureDescriptor*>(&textureDesc);
        externalImageDesc.sharedHandle = sharedHandle;

        std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage =
            dawn::native::d3d12::ExternalImageDXGI::Create(mWGPUDevice, &externalImageDesc);

        // Handle is no longer needed once resources are created.
        ::CloseHandle(sharedHandle);

        dawn::native::d3d12::ExternalImageAccessDescriptorDXGIKeyedMutex externalAccessDesc;
        externalAccessDesc.isInitialized = true;
        externalAccessDesc.usage = static_cast<WGPUTextureUsageFlags>(textureDesc.usage);

        return std::make_unique<PlatformTextureWin>(wgpu::Texture::Acquire(
            externalImage->ProduceTexture(mWGPUDevice, &externalAccessDesc)));
    }

    void DestroyVideoTextureForTest(
        std::unique_ptr<VideoViewsTestBackend::PlatformTexture>&& PlatformTexture) override {}

    WGPUDevice mWGPUDevice = nullptr;
    ComPtr<ID3D11Device> mD3d11Device;
};

// static
BackendTestConfig VideoViewsTestBackend::Backend() {
    return D3D12Backend();
}
// static
std::unique_ptr<VideoViewsTestBackend> VideoViewsTestBackend::Create() {
    return std::make_unique<VideoViewsTestBackendWin>();
}
