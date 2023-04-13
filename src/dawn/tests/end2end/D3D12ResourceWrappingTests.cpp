// Copyright 2019 The Dawn Authors
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
#include <d3d11_4.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

#include <memory>
#include <thread>
#include <utility>
#include <vector>

#include "dawn/native/D3D12Backend.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

using Microsoft::WRL::ComPtr;

namespace {

enum class SyncMode {
    kKeyedMutex,
    kFence,
};

std::ostream& operator<<(std::ostream& o, const SyncMode& m) {
    switch (m) {
        case SyncMode::kKeyedMutex:
            o << "KeyedMutex";
            break;
        case SyncMode::kFence:
            o << "Fence";
            break;
    }
    return o;
}

DAWN_TEST_PARAM_STRUCT(D3D12ResourceTestParams, SyncMode);

using dawn::native::d3d12::kDXGIKeyedMutexAcquireReleaseKey;

class D3D12ResourceTestBase : public DawnTestWithParams<D3D12ResourceTestParams> {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::DawnInternalUsages};
    }

  public:
    void SetUp() override {
        DawnTestWithParams<D3D12ResourceTestParams>::SetUp();
        if (UsesWire()) {
            return;
        }

        // Create the D3D11 device/contexts that will be used in subsequent tests
        ComPtr<ID3D12Device> d3d12Device = dawn::native::d3d12::GetD3D12Device(device.Get());

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

        mD3d11Device = std::move(d3d11Device);
        mD3d11DeviceContext = std::move(d3d11DeviceContext);

        baseDawnDescriptor.dimension = wgpu::TextureDimension::e2D;
        baseDawnDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        baseDawnDescriptor.size = {kTestWidth, kTestHeight, 1};
        baseDawnDescriptor.sampleCount = 1;
        baseDawnDescriptor.mipLevelCount = 1;
        baseDawnDescriptor.usage =
            wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc |
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopyDst;

        baseD3dDescriptor.Width = kTestWidth;
        baseD3dDescriptor.Height = kTestHeight;
        baseD3dDescriptor.MipLevels = 1;
        baseD3dDescriptor.ArraySize = 1;
        baseD3dDescriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        baseD3dDescriptor.SampleDesc.Count = 1;
        baseD3dDescriptor.SampleDesc.Quality = 0;
        baseD3dDescriptor.Usage = D3D11_USAGE_DEFAULT;
        baseD3dDescriptor.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        baseD3dDescriptor.CPUAccessFlags = 0;
        baseD3dDescriptor.MiscFlags = D3D11_RESOURCE_MISC_SHARED_NTHANDLE;
        if (GetParam().mSyncMode == SyncMode::kKeyedMutex) {
            baseD3dDescriptor.MiscFlags |= D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
        } else {
            baseD3dDescriptor.MiscFlags |= D3D11_RESOURCE_MISC_SHARED;
        }
    }

  protected:
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> CreateExternalImage(
        WGPUDevice targetDevice,
        ID3D11Texture2D* d3d11Texture,
        const wgpu::TextureDescriptor* dawnDesc) const {
        ComPtr<IDXGIResource1> dxgiResource;
        EXPECT_EQ(d3d11Texture->QueryInterface(IID_PPV_ARGS(&dxgiResource)), S_OK);

        HANDLE textureSharedHandle = nullptr;
        EXPECT_EQ(dxgiResource->CreateSharedHandle(
                      nullptr, DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE, nullptr,
                      &textureSharedHandle),
                  S_OK);

        dawn::native::d3d12::ExternalImageDescriptorDXGISharedHandle externalImageDesc;
        externalImageDesc.cTextureDescriptor =
            reinterpret_cast<const WGPUTextureDescriptor*>(dawnDesc);
        externalImageDesc.sharedHandle = textureSharedHandle;
        externalImageDesc.useFenceSynchronization = GetParam().mSyncMode == SyncMode::kFence;

        std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage =
            dawn::native::d3d12::ExternalImageDXGI::Create(targetDevice, &externalImageDesc);

        // Now that we've created all of our resources, we can close the handle
        // since we no longer need it.
        ::CloseHandle(textureSharedHandle);

        return externalImage;
    }

    void WrapSharedHandle(
        const wgpu::TextureDescriptor* dawnDesc,
        const D3D11_TEXTURE2D_DESC* baseD3dDescriptor,
        wgpu::Texture* dawnTexture,
        ID3D11Texture2D** d3d11TextureOut,
        std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI>* externalImageOut) const {
        ComPtr<ID3D11Texture2D> d3d11Texture;
        HRESULT hr = mD3d11Device->CreateTexture2D(baseD3dDescriptor, nullptr, &d3d11Texture);
        ASSERT_EQ(hr, S_OK);

        std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage =
            CreateExternalImage(device.Get(), d3d11Texture.Get(), dawnDesc);

        // Cannot access a non-existent external image (ex. validation error).
        if (externalImage == nullptr) {
            return;
        }

        dawn::native::d3d12::ExternalImageDXGIBeginAccessDescriptor externalAccessDesc;
        externalAccessDesc.usage = static_cast<WGPUTextureUsageFlags>(dawnDesc->usage);

        *dawnTexture = wgpu::Texture::Acquire(externalImage->BeginAccess(&externalAccessDesc));
        *d3d11TextureOut = d3d11Texture.Detach();

        if (externalImageOut != nullptr) {
            *externalImageOut = std::move(externalImage);
        }
    }

    static constexpr size_t kTestWidth = 10;
    static constexpr size_t kTestHeight = 10;

    ComPtr<ID3D11Device> mD3d11Device;
    ComPtr<ID3D11DeviceContext> mD3d11DeviceContext;

    D3D11_TEXTURE2D_DESC baseD3dDescriptor;
    wgpu::TextureDescriptor baseDawnDescriptor;
};

}  // anonymous namespace

