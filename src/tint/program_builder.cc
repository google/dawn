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
#include "src/tint/sem/type_expression.h"
#include "src/tint/sem/value_expression.h"
#include "src/tint/sem/variable.h"
#include "src/tint/switch.h"
#include "src/tint/utils/compiler_macros.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint {

ProgramBuilder::VarOptions::~VarOptions() = default;
ProgramBuilder::LetOptions::~LetOptions() = default;
ProgramBuilder::ConstOptions::~ConstOptions() = default;
ProgramBuilder::OverrideOptions::~OverrideOptions() = default;

ProgramBuilder::ProgramBuilder()
    : id_(ProgramID::New()),
      ast_(ast_nodes_.Create<ast::Module>(id_, AllocateNodeID(), Source{})) {}

ProgramBuilder::ProgramBuilder(ProgramBuilder&& rhs)
    : id_(std::move(rhs.id_)),
      last_ast_node_id_(std::move(rhs.last_ast_node_id_)),
      types_(std::move(rhs.types_)),
      ast_nodes_(std::move(rhs.ast_nodes_)),
      sem_nodes_(std::move(rhs.sem_nodes_)),
      ast_(std::move(rhs.ast_)),
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
    last_ast_node_id_ = std::move(rhs.last_ast_node_id_);
    types_ = std::move(rhs.types_);
    ast_nodes_ = std::move(rhs.ast_nodes_);
    sem_nodes_ = std::move(rhs.sem_nodes_);
    ast_ = std::move(rhs.ast_);
    sem_ = std::move(rhs.sem_);
    symbols_ = std::move(rhs.symbols_);
    diagnostics_ = std::move(rhs.diagnostics_);

    return *this;
}

ProgramBuilder ProgramBuilder::Wrap(const Program* program) {
    ProgramBuilder builder;
    builder.id_ = program->ID();
    builder.last_ast_node_id_ = program->HighestASTNodeID();
    builder.types_ = type::Manager::Wrap(program->Types());
    builder.ast_ =
        builder.create<ast::Module>(program->AST().source, program->AST().GlobalDeclarations());
    builder.sem_ = sem::Info::Wrap(program->Sem());
    builder.symbols_.Wrap(program->Symbols());
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
    if (TINT_UNLIKELY(moved_)) {
        TINT_ICE(ProgramBuilder, const_cast<ProgramBuilder*>(this)->diagnostics_)
            << "Attempting to use ProgramBuilder after it has been moved";
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

ProgramBuilder::TypesBuilder::TypesBuilder(ProgramBuilder* pb) : builder(pb) {}

const ast::Statement* ProgramBuilder::WrapInStatement(const ast::Expression* expr) {
    // Create a temporary variable of inferred type from expr.
    return Decl(Let(symbols_.New(), expr));
}

const ast::VariableDeclStatement* ProgramBuilder::WrapInStatement(const ast::Variable* v) {
    return create<ast::VariableDeclStatement>(v);
}

const ast::Statement* ProgramBuilder::WrapInStatement(const ast::Statement* stmt) {
    return stmt;
}

const ast::Function* ProgramBuilder::WrapInFunction(utils::VectorRef<const ast::Statement*> stmts) {
    return Func("test_function", {}, ty.void_(), std::move(stmts),
                utils::Vector{
                    create<ast::StageAttribute>(ast::PipelineStage::kCompute),
                    WorkgroupSize(1_i, 1_i, 1_i),
                });
}

const constant::Value* ProgramBuilder::createSplatOrComposite(
    const type::Type* type,
    utils::VectorRef<const constant::Value*> elements) {
    if (elements.IsEmpty()) {
        return nullptr;
    }

    bool any_zero = false;
    bool all_zero = true;
    bool all_equal = true;
    auto* first = elements.Front();
    for (auto* el : elements) {
        if (!el) {
            return nullptr;
        }
        if (!any_zero && el->AnyZero()) {
            any_zero = true;
        }
        if (all_zero && !el->AllZero()) {
            all_zero = false;
        }
        if (all_equal && el != first) {
            if (!el->Equal(first)) {
                all_equal = false;
            }
        }
    }
    if (all_equal) {
        return create<constant::Splat>(type, elements[0], elements.Length());
    }

    return constant_nodes_.Create<constant::Composite>(type, std::move(elements), all_zero,
                                                       any_zero);
}

}  // namespace tint
