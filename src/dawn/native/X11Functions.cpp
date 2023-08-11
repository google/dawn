// Copyright 2021 The Dawn Authors
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

#include "dawn/native/X11Functions.h"

namespace dawn::native {

X11Functions::X11Functions() {
    if (!mX11Lib.Open("libX11.so.6") || !mX11Lib.GetProc(&xSetErrorHandler, "XSetErrorHandler") ||
        !mX11Lib.GetProc(&xGetWindowAttributes, "XGetWindowAttributes")) {
        mX11Lib.Close();
    }

    if (!mX11XcbLib.Open("libX11-xcb.so.1") ||
        !mX11XcbLib.GetProc(&xGetXCBConnection, "XGetXCBConnection")) {
        mX11XcbLib.Close();
    }
}
X11Functions::~X11Functions() = default;

bool X11Functions::IsX11Loaded() const {
    return mX11Lib.Valid();
}

bool X11Functions::IsX11XcbLoaded() const {
    return mX11XcbLib.Valid();
}

}  // namespace dawn::native