// A small fixture used to initialize default data for the D3D12Resource validation tests.
// These tests are skipped if the harness is using the wire.
class D3D12SharedHandleValidation : public D3D12ResourceTestBase {};

// Test a successful wrapping of an D3D12Resource in a texture
TEST_P(D3D12SharedHandleValidation, Success) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    wgpu::Texture texture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;
    WrapSharedHandle(&baseDawnDescriptor, &baseD3dDescriptor, &texture, &d3d11Texture,
                     &externalImage);

    ASSERT_NE(texture.Get(), nullptr);

    dawn::native::d3d12::ExternalImageDXGIFenceDescriptor signalFence;
    externalImage->EndAccess(texture.Get(), &signalFence);
    texture.Destroy();
}

// Test a successful wrapping of an D3D12Resource with DawnTextureInternalUsageDescriptor
TEST_P(D3D12SharedHandleValidation, SuccessWithInternalUsageDescriptor) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
    baseDawnDescriptor.nextInChain = &internalDesc;
    internalDesc.internalUsage = wgpu::TextureUsage::CopySrc;
    internalDesc.sType = wgpu::SType::DawnTextureInternalUsageDescriptor;

    wgpu::Texture texture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;
    WrapSharedHandle(&baseDawnDescriptor, &baseD3dDescriptor, &texture, &d3d11Texture,
                     &externalImage);

    ASSERT_NE(texture.Get(), nullptr);

    dawn::native::d3d12::ExternalImageDXGIFenceDescriptor signalFence;
    externalImage->EndAccess(texture.Get(), &signalFence);
    texture.Destroy();
}

// Test an error occurs if an invalid sType is the nextInChain
TEST_P(D3D12SharedHandleValidation, InvalidTextureDescriptor) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    wgpu::ChainedStruct chainedDescriptor;
    chainedDescriptor.sType = wgpu::SType::SurfaceDescriptorFromWindowsSwapChainPanel;
    baseDawnDescriptor.nextInChain = &chainedDescriptor;

    wgpu::Texture texture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;
    ASSERT_DEVICE_ERROR(WrapSharedHandle(&baseDawnDescriptor, &baseD3dDescriptor, &texture,
                                         &d3d11Texture, &externalImage));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor mip level count isn't 1
TEST_P(D3D12SharedHandleValidation, InvalidMipLevelCount) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    baseDawnDescriptor.mipLevelCount = 2;

    wgpu::Texture texture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;
    ASSERT_DEVICE_ERROR(WrapSharedHandle(&baseDawnDescriptor, &baseD3dDescriptor, &texture,
                                         &d3d11Texture, &externalImage));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor depth isn't 1
TEST_P(D3D12SharedHandleValidation, InvalidDepth) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    baseDawnDescriptor.size.depthOrArrayLayers = 2;

    wgpu::Texture texture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;
    ASSERT_DEVICE_ERROR(WrapSharedHandle(&baseDawnDescriptor, &baseD3dDescriptor, &texture,
                                         &d3d11Texture, &externalImage));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor sample count isn't 1
TEST_P(D3D12SharedHandleValidation, InvalidSampleCount) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    baseDawnDescriptor.sampleCount = 4;

    wgpu::Texture texture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;
    ASSERT_DEVICE_ERROR(WrapSharedHandle(&baseDawnDescriptor, &baseD3dDescriptor, &texture,
                                         &d3d11Texture, &externalImage));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor width doesn't match the texture's
TEST_P(D3D12SharedHandleValidation, InvalidWidth) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    baseDawnDescriptor.size.width = kTestWidth + 1;

    wgpu::Texture texture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;
    ASSERT_DEVICE_ERROR(WrapSharedHandle(&baseDawnDescriptor, &baseD3dDescriptor, &texture,
                                         &d3d11Texture, &externalImage));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor height doesn't match the texture's
TEST_P(D3D12SharedHandleValidation, InvalidHeight) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    baseDawnDescriptor.size.height = kTestHeight + 1;

    wgpu::Texture texture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;
    ASSERT_DEVICE_ERROR(WrapSharedHandle(&baseDawnDescriptor, &baseD3dDescriptor, &texture,
                                         &d3d11Texture, &externalImage));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor format isn't compatible with the D3D12 Resource
TEST_P(D3D12SharedHandleValidation, InvalidFormat) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    baseDawnDescriptor.format = wgpu::TextureFormat::R8Unorm;

    wgpu::Texture texture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;
    ASSERT_DEVICE_ERROR(WrapSharedHandle(&baseDawnDescriptor, &baseD3dDescriptor, &texture,
                                         &d3d11Texture, &externalImage));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the number of D3D mip levels is greater than 1.
TEST_P(D3D12SharedHandleValidation, InvalidNumD3DMipLevels) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    baseD3dDescriptor.MipLevels = 2;

    wgpu::Texture texture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;
    ASSERT_DEVICE_ERROR(WrapSharedHandle(&baseDawnDescriptor, &baseD3dDescriptor, &texture,
                                         &d3d11Texture, &externalImage));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the number of array levels is greater than 1.
TEST_P(D3D12SharedHandleValidation, InvalidD3DArraySize) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    baseD3dDescriptor.ArraySize = 2;

    wgpu::Texture texture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;
    ASSERT_DEVICE_ERROR(WrapSharedHandle(&baseDawnDescriptor, &baseD3dDescriptor, &texture,
                                         &d3d11Texture, &externalImage));

    ASSERT_EQ(texture.Get(), nullptr);
}

