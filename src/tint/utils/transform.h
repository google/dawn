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

#include "src/tint/utils/traits.h"
#include "src/tint/utils/vector.h"

namespace tint::utils {

/// Transform performs an element-wise transformation of a vector.
/// @param in the input vector.
/// @param transform the transformation function with signature: `OUT(IN)`
/// @returns a new vector with each element of the source vector transformed by `transform`.
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
/// @param transform the transformation function with signature: `OUT(IN, size_t)`
/// @returns a new vector with each element of the source vector transformed by `transform`.
template <typename IN, typename TRANSFORMER>
auto Transform(const std::vector<IN>& in, TRANSFORMER&& transform)
    -> std::vector<decltype(transform(in[0], 1u))> {
    std::vector<decltype(transform(in[0], 1u))> result(in.size());
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = transform(in[i], i);
    }
    return result;
}

/// Transform performs an element-wise transformation of a vector.
/// @param in the input vector.
/// @param transform the transformation function with signature: `OUT(IN)`
/// @returns a new vector with each element of the source vector transformed by `transform`.
template <typename IN, size_t N, typename TRANSFORMER>
auto Transform(const Vector<IN, N>& in, TRANSFORMER&& transform)
    -> Vector<decltype(transform(in[0])), N> {
    const auto count = in.Length();
    Vector<decltype(transform(in[0])), N> result;
    result.Reserve(count);
    for (size_t i = 0; i < count; ++i) {
        result.Push(transform(in[i]));
    }
    return result;
}

/// Transform performs an element-wise transformation of a vector.
/// @param in the input vector.
/// @param transform the transformation function with signature: `OUT(IN, size_t)`
/// @returns a new vector with each element of the source vector transformed by `transform`.
template <typename IN, size_t N, typename TRANSFORMER>
auto Transform(const Vector<IN, N>& in, TRANSFORMER&& transform)
    -> Vector<decltype(transform(in[0], 1u)), N> {
    const auto count = in.Length();
    Vector<decltype(transform(in[0], 1u)), N> result;
    result.Reserve(count);
    for (size_t i = 0; i < count; ++i) {
        result.Push(transform(in[i], i));
    }
    return result;
}

/// Transform performs an element-wise transformation of a vector reference.
/// @param in the input vector.
/// @param transform the transformation function with signature: `OUT(IN)`
/// @tparam N the small-array size of the returned Vector
/// @returns a new vector with each element of the source vector transformed by `transform`.
template <size_t N, typename IN, typename TRANSFORMER>
auto Transform(VectorRef<IN> in, TRANSFORMER&& transform) -> Vector<decltype(transform(in[0])), N> {
    const auto count = in.Length();
    Vector<decltype(transform(in[0])), N> result;
    result.Reserve(count);
    for (size_t i = 0; i < count; ++i) {
        result.Push(transform(in[i]));
    }
    return result;
}

/// Transform performs an element-wise transformation of a vector reference.
/// @param in the input vector.
/// @param transform the transformation function with signature: `OUT(IN, size_t)`
/// @tparam N the small-array size of the returned Vector
/// @returns a new vector with each element of the source vector transformed by `transform`.
template <size_t N, typename IN, typename TRANSFORMER>
auto Transform(VectorRef<IN> in, TRANSFORMER&& transform)
    -> Vector<decltype(transform(in[0], 1u)), N> {
    const auto count = in.Length();
    Vector<decltype(transform(in[0], 1u)), N> result;
    result.Reserve(count);
    for (size_t i = 0; i < count; ++i) {
        result.Push(transform(in[i], i));
    }
    return result;
}

/// TransformN performs an element-wise transformation of a vector, transforming and returning at
/// most `n` elements.
/// @param in the input vector.
/// @param n the maximum number of elements to transform.
/// @param transform the transformation function with signature: `OUT(IN)`
/// @returns a new vector with at most n-elements of the source vector transformed by `transform`.
template <typename IN, typename TRANSFORMER>
auto TransformN(const std::vector<IN>& in, size_t n, TRANSFORMER&& transform)
    -> std::vector<decltype(transform(in[0]))> {
    const auto count = std::min(n, in.size());
    std::vector<decltype(transform(in[0]))> result(count);
    for (size_t i = 0; i < count; ++i) {
        result[i] = transform(in[i]);
    }
    return result;
}

/// TransformN performs an element-wise transformation of a vector, transforming and returning at
/// most `n` elements.
/// @param in the input vector.
/// @param n the maximum number of elements to transform.
/// @param transform the transformation function with signature: `OUT(IN, size_t)`
/// @returns a new vector with at most n-elements of the source vector transformed by `transform`.
template <typename IN, typename TRANSFORMER>
auto TransformN(const std::vector<IN>& in, size_t n, TRANSFORMER&& transform)
    -> std::vector<decltype(transform(in[0], 1u))> {
    const auto count = std::min(n, in.size());
    std::vector<decltype(transform(in[0], 1u))> result(count);
    for (size_t i = 0; i < count; ++i) {
        result[i] = transform(in[i], i);
    }
    return result;
}

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_TRANSFORM_H_
