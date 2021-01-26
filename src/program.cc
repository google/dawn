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

Program::Program() : ast_(nodes_.Create<ast::Module>()) {}

Program::Program(Program&& rhs) = default;

Program& Program::operator=(Program&& rhs) = default;

Program::~Program() = default;

Program Program::Clone() const {
  Program out;
  CloneContext(&out, this).Clone();
  return out;
}

void Program::Clone(CloneContext* ctx) const {
  for (auto* ty : AST().ConstructedTypes()) {
    ctx->dst->AST().AddConstructedType(ctx->Clone(ty));
  }
  for (auto* var : AST().GlobalVariables()) {
    ctx->dst->AST().AddGlobalVariable(ctx->Clone(var));
  }
  for (auto* func : AST().Functions()) {
    ctx->dst->AST().Functions().Add(ctx->Clone(func));
  }
}

Symbol Program::RegisterSymbol(const std::string& name) {
  return symbols_.Register(name);
}

Symbol Program::GetSymbol(const std::string& name) const {
  return symbols_.Get(name);
}

std::string Program::SymbolToName(const Symbol sym) const {
  return symbols_.NameFor(sym);
}

bool Program::IsValid() const {
  return ast_->IsValid();
}

std::string Program::to_str() const {
  return ast_->to_str();
}

}  // namespace tint
