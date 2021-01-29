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

#include "src/program_builder.h"

#include <assert.h>

#include <sstream>

#include "src/clone_context.h"
#include "src/demangler.h"
#include "src/type/struct_type.h"

namespace tint {

ProgramBuilder::ProgramBuilder()
    : ty(this), ast_(nodes_.Create<ast::Module>(Source{})) {}

ProgramBuilder::ProgramBuilder(ProgramBuilder&& rhs)
    : ty(std::move(rhs.ty)),
      types_(std::move(rhs.types_)),
      nodes_(std::move(rhs.nodes_)),
      ast_(rhs.ast_),
      sem_(std::move(rhs.sem_)),
      symbols_(std::move(rhs.symbols_)) {
  rhs.MarkAsMoved();
}

ProgramBuilder::~ProgramBuilder() = default;

ProgramBuilder& ProgramBuilder::operator=(ProgramBuilder&& rhs) {
  rhs.MarkAsMoved();
  AssertNotMoved();
  ty = std::move(rhs.ty);
  types_ = std::move(rhs.types_);
  nodes_ = std::move(rhs.nodes_);
  ast_ = rhs.ast_;
  sem_ = std::move(rhs.sem_);
  symbols_ = std::move(rhs.symbols_);
  return *this;
}

ProgramBuilder ProgramBuilder::Wrap(const Program* program) {
  ProgramBuilder builder;
  builder.types_ = type::Manager::Wrap(program->Types());
  builder.ast_ = builder.create<ast::Module>(
      program->AST().source(), program->AST().ConstructedTypes(),
      program->AST().Functions(), program->AST().GlobalVariables());
  builder.sem_ = semantic::Info::Wrap(program->Sem());
  builder.symbols_ = program->Symbols();
  builder.diagnostics_ = program->Diagnostics();
  return builder;
}

bool ProgramBuilder::IsValid() const {
  return !diagnostics_.contains_errors() && ast_->IsValid();
}

std::string ProgramBuilder::str(const ast::Node* node) const {
  return Demangler().Demangle(Symbols(), node->str(Sem()));
}

void ProgramBuilder::MarkAsMoved() {
  AssertNotMoved();
  moved_ = true;
}

void ProgramBuilder::AssertNotMoved() const {
  assert(!moved_);
}

ProgramBuilder::TypesBuilder::TypesBuilder(ProgramBuilder* pb) : builder(pb) {}

ast::Variable* ProgramBuilder::Var(const std::string& name,
                                   ast::StorageClass storage,
                                   type::Type* type) {
  return Var(name, storage, type, nullptr, {});
}

ast::Variable* ProgramBuilder::Var(const std::string& name,
                                   ast::StorageClass storage,
                                   type::Type* type,
                                   ast::Expression* constructor,
                                   ast::VariableDecorationList decorations) {
  auto* var = create<ast::Variable>(Symbols().Register(name), storage, type,
                                    false, constructor, decorations);
  OnVariableBuilt(var);
  return var;
}

ast::Variable* ProgramBuilder::Var(const Source& source,
                                   const std::string& name,
                                   ast::StorageClass storage,
                                   type::Type* type,
                                   ast::Expression* constructor,
                                   ast::VariableDecorationList decorations) {
  auto* var = create<ast::Variable>(source, Symbols().Register(name), storage,
                                    type, false, constructor, decorations);
  OnVariableBuilt(var);
  return var;
}

ast::Variable* ProgramBuilder::Const(const std::string& name,
                                     ast::StorageClass storage,
                                     type::Type* type) {
  return Const(name, storage, type, nullptr, {});
}

ast::Variable* ProgramBuilder::Const(const std::string& name,
                                     ast::StorageClass storage,
                                     type::Type* type,
                                     ast::Expression* constructor,
                                     ast::VariableDecorationList decorations) {
  auto* var = create<ast::Variable>(Symbols().Register(name), storage, type,
                                    true, constructor, decorations);
  OnVariableBuilt(var);
  return var;
}

ast::Variable* ProgramBuilder::Const(const Source& source,
                                     const std::string& name,
                                     ast::StorageClass storage,
                                     type::Type* type,
                                     ast::Expression* constructor,
                                     ast::VariableDecorationList decorations) {
  auto* var = create<ast::Variable>(source, Symbols().Register(name), storage,
                                    type, true, constructor, decorations);
  OnVariableBuilt(var);
  return var;
}

}  // namespace tint
