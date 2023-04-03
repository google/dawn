// Copyright 2023 The Dawn Authors
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

#include "dawn/native/d3d11/PlatformFunctionsD3D11.h"

#include <string>

namespace dawn::native::d3d11 {

PlatformFunctions::PlatformFunctions() = default;
PlatformFunctions::~PlatformFunctions() = default;

MaybeError PlatformFunctions::LoadFunctions() {
    DAWN_TRY(Base::LoadFunctions());

    DAWN_TRY(LoadD3D11());
    return {};
}

MaybeError PlatformFunctions::LoadD3D11() {
#if DAWN_PLATFORM_IS(WINUWP)
    d3d11CreateDevice = &D3D11CreateDevice;
#else
    std::string error;
    if (!mD3D11Lib.Open("d3d11.dll", &error) ||
        !mD3D11Lib.GetProc(&d3d11CreateDevice, "D3D11CreateDevice", &error)) {
        return DAWN_INTERNAL_ERROR(error.c_str());
    }
#endif

    return {};
}

}  // namespace dawn::native::d3d11
