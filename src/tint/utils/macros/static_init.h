// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_UTILS_MACROS_STATIC_INIT_H_
#define SRC_TINT_UTILS_MACROS_STATIC_INIT_H_

#include "src/tint/utils/macros/concat.h"

/// A helper macro that executes STATEMENT the first time the macro comes into scope - typically
/// used at global scope to call a function before main() is run.
/// For example: `TINT_STATIC_INIT(CallAtStartup(1,2,3));`
/// @note: This must not be used at global scope in production code, as this violates the Chromium
/// rules around static initializers. Attempting to do this will result in a compilation error.
#define TINT_STATIC_INIT(STATEMENT)                                                       \
    [[maybe_unused]] static const bool TINT_CONCAT(tint_static_init_, __COUNTER__) = [] { \
        STATEMENT;                                                                        \
        return true;                                                                      \
    }()

#endif  // SRC_TINT_UTILS_MACROS_STATIC_INIT_H_
