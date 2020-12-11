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
    : bool_(mod->create<type::Bool>()),
      f32(mod->create<type::F32>()),
      i32(mod->create<type::I32>()),
      u32(mod->create<type::U32>()),
      void_(mod->create<type::Void>()),
      mod_(mod) {}

Builder::Builder(Module* m) : mod(m), ty(m) {}

Builder::~Builder() = default;

Variable* Builder::Var(const std::string& name,
                       StorageClass storage,
                       type::Type* type) {
  return Var(name, storage, type, nullptr, {});
}

Variable* Builder::Var(const std::string& name,
                       StorageClass storage,
                       type::Type* type,
                       Expression* constructor,
                       VariableDecorationList decorations) {
  auto* var = create<Variable>(Source{}, name, storage, type, false,
                               constructor, decorations);
  OnVariableBuilt(var);
  return var;
}

Variable* Builder::Const(const std::string& name,
                         StorageClass storage,
                         type::Type* type) {
  return Const(name, storage, type, nullptr, {});
}

Variable* Builder::Const(const std::string& name,
                         StorageClass storage,
                         type::Type* type,
                         Expression* constructor,
                         VariableDecorationList decorations) {
  auto* var = create<Variable>(Source{}, name, storage, type, true, constructor,
                               decorations);
  OnVariableBuilt(var);
  return var;
}

BuilderWithModule::BuilderWithModule() : Builder(new Module()) {}

BuilderWithModule::~BuilderWithModule() {
  delete mod;
}

}  // namespace ast
}  // namespace tint
