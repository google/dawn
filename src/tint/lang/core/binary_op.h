// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_LANG_CORE_BINARY_OP_H_
#define SRC_TINT_LANG_CORE_BINARY_OP_H_

#include "src/tint/utils/traits/traits.h"

namespace tint::core {

/// An enumerator of binary operators.
enum class BinaryOp {
    kAnd,  // &
    kOr,   // |
    kXor,
    kLogicalAnd,  // &&
    kLogicalOr,   // ||
    kEqual,
    kNotEqual,
    kLessThan,
    kGreaterThan,
    kLessThanEqual,
    kGreaterThanEqual,
    kShiftLeft,
    kShiftRight,
    kAdd,
    kSubtract,
    kMultiply,
    kDivide,
    kModulo,
};

/// @param value the enum value
/// @returns the string for the given enum value
std::string_view ToString(BinaryOp value);

/// @param out the stream to write to
/// @param value the BinaryOp
/// @returns @p out so calls can be chained
template <typename STREAM, typename = traits::EnableIfIsOStream<STREAM>>
auto& operator<<(STREAM& out, BinaryOp value) {
    return out << ToString(value);
}

}  // namespace tint::core

#endif  // SRC_TINT_LANG_CORE_BINARY_OP_H_
