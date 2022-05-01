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

#include <cstdio>

#include "dawn/utils/BackendBinding.h"

#include "dawn/common/Assert.h"
#include "dawn/common/Platform.h"
#include "dawn/common/SwapChainUtils.h"
#include "dawn/dawn_wsi.h"
#include "dawn/native/OpenGLBackend.h"

#include "GLFW/glfw3.h"

namespace utils {

class OpenGLBinding : public BackendBinding {
  public:
    OpenGLBinding(GLFWwindow* window, WGPUDevice device) : BackendBinding(window, device) {}

    uint64_t GetSwapChainImplementation() override {
        if (mSwapchainImpl.userData == nullptr) {
            mSwapchainImpl = dawn::native::opengl::CreateNativeSwapChainImpl(
                mDevice,
                [](void* userdata) { glfwSwapBuffers(static_cast<GLFWwindow*>(userdata)); },
                mWindow);
        }
        return reinterpret_cast<uint64_t>(&mSwapchainImpl);
    }

    WGPUTextureFormat GetPreferredSwapChainTextureFormat() override {
        return dawn::native::opengl::GetNativeSwapChainPreferredFormat(&mSwapchainImpl);
    }

  private:
    DawnSwapChainImplementation mSwapchainImpl = {};
};

BackendBinding* CreateOpenGLBinding(GLFWwindow* window, WGPUDevice device) {
    return new OpenGLBinding(window, device);
}

}  // namespace utils
