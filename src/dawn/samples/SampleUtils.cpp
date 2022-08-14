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

#include "dawn/samples/SampleUtils.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "GLFW/glfw3.h"
#include "dawn/common/Assert.h"
#include "dawn/common/Log.h"
#include "dawn/common/Platform.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/dawn_proc.h"
#include "dawn/dawn_wsi.h"
#include "dawn/native/DawnNative.h"
#include "dawn/utils/TerribleCommandBuffer.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireServer.h"
#include "webgpu/webgpu_glfw.h"

void PrintDeviceError(WGPUErrorType errorType, const char* message, void*) {
    const char* errorTypeName = "";
    switch (errorType) {
        case WGPUErrorType_Validation:
            errorTypeName = "Validation";
            break;
        case WGPUErrorType_OutOfMemory:
            errorTypeName = "Out of memory";
            break;
        case WGPUErrorType_Unknown:
            errorTypeName = "Unknown";
            break;
        case WGPUErrorType_DeviceLost:
            errorTypeName = "Device lost";
            break;
        default:
            UNREACHABLE();
            return;
    }
    dawn::ErrorLog() << errorTypeName << " error: " << message;
}

void PrintGLFWError(int code, const char* message) {
    dawn::ErrorLog() << "GLFW error: " << code << " - " << message;
}

enum class CmdBufType {
    None,
    Terrible,
    // TODO(cwallez@chromium.org): double terrible cmdbuf
};

// Default to D3D12, Metal, Vulkan, OpenGL in that order as D3D12 and Metal are the preferred on
// their respective platforms, and Vulkan is preferred to OpenGL
#if defined(DAWN_ENABLE_BACKEND_D3D12)
static wgpu::BackendType backendType = wgpu::BackendType::D3D12;
#elif defined(DAWN_ENABLE_BACKEND_METAL)
static wgpu::BackendType backendType = wgpu::BackendType::Metal;
#elif defined(DAWN_ENABLE_BACKEND_VULKAN)
static wgpu::BackendType backendType = wgpu::BackendType::Vulkan;
#elif defined(DAWN_ENABLE_BACKEND_OPENGLES)
static wgpu::BackendType backendType = wgpu::BackendType::OpenGLES;
#elif defined(DAWN_ENABLE_BACKEND_DESKTOP_GL)
static wgpu::BackendType backendType = wgpu::BackendType::OpenGL;
#else
#error
#endif

static CmdBufType cmdBufType = CmdBufType::Terrible;
static std::unique_ptr<dawn::native::Instance> instance;
static wgpu::SwapChain swapChain;

static GLFWwindow* window = nullptr;

static dawn::wire::WireServer* wireServer = nullptr;
static dawn::wire::WireClient* wireClient = nullptr;
static utils::TerribleCommandBuffer* c2sBuf = nullptr;
static utils::TerribleCommandBuffer* s2cBuf = nullptr;

wgpu::Device CreateCppDawnDevice() {
    ScopedEnvironmentVar angleDefaultPlatform;
    if (GetEnvironmentVar("ANGLE_DEFAULT_PLATFORM").first.empty()) {
        angleDefaultPlatform.Set("ANGLE_DEFAULT_PLATFORM", "swiftshader");
    }

    glfwSetErrorCallback(PrintGLFWError);
    if (!glfwInit()) {
        return wgpu::Device();
    }

    // Create the test window with no client API.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
    window = glfwCreateWindow(640, 480, "Dawn window", nullptr, nullptr);
    if (!window) {
        return wgpu::Device();
    }

    instance = std::make_unique<dawn::native::Instance>();
    instance->DiscoverDefaultAdapters();

    // Get an adapter for the backend to use, and create the device.
    dawn::native::Adapter backendAdapter;
    {
        std::vector<dawn::native::Adapter> adapters = instance->GetAdapters();
        auto adapterIt = std::find_if(adapters.begin(), adapters.end(),
                                      [](const dawn::native::Adapter adapter) -> bool {
                                          wgpu::AdapterProperties properties;
                                          adapter.GetProperties(&properties);
                                          return properties.backendType == backendType;
                                      });
        ASSERT(adapterIt != adapters.end());
        backendAdapter = *adapterIt;
    }

    WGPUDevice backendDevice = backendAdapter.CreateDevice();
    DawnProcTable backendProcs = dawn::native::GetProcs();

    // Create the swapchain
    auto surfaceChainedDesc = wgpu::glfw::SetupWindowAndGetSurfaceDescriptor(window);
    WGPUSurfaceDescriptor surfaceDesc;
    surfaceDesc.nextInChain = reinterpret_cast<WGPUChainedStruct*>(surfaceChainedDesc.get());
    WGPUSurface surface = backendProcs.instanceCreateSurface(instance->Get(), &surfaceDesc);

    WGPUSwapChainDescriptor swapChainDesc;
    swapChainDesc.usage = WGPUTextureUsage_RenderAttachment;
    swapChainDesc.format = static_cast<WGPUTextureFormat>(GetPreferredSwapChainTextureFormat());
    swapChainDesc.width = 640;
    swapChainDesc.height = 480;
    swapChainDesc.presentMode = WGPUPresentMode_Mailbox;
    swapChainDesc.implementation = 0;
    WGPUSwapChain backendSwapChain =
        backendProcs.deviceCreateSwapChain(backendDevice, surface, &swapChainDesc);

    // Choose whether to use the backend procs and devices/swapchains directly, or set up the wire.
    WGPUDevice cDevice = nullptr;
    DawnProcTable procs;

    switch (cmdBufType) {
        case CmdBufType::None:
            procs = backendProcs;
            cDevice = backendDevice;
            swapChain = wgpu::SwapChain::Acquire(backendSwapChain);
            break;

        case CmdBufType::Terrible: {
            c2sBuf = new utils::TerribleCommandBuffer();
            s2cBuf = new utils::TerribleCommandBuffer();

            dawn::wire::WireServerDescriptor serverDesc = {};
            serverDesc.procs = &backendProcs;
            serverDesc.serializer = s2cBuf;

            wireServer = new dawn::wire::WireServer(serverDesc);
            c2sBuf->SetHandler(wireServer);

            dawn::wire::WireClientDescriptor clientDesc = {};
            clientDesc.serializer = c2sBuf;

            wireClient = new dawn::wire::WireClient(clientDesc);
            procs = dawn::wire::client::GetProcs();
            s2cBuf->SetHandler(wireClient);

            auto deviceReservation = wireClient->ReserveDevice();
            wireServer->InjectDevice(backendDevice, deviceReservation.id,
                                     deviceReservation.generation);
            cDevice = deviceReservation.device;

            auto swapChainReservation = wireClient->ReserveSwapChain(cDevice);
            wireServer->InjectSwapChain(backendSwapChain, swapChainReservation.id,
                                        swapChainReservation.generation, deviceReservation.id,
                                        deviceReservation.generation);
            swapChain = wgpu::SwapChain::Acquire(swapChainReservation.swapchain);
        } break;
    }

    dawnProcSetProcs(&procs);
    procs.deviceSetUncapturedErrorCallback(cDevice, PrintDeviceError, nullptr);
    return wgpu::Device::Acquire(cDevice);
}

