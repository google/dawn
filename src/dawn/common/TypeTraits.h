// Copyright 2020 The Dawn Authors
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

#ifndef SRC_DAWN_COMMON_TYPETRAITS_H_
#define SRC_DAWN_COMMON_TYPETRAITS_H_

#include <type_traits>

namespace dawn {

template <typename LHS, typename RHS = LHS, typename T = void>
struct HasEqualityOperator {
    static constexpr const bool value = false;
};

template <typename LHS, typename RHS>
struct HasEqualityOperator<
    LHS,
    RHS,
    std::enable_if_t<
        std::is_same<decltype(std::declval<LHS>() == std::declval<RHS>()), bool>::value>> {
    static constexpr const bool value = true;
};
template <typename T>
struct IsCString {
    static constexpr bool Eval() {
        using Tp = std::decay_t<T>;
        if (!std::is_pointer_v<Tp>) {
            return false;
        }
        return std::is_same_v<std::remove_cv_t<std::remove_pointer_t<Tp>>, char>;
    }

    static constexpr const bool value = Eval();
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_TYPETRAITS_H_
