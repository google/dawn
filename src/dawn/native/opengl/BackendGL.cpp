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
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Instance.h"
#include "dawn/native/OpenGLBackend.h"
#include "dawn/native/opengl/ContextEGL.h"
#include "dawn/native/opengl/EGLFunctions.h"
#include "dawn/native/opengl/PhysicalDeviceGL.h"

namespace dawn::native::opengl {

// Implementation of the OpenGL backend's BackendConnection

Backend::Backend(InstanceBase* instance, wgpu::BackendType backendType)
    : BackendConnection(instance, backendType) {}

std::vector<Ref<PhysicalDeviceBase>> Backend::DiscoverPhysicalDevices(
    const RequestAdapterOptions* options) {
    if (options->forceFallbackAdapter) {
        return {};
    }
    if (!options->compatibilityMode) {
        // Return an empty vector since GL physical devices can only support compatibility mode.
        return std::vector<Ref<PhysicalDeviceBase>>{};
    }

    void* (*getProc)(const char* name) = nullptr;
    EGLDisplay display = EGL_NO_DISPLAY;

    const RequestAdapterOptionsGetGLProc* glGetProcOptions = nullptr;
    FindInChain(options->nextInChain, &glGetProcOptions);
    if (glGetProcOptions) {
        getProc = glGetProcOptions->getProc;
        display = glGetProcOptions->display;
    }

    if (getProc == nullptr) {
        // getProc not passed. Try to load it from libEGL.

#if DAWN_PLATFORM_IS(WINDOWS)
        const char* eglLib = "libEGL.dll";
#elif DAWN_PLATFORM_IS(MACOS)
        const char* eglLib = "libEGL.dylib";
#else
        const char* eglLib = "libEGL.so";
#endif
        if (!mLibEGL.Valid() && !mLibEGL.Open(eglLib)) {
            GetInstance()->ConsumedErrorAndWarnOnce(
                DAWN_VALIDATION_ERROR("Failed to load %s", eglLib));
            return {};
        }

        getProc = reinterpret_cast<void* (*)(const char*)>(mLibEGL.GetProc("eglGetProcAddress"));
        if (!getProc) {
            GetInstance()->ConsumedErrorAndWarnOnce(
                DAWN_VALIDATION_ERROR("eglGetProcAddress return nullptr"));
            return {};
        }
    }

    EGLFunctions egl;
    egl.Init(getProc);

    EGLenum api = GetType() == wgpu::BackendType::OpenGLES ? EGL_OPENGL_ES_API : EGL_OPENGL_API;

    if (display == EGL_NO_DISPLAY) {
        display = egl.GetCurrentDisplay();
    }

    if (display == EGL_NO_DISPLAY) {
        display = egl.GetDisplay(EGL_DEFAULT_DISPLAY);
    }

    std::unique_ptr<ContextEGL> context;
    if (GetInstance()->ConsumedErrorAndWarnOnce(ContextEGL::Create(egl, api, display), &context)) {
        return {};
    }

    EGLContext prevDrawSurface = egl.GetCurrentSurface(EGL_DRAW);
    EGLContext prevReadSurface = egl.GetCurrentSurface(EGL_READ);
    EGLContext prevContext = egl.GetCurrentContext();

    context->MakeCurrent();
    auto physicalDevices = DiscoverPhysicalDevicesWithProcs(getProc, display);
    egl.MakeCurrent(display, prevDrawSurface, prevReadSurface, prevContext);
    return physicalDevices;
}

std::vector<Ref<PhysicalDeviceBase>> Backend::DiscoverPhysicalDevicesWithProcs(
    void* (*getProc)(const char*),
    EGLDisplay display) {
    // TODO(cwallez@chromium.org): For now only create a single OpenGL physicalDevice because don't
    // know how to handle MakeCurrent.
    if (mPhysicalDevice != nullptr && mGetProc != getProc) {
        GetInstance()->ConsumedErrorAndWarnOnce(
            DAWN_VALIDATION_ERROR("The OpenGL backend can only create a single physicalDevice."));
        return {};
    }
    if (mPhysicalDevice == nullptr) {
        if (GetInstance()->ConsumedErrorAndWarnOnce(
                PhysicalDevice::Create(GetInstance(), GetType(), getProc, display),
                &mPhysicalDevice)) {
            return {};
        }
        mGetProc = getProc;
    }
    return {mPhysicalDevice};
}

void Backend::ClearPhysicalDevices() {
    mPhysicalDevice = nullptr;
}

size_t Backend::GetPhysicalDeviceCountForTesting() const {
    return mPhysicalDevice != nullptr ? 1 : 0;
}

BackendConnection* Connect(InstanceBase* instance, wgpu::BackendType backendType) {
    return new Backend(instance, backendType);
}

}  // namespace dawn::native::opengl
