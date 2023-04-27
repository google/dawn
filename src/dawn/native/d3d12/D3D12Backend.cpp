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
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/ExternalImageDXGIImpl.h"
#include "dawn/native/d3d12/ResidencyManagerD3D12.h"
#include "dawn/native/d3d12/TextureD3D12.h"

namespace dawn::native::d3d12 {

ComPtr<ID3D12Device> GetD3D12Device(WGPUDevice device) {
    return ToBackend(FromAPI(device))->GetD3D12Device();
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

WGPUTexture ExternalImageDXGI::BeginAccess(
    const ExternalImageDXGIBeginAccessDescriptor* descriptor) {
    return mImpl->BeginAccess(descriptor);
}

void ExternalImageDXGI::EndAccess(WGPUTexture texture,
                                  ExternalImageDXGIFenceDescriptor* signalFence) {
    mImpl->EndAccess(texture, signalFence);
}

// static
std::unique_ptr<ExternalImageDXGI> ExternalImageDXGI::Create(
    WGPUDevice device,
    const ExternalImageDescriptorDXGISharedHandle* descriptor) {
    Device* backendDevice = ToBackend(FromAPI(device));
    auto deviceLock(backendDevice->GetScopedLock());
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

    auto deviceLock(backendDevice->GetScopedLock());

    return backendDevice->GetResidencyManager()->SetExternalMemoryReservation(
        memorySegment, requestedReservationSize);
}

AdapterDiscoveryOptions::AdapterDiscoveryOptions() : AdapterDiscoveryOptions(nullptr) {}

AdapterDiscoveryOptions::AdapterDiscoveryOptions(ComPtr<IDXGIAdapter> adapter)
    : d3d::AdapterDiscoveryOptions(WGPUBackendType_D3D12, std::move(adapter)) {}

}  // namespace dawn::native::d3d12