wgpu::TextureFormat GetPreferredSwapChainTextureFormat() {
    // TODO(dawn:1362): Return the adapter's preferred format when implemented.
    return wgpu::TextureFormat::BGRA8Unorm;
}

wgpu::SwapChain GetSwapChain() {
    return swapChain;
}

wgpu::TextureView CreateDefaultDepthStencilView(const wgpu::Device& device) {
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = 640;
    descriptor.size.height = 480;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.sampleCount = 1;
    descriptor.format = wgpu::TextureFormat::Depth24PlusStencil8;
    descriptor.mipLevelCount = 1;
    descriptor.usage = wgpu::TextureUsage::RenderAttachment;
    auto depthStencilTexture = device.CreateTexture(&descriptor);
    return depthStencilTexture.CreateView();
}

bool InitSample(int argc, const char** argv) {
    for (int i = 1; i < argc; i++) {
        if (std::string("-b") == argv[i] || std::string("--backend") == argv[i]) {
            i++;
            if (i < argc && std::string("d3d12") == argv[i]) {
                backendType = wgpu::BackendType::D3D12;
                continue;
            }
            if (i < argc && std::string("metal") == argv[i]) {
                backendType = wgpu::BackendType::Metal;
                continue;
            }
            if (i < argc && std::string("null") == argv[i]) {
                backendType = wgpu::BackendType::Null;
                continue;
            }
            if (i < argc && std::string("opengl") == argv[i]) {
                backendType = wgpu::BackendType::OpenGL;
                continue;
            }
            if (i < argc && std::string("opengles") == argv[i]) {
                backendType = wgpu::BackendType::OpenGLES;
                continue;
            }
            if (i < argc && std::string("vulkan") == argv[i]) {
                backendType = wgpu::BackendType::Vulkan;
                continue;
            }
            fprintf(stderr,
                    "--backend expects a backend name (opengl, opengles, metal, d3d12, null, "
                    "vulkan)\n");
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
            printf("  BACKEND is one of: d3d12, metal, null, opengl, opengles, vulkan\n");
            printf("  COMMAND_BUFFER is one of: none, terrible\n");
            return false;
        }
    }

    // TODO(dawn:810): Reenable once the OpenGL(ES) backend is able to create its own context such
    // that it can use surface-based swapchains.
    if (backendType == wgpu::BackendType::OpenGL || backendType == wgpu::BackendType::OpenGLES) {
        fprintf(stderr,
                "The OpenGL(ES) backend is temporarily not supported for samples. See "
                "https://crbug.com/dawn/810");
        return false;
    }

    return true;
}

void DoFlush() {
    if (cmdBufType == CmdBufType::Terrible) {
        bool c2sSuccess = c2sBuf->Flush();
        bool s2cSuccess = s2cBuf->Flush();

        ASSERT(c2sSuccess && s2cSuccess);
    }
    glfwPollEvents();
}

bool ShouldQuit() {
    return glfwWindowShouldClose(window);
}

GLFWwindow* GetGLFWWindow() {
    return window;
}
