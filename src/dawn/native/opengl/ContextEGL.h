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

#ifndef SRC_DAWN_NATIVE_OPENGL_CONTEXTEGL_H_
#define SRC_DAWN_NATIVE_OPENGL_CONTEXTEGL_H_

#include <EGL/egl.h>

#include <memory>

#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/native/opengl/EGLFunctions.h"

namespace dawn::native::opengl {

class ContextEGL : public Device::Context {
  public:
    static ResultOrError<std::unique_ptr<ContextEGL>> Create(const EGLFunctions& functions,
                                                             EGLenum api,
                                                             EGLDisplay display,
                                                             bool useANGLETextureSharing);
    void MakeCurrent() override;
    ~ContextEGL() override;

  private:
    ContextEGL(const EGLFunctions& functions, EGLDisplay display, EGLContext context)
        : egl(functions), mDisplay(display), mContext(context) {}

    const EGLFunctions egl;
    EGLDisplay mDisplay;
    EGLContext mContext;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_CONTEXTEGL_H_