class D3D12SharedHandleUsageTests : public D3D12ResourceTestBase {
  protected:
    // Submits a 1x1x1 copy from source to destination
    void SimpleCopyTextureToTexture(wgpu::Texture source, wgpu::Texture destination) {
        wgpu::ImageCopyTexture copySrc = utils::CreateImageCopyTexture(source, 0, {0, 0, 0});
        wgpu::ImageCopyTexture copyDst = utils::CreateImageCopyTexture(destination, 0, {0, 0, 0});

        wgpu::Extent3D copySize = {1, 1, 1};

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&copySrc, &copyDst, &copySize);
        wgpu::CommandBuffer commands = encoder.Finish();

        queue.Submit(1, &commands);
    }

    // Clear a texture on a given device
    void ClearImage(wgpu::Texture wrappedTexture,
                    const wgpu::Color& clearColor,
                    wgpu::Device wgpuDevice) {
        wgpu::TextureView wrappedView = wrappedTexture.CreateView();

        // Submit a clear operation
        utils::ComboRenderPassDescriptor renderPassDescriptor({wrappedView}, {});
        renderPassDescriptor.cColorAttachments[0].clearValue = clearColor;

        wgpu::CommandEncoder encoder = wgpuDevice.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDescriptor);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        wgpu::Queue queue = wgpuDevice.GetQueue();
        queue.Submit(1, &commands);
    }

    void WrapAndClearD3D11Texture(
        const wgpu::TextureDescriptor& dawnDescriptor,
        const D3D11_TEXTURE2D_DESC& d3dDescriptor,
        const wgpu::Color& clearColor,
        wgpu::Texture* dawnTextureOut,
        ID3D11Texture2D** d3d11TextureOut,
        std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI>* externalImageOut,
        bool isInitialized = true) const {
        ComPtr<ID3D11Texture2D> d3d11Texture;
        HRESULT hr = mD3d11Device->CreateTexture2D(&d3dDescriptor, nullptr, &d3d11Texture);
        ASSERT_EQ(hr, S_OK);

        ComPtr<IDXGIResource1> dxgiResource;
        hr = d3d11Texture.As(&dxgiResource);
        ASSERT_EQ(hr, S_OK);

        HANDLE sharedHandle;
        hr = dxgiResource->CreateSharedHandle(
            nullptr, DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE, nullptr,
            &sharedHandle);
        ASSERT_EQ(hr, S_OK);

        ComPtr<IDXGIKeyedMutex> dxgiKeyedMutex;

        HANDLE fenceSharedHandle = nullptr;
        ComPtr<ID3D11Fence> d3d11Fence;

        ComPtr<ID3D11DeviceContext4> d3d11DeviceContext4;

        if (GetParam().mSyncMode == SyncMode::kKeyedMutex) {
            hr = d3d11Texture.As(&dxgiKeyedMutex);
            ASSERT_EQ(hr, S_OK);

            hr = dxgiKeyedMutex->AcquireSync(kDXGIKeyedMutexAcquireReleaseKey, INFINITE);
            ASSERT_EQ(hr, S_OK);
        } else {
            ComPtr<ID3D11Device5> d3d11Device5;
            hr = mD3d11Device.As(&d3d11Device5);
            ASSERT_EQ(hr, S_OK);

            hr = d3d11Device5->CreateFence(0, D3D11_FENCE_FLAG_SHARED, IID_PPV_ARGS(&d3d11Fence));
            ASSERT_EQ(hr, S_OK);

            hr = d3d11Fence->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, &fenceSharedHandle);
            ASSERT_EQ(hr, S_OK);
        }

        ComPtr<ID3D11RenderTargetView> d3d11RTV;
        hr = mD3d11Device->CreateRenderTargetView(d3d11Texture.Get(), nullptr, &d3d11RTV);
        ASSERT_EQ(hr, S_OK);

        const float colorRGBA[] = {
            static_cast<float>(clearColor.r), static_cast<float>(clearColor.g),
            static_cast<float>(clearColor.b), static_cast<float>(clearColor.a)};
        mD3d11DeviceContext->ClearRenderTargetView(d3d11RTV.Get(), colorRGBA);

        constexpr uint64_t kFenceSignalValue = 1;
        if (dxgiKeyedMutex) {
            hr = dxgiKeyedMutex->ReleaseSync(kDXGIKeyedMutexAcquireReleaseKey);
            ASSERT_EQ(hr, S_OK);
        } else {
            hr = mD3d11DeviceContext.As(&d3d11DeviceContext4);
            ASSERT_EQ(hr, S_OK);
            // The fence starts with 0 signaled, but that won't capture the render target view clear
            // above, so signal explicitly with 1 and make the next Dawn access wait on 1.
            d3d11DeviceContext4->Signal(d3d11Fence.Get(), kFenceSignalValue);
        }

        dawn::native::d3d12::ExternalImageDescriptorDXGISharedHandle externalImageDesc = {};
        externalImageDesc.sharedHandle = sharedHandle;
        externalImageDesc.cTextureDescriptor =
            reinterpret_cast<const WGPUTextureDescriptor*>(&dawnDescriptor);
        externalImageDesc.useFenceSynchronization = GetParam().mSyncMode == SyncMode::kFence;

        std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage =
            dawn::native::d3d12::ExternalImageDXGI::Create(device.Get(), &externalImageDesc);

        dawn::native::d3d12::ExternalImageDXGIBeginAccessDescriptor externalAccessDesc;
        externalAccessDesc.isInitialized = isInitialized;
        externalAccessDesc.usage = static_cast<WGPUTextureUsageFlags>(dawnDescriptor.usage);
        if (fenceSharedHandle != nullptr) {
            externalAccessDesc.waitFences.push_back({fenceSharedHandle, kFenceSignalValue});
        }

        *dawnTextureOut = wgpu::Texture::Acquire(externalImage->BeginAccess(&externalAccessDesc));
        *d3d11TextureOut = d3d11Texture.Detach();
        *externalImageOut = std::move(externalImage);

        if (fenceSharedHandle != nullptr) {
            ::CloseHandle(fenceSharedHandle);
        }
    }

    void ExpectPixelRGBA8EQ(
        ID3D11Texture2D* d3d11Texture,
        const wgpu::Color& color,
        const dawn::native::d3d12::ExternalImageDXGIFenceDescriptor* waitFence = nullptr) {
        D3D11_TEXTURE2D_DESC texture2DDesc;
        d3d11Texture->GetDesc(&texture2DDesc);

        const CD3D11_TEXTURE2D_DESC texture2DStagingDesc(
            texture2DDesc.Format,                             // Format
            texture2DDesc.Width,                              // Width
            texture2DDesc.Height,                             // Height
            1,                                                // ArraySize
            1,                                                // MipLevels
            0,                                                // BindFlags
            D3D11_USAGE_STAGING,                              // Usage
            D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE);  // CPUAccessFlags

        ComPtr<ID3D11Texture2D> spD3DTextureStaging;
        HRESULT hr =
            mD3d11Device->CreateTexture2D(&texture2DStagingDesc, nullptr, &spD3DTextureStaging);
        ASSERT_EQ(hr, S_OK);

        D3D11_BOX d3dRc;
        d3dRc.back = 1;
        d3dRc.front = 0;
        d3dRc.top = 0;
        d3dRc.left = 0;
        d3dRc.bottom = texture2DDesc.Height;
        d3dRc.right = texture2DDesc.Width;

        ComPtr<IDXGIKeyedMutex> dxgiKeyedMutex;
        d3d11Texture->QueryInterface(IID_PPV_ARGS(&dxgiKeyedMutex));

        if (dxgiKeyedMutex != nullptr) {
            hr = dxgiKeyedMutex->AcquireSync(kDXGIKeyedMutexAcquireReleaseKey, INFINITE);
            ASSERT_EQ(hr, S_OK);
        }

        if (waitFence != nullptr && waitFence->fenceHandle != nullptr) {
            ComPtr<ID3D11Device5> d3d11Device5;
            mD3d11Device.As(&d3d11Device5);
            ASSERT_EQ(hr, S_OK);

            ComPtr<ID3D11Fence> d3d11Fence;
            hr = d3d11Device5->OpenSharedFence(waitFence->fenceHandle, IID_PPV_ARGS(&d3d11Fence));
            ASSERT_EQ(hr, S_OK);

            ComPtr<ID3D11DeviceContext4> d3d11DeviceContext4;
            hr = mD3d11DeviceContext.As(&d3d11DeviceContext4);
            ASSERT_EQ(hr, S_OK);

            hr = d3d11DeviceContext4->Wait(d3d11Fence.Get(), waitFence->fenceValue);
            ASSERT_EQ(hr, S_OK);
        }

        mD3d11DeviceContext->CopySubresourceRegion(spD3DTextureStaging.Get(),  // pDstResource
                                                   0,                          // DstSubresource
                                                   0,                          // DstX
                                                   0,                          // DstY
                                                   0,                          // DstZ
                                                   d3d11Texture,               // pSrcResource
                                                   0,                          // SrcSubresource
                                                   &d3dRc);                    // pSrcBox

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        hr = mD3d11DeviceContext->Map(spD3DTextureStaging.Get(), 0, D3D11_MAP_READ_WRITE, 0,
                                      &mappedResource);
        ASSERT_EQ(hr, S_OK);

        const uint8_t* colorData = static_cast<uint8_t*>(mappedResource.pData);
        EXPECT_EQ(colorData[0], color.r * 255u);
        EXPECT_EQ(colorData[1], color.g * 255u);
        EXPECT_EQ(colorData[2], color.b * 255u);
        EXPECT_EQ(colorData[3], color.a * 255u);

        mD3d11DeviceContext->Unmap(spD3DTextureStaging.Get(), 0);

        if (dxgiKeyedMutex != nullptr) {
            hr = dxgiKeyedMutex->ReleaseSync(kDXGIKeyedMutexAcquireReleaseKey);
            ASSERT_EQ(hr, S_OK);
        }
    }
};

