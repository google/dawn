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

#ifndef SRC_TINT_AST_FLOAT_LITERAL_EXPRESSION_H_
#define SRC_TINT_AST_FLOAT_LITERAL_EXPRESSION_H_

#include <string>

#include "src/tint/ast/literal_expression.h"

namespace tint::ast {

/// A float literal
class FloatLiteralExpression final
    : public utils::Castable<FloatLiteralExpression, LiteralExpression> {
  public:
    /// Literal suffix
    enum class Suffix {
        /// No suffix
        kNone,
        /// 'f' suffix (f32)
        kF,
        /// 'h' suffix (f16)
        kH,
    };

    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param val the literal value
    /// @param suf the literal suffix
    FloatLiteralExpression(ProgramID pid, NodeID nid, const Source& src, double val, Suffix suf);
    ~FloatLiteralExpression() override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const FloatLiteralExpression* Clone(CloneContext* ctx) const override;

    /// The literal value
    const double value;

    /// The literal suffix
    const Suffix suffix;
};

/// Writes the float literal suffix to the stream.
/// @param out the stream to write to
/// @param suffix the suffix to write
/// @returns out so calls can be chained
utils::StringStream& operator<<(utils::StringStream& out, FloatLiteralExpression::Suffix suffix);

}  // namespace tint::ast

#endif  // SRC_TINT_AST_FLOAT_LITERAL_EXPRESSION_H_
