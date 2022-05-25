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

#ifndef SRC_TINT_UTILS_COMPILER_MACROS_H_
#define SRC_TINT_UTILS_COMPILER_MACROS_H_

#define TINT_REQUIRE_SEMICOLON \
    do {                       \
    } while (false)

#if defined(_MSC_VER)
// clang-format off
#define TINT_BEGIN_DISABLE_WARNING_UNREACHABLE_CODE() \
    __pragma(warning(push))                           \
    __pragma(warning(disable:4702))                   \
    TINT_REQUIRE_SEMICOLON
#define TINT_END_DISABLE_WARNING_UNREACHABLE_CODE()   \
    __pragma(warning(pop))                            \
    TINT_REQUIRE_SEMICOLON
// clang-format on
#else
// clang-format off
#define TINT_BEGIN_DISABLE_WARNING_UNREACHABLE_CODE() TINT_REQUIRE_SEMICOLON
#define TINT_END_DISABLE_WARNING_UNREACHABLE_CODE() TINT_REQUIRE_SEMICOLON
// clang-format on
#endif  // defined(_MSC_VER)

#endif  // SRC_TINT_UTILS_COMPILER_MACROS_H_