// 1. Create and clear a D3D11 texture
// 2. Copy the wrapped texture to another dawn texture
// 3. Readback the copied texture and ensure the color matches the original clear color.
TEST_P(D3D12SharedHandleUsageTests, ClearInD3D11CopyAndReadbackInD3D12) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    const wgpu::Color clearColor{1.0f, 1.0f, 0.0f, 1.0f};
    wgpu::Texture dawnSrcTexture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;
    WrapAndClearD3D11Texture(baseDawnDescriptor, baseD3dDescriptor, clearColor, &dawnSrcTexture,
                             &d3d11Texture, &externalImage);
    ASSERT_NE(dawnSrcTexture.Get(), nullptr);

    // Create a texture on the device and copy the source texture to it.
    wgpu::Texture dawnCopyDestTexture = device.CreateTexture(&baseDawnDescriptor);
    SimpleCopyTextureToTexture(dawnSrcTexture, dawnCopyDestTexture);

    dawn::native::d3d12::ExternalImageDXGIFenceDescriptor signalFence;
    externalImage->EndAccess(dawnSrcTexture.Get(), &signalFence);
    dawnSrcTexture.Destroy();

    // Readback the destination texture and ensure it contains the colors we used
    // to clear the source texture on the D3D device.
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(clearColor.r * 255u, clearColor.g * 255u,
                                       clearColor.b * 255u, clearColor.a * 255u),
                          dawnCopyDestTexture, 0, 0);
}

