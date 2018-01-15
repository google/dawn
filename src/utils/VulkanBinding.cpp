// Copyright 2017 The NXT Authors
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

#include "common/SwapChainUtils.h"
#include "nxt/nxt_wsi.h"

namespace backend { namespace vulkan {
    void Init(nxtProcTable* procs, nxtDevice* device);
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
        }
        void GetProcAndDevice(nxtProcTable* procs, nxtDevice* device) override {
            backend::vulkan::Init(procs, device);
        }
        uint64_t GetSwapChainImplementation() override {
            if (mSwapchainImpl.userData == nullptr) {
                mSwapchainImpl = CreateSwapChainImplementation(new SwapChainImplVulkan(mWindow));
            }
            return reinterpret_cast<uint64_t>(&mSwapchainImpl);
        }
        nxtTextureFormat GetPreferredSwapChainTextureFormat() override {
            return NXT_TEXTURE_FORMAT_R8_G8_B8_A8_UNORM;
        }

      private:
        nxtSwapChainImplementation mSwapchainImpl = {};
    };

    BackendBinding* CreateVulkanBinding() {
        return new VulkanBinding;
    }

}  // namespace utils
