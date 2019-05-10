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

#include "dawn_native/d3d12/AdapterD3D12.h"

#include "common/Constants.h"
#include "dawn_native/d3d12/BackendD3D12.h"
#include "dawn_native/d3d12/DeviceD3D12.h"
#include "dawn_native/d3d12/PlatformFunctions.h"

#include <locale>

namespace dawn_native { namespace d3d12 {

    // utility wrapper to adapt locale-bound facets for wstring/wbuffer convert
    template <class Facet>
    struct DeletableFacet : Facet {
        template <class... Args>
        DeletableFacet(Args&&... args) : Facet(std::forward<Args>(args)...) {
        }

        ~DeletableFacet() {
        }
    };

    Adapter::Adapter(Backend* backend, ComPtr<IDXGIAdapter1> hardwareAdapter)
        : AdapterBase(backend->GetInstance(), BackendType::D3D12),
          mHardwareAdapter(hardwareAdapter),
          mBackend(backend) {
        DXGI_ADAPTER_DESC1 adapterDesc;
        mHardwareAdapter->GetDesc1(&adapterDesc);

        mPCIInfo.deviceId = adapterDesc.DeviceId;
        mPCIInfo.vendorId = adapterDesc.VendorId;

        if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            mDeviceType = DeviceType::CPU;
        } else {
            // Using DXGI_ADAPTER_DESC1 approach to determine integrated vs dedicated is
            // vendor-specific.
            switch (mPCIInfo.vendorId) {
                case kVendorID_Intel: {
                    // On Intel GPUs, dedicated video memory is always set to 128MB when the GPU is
                    // integrated.
                    static constexpr uint64_t kDedicatedVideoMemory = 128 * 1024 * 1024;
                    mDeviceType = (adapterDesc.DedicatedVideoMemory == kDedicatedVideoMemory)
                                      ? DeviceType::IntegratedGPU
                                      : DeviceType::DiscreteGPU;
                    break;
                }
                default:
                    // TODO: Support additional GPU vendors.
                    mDeviceType = DeviceType::Unknown;
                    break;
            }
        }

        std::wstring_convert<DeletableFacet<std::codecvt<wchar_t, char, std::mbstate_t>>> converter(
            "Error converting");
        mPCIInfo.name = converter.to_bytes(adapterDesc.Description);
    }

    Backend* Adapter::GetBackend() const {
        return mBackend;
    }

    ResultOrError<DeviceBase*> Adapter::CreateDeviceImpl(const DeviceDescriptor* descriptor) {
        ComPtr<ID3D12Device> d3d12Device;
        if (FAILED(mBackend->GetFunctions()->d3d12CreateDevice(
                mHardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device)))) {
            return DAWN_CONTEXT_LOST_ERROR("D3D12CreateDevice failed");
        }

        ASSERT(d3d12Device != nullptr);
        return new Device(this, d3d12Device, descriptor);
    }

}}  // namespace dawn_native::d3d12
