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
#include "dawn/native/d3d12/ExternalImageDXGIImpl.h"
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

ExternalImageDXGI::ExternalImageDXGI(std::unique_ptr<ExternalImageDXGIImpl> impl)
    : mImpl(std::move(impl)) {
    ASSERT(mImpl != nullptr);
}

ExternalImageDXGI::~ExternalImageDXGI() = default;

bool ExternalImageDXGI::IsValid() const {
    return mImpl->IsValid();
}

WGPUTexture ExternalImageDXGI::ProduceTexture(
    WGPUDevice device,
    const ExternalImageDXGIBeginAccessDescriptor* descriptor) {
    return BeginAccess(descriptor);
}

WGPUTexture ExternalImageDXGI::BeginAccess(
    const ExternalImageDXGIBeginAccessDescriptor* descriptor) {
    if (!IsValid()) {
        dawn::ErrorLog() << "Cannot use external image after device destruction";
        return nullptr;
    }
    return mImpl->BeginAccess(descriptor);
}

void ExternalImageDXGI::EndAccess(WGPUTexture texture,
                                  ExternalImageDXGIFenceDescriptor* signalFence) {
    if (!IsValid()) {
        dawn::ErrorLog() << "Cannot use external image after device destruction";
        return;
    }
    mImpl->EndAccess(texture, signalFence);
}

// static
std::unique_ptr<ExternalImageDXGI> ExternalImageDXGI::Create(
    WGPUDevice device,
    const ExternalImageDescriptorDXGISharedHandle* descriptor) {
    Device* backendDevice = ToBackend(FromAPI(device));
    std::unique_ptr<ExternalImageDXGIImpl> impl =
        backendDevice->CreateExternalImageDXGIImpl(descriptor);
    if (!impl) {
        dawn::ErrorLog() << "Failed to create DXGI external image";
        return nullptr;
    }
    return std::unique_ptr<ExternalImageDXGI>(new ExternalImageDXGI(std::move(impl)));
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
