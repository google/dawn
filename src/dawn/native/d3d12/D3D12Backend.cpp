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

// D3D12Backend.cpp: contains the definition of symbols exported by D3D12Backend.h so that they
// can be compiled twice: once export (shared library), once not exported (static library)

#include "dawn/native/D3D12Backend.h"

#include <memory>
#include <utility>

#include "dawn/common/Log.h"
#include "dawn/common/Math.h"
#include "dawn/common/SwapChainUtils.h"
#include "dawn/native/d3d12/D3D11on12Util.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/NativeSwapChainImplD3D12.h"
#include "dawn/native/d3d12/ResidencyManagerD3D12.h"
#include "dawn/native/d3d12/TextureD3D12.h"

namespace dawn::native::d3d12 {

ComPtr<ID3D12Device> GetD3D12Device(WGPUDevice device) {
    return ToBackend(FromAPI(device))->GetD3D12Device();
}

DawnSwapChainImplementation CreateNativeSwapChainImpl(WGPUDevice device, HWND window) {
    Device* backendDevice = ToBackend(FromAPI(device));

    DawnSwapChainImplementation impl;
    impl = CreateSwapChainImplementation(new NativeSwapChainImpl(backendDevice, window));
    impl.textureUsage = WGPUTextureUsage_Present;

    return impl;
}

WGPUTextureFormat GetNativeSwapChainPreferredFormat(const DawnSwapChainImplementation* swapChain) {
    NativeSwapChainImpl* impl = reinterpret_cast<NativeSwapChainImpl*>(swapChain->userData);
    return static_cast<WGPUTextureFormat>(impl->GetPreferredFormat());
}

ExternalImageDescriptorDXGISharedHandle::ExternalImageDescriptorDXGISharedHandle()
    : ExternalImageDescriptor(ExternalImageType::DXGISharedHandle) {}

ExternalImageDXGI::ExternalImageDXGI(ComPtr<ID3D12Resource> d3d12Resource,
                                     const WGPUTextureDescriptor* descriptor)
    : mD3D12Resource(std::move(d3d12Resource)),
      mUsage(descriptor->usage),
      mDimension(descriptor->dimension),
      mSize(descriptor->size),
      mFormat(descriptor->format),
      mMipLevelCount(descriptor->mipLevelCount),
      mSampleCount(descriptor->sampleCount) {
    ASSERT(!descriptor->nextInChain ||
           descriptor->nextInChain->sType == WGPUSType_DawnTextureInternalUsageDescriptor);
    if (descriptor->nextInChain) {
        mUsageInternal =
            reinterpret_cast<const WGPUDawnTextureInternalUsageDescriptor*>(descriptor->nextInChain)
                ->internalUsage;
    }
    mD3D11on12ResourceCache = std::make_unique<D3D11on12ResourceCache>();
}

ExternalImageDXGI::~ExternalImageDXGI() = default;

WGPUTexture ExternalImageDXGI::ProduceTexture(
    WGPUDevice device,
    const ExternalImageAccessDescriptorDXGIKeyedMutex* descriptor) {
    Device* backendDevice = ToBackend(FromAPI(device));

    // Ensure the texture usage is allowed
    if (!IsSubset(descriptor->usage, mUsage)) {
        dawn::ErrorLog() << "Texture usage is not valid for external image";
        return nullptr;
    }

    TextureDescriptor textureDescriptor = {};
    textureDescriptor.usage = static_cast<wgpu::TextureUsage>(descriptor->usage);
    textureDescriptor.dimension = static_cast<wgpu::TextureDimension>(mDimension);
    textureDescriptor.size = {mSize.width, mSize.height, mSize.depthOrArrayLayers};
    textureDescriptor.format = static_cast<wgpu::TextureFormat>(mFormat);
    textureDescriptor.mipLevelCount = mMipLevelCount;
    textureDescriptor.sampleCount = mSampleCount;

    DawnTextureInternalUsageDescriptor internalDesc = {};
    if (mUsageInternal) {
        textureDescriptor.nextInChain = &internalDesc;
        internalDesc.internalUsage = static_cast<wgpu::TextureUsage>(mUsageInternal);
        internalDesc.sType = wgpu::SType::DawnTextureInternalUsageDescriptor;
    }

    Ref<D3D11on12ResourceCacheEntry> d3d11on12Resource =
        mD3D11on12ResourceCache->GetOrCreateD3D11on12Resource(device, mD3D12Resource.Get());
    if (d3d11on12Resource == nullptr) {
        dawn::ErrorLog() << "Unable to create 11on12 resource for external image";
        return nullptr;
    }

    Ref<TextureBase> texture = backendDevice->CreateD3D12ExternalTexture(
        &textureDescriptor, mD3D12Resource, std::move(d3d11on12Resource),
        descriptor->isSwapChainTexture, descriptor->isInitialized);

    return ToAPI(texture.Detach());
}

// static
std::unique_ptr<ExternalImageDXGI> ExternalImageDXGI::Create(
    WGPUDevice device,
    const ExternalImageDescriptorDXGISharedHandle* descriptor) {
    Device* backendDevice = ToBackend(FromAPI(device));

    Microsoft::WRL::ComPtr<ID3D12Resource> d3d12Resource;
    if (FAILED(backendDevice->GetD3D12Device()->OpenSharedHandle(descriptor->sharedHandle,
                                                                 IID_PPV_ARGS(&d3d12Resource)))) {
        return nullptr;
    }

    const TextureDescriptor* textureDescriptor = FromAPI(descriptor->cTextureDescriptor);

    if (backendDevice->ConsumedError(ValidateTextureDescriptor(backendDevice, textureDescriptor))) {
        return nullptr;
    }

    if (backendDevice->ConsumedError(
            ValidateTextureDescriptorCanBeWrapped(textureDescriptor),
            "validating that a D3D12 external image can be wrapped with %s", textureDescriptor)) {
        return nullptr;
    }

    if (backendDevice->ConsumedError(
            ValidateD3D12TextureCanBeWrapped(d3d12Resource.Get(), textureDescriptor))) {
        return nullptr;
    }

    // Shared handle is assumed to support resource sharing capability. The resource
    // shared capability tier must agree to share resources between D3D devices.
    const Format* format =
        backendDevice->GetInternalFormat(textureDescriptor->format).AcquireSuccess();
    if (format->IsMultiPlanar()) {
        if (backendDevice->ConsumedError(ValidateD3D12VideoTextureCanBeShared(
                backendDevice, D3D12TextureFormat(textureDescriptor->format)))) {
            return nullptr;
        }
    }

    std::unique_ptr<ExternalImageDXGI> result(
        new ExternalImageDXGI(std::move(d3d12Resource), descriptor->cTextureDescriptor));
    return result;
}

uint64_t SetExternalMemoryReservation(WGPUDevice device,
                                      uint64_t requestedReservationSize,
                                      MemorySegment memorySegment) {
    Device* backendDevice = ToBackend(FromAPI(device));

    return backendDevice->GetResidencyManager()->SetExternalMemoryReservation(
        memorySegment, requestedReservationSize);
}

AdapterDiscoveryOptions::AdapterDiscoveryOptions()
    : AdapterDiscoveryOptionsBase(WGPUBackendType_D3D12), dxgiAdapter(nullptr) {}

AdapterDiscoveryOptions::AdapterDiscoveryOptions(ComPtr<IDXGIAdapter> adapter)
    : AdapterDiscoveryOptionsBase(WGPUBackendType_D3D12), dxgiAdapter(std::move(adapter)) {}
}  // namespace dawn::native::d3d12
