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
#include "common/Platform.h"
#include "common/SwapChainUtils.h"
#include "dawn/dawn_wsi.h"

// Glad needs to be included before GLFW otherwise it complain that GL.h was already included
#include "glad/glad.h"

#include <cstdio>
#include "GLFW/glfw3.h"

namespace backend { namespace opengl {
    void Init(void* (*getProc)(const char*), nxtProcTable* procs, dawnDevice* device);
}}  // namespace backend::opengl

namespace utils {
    class SwapChainImplGL {
      public:
        using WSIContext = dawnWSIContextGL;

        SwapChainImplGL(GLFWwindow* window) : mWindow(window) {
        }

        ~SwapChainImplGL() {
            glDeleteTextures(1, &mBackTexture);
            glDeleteFramebuffers(1, &mBackFBO);
        }

        void Init(dawnWSIContextGL*) {
            glGenTextures(1, &mBackTexture);
            glBindTexture(GL_TEXTURE_2D, mBackTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

            glGenFramebuffers(1, &mBackFBO);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, mBackFBO);
            glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                   mBackTexture, 0);
        }

        dawnSwapChainError Configure(dawnTextureFormat format,
                                     dawnTextureUsageBit,
                                     uint32_t width,
                                     uint32_t height) {
            if (format != DAWN_TEXTURE_FORMAT_R8_G8_B8_A8_UNORM) {
                return "unsupported format";
            }
            ASSERT(width > 0);
            ASSERT(height > 0);
            mWidth = width;
            mHeight = height;

            glBindTexture(GL_TEXTURE_2D, mBackTexture);
            // Reallocate the texture
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         nullptr);

            return DAWN_SWAP_CHAIN_NO_ERROR;
        }

        dawnSwapChainError GetNextTexture(dawnSwapChainNextTexture* nextTexture) {
            nextTexture->texture.u32 = mBackTexture;
            return DAWN_SWAP_CHAIN_NO_ERROR;
        }

        dawnSwapChainError Present() {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, mBackFBO);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBlitFramebuffer(0, 0, mWidth, mHeight, 0, mHeight, mWidth, 0, GL_COLOR_BUFFER_BIT,
                              GL_NEAREST);
            glfwSwapBuffers(mWindow);

            return DAWN_SWAP_CHAIN_NO_ERROR;
        }

      private:
        GLFWwindow* mWindow = nullptr;
        uint32_t mWidth = 0;
        uint32_t mHeight = 0;
        GLuint mBackFBO = 0;
        GLuint mBackTexture = 0;
    };

    class OpenGLBinding : public BackendBinding {
      public:
        void SetupGLFWWindowHints() override {
#if defined(DAWN_PLATFORM_APPLE)
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
        }
        void GetProcAndDevice(nxtProcTable* procs, dawnDevice* device) override {
            glfwMakeContextCurrent(mWindow);
            backend::opengl::Init(reinterpret_cast<void* (*)(const char*)>(glfwGetProcAddress),
                                  procs, device);

            mBackendDevice = *device;
        }

        uint64_t GetSwapChainImplementation() override {
            if (mSwapchainImpl.userData == nullptr) {
                mSwapchainImpl = CreateSwapChainImplementation(new SwapChainImplGL(mWindow));
            }
            return reinterpret_cast<uint64_t>(&mSwapchainImpl);
        }

        dawnTextureFormat GetPreferredSwapChainTextureFormat() override {
            return DAWN_TEXTURE_FORMAT_R8_G8_B8_A8_UNORM;
        }

      private:
        dawnDevice mBackendDevice = nullptr;
        dawnSwapChainImplementation mSwapchainImpl = {};
    };

    BackendBinding* CreateOpenGLBinding() {
        return new OpenGLBinding;
    }

}  // namespace utils
