// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_CHANGE_UNARY_OPERATOR_H_
#define SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_CHANGE_UNARY_OPERATOR_H_

#include "src/tint/fuzzers/tint_ast_fuzzer/mutation.h"
#include "src/tint/lang/core/unary_op.h"
#include "src/tint/lang/wgsl/sem/variable.h"

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
    MutationChangeUnaryOperator(uint32_t unary_expr_id, core::UnaryOp new_operator);

    /// @copybrief Mutation::IsApplicable
    ///
    /// The mutation is applicable if and only if:
    /// - `expression_id` is an id of an `ast::UnaryOpExpression`, that references
    ///   a valid unary expression.
    /// - `new_unary_op` is a valid unary operator of type `core::UnaryOp`
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
               tint::program::CloneContext& clone_context,
               NodeIdMap* new_node_id_map) const override;

    protobufs::Mutation ToMessage() const override;

    /// Toggles between the complement and negate unary operators.
    /// @param original_op - a complement or negation unary operator.
    /// @return the other operator.
    static core::UnaryOp ToggleOperator(const core::UnaryOp& original_op);

  private:
    protobufs::MutationChangeUnaryOperator message_;
};

}  // namespace tint::fuzzers::ast_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_CHANGE_UNARY_OPERATOR_H_
