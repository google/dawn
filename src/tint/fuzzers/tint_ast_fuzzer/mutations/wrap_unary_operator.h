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

#ifndef SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_WRAP_UNARY_OPERATOR_H_
#define SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_WRAP_UNARY_OPERATOR_H_

#include <vector>

#include "src/tint/fuzzers/tint_ast_fuzzer/mutation.h"

#include "src/tint/lang/core/unary_op.h"
#include "src/tint/lang/wgsl/sem/variable.h"

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
    /// @param unary_op_wrapper - a `core::UnaryOp` instance.
    MutationWrapUnaryOperator(uint32_t expression_id,
                              uint32_t fresh_id,
                              core::UnaryOp unary_op_wrapper);

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
               tint::program::CloneContext& clone_context,
               NodeIdMap* new_node_id_map) const override;

    protobufs::Mutation ToMessage() const override;

    /// Return list of unary operator wrappers allowed for the given
    /// expression.
    /// @param expr - an `ast::Expression` instance from node id map.
    /// @return a list of unary operators.
    static std::vector<core::UnaryOp> GetValidUnaryWrapper(const sem::ValueExpression& expr);

  private:
    protobufs::MutationWrapUnaryOperator message_;
};

}  // namespace tint::fuzzers::ast_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_AST_FUZZER_MUTATIONS_WRAP_UNARY_OPERATOR_H_
