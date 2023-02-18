// Copyright 2020 The Dawn Authors
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

#include <cstdlib>
#include <memory>
#include <utility>

#include "GLFW/glfw3.h"
#include "dawn/common/Platform.h"
#include "webgpu/webgpu_glfw.h"

#if DAWN_PLATFORM_IS(WINDOWS)
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#if defined(DAWN_USE_X11)
#define GLFW_EXPOSE_NATIVE_X11
#endif
#if defined(DAWN_USE_WAYLAND)
#define GLFW_EXPOSE_NATIVE_WAYLAND
#endif
#include "GLFW/glfw3native.h"

namespace wgpu::glfw {

wgpu::Surface CreateSurfaceForWindow(const wgpu::Instance& instance, GLFWwindow* window) {
    std::unique_ptr<wgpu::ChainedStruct> chainedDescriptor =
        SetupWindowAndGetSurfaceDescriptor(window);

    wgpu::SurfaceDescriptor descriptor;
    descriptor.nextInChain = chainedDescriptor.get();
    wgpu::Surface surface = instance.CreateSurface(&descriptor);

    return surface;
}

// SetupWindowAndGetSurfaceDescriptorCocoa defined in GLFWUtils_metal.mm
std::unique_ptr<wgpu::ChainedStruct> SetupWindowAndGetSurfaceDescriptorCocoa(GLFWwindow* window);

std::unique_ptr<wgpu::ChainedStruct> SetupWindowAndGetSurfaceDescriptor(GLFWwindow* window) {
#if DAWN_PLATFORM_IS(WINDOWS)
    std::unique_ptr<wgpu::SurfaceDescriptorFromWindowsHWND> desc =
        std::make_unique<wgpu::SurfaceDescriptorFromWindowsHWND>();
    desc->hwnd = glfwGetWin32Window(window);
    desc->hinstance = GetModuleHandle(nullptr);
    return std::move(desc);
#elif defined(DAWN_ENABLE_BACKEND_METAL)
    return SetupWindowAndGetSurfaceDescriptorCocoa(window);
#elif defined(DAWN_USE_WAYLAND) || defined(DAWN_USE_X11)
#if defined(GLFW_PLATFORM_WAYLAND) && defined(DAWN_USE_WAYLAND)
    if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND) {
        std::unique_ptr<wgpu::SurfaceDescriptorFromWaylandSurface> desc =
            std::make_unique<wgpu::SurfaceDescriptorFromWaylandSurface>();
        desc->display = glfwGetWaylandDisplay();
        desc->surface = glfwGetWaylandWindow(window);
        return std::move(desc);
    } else  // NOLINT(readability/braces)
#endif
#if defined(DAWN_USE_X11)
    {
        std::unique_ptr<wgpu::SurfaceDescriptorFromXlibWindow> desc =
            std::make_unique<wgpu::SurfaceDescriptorFromXlibWindow>();
        desc->display = glfwGetX11Display();
        desc->window = glfwGetX11Window(window);
        return std::move(desc);
    }
#else
    {
        return nullptr;
    }
#endif
#else
    return nullptr;
#endif
}

}  // namespace wgpu::glfw
