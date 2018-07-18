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
#include "nxt/nxt_wsi.h"

#include "GLFW/glfw3.h"

#include <vector>

namespace backend { namespace vulkan {
    void Init(nxtProcTable* procs,
              nxtDevice* device,
              const std::vector<const char*>& requiredInstanceExtensions);

    VkInstance GetInstance(nxtDevice device);

    nxtSwapChainImplementation CreateNativeSwapChainImpl(nxtDevice device, VkSurfaceKHR surface);
    nxtTextureFormat GetNativeSwapChainPreferredFormat(const nxtSwapChainImplementation* swapChain);
}}  // namespace backend::vulkan

namespace utils {

    class SwapChainImplVulkan {
      public:
        using WSIContext = nxtWSIContextVulkan;

        SwapChainImplVulkan(GLFWwindow* /*window*/) {
        }

        ~SwapChainImplVulkan() {
        }

        void Init(nxtWSIContextVulkan*) {
        }

        nxtSwapChainError Configure(nxtTextureFormat, nxtTextureUsageBit, uint32_t, uint32_t) {
            return NXT_SWAP_CHAIN_NO_ERROR;
        }

        nxtSwapChainError GetNextTexture(nxtSwapChainNextTexture*) {
            return NXT_SWAP_CHAIN_NO_ERROR;
        }

        nxtSwapChainError Present() {
            return NXT_SWAP_CHAIN_NO_ERROR;
        }
    };

    class VulkanBinding : public BackendBinding {
      public:
        void SetupGLFWWindowHints() override {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }
        void GetProcAndDevice(nxtProcTable* procs, nxtDevice* device) override {
            uint32_t extensionCount = 0;
            const char** glfwInstanceExtensions =
                glfwGetRequiredInstanceExtensions(&extensionCount);
            std::vector<const char*> requiredExtensions(glfwInstanceExtensions,
                                                        glfwInstanceExtensions + extensionCount);

            backend::vulkan::Init(procs, device, requiredExtensions);
            mDevice = *device;
        }
        uint64_t GetSwapChainImplementation() override {
            if (mSwapchainImpl.userData == nullptr) {
                VkSurfaceKHR surface = VK_NULL_HANDLE;
                if (glfwCreateWindowSurface(backend::vulkan::GetInstance(mDevice), mWindow, nullptr,
                                            &surface) != VK_SUCCESS) {
                    ASSERT(false);
                }

                mSwapchainImpl = backend::vulkan::CreateNativeSwapChainImpl(mDevice, surface);
            }
            return reinterpret_cast<uint64_t>(&mSwapchainImpl);
        }
        nxtTextureFormat GetPreferredSwapChainTextureFormat() override {
            ASSERT(mSwapchainImpl.userData != nullptr);
            return backend::vulkan::GetNativeSwapChainPreferredFormat(&mSwapchainImpl);
        }

      private:
        nxtDevice mDevice;
        nxtSwapChainImplementation mSwapchainImpl = {};
    };

    BackendBinding* CreateVulkanBinding() {
        return new VulkanBinding;
    }

}  // namespace utils
