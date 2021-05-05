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

#include "src/ast/module.h"

#include <utility>

#include "src/ast/named_type.h"
#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Module);

namespace tint {
namespace ast {

Module::Module(ProgramID program_id, const Source& source)
    : Base(program_id, source) {}

Module::Module(ProgramID program_id,
               const Source& source,
               std::vector<ast::Node*> global_decls)
    : Base(program_id, source), global_declarations_(std::move(global_decls)) {
  for (auto* decl : global_declarations_) {
    if (decl == nullptr) {
      continue;
    }

    if (auto* ty = decl->As<ast::NamedType>()) {
      constructed_types_.push_back(ty);
    } else if (auto* func = decl->As<Function>()) {
      functions_.push_back(func);
    } else if (auto* var = decl->As<Variable>()) {
      global_variables_.push_back(var);
    } else {
      diag::List diagnostics;
      TINT_ICE(diagnostics) << "Unknown global declaration type";
    }
  }
}

Module::~Module() = default;

const ast::NamedType* Module::LookupType(Symbol name) const {
  for (auto* ty : ConstructedTypes()) {
    if (ty->name() == name) {
      return ty;
    }
  }
  return nullptr;
}

void Module::AddGlobalVariable(ast::Variable* var) {
  TINT_ASSERT(var);
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(var, program_id());
  global_variables_.push_back(var);
  global_declarations_.push_back(var);
}

void Module::AddConstructedType(ast::NamedType* type) {
  TINT_ASSERT(type);
  constructed_types_.push_back(type);
  global_declarations_.push_back(type);
}

void Module::AddFunction(ast::Function* func) {
  TINT_ASSERT(func);
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(func, program_id());
  functions_.push_back(func);
  global_declarations_.push_back(func);
}

Module* Module::Clone(CloneContext* ctx) const {
  auto* out = ctx->dst->create<Module>();
  out->Copy(ctx, this);
  return out;
}

void Module::Copy(CloneContext* ctx, const Module* src) {
  for (auto* decl : ctx->Clone(src->global_declarations_)) {
    if (!decl) {
      TINT_ICE(ctx->dst->Diagnostics()) << "src global declaration was nullptr";
      continue;
    }
    if (auto* ty = decl->As<ast::NamedType>()) {
      AddConstructedType(ty);
    } else if (auto* func = decl->As<Function>()) {
      AddFunction(func);
    } else if (auto* var = decl->As<Variable>()) {
      AddGlobalVariable(var);
    } else {
      TINT_ICE(ctx->dst->Diagnostics()) << "Unknown global declaration type";
    }
  }
}

void Module::to_str(const sem::Info& sem,
                    std::ostream& out,
                    size_t indent) const {
  make_indent(out, indent);
  out << "Module{" << std::endl;
  indent += 2;
  for (auto* ty : constructed_types_) {
    make_indent(out, indent);
    if (auto* alias = ty->As<ast::Alias>()) {
      out << alias->symbol().to_str() << " -> " << alias->type()->type_name()
          << std::endl;
      if (auto* str = alias->type()->As<ast::Struct>()) {
        str->to_str(sem, out, indent);
      }
    } else if (auto* str = ty->As<ast::Struct>()) {
      str->to_str(sem, out, indent);
    }
  }
  for (auto* var : global_variables_) {
    var->to_str(sem, out, indent);
  }
  for (auto* func : functions_) {
    func->to_str(sem, out, indent);
  }
  out << "}" << std::endl;
}

std::string Module::to_str(const sem::Info& sem) const {
  std::ostringstream out;
  to_str(sem, out, 0);
  return out.str();
}

}  // namespace ast
}  // namespace tint
