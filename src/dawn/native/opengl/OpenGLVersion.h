// Copyright 2020 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_OPENGL_OPENGLVERSION_H_
#define SRC_DAWN_NATIVE_OPENGL_OPENGLVERSION_H_

#include "dawn/native/opengl/OpenGLFunctionsBase_autogen.h"

namespace dawn::native::opengl {

struct OpenGLVersion {
  public:
    enum class Standard {
        Desktop,
        ES,
    };

    MaybeError Initialize(GetProcAddress getProc);

    bool IsDesktop() const;
    bool IsES() const;
    Standard GetStandard() const;
    uint32_t GetMajor() const;
    uint32_t GetMinor() const;
    bool IsAtLeast(uint32_t majorVersion, uint32_t minorVersion) const;

  private:
    uint32_t mMajorVersion;
    uint32_t mMinorVersion;
    Standard mStandard;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_OPENGLVERSION_H_
