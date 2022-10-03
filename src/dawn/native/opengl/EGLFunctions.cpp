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

#include "dawn/native/opengl/EGLFunctions.h"

namespace dawn::native::opengl {

void EGLFunctions::Init(void* (*getProc)(const char*)) {
    GetProcAddress = reinterpret_cast<PFNEGLGETPROCADDRESSPROC>(getProc);
    BindAPI = reinterpret_cast<PFNEGLBINDAPIPROC>(GetProcAddress("eglBindAPI"));
    ChooseConfig = reinterpret_cast<PFNEGLCHOOSECONFIGPROC>(GetProcAddress("eglChooseConfig"));
    CreateContext = reinterpret_cast<PFNEGLCREATECONTEXTPROC>(GetProcAddress("eglCreateContext"));
    CreatePbufferSurface =
        reinterpret_cast<PFNEGLCREATEPBUFFERSURFACEPROC>(GetProcAddress("eglCreatePbufferSurface"));
    CreatePlatformWindowSurface = reinterpret_cast<PFNEGLCREATEPLATFORMWINDOWSURFACEPROC>(
        GetProcAddress("eglCreatePlatformWindowSurface"));
    DestroyContext =
        reinterpret_cast<PFNEGLDESTROYCONTEXTPROC>(GetProcAddress("eglDestroyContext"));
    GetConfigs = reinterpret_cast<PFNEGLGETCONFIGSPROC>(GetProcAddress("eglGetConfigs"));
    GetCurrentContext =
        reinterpret_cast<PFNEGLGETCURRENTCONTEXTPROC>(GetProcAddress("eglGetCurrentContext"));
    GetCurrentDisplay =
        reinterpret_cast<PFNEGLGETCURRENTDISPLAYPROC>(GetProcAddress("eglGetCurrentDisplay"));
    GetCurrentSurface =
        reinterpret_cast<PFNEGLGETCURRENTSURFACEPROC>(GetProcAddress("eglGetCurrentSurface"));
    GetDisplay = reinterpret_cast<PFNEGLGETDISPLAYPROC>(GetProcAddress("eglGetDisplay"));
    GetError = reinterpret_cast<PFNEGLGETERRORPROC>(GetProcAddress("eglGetError"));
    Initialize = reinterpret_cast<PFNEGLINITIALIZEPROC>(GetProcAddress("eglInitialize"));
    MakeCurrent = reinterpret_cast<PFNEGLMAKECURRENTPROC>(GetProcAddress("eglMakeCurrent"));
    QueryString = reinterpret_cast<PFNEGLQUERYSTRINGPROC>(GetProcAddress("eglQueryString"));
}

}  // namespace dawn::native::opengl
