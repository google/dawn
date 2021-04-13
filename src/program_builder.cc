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

#include "src/ast/assignment_statement.h"
#include "src/ast/call_statement.h"
#include "src/ast/variable_decl_statement.h"
#include "src/debug.h"
#include "src/demangler.h"
#include "src/semantic/expression.h"

namespace tint {

ProgramBuilder::ProgramBuilder()
    : id_(ProgramID::New()),
      ast_(ast_nodes_.Create<ast::Module>(id_, Source{})) {}

ProgramBuilder::ProgramBuilder(ProgramBuilder&& rhs)
    : id_(std::move(rhs.id_)),
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
  id_ = std::move(rhs.id_);
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
  builder.id_ = program->ID();
  builder.types_ = type::Manager::Wrap(program->Types());
  builder.ast_ = builder.create<ast::Module>(
      program->AST().source(), program->AST().GlobalDeclarations());
  builder.sem_ = semantic::Info::Wrap(program->Sem());
  builder.symbols_ = program->Symbols();
  builder.diagnostics_ = program->Diagnostics();
  return builder;
}

bool ProgramBuilder::IsValid() const {
  return !diagnostics_.contains_errors();
}

std::string ProgramBuilder::str(const ast::Node* node) const {
  return Demangler().Demangle(Symbols(), node->str(Sem()));
}

void ProgramBuilder::MarkAsMoved() {
  AssertNotMoved();
  moved_ = true;
}

void ProgramBuilder::AssertNotMoved() const {
  if (moved_) {
    TINT_ICE(const_cast<ProgramBuilder*>(this)->Diagnostics())
        << "Attempting to use ProgramBuilder after it has been moved";
  }
}

type::Type* ProgramBuilder::TypeOf(ast::Expression* expr) const {
  auto* sem = Sem().Get(expr);
  return sem ? sem->Type() : nullptr;
}

ast::ConstructorExpression* ProgramBuilder::ConstructValueFilledWith(
    type::Type* type,
    int elem_value) {
  auto* unwrapped_type = type->UnwrapAliasIfNeeded();
  if (unwrapped_type->Is<type::Bool>()) {
    return create<ast::ScalarConstructorExpression>(
        create<ast::BoolLiteral>(type, elem_value == 0 ? false : true));
  }
  if (unwrapped_type->Is<type::I32>()) {
    return create<ast::ScalarConstructorExpression>(create<ast::SintLiteral>(
        type, static_cast<ProgramBuilder::i32>(elem_value)));
  }
  if (unwrapped_type->Is<type::U32>()) {
    return create<ast::ScalarConstructorExpression>(create<ast::UintLiteral>(
        type, static_cast<ProgramBuilder::u32>(elem_value)));
  }
  if (unwrapped_type->Is<type::F32>()) {
    return create<ast::ScalarConstructorExpression>(create<ast::FloatLiteral>(
        type, static_cast<ProgramBuilder::f32>(elem_value)));
  }
  if (auto* v = unwrapped_type->As<type::Vector>()) {
    auto* elem_default_value = ConstructValueFilledWith(v->type(), elem_value);
    ast::ExpressionList el(v->size());
    std::fill(el.begin(), el.end(), elem_default_value);
    return create<ast::TypeConstructorExpression>(type, std::move(el));
  }
  if (auto* m = unwrapped_type->As<type::Matrix>()) {
    auto* col_vec_type = create<type::Vector>(m->type(), m->rows());
    auto* vec_default_value =
        ConstructValueFilledWith(col_vec_type, elem_value);
    ast::ExpressionList el(m->columns());
    std::fill(el.begin(), el.end(), vec_default_value);
    return create<ast::TypeConstructorExpression>(type, std::move(el));
  }
  TINT_ASSERT(false);
  return nullptr;
}

ProgramBuilder::TypesBuilder::TypesBuilder(ProgramBuilder* pb) : builder(pb) {}

ast::VariableDeclStatement* ProgramBuilder::WrapInStatement(ast::Variable* v) {
  return create<ast::VariableDeclStatement>(v);
}

ast::Statement* ProgramBuilder::WrapInStatement(ast::Expression* expr) {
  // Create a temporary variable of inferred type from expr.
  return Decl(Var(symbols_.New(), nullptr, ast::StorageClass::kFunction, expr));
}

ast::Statement* ProgramBuilder::WrapInStatement(ast::Statement* stmt) {
  return stmt;
}

ast::Function* ProgramBuilder::WrapInFunction(ast::StatementList stmts) {
  return Func("test_function", {}, ty.void_(), std::move(stmts),
              {create<ast::StageDecoration>(ast::PipelineStage::kCompute)});
}

}  // namespace tint
