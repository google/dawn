// Copyright 2022 The Dawn Authors
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

#include "dawn/native/opengl/ContextEGL.h"

#include <memory>
#include <vector>

#include "dawn/native/opengl/UtilsEGL.h"

namespace dawn::native::opengl {

ResultOrError<std::unique_ptr<ContextEGL>> ContextEGL::Create(const EGLFunctions& egl,
                                                              EGLenum api,
                                                              EGLDisplay display) {
    EGLint renderableType = api == EGL_OPENGL_ES_API ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_BIT;

    EGLint major, minor;

    DAWN_TRY(CheckEGL(egl, egl.Initialize(display, &major, &minor), "eglInitialize"));

    // We use EGLImage unconditionally, which only became core in 1.5.
    DAWN_INVALID_IF(major < 1 || (major == 1 && minor < 5),
                    "EGL version (%u.%u) must be at least 1.5", major, minor);

    // Since we're creating a surfaceless context, the only thing we really care
    // about is the RENDERABLE_TYPE.
    EGLint config_attribs[] = {EGL_RENDERABLE_TYPE, renderableType, EGL_NONE};

    EGLint num_config;
    EGLConfig config;
    DAWN_TRY(CheckEGL(egl, egl.ChooseConfig(display, config_attribs, &config, 1, &num_config),
                      "eglChooseConfig"));

    DAWN_INVALID_IF(num_config == 0, "eglChooseConfig returned zero configs");

    DAWN_TRY(CheckEGL(egl, egl.BindAPI(api), "eglBindAPI"));

    if (api == EGL_OPENGL_ES_API) {
        major = 3;
        minor = 1;
    } else {
        major = 4;
        minor = 4;
    }

    const char* extensions = egl.QueryString(display, EGL_EXTENSIONS);
    if (strstr(extensions, "EGL_EXT_create_context_robustness") == nullptr) {
        return DAWN_INTERNAL_ERROR("EGL_EXT_create_context_robustness must be supported");
    }

    EGLint attrib_list[] = {
        EGL_CONTEXT_MAJOR_VERSION,
        major,
        EGL_CONTEXT_MINOR_VERSION,
        minor,
        EGL_CONTEXT_OPENGL_ROBUST_ACCESS,  // Core in EGL 1.5
        EGL_TRUE,
        EGL_NONE,
    };

    EGLContext context = egl.CreateContext(display, config, EGL_NO_CONTEXT, attrib_list);
    DAWN_TRY(CheckEGL(egl, context != EGL_NO_CONTEXT, "eglCreateContext"));

    return std::unique_ptr<ContextEGL>(new ContextEGL(egl, display, context));
}

void ContextEGL::MakeCurrent() {
    egl.MakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, mContext);
}

ContextEGL::~ContextEGL() {
    egl.DestroyContext(mDisplay, mContext);
}

}  // namespace dawn::native::opengl
