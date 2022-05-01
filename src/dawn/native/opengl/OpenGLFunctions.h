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

#ifndef SRC_DAWN_NATIVE_OPENGL_OPENGLFUNCTIONS_H_
#define SRC_DAWN_NATIVE_OPENGL_OPENGLFUNCTIONS_H_

#include <string>
#include <unordered_set>

#include "dawn/native/opengl/OpenGLFunctionsBase_autogen.h"
#include "dawn/native/opengl/OpenGLVersion.h"

namespace dawn::native::opengl {

struct OpenGLFunctions : OpenGLFunctionsBase {
  public:
    MaybeError Initialize(GetProcAddress getProc);

    const OpenGLVersion& GetVersion() const;
    bool IsAtLeastGL(uint32_t majorVersion, uint32_t minorVersion) const;
    bool IsAtLeastGLES(uint32_t majorVersion, uint32_t minorVersion) const;

    bool IsGLExtensionSupported(const char* extension) const;

  private:
    void InitializeSupportedGLExtensions();

    OpenGLVersion mVersion;

    std::unordered_set<std::string> mSupportedGLExtensionsSet;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_OPENGLFUNCTIONS_H_
