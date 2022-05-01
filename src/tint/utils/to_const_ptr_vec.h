
// Copyright 2021 The Tint Authors.
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

#ifndef SRC_TINT_UTILS_TO_CONST_PTR_VEC_H_
#define SRC_TINT_UTILS_TO_CONST_PTR_VEC_H_

#include <vector>

namespace tint::utils {

/// @param in a vector of `T*`
/// @returns a vector of `const T*` with the content of `in`.
template <typename T>
std::vector<const T*> ToConstPtrVec(const std::vector<T*>& in) {
    std::vector<const T*> out;
    out.reserve(in.size());
    for (auto* ptr : in) {
        out.emplace_back(ptr);
    }
    return out;
}

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_TO_CONST_PTR_VEC_H_
