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

#include "dawn/native/opengl/OpenGLVersion.h"

#include <cctype>
#include <string>
#include <tuple>

namespace dawn::native::opengl {

MaybeError OpenGLVersion::Initialize(GetProcAddress getProc) {
    PFNGLGETSTRINGPROC getString = reinterpret_cast<PFNGLGETSTRINGPROC>(getProc("glGetString"));
    if (getString == nullptr) {
        return DAWN_INTERNAL_ERROR("Couldn't load glGetString");
    }

    const char* version = reinterpret_cast<const char*>(getString(GL_VERSION));

    if (strstr(version, "OpenGL ES") != nullptr) {
        // ES spec states that the GL_VERSION string will be in the following format:
        // "OpenGL ES N.M vendor-specific information"
        mStandard = Standard::ES;
        mMajorVersion = version[10] - '0';
        mMinorVersion = version[12] - '0';

        // The minor version shouldn't get to two digits.
        ASSERT(strlen(version) <= 13 || !isdigit(version[13]));
    } else {
        // OpenGL spec states the GL_VERSION string will be in the following format:
        // <version number><space><vendor-specific information>
        // The version number is either of the form major number.minor number or major
        // number.minor number.release number, where the numbers all have one or more
        // digits
        mStandard = Standard::Desktop;
        mMajorVersion = version[0] - '0';
        mMinorVersion = version[2] - '0';

        // The minor version shouldn't get to two digits.
        ASSERT(strlen(version) <= 3 || !isdigit(version[3]));
    }

    return {};
}

bool OpenGLVersion::IsDesktop() const {
    return mStandard == Standard::Desktop;
}

bool OpenGLVersion::IsES() const {
    return mStandard == Standard::ES;
}

OpenGLVersion::Standard OpenGLVersion::GetStandard() const {
    return mStandard;
}

uint32_t OpenGLVersion::GetMajor() const {
    return mMajorVersion;
}

uint32_t OpenGLVersion::GetMinor() const {
    return mMinorVersion;
}

bool OpenGLVersion::IsAtLeast(uint32_t majorVersion, uint32_t minorVersion) const {
    return std::tie(mMajorVersion, mMinorVersion) >= std::tie(majorVersion, minorVersion);
}

}  // namespace dawn::native::opengl
