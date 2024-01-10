// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
            DAWN_UNREACHABLE();
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

static wgpu::AdapterType adapterType = wgpu::AdapterType::Unknown;

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

    WGPUInstanceDescriptor instanceDescriptor{};
    instanceDescriptor.features.timedWaitAnyEnable = true;
    instance = std::make_unique<dawn::native::Instance>(&instanceDescriptor);

    wgpu::RequestAdapterOptions options = {};
    options.backendType = backendType;

    // Get an adapter for the backend to use, and create the device.
    auto adapters = instance->EnumerateAdapters(&options);
    wgpu::DawnAdapterPropertiesPowerPreference power_props{};
    wgpu::AdapterProperties adapterProperties{};
    adapterProperties.nextInChain = &power_props;
    // Find the first adapter which satisfies the adapterType requirement.
    auto isAdapterType = [&adapterProperties](const auto& adapter) -> bool {
        // picks the first adapter when adapterType is unknown.
        if (adapterType == wgpu::AdapterType::Unknown) {
            return true;
        }
        adapter.GetProperties(&adapterProperties);
        return adapterProperties.adapterType == adapterType;
    };
    auto preferredAdapter = std::find_if(adapters.begin(), adapters.end(), isAdapterType);
    if (preferredAdapter == adapters.end()) {
        fprintf(stderr, "Failed to find an adapter! Please try another adapter type.\n");
        return wgpu::Device();
    }

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
    toggles.enabledToggleCount = enableToggleNames.size();
    toggles.disabledToggles = disabledToggleNames.data();
    toggles.disabledToggleCount = disabledToggleNames.size();

    WGPUDeviceDescriptor deviceDesc = {};
    deviceDesc.nextInChain = reinterpret_cast<WGPUChainedStruct*>(&toggles);

    WGPUDevice backendDevice = preferredAdapter->CreateDevice(&deviceDesc);
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
            {"-a", "--adapter-type=", true},  {"-h", "--help", false},
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

        if (opt == "-a") {
            if (value == "discrete") {
                adapterType = wgpu::AdapterType::DiscreteGPU;
                continue;
            }
            if (value == "integrated") {
                adapterType = wgpu::AdapterType::IntegratedGPU;
                continue;
            }
            if (value == "cpu") {
                adapterType = wgpu::AdapterType::CPU;
                continue;
            }
            fprintf(stderr, "--adapter-type expects an adapter type (discrete, integrated, cpu)\n");
            return false;
        }

        if (opt == "-h") {
            printf(
                "Usage: %s [-b BACKEND] [-c COMMAND_BUFFER] [-e TOGGLE] [-d TOGGLE] [-a "
                "ADAPTER]\n",
                argv[0]);
            printf("  BACKEND is one of: d3d12, metal, null, opengl, opengles, vulkan\n");
            printf("  COMMAND_BUFFER is one of: none, terrible\n");
            printf("  TOGGLE is device toggle name to enable or disable\n");
            printf("  ADAPTER is one of: discrete, integrated, cpu\n");
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

        DAWN_ASSERT(c2sSuccess && s2cSuccess);
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
