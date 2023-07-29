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

#include "src/tint/lang/wgsl/program/program_builder.h"

#include "src/tint/lang/wgsl/ast/assignment_statement.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/lang/wgsl/sem/type_expression.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/rtti/switch.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint {

ProgramBuilder::ProgramBuilder() = default;

ProgramBuilder::ProgramBuilder(ProgramBuilder&& rhs)
    : Builder(std::move(rhs)),
      constants(std::move(rhs.constants)),
      sem_nodes_(std::move(rhs.sem_nodes_)),
      sem_(std::move(rhs.sem_)) {}

ProgramBuilder::~ProgramBuilder() = default;

ProgramBuilder& ProgramBuilder::operator=(ProgramBuilder&& rhs) {
    *static_cast<Builder*>(this) = std::move(rhs);
    constants = std::move(rhs.constants);
    sem_nodes_ = std::move(rhs.sem_nodes_);
    sem_ = std::move(rhs.sem_);
    return *this;
}

ProgramBuilder ProgramBuilder::Wrap(const Program* program) {
    ProgramBuilder builder;
    builder.id_ = program->ID();
    builder.last_ast_node_id_ = program->HighestASTNodeID();
    builder.constants = constant::Manager::Wrap(program->Constants());
    builder.ast_ =
        builder.create<ast::Module>(program->AST().source, program->AST().GlobalDeclarations());
    builder.sem_ = sem::Info::Wrap(program->Sem());
    builder.symbols_.Wrap(program->Symbols());
    builder.diagnostics_ = program->Diagnostics();
    return builder;
}

void ProgramBuilder::AssertNotMoved() const {
    if (TINT_UNLIKELY(moved_)) {
        TINT_ICE() << "Attempting to use ProgramBuilder after it has been moved";
    }
}

const type::Type* ProgramBuilder::TypeOf(const ast::Expression* expr) const {
    return tint::Switch(
        Sem().Get(expr),  //
        [](const sem::ValueExpression* e) { return e->Type(); },
        [](const sem::TypeExpression* e) { return e->Type(); });
}

const type::Type* ProgramBuilder::TypeOf(const ast::Variable* var) const {
    auto* sem = Sem().Get(var);
    return sem ? sem->Type() : nullptr;
}

const type::Type* ProgramBuilder::TypeOf(const ast::TypeDecl* type_decl) const {
    return Sem().Get(type_decl);
}

}  // namespace tint
