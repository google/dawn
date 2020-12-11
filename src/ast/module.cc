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

#include "src/ast/module.h"

#include <sstream>

#include "src/ast/clone_context.h"
#include "src/ast/type/struct_type.h"

namespace tint {
namespace ast {

Module::Module() = default;

Module::Module(Module&&) = default;

Module& Module::operator=(Module&& rhs) = default;

Module::~Module() = default;

Module Module::Clone() {
  Module out;
  CloneContext ctx(&out);
  Clone(&ctx);
  return out;
}

void Module::Clone(CloneContext* ctx) {
  for (auto* ty : constructed_types_) {
    ctx->mod->constructed_types_.emplace_back(ctx->Clone(ty));
  }
  for (auto* var : global_variables_) {
    ctx->mod->global_variables_.emplace_back(ctx->Clone(var));
  }
  for (auto* func : functions_) {
    ctx->mod->functions_.emplace_back(ctx->Clone(func));
  }

  ctx->mod->symbol_table_ = symbol_table_;
}

Function* Module::FindFunctionBySymbol(Symbol sym) const {
  for (auto* func : functions_) {
    if (func->symbol() == sym) {
      return func;
    }
  }
  return nullptr;
}

Function* Module::FindFunctionBySymbolAndStage(Symbol sym,
                                               PipelineStage stage) const {
  for (auto* func : functions_) {
    if (func->symbol() == sym && func->pipeline_stage() == stage) {
      return func;
    }
  }
  return nullptr;
}

bool Module::HasStage(ast::PipelineStage stage) const {
  for (auto* func : functions_) {
    if (func->pipeline_stage() == stage) {
      return true;
    }
  }
  return false;
}

Symbol Module::RegisterSymbol(const std::string& name) {
  return symbol_table_.Register(name);
}

Symbol Module::GetSymbol(const std::string& name) const {
  return symbol_table_.GetSymbol(name);
}

std::string Module::SymbolToName(const Symbol sym) const {
  return symbol_table_.NameFor(sym);
}

bool Module::IsValid() const {
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
        if (str->name().empty()) {
          return false;
        }
      }
    } else if (auto* str = ty->As<type::Struct>()) {
      if (str->name().empty()) {
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

std::string Module::to_str() const {
  std::ostringstream out;

  out << "Module{" << std::endl;
  const auto indent = 2;
  for (auto* const ty : constructed_types_) {
    for (size_t i = 0; i < indent; ++i) {
      out << " ";
    }
    if (auto* alias = ty->As<type::Alias>()) {
      out << alias->name() << " -> " << alias->type()->type_name() << std::endl;
      if (auto* str = alias->type()->As<type::Struct>()) {
        str->impl()->to_str(out, indent);
      }
    } else if (auto* str = ty->As<type::Struct>()) {
      out << str->name() << " ";
      str->impl()->to_str(out, indent);
    }
  }
  for (auto* var : global_variables_) {
    var->to_str(out, indent);
  }
  for (auto* func : functions_) {
    func->to_str(out, indent);
  }
  out << "}" << std::endl;

  return out.str();
}

}  // namespace ast
}  // namespace tint
