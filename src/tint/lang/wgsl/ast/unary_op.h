// Copyright 2020 The Tint Authors.
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

#ifndef SRC_TINT_LANG_WGSL_AST_UNARY_OP_H_
#define SRC_TINT_LANG_WGSL_AST_UNARY_OP_H_

#include "src/tint/utils/text/string_stream.h"
#include "src/tint/utils/traits/traits.h"

namespace tint::ast {

/// The unary op
enum class UnaryOp {
    kAddressOf,    // &EXPR
    kComplement,   // ~EXPR
    kIndirection,  // *EXPR
    kNegation,     // -EXPR
    kNot,          // !EXPR
};

/// @param mod the enum value
/// @returns the string for the given enum value
std::string_view ToString(UnaryOp mod);

/// @param out the stream to write to
/// @param mod the UnaryOp
/// @return the stream so calls can be chained
template <typename STREAM, typename = traits::EnableIfIsOStream<STREAM>>
auto& operator<<(STREAM& out, UnaryOp mod) {
    return out << ToString(mod);
}

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_UNARY_OP_H_
