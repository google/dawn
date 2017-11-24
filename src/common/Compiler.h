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

// MSVC
#elif defined(_MSC_VER)
#    define NXT_COMPILER_MSVC

extern void __cdecl __debugbreak(void);
#    define NXT_BREAKPOINT() __debugbreak()

#    define NXT_BUILTIN_UNREACHABLE() __assume(false)

#else
#    error "Unsupported compiler"
#endif

#endif  // COMMON_COMPILER_H_
