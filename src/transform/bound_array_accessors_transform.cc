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

BoundArrayAccessorsTransform::BoundArrayAccessorsTransform(Context* ctx,
                                                           ast::Module* mod)
    : Transformer(ctx, mod) {}

BoundArrayAccessorsTransform::~BoundArrayAccessorsTransform() = default;

bool BoundArrayAccessorsTransform::Run() {
  // We skip over global variables as the constructor for a global must be a
  // constant expression. There can't be any array accessors as per the current
  // grammar.

  for (auto* func : mod_->functions()) {
    scope_stack_.push_scope();
    if (!ProcessStatement(func->body())) {
      return false;
    }
    scope_stack_.pop_scope();
  }
  return true;
}

bool BoundArrayAccessorsTransform::ProcessStatement(ast::Statement* stmt) {
  if (auto* as = stmt->As<ast::AssignmentStatement>()) {
    return ProcessExpression(as->lhs()) && ProcessExpression(as->rhs());
  } else if (auto* block = stmt->As<ast::BlockStatement>()) {
    for (auto* s : *block) {
      if (!ProcessStatement(s)) {
        return false;
      }
    }
  } else if (stmt->Is<ast::BreakStatement>()) {
    /* nop */
  } else if (auto* call = stmt->As<ast::CallStatement>()) {
    return ProcessExpression(call->expr());
  } else if (auto* kase = stmt->As<ast::CaseStatement>()) {
    return ProcessStatement(kase->body());
  } else if (stmt->Is<ast::ContinueStatement>()) {
    /* nop */
  } else if (stmt->Is<ast::DiscardStatement>()) {
    /* nop */
  } else if (auto* e = stmt->As<ast::ElseStatement>()) {
    return ProcessExpression(e->condition()) && ProcessStatement(e->body());
  } else if (stmt->Is<ast::FallthroughStatement>()) {
    /* nop */
  } else if (auto* i = stmt->As<ast::IfStatement>()) {
    if (!ProcessExpression(i->condition()) || !ProcessStatement(i->body())) {
      return false;
    }
    for (auto* s : i->else_statements()) {
      if (!ProcessStatement(s)) {
        return false;
      }
    }
  } else if (auto* l = stmt->As<ast::LoopStatement>()) {
    if (l->has_continuing() && !ProcessStatement(l->continuing())) {
      return false;
    }
    return ProcessStatement(l->body());
  } else if (auto* r = stmt->As<ast::ReturnStatement>()) {
    if (r->has_value()) {
      return ProcessExpression(r->value());
    }
  } else if (auto* s = stmt->As<ast::SwitchStatement>()) {
    if (!ProcessExpression(s->condition())) {
      return false;
    }

    for (auto* c : s->body()) {
      if (!ProcessStatement(c)) {
        return false;
      }
    }
  } else if (auto* vd = stmt->As<ast::VariableDeclStatement>()) {
    auto* v = vd->variable();
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
  if (auto* array = expr->As<ast::ArrayAccessorExpression>()) {
    return ProcessArrayAccessor(array);
  } else if (auto* bitcast = expr->As<ast::BitcastExpression>()) {
    return ProcessExpression(bitcast->expr());
  } else if (auto* call = expr->As<ast::CallExpression>()) {
    if (!ProcessExpression(call->func())) {
      return false;
    }
    for (auto* e : call->params()) {
      if (!ProcessExpression(e)) {
        return false;
      }
    }
  } else if (expr->Is<ast::IdentifierExpression>()) {
    /* nop */
  } else if (expr->Is<ast::ConstructorExpression>()) {
    if (auto* c = expr->As<ast::TypeConstructorExpression>()) {
      for (auto* e : c->values()) {
        if (!ProcessExpression(e)) {
          return false;
        }
      }
    }
  } else if (auto* m = expr->As<ast::MemberAccessorExpression>()) {
    return ProcessExpression(m->structure()) && ProcessExpression(m->member());
  } else if (auto* b = expr->As<ast::BinaryExpression>()) {
    return ProcessExpression(b->lhs()) && ProcessExpression(b->rhs());
  } else if (auto* u = expr->As<ast::UnaryOpExpression>()) {
    return ProcessExpression(u->expr());
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

  auto* ret_type = expr->array()->result_type()->UnwrapAll();
  if (!ret_type->Is<ast::type::ArrayType>() &&
      !ret_type->Is<ast::type::MatrixType>() &&
      !ret_type->Is<ast::type::VectorType>()) {
    return true;
  }

  if (ret_type->Is<ast::type::VectorType>() ||
      ret_type->Is<ast::type::ArrayType>()) {
    uint32_t size = ret_type->Is<ast::type::VectorType>()
                        ? ret_type->As<ast::type::VectorType>()->size()
                        : ret_type->As<ast::type::ArrayType>()->size();
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
    uint32_t size = ret_type->As<ast::type::MatrixType>()->columns();
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
  if (auto* c = expr->idx_expr()->As<ast::ScalarConstructorExpression>()) {
    auto* lit = c->literal();
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
    auto* u32 = mod_->create<ast::type::U32Type>();

    ast::ExpressionList cast_expr;
    cast_expr.push_back(expr->idx_expr());

    ast::ExpressionList params;
    params.push_back(create<ast::TypeConstructorExpression>(u32, cast_expr));
    params.push_back(create<ast::ScalarConstructorExpression>(
        create<ast::UintLiteral>(u32, size - 1)));

    auto* call_expr = create<ast::CallExpression>(
        create<ast::IdentifierExpression>("min"), std::move(params));
    call_expr->set_result_type(u32);

    expr->set_idx_expr(call_expr);
  }
  return true;
}

}  // namespace transform
}  // namespace tint
