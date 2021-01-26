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

#include "src/program.h"

#include <sstream>

#include "src/clone_context.h"
#include "src/type/struct_type.h"

namespace tint {

Program::Program() = default;

Program::Program(Program&&) = default;

Program& Program::operator=(Program&& rhs) = default;

Program::~Program() = default;

Program Program::Clone() const {
  Program out;
  CloneContext(&out, this).Clone();
  return out;
}

void Program::Clone(CloneContext* ctx) const {
  for (auto* ty : constructed_types_) {
    ctx->dst->constructed_types_.emplace_back(ctx->Clone(ty));
  }
  for (auto* var : global_variables_) {
    ctx->dst->global_variables_.emplace_back(ctx->Clone(var));
  }
  for (auto* func : functions_) {
    ctx->dst->functions_.emplace_back(ctx->Clone(func));
  }
}

Symbol Program::RegisterSymbol(const std::string& name) {
  return symbol_table_.Register(name);
}

Symbol Program::GetSymbol(const std::string& name) const {
  return symbol_table_.Get(name);
}

std::string Program::SymbolToName(const Symbol sym) const {
  return symbol_table_.NameFor(sym);
}

bool Program::IsValid() const {
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

std::string Program::to_str() const {
  std::ostringstream out;

  out << "Module{" << std::endl;
  const auto indent = 2;
  for (auto* const ty : constructed_types_) {
    for (size_t i = 0; i < indent; ++i) {
      out << " ";
    }
    if (auto* alias = ty->As<type::Alias>()) {
      out << alias->symbol().to_str() << " -> " << alias->type()->type_name()
          << std::endl;
      if (auto* str = alias->type()->As<type::Struct>()) {
        str->impl()->to_str(out, indent);
      }
    } else if (auto* str = ty->As<type::Struct>()) {
      out << str->symbol().to_str() << " ";
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

}  // namespace tint
