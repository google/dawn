// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/utils/macros/concat.h"

#ifndef SRC_TINT_UTILS_MACROS_COMPILER_H_
#define SRC_TINT_UTILS_MACROS_COMPILER_H_

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
#define TINT_DISABLE_WARNING_DEPRECATED __pragma(warning(disable : 4996))
#define TINT_DISABLE_WARNING_RESERVED_IDENTIFIER /* currently no-op */
#define TINT_DISABLE_WARNING_UNUSED_VALUE        /* currently no-op */

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
#define TINT_DISABLE_WARNING_DEPRECATED /* currently no-op */
#define TINT_DISABLE_WARNING_RESERVED_IDENTIFIER \
    _Pragma("clang diagnostic ignored \"-Wreserved-identifier\"")
#define TINT_DISABLE_WARNING_UNUSED_VALUE _Pragma("clang diagnostic ignored \"-Wunused-value\"")

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
#define TINT_DISABLE_WARNING_NEWLINE_EOF         /* currently no-op */
#define TINT_DISABLE_WARNING_OLD_STYLE_CAST      /* currently no-op */
#define TINT_DISABLE_WARNING_SIGN_CONVERSION     /* currently no-op */
#define TINT_DISABLE_WARNING_UNREACHABLE_CODE    /* currently no-op */
#define TINT_DISABLE_WARNING_WEAK_VTABLES        /* currently no-op */
#define TINT_DISABLE_WARNING_FLOAT_EQUAL         /* currently no-op */
#define TINT_DISABLE_WARNING_DEPRECATED          /* currently no-op */
#define TINT_DISABLE_WARNING_RESERVED_IDENTIFIER /* currently no-op */
#define TINT_DISABLE_WARNING_UNUSED_VALUE _Pragma("GCC diagnostic ignored \"-Wunused-value\"")

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

#endif  // SRC_TINT_UTILS_MACROS_COMPILER_H_
