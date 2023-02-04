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

#ifndef SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_WRAP_UNARY_OPERATOR_H_
#define SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_WRAP_UNARY_OPERATOR_H_

#include <vector>

#include "src/tint/fuzzers/tint_ast_fuzzer/mutation.h"

#include "src/tint/ast/unary_op.h"
#include "src/tint/sem/variable.h"

namespace tint::fuzzers::ast_fuzzer {

/// @see MutationWrapUnaryOperator::Apply
class MutationWrapUnaryOperator : public Mutation {
  public:
    /// @brief Constructs an instance of this mutation from a protobuf message.
    /// @param message - protobuf message
    explicit MutationWrapUnaryOperator(protobufs::MutationWrapUnaryOperator message);

    /// @brief Constructor.
    /// @param expression_id - the id of an expression.
    /// @param fresh_id - a fresh id for the created expression node with
    /// unary operator wrapper.
    /// @param unary_op_wrapper - a `ast::UnaryOp` instance.
    MutationWrapUnaryOperator(uint32_t expression_id,
                              uint32_t fresh_id,
                              ast::UnaryOp unary_op_wrapper);

    /// @copybrief Mutation::IsApplicable
    ///
    /// The mutation is applicable iff:
    /// - `expression_id` must refer to a valid expression that can be wrapped
    ///    with unary operator.
    /// - `fresh_id` must be fresh.
    /// - `unary_op_wrapper` is a unary expression that is valid based on the
    ///   type of the given expression.
    ///
    /// @copydetails Mutation::IsApplicable
    bool IsApplicable(const tint::Program& program, const NodeIdMap& node_id_map) const override;

    /// @copybrief Mutation::Apply
    ///
    /// Wrap an expression in a unary operator that is valid based on
    /// the type of the expression.
    ///
    /// @copydetails Mutation::Apply
    void Apply(const NodeIdMap& node_id_map,
               tint::CloneContext* clone_context,
               NodeIdMap* new_node_id_map) const override;

    protobufs::Mutation ToMessage() const override;

    /// Return list of unary operator wrappers allowed for the given
    /// expression.
    /// @param expr - an `ast::Expression` instance from node id map.
    /// @return a list of unary operators.
    static std::vector<ast::UnaryOp> GetValidUnaryWrapper(const sem::ValueExpression& expr);

  private:
    protobufs::MutationWrapUnaryOperator message_;
};

}  // namespace tint::fuzzers::ast_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_WRAP_UNARY_OPERATOR_H_
