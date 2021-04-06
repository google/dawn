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
  variable_stack_.set_global(var->symbol(), var);
  return true;
}

bool ValidatorImpl::ValidateEntryPoint(const ast::FunctionList& funcs) {
  for (auto* func : funcs) {
    if (func->IsEntryPoint()) {
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
  variable_stack_.set(symbol, decl->variable());
  return true;
}

bool ValidatorImpl::ValidateStatement(const ast::Statement* stmt) {
  if (!stmt) {
    return false;
  }
  if (auto* v = stmt->As<ast::VariableDeclStatement>()) {
    return ValidateDeclStatement(v);
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

}  // namespace tint
