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

#include "SampleUtils.h"

#include "common/Assert.h"
#include "common/Platform.h"
#include "utils/BackendBinding.h"
#include "wire/TerribleCommandBuffer.h"

#include <nxt/nxt.h>
#include <nxt/nxtcpp.h>
#include <nxt/nxt_wsi.h>
#include "GLFW/glfw3.h"

#include <cstring>
#include <iostream>

void PrintDeviceError(const char* message, nxt::CallbackUserdata) {
    std::cout << "Device error: " << message << std::endl;
}

void PrintGLFWError(int code, const char* message) {
    std::cout << "GLFW error: " << code << " - " << message << std::endl;
}

enum class CmdBufType {
    None,
    Terrible,
    //TODO(cwallez@chromium.org) double terrible cmdbuf
};

// Default to D3D12, Metal, Vulkan, OpenGL in that order as D3D12 and Metal are the preferred on
// their respective platforms, and Vulkan is preferred to OpenGL
#if defined(NXT_ENABLE_BACKEND_D3D12)
    static utils::BackendType backendType = utils::BackendType::D3D12;
#elif defined(NXT_ENABLE_BACKEND_METAL)
    static utils::BackendType backendType = utils::BackendType::Metal;
#elif defined(NXT_ENABLE_BACKEND_OPENGL)
    static utils::BackendType backendType = utils::BackendType::OpenGL;
#elif defined(NXT_ENABLE_BACKEND_VULKAN)
    static utils::BackendType backendType = utils::BackendType::Vulkan;
#else
    #error
#endif

static CmdBufType cmdBufType = CmdBufType::Terrible;
static utils::BackendBinding* binding = nullptr;

static GLFWwindow* window = nullptr;

static nxt::wire::CommandHandler* wireServer = nullptr;
static nxt::wire::CommandHandler* wireClient = nullptr;
static nxt::wire::TerribleCommandBuffer* c2sBuf = nullptr;
static nxt::wire::TerribleCommandBuffer* s2cBuf = nullptr;

nxt::Device CreateCppNXTDevice() {
    binding = utils::CreateBinding(backendType);
    if (binding == nullptr) {
        return nxt::Device();
    }

    glfwSetErrorCallback(PrintGLFWError);
    if (!glfwInit()) {
        return nxt::Device();
    }

    binding->SetupGLFWWindowHints();
    window = glfwCreateWindow(640, 480, "NXT window", nullptr, nullptr);
    if (!window) {
        return nxt::Device();
    }

    binding->SetWindow(window);

    nxtDevice backendDevice;
    nxtProcTable backendProcs;
    binding->GetProcAndDevice(&backendProcs, &backendDevice);

    nxtDevice cDevice = nullptr;
    nxtProcTable procs;
    switch (cmdBufType) {
        case CmdBufType::None:
            procs = backendProcs;
            cDevice = backendDevice;
            break;

        case CmdBufType::Terrible:
            {
                c2sBuf = new nxt::wire::TerribleCommandBuffer();
                s2cBuf = new nxt::wire::TerribleCommandBuffer();

                wireServer = nxt::wire::NewServerCommandHandler(backendDevice, backendProcs, s2cBuf);
                c2sBuf->SetHandler(wireServer);

                nxtDevice clientDevice;
                nxtProcTable clientProcs;
                wireClient = nxt::wire::NewClientDevice(&clientProcs, &clientDevice, c2sBuf);
                s2cBuf->SetHandler(wireClient);

                procs = clientProcs;
                cDevice = clientDevice;
            }
            break;
    }

    nxtSetProcs(&procs);
    procs.deviceSetErrorCallback(cDevice, PrintDeviceError, 0);
    return nxt::Device::Acquire(cDevice);
}

uint64_t GetSwapChainImplementation() {
    return binding->GetSwapChainImplementation();
}

nxt::TextureFormat GetPreferredSwapChainTextureFormat() {
    DoFlush();
    return static_cast<nxt::TextureFormat>(binding->GetPreferredSwapChainTextureFormat());
}

nxt::SwapChain GetSwapChain(const nxt::Device &device) {
    return device.CreateSwapChainBuilder()
        .SetImplementation(GetSwapChainImplementation())
        .GetResult();
}

nxt::TextureView CreateDefaultDepthStencilView(const nxt::Device& device) {
    auto depthStencilTexture = device.CreateTextureBuilder()
        .SetDimension(nxt::TextureDimension::e2D)
        .SetExtent(640, 480, 1)
        .SetFormat(nxt::TextureFormat::D32FloatS8Uint)
        .SetMipLevels(1)
        .SetAllowedUsage(nxt::TextureUsageBit::OutputAttachment)
        .GetResult();
    return depthStencilTexture.CreateTextureViewBuilder()
        .GetResult();
}

void GetNextRenderPassDescriptor(const nxt::Device& device,
    const nxt::SwapChain& swapchain,
    const nxt::TextureView& depthStencilView,
    nxt::Texture* backbuffer,
    nxt::RenderPassDescriptor* info) {
    *backbuffer = swapchain.GetNextTexture();
    auto backbufferView = backbuffer->CreateTextureViewBuilder().GetResult();
    *info = device.CreateRenderPassDescriptorBuilder()
        .SetColorAttachment(0, backbufferView, nxt::LoadOp::Clear)
        .SetDepthStencilAttachment(depthStencilView, nxt::LoadOp::Clear, nxt::LoadOp::Clear)
        .GetResult();
}

bool InitSample(int argc, const char** argv) {
    for (int i = 1; i < argc; i++) {
        if (std::string("-b") == argv[i] || std::string("--backend") == argv[i]) {
            i++;
            if (i < argc && std::string("d3d12") == argv[i]) {
                backendType = utils::BackendType::D3D12;
                continue;
            }
            if (i < argc && std::string("metal") == argv[i]) {
                backendType = utils::BackendType::Metal;
                continue;
            }
            if (i < argc && std::string("null") == argv[i]) {
                backendType = utils::BackendType::Null;
                continue;
            }
            if (i < argc && std::string("opengl") == argv[i]) {
                backendType = utils::BackendType::OpenGL;
                continue;
            }
            if (i < argc && std::string("vulkan") == argv[i]) {
                backendType = utils::BackendType::Vulkan;
                continue;
            }
            fprintf(stderr, "--backend expects a backend name (opengl, metal, d3d12, null, vulkan)\n");
            return false;
        }
        if (std::string("-c") == argv[i] || std::string("--command-buffer") == argv[i]) {
            i++;
            if (i < argc && std::string("none") == argv[i]) {
                cmdBufType = CmdBufType::None;
                continue;
            }
            if (i < argc && std::string("terrible") == argv[i]) {
                cmdBufType = CmdBufType::Terrible;
                continue;
            }
            fprintf(stderr, "--command-buffer expects a command buffer name (none, terrible)\n");
            return false;
        }
        if (std::string("-h") == argv[i] || std::string("--help") == argv[i]) {
            printf("Usage: %s [-b BACKEND] [-c COMMAND_BUFFER]\n", argv[0]);
            printf("  BACKEND is one of: d3d12, metal, null, opengl, vulkan\n");
            printf("  COMMAND_BUFFER is one of: none, terrible\n");
            return false;
        }
    }
    return true;
}

void DoFlush() {
    if (cmdBufType == CmdBufType::Terrible) {
        ASSERT(c2sBuf->Flush());
        ASSERT(s2cBuf->Flush());
    }
    glfwPollEvents();
}

bool ShouldQuit() {
    return glfwWindowShouldClose(window);
}

GLFWwindow* GetGLFWWindow() {
    return window;
}
