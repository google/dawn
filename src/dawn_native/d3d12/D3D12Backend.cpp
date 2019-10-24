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

#include "dawn_native/D3D12Backend.h"

#include "common/SwapChainUtils.h"
#include "dawn_native/d3d12/DeviceD3D12.h"
#include "dawn_native/d3d12/NativeSwapChainImplD3D12.h"
#include "dawn_native/d3d12/TextureD3D12.h"

namespace dawn_native { namespace d3d12 {

    ComPtr<ID3D12Device> GetD3D12Device(WGPUDevice device) {
        Device* backendDevice = reinterpret_cast<Device*>(device);

        return backendDevice->GetD3D12Device();
    }

    DawnSwapChainImplementation CreateNativeSwapChainImpl(WGPUDevice device, HWND window) {
        Device* backendDevice = reinterpret_cast<Device*>(device);

        DawnSwapChainImplementation impl;
        impl = CreateSwapChainImplementation(new NativeSwapChainImpl(backendDevice, window));
        impl.textureUsage = WGPUTextureUsage_Present;

        return impl;
    }

    WGPUTextureFormat GetNativeSwapChainPreferredFormat(
        const DawnSwapChainImplementation* swapChain) {
        NativeSwapChainImpl* impl = reinterpret_cast<NativeSwapChainImpl*>(swapChain->userData);
        return static_cast<WGPUTextureFormat>(impl->GetPreferredFormat());
    }

    WGPUTexture WrapSharedHandle(WGPUDevice device,
                                 const WGPUTextureDescriptor* descriptor,
                                 HANDLE sharedHandle,
                                 uint64_t acquireMutexKey) {
        Device* backendDevice = reinterpret_cast<Device*>(device);
        const TextureDescriptor* backendDescriptor =
            reinterpret_cast<const TextureDescriptor*>(descriptor);
        TextureBase* texture =
            backendDevice->WrapSharedHandle(backendDescriptor, sharedHandle, acquireMutexKey);
        return reinterpret_cast<WGPUTexture>(texture);
    }
}}  // namespace dawn_native::d3d12
