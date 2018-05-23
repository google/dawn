// Copyright 2017 The NXT Authors
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

#ifndef COMMON_COMPILER_H_
#define COMMON_COMPILER_H_

// Defines macros for compiler-specific functionality
//  - NXT_COMPILER_[CLANG|GCC|MSVC]: Compiler detection
//  - NXT_BREAKPOINT(): Raises an exception and breaks in the debugger
//  - NXT_BUILTIN_UNREACHABLE(): Hints the compiler that a code path is unreachable
//  - NXT_NO_DISCARD: An attribute that is C++17 [[nodiscard]] where available

// Clang and GCC
#if defined(__GNUC__)
#    if defined(__clang__)
#        define NXT_COMPILER_CLANG
#    else
#        define NXT_COMPILER_GCC
#    endif

#    if defined(__i386__) || defined(__x86_64__)
#        define NXT_BREAKPOINT() __asm__ __volatile__("int $3\n\t")
#    else
#        error "Implement BREAKPOINT on your platform"
#    endif

#    define NXT_BUILTIN_UNREACHABLE() __builtin_unreachable()

#    if !defined(__has_cpp_attribute)
#        define __has_cpp_attribute(name) 0
#    endif

// Use warn_unused_result on clang otherwise we can a c++1z extension warning in C++14 mode
// Also avoid warn_unused_result with GCC because it is only a function attribute and not a type
// attribute.
#    if __has_cpp_attribute(warn_unused_result) && defined(__clang__)
#        define NXT_NO_DISCARD __attribute__((warn_unused_result))
#    elif NXT_CPP_VERSION >= 17 && __has_cpp_attribute(nodiscard)
#        define NXT_NO_DISCARD [[nodiscard]]
#    endif

// MSVC
#elif defined(_MSC_VER)
#    define NXT_COMPILER_MSVC

extern void __cdecl __debugbreak(void);
#    define NXT_BREAKPOINT() __debugbreak()

#    define NXT_BUILTIN_UNREACHABLE() __assume(false)

// Visual Studio 2017 15.3 adds support for [[nodiscard]]
#    if _MSC_VER >= 1911 && NXT_CPP_VERSION >= 17
#        define NXT_NO_DISCARD [[nodiscard]]
#    endif

#else
#    error "Unsupported compiler"
#endif

// Add noop replacements for macros for features that aren't supported by the compiler.
#if !defined(NXT_NO_DISCARD)
#    define NXT_NO_DISCARD
#endif

#endif  // COMMON_COMPILER_H_
