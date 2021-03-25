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

#include "src/validator/validator_impl.h"

#include <utility>

#include "src/ast/call_statement.h"
#include "src/ast/constant_id_decoration.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/sint_literal.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/stride_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/struct_member_align_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/struct_member_size_decoration.h"
#include "src/ast/uint_literal.h"
#include "src/ast/workgroup_decoration.h"
#include "src/semantic/call.h"
#include "src/semantic/function.h"
#include "src/semantic/variable.h"
#include "src/type/alias_type.h"
#include "src/type/array_type.h"
#include "src/type/matrix_type.h"
#include "src/type/pointer_type.h"
#include "src/type/struct_type.h"
#include "src/type/u32_type.h"
#include "src/type/vector_type.h"
#include "src/type/void_type.h"

namespace tint {

ValidatorImpl::ValidatorImpl(const Program* program) : program_(program) {}

ValidatorImpl::~ValidatorImpl() = default;

void ValidatorImpl::add_error(const Source& src,
                              const char* code,
                              const std::string& msg) {
  diag::Diagnostic diag;
  diag.severity = diag::Severity::Error;
  diag.source = src;
  diag.message = msg;
  diag.code = code;
  diags_.add(std::move(diag));
}

void ValidatorImpl::add_error(const Source& src, const std::string& msg) {
  diags_.add_error(msg, src);
}

bool ValidatorImpl::Validate() {
  if (!program_->IsValid()) {
    // If we're attempting to validate an invalid program, fail with the
    // program's diagnostics.
    diags_.add(program_->Diagnostics());
    return false;
  }

  // Validate global declarations in the order they appear in the module.
  for (auto* decl : program_->AST().GlobalDeclarations()) {
    if (decl->Is<type::Type>()) {
      // Validated by Resolver (Struct types only)
      return true;
    } else if (auto* func = decl->As<ast::Function>()) {
      current_function_ = func;
      if (!ValidateFunction(func)) {
        return false;
      }
      current_function_ = nullptr;
    } else if (auto* var = decl->As<ast::Variable>()) {
      if (!ValidateGlobalVariable(var)) {
        return false;
      }
    } else {
      TINT_UNREACHABLE(diags_);
      return false;
    }
  }
  if (!ValidateEntryPoint(program_->AST().Functions())) {
    return false;
  }

  return true;
}

bool ValidatorImpl::ValidateGlobalVariable(const ast::Variable* var) {
  auto* sem = program_->Sem().Get(var);
  if (!sem) {
    add_error(var->source(), "no semantic information for variable '" +
                                 program_->Symbols().NameFor(var->symbol()) +
                                 "'");
    return false;
  }

  if (variable_stack_.has(var->symbol())) {
    add_error(var->source(), "v-0011",
              "redeclared global identifier '" +
                  program_->Symbols().NameFor(var->symbol()) + "'");
    return false;
  }
  if (!var->is_const() && sem->StorageClass() == ast::StorageClass::kNone) {
    add_error(var->source(), "v-0022",
              "global variables must have a storage class");
    return false;
  }
  if (var->is_const() && !(sem->StorageClass() == ast::StorageClass::kNone)) {
    add_error(var->source(), "v-global01",
              "global constants shouldn't have a storage class");
    return false;
  }

  for (auto* deco : var->decorations()) {
    if (!(deco->Is<ast::BindingDecoration>() ||
          deco->Is<ast::BuiltinDecoration>() ||
          deco->Is<ast::ConstantIdDecoration>() ||
          deco->Is<ast::GroupDecoration>() ||
          deco->Is<ast::LocationDecoration>())) {
      add_error(deco->source(), "decoration is not valid for variables");
      return false;
    }
  }

  variable_stack_.set_global(var->symbol(), var);
  return true;
}

bool ValidatorImpl::ValidateEntryPoint(const ast::FunctionList& funcs) {
  auto shader_is_present = false;
  for (auto* func : funcs) {
    if (func->IsEntryPoint()) {
      shader_is_present = true;
      auto stage_deco_count = 0;
      for (auto* deco : func->decorations()) {
        if (deco->Is<ast::StageDecoration>()) {
          stage_deco_count++;
        } else if (!deco->Is<ast::WorkgroupDecoration>()) {
          add_error(func->source(), "decoration is not valid for functions");
          return false;
        }
      }
      if (stage_deco_count > 1) {
        add_error(func->source(), "v-0020",
                  "only one stage decoration permitted per entry point");
        return false;
      }
    }
  }
  if (!shader_is_present) {
    add_error(Source{}, "v-0003",
              "At least one of vertex, fragment or compute shader must "
              "be present");
    return false;
  }
  return true;
}

bool ValidatorImpl::ValidateFunction(const ast::Function* func) {
  // TODO(amaiorano): Remove ValidateFunction once we've moved all the statement
  // validation to Resovler

  variable_stack_.push_scope();
  for (auto* param : func->params()) {
    variable_stack_.set(param->symbol(), param);
  }
  if (!ValidateStatements(func->body())) {
    return false;
  }
  variable_stack_.pop_scope();
  return true;
}

bool ValidatorImpl::ValidateStatements(const ast::BlockStatement* block) {
  if (!block) {
    return false;
  }

  bool is_valid = true;
  variable_stack_.push_scope();
  for (auto* stmt : *block) {
    if (!ValidateStatement(stmt)) {
      is_valid = false;
      break;
    }
  }
  variable_stack_.pop_scope();

  return is_valid;
}

bool ValidatorImpl::ValidateDeclStatement(
    const ast::VariableDeclStatement* decl) {
  auto symbol = decl->variable()->symbol();
  bool is_global = false;
  if (variable_stack_.get(symbol, nullptr, &is_global)) {
    const char* error_code = "v-0014";
    if (is_global) {
      error_code = "v-0013";
    }
    add_error(
        decl->source(), error_code,
        "redeclared identifier '" + program_->Symbols().NameFor(symbol) + "'");
    return false;
  }
  // TODO(dneto): Check type compatibility of the initializer.
  //  - if it's non-constant, then is storable or can be dereferenced to be
  //    storable.
  //  - types match or the RHS can be dereferenced to equal the LHS type.
  variable_stack_.set(symbol, decl->variable());
  if (auto* arr = decl->variable()->type()->UnwrapAll()->As<type::Array>()) {
    if (arr->IsRuntimeArray()) {
      add_error(
          decl->source(), "v-0015",
          "runtime arrays may only appear as the last member of a struct");
      return false;
    }
  }
  return true;
}

bool ValidatorImpl::ValidateStatement(const ast::Statement* stmt) {
  if (!stmt) {
    return false;
  }
  if (auto* v = stmt->As<ast::VariableDeclStatement>()) {
    return ValidateDeclStatement(v);
  }
  if (auto* a = stmt->As<ast::AssignmentStatement>()) {
    return ValidateAssign(a);
  }
  if (auto* s = stmt->As<ast::SwitchStatement>()) {
    return ValidateSwitch(s);
  }
  if (auto* c = stmt->As<ast::CaseStatement>()) {
    return ValidateCase(c);
  }
  if (auto* b = stmt->As<ast::BlockStatement>()) {
    return ValidateStatements(b);
  }
  return true;
}

bool ValidatorImpl::ValidateSwitch(const ast::SwitchStatement* s) {
  // TODO(amaiorano): Switch validation has moved to Resolver, but we need this
  // logic to validate case statements for now. Remove once ValidateStatement()
  // can be removed.
  for (auto* case_stmt : s->body()) {
    if (!ValidateStatement(case_stmt)) {
      return false;
    }
  }
  return true;
}

bool ValidatorImpl::ValidateCase(const ast::CaseStatement* c) {
  if (!ValidateStatement(c->body())) {
    return false;
  }
  return true;
}

bool ValidatorImpl::ValidateBadAssignmentToIdentifier(
    const ast::AssignmentStatement* assign) {
  auto* ident = assign->lhs()->As<ast::IdentifierExpression>();
  if (!ident) {
    // It wasn't an identifier in the first place.
    return true;
  }
  const ast::Variable* var;
  if (variable_stack_.get(ident->symbol(), &var)) {
    // Give a nicer message if the LHS of the assignment is a const identifier.
    // It's likely to be a common programmer error.
    if (var->is_const()) {
      add_error(assign->source(), "v-0021",
                "cannot re-assign a constant: '" +
                    program_->Symbols().NameFor(ident->symbol()) + "'");
      return false;
    }
  } else {
    // The identifier is not defined.
    // Shouldn't reach here. This should already have been caught when
    // validation expressions in the Resolver
    TINT_UNREACHABLE(diagnostics());
    return false;
  }
  return true;
}

bool ValidatorImpl::ValidateAssign(const ast::AssignmentStatement* assign) {
  if (!assign) {
    return false;
  }
  auto* lhs = assign->lhs();
  auto* rhs = assign->rhs();

  // Pointers are not storable in WGSL, but the right-hand side must be
  // storable. The raw right-hand side might be a pointer value which must be
  // loaded (dereferenced) to provide the value to be stored.
  auto* rhs_result_type = program_->Sem().Get(rhs)->Type()->UnwrapAll();
  if (!IsStorable(rhs_result_type)) {
    add_error(assign->source(), "v-000x",
              "invalid assignment: right-hand-side is not storable: " +
                  program_->Sem().Get(rhs)->Type()->type_name());
    return false;
  }
  auto* lhs_result_type = program_->Sem().Get(lhs)->Type()->UnwrapIfNeeded();
  if (!Is<type::Pointer>(lhs_result_type)) {
    if (!ValidateBadAssignmentToIdentifier(assign)) {
      return false;
    }
    // Issue a generic error.
    add_error(
        assign->source(), "v-000x",
        "invalid assignment: left-hand-side does not reference storage: " +
            program_->Sem().Get(lhs)->Type()->type_name());
    return false;
  }

  return true;
}

bool ValidatorImpl::IsStorable(type::Type* type) {
  if (type == nullptr) {
    return false;
  }
  if (type->is_scalar() || type->Is<type::Vector>() ||
      type->Is<type::Matrix>()) {
    return true;
  }
  if (type::Array* array_type = type->As<type::Array>()) {
    return IsStorable(array_type->type());
  }
  if (type::Struct* struct_type = type->As<type::Struct>()) {
    for (const auto* member : struct_type->impl()->members()) {
      if (!IsStorable(member->type())) {
        return false;
      }
    }
    return true;
  }
  if (type::Alias* alias_type = type->As<type::Alias>()) {
    return IsStorable(alias_type->type());
  }
  return false;
}

}  // namespace tint
