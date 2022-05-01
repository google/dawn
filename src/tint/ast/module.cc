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

#include "src/tint/ast/module.h"

#include <utility>

#include "src/tint/ast/type_decl.h"
#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Module);

namespace tint::ast {

Module::Module(ProgramID pid, const Source& src) : Base(pid, src) {}

Module::Module(ProgramID pid, const Source& src, std::vector<const ast::Node*> global_decls)
    : Base(pid, src), global_declarations_(std::move(global_decls)) {
    for (auto* decl : global_declarations_) {
        if (decl == nullptr) {
            continue;
        }
        diag::List diags;
        BinGlobalDeclaration(decl, diags);
    }
}

Module::~Module() = default;

const ast::TypeDecl* Module::LookupType(Symbol name) const {
    for (auto* ty : TypeDecls()) {
        if (ty->name == name) {
            return ty;
        }
    }
    return nullptr;
}

void Module::AddGlobalDeclaration(const tint::ast::Node* decl) {
    diag::List diags;
    BinGlobalDeclaration(decl, diags);
    global_declarations_.emplace_back(decl);
}

void Module::BinGlobalDeclaration(const tint::ast::Node* decl, diag::List& diags) {
    Switch(
        decl,  //
        [&](const ast::TypeDecl* type) {
            TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, type, program_id);
            type_decls_.push_back(type);
        },
        [&](const Function* func) {
            TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, func, program_id);
            functions_.push_back(func);
        },
        [&](const Variable* var) {
            TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, var, program_id);
            global_variables_.push_back(var);
        },
        [&](const Enable* ext) {
            TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, ext, program_id);
            extensions_.insert(ext->kind);
        },
        [&](Default) { TINT_ICE(AST, diags) << "Unknown global declaration type"; });
}

void Module::AddEnable(const ast::Enable* ext) {
    TINT_ASSERT(AST, ext);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, ext, program_id);
    global_declarations_.push_back(ext);
    extensions_.insert(ext->kind);
}

void Module::AddGlobalVariable(const ast::Variable* var) {
    TINT_ASSERT(AST, var);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, var, program_id);
    global_variables_.push_back(var);
    global_declarations_.push_back(var);
}

void Module::AddTypeDecl(const ast::TypeDecl* type) {
    TINT_ASSERT(AST, type);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, type, program_id);
    type_decls_.push_back(type);
    global_declarations_.push_back(type);
}

void Module::AddFunction(const ast::Function* func) {
    TINT_ASSERT(AST, func);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, func, program_id);
    functions_.push_back(func);
    global_declarations_.push_back(func);
}

const Module* Module::Clone(CloneContext* ctx) const {
    auto* out = ctx->dst->create<Module>();
    out->Copy(ctx, this);
    return out;
}

void Module::Copy(CloneContext* ctx, const Module* src) {
    ctx->Clone(global_declarations_, src->global_declarations_);

    // During the clone, declarations may have been placed into the module.
    // Clear everything out, as we're about to re-bin the declarations.
    type_decls_.clear();
    functions_.clear();
    global_variables_.clear();
    extensions_.clear();

    for (auto* decl : global_declarations_) {
        if (!decl) {
            TINT_ICE(AST, ctx->dst->Diagnostics()) << "src global declaration was nullptr";
            continue;
        }
        BinGlobalDeclaration(decl, ctx->dst->Diagnostics());
    }
}

}  // namespace tint::ast
