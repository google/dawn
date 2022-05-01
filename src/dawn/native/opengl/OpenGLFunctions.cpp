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

#include "dawn/native/opengl/OpenGLFunctions.h"

#include <cctype>

namespace dawn::native::opengl {

MaybeError OpenGLFunctions::Initialize(GetProcAddress getProc) {
    DAWN_TRY(mVersion.Initialize(getProc));
    if (mVersion.IsES()) {
        DAWN_TRY(LoadOpenGLESProcs(getProc, mVersion.GetMajor(), mVersion.GetMinor()));
    } else {
        DAWN_TRY(LoadDesktopGLProcs(getProc, mVersion.GetMajor(), mVersion.GetMinor()));
    }

    InitializeSupportedGLExtensions();

    return {};
}

void OpenGLFunctions::InitializeSupportedGLExtensions() {
    int32_t numExtensions;
    GetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

    for (int32_t i = 0; i < numExtensions; ++i) {
        const char* extensionName = reinterpret_cast<const char*>(GetStringi(GL_EXTENSIONS, i));
        mSupportedGLExtensionsSet.insert(extensionName);
    }
}

bool OpenGLFunctions::IsGLExtensionSupported(const char* extension) const {
    ASSERT(extension != nullptr);
    return mSupportedGLExtensionsSet.count(extension) != 0;
}

const OpenGLVersion& OpenGLFunctions::GetVersion() const {
    return mVersion;
}

bool OpenGLFunctions::IsAtLeastGL(uint32_t majorVersion, uint32_t minorVersion) const {
    return mVersion.IsDesktop() && mVersion.IsAtLeast(majorVersion, minorVersion);
}

bool OpenGLFunctions::IsAtLeastGLES(uint32_t majorVersion, uint32_t minorVersion) const {
    return mVersion.IsES() && mVersion.IsAtLeast(majorVersion, minorVersion);
}

}  // namespace dawn::native::opengl
