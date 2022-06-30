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
#include "dawn/native/opengl/AdapterGL.h"
#include "dawn/native/opengl/ContextEGL.h"
#include "dawn/native/opengl/EGLFunctions.h"

namespace dawn::native::opengl {

// Implementation of the OpenGL backend's BackendConnection

Backend::Backend(InstanceBase* instance, wgpu::BackendType backendType)
    : BackendConnection(instance, backendType) {}

std::vector<Ref<AdapterBase>> Backend::DiscoverDefaultAdapters() {
    std::vector<Ref<AdapterBase>> adapters;
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

    AdapterDiscoveryOptions options(ToAPI(GetType()));
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

    auto result = DiscoverAdapters(&options);

    if (result.IsError()) {
        GetInstance()->ConsumedError(result.AcquireError());
    } else {
        auto value = result.AcquireSuccess();
        adapters.insert(adapters.end(), value.begin(), value.end());
    }

    egl.MakeCurrent(prevDisplay, prevDrawSurface, prevReadSurface, prevContext);

    return adapters;
}

ResultOrError<std::vector<Ref<AdapterBase>>> Backend::DiscoverAdapters(
    const AdapterDiscoveryOptionsBase* optionsBase) {
    // TODO(cwallez@chromium.org): For now only create a single OpenGL adapter because don't
    // know how to handle MakeCurrent.
    DAWN_INVALID_IF(mCreatedAdapter, "The OpenGL backend can only create a single adapter.");

    ASSERT(static_cast<wgpu::BackendType>(optionsBase->backendType) == GetType());
    const AdapterDiscoveryOptions* options =
        static_cast<const AdapterDiscoveryOptions*>(optionsBase);

    DAWN_INVALID_IF(options->getProc == nullptr, "AdapterDiscoveryOptions::getProc must be set");

    Ref<Adapter> adapter = AcquireRef(
        new Adapter(GetInstance(), static_cast<wgpu::BackendType>(optionsBase->backendType)));
    DAWN_TRY(adapter->InitializeGLFunctions(options->getProc));
    DAWN_TRY(adapter->Initialize());

    mCreatedAdapter = true;
    std::vector<Ref<AdapterBase>> adapters{std::move(adapter)};
    return std::move(adapters);
}

BackendConnection* Connect(InstanceBase* instance, wgpu::BackendType backendType) {
    return new Backend(instance, backendType);
}

}  // namespace dawn::native::opengl
