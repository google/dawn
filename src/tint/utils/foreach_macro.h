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

#ifndef SRC_TINT_UTILS_FOREACH_MACRO_H_
#define SRC_TINT_UTILS_FOREACH_MACRO_H_

// Macro magic to perform macro variadic dispatch.
// See:
// https://renenyffenegger.ch/notes/development/languages/C-C-plus-plus/preprocessor/macros/__VA_ARGS__/count-arguments
// Note, this doesn't attempt to use the ##__VA_ARGS__ trick to handle 0

// Helper macro to force expanding __VA_ARGS__ to satisfy MSVC compiler.
#define TINT_MSVC_EXPAND_BUG(X) X

/// TINT_COUNT_ARGUMENTS_NTH_ARG is used by TINT_COUNT_ARGUMENTS to get the number of arguments in a
/// variadic macro call.
#define TINT_COUNT_ARGUMENTS_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, \
                                     _15, _16, N, ...)                                            \
    N

/// TINT_COUNT_ARGUMENTS evaluates to the number of arguments passed to the macro
#define TINT_COUNT_ARGUMENTS(...)                                                                 \
    TINT_MSVC_EXPAND_BUG(TINT_COUNT_ARGUMENTS_NTH_ARG(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, \
                                                      8, 7, 6, 5, 4, 3, 2, 1, 0))

// Correctness checks.
static_assert(1 == TINT_COUNT_ARGUMENTS(a), "TINT_COUNT_ARGUMENTS broken");
static_assert(2 == TINT_COUNT_ARGUMENTS(a, b), "TINT_COUNT_ARGUMENTS broken");
static_assert(3 == TINT_COUNT_ARGUMENTS(a, b, c), "TINT_COUNT_ARGUMENTS broken");

/// TINT_FOREACH calls CB with each of the variadic arguments.
#define TINT_FOREACH(CB, ...) \
    TINT_MSVC_EXPAND_BUG(     \
        TINT_CONCAT(TINT_FOREACH_, TINT_COUNT_ARGUMENTS(__VA_ARGS__))(CB, __VA_ARGS__))

#define TINT_FOREACH_1(CB, _1) CB(_1)
#define TINT_FOREACH_2(CB, _1, _2) \
    TINT_FOREACH_1(CB, _1)         \
    CB(_2)
#define TINT_FOREACH_3(CB, _1, _2, _3) \
    TINT_FOREACH_2(CB, _1, _2)         \
    CB(_3)
#define TINT_FOREACH_4(CB, _1, _2, _3, _4) \
    TINT_FOREACH_3(CB, _1, _2, _3)         \
    CB(_4)
#define TINT_FOREACH_5(CB, _1, _2, _3, _4, _5) \
    TINT_FOREACH_4(CB, _1, _2, _3, _4)         \
    CB(_5)
#define TINT_FOREACH_6(CB, _1, _2, _3, _4, _5, _6) \
    TINT_FOREACH_5(CB, _1, _2, _3, _4, _5)         \
    CB(_6)
#define TINT_FOREACH_7(CB, _1, _2, _3, _4, _5, _6, _7) \
    TINT_FOREACH_6(CB, _1, _2, _3, _4, _5, _6)         \
    CB(_7)
#define TINT_FOREACH_8(CB, _1, _2, _3, _4, _5, _6, _7, _8) \
    TINT_FOREACH_7(CB, _1, _2, _3, _4, _5, _6, _7)         \
    CB(_8)
#define TINT_FOREACH_9(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9) \
    TINT_FOREACH_8(CB, _1, _2, _3, _4, _5, _6, _7, _8)         \
    CB(_9)
#define TINT_FOREACH_10(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10) \
    TINT_FOREACH_9(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9)           \
    CB(_10)
#define TINT_FOREACH_11(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11) \
    TINT_FOREACH_10(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10)          \
    CB(_11)
#define TINT_FOREACH_12(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12) \
    TINT_FOREACH_11(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11)          \
    CB(_12)
#define TINT_FOREACH_13(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13) \
    TINT_FOREACH_11(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12)          \
    CB(_13)
#define TINT_FOREACH_14(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14) \
    TINT_FOREACH_11(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13)          \
    CB(_14)
#define TINT_FOREACH_15(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15) \
    TINT_FOREACH_11(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14)          \
    CB(_15)

#endif  // SRC_TINT_UTILS_FOREACH_MACRO_H_