// 1. Create and clear a D3D11 texture
// 2. Readback the wrapped texture and ensure the color matches the original clear color.
TEST_P(D3D12SharedHandleUsageTests, ClearInD3D11ReadbackInD3D12) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    const wgpu::Color clearColor{1.0f, 1.0f, 0.0f, 1.0f};
    wgpu::Texture dawnTexture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;
    WrapAndClearD3D11Texture(baseDawnDescriptor, baseD3dDescriptor, clearColor, &dawnTexture,
                             &d3d11Texture, &externalImage);
    ASSERT_NE(dawnTexture.Get(), nullptr);

    // Readback the destination texture and ensure it contains the colors we used
    // to clear the source texture on the D3D device.
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(clearColor.r * 255, clearColor.g * 255, clearColor.b * 255,
                                       clearColor.a * 255),
                          dawnTexture, 0, 0);

    dawn::native::d3d12::ExternalImageDXGIFenceDescriptor signalFence;
    externalImage->EndAccess(dawnTexture.Get(), &signalFence);
    dawnTexture.Destroy();
}

// 1. Create and clear a D3D11 texture
// 2. Wrap it in a Dawn texture and clear it to a different color
// 3. Readback the texture with D3D11 and ensure we receive the color we cleared with Dawn.
TEST_P(D3D12SharedHandleUsageTests, ClearInD3D12ReadbackInD3D11) {
    // TODO(crbug.com/dawn/735): This test appears to hang for
    // D3D12_Microsoft_Basic_Render_Driver_CPU when validation is enabled.
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsWARP() && IsBackendValidationEnabled());

    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    const wgpu::Color d3d11ClearColor{1.0f, 1.0f, 0.0f, 1.0f};
    wgpu::Texture dawnTexture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;
    ComPtr<IDXGIKeyedMutex> dxgiKeyedMutex;
    WrapAndClearD3D11Texture(baseDawnDescriptor, baseD3dDescriptor, d3d11ClearColor, &dawnTexture,
                             &d3d11Texture, &externalImage, /*isInitialized=*/true);
    ASSERT_NE(dawnTexture.Get(), nullptr);

    const wgpu::Color d3d12ClearColor{0.0f, 0.0f, 1.0f, 1.0f};
    ClearImage(dawnTexture, d3d12ClearColor, device);

    dawn::native::d3d12::ExternalImageDXGIFenceDescriptor signalFence;
    externalImage->EndAccess(dawnTexture.Get(), &signalFence);
    dawnTexture.Destroy();

    // Now that Dawn (via D3D12) has finished writing to the texture, we should be
    // able to read it back by copying it to a staging texture and verifying the
    // color matches the D3D12 clear color.
    ExpectPixelRGBA8EQ(d3d11Texture.Get(), d3d12ClearColor, &signalFence);
}

// 1. Create and clear a D3D11 texture
// 2. Wrap it in a Dawn texture and clear the texture to two different colors.
// 3. Readback the texture with D3D11.
// 4. Verify the readback color was the final color cleared.
TEST_P(D3D12SharedHandleUsageTests, ClearTwiceInD3D12ReadbackInD3D11) {
    // TODO(crbug.com/dawn/735): This test appears to hang for
    // D3D12_Microsoft_Basic_Render_Driver_CPU when validation is enabled.
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsWARP() && IsBackendValidationEnabled());

    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    const wgpu::Color d3d11ClearColor{1.0f, 1.0f, 0.0f, 1.0f};
    wgpu::Texture dawnTexture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;
    WrapAndClearD3D11Texture(baseDawnDescriptor, baseD3dDescriptor, d3d11ClearColor, &dawnTexture,
                             &d3d11Texture, &externalImage, /*isInitialized=*/true);
    ASSERT_NE(dawnTexture.Get(), nullptr);

    const wgpu::Color d3d12ClearColor1{0.0f, 0.0f, 1.0f, 1.0f};
    ClearImage(dawnTexture, d3d12ClearColor1, device);

    const wgpu::Color d3d12ClearColor2{0.0f, 1.0f, 1.0f, 1.0f};
    ClearImage(dawnTexture, d3d12ClearColor2, device);

    dawn::native::d3d12::ExternalImageDXGIFenceDescriptor signalFence;
    externalImage->EndAccess(dawnTexture.Get(), &signalFence);
    dawnTexture.Destroy();

    // Now that Dawn (via D3D12) has finished writing to the texture, we should be
    // able to read it back by copying it to a staging texture and verifying the
    // color matches the last D3D12 clear color.
    ExpectPixelRGBA8EQ(d3d11Texture.Get(), d3d12ClearColor2, &signalFence);
}

// 1. Create and clear a D3D11 texture with clearColor
// 2. Import the texture with isInitialized = false
// 3. Verify clearColor is not visible in wrapped texture
TEST_P(D3D12SharedHandleUsageTests, UninitializedTextureIsCleared) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    const wgpu::Color clearColor{1.0f, 0.0f, 0.0f, 1.0f};
    wgpu::Texture dawnTexture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;
    WrapAndClearD3D11Texture(baseDawnDescriptor, baseD3dDescriptor, clearColor, &dawnTexture,
                             &d3d11Texture, &externalImage, /*isInitialized=*/false);
    ASSERT_NE(dawnTexture.Get(), nullptr);

    // Readback the destination texture and ensure it contains the colors we used
    // to clear the source texture on the D3D device.
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(0, 0, 0, 0), dawnTexture, 0, 0);

    dawn::native::d3d12::ExternalImageDXGIFenceDescriptor signalFence;
    externalImage->EndAccess(dawnTexture.Get(), &signalFence);
    dawnTexture.Destroy();
}

