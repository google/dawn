// Copyright 2019 The Dawn Authors
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

#include "dawn/native/opengl/BackendGL.h"

#include <EGL/egl.h>

#include <memory>
#include <utility>

#include "dawn/common/SystemUtils.h"
#include "dawn/native/Instance.h"
#include "dawn/native/OpenGLBackend.h"
#include "dawn/native/opengl/ContextEGL.h"
#include "dawn/native/opengl/EGLFunctions.h"
#include "dawn/native/opengl/PhysicalDeviceGL.h"

namespace dawn::native::opengl {

// Implementation of the OpenGL backend's BackendConnection

Backend::Backend(InstanceBase* instance, wgpu::BackendType backendType)
    : BackendConnection(instance, backendType) {}

std::vector<Ref<PhysicalDeviceBase>> Backend::DiscoverDefaultPhysicalDevices() {
    std::vector<Ref<PhysicalDeviceBase>> physicalDevices;
#if DAWN_PLATFORM_IS(WINDOWS)
    const char* eglLib = "libEGL.dll";
#elif DAWN_PLATFORM_IS(MACOS)
    const char* eglLib = "libEGL.dylib";
#else
    const char* eglLib = "libEGL.so";
#endif
    if (!mLibEGL.Valid() && !mLibEGL.Open(eglLib)) {
        return {};
    }

    PhysicalDeviceDiscoveryOptions options(ToAPI(GetType()));
    options.getProc =
        reinterpret_cast<void* (*)(const char*)>(mLibEGL.GetProc("eglGetProcAddress"));
    if (!options.getProc) {
        return {};
    }

    EGLFunctions egl;
    egl.Init(options.getProc);

    EGLenum api = GetType() == wgpu::BackendType::OpenGLES ? EGL_OPENGL_ES_API : EGL_OPENGL_API;
    std::unique_ptr<ContextEGL> context;
    if (GetInstance()->ConsumedError(ContextEGL::Create(egl, api), &context)) {
        return {};
    }

    EGLDisplay prevDisplay = egl.GetCurrentDisplay();
    EGLContext prevDrawSurface = egl.GetCurrentSurface(EGL_DRAW);
    EGLContext prevReadSurface = egl.GetCurrentSurface(EGL_READ);
    EGLContext prevContext = egl.GetCurrentContext();

    context->MakeCurrent();

    auto result = DiscoverPhysicalDevices(&options);

    if (result.IsError()) {
        GetInstance()->ConsumedError(result.AcquireError());
    } else {
        auto value = result.AcquireSuccess();
        physicalDevices.insert(physicalDevices.end(), value.begin(), value.end());
    }

    egl.MakeCurrent(prevDisplay, prevDrawSurface, prevReadSurface, prevContext);

    return physicalDevices;
}

ResultOrError<std::vector<Ref<PhysicalDeviceBase>>> Backend::DiscoverPhysicalDevices(
    const PhysicalDeviceDiscoveryOptionsBase* optionsBase) {
    // TODO(cwallez@chromium.org): For now only create a single OpenGL physicalDevice because don't
    // know how to handle MakeCurrent.
    DAWN_INVALID_IF(mCreatedPhysicalDevice,
                    "The OpenGL backend can only create a single physicalDevice.");

    ASSERT(static_cast<wgpu::BackendType>(optionsBase->backendType) == GetType());
    const PhysicalDeviceDiscoveryOptions* options =
        static_cast<const PhysicalDeviceDiscoveryOptions*>(optionsBase);

    DAWN_INVALID_IF(options->getProc == nullptr,
                    "PhysicalDeviceDiscoveryOptions::getProc must be set");

    Ref<PhysicalDevice> physicalDevice = AcquireRef(new PhysicalDevice(
        GetInstance(), static_cast<wgpu::BackendType>(optionsBase->backendType)));
    DAWN_TRY(physicalDevice->InitializeGLFunctions(options->getProc));
    DAWN_TRY(physicalDevice->Initialize());

    mCreatedPhysicalDevice = true;
    std::vector<Ref<PhysicalDeviceBase>> adapters{std::move(physicalDevice)};
    return std::move(adapters);
}

BackendConnection* Connect(InstanceBase* instance, wgpu::BackendType backendType) {
    return new Backend(instance, backendType);
}

}  // namespace dawn::native::opengl
