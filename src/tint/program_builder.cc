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

#include "src/tint/program_builder.h"

#include "src/tint/ast/assignment_statement.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/debug.h"
#include "src/tint/demangler.h"
#include "src/tint/sem/expression.h"
#include "src/tint/sem/variable.h"

namespace tint {

ProgramBuilder::VarOptionals::~VarOptionals() = default;

ProgramBuilder::ProgramBuilder()
    : id_(ProgramID::New()), ast_(ast_nodes_.Create<ast::Module>(id_, Source{})) {}

ProgramBuilder::ProgramBuilder(ProgramBuilder&& rhs)
    : id_(std::move(rhs.id_)),
      types_(std::move(rhs.types_)),
      ast_nodes_(std::move(rhs.ast_nodes_)),
      sem_nodes_(std::move(rhs.sem_nodes_)),
      ast_(rhs.ast_),
      sem_(std::move(rhs.sem_)),
      symbols_(std::move(rhs.symbols_)),
      diagnostics_(std::move(rhs.diagnostics_)) {
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
    diagnostics_ = std::move(rhs.diagnostics_);

    return *this;
}

ProgramBuilder ProgramBuilder::Wrap(const Program* program) {
    ProgramBuilder builder;
    builder.id_ = program->ID();
    builder.types_ = sem::Manager::Wrap(program->Types());
    builder.ast_ =
        builder.create<ast::Module>(program->AST().source, program->AST().GlobalDeclarations());
    builder.sem_ = sem::Info::Wrap(program->Sem());
    builder.symbols_ = program->Symbols();
    builder.diagnostics_ = program->Diagnostics();
    return builder;
}

bool ProgramBuilder::IsValid() const {
    return !diagnostics_.contains_errors();
}

void ProgramBuilder::MarkAsMoved() {
    AssertNotMoved();
    moved_ = true;
}

void ProgramBuilder::AssertNotMoved() const {
    if (moved_) {
        TINT_ICE(ProgramBuilder, const_cast<ProgramBuilder*>(this)->diagnostics_)
            << "Attempting to use ProgramBuilder after it has been moved";
    }
}

const sem::Type* ProgramBuilder::TypeOf(const ast::Expression* expr) const {
    auto* sem = Sem().Get(expr);
    return sem ? sem->Type() : nullptr;
}

const sem::Type* ProgramBuilder::TypeOf(const ast::Variable* var) const {
    auto* sem = Sem().Get(var);
    return sem ? sem->Type() : nullptr;
}

const sem::Type* ProgramBuilder::TypeOf(const ast::Type* type) const {
    return Sem().Get(type);
}

const sem::Type* ProgramBuilder::TypeOf(const ast::TypeDecl* type_decl) const {
    return Sem().Get(type_decl);
}

const ast::TypeName* ProgramBuilder::TypesBuilder::Of(const ast::TypeDecl* decl) const {
    return type_name(decl->name);
}

ProgramBuilder::TypesBuilder::TypesBuilder(ProgramBuilder* pb) : builder(pb) {}

const ast::Statement* ProgramBuilder::WrapInStatement(const ast::Expression* expr) {
    // Create a temporary variable of inferred type from expr.
    return Decl(Let(symbols_.New(), nullptr, expr));
}

const ast::VariableDeclStatement* ProgramBuilder::WrapInStatement(const ast::Variable* v) {
    return create<ast::VariableDeclStatement>(v);
}

const ast::Statement* ProgramBuilder::WrapInStatement(const ast::Statement* stmt) {
    return stmt;
}

const ast::Function* ProgramBuilder::WrapInFunction(const ast::StatementList stmts) {
    return Func(
        "test_function", {}, ty.void_(), std::move(stmts),
        {create<ast::StageAttribute>(ast::PipelineStage::kCompute), WorkgroupSize(1, 1, 1)});
}

}  // namespace tint
