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

#include "tests/DawnTest.h"

#include <d3d11.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

#include "dawn_native/D3D12Backend.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/DawnHelpers.h"

using Microsoft::WRL::ComPtr;

namespace {

    class D3D12ResourceTestBase : public DawnTest {
      public:
        void TestSetUp() override {
            DawnTest::TestSetUp();
            if (UsesWire()) {
                return;
            }

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

            mD3d11Device = std::move(d3d11Device);
            mD3d11DeviceContext = std::move(d3d11DeviceContext);
        }

      protected:
        void WrapSharedHandle(const dawn::TextureDescriptor* dawnDescriptor,
                              const D3D11_TEXTURE2D_DESC* d3dDescriptor,
                              dawn::Texture* dawnTexture) const {
            ComPtr<ID3D11Texture2D> d3d11Texture;
            HRESULT hr = mD3d11Device->CreateTexture2D(d3dDescriptor, nullptr, &d3d11Texture);
            ASSERT_EQ(hr, S_OK);

            ComPtr<IDXGIResource1> dxgiResource;
            hr = d3d11Texture.As(&dxgiResource);
            ASSERT_EQ(hr, S_OK);

            HANDLE sharedHandle;
            hr = dxgiResource->CreateSharedHandle(
                nullptr, DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE, nullptr,
                &sharedHandle);
            ASSERT_EQ(hr, S_OK);

            DawnTexture texture = dawn_native::d3d12::WrapSharedHandle(
                device.Get(), reinterpret_cast<const DawnTextureDescriptor*>(dawnDescriptor),
                sharedHandle);
            // Now that we've created all of our resources, we can close the handle
            // since we no longer need it.
            ::CloseHandle(sharedHandle);

            *dawnTexture = dawn::Texture::Acquire(texture);
        }

        static constexpr size_t kTestWidth = 10;
        static constexpr size_t kTestHeight = 10;

        ComPtr<ID3D11Device> mD3d11Device;
        ComPtr<ID3D11DeviceContext> mD3d11DeviceContext;
    };

}  // anonymous namespace

// A small fixture used to initialize default data for the D3D12Resource validation tests.
// These tests are skipped if the harness is using the wire.
class D3D12SharedHandleValidation : public D3D12ResourceTestBase {
  public:
    void TestSetUp() override {
        D3D12ResourceTestBase::TestSetUp();

        dawnDescriptor.dimension = dawn::TextureDimension::e2D;
        dawnDescriptor.format = dawn::TextureFormat::BGRA8Unorm;
        dawnDescriptor.size = {kTestWidth, kTestHeight, 1};
        dawnDescriptor.sampleCount = 1;
        dawnDescriptor.arrayLayerCount = 1;
        dawnDescriptor.mipLevelCount = 1;
        dawnDescriptor.usage = dawn::TextureUsage::OutputAttachment;

        d3dDescriptor.Width = kTestWidth;
        d3dDescriptor.Height = kTestHeight;
        d3dDescriptor.MipLevels = 1;
        d3dDescriptor.ArraySize = 1;
        d3dDescriptor.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        d3dDescriptor.SampleDesc.Count = 1;
        d3dDescriptor.SampleDesc.Quality = 0;
        d3dDescriptor.Usage = D3D11_USAGE_DEFAULT;
        d3dDescriptor.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        d3dDescriptor.CPUAccessFlags = 0;
        d3dDescriptor.MiscFlags =
            D3D11_RESOURCE_MISC_SHARED_NTHANDLE | D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
    }

  protected:
    D3D11_TEXTURE2D_DESC d3dDescriptor;
    dawn::TextureDescriptor dawnDescriptor;
};

// Test a successful wrapping of an D3D12Resource in a texture
TEST_P(D3D12SharedHandleValidation, Success) {
    DAWN_SKIP_TEST_IF(UsesWire());

    dawn::Texture texture;
    WrapSharedHandle(&dawnDescriptor, &d3dDescriptor, &texture);

    ASSERT_NE(texture.Get(), nullptr);
}

