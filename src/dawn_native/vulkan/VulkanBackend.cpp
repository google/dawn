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

// VulkanBackend.cpp: contains the definition of symbols exported by VulkanBackend.h so that they
// can be compiled twice: once export (shared library), once not exported (static library)

// Include vulkan_platform.h before VulkanBackend.h includes vulkan.h so that we use our version
// of the non-dispatchable handles.
#include "common/vulkan_platform.h"

#include "dawn_native/VulkanBackend.h"

#include "common/SwapChainUtils.h"
#include "dawn_native/vulkan/DeviceVk.h"
#include "dawn_native/vulkan/NativeSwapChainImplVk.h"

namespace dawn_native { namespace vulkan {

    dawnDevice CreateDevice() {
        return reinterpret_cast<dawnDevice>(new Device());
    }

    VkInstance GetInstance(dawnDevice device) {
        Device* backendDevice = reinterpret_cast<Device*>(device);
        return backendDevice->GetInstance();
    }

    DAWN_NATIVE_EXPORT dawnSwapChainImplementation
    CreateNativeSwapChainImpl(dawnDevice device, VkSurfaceKHRNative surfaceNative) {
        Device* backendDevice = reinterpret_cast<Device*>(device);
        VkSurfaceKHR surface = VkSurfaceKHR::CreateFromHandle(surfaceNative);

        dawnSwapChainImplementation impl;
        impl = CreateSwapChainImplementation(new NativeSwapChainImpl(backendDevice, surface));
        impl.textureUsage = DAWN_TEXTURE_USAGE_BIT_PRESENT;

        return impl;
    }

    dawnTextureFormat GetNativeSwapChainPreferredFormat(
        const dawnSwapChainImplementation* swapChain) {
        NativeSwapChainImpl* impl = reinterpret_cast<NativeSwapChainImpl*>(swapChain->userData);
        return static_cast<dawnTextureFormat>(impl->GetPreferredFormat());
    }

}}  // namespace dawn_native::vulkan
