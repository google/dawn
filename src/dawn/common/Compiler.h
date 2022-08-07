// Copyright 2017 The Dawn Authors
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

#ifndef SRC_DAWN_COMMON_COMPILER_H_
#define SRC_DAWN_COMMON_COMPILER_H_

// Defines macros for compiler-specific functionality
//  - DAWN_COMPILER_IS(CLANG|GCC|MSVC): Compiler detection
//  - DAWN_BUILTIN_UNREACHABLE(): Hints the compiler that a code path is unreachable
//  - DAWN_(UN)?LIKELY(EXPR): Where available, hints the compiler that the expression will be true
//      (resp. false) to help it generate code that leads to better branch prediction.
//  - DAWN_UNUSED(EXPR): Prevents unused variable/expression warnings on EXPR.
//  - DAWN_UNUSED_FUNC(FUNC): Prevents unused function warnings on FUNC.
//  - DAWN_DECLARE_UNUSED:    Prevents unused function warnings a subsequent declaration.
//  Both DAWN_UNUSED_FUNC and DAWN_DECLARE_UNUSED may be necessary, e.g. to suppress clang's
//  unneeded-internal-declaration warning.

// Clang and GCC, check for __clang__ too to catch clang-cl masquarading as MSVC
#if defined(__GNUC__) || defined(__clang__)
#if defined(__clang__)
#define DAWN_COMPILER_IS_CLANG 1
#else
#define DAWN_COMPILER_IS_GCC 1
#endif

#define DAWN_BUILTIN_UNREACHABLE() __builtin_unreachable()
#define DAWN_LIKELY(x) __builtin_expect(!!(x), 1)
#define DAWN_UNLIKELY(x) __builtin_expect(!!(x), 0)

#if !defined(__has_cpp_attribute)
#define __has_cpp_attribute(name) 0
#endif

#define DAWN_DECLARE_UNUSED __attribute__((unused))
#if defined(NDEBUG)
#define DAWN_FORCE_INLINE inline __attribute__((always_inline))
#endif
#define DAWN_NOINLINE __attribute__((noinline))

// MSVC
#elif defined(_MSC_VER)
#define DAWN_COMPILER_IS_MSVC 1

#define DAWN_BUILTIN_UNREACHABLE() __assume(false)

#define DAWN_DECLARE_UNUSED
#if defined(NDEBUG)
#define DAWN_FORCE_INLINE __forceinline
#endif
#define DAWN_NOINLINE __declspec(noinline)

#else
#error "Unsupported compiler"
#endif

// This section defines other compiler macros to 0 to avoid undefined macro usage error.
#if !defined(DAWN_COMPILER_IS_CLANG)
#define DAWN_COMPILER_IS_CLANG 0
#endif
#if !defined(DAWN_COMPILER_IS_GCC)
#define DAWN_COMPILER_IS_GCC 0
#endif
#if !defined(DAWN_COMPILER_IS_MSVC)
#define DAWN_COMPILER_IS_MSVC 0
#endif

// Use #if DAWN_COMPILER_IS(XXX) for compiler specific code.
// Do not use #ifdef or the naked macro DAWN_COMPILER_IS_XXX.
// This can help avoid common mistakes like not including "Compiler.h" and falling into unwanted
// code block as usage of undefined macro "function" will be blocked by the compiler.
#define DAWN_COMPILER_IS(X) (1 == DAWN_COMPILER_IS_##X)

// It seems that (void) EXPR works on all compilers to silence the unused variable warning.
#define DAWN_UNUSED(EXPR) (void)EXPR
// Likewise using static asserting on sizeof(&FUNC) seems to make it tagged as used
#define DAWN_UNUSED_FUNC(FUNC) static_assert(sizeof(&FUNC) == sizeof(void (*)()))

// Add noop replacements for macros for features that aren't supported by the compiler.
#if !defined(DAWN_LIKELY)
#define DAWN_LIKELY(X) X
#endif
#if !defined(DAWN_UNLIKELY)
#define DAWN_UNLIKELY(X) X
#endif
#if !defined(DAWN_FORCE_INLINE)
#define DAWN_FORCE_INLINE inline
#endif
#if !defined(DAWN_NOINLINE)
#define DAWN_NOINLINE
#endif

#endif  // SRC_DAWN_COMMON_COMPILER_H_
