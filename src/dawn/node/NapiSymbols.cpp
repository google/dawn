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

#include "src/dawn/node/utils/Debug.h"

// To reduce the build dependencies for compiling the dawn.node targets, we do
// not use cmake-js for building, but instead just depend on node_api_headers.
// As the name suggests, node_api_headers contains just the *headers* of Napi,
// and does not provide a library to link against.
// Fortunately node_api_headers provides a list of Napi symbols exported by Node,
// which we can use to produce weak-symbol stubs.

#ifdef _WIN32
#error "NapiSymbols.cpp is not used on Windows"
#endif

#define NAPI_SYMBOL(NAME)                                                              \
    __attribute__((weak)) void NAME() {                                                \
        UNREACHABLE(                                                                   \
            "#NAME is a weak stub, and should have been runtime replaced by the node " \
            "implementation");                                                         \
    }

extern "C" {
// List of Napi symbols generated from the node_api_headers/symbols.js file
#include "NapiSymbols.h"
}
