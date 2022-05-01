// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_CHANGE_BINARY_OPERATOR_H_
#define SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_CHANGE_BINARY_OPERATOR_H_

#include "src/tint/fuzzers/tint_ast_fuzzer/mutation.h"

#include "src/tint/ast/binary_expression.h"
#include "src/tint/program.h"
#include "src/tint/sem/variable.h"

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
    MutationChangeBinaryOperator(uint32_t binary_expr_id, ast::BinaryOp new_operator);

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
               tint::CloneContext* clone_context,
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
                                         ast::BinaryOp new_operator);

  private:
    protobufs::MutationChangeBinaryOperator message_;
};

}  // namespace tint::fuzzers::ast_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_CHANGE_BINARY_OPERATOR_H_
