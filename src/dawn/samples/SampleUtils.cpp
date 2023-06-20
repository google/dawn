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
#include <string_view>
#include <vector>

#include "GLFW/glfw3.h"
#include "dawn/common/Assert.h"
#include "dawn/common/Log.h"
#include "dawn/common/Platform.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/dawn_proc.h"
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

void DeviceLostCallback(WGPUDeviceLostReason reason, const char* message, void*) {
    dawn::ErrorLog() << "Device lost: " << message;
}

void PrintGLFWError(int code, const char* message) {
    dawn::ErrorLog() << "GLFW error: " << code << " - " << message;
}

void DeviceLogCallback(WGPULoggingType type, const char* message, void*) {
    dawn::ErrorLog() << "Device log: " << message;
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
#elif defined(DAWN_ENABLE_BACKEND_D3D11)
static wgpu::BackendType backendType = wgpu::BackendType::D3D11;
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

static std::vector<std::string> enableToggles;
static std::vector<std::string> disableToggles;

static CmdBufType cmdBufType = CmdBufType::Terrible;
static std::unique_ptr<dawn::native::Instance> instance;
static wgpu::SwapChain swapChain;

static GLFWwindow* window = nullptr;

static dawn::wire::WireServer* wireServer = nullptr;
static dawn::wire::WireClient* wireClient = nullptr;
static dawn::utils::TerribleCommandBuffer* c2sBuf = nullptr;
static dawn::utils::TerribleCommandBuffer* s2cBuf = nullptr;

static constexpr uint32_t kWidth = 640;
static constexpr uint32_t kHeight = 480;

wgpu::Device CreateCppDawnDevice() {
    dawn::ScopedEnvironmentVar angleDefaultPlatform;
    if (dawn::GetEnvironmentVar("ANGLE_DEFAULT_PLATFORM").first.empty()) {
        angleDefaultPlatform.Set("ANGLE_DEFAULT_PLATFORM", "swiftshader");
    }

    glfwSetErrorCallback(PrintGLFWError);
    if (!glfwInit()) {
        return wgpu::Device();
    }

    // Create the test window with no client API.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
    window = glfwCreateWindow(kWidth, kHeight, "Dawn window", nullptr, nullptr);
    if (!window) {
        return wgpu::Device();
    }

    instance = std::make_unique<dawn::native::Instance>();

    wgpu::RequestAdapterOptionsBackendType backendTypeOptions = {};
    backendTypeOptions.backendType = backendType;

    wgpu::RequestAdapterOptions options = {};
    options.nextInChain = &backendTypeOptions;

    // Get an adapter for the backend to use, and create the device.
    dawn::native::Adapter backendAdapter = instance->EnumerateAdapters(&options)[0];

    std::vector<const char*> enableToggleNames;
    std::vector<const char*> disabledToggleNames;
    for (const std::string& toggle : enableToggles) {
        enableToggleNames.push_back(toggle.c_str());
    }

    for (const std::string& toggle : disableToggles) {
        disabledToggleNames.push_back(toggle.c_str());
    }
    WGPUDawnTogglesDescriptor toggles;
    toggles.chain.sType = WGPUSType_DawnTogglesDescriptor;
    toggles.chain.next = nullptr;
    toggles.enabledToggles = enableToggleNames.data();
    toggles.enabledTogglesCount = static_cast<uint32_t>(enableToggleNames.size());
    toggles.disabledToggles = disabledToggleNames.data();
    toggles.disabledTogglesCount = static_cast<uint32_t>(disabledToggleNames.size());

    WGPUDeviceDescriptor deviceDesc = {};
    deviceDesc.nextInChain = reinterpret_cast<WGPUChainedStruct*>(&toggles);

    WGPUDevice backendDevice = backendAdapter.CreateDevice(&deviceDesc);
    DawnProcTable backendProcs = dawn::native::GetProcs();

    // Create the swapchain
    auto surfaceChainedDesc = wgpu::glfw::SetupWindowAndGetSurfaceDescriptor(window);
    WGPUSurfaceDescriptor surfaceDesc;
    surfaceDesc.nextInChain = reinterpret_cast<WGPUChainedStruct*>(surfaceChainedDesc.get());
    WGPUSurface surface = backendProcs.instanceCreateSurface(instance->Get(), &surfaceDesc);

    WGPUSwapChainDescriptor swapChainDesc = {};
    swapChainDesc.usage = WGPUTextureUsage_RenderAttachment;
    swapChainDesc.format = static_cast<WGPUTextureFormat>(GetPreferredSwapChainTextureFormat());
    swapChainDesc.width = kWidth;
    swapChainDesc.height = kHeight;
    swapChainDesc.presentMode = WGPUPresentMode_Mailbox;
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
            c2sBuf = new dawn::utils::TerribleCommandBuffer();
            s2cBuf = new dawn::utils::TerribleCommandBuffer();

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

            auto swapChainReservation = wireClient->ReserveSwapChain(cDevice, &swapChainDesc);
            wireServer->InjectSwapChain(backendSwapChain, swapChainReservation.id,
                                        swapChainReservation.generation, deviceReservation.id,
                                        deviceReservation.generation);
            swapChain = wgpu::SwapChain::Acquire(swapChainReservation.swapchain);
        } break;
    }

    dawnProcSetProcs(&procs);
    procs.deviceSetUncapturedErrorCallback(cDevice, PrintDeviceError, nullptr);
    procs.deviceSetDeviceLostCallback(cDevice, DeviceLostCallback, nullptr);
    procs.deviceSetLoggingCallback(cDevice, DeviceLogCallback, nullptr);
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
    descriptor.size.width = kWidth;
    descriptor.size.height = kHeight;
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
        std::string_view arg(argv[i]);
        std::string_view opt, value;

        static constexpr struct Option {
            const char* shortOpt;
            const char* longOpt;
            bool hasValue;
        } options[] = {
            {"-b", "--backend=", true},       {"-c", "--cmd-buf=", true},
            {"-e", "--enable-toggle=", true}, {"-d", "--disable-toggle=", true},
            {"-h", "--help", false},
        };

        for (const Option& option : options) {
            if (!option.hasValue) {
                if (arg == option.shortOpt || arg == option.longOpt) {
                    opt = option.shortOpt;
                    break;
                }
                continue;
            }

            if (arg == option.shortOpt) {
                opt = option.shortOpt;
                if (++i < argc) {
                    value = argv[i];
                }
                break;
            }

            if (arg.rfind(option.longOpt, 0) == 0) {
                opt = option.shortOpt;
                if (option.hasValue) {
                    value = arg.substr(strlen(option.longOpt));
                }
                break;
            }
        }

        if (opt == "-b") {
            if (value == "d3d11") {
                backendType = wgpu::BackendType::D3D11;
                continue;
            }
            if (value == "d3d12") {
                backendType = wgpu::BackendType::D3D12;
                continue;
            }
            if (value == "metal") {
                backendType = wgpu::BackendType::Metal;
                continue;
            }
            if (value == "null") {
                backendType = wgpu::BackendType::Null;
                continue;
            }
            if (value == "opengl") {
                backendType = wgpu::BackendType::OpenGL;
                continue;
            }
            if (value == "opengles") {
                backendType = wgpu::BackendType::OpenGLES;
                continue;
            }
            if (value == "vulkan") {
                backendType = wgpu::BackendType::Vulkan;
                continue;
            }
            fprintf(stderr,
                    "--backend expects a backend name (opengl, opengles, metal, d3d12, null, "
                    "vulkan)\n");
            return false;
        }

        if (opt == "-c") {
            if (value == "none") {
                cmdBufType = CmdBufType::None;
                continue;
            }
            if (value == "terrible") {
                cmdBufType = CmdBufType::Terrible;
                continue;
            }
            fprintf(stderr, "--command-buffer expects a command buffer name (none, terrible)\n");
            return false;
        }

        if (opt == "-e") {
            enableToggles.push_back(std::string(value));
            continue;
        }

        if (opt == "-d") {
            disableToggles.push_back(std::string(value));
            continue;
        }

        if (opt == "-h") {
            printf("Usage: %s [-b BACKEND] [-c COMMAND_BUFFER] [-e TOGGLE] [-d TOGGLE]\n", argv[0]);
            printf("  BACKEND is one of: d3d12, metal, null, opengl, opengles, vulkan\n");
            printf("  COMMAND_BUFFER is one of: none, terrible\n");
            printf("  TOGGLE is device toggle name to enable or disable\n");
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

void ProcessEvents() {
    dawn::native::InstanceProcessEvents(instance->Get());
}
