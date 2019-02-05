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

namespace utils {

#if defined(DAWN_ENABLE_BACKEND_D3D12)
    BackendBinding* CreateD3D12Binding();
#endif
#if defined(DAWN_ENABLE_BACKEND_METAL)
    BackendBinding* CreateMetalBinding();
#endif
#if defined(DAWN_ENABLE_BACKEND_NULL)
    BackendBinding* CreateNullBinding();
#endif
#if defined(DAWN_ENABLE_BACKEND_OPENGL)
    BackendBinding* CreateOpenGLBinding();
#endif
#if defined(DAWN_ENABLE_BACKEND_VULKAN)
    BackendBinding* CreateVulkanBinding();
#endif

    void BackendBinding::SetWindow(GLFWwindow* window) {
        mWindow = window;
    }

    BackendBinding* CreateBinding(dawn_native::BackendType type) {
        switch (type) {
#if defined(DAWN_ENABLE_BACKEND_D3D12)
            case dawn_native::BackendType::D3D12:
                return CreateD3D12Binding();
#endif

#if defined(DAWN_ENABLE_BACKEND_METAL)
            case dawn_native::BackendType::Metal:
                return CreateMetalBinding();
#endif

#if defined(DAWN_ENABLE_BACKEND_NULL)
            case dawn_native::BackendType::Null:
                return CreateNullBinding();
#endif

#if defined(DAWN_ENABLE_BACKEND_OPENGL)
            case dawn_native::BackendType::OpenGL:
                return CreateOpenGLBinding();
#endif

#if defined(DAWN_ENABLE_BACKEND_VULKAN)
            case dawn_native::BackendType::Vulkan:
                return CreateVulkanBinding();
#endif

            default:
                return nullptr;
        }
    }

}  // namespace utils
