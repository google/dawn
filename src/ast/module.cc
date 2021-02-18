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

#include <sstream>
#include <string>
#include <utility>

#include "src/debug.h"
#include "src/program_builder.h"
#include "src/type/alias_type.h"
#include "src/type/struct_type.h"

TINT_INSTANTIATE_CLASS_ID(tint::ast::Module);

namespace tint {
namespace ast {

Module::Module(const Source& source) : Base(source) {}

Module::Module(const Source& source, std::vector<CastableBase*> global_decls)
    : Base(source), global_declarations_(std::move(global_decls)) {
  for (auto* decl : global_declarations_) {
    if (decl == nullptr) {
      continue;
    }

    if (auto* ty = decl->As<type::Type>()) {
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

bool Module::IsValid() const {
  for (auto* decl : global_declarations_) {
    if (decl == nullptr) {
      return false;
    }
  }
  for (auto* var : global_variables_) {
    if (var == nullptr || !var->IsValid()) {
      return false;
    }
  }
  for (auto* const ty : constructed_types_) {
    if (ty == nullptr) {
      return false;
    }
    if (auto* alias = ty->As<type::Alias>()) {
      if (alias->type() == nullptr) {
        return false;
      }
      if (auto* str = alias->type()->As<type::Struct>()) {
        if (!str->symbol().IsValid()) {
          return false;
        }
      }
    } else if (auto* str = ty->As<type::Struct>()) {
      if (!str->symbol().IsValid()) {
        return false;
      }
    } else {
      return false;
    }
  }
  for (auto* func : functions_) {
    if (func == nullptr || !func->IsValid()) {
      return false;
    }
  }
  return true;
}

Module* Module::Clone(CloneContext* ctx) const {
  auto* out = ctx->dst->create<Module>();
  out->Copy(ctx, this);
  return out;
}

void Module::Copy(CloneContext* ctx, const Module* src) {
  for (auto* decl : src->global_declarations_) {
    assert(decl);
    if (auto* ty = decl->As<type::Type>()) {
      AddConstructedType(ctx->Clone(ty));
    } else if (auto* func = decl->As<Function>()) {
      AddFunction(ctx->Clone(func));
    } else if (auto* var = decl->As<Variable>()) {
      AddGlobalVariable(ctx->Clone(var));
    } else {
      TINT_ICE(ctx->dst->Diagnostics()) << "Unknown global declaration type";
    }
  }
}

void Module::to_str(const semantic::Info& sem,
                    std::ostream& out,
                    size_t indent) const {
  make_indent(out, indent);
  out << "Module{" << std::endl;
  indent += 2;
  for (auto* const ty : constructed_types_) {
    make_indent(out, indent);
    if (auto* alias = ty->As<type::Alias>()) {
      out << alias->symbol().to_str() << " -> " << alias->type()->type_name()
          << std::endl;
      if (auto* str = alias->type()->As<type::Struct>()) {
        str->impl()->to_str(sem, out, indent);
      }
    } else if (auto* str = ty->As<type::Struct>()) {
      out << str->symbol().to_str() << " ";
      str->impl()->to_str(sem, out, indent);
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

std::string Module::to_str(const semantic::Info& sem) const {
  std::ostringstream out;
  to_str(sem, out, 0);
  return out.str();
}

}  // namespace ast
}  // namespace tint
