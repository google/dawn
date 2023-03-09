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

#ifndef SRC_TINT_RESOLVER_INTRINSIC_TABLE_H_
#define SRC_TINT_RESOLVER_INTRINSIC_TABLE_H_

#include <memory>
#include <string>

#include "src/tint/ast/binary_expression.h"
#include "src/tint/ast/unary_op.h"
#include "src/tint/resolver/const_eval.h"
#include "src/tint/resolver/ctor_conv_intrinsic.h"
#include "src/tint/sem/builtin.h"
#include "src/tint/utils/vector.h"

// Forward declarations
namespace tint {
class ProgramBuilder;
}  // namespace tint

namespace tint::resolver {

/// IntrinsicTable is a lookup table of all the WGSL builtin functions and intrinsic operators
class IntrinsicTable {
  public:
    /// @param builder the program builder
    /// @return a pointer to a newly created IntrinsicTable
    static std::unique_ptr<IntrinsicTable> Create(ProgramBuilder& builder);

    /// Destructor
    virtual ~IntrinsicTable();

    /// Builtin describes a resolved builtin function
    struct Builtin {
        /// The semantic info for the builtin
        const sem::Builtin* sem = nullptr;
        /// The constant evaluation function
        ConstEval::Function const_eval_fn = nullptr;
    };

    /// UnaryOperator describes a resolved unary operator
    struct UnaryOperator {
        /// The result type of the unary operator
        const type::Type* result = nullptr;
        /// The type of the parameter of the unary operator
        const type::Type* parameter = nullptr;
        /// The constant evaluation function
        ConstEval::Function const_eval_fn = nullptr;
    };

    /// BinaryOperator describes a resolved binary operator
    struct BinaryOperator {
        /// The result type of the binary operator
        const type::Type* result = nullptr;
        /// The type of LHS parameter of the binary operator
        const type::Type* lhs = nullptr;
        /// The type of RHS parameter of the binary operator
        const type::Type* rhs = nullptr;
        /// The constant evaluation function
        ConstEval::Function const_eval_fn = nullptr;
    };

    /// CtorOrConv describes a resolved value constructor or conversion
    struct CtorOrConv {
        /// The result type of the value constructor or conversion
        const sem::CallTarget* target = nullptr;
        /// The constant evaluation function
        ConstEval::Function const_eval_fn = nullptr;
    };

    /// Lookup looks for the builtin overload with the given signature, raising an error diagnostic
    /// if the builtin was not found.
    /// @param type the builtin type
    /// @param args the argument types passed to the builtin function
    /// @param earliest_eval_stage the the earliest evaluation stage that a call to
    ///        the builtin can be made. This can alter the overloads considered.
    ///        For example, if the earliest evaluation stage is
    ///        `sem::EvaluationStage::kRuntime`, then only overloads with concrete argument types
    ///        will be considered, as all abstract-numerics will have been materialized
    ///        after shader creation time (sem::EvaluationStage::kConstant).
    /// @param source the source of the builtin call
    /// @return the semantic builtin if found, otherwise nullptr
    virtual Builtin Lookup(builtin::Function type,
                           utils::VectorRef<const type::Type*> args,
                           sem::EvaluationStage earliest_eval_stage,
                           const Source& source) = 0;

    /// Lookup looks for the unary op overload with the given signature, raising an error
    /// diagnostic if the operator was not found.
    /// @param op the unary operator
    /// @param arg the type of the expression passed to the operator
    /// @param earliest_eval_stage the the earliest evaluation stage that a call to
    ///        the unary operator can be made. This can alter the overloads considered.
    ///        For example, if the earliest evaluation stage is
    ///        `sem::EvaluationStage::kRuntime`, then only overloads with concrete argument types
    ///        will be considered, as all abstract-numerics will have been materialized
    ///        after shader creation time (sem::EvaluationStage::kConstant).
    /// @param source the source of the operator call
    /// @return the operator call target signature. If the operator was not found
    ///         UnaryOperator::result will be nullptr.
    virtual UnaryOperator Lookup(ast::UnaryOp op,
                                 const type::Type* arg,
                                 sem::EvaluationStage earliest_eval_stage,
                                 const Source& source) = 0;

    /// Lookup looks for the binary op overload with the given signature, raising an error
    /// diagnostic if the operator was not found.
    /// @param op the binary operator
    /// @param lhs the LHS value type passed to the operator
    /// @param rhs the RHS value type passed to the operator
    /// @param source the source of the operator call
    /// @param earliest_eval_stage the the earliest evaluation stage that a call to
    ///        the binary operator can be made. This can alter the overloads considered.
    ///        For example, if the earliest evaluation stage is
    ///        `sem::EvaluationStage::kRuntime`, then only overloads with concrete argument types
    ///        will be considered, as all abstract-numerics will have been materialized
    ///        after shader creation time (sem::EvaluationStage::kConstant).
    /// @param is_compound true if the binary operator is being used as a compound assignment
    /// @return the operator call target signature. If the operator was not found
    ///         BinaryOperator::result will be nullptr.
    virtual BinaryOperator Lookup(ast::BinaryOp op,
                                  const type::Type* lhs,
                                  const type::Type* rhs,
                                  sem::EvaluationStage earliest_eval_stage,
                                  const Source& source,
                                  bool is_compound) = 0;

    /// Lookup looks for the value constructor or conversion overload for the given
    /// CtorConvIntrinsic.
    /// @param type the type being constructed or converted
    /// @param template_arg the optional template argument
    /// @param args the argument types passed to the constructor / conversion call
    /// @param earliest_eval_stage the the earliest evaluation stage that a call to
    ///        the constructor or conversion can be made. This can alter the overloads considered.
    ///        For example, if the earliest evaluation stage is
    ///        `sem::EvaluationStage::kRuntime`, then only overloads with concrete argument types
    ///        will be considered, as all abstract-numerics will have been materialized
    ///        after shader creation time (sem::EvaluationStage::kConstant).
    /// @param source the source of the call
    /// @return a sem::ValueConstructor, sem::ValueConversion or nullptr if nothing matched
    virtual CtorOrConv Lookup(CtorConvIntrinsic type,
                              const type::Type* template_arg,
                              utils::VectorRef<const type::Type*> args,
                              sem::EvaluationStage earliest_eval_stage,
                              const Source& source) = 0;
};

}  // namespace tint::resolver

#endif  // SRC_TINT_RESOLVER_INTRINSIC_TABLE_H_
