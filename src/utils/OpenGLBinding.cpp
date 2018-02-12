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
#include "common/Platform.h"
#include "common/SwapChainUtils.h"
#include "nxt/nxt_wsi.h"

// Glad needs to be included before GLFW otherwise it complain that GL.h was already included
#include "glad/glad.h"

#include <cstdio>
#include "GLFW/glfw3.h"

namespace backend { namespace opengl {
    void Init(void* (*getProc)(const char*), nxtProcTable* procs, nxtDevice* device);
}}  // namespace backend::opengl

namespace utils {
    class SwapChainImplGL {
      public:
        using WSIContext = nxtWSIContextGL;

        SwapChainImplGL(GLFWwindow* window) : mWindow(window) {
        }

        ~SwapChainImplGL() {
            glDeleteTextures(1, &mBackTexture);
            glDeleteFramebuffers(1, &mBackFBO);
        }

        void Init(nxtWSIContextGL*) {
            glGenTextures(1, &mBackTexture);
            glBindTexture(GL_TEXTURE_2D, mBackTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

            glGenFramebuffers(1, &mBackFBO);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, mBackFBO);
            glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                   mBackTexture, 0);
        }

        nxtSwapChainError Configure(nxtTextureFormat format,
                                    nxtTextureUsageBit,
                                    uint32_t width,
                                    uint32_t height) {
            if (format != NXT_TEXTURE_FORMAT_R8_G8_B8_A8_UNORM) {
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

            return NXT_SWAP_CHAIN_NO_ERROR;
        }

        nxtSwapChainError GetNextTexture(nxtSwapChainNextTexture* nextTexture) {
            nextTexture->texture.u32 = mBackTexture;
            return NXT_SWAP_CHAIN_NO_ERROR;
        }

        nxtSwapChainError Present() {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, mBackFBO);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBlitFramebuffer(0, 0, mWidth, mHeight, 0, mHeight, mWidth, 0, GL_COLOR_BUFFER_BIT,
                              GL_NEAREST);
            glfwSwapBuffers(mWindow);

            return NXT_SWAP_CHAIN_NO_ERROR;
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
#if defined(NXT_PLATFORM_APPLE)
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
        void GetProcAndDevice(nxtProcTable* procs, nxtDevice* device) override {
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

        nxtTextureFormat GetPreferredSwapChainTextureFormat() override {
            return NXT_TEXTURE_FORMAT_R8_G8_B8_A8_UNORM;
        }

      private:
        nxtDevice mBackendDevice = nullptr;
        nxtSwapChainImplementation mSwapchainImpl = {};
    };

    BackendBinding* CreateOpenGLBinding() {
        return new OpenGLBinding;
    }

}  // namespace utils
