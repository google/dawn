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

#ifndef SRC_DAWN_NATIVE_X11FUNCTIONS_H_
#define SRC_DAWN_NATIVE_X11FUNCTIONS_H_

#include "dawn/common/DynamicLib.h"
#include "dawn/native/Error.h"

#include "dawn/common/xlib_with_undefs.h"

class DynamicLib;

namespace dawn::native {

// A helper class that dynamically loads the x11 and x11-xcb libraries that might not be present
// on all platforms Dawn is deployed on. Note that x11-xcb might not be present even if x11 is.
class X11Functions {
  public:
    X11Functions();
    ~X11Functions();

    bool IsX11Loaded() const;
    bool IsX11XcbLoaded() const;

    // Functions from x11
    decltype(&::XSetErrorHandler) xSetErrorHandler = nullptr;
    decltype(&::XGetWindowAttributes) xGetWindowAttributes = nullptr;

    // Functions from x11-xcb
    decltype(&::XGetXCBConnection) xGetXCBConnection = nullptr;

  private:
    DynamicLib mX11Lib;
    DynamicLib mX11XcbLib;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_X11FUNCTIONS_H_
