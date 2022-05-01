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

#ifndef SRC_DAWN_UTILS_BACKENDBINDING_H_
#define SRC_DAWN_UTILS_BACKENDBINDING_H_

#include "dawn/native/DawnNative.h"
#include "dawn/webgpu_cpp.h"

struct GLFWwindow;

namespace utils {

class BackendBinding {
  public:
    virtual ~BackendBinding() = default;

    virtual uint64_t GetSwapChainImplementation() = 0;
    virtual WGPUTextureFormat GetPreferredSwapChainTextureFormat() = 0;

  protected:
    BackendBinding(GLFWwindow* window, WGPUDevice device);

    GLFWwindow* mWindow = nullptr;
    WGPUDevice mDevice = nullptr;
};

void DiscoverAdapter(dawn::native::Instance* instance, GLFWwindow* window, wgpu::BackendType type);
BackendBinding* CreateBinding(wgpu::BackendType type, GLFWwindow* window, WGPUDevice device);

}  // namespace utils

#endif  // SRC_DAWN_UTILS_BACKENDBINDING_H_
