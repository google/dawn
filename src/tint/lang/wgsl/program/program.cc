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

#include "src/tint/lang/wgsl/program/program.h"

#include <utility>

#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/sem/type_expression.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint {
namespace {

std::string DefaultPrinter(const Program*) {
    return "<no program printer assigned>";
}

}  // namespace

Program::Printer Program::printer = DefaultPrinter;

Program::Program() = default;

Program::Program(Program&& program)
    : id_(std::move(program.id_)),
      highest_node_id_(std::move(program.highest_node_id_)),
      constants_(std::move(program.constants_)),
      ast_nodes_(std::move(program.ast_nodes_)),
      sem_nodes_(std::move(program.sem_nodes_)),
      ast_(std::move(program.ast_)),
      sem_(std::move(program.sem_)),
      symbols_(std::move(program.symbols_)),
      diagnostics_(std::move(program.diagnostics_)),
      is_valid_(program.is_valid_) {
    program.AssertNotMoved();
    program.moved_ = true;
}

Program::Program(ProgramBuilder&& builder) {
    id_ = builder.ID();
    highest_node_id_ = builder.LastAllocatedNodeID();
    is_valid_ = builder.IsValid();

    // The above must be called *before* the calls to std::move() below
    constants_ = std::move(builder.constants);
    ast_nodes_ = std::move(builder.ASTNodes());
    sem_nodes_ = std::move(builder.SemNodes());
    ast_ = &builder.AST();  // ast::Module is actually a heap allocation.
    sem_ = std::move(builder.Sem());
    symbols_ = std::move(builder.Symbols());
    diagnostics_.add(std::move(builder.Diagnostics()));
    builder.MarkAsMoved();

    if (!is_valid_ && !diagnostics_.contains_errors()) {
        // If the builder claims to be invalid, then we really should have an error
        // message generated. If we find a situation where the program is not valid
        // and there are no errors reported, add one here.
        diagnostics_.add_error(diag::System::Program, "invalid program generated");
    }
}

Program::~Program() = default;

Program& Program::operator=(Program&& program) {
    program.AssertNotMoved();
    program.moved_ = true;
    moved_ = false;
    id_ = std::move(program.id_);
    highest_node_id_ = std::move(program.highest_node_id_);
    constants_ = std::move(program.constants_);
    ast_nodes_ = std::move(program.ast_nodes_);
    sem_nodes_ = std::move(program.sem_nodes_);
    ast_ = std::move(program.ast_);
    sem_ = std::move(program.sem_);
    symbols_ = std::move(program.symbols_);
    diagnostics_ = std::move(program.diagnostics_);
    is_valid_ = program.is_valid_;
    return *this;
}

Program Program::Clone() const {
    AssertNotMoved();
    return Program(CloneAsBuilder());
}

ProgramBuilder Program::CloneAsBuilder() const {
    AssertNotMoved();
    ProgramBuilder out;
    program::CloneContext(&out, this).Clone();
    return out;
}

bool Program::IsValid() const {
    AssertNotMoved();
    return is_valid_;
}

const type::Type* Program::TypeOf(const ast::Expression* expr) const {
    return tint::Switch(
        Sem().Get(expr),  //
        [](const sem::ValueExpression* ty_expr) { return ty_expr->Type(); },
        [](const sem::TypeExpression* ty_expr) { return ty_expr->Type(); });
}

const type::Type* Program::TypeOf(const ast::Variable* var) const {
    auto* sem = Sem().Get(var);
    return sem ? sem->Type() : nullptr;
}

const type::Type* Program::TypeOf(const ast::TypeDecl* type_decl) const {
    return Sem().Get(type_decl);
}

void Program::AssertNotMoved() const {
    TINT_ASSERT(!moved_);
}

}  // namespace tint
