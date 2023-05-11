// Copyright 2023 The Dawn Authors
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

#include "dawn/native/d3d/PhysicalDeviceD3D.h"

#include <string>
#include <utility>

#include "dawn/common/WindowsUtils.h"
#include "dawn/native/d3d/BackendD3D.h"

namespace dawn::native::d3d {

PhysicalDevice::PhysicalDevice(Backend* backend,
                               ComPtr<IDXGIAdapter3> hardwareAdapter,
                               wgpu::BackendType backendType)
    : PhysicalDeviceBase(backend->GetInstance(), backendType),
      mHardwareAdapter(std::move(hardwareAdapter)),
      mBackend(backend) {}

PhysicalDevice::~PhysicalDevice() = default;

IDXGIAdapter3* PhysicalDevice::GetHardwareAdapter() const {
    return mHardwareAdapter.Get();
}

Backend* PhysicalDevice::GetBackend() const {
    return mBackend;
}

MaybeError PhysicalDevice::InitializeImpl() {
    DXGI_ADAPTER_DESC1 adapterDesc;
    GetHardwareAdapter()->GetDesc1(&adapterDesc);

    mDeviceId = adapterDesc.DeviceId;
    mVendorId = adapterDesc.VendorId;
    mName = WCharToUTF8(adapterDesc.Description);

    if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        mAdapterType = wgpu::AdapterType::CPU;
    } else {
        // Assume it is a discrete GPU. If it is an integrated GPU, it will be overwritten later.
        mAdapterType = wgpu::AdapterType::DiscreteGPU;
    }

    // Convert the adapter's D3D driver version to a readable string like "24.21.13.9793".
    LARGE_INTEGER umdVersion;
    if (GetHardwareAdapter()->CheckInterfaceSupport(__uuidof(IDXGIDevice), &umdVersion) !=
        DXGI_ERROR_UNSUPPORTED) {
        uint64_t encodedVersion = umdVersion.QuadPart;
        uint16_t mask = 0xFFFF;
        mDriverVersion = {static_cast<uint16_t>((encodedVersion >> 48) & mask),
                          static_cast<uint16_t>((encodedVersion >> 32) & mask),
                          static_cast<uint16_t>((encodedVersion >> 16) & mask),
                          static_cast<uint16_t>(encodedVersion & mask)};
        switch (GetBackendType()) {
            case wgpu::BackendType::D3D11:
                mDriverDescription =
                    std::string("D3D11 driver version ") + mDriverVersion.ToString();
                break;
            case wgpu::BackendType::D3D12:
                mDriverDescription =
                    std::string("D3D12 driver version ") + mDriverVersion.ToString();
                break;
            default:
                UNREACHABLE();
        }
    }

    return {};
}

}  // namespace dawn::native::d3d
