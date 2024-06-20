// Copyright 2022 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_OPENGL_CONTEXTEGL_H_
#define SRC_DAWN_NATIVE_OPENGL_CONTEXTEGL_H_

#include <memory>

#include "dawn/common/NonMovable.h"
#include "dawn/common/egl_platform.h"
#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/native/opengl/EGLFunctions.h"

namespace dawn::native::opengl {

class ContextEGL : NonMovable {
  public:
    static ResultOrError<std::unique_ptr<ContextEGL>> Create(const EGLFunctions& functions,
                                                             EGLenum api,
                                                             EGLDisplay display,
                                                             bool useANGLETextureSharing);
    ~ContextEGL();

    void MakeCurrent();
    EGLDisplay GetEGLDisplay() const;
    const EGLFunctions& GetEGL() const;

  private:
    ContextEGL(const EGLFunctions& functions, EGLDisplay display, EGLContext context);

    const EGLFunctions mEgl;
    EGLDisplay mDisplay;
    EGLContext mContext;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_CONTEXTEGL_H_
