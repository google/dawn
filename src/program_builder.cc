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
#include "src/sem/expression.h"
#include "src/sem/variable.h"

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
  builder.types_ = sem::Manager::Wrap(program->Types());
  builder.ast_ = builder.create<ast::Module>(
      program->AST().source(), program->AST().GlobalDeclarations());
  builder.sem_ = sem::Info::Wrap(program->Sem());
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
    TINT_ICE(const_cast<ProgramBuilder*>(this)->diagnostics_)
        << "Attempting to use ProgramBuilder after it has been moved";
  }
}

sem::Type* ProgramBuilder::TypeOf(const ast::Expression* expr) const {
  auto* sem = Sem().Get(expr);
  return sem ? sem->Type() : nullptr;
}

sem::Type* ProgramBuilder::TypeOf(const ast::Variable* var) const {
  auto* sem = Sem().Get(var);
  return sem ? sem->Type() : nullptr;
}

const sem::Type* ProgramBuilder::TypeOf(const ast::Type* type) const {
  return Sem().Get(type);
}

ast::ConstructorExpression* ProgramBuilder::ConstructValueFilledWith(
    const ast::Type* type,
    int elem_value) {
  CloneContext ctx(this);

  if (type->Is<ast::Bool>()) {
    return create<ast::ScalarConstructorExpression>(
        create<ast::BoolLiteral>(elem_value == 0 ? false : true));
  }
  if (type->Is<ast::I32>()) {
    return create<ast::ScalarConstructorExpression>(
        create<ast::SintLiteral>(static_cast<i32>(elem_value)));
  }
  if (type->Is<ast::U32>()) {
    return create<ast::ScalarConstructorExpression>(
        create<ast::UintLiteral>(static_cast<u32>(elem_value)));
  }
  if (type->Is<ast::F32>()) {
    return create<ast::ScalarConstructorExpression>(
        create<ast::FloatLiteral>(static_cast<f32>(elem_value)));
  }
  if (auto* v = type->As<ast::Vector>()) {
    ast::ExpressionList el(v->size());
    for (size_t i = 0; i < el.size(); i++) {
      el[i] = ConstructValueFilledWith(ctx.Clone(v->type()), elem_value);
    }
    return create<ast::TypeConstructorExpression>(const_cast<ast::Type*>(type),
                                                  std::move(el));
  }
  if (auto* m = type->As<ast::Matrix>()) {
    ast::ExpressionList el(m->columns());
    for (size_t i = 0; i < el.size(); i++) {
      auto* col_vec_type = create<ast::Vector>(ctx.Clone(m->type()), m->rows());
      el[i] = ConstructValueFilledWith(col_vec_type, elem_value);
    }
    return create<ast::TypeConstructorExpression>(const_cast<ast::Type*>(type),
                                                  std::move(el));
  }
  if (auto* tn = type->As<ast::TypeName>()) {
    if (auto* lookup = AST().LookupType(tn->name())) {
      if (auto* alias = lookup->As<ast::Alias>()) {
        return ConstructValueFilledWith(ctx.Clone(alias->type()), elem_value);
      }
    }
    TINT_ICE(diagnostics_) << "unable to find NamedType '"
                           << Symbols().NameFor(tn->name()) << "'";
    return nullptr;
  }

  TINT_ICE(diagnostics_) << "unhandled type: " << type->TypeInfo().name;
  return nullptr;
}

ast::Type* ProgramBuilder::TypesBuilder::MaybeCreateTypename(
    ast::Type* type) const {
  if (auto* nt = As<ast::NamedType>(type)) {
    return type_name(nt->name());
  }
  return type;
}

const ast::Type* ProgramBuilder::TypesBuilder::MaybeCreateTypename(
    const ast::Type* type) const {
  if (auto* nt = As<ast::NamedType>(type)) {
    return type_name(nt->name());
  }
  return type;
}

ProgramBuilder::TypesBuilder::TypesBuilder(ProgramBuilder* pb) : builder(pb) {}

ast::Statement* ProgramBuilder::WrapInStatement(ast::Literal* lit) {
  return WrapInStatement(create<ast::ScalarConstructorExpression>(lit));
}

ast::Statement* ProgramBuilder::WrapInStatement(ast::Expression* expr) {
  if (auto* ce = expr->As<ast::CallExpression>()) {
    return create<ast::CallStatement>(ce);
  }
  // Create a temporary variable of inferred type from expr.
  return Decl(Const(symbols_.New(), nullptr, expr));
}

ast::VariableDeclStatement* ProgramBuilder::WrapInStatement(ast::Variable* v) {
  return create<ast::VariableDeclStatement>(v);
}

ast::Statement* ProgramBuilder::WrapInStatement(ast::Statement* stmt) {
  return stmt;
}

ast::Function* ProgramBuilder::WrapInFunction(ast::StatementList stmts) {
  return Func("test_function", {}, ty.void_(), std::move(stmts),
              {create<ast::StageDecoration>(ast::PipelineStage::kCompute)});
}

}  // namespace tint
