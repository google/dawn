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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/change_binary_operator.h"

#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/type/bool.h"
#include "src/tint/type/reference.h"

namespace tint::fuzzers::ast_fuzzer {

namespace {

bool IsSuitableForShift(const type::Type* lhs_type, const type::Type* rhs_type) {
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

bool CanReplaceAddSubtractWith(const type::Type* lhs_type,
                               const type::Type* rhs_type,
                               ast::BinaryOp new_operator) {
    // The program is assumed to be well-typed, so this method determines when
    // 'new_operator' can be used as a type-preserving replacement in an '+' or
    // '-' expression.
    switch (new_operator) {
        case ast::BinaryOp::kAdd:
        case ast::BinaryOp::kSubtract:
            // '+' and '-' are fully type compatible.
            return true;
        case ast::BinaryOp::kAnd:
        case ast::BinaryOp::kOr:
        case ast::BinaryOp::kXor:
            // These operators do not have a mixed vector-scalar form, and only work
            // on integer types.
            return lhs_type == rhs_type && lhs_type->is_integer_scalar_or_vector();
        case ast::BinaryOp::kMultiply:
            // '+' and '*' are largely type-compatible, but for matrices they are only
            // type-compatible if the matrices are square.
            return !lhs_type->is_float_matrix() || lhs_type->is_square_float_matrix();
        case ast::BinaryOp::kDivide:
        case ast::BinaryOp::kModulo:
            // '/' is not defined for matrices.
            return lhs_type->is_numeric_scalar_or_vector() &&
                   rhs_type->is_numeric_scalar_or_vector();
        case ast::BinaryOp::kShiftLeft:
        case ast::BinaryOp::kShiftRight:
            return IsSuitableForShift(lhs_type, rhs_type);
        default:
            return false;
    }
}

bool CanReplaceMultiplyWith(const type::Type* lhs_type,
                            const type::Type* rhs_type,
                            ast::BinaryOp new_operator) {
    // The program is assumed to be well-typed, so this method determines when
    // 'new_operator' can be used as a type-preserving replacement in a '*'
    // expression.
    switch (new_operator) {
        case ast::BinaryOp::kMultiply:
            return true;
        case ast::BinaryOp::kAdd:
        case ast::BinaryOp::kSubtract:
            // '*' is type-compatible with '+' and '-' for square matrices, and for
            // numeric scalars/vectors.
            if (lhs_type->is_square_float_matrix() && rhs_type->is_square_float_matrix()) {
                return true;
            }
            return lhs_type->is_numeric_scalar_or_vector() &&
                   rhs_type->is_numeric_scalar_or_vector();
        case ast::BinaryOp::kAnd:
        case ast::BinaryOp::kOr:
        case ast::BinaryOp::kXor:
            // These operators require homogeneous integer types.
            return lhs_type == rhs_type && lhs_type->is_integer_scalar_or_vector();
        case ast::BinaryOp::kDivide:
        case ast::BinaryOp::kModulo:
            // '/' is not defined for matrices.
            return lhs_type->is_numeric_scalar_or_vector() &&
                   rhs_type->is_numeric_scalar_or_vector();
        case ast::BinaryOp::kShiftLeft:
        case ast::BinaryOp::kShiftRight:
            return IsSuitableForShift(lhs_type, rhs_type);
        default:
            return false;
    }
}

bool CanReplaceDivideOrModuloWith(const type::Type* lhs_type,
                                  const type::Type* rhs_type,
                                  ast::BinaryOp new_operator) {
    // The program is assumed to be well-typed, so this method determines when
    // 'new_operator' can be used as a type-preserving replacement in a '/'
    // expression.
    switch (new_operator) {
        case ast::BinaryOp::kAdd:
        case ast::BinaryOp::kSubtract:
        case ast::BinaryOp::kMultiply:
        case ast::BinaryOp::kDivide:
        case ast::BinaryOp::kModulo:
            // These operators work in all contexts where '/' works.
            return true;
        case ast::BinaryOp::kAnd:
        case ast::BinaryOp::kOr:
        case ast::BinaryOp::kXor:
            // These operators require homogeneous integer types.
            return lhs_type == rhs_type && lhs_type->is_integer_scalar_or_vector();
        case ast::BinaryOp::kShiftLeft:
        case ast::BinaryOp::kShiftRight:
            return IsSuitableForShift(lhs_type, rhs_type);
        default:
            return false;
    }
}

bool CanReplaceLogicalAndLogicalOrWith(ast::BinaryOp new_operator) {
    switch (new_operator) {
        case ast::BinaryOp::kLogicalAnd:
        case ast::BinaryOp::kLogicalOr:
        case ast::BinaryOp::kAnd:
        case ast::BinaryOp::kOr:
        case ast::BinaryOp::kEqual:
        case ast::BinaryOp::kNotEqual:
            // These operators all work whenever '&&' and '||' work.
            return true;
        default:
            return false;
    }
}

bool CanReplaceAndOrWith(const type::Type* lhs_type,
                         const type::Type* rhs_type,
                         ast::BinaryOp new_operator) {
    switch (new_operator) {
        case ast::BinaryOp::kAnd:
        case ast::BinaryOp::kOr:
            // '&' and '|' work in all the same contexts.
            return true;
        case ast::BinaryOp::kAdd:
        case ast::BinaryOp::kSubtract:
        case ast::BinaryOp::kMultiply:
        case ast::BinaryOp::kDivide:
        case ast::BinaryOp::kModulo:
        case ast::BinaryOp::kXor:
            // '&' and '|' can be applied to booleans. In all other contexts,
            // integer numeric operators work.
            return !lhs_type->is_bool_scalar_or_vector();
        case ast::BinaryOp::kShiftLeft:
        case ast::BinaryOp::kShiftRight:
            return IsSuitableForShift(lhs_type, rhs_type);
        case ast::BinaryOp::kLogicalAnd:
        case ast::BinaryOp::kLogicalOr:
            // '&' and '|' can be applied to booleans, and for boolean scalar
            // scalar contexts, their logical counterparts work.
            return lhs_type->Is<type::Bool>();
        case ast::BinaryOp::kEqual:
        case ast::BinaryOp::kNotEqual:
            // '&' and '|' can be applied to booleans, and in these contexts equality
            // comparison operators also work.
            return lhs_type->is_bool_scalar_or_vector();
        default:
            return false;
    }
}

bool CanReplaceXorWith(const type::Type* lhs_type,
                       const type::Type* rhs_type,
                       ast::BinaryOp new_operator) {
    switch (new_operator) {
        case ast::BinaryOp::kAdd:
        case ast::BinaryOp::kSubtract:
        case ast::BinaryOp::kMultiply:
        case ast::BinaryOp::kDivide:
        case ast::BinaryOp::kModulo:
        case ast::BinaryOp::kAnd:
        case ast::BinaryOp::kOr:
        case ast::BinaryOp::kXor:
            // '^' only works on integer types, and in any such context, all other
            // integer operators also work.
            return true;
        case ast::BinaryOp::kShiftLeft:
        case ast::BinaryOp::kShiftRight:
            return IsSuitableForShift(lhs_type, rhs_type);
        default:
            return false;
    }
}

bool CanReplaceShiftLeftShiftRightWith(const type::Type* lhs_type,
                                       const type::Type* rhs_type,
                                       ast::BinaryOp new_operator) {
    switch (new_operator) {
        case ast::BinaryOp::kShiftLeft:
        case ast::BinaryOp::kShiftRight:
            // These operators are type-compatible.
            return true;
        case ast::BinaryOp::kAdd:
        case ast::BinaryOp::kSubtract:
        case ast::BinaryOp::kMultiply:
        case ast::BinaryOp::kDivide:
        case ast::BinaryOp::kModulo:
        case ast::BinaryOp::kAnd:
        case ast::BinaryOp::kOr:
        case ast::BinaryOp::kXor:
            // Shift operators allow mixing of signed and unsigned arguments, but in
            // the case where the arguments are homogeneous, they are type-compatible
            // with other numeric operators.
            return lhs_type == rhs_type;
        default:
            return false;
    }
}

bool CanReplaceEqualNotEqualWith(const type::Type* lhs_type, ast::BinaryOp new_operator) {
    switch (new_operator) {
        case ast::BinaryOp::kEqual:
        case ast::BinaryOp::kNotEqual:
            // These operators are type-compatible.
            return true;
        case ast::BinaryOp::kLessThan:
        case ast::BinaryOp::kLessThanEqual:
        case ast::BinaryOp::kGreaterThan:
        case ast::BinaryOp::kGreaterThanEqual:
            // An equality comparison between numeric types can be changed to an
            // ordered comparison.
            return lhs_type->is_numeric_scalar_or_vector();
        case ast::BinaryOp::kLogicalAnd:
        case ast::BinaryOp::kLogicalOr:
            // An equality comparison between boolean scalars can be turned into a
            // logical operation.
            return lhs_type->Is<type::Bool>();
        case ast::BinaryOp::kAnd:
        case ast::BinaryOp::kOr:
            // An equality comparison between boolean scalars or vectors can be turned
            // into a component-wise non-short-circuit logical operation.
            return lhs_type->is_bool_scalar_or_vector();
        default:
            return false;
    }
}

bool CanReplaceLessThanLessThanEqualGreaterThanGreaterThanEqualWith(ast::BinaryOp new_operator) {
    switch (new_operator) {
        case ast::BinaryOp::kEqual:
        case ast::BinaryOp::kNotEqual:
        case ast::BinaryOp::kLessThan:
        case ast::BinaryOp::kLessThanEqual:
        case ast::BinaryOp::kGreaterThan:
        case ast::BinaryOp::kGreaterThanEqual:
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
                                                           ast::BinaryOp new_operator) {
    message_.set_binary_expr_id(binary_expr_id);
    message_.set_new_operator(static_cast<uint32_t>(new_operator));
}

bool MutationChangeBinaryOperator::CanReplaceBinaryOperator(
    const Program& program,
    const ast::BinaryExpression& binary_expr,
    ast::BinaryOp new_operator) {
    if (new_operator == binary_expr.op) {
        // An operator should not be replaced with itself, as this would be a no-op.
        return false;
    }

    // Get the types of the operators.
    const auto* lhs_type = program.Sem().GetVal(binary_expr.lhs)->Type();
    const auto* rhs_type = program.Sem().GetVal(binary_expr.rhs)->Type();

    // If these are reference types, unwrap them to get the pointee type.
    const type::Type* lhs_basic_type =
        lhs_type->Is<type::Reference>() ? lhs_type->As<type::Reference>()->StoreType() : lhs_type;
    const type::Type* rhs_basic_type =
        rhs_type->Is<type::Reference>() ? rhs_type->As<type::Reference>()->StoreType() : rhs_type;

    switch (binary_expr.op) {
        case ast::BinaryOp::kAdd:
        case ast::BinaryOp::kSubtract:
            return CanReplaceAddSubtractWith(lhs_basic_type, rhs_basic_type, new_operator);
        case ast::BinaryOp::kMultiply:
            return CanReplaceMultiplyWith(lhs_basic_type, rhs_basic_type, new_operator);
        case ast::BinaryOp::kDivide:
        case ast::BinaryOp::kModulo:
            return CanReplaceDivideOrModuloWith(lhs_basic_type, rhs_basic_type, new_operator);
        case ast::BinaryOp::kAnd:
        case ast::BinaryOp::kOr:
            return CanReplaceAndOrWith(lhs_basic_type, rhs_basic_type, new_operator);
        case ast::BinaryOp::kXor:
            return CanReplaceXorWith(lhs_basic_type, rhs_basic_type, new_operator);
        case ast::BinaryOp::kShiftLeft:
        case ast::BinaryOp::kShiftRight:
            return CanReplaceShiftLeftShiftRightWith(lhs_basic_type, rhs_basic_type, new_operator);
        case ast::BinaryOp::kLogicalAnd:
        case ast::BinaryOp::kLogicalOr:
            return CanReplaceLogicalAndLogicalOrWith(new_operator);
        case ast::BinaryOp::kEqual:
        case ast::BinaryOp::kNotEqual:
            return CanReplaceEqualNotEqualWith(lhs_basic_type, new_operator);
        case ast::BinaryOp::kLessThan:
        case ast::BinaryOp::kLessThanEqual:
        case ast::BinaryOp::kGreaterThan:
        case ast::BinaryOp::kGreaterThanEqual:
        case ast::BinaryOp::kNone:
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
    const auto new_operator = static_cast<ast::BinaryOp>(message_.new_operator());
    return CanReplaceBinaryOperator(program, *binary_expr_node, new_operator);
}

void MutationChangeBinaryOperator::Apply(const NodeIdMap& node_id_map,
                                         CloneContext* clone_context,
                                         NodeIdMap* new_node_id_map) const {
    // Get the node whose operator is to be replaced.
    const auto* binary_expr_node =
        As<ast::BinaryExpression>(node_id_map.GetNode(message_.binary_expr_id()));

    // Clone the binary expression, with the appropriate new operator.
    const ast::BinaryExpression* cloned_replacement;
    switch (static_cast<ast::BinaryOp>(message_.new_operator())) {
        case ast::BinaryOp::kAnd:
            cloned_replacement =
                clone_context->dst->And(clone_context->Clone(binary_expr_node->lhs),
                                        clone_context->Clone(binary_expr_node->rhs));
            break;
        case ast::BinaryOp::kOr:
            cloned_replacement =
                clone_context->dst->Or(clone_context->Clone(binary_expr_node->lhs),
                                       clone_context->Clone(binary_expr_node->rhs));
            break;
        case ast::BinaryOp::kXor:
            cloned_replacement =
                clone_context->dst->Xor(clone_context->Clone(binary_expr_node->lhs),
                                        clone_context->Clone(binary_expr_node->rhs));
            break;
        case ast::BinaryOp::kLogicalAnd:
            cloned_replacement =
                clone_context->dst->LogicalAnd(clone_context->Clone(binary_expr_node->lhs),
                                               clone_context->Clone(binary_expr_node->rhs));
            break;
        case ast::BinaryOp::kLogicalOr:
            cloned_replacement =
                clone_context->dst->LogicalOr(clone_context->Clone(binary_expr_node->lhs),
                                              clone_context->Clone(binary_expr_node->rhs));
            break;
        case ast::BinaryOp::kEqual:
            cloned_replacement =
                clone_context->dst->Equal(clone_context->Clone(binary_expr_node->lhs),
                                          clone_context->Clone(binary_expr_node->rhs));
            break;
        case ast::BinaryOp::kNotEqual:
            cloned_replacement =
                clone_context->dst->NotEqual(clone_context->Clone(binary_expr_node->lhs),
                                             clone_context->Clone(binary_expr_node->rhs));
            break;
        case ast::BinaryOp::kLessThan:
            cloned_replacement =
                clone_context->dst->LessThan(clone_context->Clone(binary_expr_node->lhs),
                                             clone_context->Clone(binary_expr_node->rhs));
            break;
        case ast::BinaryOp::kGreaterThan:
            cloned_replacement =
                clone_context->dst->GreaterThan(clone_context->Clone(binary_expr_node->lhs),
                                                clone_context->Clone(binary_expr_node->rhs));
            break;
        case ast::BinaryOp::kLessThanEqual:
            cloned_replacement =
                clone_context->dst->LessThanEqual(clone_context->Clone(binary_expr_node->lhs),
                                                  clone_context->Clone(binary_expr_node->rhs));
            break;
        case ast::BinaryOp::kGreaterThanEqual:
            cloned_replacement =
                clone_context->dst->GreaterThanEqual(clone_context->Clone(binary_expr_node->lhs),
                                                     clone_context->Clone(binary_expr_node->rhs));
            break;
        case ast::BinaryOp::kShiftLeft:
            cloned_replacement =
                clone_context->dst->Shl(clone_context->Clone(binary_expr_node->lhs),
                                        clone_context->Clone(binary_expr_node->rhs));
            break;
        case ast::BinaryOp::kShiftRight:
            cloned_replacement =
                clone_context->dst->Shr(clone_context->Clone(binary_expr_node->lhs),
                                        clone_context->Clone(binary_expr_node->rhs));
            break;
        case ast::BinaryOp::kAdd:
            cloned_replacement =
                clone_context->dst->Add(clone_context->Clone(binary_expr_node->lhs),
                                        clone_context->Clone(binary_expr_node->rhs));
            break;
        case ast::BinaryOp::kSubtract:
            cloned_replacement =
                clone_context->dst->Sub(clone_context->Clone(binary_expr_node->lhs),
                                        clone_context->Clone(binary_expr_node->rhs));
            break;
        case ast::BinaryOp::kMultiply:
            cloned_replacement =
                clone_context->dst->Mul(clone_context->Clone(binary_expr_node->lhs),
                                        clone_context->Clone(binary_expr_node->rhs));
            break;
        case ast::BinaryOp::kDivide:
            cloned_replacement =
                clone_context->dst->Div(clone_context->Clone(binary_expr_node->lhs),
                                        clone_context->Clone(binary_expr_node->rhs));
            break;
        case ast::BinaryOp::kModulo:
            cloned_replacement =
                clone_context->dst->Mod(clone_context->Clone(binary_expr_node->lhs),
                                        clone_context->Clone(binary_expr_node->rhs));
            break;
        case ast::BinaryOp::kNone:
            cloned_replacement = nullptr;
            assert(false && "Unreachable");
    }
    // Set things up so that the original binary expression will be replaced with
    // its clone, and update the id mapping.
    clone_context->Replace(binary_expr_node, cloned_replacement);
    new_node_id_map->Add(cloned_replacement, message_.binary_expr_id());
}

protobufs::Mutation MutationChangeBinaryOperator::ToMessage() const {
    protobufs::Mutation mutation;
    *mutation.mutable_change_binary_operator() = message_;
    return mutation;
}

}  // namespace tint::fuzzers::ast_fuzzer
