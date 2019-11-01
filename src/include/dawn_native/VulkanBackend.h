// Copyright 2018 The Dawn Authors
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

#ifndef DAWNNATIVE_VULKANBACKEND_H_
#define DAWNNATIVE_VULKANBACKEND_H_

#include <dawn/dawn_wsi.h>
#include <dawn_native/DawnNative.h>

#include <vulkan/vulkan.h>

#include <vector>

namespace dawn_native { namespace vulkan {

    // Common properties of external images
    struct ExternalImageDescriptor {
        const WGPUTextureDescriptor* cTextureDescriptor;  // Must match image creation params
        bool isCleared;               // Sets whether the texture will be cleared before use
        VkDeviceSize allocationSize;  // Must match VkMemoryAllocateInfo from image creation
        uint32_t memoryTypeIndex;     // Must match VkMemoryAllocateInfo from image creation
    };

    DAWN_NATIVE_EXPORT VkInstance GetInstance(WGPUDevice device);

    DAWN_NATIVE_EXPORT PFN_vkVoidFunction GetInstanceProcAddr(WGPUDevice device, const char* pName);

    DAWN_NATIVE_EXPORT DawnSwapChainImplementation CreateNativeSwapChainImpl(WGPUDevice device,
                                                                             VkSurfaceKHR surface);
    DAWN_NATIVE_EXPORT WGPUTextureFormat
    GetNativeSwapChainPreferredFormat(const DawnSwapChainImplementation* swapChain);

// Can't use DAWN_PLATFORM_LINUX since header included in both dawn and chrome
#ifdef __linux__
        // Descriptor for opaque file descriptor image import
        struct ExternalImageDescriptorOpaqueFD : ExternalImageDescriptor {
            int memoryFD;  // A file descriptor from an export of the memory of the image
            std::vector<int> waitFDs;  // File descriptors of semaphores which will be waited on
        };

        // Imports an external vulkan image from an opaque file descriptor. Internally, this uses
        // external memory / semaphore extensions to import the image. Then, waits on the provided
        // |descriptor->waitFDs| before the texture can be used. Finally, a signal semaphore
        // can be exported, transferring control back to the caller.
        // On failure, returns a nullptr
        DAWN_NATIVE_EXPORT WGPUTexture
        WrapVulkanImageOpaqueFD(WGPUDevice cDevice,
                                const ExternalImageDescriptorOpaqueFD* descriptor);

        // Exports a signal semaphore from a wrapped texture. This must be called on wrapped
        // textures before they are destroyed. On failure, returns -1
        DAWN_NATIVE_EXPORT int ExportSignalSemaphoreOpaqueFD(WGPUDevice cDevice,
                                                             WGPUTexture cTexture);
#endif  // __linux__
}}  // namespace dawn_native::vulkan

#endif  // DAWNNATIVE_VULKANBACKEND_H_
