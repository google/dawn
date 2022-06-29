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

#ifndef SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_CHANGE_UNARY_OPERATOR_H_
#define SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_CHANGE_UNARY_OPERATOR_H_

#include "src/tint/ast/unary_op.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutation.h"
#include "src/tint/sem/variable.h"

namespace tint::fuzzers::ast_fuzzer {

/// @see MutationChangeUnaryOperator::Apply
class MutationChangeUnaryOperator : public Mutation {
  public:
    /// @brief Constructs an instance of this mutation from a protobuf message.
    /// @param message - protobuf message
    explicit MutationChangeUnaryOperator(protobufs::MutationChangeUnaryOperator message);

    /// @brief Constructor.
    /// @param unary_expr_id - the id of the `ast::UnaryOpExpression` instance
    /// to change its operator.
    /// @param new_operator - A new unary operator for the unary expression
    /// specified by `expression_id`.
    MutationChangeUnaryOperator(uint32_t unary_expr_id, ast::UnaryOp new_operator);

    /// @copybrief Mutation::IsApplicable
    ///
    /// The mutation is applicable if and only if:
    /// - `expression_id` is an id of an `ast::UnaryOpExpression`, that references
    ///   a valid unary expression.
    /// - `new_unary_op` is a valid unary operator of type `ast::UnaryOp`
    ///   to the target expression.
    ///
    /// @copydetails Mutation::IsApplicable
    bool IsApplicable(const tint::Program& program, const NodeIdMap& node_id_map) const override;

    /// @copybrief Mutation::Apply
    ///
    /// Replaces the operator of an unary op expression with `expression_id`
    /// with a new unary operator specified by `new_unary_op'. The modified
    /// expression preserves the same type as the original expression.
    ///
    /// @copydetails Mutation::Apply
    void Apply(const NodeIdMap& node_id_map,
               tint::CloneContext* clone_context,
               NodeIdMap* new_node_id_map) const override;

    protobufs::Mutation ToMessage() const override;

    /// Toggles between the complement and negate unary operators.
    /// @param original_op - a complement or negation unary operator.
    /// @return the other operator.
    static ast::UnaryOp ToggleOperator(const ast::UnaryOp& original_op);

  private:
    protobufs::MutationChangeUnaryOperator message_;
};

}  // namespace tint::fuzzers::ast_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_CHANGE_UNARY_OPERATOR_H_
