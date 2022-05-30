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

#ifndef SRC_DAWN_NATIVE_OPENGL_GLFORMAT_H_
#define SRC_DAWN_NATIVE_OPENGL_GLFORMAT_H_

#include "dawn/native/Format.h"
#include "dawn/native/opengl/opengl_platform.h"

namespace dawn::native::opengl {

struct GLFormat {
    GLenum internalFormat = 0;
    GLenum format = 0;
    GLenum type = 0;
    bool isSupportedOnBackend = false;

    // OpenGL has different functions depending on the format component type, for example
    // glClearBufferfv is only valid on formats with the Float ComponentType
    enum ComponentType { Float, Int, Uint, DepthStencil };
    ComponentType componentType;
};

using GLFormatTable = ityp::array<FormatIndex, GLFormat, kKnownFormatCount>;
GLFormatTable BuildGLFormatTable(GLenum internalFormatForBGRA);

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_GLFORMAT_H_