// 1. Create an external image from the DX11 texture.
// 2. Produce two Dawn textures from the external image.
// 3. Clear each Dawn texture and verify the texture was cleared to a unique color.
TEST_P(D3D12SharedHandleUsageTests, ReuseExternalImage) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    // Create the first Dawn texture then clear it to red.
    wgpu::Texture texture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;
    WrapSharedHandle(&baseDawnDescriptor, &baseD3dDescriptor, &texture, &d3d11Texture,
                     &externalImage);
    {
        const wgpu::Color solidRed{1.0f, 0.0f, 0.0f, 1.0f};
        ASSERT_NE(texture.Get(), nullptr);
        ClearImage(texture.Get(), solidRed, device);

        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(0xFF, 0, 0, 0xFF), texture.Get(), 0, 0);
    }

    // Once finished with the first texture, destroy it so we may re-acquire the external image
    // again.
    dawn::native::d3d12::ExternalImageDXGIFenceDescriptor signalFence;
    externalImage->EndAccess(texture.Get(), &signalFence);
    texture.Destroy();

    // Create another Dawn texture then clear it with another color.
    dawn::native::d3d12::ExternalImageDXGIBeginAccessDescriptor externalAccessDesc;
    externalAccessDesc.isInitialized = true;
    externalAccessDesc.usage = static_cast<WGPUTextureUsageFlags>(baseDawnDescriptor.usage);
    externalAccessDesc.waitFences.push_back(signalFence);

    texture = wgpu::Texture::Acquire(externalImage->BeginAccess(&externalAccessDesc));

    // Check again that the new texture is still red
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(0xFF, 0, 0, 0xFF), texture.Get(), 0, 0);

    // Clear the new texture to blue
    {
        const wgpu::Color solidBlue{0.0f, 0.0f, 1.0f, 1.0f};
        ASSERT_NE(texture.Get(), nullptr);
        ClearImage(texture.Get(), solidBlue, device);

        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(0, 0, 0xFF, 0xFF), texture.Get(), 0, 0);
    }

    externalImage->EndAccess(texture.Get(), &signalFence);
    texture.Destroy();
}

TEST_P(D3D12SharedHandleUsageTests, ConcurrentExternalImageReadAccess) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    DAWN_TEST_UNSUPPORTED_IF(GetParam().mSyncMode == SyncMode::kKeyedMutex);

    wgpu::Device device2 = CreateDevice();
    EXPECT_NE(device2, nullptr);

    wgpu::Device device3 = CreateDevice();
    EXPECT_NE(device3, nullptr);

    wgpu::Device device4 = CreateDevice();
    EXPECT_NE(device4, nullptr);

    wgpu::Device device5 = CreateDevice();
    EXPECT_NE(device5, nullptr);

    // Create Dawn texture with write access, then clear it to red.
    wgpu::Texture texture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;
    WrapSharedHandle(&baseDawnDescriptor, &baseD3dDescriptor, &texture, &d3d11Texture,
                     &externalImage);

    // Clear to red.
    const wgpu::Color solidRed{1.0f, 0.0f, 0.0f, 1.0f};
    ASSERT_NE(texture.Get(), nullptr);
    ClearImage(texture.Get(), solidRed, device);

    dawn::native::d3d12::ExternalImageDXGIFenceDescriptor signalFence;
    externalImage->EndAccess(texture.Get(), &signalFence);
    texture.Destroy();

    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage2 =
        CreateExternalImage(device2.Get(), d3d11Texture.Get(), &baseDawnDescriptor);
    EXPECT_NE(externalImage2, nullptr);

    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage3 =
        CreateExternalImage(device3.Get(), d3d11Texture.Get(), &baseDawnDescriptor);
    EXPECT_NE(externalImage3, nullptr);

    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage4 =
        CreateExternalImage(device4.Get(), d3d11Texture.Get(), &baseDawnDescriptor);
    EXPECT_NE(externalImage4, nullptr);

    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage5 =
        CreateExternalImage(device5.Get(), d3d11Texture.Get(), &baseDawnDescriptor);
    EXPECT_NE(externalImage5, nullptr);

    // Create two Dawn textures for concurrent read on second device.
    dawn::native::d3d12::ExternalImageDXGIBeginAccessDescriptor externalAccessDesc;
    externalAccessDesc.isInitialized = true;
    externalAccessDesc.usage = WGPUTextureUsage_CopySrc;
    externalAccessDesc.waitFences = {signalFence};

    // Concurrent read access on device 2 and 3.
    dawn::native::d3d12::ExternalImageDXGIFenceDescriptor signalFence2;
    dawn::native::d3d12::ExternalImageDXGIFenceDescriptor signalFence3;
    {
        wgpu::Texture texture2 =
            wgpu::Texture::Acquire(externalImage2->BeginAccess(&externalAccessDesc));
        EXPECT_NE(texture2, nullptr);

        wgpu::Texture texture3 =
            wgpu::Texture::Acquire(externalImage3->BeginAccess(&externalAccessDesc));
        EXPECT_NE(texture3, nullptr);

        // Check again that the new textures are also red.
        const utils::RGBA8 solidRed(0xFF, 0, 0, 0xFF);
        EXPECT_TEXTURE_EQ(device2, solidRed, texture2.Get(), {0, 0});
        EXPECT_TEXTURE_EQ(device3, solidRed, texture3.Get(), {0, 0});

        externalImage2->EndAccess(texture2.Get(), &signalFence2);
        texture2.Destroy();

        externalImage3->EndAccess(texture3.Get(), &signalFence3);
        texture3.Destroy();
    }

    externalAccessDesc.usage = static_cast<WGPUTextureUsageFlags>(baseDawnDescriptor.usage);
    externalAccessDesc.waitFences = {signalFence2, signalFence3};

    // Exclusive read-write access on device 4.
    dawn::native::d3d12::ExternalImageDXGIFenceDescriptor signalFence4;
    {
        wgpu::Texture texture4 =
            wgpu::Texture::Acquire(externalImage4->BeginAccess(&externalAccessDesc));
        EXPECT_NE(texture4, nullptr);

        const utils::RGBA8 solidRed(0xFF, 0, 0, 0xFF);
        EXPECT_TEXTURE_EQ(device4, solidRed, texture4.Get(), {0, 0});

        // Clear to blue.
        const wgpu::Color solidBlue{0.0f, 0.0f, 1.0f, 1.0f};
        ASSERT_NE(texture4.Get(), nullptr);
        ClearImage(texture4.Get(), solidBlue, device4);

        externalImage4->EndAccess(texture4.Get(), &signalFence4);
        texture4.Destroy();
    }

    externalAccessDesc.waitFences = {signalFence4};

    // Import texture on device 5, but do nothing with it.
    dawn::native::d3d12::ExternalImageDXGIFenceDescriptor signalFence5;
    {
        wgpu::Texture texture5 =
            wgpu::Texture::Acquire(externalImage5->BeginAccess(&externalAccessDesc));
        EXPECT_NE(texture5, nullptr);
        externalImage5->EndAccess(texture5.Get(), &signalFence5);
        texture5.Destroy();
    }

    externalAccessDesc.usage = WGPUTextureUsage_CopySrc;
    externalAccessDesc.waitFences = {signalFence5};

    // Concurrent read access on device 1 (twice), 2 and 3.
    {
        texture = wgpu::Texture::Acquire(externalImage->BeginAccess(&externalAccessDesc));
        EXPECT_NE(texture, nullptr);

        wgpu::Texture texture1 =
            wgpu::Texture::Acquire(externalImage->BeginAccess(&externalAccessDesc));
        EXPECT_NE(texture1, nullptr);

        wgpu::Texture texture2 =
            wgpu::Texture::Acquire(externalImage2->BeginAccess(&externalAccessDesc));
        EXPECT_NE(texture2, nullptr);

        wgpu::Texture texture3 =
            wgpu::Texture::Acquire(externalImage3->BeginAccess(&externalAccessDesc));
        EXPECT_NE(texture3, nullptr);

        // Check again that the new textures are now blue.
        const utils::RGBA8 solidBlue(0, 0, 0xFF, 0xFF);
        EXPECT_TEXTURE_EQ(device, solidBlue, texture.Get(), {0, 0});
        EXPECT_TEXTURE_EQ(device, solidBlue, texture1.Get(), {0, 0});
        EXPECT_TEXTURE_EQ(device2, solidBlue, texture2.Get(), {0, 0});
        EXPECT_TEXTURE_EQ(device3, solidBlue, texture3.Get(), {0, 0});

        externalImage->EndAccess(texture.Get(), &signalFence);
        texture.Destroy();

        externalImage->EndAccess(texture1.Get(), &signalFence);
        texture1.Destroy();

        externalImage2->EndAccess(texture2.Get(), &signalFence2);
        texture2.Destroy();

        externalImage3->EndAccess(texture3.Get(), &signalFence3);
        texture3.Destroy();
    }
}

