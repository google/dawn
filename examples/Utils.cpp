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
#include <nxt/nxtcpp.h>
#include <shaderc/shaderc.hpp>
#include "GLFW/glfw3.h"

#include "BackendBinding.h"
#include "../src/wire/TerribleCommandBuffer.h"

#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
    #include <Windows.h>
#else
    #include <unistd.h>
#endif

BackendBinding* CreateMetalBinding();
BackendBinding* CreateD3D12Binding();

namespace backend {
    namespace opengl {
        void Init(void* (*getProc)(const char*), nxtProcTable* procs, nxtDevice* device);
        void HACKCLEAR();
    }
}

class OpenGLBinding : public BackendBinding {
    public:
        void SetupGLFWWindowHints() override {
            #ifdef __APPLE__
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
        }
        void SwapBuffers() override {
            glfwSwapBuffers(window);
            backend::opengl::HACKCLEAR();
        }
};

namespace backend {
    namespace null {
        void Init(nxtProcTable* procs, nxtDevice* device);
    }
}

class NullBinding : public BackendBinding {
    public:
        void SetupGLFWWindowHints() override {
        }
        void GetProcAndDevice(nxtProcTable* procs, nxtDevice* device) override {
            backend::null::Init(procs, device);
        }
        void SwapBuffers() override {
        }
};

void PrintDeviceError(const char* message, nxt::CallbackUserdata) {
    std::cout << "Device error: " << message << std::endl;
}

enum class BackendType {
    OpenGL,
    Metal,
    D3D12,
    Null,
};

enum class CmdBufType {
    None,
    Terrible,
    //TODO(cwallez@chromium.org) double terrible cmdbuf
};

#if defined(__APPLE__)
static BackendType backendType = BackendType::Metal;
#elif defined(_WIN32)
static BackendType backendType = BackendType::D3D12;
#else
static BackendType backendType = BackendType::OpenGL;
#endif

static CmdBufType cmdBufType = CmdBufType::Terrible;
static BackendBinding* binding = nullptr;

static GLFWwindow* window = nullptr;

static nxt::wire::CommandHandler* wireServer = nullptr;
static nxt::wire::CommandHandler* wireClient = nullptr;
static nxt::wire::TerribleCommandBuffer* c2sBuf = nullptr;
static nxt::wire::TerribleCommandBuffer* s2cBuf = nullptr;

nxt::Device CreateCppNXTDevice() {
    switch (backendType) {
        case BackendType::OpenGL:
            binding = new OpenGLBinding;
            break;
        case BackendType::Metal:
            #if defined(__APPLE__)
                binding = CreateMetalBinding();
            #else
                fprintf(stderr, "Metal backend not present on this platform\n");
            #endif
            break;
        case BackendType::D3D12:
            #if defined(_WIN32)
                binding = CreateD3D12Binding();
            #else
                fprintf(stderr, "D3D12 backend not present on this platform\n");
            #endif
            break;
        case BackendType::Null:
            binding = new NullBinding;
            break;
    }

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

nxt::ShaderModule CreateShaderModule(const nxt::Device& device, nxt::ShaderStage stage, const char* source) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    shaderc_shader_kind kind;
    switch (stage) {
        case nxt::ShaderStage::Vertex:
            kind = shaderc_glsl_vertex_shader;
            break;
        case nxt::ShaderStage::Fragment:
            kind = shaderc_glsl_fragment_shader;
            break;
        case nxt::ShaderStage::Compute:
            kind = shaderc_glsl_compute_shader;
            break;
    }

    auto result = compiler.CompileGlslToSpv(source, strlen(source), kind, "myshader?", options);
    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cerr << result.GetErrorMessage();
        return {};
    }

    size_t size = (result.cend() - result.cbegin());

#ifdef DUMP_SPIRV_ASSEMBLY
    {
        auto resultAsm = compiler.CompileGlslToSpvAssembly(source, strlen(source), kind, "myshader?", options);
        size_t sizeAsm = (resultAsm.cend() - resultAsm.cbegin());

        char* buffer = reinterpret_cast<char*>(malloc(sizeAsm + 1));
        memcpy(buffer, resultAsm.cbegin(), sizeAsm);
        buffer[sizeAsm] = '\0';
        printf("SPIRV ASSEMBLY DUMP START\n%s\nSPIRV ASSEMBLY DUMP END\n", buffer);
        free(buffer);
    }
