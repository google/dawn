// Copyright 2022 The Dawn & Tint Authors
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

#ifndef SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_CHANGE_BINARY_OPERATOR_H_
#define SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_CHANGE_BINARY_OPERATOR_H_

#include "src/tint/fuzzers/tint_ast_fuzzer/mutation.h"

#include "src/tint/lang/wgsl/ast/binary_expression.h"
#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/lang/wgsl/sem/variable.h"

namespace tint::fuzzers::ast_fuzzer {

/// @see MutationChangeBinaryOperator::Apply
class MutationChangeBinaryOperator : public Mutation {
  public:
    /// @brief Constructs an instance of this mutation from a protobuf message.
    /// @param message - protobuf message
    explicit MutationChangeBinaryOperator(protobufs::MutationChangeBinaryOperator message);

    /// @brief Constructor.
    /// @param binary_expr_id - the id of a binary expression.
    /// @param new_operator - a new binary operator to replace the one used in the
    /// expression.
    MutationChangeBinaryOperator(uint32_t binary_expr_id, core::BinaryOp new_operator);

    /// @copybrief Mutation::IsApplicable
    ///
    /// The mutation is applicable iff:
    /// - `binary_expr_id` is a valid id of an `ast::BinaryExpression`.
    /// - `new_operator` is type-compatible with the arguments of the binary
    /// expression.
    ///
    /// @copydetails Mutation::IsApplicable
    bool IsApplicable(const tint::Program& program, const NodeIdMap& node_id_map) const override;

    /// @copybrief Mutation::Apply
    ///
    /// Replaces binary operator in the binary expression corresponding to
    /// `binary_expr_id` with `new_operator`.
    ///
    /// @copydetails Mutation::Apply
    void Apply(const NodeIdMap& node_id_map,
               tint::program::CloneContext& clone_context,
               NodeIdMap* new_node_id_map) const override;

    protobufs::Mutation ToMessage() const override;

    /// @brief Determines whether replacing the operator of a binary expression
    ///     with another operator would preserve well-typedness.
    /// @param program - the program that owns the binary expression.
    /// @param binary_expr - the binary expression being considered for mutation.
    /// @param new_operator - a new binary operator to be checked as a candidate
    ///     replacement for the binary expression's operator.
    /// @return `true` if and only if the replacement would be well-typed.
    static bool CanReplaceBinaryOperator(const Program& program,
                                         const ast::BinaryExpression& binary_expr,
                                         core::BinaryOp new_operator);

  private:
    protobufs::MutationChangeBinaryOperator message_;
};

}  // namespace tint::fuzzers::ast_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_CHANGE_BINARY_OPERATOR_H_
