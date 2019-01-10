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
#include "dawn_native/NullBackend.h"

namespace utils {

    class NullBinding : public BackendBinding {
      public:
        void SetupGLFWWindowHints() override {
        }
        dawnDevice CreateDevice() override {
            // Make an instance and find the null adapter
            mInstance = std::make_unique<dawn_native::Instance>();
            mInstance->DiscoverDefaultAdapters();

            std::vector<dawn_native::Adapter> adapters = mInstance->GetAdapters();
            for (dawn_native::Adapter adapter : adapters) {
                if (adapter.GetBackendType() == dawn_native::BackendType::Null) {
                    return adapter.CreateDevice();
                }
            }

            UNREACHABLE();
            return {};
        }
        uint64_t GetSwapChainImplementation() override {
            if (mSwapchainImpl.userData == nullptr) {
                mSwapchainImpl = dawn_native::null::CreateNativeSwapChainImpl();
            }
            return reinterpret_cast<uint64_t>(&mSwapchainImpl);
        }
        dawnTextureFormat GetPreferredSwapChainTextureFormat() override {
            return DAWN_TEXTURE_FORMAT_R8_G8_B8_A8_UNORM;
        }

      private:
        std::unique_ptr<dawn_native::Instance> mInstance;
        dawnSwapChainImplementation mSwapchainImpl = {};
    };

    BackendBinding* CreateNullBinding() {
        return new NullBinding;
    }

}  // namespace utils
