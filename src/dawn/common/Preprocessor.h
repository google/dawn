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

#ifndef SRC_DAWN_COMMON_PREPROCESSOR_H_
#define SRC_DAWN_COMMON_PREPROCESSOR_H_

// DAWN_PP_GET_HEAD: get the first element of a __VA_ARGS__ without triggering empty
// __VA_ARGS__ warnings.
#define DAWN_INTERNAL_PP_GET_HEAD(firstParam, ...) firstParam
#define DAWN_PP_GET_HEAD(...) DAWN_INTERNAL_PP_GET_HEAD(__VA_ARGS__, placeholderArg)

// DAWN_PP_CONCATENATE: Concatenate tokens, first expanding the arguments passed in.
#define DAWN_PP_CONCATENATE(arg1, arg2) DAWN_PP_CONCATENATE_1(arg1, arg2)
#define DAWN_PP_CONCATENATE_1(arg1, arg2) DAWN_PP_CONCATENATE_2(arg1, arg2)
#define DAWN_PP_CONCATENATE_2(arg1, arg2) arg1##arg2

// DAWN_PP_EXPAND: Needed to help expand __VA_ARGS__ out on MSVC
#define DAWN_PP_EXPAND(...) __VA_ARGS__

// Implementation of DAWN_PP_FOR_EACH, called by concatenating DAWN_PP_FOR_EACH_ with a number.
#define DAWN_PP_FOR_EACH_1(func, x) func(x)
#define DAWN_PP_FOR_EACH_2(func, x, ...) \
    func(x) DAWN_PP_EXPAND(DAWN_PP_EXPAND(DAWN_PP_FOR_EACH_1)(func, __VA_ARGS__))
#define DAWN_PP_FOR_EACH_3(func, x, ...) \
    func(x) DAWN_PP_EXPAND(DAWN_PP_EXPAND(DAWN_PP_FOR_EACH_2)(func, __VA_ARGS__))
#define DAWN_PP_FOR_EACH_4(func, x, ...) \
    func(x) DAWN_PP_EXPAND(DAWN_PP_EXPAND(DAWN_PP_FOR_EACH_3)(func, __VA_ARGS__))
#define DAWN_PP_FOR_EACH_5(func, x, ...) \
    func(x) DAWN_PP_EXPAND(DAWN_PP_EXPAND(DAWN_PP_FOR_EACH_4)(func, __VA_ARGS__))
#define DAWN_PP_FOR_EACH_6(func, x, ...) \
    func(x) DAWN_PP_EXPAND(DAWN_PP_EXPAND(DAWN_PP_FOR_EACH_5)(func, __VA_ARGS__))
#define DAWN_PP_FOR_EACH_7(func, x, ...) \
    func(x) DAWN_PP_EXPAND(DAWN_PP_EXPAND(DAWN_PP_FOR_EACH_6)(func, __VA_ARGS__))
#define DAWN_PP_FOR_EACH_8(func, x, ...) \
    func(x) DAWN_PP_EXPAND(DAWN_PP_EXPAND(DAWN_PP_FOR_EACH_7)(func, __VA_ARGS__))

// Implementation for DAWN_PP_FOR_EACH. Get the number of args in __VA_ARGS__ so we can concat
// DAWN_PP_FOR_EACH_ and N.
// ex.) DAWN_PP_FOR_EACH_NARG(a, b, c) ->
//      DAWN_PP_FOR_EACH_NARG(a, b, c, DAWN_PP_FOR_EACH_RSEQ()) ->
//      DAWN_PP_FOR_EACH_NARG_(a, b, c, 8, 7, 6, 5, 4, 3, 2, 1, 0) ->
//      DAWN_PP_FOR_EACH_ARG_N(a, b, c, 8, 7, 6, 5, 4, 3, 2, 1, 0) ->
//      DAWN_PP_FOR_EACH_ARG_N( ,  ,  ,  ,  ,  ,  , ,  N) ->
//      3
#define DAWN_PP_FOR_EACH_NARG(...) DAWN_PP_FOR_EACH_NARG_(__VA_ARGS__, DAWN_PP_FOR_EACH_RSEQ())
#define DAWN_PP_FOR_EACH_NARG_(...) \
    DAWN_PP_EXPAND(DAWN_PP_EXPAND(DAWN_PP_FOR_EACH_ARG_N)(__VA_ARGS__))
#define DAWN_PP_FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define DAWN_PP_FOR_EACH_RSEQ() 8, 7, 6, 5, 4, 3, 2, 1, 0

// Implementation for DAWN_PP_FOR_EACH.
// Creates a call to DAWN_PP_FOR_EACH_X where X is 1, 2, ..., etc.
#define DAWN_PP_FOR_EACH_(N, func, ...) DAWN_PP_CONCATENATE(DAWN_PP_FOR_EACH_, N)(func, __VA_ARGS__)

// DAWN_PP_FOR_EACH: Apply |func| to each argument in |x| and __VA_ARGS__
#define DAWN_PP_FOR_EACH(func, ...) \
    DAWN_PP_FOR_EACH_(DAWN_PP_FOR_EACH_NARG(__VA_ARGS__), func, __VA_ARGS__)

#endif  // SRC_DAWN_COMMON_PREPROCESSOR_H_
