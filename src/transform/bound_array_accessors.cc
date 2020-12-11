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

#include "src/transform/bound_array_accessors.h"

#include <memory>
#include <utility>

#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/block_statement.h"
#include "src/ast/break_statement.h"
#include "src/ast/call_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/case_statement.h"
#include "src/ast/clone_context.h"
#include "src/ast/continue_statement.h"
#include "src/ast/discard_statement.h"
#include "src/ast/else_statement.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/if_statement.h"
#include "src/ast/loop_statement.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/switch_statement.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/uint_literal.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"

namespace tint {
namespace transform {

BoundArrayAccessors::BoundArrayAccessors() = default;
BoundArrayAccessors::~BoundArrayAccessors() = default;

Transform::Output BoundArrayAccessors::Run(ast::Module* mod) {
  Output out;
  ast::CloneContext ctx(&out.module);
  ctx.ReplaceAll([&](ast::ArrayAccessorExpression* expr) {
    return Transform(expr, &ctx, &out.diagnostics);
  });
  mod->Clone(&ctx);
  return out;
}

ast::ArrayAccessorExpression* BoundArrayAccessors::Transform(
    ast::ArrayAccessorExpression* expr,
    ast::CloneContext* ctx,
    diag::List* diags) {
  auto* ret_type = expr->array()->result_type()->UnwrapAll();
  if (!ret_type->Is<ast::type::Array>() && !ret_type->Is<ast::type::Matrix>() &&
      !ret_type->Is<ast::type::Vector>()) {
    return nullptr;
  }

  uint32_t size = 0;
  if (ret_type->Is<ast::type::Vector>() || ret_type->Is<ast::type::Array>()) {
    size = ret_type->Is<ast::type::Vector>()
               ? ret_type->As<ast::type::Vector>()->size()
               : ret_type->As<ast::type::Array>()->size();
    if (size == 0) {
      diag::Diagnostic err;
      err.severity = diag::Severity::Error;
      err.message = "invalid 0 size for array or vector";
      err.source = expr->source();
      diags->add(std::move(err));
      return nullptr;
    }

  } else {
    // The row accessor would have been an embedded array accessor and already
    // handled, so we just need to do columns here.
    size = ret_type->As<ast::type::Matrix>()->columns();
  }

  ast::Expression* idx_expr = nullptr;

  // Scalar constructor we can re-write the value to be within bounds.
  if (auto* c = expr->idx_expr()->As<ast::ScalarConstructorExpression>()) {
    auto* lit = c->literal();
    if (auto* sint = lit->As<ast::SintLiteral>()) {
      int32_t val = sint->value();
      if (val < 0) {
        val = 0;
      } else if (val >= int32_t(size)) {
        val = int32_t(size) - 1;
      }
      lit = ctx->mod->create<ast::SintLiteral>(ctx->Clone(sint->type()), val);
    } else if (auto* uint = lit->As<ast::UintLiteral>()) {
      uint32_t val = uint->value();
      if (val >= size - 1) {
        val = size - 1;
      }
      lit = ctx->mod->create<ast::UintLiteral>(ctx->Clone(uint->type()), val);
    } else {
      diag::Diagnostic err;
      err.severity = diag::Severity::Error;
      err.message = "unknown scalar constructor type for accessor";
      err.source = expr->source();
      diags->add(std::move(err));
      return nullptr;
    }
    idx_expr =
        ctx->mod->create<ast::ScalarConstructorExpression>(c->source(), lit);
  } else {
    auto* u32 = ctx->mod->create<ast::type::U32>();

    ast::ExpressionList cast_expr;
    cast_expr.push_back(ctx->Clone(expr->idx_expr()));

    ast::ExpressionList params;
    params.push_back(
        ctx->mod->create<ast::TypeConstructorExpression>(u32, cast_expr));
    params.push_back(ctx->mod->create<ast::ScalarConstructorExpression>(
        ctx->mod->create<ast::UintLiteral>(u32, size - 1)));

    auto* call_expr = ctx->mod->create<ast::CallExpression>(
        ctx->mod->create<ast::IdentifierExpression>(
            ctx->mod->RegisterSymbol("min"), "min"),
        std::move(params));
    call_expr->set_result_type(u32);

    idx_expr = call_expr;
  }

  auto* arr = ctx->Clone(expr->array());
  return ctx->mod->create<ast::ArrayAccessorExpression>(arr, idx_expr);
}

}  // namespace transform
}  // namespace tint
