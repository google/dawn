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

#ifndef UTILS_BACKENDBINDING_H_
#define UTILS_BACKENDBINDING_H_

#include <dawn/dawn_wsi.h>

struct GLFWwindow;
typedef struct dawnProcTable_s dawnProcTable;
typedef struct dawnDeviceImpl* dawnDevice;

namespace utils {

    enum class BackendType {
        D3D12,
        Metal,
        OpenGL,
        Null,
        Vulkan,
    };

    class BackendBinding {
      public:
        virtual ~BackendBinding() = default;

        virtual void SetupGLFWWindowHints() = 0;
        virtual void GetProcAndDevice(dawnProcTable* procs, dawnDevice* device) = 0;
        virtual uint64_t GetSwapChainImplementation() = 0;
        virtual dawnTextureFormat GetPreferredSwapChainTextureFormat() = 0;

        void SetWindow(GLFWwindow* window);

      protected:
        GLFWwindow* mWindow = nullptr;
    };

    BackendBinding* CreateBinding(BackendType type);

}  // namespace utils

#endif  // UTILS_BACKENDBINDING_H_
