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

#include "src/tint/lang/wgsl/ast/builder.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::ast {

Builder::VarOptions::~VarOptions() = default;
Builder::LetOptions::~LetOptions() = default;
Builder::ConstOptions::~ConstOptions() = default;
Builder::OverrideOptions::~OverrideOptions() = default;

Builder::Builder()
    : id_(GenerationID::New()),
      ast_(ast_nodes_.Create<ast::Module>(id_, AllocateNodeID(), Source{})) {}

Builder::Builder(Builder&& rhs)
    : id_(std::move(rhs.id_)),
      last_ast_node_id_(std::move(rhs.last_ast_node_id_)),
      ast_nodes_(std::move(rhs.ast_nodes_)),
      ast_(rhs.ast_),
      symbols_(std::move(rhs.symbols_)),
      diagnostics_(std::move(rhs.diagnostics_)) {
    rhs.MarkAsMoved();
}

Builder::~Builder() = default;

Builder& Builder::operator=(Builder&& rhs) {
    rhs.MarkAsMoved();
    AssertNotMoved();
    id_ = std::move(rhs.id_);
    last_ast_node_id_ = std::move(rhs.last_ast_node_id_);
    ast_nodes_ = std::move(rhs.ast_nodes_);
    ast_ = std::move(rhs.ast_);
    symbols_ = std::move(rhs.symbols_);
    diagnostics_ = std::move(rhs.diagnostics_);

    return *this;
}

bool Builder::IsValid() const {
    return !diagnostics_.contains_errors();
}

void Builder::MarkAsMoved() {
    AssertNotMoved();
    moved_ = true;
}

void Builder::AssertNotMoved() const {
    if (TINT_UNLIKELY(moved_)) {
        TINT_ICE() << "Attempting to use Builder after it has been moved";
    }
}

Builder::TypesBuilder::TypesBuilder(Builder* pb) : builder(pb) {}

const ast::Statement* Builder::WrapInStatement(const ast::Expression* expr) {
    // Create a temporary variable of inferred type from expr.
    return Decl(Let(symbols_.New(), expr));
}

const ast::VariableDeclStatement* Builder::WrapInStatement(const ast::Variable* v) {
    return create<ast::VariableDeclStatement>(v);
}

const ast::Statement* Builder::WrapInStatement(const ast::Statement* stmt) {
    return stmt;
}

const ast::Function* Builder::WrapInFunction(VectorRef<const ast::Statement*> stmts) {
    return Func("test_function", {}, ty.void_(), std::move(stmts),
                Vector{
                    create<ast::StageAttribute>(ast::PipelineStage::kCompute),
                    WorkgroupSize(1_i, 1_i, 1_i),
                });
}

}  // namespace tint::ast