// Produce a new texture with a usage not specified in the external image.
TEST_P(D3D12SharedHandleUsageTests, ExternalImageUsage) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    wgpu::Texture texture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;
    WrapSharedHandle(&baseDawnDescriptor, &baseD3dDescriptor, &texture, &d3d11Texture,
                     &externalImage);
    ASSERT_NE(texture.Get(), nullptr);

    dawn::native::d3d12::ExternalImageDXGIFenceDescriptor signalFence;
    externalImage->EndAccess(texture.Get(), &signalFence);
    texture.Destroy();

    dawn::native::d3d12::ExternalImageDXGIBeginAccessDescriptor externalAccessDesc;
    externalAccessDesc.isInitialized = true;
    externalAccessDesc.usage = WGPUTextureUsage_StorageBinding;
    externalAccessDesc.waitFences.push_back(signalFence);
    texture = wgpu::Texture::Acquire(externalImage->BeginAccess(&externalAccessDesc));
    ASSERT_EQ(texture.Get(), nullptr);

    externalAccessDesc.usage = WGPUTextureUsage_TextureBinding;
    texture = wgpu::Texture::Acquire(externalImage->BeginAccess(&externalAccessDesc));
    ASSERT_NE(texture.Get(), nullptr);
    externalImage->EndAccess(texture.Get(), &signalFence);
    texture.Destroy();
}

// Verify external image cannot be used after its creating device is destroyed.
TEST_P(D3D12SharedHandleUsageTests, InvalidateExternalImageOnDestroyDevice) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    wgpu::Texture texture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;

    // Create the Dawn texture then clear it to red.
    WrapSharedHandle(&baseDawnDescriptor, &baseD3dDescriptor, &texture, &d3d11Texture,
                     &externalImage);
    const wgpu::Color solidRed{1.0f, 0.0f, 0.0f, 1.0f};
    ASSERT_NE(texture.Get(), nullptr);
    ClearImage(texture.Get(), solidRed, device);

    dawn::native::d3d12::ExternalImageDXGIFenceDescriptor signalFence;
    externalImage->EndAccess(texture.Get(), &signalFence);
    texture.Destroy();

    // Do not readback pixels since that requires device to be alive during DawnTest::TearDown().
    DestroyDevice();

    dawn::native::d3d12::ExternalImageDXGIBeginAccessDescriptor externalAccessDesc;
    externalAccessDesc.isInitialized = true;
    externalAccessDesc.usage = static_cast<WGPUTextureUsageFlags>(baseDawnDescriptor.usage);

    EXPECT_EQ(wgpu::Texture::Acquire(externalImage->BeginAccess(&externalAccessDesc)), nullptr);
}

// Verify external image cannot be created after the target device is destroyed.
TEST_P(D3D12SharedHandleUsageTests, DisallowExternalImageAfterDestroyDevice) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    wgpu::Texture texture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;

    DestroyDevice();

    ASSERT_DEVICE_ERROR(WrapSharedHandle(&baseDawnDescriptor, &baseD3dDescriptor, &texture,
                                         &d3d11Texture, &externalImage));

    EXPECT_EQ(externalImage, nullptr);
    EXPECT_EQ(texture, nullptr);
}

