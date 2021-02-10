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

#include "src/ast/assignment_statement.h"
#include "src/ast/variable_decl_statement.h"
#include "src/clone_context.h"
#include "src/demangler.h"
#include "src/semantic/expression.h"
#include "src/type/struct_type.h"

namespace tint {

ProgramBuilder::ProgramBuilder()
    : ty(this), ast_(ast_nodes_.Create<ast::Module>(Source{})) {}

ProgramBuilder::ProgramBuilder(ProgramBuilder&& rhs)
    : ty(std::move(rhs.ty)),
      types_(std::move(rhs.types_)),
      ast_nodes_(std::move(rhs.ast_nodes_)),
      sem_nodes_(std::move(rhs.sem_nodes_)),
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
  ast_nodes_ = std::move(rhs.ast_nodes_);
  sem_nodes_ = std::move(rhs.sem_nodes_);
  ast_ = rhs.ast_;
  sem_ = std::move(rhs.sem_);
  symbols_ = std::move(rhs.symbols_);
  return *this;
}

ProgramBuilder ProgramBuilder::Wrap(const Program* program) {
  ProgramBuilder builder;
  builder.types_ = type::Manager::Wrap(program->Types());
  builder.ast_ = builder.create<ast::Module>(
      program->AST().source(), program->AST().GlobalDeclarations());
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

type::Type* ProgramBuilder::TypeOf(ast::Expression* expr) const {
  auto* sem = Sem().Get(expr);
  return sem ? sem->Type() : nullptr;
}

ProgramBuilder::TypesBuilder::TypesBuilder(ProgramBuilder* pb) : builder(pb) {}

ast::VariableDeclStatement* ProgramBuilder::WrapInStatement(ast::Variable* v) {
  return create<ast::VariableDeclStatement>(v);
}

ast::Statement* ProgramBuilder::WrapInStatement(ast::Expression* expr) {
  // TODO(ben-clayton): This is valid enough for the TypeDeterminer, but the LHS
  // may not be assignable, and so may not validate.
  return create<ast::AssignmentStatement>(expr, expr);
}

ast::Statement* ProgramBuilder::WrapInStatement(ast::Statement* stmt) {
  return stmt;
}

void ProgramBuilder::WrapInFunction(ast::StatementList stmts) {
  Func("test_function", {}, ty.void_(), stmts, {});
}

}  // namespace tint
