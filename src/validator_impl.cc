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

#include "src/validator_impl.h"
#include "src/ast/call_statement.h"
#include "src/ast/function.h"
#include "src/ast/intrinsic.h"
#include "src/ast/type/void_type.h"
#include "src/ast/variable_decl_statement.h"

namespace tint {

ValidatorImpl::ValidatorImpl() = default;

ValidatorImpl::~ValidatorImpl() = default;

void ValidatorImpl::set_error(const Source& src, const std::string& msg) {
  error_ +=
      std::to_string(src.line) + ":" + std::to_string(src.column) + ": " + msg;
}

bool ValidatorImpl::Validate(const ast::Module* module) {
  if (!module) {
    return false;
  }
  function_stack_.push_scope();
  for (const auto& var : module->global_variables()) {
    if (variable_stack_.has(var->name())) {
      set_error(var->source(),
                "v-0011: redeclared global identifier '" + var->name() + "'");
      return false;
    }
    variable_stack_.set_global(var->name(), var.get());
  }
  if (!CheckImports(module)) {
    return false;
  }
  if (!ValidateFunctions(module->functions())) {
    return false;
  }
  function_stack_.pop_scope();
  return true;
}

bool ValidatorImpl::ValidateFunctions(const ast::FunctionList& funcs) {
  for (const auto& func : funcs) {
    auto* func_ptr = func.get();
    if (function_stack_.has(func_ptr->name())) {
      set_error(func_ptr->source(), "v-0016: function names must be unique '" +
                                        func_ptr->name() + "'");
      return false;
    }

    function_stack_.set(func_ptr->name(), func_ptr);
    current_function_ = func_ptr;
    if (!ValidateFunction(func_ptr)) {
      return false;
    }
    current_function_ = nullptr;
  }
  return true;
}

bool ValidatorImpl::ValidateFunction(const ast::Function* func) {
  variable_stack_.push_scope();

  for (const auto& param : func->params()) {
    variable_stack_.set(param->name(), param.get());
  }
  if (!ValidateStatements(func->body())) {
    return false;
  }
  variable_stack_.pop_scope();

  if (!func->get_last_statement() || !func->get_last_statement()->IsReturn()) {
    set_error(func->source(),
              "v-0002: function must end with a return statement");
    return false;
  }
  return true;
}

bool ValidatorImpl::ValidateReturnStatement(const ast::ReturnStatement* ret) {
  // TODO(sarahM0): update this when this issue resolves:
  // https://github.com/gpuweb/gpuweb/issues/996
  ast::type::Type* func_type = current_function_->return_type();

  ast::type::VoidType void_type;
  auto* ret_type = ret->has_value()
                       ? ret->value()->result_type()->UnwrapAliasPtrAlias()
                       : &void_type;

  if (func_type->type_name() != ret_type->type_name()) {
    set_error(ret->source(),
              "v-000y: return statement type must match its function return "
              "type, returned '" +
                  ret_type->type_name() + "', expected '" +
                  func_type->type_name() + "'");
    return false;
  }

  return true;
}

bool ValidatorImpl::ValidateStatements(const ast::BlockStatement* block) {
  if (!block) {
    return false;
  }
  for (const auto& stmt : *block) {
    if (!ValidateStatement(stmt.get())) {
      return false;
    }
  }
  return true;
}

bool ValidatorImpl::ValidateDeclStatement(
    const ast::VariableDeclStatement* decl) {
  auto name = decl->variable()->name();
  bool is_global = false;
  if (variable_stack_.get(name, nullptr, &is_global)) {
    std::string error_number = "v-0014: ";
    if (is_global) {
      error_number = "v-0013: ";
    }
    set_error(decl->source(),
              error_number + "redeclared identifier '" + name + "'");
    return false;
  }
  variable_stack_.set(name, decl->variable());
  return true;
}

bool ValidatorImpl::ValidateStatement(const ast::Statement* stmt) {
  if (!stmt) {
    return false;
  }
  if (stmt->IsVariableDecl()) {
    auto* v = stmt->AsVariableDecl();
    bool constructor_valid =
        v->variable()->has_constructor()
            ? ValidateExpression(v->variable()->constructor())
            : true;

    return constructor_valid && ValidateDeclStatement(stmt->AsVariableDecl());
  }
  if (stmt->IsAssign()) {
    return ValidateAssign(stmt->AsAssign());
  }
  if (stmt->IsReturn()) {
    return ValidateReturnStatement(stmt->AsReturn());
  }
  if (stmt->IsCall()) {
    return ValidateCallExpr(stmt->AsCall()->expr());
  }
  return true;
}

bool ValidatorImpl::ValidateCallExpr(const ast::CallExpression* expr) {
  if (!expr) {
    // TODO(sarahM0): Here and other Validate.*: figure out whether return false
    // or true
    return false;
  }

  if (expr->func()->IsIdentifier()) {
    auto* ident = expr->func()->AsIdentifier();
    auto func_name = ident->name();
    if (ident->has_path()) {
      // TODO(sarahM0): validate import statements
    } else if (ast::intrinsic::IsIntrinsic(ident->name())) {
      // TODO(sarahM0): validate intrinsics - tied with type-determiner
    } else {
      if (!function_stack_.has(func_name)) {
        set_error(expr->source(),
                  "v-0005: function must be declared before use: '" +
                      func_name + "'");
        return false;
      }
      if (func_name == current_function_->name()) {
        set_error(expr->source(),
                  "v-0004: recursion is not allowed: '" + func_name + "'");
        return false;
      }
    }
  } else {
    set_error(expr->source(), "Invalid function call expression");
    return false;
  }

  return true;
}

bool ValidatorImpl::ValidateAssign(const ast::AssignmentStatement* a) {
  if (!a) {
    return false;
  }
  if (!(ValidateConstant(a))) {
    return false;
  }
  if (!(ValidateExpression(a->lhs()) && ValidateExpression(a->rhs()))) {
    return false;
  }
  if (!ValidateResultTypes(a)) {
    return false;
  }

  return true;
}

bool ValidatorImpl::ValidateConstant(const ast::AssignmentStatement* assign) {
  if (!assign) {
    return false;
  }

  if (assign->lhs()->IsIdentifier()) {
    ast::Variable* var;
    auto* ident = assign->lhs()->AsIdentifier();
    if (variable_stack_.get(ident->name(), &var)) {
      if (var->is_const()) {
        set_error(assign->source(), "v-0021: cannot re-assign a constant: '" +
                                        ident->name() + "'");
        return false;
      }
    }
  }
  return true;
}

bool ValidatorImpl::ValidateResultTypes(const ast::AssignmentStatement* a) {
  if (!a->lhs()->result_type() || !a->rhs()->result_type()) {
    set_error(a->source(), "result_type() is nullptr");
    return false;
  }

  auto* lhs_result_type = a->lhs()->result_type()->UnwrapAliasPtrAlias();
  auto* rhs_result_type = a->rhs()->result_type()->UnwrapAliasPtrAlias();
  if (lhs_result_type != rhs_result_type) {
    // TODO(sarahM0): figur out what should be the error number.
    set_error(a->source(), "v-000x: invalid assignment of '" +
                               lhs_result_type->type_name() + "' to '" +
                               rhs_result_type->type_name() + "'");
    return false;
  }
  return true;
}

bool ValidatorImpl::ValidateExpression(const ast::Expression* expr) {
  if (!expr) {
    return false;
  }
  if (expr->IsIdentifier()) {
    return ValidateIdentifier(expr->AsIdentifier());
  }

  if (expr->IsCall()) {
    return ValidateCallExpr(expr->AsCall());
  }
  return true;
}

bool ValidatorImpl::ValidateIdentifier(const ast::IdentifierExpression* ident) {
  ast::Variable* var;
  if (!variable_stack_.get(ident->name(), &var)) {
    set_error(ident->source(),
              "v-0006: '" + ident->name() + "' is not declared");
    return false;
  }
  return true;
}

bool ValidatorImpl::CheckImports(const ast::Module* module) {
  for (const auto& import : module->imports()) {
    if (import->path() != "GLSL.std.450") {
      set_error(import->source(), "v-0001: unknown import: " + import->path());
      return false;
    }
  }
  return true;
}

}  // namespace tint
