// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/native/opengl/BackendGL.h"

#include "dawn/native/ChainUtils.h"
#include "dawn/native/Instance.h"
#include "dawn/native/OpenGLBackend.h"
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

    return DiscoverPhysicalDevicesWithProcs(getProc, display);
}

std::vector<Ref<PhysicalDeviceBase>> Backend::DiscoverPhysicalDevicesWithProcs(
    void* (*getProc)(const char*),
    EGLDisplay display) {
    // TODO(cwallez@chromium.org): For now only create a single OpenGL physicalDevice because don't
    // know how to handle MakeCurrent.
    if (mPhysicalDevice != nullptr && (mGetProc != getProc || mDisplay != display)) {
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
        mDisplay = display;
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
