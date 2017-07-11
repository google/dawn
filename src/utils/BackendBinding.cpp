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

#include "common/Assert.h"

namespace utils {

    BackendBinding* CreateD3D12Binding();
    BackendBinding* CreateMetalBinding();
    BackendBinding* CreateOpenGLBinding();
    BackendBinding* CreateNullBinding();
    BackendBinding* CreateVulkanBinding();

    void BackendBinding::SetWindow(GLFWwindow* window) {
        this->window = window;
    }

    BackendBinding* CreateBinding(BackendType type) {
        switch (type) {
            case BackendType::D3D12:
                #if defined(_WIN32)
                    return CreateD3D12Binding();
                #else
                    return nullptr;
                #endif

            case BackendType::OpenGL:
                return CreateOpenGLBinding();

            case BackendType::Metal:
                #if defined(__APPLE__)
                    return CreateMetalBinding();
                #else
                    return nullptr;
                #endif

            case BackendType::Null:
                return CreateNullBinding();

            case BackendType::Vulkan:
                return nullptr; // TODO(cwallez@chromium.org) change it to CreateVulkanBinding();

            default:
                UNREACHABLE();
        }
    }

}
