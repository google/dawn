// Copyright 2021 The Tint Authors
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

#ifndef SRC_TINT_UTILS_TRANSFORM_H_
#define SRC_TINT_UTILS_TRANSFORM_H_

#include <algorithm>
#include <type_traits>
#include <utility>
#include <vector>

#include "src/tint/traits.h"

namespace tint::utils {

/// Transform performs an element-wise transformation of a vector.
/// @param in the input vector.
/// @param transform the transformation function with signature: `OUT(IN)`
/// @returns a new vector with each element of the source vector transformed by
/// `transform`.
template <typename IN, typename TRANSFORMER>
auto Transform(const std::vector<IN>& in, TRANSFORMER&& transform)
    -> std::vector<decltype(transform(in[0]))> {
    std::vector<decltype(transform(in[0]))> result(in.size());
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = transform(in[i]);
    }
    return result;
}

/// Transform performs an element-wise transformation of a vector.
/// @param in the input vector.
/// @param transform the transformation function with signature:
/// `OUT(IN, size_t)`
/// @returns a new vector with each element of the source vector transformed by
/// `transform`.
template <typename IN, typename TRANSFORMER>
auto Transform(const std::vector<IN>& in, TRANSFORMER&& transform)
    -> std::vector<decltype(transform(in[0], 1u))> {
    std::vector<decltype(transform(in[0], 1u))> result(in.size());
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = transform(in[i], i);
    }
    return result;
}

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_TRANSFORM_H_
