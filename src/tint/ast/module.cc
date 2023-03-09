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
#include "src/tint/switch.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Module);

namespace tint::ast {

Module::Module(ProgramID pid, NodeID nid, const Source& src) : Base(pid, nid, src) {}

Module::Module(ProgramID pid,
               NodeID nid,
               const Source& src,
               utils::VectorRef<const Node*> global_decls)
    : Base(pid, nid, src), global_declarations_(std::move(global_decls)) {
    for (auto* decl : global_declarations_) {
        if (decl == nullptr) {
            continue;
        }
        diag::List diags;
        BinGlobalDeclaration(decl, diags);
    }
}

Module::~Module() = default;

const TypeDecl* Module::LookupType(Symbol name) const {
    for (auto* ty : TypeDecls()) {
        if (ty->name->symbol == name) {
            return ty;
        }
    }
    return nullptr;
}

void Module::AddGlobalDeclaration(const tint::ast::Node* decl) {
    diag::List diags;
    BinGlobalDeclaration(decl, diags);
    global_declarations_.Push(decl);
}

void Module::BinGlobalDeclaration(const tint::ast::Node* decl, diag::List& diags) {
    Switch(
        decl,  //
        [&](const TypeDecl* type) {
            TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, type, program_id);
            type_decls_.Push(type);
        },
        [&](const Function* func) {
            TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, func, program_id);
            functions_.Push(func);
        },
        [&](const Variable* var) {
            TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, var, program_id);
            global_variables_.Push(var);
        },
        [&](const DiagnosticDirective* diagnostic) {
            TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, diagnostic, program_id);
            diagnostic_directives_.Push(diagnostic);
        },
        [&](const Enable* enable) {
            TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, enable, program_id);
            enables_.Push(enable);
        },
        [&](const ConstAssert* assertion) {
            TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, assertion, program_id);
            const_asserts_.Push(assertion);
        },
        [&](Default) { TINT_ICE(AST, diags) << "Unknown global declaration type"; });
}

void Module::AddDiagnosticDirective(const DiagnosticDirective* directive) {
    TINT_ASSERT(AST, directive);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, directive, program_id);
    global_declarations_.Push(directive);
    diagnostic_directives_.Push(directive);
}

void Module::AddEnable(const Enable* enable) {
    TINT_ASSERT(AST, enable);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, enable, program_id);
    global_declarations_.Push(enable);
    enables_.Push(enable);
}

void Module::AddGlobalVariable(const Variable* var) {
    TINT_ASSERT(AST, var);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, var, program_id);
    global_variables_.Push(var);
    global_declarations_.Push(var);
}

void Module::AddConstAssert(const ConstAssert* assertion) {
    TINT_ASSERT(AST, assertion);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, assertion, program_id);
    const_asserts_.Push(assertion);
    global_declarations_.Push(assertion);
}

void Module::AddTypeDecl(const TypeDecl* type) {
    TINT_ASSERT(AST, type);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, type, program_id);
    type_decls_.Push(type);
    global_declarations_.Push(type);
}

void Module::AddFunction(const Function* func) {
    TINT_ASSERT(AST, func);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, func, program_id);
    functions_.Push(func);
    global_declarations_.Push(func);
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
    type_decls_.Clear();
    functions_.Clear();
    global_variables_.Clear();
    enables_.Clear();
    diagnostic_directives_.Clear();

    for (auto* decl : global_declarations_) {
        if (TINT_UNLIKELY(!decl)) {
            TINT_ICE(AST, ctx->dst->Diagnostics()) << "src global declaration was nullptr";
            continue;
        }
        BinGlobalDeclaration(decl, ctx->dst->Diagnostics());
    }
}

}  // namespace tint::ast
