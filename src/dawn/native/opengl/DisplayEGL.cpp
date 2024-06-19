// Copyright 2024 The Dawn & Tint Authors
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

#include "dawn/native/opengl/DisplayEGL.h"

#include <string>
#include <utility>

namespace dawn::native::opengl {

// static
ResultOrError<std::unique_ptr<DisplayEGL>> DisplayEGL::CreateFromDynamicLoading(
    wgpu::BackendType backend,
    const char* libName) {
    auto display = std::make_unique<DisplayEGL>(backend);
    DAWN_TRY(display->InitializeWithDynamicLoading(libName));
    return std::move(display);
}

// static
ResultOrError<std::unique_ptr<DisplayEGL>> DisplayEGL::CreateFromProcAndDisplay(
    wgpu::BackendType backend,
    EGLGetProcProc getProc,
    EGLDisplay eglDisplay) {
    auto display = std::make_unique<DisplayEGL>(backend);
    DAWN_TRY(display->InitializeWithProcAndDisplay(getProc, eglDisplay));
    return std::move(display);
}

DisplayEGL::DisplayEGL(wgpu::BackendType backend) : egl(mFunctions) {
    switch (backend) {
        case wgpu::BackendType::OpenGL:
            mApiEnum = EGL_OPENGL_API;
            mApiBit = EGL_OPENGL_BIT;
            break;
        case wgpu::BackendType::OpenGLES:
            mApiEnum = EGL_OPENGL_ES_API;
            mApiBit = EGL_OPENGL_ES3_BIT;
            break;
        default:
            DAWN_UNREACHABLE();
    }
}

DisplayEGL::~DisplayEGL() {
    if (mDisplay != EGL_NO_DISPLAY) {
        if (mOwnsDisplay) {
            egl.Terminate(mDisplay);
        }
        mDisplay = EGL_NO_DISPLAY;
    }
}

MaybeError DisplayEGL::InitializeWithDynamicLoading(const char* libName) {
    std::string err;
    if (!mLib.Valid() && !mLib.Open(libName, &err)) {
        return DAWN_VALIDATION_ERROR("Failed to load %s: \"%s\".", libName, err.c_str());
    }

    EGLGetProcProc getProc = reinterpret_cast<EGLGetProcProc>(mLib.GetProc("eglGetProcAddress"));
    if (!getProc) {
        return DAWN_VALIDATION_ERROR("Couldn't get \"eglGetProcAddress\" from %s.", libName);
    }

    return InitializeWithProcAndDisplay(getProc, EGL_NO_DISPLAY);
}

MaybeError DisplayEGL::InitializeWithProcAndDisplay(EGLGetProcProc getProc, EGLDisplay display) {
    // Load the EGL functions.
    DAWN_TRY(mFunctions.LoadClientProcs(getProc));

    mDisplay = display;
    if (mDisplay == EGL_NO_DISPLAY) {
        mOwnsDisplay = true;
        mDisplay = egl.GetDisplay(EGL_DEFAULT_DISPLAY);
    }
    if (mDisplay == EGL_NO_DISPLAY) {
        return DAWN_VALIDATION_ERROR("Couldn't create the default EGL display.");
    }

    DAWN_TRY(mFunctions.LoadDisplayProcs(mDisplay));

    // We require at least EGL 1.4.
    DAWN_INVALID_IF(
        egl.GetMajorVersion() < 1 || (egl.GetMajorVersion() == 1 && egl.GetMinorVersion() < 4),
        "EGL version (%u.%u) must be at least 1.4", egl.GetMajorVersion(), egl.GetMinorVersion());

    return {};
}

EGLDisplay DisplayEGL::GetDisplay() const {
    return mDisplay;
}

EGLint DisplayEGL::GetAPIEnum() const {
    return mApiEnum;
}

EGLint DisplayEGL::GetAPIBit() const {
    return mApiBit;
}

}  // namespace dawn::native::opengl
