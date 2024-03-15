// Copyright 2024 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <utility>

#include "src/tint/lang/wgsl/ast/identifier.h"
#include "src/tint/lang/wgsl/ast/identifier_expression.h"
#include "src/tint/lang/wgsl/ast/member_accessor_expression.h"
#include "src/tint/lang/wgsl/ls/file.h"
#include "src/tint/lang/wgsl/ls/utils.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/function_expression.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/lang/wgsl/sem/type_expression.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::wgsl::ls {

File::File(std::unique_ptr<Source::File>&& source_, int64_t version_, Program program_)
    : source(std::move(source_)), version(version_), program(std::move(program_)) {
    nodes.reserve(program.ASTNodes().Count());
    for (auto* node : program.ASTNodes().Objects()) {
        nodes.push_back(node);
    }
    std::stable_sort(nodes.begin(), nodes.end(), [](const ast::Node* a, const ast::Node* b) {
        if (a->source.range.begin < b->source.range.begin) {
            return true;
        }
        if (a->source.range.begin > b->source.range.begin) {
            return false;
        }
        if (a->source.range.end < b->source.range.end) {
            return true;
        }
        if (a->source.range.end > b->source.range.end) {
            return false;
        }
        return false;
    });
}

std::vector<Source::Range> File::References(Source::Location l, bool include_declaration) {
    std::vector<Source::Range> references;
    auto variable_refs = [&](const sem::Variable* v) {
        if (include_declaration) {
            references.push_back(v->Declaration()->name->source.range);
        }
        for (auto* user : v->Users()) {
            references.push_back(user->Declaration()->source.range);
        }
    };
    auto function_refs = [&](const sem::Function* f) {
        if (include_declaration) {
            references.push_back(f->Declaration()->name->source.range);
        }
        for (auto* call : f->CallSites()) {
            references.push_back(call->Declaration()->target->source.range);
        }
    };
    auto struct_member = [&](const sem::StructMember* m) {
        if (include_declaration) {
            references.push_back(m->Declaration()->name->source.range);
        }
        for (auto* node : nodes) {
            if (auto* access = program.Sem().Get<sem::StructMemberAccess>(node)) {
                if (access->Member() == m) {
                    references.push_back(access->Declaration()->member->source.range);
                }
            }
        }
    };
    auto struct_ = [&](const sem::Struct* s) {
        if (include_declaration) {
            references.push_back(s->Declaration()->name->source.range);
        }
        for (auto* node : nodes) {
            if (auto* ident = node->As<ast::IdentifierExpression>()) {
                if (program.Sem().Get<sem::Struct>(node) == s) {
                    references.push_back(ident->source.range);
                }
            }
        }
    };

    Switch(
        Unwrap(NodeAt<CastableBase>(l)),  //
        [&](const sem::VariableUser* v) { variable_refs(v->Variable()); },
        [&](const sem::Variable* v) { variable_refs(v); },
        [&](const sem::FunctionExpression* e) { function_refs(e->Function()); },
        [&](const sem::Function* f) { function_refs(f); },
        [&](const sem::StructMemberAccess* a) {
            if (auto* member = a->Member()->As<sem::StructMember>()) {
                struct_member(member);
            }
        },
        [&](const sem::StructMember* m) { struct_member(m); },
        [&](const sem::TypeExpression* te) {
            return Switch(te->Type(),  //
                          [&](const sem::Struct* s) { return struct_(s); });
        });
    return references;
}

std::optional<File::TextAndRange> File::Definition(Source::Location l) {
    auto* ident = Switch<const ast::Identifier*>(
        Unwrap(NodeAt<CastableBase>(l)),  //
        [&](const sem::VariableUser* v) { return v->Variable()->Declaration()->name; },
        [&](const sem::Variable* v) { return v->Declaration()->name; },
        [&](const sem::FunctionExpression* f) { return f->Function()->Declaration()->name; },
        [&](const sem::Function* f) { return f->Declaration()->name; },
        [&](const sem::StructMemberAccess* a) -> const ast::Identifier* {
            if (auto* member = a->Member()->As<sem::StructMember>()) {
                return member->Declaration()->name;
            }
            return nullptr;
        },
        [&](const sem::StructMember* m) { return m->Declaration()->name; },
        [&](const sem::TypeExpression* te) {
            return Switch(te->Type(),  //
                          [&](const sem::Struct* s) { return s->Declaration()->name; });
        });
    if (ident) {
        return TextAndRange{ident->symbol.Name(), ident->source.range};
    }
    return std::nullopt;
}

}  // namespace tint::wgsl::ls