// Test an error occurs if the texture descriptor is invalid
TEST_P(D3D12SharedHandleValidation, InvalidTextureDescriptor) {
    DAWN_SKIP_TEST_IF(UsesWire());
    dawnDescriptor.nextInChain = this;

    dawn::Texture texture;
    ASSERT_DEVICE_ERROR(WrapSharedHandle(&dawnDescriptor, &d3dDescriptor, &texture));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor mip level count isn't 1
TEST_P(D3D12SharedHandleValidation, InvalidMipLevelCount) {
    DAWN_SKIP_TEST_IF(UsesWire());
    dawnDescriptor.mipLevelCount = 2;

    dawn::Texture texture;
    ASSERT_DEVICE_ERROR(WrapSharedHandle(&dawnDescriptor, &d3dDescriptor, &texture));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor array layer count isn't 1
TEST_P(D3D12SharedHandleValidation, InvalidArrayLayerCount) {
    DAWN_SKIP_TEST_IF(UsesWire());
    dawnDescriptor.arrayLayerCount = 2;

    dawn::Texture texture;
    ASSERT_DEVICE_ERROR(WrapSharedHandle(&dawnDescriptor, &d3dDescriptor, &texture));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor sample count isn't 1
TEST_P(D3D12SharedHandleValidation, InvalidSampleCount) {
    DAWN_SKIP_TEST_IF(UsesWire());
    dawnDescriptor.sampleCount = 4;

    dawn::Texture texture;
    ASSERT_DEVICE_ERROR(WrapSharedHandle(&dawnDescriptor, &d3dDescriptor, &texture));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor width doesn't match the texture's
TEST_P(D3D12SharedHandleValidation, InvalidWidth) {
    DAWN_SKIP_TEST_IF(UsesWire());
    dawnDescriptor.size.width = kTestWidth + 1;

    dawn::Texture texture;
    ASSERT_DEVICE_ERROR(WrapSharedHandle(&dawnDescriptor, &d3dDescriptor, &texture));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor height doesn't match the texture's
TEST_P(D3D12SharedHandleValidation, InvalidHeight) {
    DAWN_SKIP_TEST_IF(UsesWire());
    dawnDescriptor.size.height = kTestHeight + 1;

    dawn::Texture texture;
    ASSERT_DEVICE_ERROR(WrapSharedHandle(&dawnDescriptor, &d3dDescriptor, &texture));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor format isn't compatible with the D3D12 Resource
TEST_P(D3D12SharedHandleValidation, InvalidFormat) {
    DAWN_SKIP_TEST_IF(UsesWire());
    dawnDescriptor.format = dawn::TextureFormat::R8Unorm;

    dawn::Texture texture;
    ASSERT_DEVICE_ERROR(WrapSharedHandle(&dawnDescriptor, &d3dDescriptor, &texture));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the number of D3D mip levels is greater than 1.
TEST_P(D3D12SharedHandleValidation, InvalidNumD3DMipLevels) {
    DAWN_SKIP_TEST_IF(UsesWire());
    d3dDescriptor.MipLevels = 2;

    dawn::Texture texture;
    ASSERT_DEVICE_ERROR(WrapSharedHandle(&dawnDescriptor, &d3dDescriptor, &texture));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the number of array levels is greater than 1.
TEST_P(D3D12SharedHandleValidation, InvalidD3DArraySize) {
    DAWN_SKIP_TEST_IF(UsesWire());
    d3dDescriptor.ArraySize = 2;

    dawn::Texture texture;
    ASSERT_DEVICE_ERROR(WrapSharedHandle(&dawnDescriptor, &d3dDescriptor, &texture));

    ASSERT_EQ(texture.Get(), nullptr);
}

DAWN_INSTANTIATE_TEST(D3D12SharedHandleValidation, D3D12Backend);
