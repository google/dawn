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

#include <nxt/nxt.h>

#if defined(__cplusplus)
extern "C" {
#endif
    bool InitUtils(int argc, const char** argv);
    void SwapBuffers();
    bool ShouldQuit();

    struct GLFWwindow;
    struct GLFWwindow* GetWindow();
#if defined(__cplusplus)
}
#endif

// Yuck
#if defined(__cplusplus)
    #include <nxt/nxtcpp.h>
    void GetProcTableAndDevice(nxtProcTable* procs, nxt::Device* device);
    nxt::ShaderModule CreateShaderModule(const nxt::Device& device, nxt::ShaderStage stage, const char* source);
#else
    void GetProcTableAndDevice(nxtProcTable* procs, nxtDevice* device);
    nxtShaderModule CreateShaderModule(nxtDevice device, nxtShaderStage stage, const char* source);
#endif
