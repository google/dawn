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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/change_binary_operator.h"

#include <utility>

#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/wgsl/program/program_builder.h"

namespace tint::fuzzers::ast_fuzzer {

namespace {

bool IsSuitableForShift(const core::type::Type* lhs_type, const core::type::Type* rhs_type) {
    // `a << b` requires b to be an unsigned scalar or vector, and `a` to be an
    // integer scalar or vector with the same width as `b`. Similar for `a >> b`.

    if (rhs_type->is_unsigned_integer_scalar()) {
        return lhs_type->is_integer_scalar();
    }
    if (rhs_type->is_unsigned_integer_vector()) {
        return lhs_type->is_unsigned_integer_vector();
    }
    return false;
}

bool CanReplaceAddSubtractWith(const core::type::Type* lhs_type,
                               const core::type::Type* rhs_type,
                               core::BinaryOp new_operator) {
    // The program is assumed to be well-typed, so this method determines when
    // 'new_operator' can be used as a type-preserving replacement in an '+' or
    // '-' expression.
    switch (new_operator) {
        case core::BinaryOp::kAdd:
        case core::BinaryOp::kSubtract:
            // '+' and '-' are fully type compatible.
            return true;
        case core::BinaryOp::kAnd:
        case core::BinaryOp::kOr:
        case core::BinaryOp::kXor:
            // These operators do not have a mixed vector-scalar form, and only work
            // on integer types.
            return lhs_type == rhs_type && lhs_type->is_integer_scalar_or_vector();
        case core::BinaryOp::kMultiply:
            // '+' and '*' are largely type-compatible, but for matrices they are only
            // type-compatible if the matrices are square.
            return !lhs_type->is_float_matrix() || lhs_type->is_square_float_matrix();
        case core::BinaryOp::kDivide:
        case core::BinaryOp::kModulo:
            // '/' is not defined for matrices.
            return lhs_type->is_numeric_scalar_or_vector() &&
                   rhs_type->is_numeric_scalar_or_vector();
        case core::BinaryOp::kShiftLeft:
        case core::BinaryOp::kShiftRight:
            return IsSuitableForShift(lhs_type, rhs_type);
        default:
            return false;
    }
}

bool CanReplaceMultiplyWith(const core::type::Type* lhs_type,
                            const core::type::Type* rhs_type,
                            core::BinaryOp new_operator) {
    // The program is assumed to be well-typed, so this method determines when
    // 'new_operator' can be used as a type-preserving replacement in a '*'
    // expression.
    switch (new_operator) {
        case core::BinaryOp::kMultiply:
            return true;
        case core::BinaryOp::kAdd:
        case core::BinaryOp::kSubtract:
            // '*' is type-compatible with '+' and '-' for square matrices, and for
            // numeric scalars/vectors.
            if (lhs_type->is_square_float_matrix() && rhs_type->is_square_float_matrix()) {
                return true;
            }
            return lhs_type->is_numeric_scalar_or_vector() &&
                   rhs_type->is_numeric_scalar_or_vector();
        case core::BinaryOp::kAnd:
        case core::BinaryOp::kOr:
        case core::BinaryOp::kXor:
            // These operators require homogeneous integer types.
            return lhs_type == rhs_type && lhs_type->is_integer_scalar_or_vector();
        case core::BinaryOp::kDivide:
        case core::BinaryOp::kModulo:
            // '/' is not defined for matrices.
            return lhs_type->is_numeric_scalar_or_vector() &&
                   rhs_type->is_numeric_scalar_or_vector();
        case core::BinaryOp::kShiftLeft:
        case core::BinaryOp::kShiftRight:
            return IsSuitableForShift(lhs_type, rhs_type);
        default:
            return false;
    }
}

bool CanReplaceDivideOrModuloWith(const core::type::Type* lhs_type,
                                  const core::type::Type* rhs_type,
                                  core::BinaryOp new_operator) {
    // The program is assumed to be well-typed, so this method determines when
    // 'new_operator' can be used as a type-preserving replacement in a '/'
    // expression.
    switch (new_operator) {
        case core::BinaryOp::kAdd:
        case core::BinaryOp::kSubtract:
        case core::BinaryOp::kMultiply:
        case core::BinaryOp::kDivide:
        case core::BinaryOp::kModulo:
            // These operators work in all contexts where '/' works.
            return true;
        case core::BinaryOp::kAnd:
        case core::BinaryOp::kOr:
        case core::BinaryOp::kXor:
            // These operators require homogeneous integer types.
            return lhs_type == rhs_type && lhs_type->is_integer_scalar_or_vector();
        case core::BinaryOp::kShiftLeft:
        case core::BinaryOp::kShiftRight:
            return IsSuitableForShift(lhs_type, rhs_type);
        default:
            return false;
    }
}

bool CanReplaceLogicalAndLogicalOrWith(core::BinaryOp new_operator) {
    switch (new_operator) {
        case core::BinaryOp::kLogicalAnd:
        case core::BinaryOp::kLogicalOr:
        case core::BinaryOp::kAnd:
        case core::BinaryOp::kOr:
        case core::BinaryOp::kEqual:
        case core::BinaryOp::kNotEqual:
            // These operators all work whenever '&&' and '||' work.
            return true;
        default:
            return false;
    }
}

bool CanReplaceAndOrWith(const core::type::Type* lhs_type,
                         const core::type::Type* rhs_type,
                         core::BinaryOp new_operator) {
    switch (new_operator) {
        case core::BinaryOp::kAnd:
        case core::BinaryOp::kOr:
            // '&' and '|' work in all the same contexts.
            return true;
        case core::BinaryOp::kAdd:
        case core::BinaryOp::kSubtract:
        case core::BinaryOp::kMultiply:
        case core::BinaryOp::kDivide:
        case core::BinaryOp::kModulo:
        case core::BinaryOp::kXor:
            // '&' and '|' can be applied to booleans. In all other contexts,
            // integer numeric operators work.
            return !lhs_type->is_bool_scalar_or_vector();
        case core::BinaryOp::kShiftLeft:
        case core::BinaryOp::kShiftRight:
            return IsSuitableForShift(lhs_type, rhs_type);
        case core::BinaryOp::kLogicalAnd:
        case core::BinaryOp::kLogicalOr:
            // '&' and '|' can be applied to booleans, and for boolean scalar
            // scalar contexts, their logical counterparts work.
            return lhs_type->Is<core::type::Bool>();
        case core::BinaryOp::kEqual:
        case core::BinaryOp::kNotEqual:
            // '&' and '|' can be applied to booleans, and in these contexts equality
            // comparison operators also work.
            return lhs_type->is_bool_scalar_or_vector();
        default:
            return false;
    }
}

bool CanReplaceXorWith(const core::type::Type* lhs_type,
                       const core::type::Type* rhs_type,
                       core::BinaryOp new_operator) {
    switch (new_operator) {
        case core::BinaryOp::kAdd:
        case core::BinaryOp::kSubtract:
        case core::BinaryOp::kMultiply:
        case core::BinaryOp::kDivide:
        case core::BinaryOp::kModulo:
        case core::BinaryOp::kAnd:
        case core::BinaryOp::kOr:
        case core::BinaryOp::kXor:
            // '^' only works on integer types, and in any such context, all other
            // integer operators also work.
            return true;
        case core::BinaryOp::kShiftLeft:
        case core::BinaryOp::kShiftRight:
            return IsSuitableForShift(lhs_type, rhs_type);
        default:
            return false;
    }
}

bool CanReplaceShiftLeftShiftRightWith(const core::type::Type* lhs_type,
                                       const core::type::Type* rhs_type,
                                       core::BinaryOp new_operator) {
    switch (new_operator) {
        case core::BinaryOp::kShiftLeft:
        case core::BinaryOp::kShiftRight:
            // These operators are type-compatible.
            return true;
        case core::BinaryOp::kAdd:
        case core::BinaryOp::kSubtract:
        case core::BinaryOp::kMultiply:
        case core::BinaryOp::kDivide:
        case core::BinaryOp::kModulo:
        case core::BinaryOp::kAnd:
        case core::BinaryOp::kOr:
        case core::BinaryOp::kXor:
            // Shift operators allow mixing of signed and unsigned arguments, but in
            // the case where the arguments are homogeneous, they are type-compatible
            // with other numeric operators.
            return lhs_type == rhs_type;
        default:
            return false;
    }
}

bool CanReplaceEqualNotEqualWith(const core::type::Type* lhs_type, core::BinaryOp new_operator) {
    switch (new_operator) {
        case core::BinaryOp::kEqual:
        case core::BinaryOp::kNotEqual:
            // These operators are type-compatible.
            return true;
        case core::BinaryOp::kLessThan:
        case core::BinaryOp::kLessThanEqual:
        case core::BinaryOp::kGreaterThan:
        case core::BinaryOp::kGreaterThanEqual:
            // An equality comparison between numeric types can be changed to an
            // ordered comparison.
            return lhs_type->is_numeric_scalar_or_vector();
        case core::BinaryOp::kLogicalAnd:
        case core::BinaryOp::kLogicalOr:
            // An equality comparison between boolean scalars can be turned into a
            // logical operation.
            return lhs_type->Is<core::type::Bool>();
        case core::BinaryOp::kAnd:
        case core::BinaryOp::kOr:
            // An equality comparison between boolean scalars or vectors can be turned
            // into a component-wise non-short-circuit logical operation.
            return lhs_type->is_bool_scalar_or_vector();
        default:
            return false;
    }
}

bool CanReplaceLessThanLessThanEqualGreaterThanGreaterThanEqualWith(core::BinaryOp new_operator) {
    switch (new_operator) {
        case core::BinaryOp::kEqual:
        case core::BinaryOp::kNotEqual:
        case core::BinaryOp::kLessThan:
        case core::BinaryOp::kLessThanEqual:
        case core::BinaryOp::kGreaterThan:
        case core::BinaryOp::kGreaterThanEqual:
            // Ordered comparison operators can be interchanged, and equality
            // operators can be used in their place.
            return true;
        default:
            return false;
    }
}
}  // namespace

MutationChangeBinaryOperator::MutationChangeBinaryOperator(
    protobufs::MutationChangeBinaryOperator message)
    : message_(std::move(message)) {}

MutationChangeBinaryOperator::MutationChangeBinaryOperator(uint32_t binary_expr_id,
                                                           core::BinaryOp new_operator) {
    message_.set_binary_expr_id(binary_expr_id);
    message_.set_new_operator(static_cast<uint32_t>(new_operator));
}

bool MutationChangeBinaryOperator::CanReplaceBinaryOperator(
    const Program& program,
    const ast::BinaryExpression& binary_expr,
    core::BinaryOp new_operator) {
    if (new_operator == binary_expr.op) {
        // An operator should not be replaced with itself, as this would be a no-op.
        return false;
    }

    // Get the types of the operators.
    const auto* lhs_type = program.Sem().GetVal(binary_expr.lhs)->Type();
    const auto* rhs_type = program.Sem().GetVal(binary_expr.rhs)->Type();

    // If these are reference types, unwrap them to get the pointee type.
    const core::type::Type* lhs_basic_type =
        lhs_type->Is<core::type::Reference>() ? lhs_type->As<core::type::Reference>()->StoreType()
                                              : lhs_type;
    const core::type::Type* rhs_basic_type =
        rhs_type->Is<core::type::Reference>() ? rhs_type->As<core::type::Reference>()->StoreType()
                                              : rhs_type;

    switch (binary_expr.op) {
        case core::BinaryOp::kAdd:
        case core::BinaryOp::kSubtract:
            return CanReplaceAddSubtractWith(lhs_basic_type, rhs_basic_type, new_operator);
        case core::BinaryOp::kMultiply:
            return CanReplaceMultiplyWith(lhs_basic_type, rhs_basic_type, new_operator);
        case core::BinaryOp::kDivide:
        case core::BinaryOp::kModulo:
            return CanReplaceDivideOrModuloWith(lhs_basic_type, rhs_basic_type, new_operator);
        case core::BinaryOp::kAnd:
        case core::BinaryOp::kOr:
            return CanReplaceAndOrWith(lhs_basic_type, rhs_basic_type, new_operator);
        case core::BinaryOp::kXor:
            return CanReplaceXorWith(lhs_basic_type, rhs_basic_type, new_operator);
        case core::BinaryOp::kShiftLeft:
        case core::BinaryOp::kShiftRight:
            return CanReplaceShiftLeftShiftRightWith(lhs_basic_type, rhs_basic_type, new_operator);
        case core::BinaryOp::kLogicalAnd:
        case core::BinaryOp::kLogicalOr:
            return CanReplaceLogicalAndLogicalOrWith(new_operator);
        case core::BinaryOp::kEqual:
        case core::BinaryOp::kNotEqual:
            return CanReplaceEqualNotEqualWith(lhs_basic_type, new_operator);
        case core::BinaryOp::kLessThan:
        case core::BinaryOp::kLessThanEqual:
        case core::BinaryOp::kGreaterThan:
        case core::BinaryOp::kGreaterThanEqual:
            return CanReplaceLessThanLessThanEqualGreaterThanGreaterThanEqualWith(new_operator);
            assert(false && "Unreachable");
            return false;
    }
}

bool MutationChangeBinaryOperator::IsApplicable(const Program& program,
                                                const NodeIdMap& node_id_map) const {
    const auto* binary_expr_node =
        As<ast::BinaryExpression>(node_id_map.GetNode(message_.binary_expr_id()));
    if (binary_expr_node == nullptr) {
        // Either the id does not exist, or does not correspond to a binary
        // expression.
        return false;
    }
    // Check whether the replacement is acceptable.
    const auto new_operator = static_cast<core::BinaryOp>(message_.new_operator());
    return CanReplaceBinaryOperator(program, *binary_expr_node, new_operator);
}

void MutationChangeBinaryOperator::Apply(const NodeIdMap& node_id_map,
                                         program::CloneContext& clone_context,
                                         NodeIdMap* new_node_id_map) const {
    // Get the node whose operator is to be replaced.
    const auto* binary_expr_node =
        As<ast::BinaryExpression>(node_id_map.GetNode(message_.binary_expr_id()));

    // Clone the binary expression, with the appropriate new operator.
    const ast::BinaryExpression* cloned_replacement;
    switch (static_cast<core::BinaryOp>(message_.new_operator())) {
        case core::BinaryOp::kAnd:
            cloned_replacement = clone_context.dst->And(clone_context.Clone(binary_expr_node->lhs),
                                                        clone_context.Clone(binary_expr_node->rhs));
            break;
        case core::BinaryOp::kOr:
            cloned_replacement = clone_context.dst->Or(clone_context.Clone(binary_expr_node->lhs),
                                                       clone_context.Clone(binary_expr_node->rhs));
            break;
        case core::BinaryOp::kXor:
            cloned_replacement = clone_context.dst->Xor(clone_context.Clone(binary_expr_node->lhs),
                                                        clone_context.Clone(binary_expr_node->rhs));
            break;
        case core::BinaryOp::kLogicalAnd:
            cloned_replacement =
                clone_context.dst->LogicalAnd(clone_context.Clone(binary_expr_node->lhs),
                                              clone_context.Clone(binary_expr_node->rhs));
            break;
        case core::BinaryOp::kLogicalOr:
            cloned_replacement =
                clone_context.dst->LogicalOr(clone_context.Clone(binary_expr_node->lhs),
                                             clone_context.Clone(binary_expr_node->rhs));
            break;
        case core::BinaryOp::kEqual:
            cloned_replacement =
                clone_context.dst->Equal(clone_context.Clone(binary_expr_node->lhs),
                                         clone_context.Clone(binary_expr_node->rhs));
            break;
        case core::BinaryOp::kNotEqual:
            cloned_replacement =
                clone_context.dst->NotEqual(clone_context.Clone(binary_expr_node->lhs),
                                            clone_context.Clone(binary_expr_node->rhs));
            break;
        case core::BinaryOp::kLessThan:
            cloned_replacement =
                clone_context.dst->LessThan(clone_context.Clone(binary_expr_node->lhs),
                                            clone_context.Clone(binary_expr_node->rhs));
            break;
        case core::BinaryOp::kGreaterThan:
            cloned_replacement =
                clone_context.dst->GreaterThan(clone_context.Clone(binary_expr_node->lhs),
                                               clone_context.Clone(binary_expr_node->rhs));
            break;
        case core::BinaryOp::kLessThanEqual:
            cloned_replacement =
                clone_context.dst->LessThanEqual(clone_context.Clone(binary_expr_node->lhs),
                                                 clone_context.Clone(binary_expr_node->rhs));
            break;
        case core::BinaryOp::kGreaterThanEqual:
            cloned_replacement =
                clone_context.dst->GreaterThanEqual(clone_context.Clone(binary_expr_node->lhs),
                                                    clone_context.Clone(binary_expr_node->rhs));
            break;
        case core::BinaryOp::kShiftLeft:
            cloned_replacement = clone_context.dst->Shl(clone_context.Clone(binary_expr_node->lhs),
                                                        clone_context.Clone(binary_expr_node->rhs));
            break;
        case core::BinaryOp::kShiftRight:
            cloned_replacement = clone_context.dst->Shr(clone_context.Clone(binary_expr_node->lhs),
                                                        clone_context.Clone(binary_expr_node->rhs));
            break;
        case core::BinaryOp::kAdd:
            cloned_replacement = clone_context.dst->Add(clone_context.Clone(binary_expr_node->lhs),
                                                        clone_context.Clone(binary_expr_node->rhs));
            break;
        case core::BinaryOp::kSubtract:
            cloned_replacement = clone_context.dst->Sub(clone_context.Clone(binary_expr_node->lhs),
                                                        clone_context.Clone(binary_expr_node->rhs));
            break;
        case core::BinaryOp::kMultiply:
            cloned_replacement = clone_context.dst->Mul(clone_context.Clone(binary_expr_node->lhs),
                                                        clone_context.Clone(binary_expr_node->rhs));
            break;
        case core::BinaryOp::kDivide:
            cloned_replacement = clone_context.dst->Div(clone_context.Clone(binary_expr_node->lhs),
                                                        clone_context.Clone(binary_expr_node->rhs));
            break;
        case core::BinaryOp::kModulo:
            cloned_replacement = clone_context.dst->Mod(clone_context.Clone(binary_expr_node->lhs),
                                                        clone_context.Clone(binary_expr_node->rhs));
            break;
    }
    // Set things up so that the original binary expression will be replaced with
    // its clone, and update the id mapping.
    clone_context.Replace(binary_expr_node, cloned_replacement);
    new_node_id_map->Add(cloned_replacement, message_.binary_expr_id());
}

protobufs::Mutation MutationChangeBinaryOperator::ToMessage() const {
    protobufs::Mutation mutation;
    *mutation.mutable_change_binary_operator() = message_;
    return mutation;
}

}  // namespace tint::fuzzers::ast_fuzzer
