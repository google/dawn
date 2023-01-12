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

#if defined(_MSC_VER) && !defined(__clang__)
////////////////////////////////////////////////////////////////////////////////
// MSVC
////////////////////////////////////////////////////////////////////////////////
#define TINT_DISABLE_WARNING_CONSTANT_OVERFLOW __pragma(warning(disable : 4756))
#define TINT_DISABLE_WARNING_MAYBE_UNINITIALIZED /* currently no-op */
#define TINT_DISABLE_WARNING_NEWLINE_EOF         /* currently no-op */
#define TINT_DISABLE_WARNING_OLD_STYLE_CAST      /* currently no-op */
#define TINT_DISABLE_WARNING_SIGN_CONVERSION     /* currently no-op */
#define TINT_DISABLE_WARNING_UNREACHABLE_CODE __pragma(warning(disable : 4702))
#define TINT_DISABLE_WARNING_WEAK_VTABLES /* currently no-op */
#define TINT_DISABLE_WARNING_FLOAT_EQUAL  /* currently no-op */

// clang-format off
#define TINT_BEGIN_DISABLE_WARNING(name)     \
    __pragma(warning(push))                  \
    TINT_CONCAT(TINT_DISABLE_WARNING_, name) \
    TINT_REQUIRE_SEMICOLON
#define TINT_END_DISABLE_WARNING(name)       \
    __pragma(warning(pop))                   \
    TINT_REQUIRE_SEMICOLON
// clang-format on

#define TINT_UNLIKELY(x) x /* currently no-op */
#define TINT_LIKELY(x) x   /* currently no-op */
#elif defined(__clang__)
////////////////////////////////////////////////////////////////////////////////
// Clang
////////////////////////////////////////////////////////////////////////////////
#define TINT_DISABLE_WARNING_CONSTANT_OVERFLOW   /* currently no-op */
#define TINT_DISABLE_WARNING_MAYBE_UNINITIALIZED /* currently no-op */
#define TINT_DISABLE_WARNING_NEWLINE_EOF _Pragma("clang diagnostic ignored \"-Wnewline-eof\"")
#define TINT_DISABLE_WARNING_OLD_STYLE_CAST _Pragma("clang diagnostic ignored \"-Wold-style-cast\"")
#define TINT_DISABLE_WARNING_SIGN_CONVERSION \
    _Pragma("clang diagnostic ignored \"-Wsign-conversion\"")
#define TINT_DISABLE_WARNING_UNREACHABLE_CODE /* currently no-op */
#define TINT_DISABLE_WARNING_WEAK_VTABLES _Pragma("clang diagnostic ignored \"-Wweak-vtables\"")
#define TINT_DISABLE_WARNING_FLOAT_EQUAL _Pragma("clang diagnostic ignored \"-Wfloat-equal\"")

// clang-format off
#define TINT_BEGIN_DISABLE_WARNING(name)     \
    _Pragma("clang diagnostic push")         \
    TINT_CONCAT(TINT_DISABLE_WARNING_, name) \
    TINT_REQUIRE_SEMICOLON
#define TINT_END_DISABLE_WARNING(name)       \
    _Pragma("clang diagnostic pop")          \
    TINT_REQUIRE_SEMICOLON
// clang-format on

#define TINT_UNLIKELY(x) __builtin_expect(!!(x), false)
#define TINT_LIKELY(x) __builtin_expect(!!(x), true)
#elif defined(__GNUC__)
////////////////////////////////////////////////////////////////////////////////
// GCC
////////////////////////////////////////////////////////////////////////////////
#define TINT_DISABLE_WARNING_CONSTANT_OVERFLOW /* currently no-op */
#define TINT_DISABLE_WARNING_MAYBE_UNINITIALIZED \
    _Pragma("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
#define TINT_DISABLE_WARNING_NEWLINE_EOF      /* currently no-op */
#define TINT_DISABLE_WARNING_OLD_STYLE_CAST   /* currently no-op */
#define TINT_DISABLE_WARNING_SIGN_CONVERSION  /* currently no-op */
#define TINT_DISABLE_WARNING_UNREACHABLE_CODE /* currently no-op */
#define TINT_DISABLE_WARNING_WEAK_VTABLES     /* currently no-op */
#define TINT_DISABLE_WARNING_FLOAT_EQUAL      /* currently no-op */

// clang-format off
#define TINT_BEGIN_DISABLE_WARNING(name)     \
    _Pragma("GCC diagnostic push")           \
    TINT_CONCAT(TINT_DISABLE_WARNING_, name) \
    TINT_REQUIRE_SEMICOLON
#define TINT_END_DISABLE_WARNING(name)       \
    _Pragma("GCC diagnostic pop")            \
    TINT_REQUIRE_SEMICOLON
// clang-format on

#define TINT_UNLIKELY(x) __builtin_expect(!!(x), false)
#define TINT_LIKELY(x) __builtin_expect(!!(x), true)
#else
////////////////////////////////////////////////////////////////////////////////
// Other
////////////////////////////////////////////////////////////////////////////////
#define TINT_BEGIN_DISABLE_WARNING(name) TINT_REQUIRE_SEMICOLON
#define TINT_END_DISABLE_WARNING(name) TINT_REQUIRE_SEMICOLON
#define TINT_UNLIKELY(x) x
#define TINT_LIKELY(x) x

#endif

#endif  // SRC_TINT_UTILS_COMPILER_MACROS_H_
