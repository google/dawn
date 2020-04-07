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

#include "src/type_determiner.h"

#include "src/ast/assignment_statement.h"
#include "src/ast/break_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/type_constructor_expression.h"

namespace tint {

TypeDeterminer::TypeDeterminer(Context* ctx) : ctx_(*ctx) {
  // TODO(dsinclair): Temporary usage to avoid compiler warning
  static_cast<void>(ctx_.type_mgr());
}

TypeDeterminer::~TypeDeterminer() = default;

bool TypeDeterminer::Determine(ast::Module* mod) {
  for (const auto& var : mod->global_variables()) {
    variable_stack_.set_global(var->name(), var.get());
  }

  for (const auto& func : mod->functions()) {
    name_to_function_[func->name()] = func.get();
  }

  if (!DetermineFunctions(mod->functions())) {
    return false;
  }
  return true;
}

bool TypeDeterminer::DetermineFunctions(const ast::FunctionList& funcs) {
  for (const auto& func : funcs) {
    if (!DetermineFunction(func.get())) {
      return false;
    }
  }
  return true;
}

bool TypeDeterminer::DetermineFunction(ast::Function* func) {
  variable_stack_.push_scope();
  if (!DetermineResultType(func->body())) {
    return false;
  }
  variable_stack_.pop_scope();

  return true;
}

bool TypeDeterminer::DetermineResultType(const ast::StatementList& stmts) {
  for (const auto& stmt : stmts) {
    if (!DetermineResultType(stmt.get())) {
      return false;
    }
  }
  return true;
}

bool TypeDeterminer::DetermineResultType(ast::Statement* stmt) {
  if (stmt->IsAssign()) {
    auto a = stmt->AsAssign();
    return DetermineResultType(a->lhs()) && DetermineResultType(a->rhs());
  }
  if (stmt->IsBreak()) {
    auto b = stmt->AsBreak();
    return DetermineResultType(b->conditional());
  }

  error_ = "unknown statement type for type determination";
  return false;
}

bool TypeDeterminer::DetermineResultType(ast::Expression* expr) {
  // This is blindly called above, so in some cases the expression won't exist.
  if (!expr) {
    return true;
  }

  if (expr->IsConstructor()) {
    return DetermineConstructor(expr->AsConstructor());
  }

  error_ = "unknown expression for type determination";
  return false;
}

bool TypeDeterminer::DetermineConstructor(ast::ConstructorExpression* expr) {
  if (expr->IsTypeConstructor()) {
    expr->set_result_type(expr->AsTypeConstructor()->type());
  } else {
    expr->set_result_type(expr->AsScalarConstructor()->literal()->type());
  }
  return true;
}

}  // namespace tint
