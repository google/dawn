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

#include "GLFW/glfw3.h"

namespace backend {
    namespace opengl {
        void Init(void* (*getProc)(const char*), nxtProcTable* procs, nxtDevice* device);
        void HACKCLEAR(nxtDevice device);
        void InitBackbuffer(nxtDevice device);
        void CommitBackbuffer(nxtDevice device);
    }
}

namespace utils {
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
                    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
                    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
                    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
                #endif
            }
            void GetProcAndDevice(nxtProcTable* procs, nxtDevice* device) override {
                glfwMakeContextCurrent(window);
                backend::opengl::Init(reinterpret_cast<void*(*)(const char*)>(glfwGetProcAddress), procs, device);

                backendDevice = *device;
                backend::opengl::InitBackbuffer(backendDevice);
            }
            void SwapBuffers() override {
                backend::opengl::CommitBackbuffer(backendDevice);
                glfwSwapBuffers(window);
                backend::opengl::HACKCLEAR(backendDevice);
            }

        private:
            nxtDevice backendDevice = nullptr;
    };

    BackendBinding* CreateOpenGLBinding() {
        return new OpenGLBinding;
    }

}
