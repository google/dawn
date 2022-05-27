// Copyright 2022 The Tint Authors.
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

#include "src/tint/utils/concat.h"

#ifndef SRC_TINT_UTILS_COMPILER_MACROS_H_
#define SRC_TINT_UTILS_COMPILER_MACROS_H_

#define TINT_REQUIRE_SEMICOLON static_assert(true)

#if defined(_MSC_VER)
#define TINT_WARNING_UNREACHABLE_CODE 4702
#define TINT_WARNING_CONSTANT_OVERFLOW 4756

// clang-format off
#define TINT_BEGIN_DISABLE_WARNING(name)                        \
    __pragma(warning(push))                                     \
    __pragma(warning(disable:TINT_CONCAT(TINT_WARNING_, name))) \
    TINT_REQUIRE_SEMICOLON
#define TINT_END_DISABLE_WARNING(name)                          \
    __pragma(warning(pop))                                      \
    TINT_REQUIRE_SEMICOLON
// clang-format on
#else
// clang-format off
#define TINT_BEGIN_DISABLE_WARNING(name) TINT_REQUIRE_SEMICOLON
#define TINT_END_DISABLE_WARNING(name) TINT_REQUIRE_SEMICOLON
// clang-format on
#endif  // defined(_MSC_VER)

#endif  // SRC_TINT_UTILS_COMPILER_MACROS_H_