#endif

#ifdef DUMP_SPIRV_JS_ARRAY
    printf("SPIRV JS ARRAY DUMP START\n");
    for (size_t i = 0; i < size; i++) {
        printf("%#010x", result.cbegin()[i]);
        if ((i + 1) % 4 == 0) {
            printf(",\n");
        } else {
            printf(", ");
        }
    }
    printf("\n");
    printf("SPIRV JS ARRAY DUMP END\n");
#endif

    return device.CreateShaderModuleBuilder()
        .SetSource(size, result.cbegin())
        .GetResult();
}

void CreateDefaultRenderPass(const nxt::Device& device, nxt::RenderPass* renderPass, nxt::Framebuffer* framebuffer) {
    *renderPass = device.CreateRenderPassBuilder()
        .SetAttachmentCount(1)
        .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
        .SetSubpassCount(1)
        .SubpassSetColorAttachment(0, 0, 0)
        .GetResult();
    *framebuffer = device.CreateFramebufferBuilder()
        .SetRenderPass(*renderPass)
        .SetDimensions(640, 480)
        .GetResult();
}

nxt::Buffer CreateFrozenBufferFromData(const nxt::Device& device, const void* data, uint32_t size, nxt::BufferUsageBit usage) {
    nxt::Buffer buffer = device.CreateBufferBuilder()
        .SetAllowedUsage(nxt::BufferUsageBit::TransferDst | usage)
        .SetInitialUsage(nxt::BufferUsageBit::TransferDst)
        .SetSize(size)
        .GetResult();
    buffer.SetSubData(0, size / sizeof(uint32_t), reinterpret_cast<const uint32_t*>(data));
    buffer.FreezeUsage(usage);
    return buffer;
}

extern "C" {
    bool InitUtils(int argc, const char** argv) {
        for (int i = 0; i < argc; i++) {
            if (std::string("-b") == argv[i] || std::string("--backend") == argv[i]) {
                i++;
                if (i < argc && std::string("opengl") == argv[i]) {
                    backendType = BackendType::OpenGL;
                    continue;
                }
                if (i < argc && std::string("metal") == argv[i]) {
                    backendType = BackendType::Metal;
                    continue;
                }
                if (i < argc && std::string("d3d12") == argv[i]) {
                    backendType = BackendType::D3D12;
                    continue;
                }
                if (i < argc && std::string("null") == argv[i]) {
                    backendType = BackendType::Null;
                    continue;
                }
                fprintf(stderr, "--backend expects a backend name (opengl, metal, d3d12, null)\n");
                return false;
            }
            if (std::string("-c") == argv[i] || std::string("--comand-buffer") == argv[i]) {
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
                printf("  BACKEND is one of: opengl, metal, d3d12, null\n");
                printf("  COMMAND_BUFFER is one of: none, terrible\n");
                return false;
            }
        }
        return true;
    }

    nxtDevice CreateNXTDevice() {
        return CreateCppNXTDevice().Release();
    }

    nxtShaderModule CreateShaderModule(nxtDevice device, nxtShaderStage stage, const char* source) {
        return CreateShaderModule(device, static_cast<nxt::ShaderStage>(stage), source).Release();
    }

    void DoSwapBuffers() {
        if (cmdBufType == CmdBufType::Terrible) {
            c2sBuf->Flush();
            s2cBuf->Flush();
        }
        glfwPollEvents();
        binding->SwapBuffers();
    }

#ifdef _WIN32
    void USleep(uint64_t usecs) {
        Sleep(usecs / 1000);
    }
#else
    void USleep(uint64_t usecs) {
        usleep(usecs);
    }
#endif

    bool ShouldQuit() {
        return glfwWindowShouldClose(window);
    }

    GLFWwindow* GetGLFWWindow() {
        return window;
    }
}
