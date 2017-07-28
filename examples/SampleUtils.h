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

#include <nxt/nxtcpp.h>
#include <nxt/nxt_wsi.h>

bool InitSample(int argc, const char** argv);
void DoFlush();
bool ShouldQuit();

struct GLFWwindow;
struct GLFWwindow* GetGLFWWindow();

nxt::Device CreateCppNXTDevice();
uint64_t GetSwapChainImplementation();
nxt::SwapChain GetSwapChain(const nxt::Device& device);
nxt::RenderPass CreateDefaultRenderPass(const nxt::Device& device);
nxt::TextureView CreateDefaultDepthStencilView(const nxt::Device& device);
void GetNextFramebuffer(const nxt::Device& device,
    const nxt::RenderPass& renderPass,
    const nxt::SwapChain& swapchain,
    const nxt::TextureView& depthStencilView,
    nxt::Texture* backbuffer,
    nxt::Framebuffer* framebuffer);