// Verify there is no error generated when we destroy an external image with CommandRecordingContext
// open.
TEST_P(D3D12SharedHandleUsageTests, CallWriteBufferBeforeDestroyingExternalImage) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    wgpu::Texture texture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;
    WrapSharedHandle(&baseDawnDescriptor, &baseD3dDescriptor, &texture, &d3d11Texture,
                     &externalImage);

    // In utils::CreateBufferFromData() we will call queue.WriteBuffer(), which will make a
    // recording context pending.
    constexpr uint32_t kExpected = 1u;
    wgpu::Buffer buffer = utils::CreateBufferFromData(
        device, wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst, {kExpected});

    dawn::native::d3d12::ExternalImageDXGIFenceDescriptor signalFence;
    externalImage->EndAccess(texture.Get(), &signalFence);
    texture.Destroy();
    externalImage = nullptr;

    EXPECT_BUFFER_U32_EQ(kExpected, buffer, 0);
}

// Test that texture descriptor view formats are passed to the backend for wrapped external
// textures, and that contents may be reinterpreted as sRGB.
TEST_P(D3D12SharedHandleUsageTests, SRGBReinterpretation) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    wgpu::Texture texture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;

    // The texture will be reinterpreted as sRGB.
    wgpu::TextureViewDescriptor viewDesc = {};
    viewDesc.format = wgpu::TextureFormat::RGBA8UnormSrgb;

    wgpu::TextureDescriptor textureDesc = baseDawnDescriptor;
    textureDesc.viewFormatCount = 1;
    textureDesc.viewFormats = &viewDesc.format;
    // Check that the base format is not sRGB.
    ASSERT_EQ(textureDesc.format, wgpu::TextureFormat::RGBA8Unorm);

    // Wrap a shared handle as a Dawn texture.
    WrapSharedHandle(&textureDesc, &baseD3dDescriptor, &texture, &d3d11Texture, &externalImage);
    ASSERT_NE(texture.Get(), nullptr);

    // Submit a clear operation to sRGB value rgb(234, 51, 35).
    {
        utils::ComboRenderPassDescriptor renderPassDescriptor({texture.CreateView(&viewDesc)}, {});
        renderPassDescriptor.cColorAttachments[0].clearValue = {234.0 / 255.0, 51.0 / 255.0,
                                                                35.0 / 255.0, 1.0};
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.BeginRenderPass(&renderPassDescriptor).End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    // Expect the contents to be approximately rgb(246 124 104)
    EXPECT_PIXEL_RGBA8_BETWEEN(            //
        utils::RGBA8(245, 123, 103, 255),  //
        utils::RGBA8(247, 125, 105, 255), texture, 0, 0);
}

class D3D12SharedHandleMultithreadTests : public D3D12SharedHandleUsageTests {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> features;
        // TODO(crbug.com/dawn/1678): DawnWire doesn't support thread safe API yet.
        if (!UsesWire()) {
            features.push_back(wgpu::FeatureName::ImplicitDeviceSynchronization);
        }
        return features;
    }

    void SetUp() override {
        D3D12SharedHandleUsageTests::SetUp();
        // TODO(crbug.com/dawn/1678): DawnWire doesn't support thread safe API yet.
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    }
};

// Test the destroy device before destroying the external image won't cause deadlock.
TEST_P(D3D12SharedHandleMultithreadTests, DestroyDeviceBeforeImageNoDeadLock) {
    wgpu::Texture texture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;
    WrapSharedHandle(&baseDawnDescriptor, &baseD3dDescriptor, &texture, &d3d11Texture,
                     &externalImage);

    ASSERT_NE(texture.Get(), nullptr);

    dawn::native::d3d12::ExternalImageDXGIFenceDescriptor signalFence;
    externalImage->EndAccess(texture.Get(), &signalFence);
    EXPECT_TRUE(externalImage->IsValid());

    // Destroy device, it should destroy image internally.
    device.Destroy();
    EXPECT_FALSE(externalImage->IsValid());
}

// Test that using the external image and destroying the device simultaneously on different threads
// won't race.
TEST_P(D3D12SharedHandleMultithreadTests, DestroyDeviceAndUseImageInParallel) {
    wgpu::Texture texture;
    ComPtr<ID3D11Texture2D> d3d11Texture;
    std::unique_ptr<dawn::native::d3d12::ExternalImageDXGI> externalImage;
    WrapSharedHandle(&baseDawnDescriptor, &baseD3dDescriptor, &texture, &d3d11Texture,
                     &externalImage);

    ASSERT_NE(texture.Get(), nullptr);
    EXPECT_TRUE(externalImage->IsValid());

    std::thread thread1([&] {
        dawn::native::d3d12::ExternalImageDXGIFenceDescriptor signalFence;
        externalImage->EndAccess(texture.Get(), &signalFence);
    });

    std::thread thread2([&] {
        // Destroy device, it should destroy image internally.
        device.Destroy();
        EXPECT_FALSE(externalImage->IsValid());
    });

    thread1.join();
    thread2.join();
}

DAWN_INSTANTIATE_TEST_P(D3D12SharedHandleValidation,
                        {D3D12Backend()},
                        {SyncMode::kKeyedMutex, SyncMode::kFence});
DAWN_INSTANTIATE_TEST_P(D3D12SharedHandleUsageTests,
                        {D3D12Backend()},
                        {SyncMode::kKeyedMutex, SyncMode::kFence});
DAWN_INSTANTIATE_TEST_P(D3D12SharedHandleMultithreadTests,
                        {D3D12Backend()},
                        {SyncMode::kKeyedMutex, SyncMode::kFence});
