// Copyright 2017 The Dawn Authors
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

#include "utils/BackendBinding.h"

#include "common/Assert.h"
#include "common/vulkan_platform.h"
#include "dawn/dawn_wsi.h"

#include "GLFW/glfw3.h"

#include <vector>

namespace dawn_native { namespace vulkan {
    void Init(dawnProcTable* procs,
              dawnDevice* device,
              const std::vector<const char*>& requiredInstanceExtensions);

    VkInstance GetInstance(dawnDevice device);

    dawnSwapChainImplementation CreateNativeSwapChainImpl(dawnDevice device, VkSurfaceKHR surface);
    dawnTextureFormat GetNativeSwapChainPreferredFormat(
        const dawnSwapChainImplementation* swapChain);
}}  // namespace dawn_native::vulkan

namespace utils {

    class SwapChainImplVulkan {
      public:
        using WSIContext = dawnWSIContextVulkan;

        SwapChainImplVulkan(GLFWwindow* /*window*/) {
        }

        ~SwapChainImplVulkan() {
        }

        void Init(dawnWSIContextVulkan*) {
        }

        dawnSwapChainError Configure(dawnTextureFormat, dawnTextureUsageBit, uint32_t, uint32_t) {
            return DAWN_SWAP_CHAIN_NO_ERROR;
        }

        dawnSwapChainError GetNextTexture(dawnSwapChainNextTexture*) {
            return DAWN_SWAP_CHAIN_NO_ERROR;
        }

        dawnSwapChainError Present() {
            return DAWN_SWAP_CHAIN_NO_ERROR;
        }
    };

    class VulkanBinding : public BackendBinding {
      public:
        void SetupGLFWWindowHints() override {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }
        void GetProcAndDevice(dawnProcTable* procs, dawnDevice* device) override {
            uint32_t extensionCount = 0;
            const char** glfwInstanceExtensions =
                glfwGetRequiredInstanceExtensions(&extensionCount);
            std::vector<const char*> requiredExtensions(glfwInstanceExtensions,
                                                        glfwInstanceExtensions + extensionCount);

            dawn_native::vulkan::Init(procs, device, requiredExtensions);
            mDevice = *device;
        }
        uint64_t GetSwapChainImplementation() override {
            if (mSwapchainImpl.userData == nullptr) {
                VkSurfaceKHR surface = VK_NULL_HANDLE;
                if (glfwCreateWindowSurface(dawn_native::vulkan::GetInstance(mDevice), mWindow,
                                            nullptr, &surface) != VK_SUCCESS) {
                    ASSERT(false);
                }

                mSwapchainImpl = dawn_native::vulkan::CreateNativeSwapChainImpl(mDevice, surface);
            }
            return reinterpret_cast<uint64_t>(&mSwapchainImpl);
        }
        dawnTextureFormat GetPreferredSwapChainTextureFormat() override {
            ASSERT(mSwapchainImpl.userData != nullptr);
            return dawn_native::vulkan::GetNativeSwapChainPreferredFormat(&mSwapchainImpl);
        }

      private:
        dawnDevice mDevice;
        dawnSwapChainImplementation mSwapchainImpl = {};
    };

    BackendBinding* CreateVulkanBinding() {
        return new VulkanBinding;
    }

}  // namespace utils
