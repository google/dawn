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

#include "src/ast/builder.h"

namespace tint {
namespace ast {

TypesBuilder::TypesBuilder(Module* mod)
    : bool_(mod->create<ast::type::BoolType>()),
      f32(mod->create<ast::type::F32Type>()),
      i32(mod->create<ast::type::I32Type>()),
      u32(mod->create<ast::type::U32Type>()),
      void_(mod->create<ast::type::VoidType>()),
      mod_(mod) {}

Builder::Builder(tint::Context* c, tint::ast::Module* m)
    : ctx(c), mod(m), ty(m) {}
Builder::~Builder() = default;

ast::Variable* Builder::Var(const std::string& name,
                            ast::StorageClass storage,
                            ast::type::Type* type) {
  auto* var = create<ast::Variable>(name, storage, type);
  OnVariableBuilt(var);
  return var;
}

BuilderWithContextAndModule::BuilderWithContextAndModule()
    : Builder(new Context(), new ast::Module()) {}
BuilderWithContextAndModule::~BuilderWithContextAndModule() {
  delete ctx;
  delete mod;
}

}  // namespace ast
}  // namespace tint
