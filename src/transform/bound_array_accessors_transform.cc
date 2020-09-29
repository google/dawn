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

#include "src/transform/bound_array_accessors_transform.h"

#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/block_statement.h"
#include "src/ast/call_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/case_statement.h"
#include "src/ast/else_statement.h"
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
#include "src/type_manager.h"

namespace tint {
namespace transform {

BoundArrayAccessorsTransform::BoundArrayAccessorsTransform(Context* ctx,
                                                           ast::Module* mod)
    : Transformer(ctx, mod) {}

BoundArrayAccessorsTransform::~BoundArrayAccessorsTransform() = default;

bool BoundArrayAccessorsTransform::Run() {
  // We skip over global variables as the constructor for a global must be a
  // constant expression. There can't be any array accessors as per the current
  // grammar.

  for (auto& func : mod_->functions()) {
    scope_stack_.push_scope();
    if (!ProcessStatement(func->body())) {
      return false;
    }
    scope_stack_.pop_scope();
  }
  return true;
}

bool BoundArrayAccessorsTransform::ProcessStatement(ast::Statement* stmt) {
  if (stmt->IsAssign()) {
    auto* as = stmt->AsAssign();
    return ProcessExpression(as->lhs()) && ProcessExpression(as->rhs());
  } else if (stmt->IsBlock()) {
    for (auto& s : *(stmt->AsBlock())) {
      if (!ProcessStatement(s.get())) {
        return false;
      }
    }
  } else if (stmt->IsBreak()) {
    /* nop */
  } else if (stmt->IsCall()) {
    return ProcessExpression(stmt->AsCall()->expr());
  } else if (stmt->IsCase()) {
    return ProcessStatement(stmt->AsCase()->body());
  } else if (stmt->IsContinue()) {
    /* nop */
  } else if (stmt->IsDiscard()) {
    /* nop */
  } else if (stmt->IsElse()) {
    auto* e = stmt->AsElse();
    return ProcessExpression(e->condition()) && ProcessStatement(e->body());
  } else if (stmt->IsFallthrough()) {
    /* nop */
  } else if (stmt->IsIf()) {
    auto* e = stmt->AsIf();
    if (!ProcessExpression(e->condition()) || !ProcessStatement(e->body())) {
      return false;
    }
    for (auto& s : e->else_statements()) {
      if (!ProcessStatement(s.get())) {
        return false;
      }
    }
  } else if (stmt->IsLoop()) {
    auto* l = stmt->AsLoop();
    if (l->has_continuing() && !ProcessStatement(l->continuing())) {
      return false;
    }
    return ProcessStatement(l->body());
  } else if (stmt->IsReturn()) {
    if (stmt->AsReturn()->has_value()) {
      return ProcessExpression(stmt->AsReturn()->value());
    }
  } else if (stmt->IsSwitch()) {
    auto* s = stmt->AsSwitch();
    if (!ProcessExpression(s->condition())) {
      return false;
    }

    for (auto& c : s->body()) {
      if (!ProcessStatement(c.get())) {
        return false;
      }
    }
  } else if (stmt->IsVariableDecl()) {
    auto* v = stmt->AsVariableDecl()->variable();
    if (v->has_constructor() && !ProcessExpression(v->constructor())) {
      return false;
    }
    scope_stack_.set(v->name(), v);
  } else {
    error_ = "unknown statement in bound array accessors transform";
    return false;
  }
  return true;
}

bool BoundArrayAccessorsTransform::ProcessExpression(ast::Expression* expr) {
  if (expr->IsArrayAccessor()) {
    return ProcessArrayAccessor(expr->AsArrayAccessor());
  } else if (expr->IsBitcast()) {
    return ProcessExpression(expr->AsBitcast()->expr());
  } else if (expr->IsCall()) {
    auto* c = expr->AsCall();
    if (!ProcessExpression(c->func())) {
      return false;
    }
    for (auto& e : c->params()) {
      if (!ProcessExpression(e.get())) {
        return false;
      }
    }
  } else if (expr->IsIdentifier()) {
    /* nop */
  } else if (expr->IsConstructor()) {
    auto* c = expr->AsConstructor();
    if (c->IsTypeConstructor()) {
      for (auto& e : c->AsTypeConstructor()->values()) {
        if (!ProcessExpression(e.get())) {
          return false;
        }
      }
    }
  } else if (expr->IsMemberAccessor()) {
    auto* m = expr->AsMemberAccessor();
    return ProcessExpression(m->structure()) && ProcessExpression(m->member());
  } else if (expr->IsBinary()) {
    auto* b = expr->AsBinary();
    return ProcessExpression(b->lhs()) && ProcessExpression(b->rhs());
  } else if (expr->IsUnaryOp()) {
    return ProcessExpression(expr->AsUnaryOp()->expr());
  } else {
    error_ = "unknown statement in bound array accessors transform";
    return false;
  }
  return true;
}

bool BoundArrayAccessorsTransform::ProcessArrayAccessor(
    ast::ArrayAccessorExpression* expr) {
  if (!ProcessExpression(expr->array()) ||
      !ProcessExpression(expr->idx_expr())) {
    return false;
  }

  auto* ret_type = expr->array()->result_type()->UnwrapAliasPtrAlias();
  if (!ret_type->IsArray() && !ret_type->IsMatrix() && !ret_type->IsVector()) {
    return true;
  }

  if (ret_type->IsVector() || ret_type->IsArray()) {
    uint32_t size = ret_type->IsVector() ? ret_type->AsVector()->size()
                                         : ret_type->AsArray()->size();
    if (size == 0) {
      error_ = "invalid 0 size for array or vector";
      return false;
    }

    if (!ProcessAccessExpression(expr, size)) {
      return false;
    }
  } else {
    // The row accessor would have been an embedded array accessor and already
    // handled, so we just need to do columns here.
    uint32_t size = ret_type->AsMatrix()->columns();
    if (!ProcessAccessExpression(expr, size)) {
      return false;
    }
  }
  return true;
}

bool BoundArrayAccessorsTransform::ProcessAccessExpression(
    ast::ArrayAccessorExpression* expr,
    uint32_t size) {
  // Scalar constructor we can re-write the value to be within bounds.
  if (expr->idx_expr()->IsConstructor() &&
      expr->idx_expr()->AsConstructor()->IsScalarConstructor()) {
    auto* lit =
        expr->idx_expr()->AsConstructor()->AsScalarConstructor()->literal();
    if (lit->IsSint()) {
      int32_t val = lit->AsSint()->value();
      if (val < 0) {
        val = 0;
      } else if (val >= int32_t(size)) {
        val = int32_t(size) - 1;
      }
      lit->AsSint()->set_value(val);
    } else if (lit->IsUint()) {
      uint32_t val = lit->AsUint()->value();
      if (val >= size - 1) {
        val = size - 1;
      }
      lit->AsUint()->set_value(val);
    } else {
      error_ = "unknown scalar constructor type for accessor";
      return false;
    }
  } else {
    auto* u32 = ctx_->type_mgr().Get(std::make_unique<ast::type::U32Type>());

    ast::ExpressionList params;
    params.push_back(expr->take_idx_expr());
    params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
        std::make_unique<ast::UintLiteral>(u32, 0)));
    params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
        std::make_unique<ast::UintLiteral>(u32, size - 1)));

    auto call_expr = std::make_unique<ast::CallExpression>(
        std::make_unique<ast::IdentifierExpression>("clamp"),
        std::move(params));
    call_expr->set_result_type(u32);

    expr->set_idx_expr(std::move(call_expr));
  }
  return true;
}

}  // namespace transform
}  // namespace tint
