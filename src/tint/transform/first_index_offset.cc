// Copyright 2020 The Tint Authors.
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

#include "src/tint/transform/first_index_offset.h"

#include <memory>
#include <unordered_map>
#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/struct.h"
#include "src/tint/sem/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::FirstIndexOffset);
TINT_INSTANTIATE_TYPEINFO(tint::transform::FirstIndexOffset::BindingPoint);
TINT_INSTANTIATE_TYPEINFO(tint::transform::FirstIndexOffset::Data);

namespace tint::transform {
namespace {

// Uniform buffer member names
constexpr char kFirstVertexName[] = "first_vertex_index";
constexpr char kFirstInstanceName[] = "first_instance_index";

}  // namespace

FirstIndexOffset::BindingPoint::BindingPoint() = default;
FirstIndexOffset::BindingPoint::BindingPoint(uint32_t b, uint32_t g) : binding(b), group(g) {}
FirstIndexOffset::BindingPoint::~BindingPoint() = default;

FirstIndexOffset::Data::Data(bool has_vtx_or_inst_index)
    : has_vertex_or_instance_index(has_vtx_or_inst_index) {}
FirstIndexOffset::Data::Data(const Data&) = default;
FirstIndexOffset::Data::~Data() = default;

FirstIndexOffset::FirstIndexOffset() = default;
FirstIndexOffset::~FirstIndexOffset() = default;

bool FirstIndexOffset::ShouldRun(const Program* program, const DataMap&) const {
    for (auto* fn : program->AST().Functions()) {
        if (fn->PipelineStage() == ast::PipelineStage::kVertex) {
            return true;
        }
    }
    return false;
}

void FirstIndexOffset::Run(CloneContext& ctx, const DataMap& inputs, DataMap& outputs) const {
    // Get the uniform buffer binding point
    uint32_t ub_binding = binding_;
    uint32_t ub_group = group_;
    if (auto* binding_point = inputs.Get<BindingPoint>()) {
        ub_binding = binding_point->binding;
        ub_group = binding_point->group;
    }

    // Map of builtin usages
    std::unordered_map<const sem::Variable*, const char*> builtin_vars;
    std::unordered_map<const sem::StructMember*, const char*> builtin_members;

    bool has_vertex_or_instance_index = false;

    // Traverse the AST scanning for builtin accesses via variables (includes
    // parameters) or structure member accesses.
    for (auto* node : ctx.src->ASTNodes().Objects()) {
        if (auto* var = node->As<ast::Variable>()) {
            for (auto* attr : var->attributes) {
                if (auto* builtin_attr = attr->As<ast::BuiltinAttribute>()) {
                    ast::Builtin builtin = builtin_attr->builtin;
                    if (builtin == ast::Builtin::kVertexIndex) {
                        auto* sem_var = ctx.src->Sem().Get(var);
                        builtin_vars.emplace(sem_var, kFirstVertexName);
                        has_vertex_or_instance_index = true;
                    }
                    if (builtin == ast::Builtin::kInstanceIndex) {
                        auto* sem_var = ctx.src->Sem().Get(var);
                        builtin_vars.emplace(sem_var, kFirstInstanceName);
                        has_vertex_or_instance_index = true;
                    }
                }
            }
        }
        if (auto* member = node->As<ast::StructMember>()) {
            for (auto* attr : member->attributes) {
                if (auto* builtin_attr = attr->As<ast::BuiltinAttribute>()) {
                    ast::Builtin builtin = builtin_attr->builtin;
                    if (builtin == ast::Builtin::kVertexIndex) {
                        auto* sem_mem = ctx.src->Sem().Get(member);
                        builtin_members.emplace(sem_mem, kFirstVertexName);
                        has_vertex_or_instance_index = true;
                    }
                    if (builtin == ast::Builtin::kInstanceIndex) {
                        auto* sem_mem = ctx.src->Sem().Get(member);
                        builtin_members.emplace(sem_mem, kFirstInstanceName);
                        has_vertex_or_instance_index = true;
                    }
                }
            }
        }
    }

    if (has_vertex_or_instance_index) {
        // Add uniform buffer members and calculate byte offsets
        ast::StructMemberList members;
        members.push_back(ctx.dst->Member(kFirstVertexName, ctx.dst->ty.u32()));
        members.push_back(ctx.dst->Member(kFirstInstanceName, ctx.dst->ty.u32()));
        auto* struct_ = ctx.dst->Structure(ctx.dst->Sym(), std::move(members));

        // Create a global to hold the uniform buffer
        Symbol buffer_name = ctx.dst->Sym();
        ctx.dst->Global(buffer_name, ctx.dst->ty.Of(struct_), ast::StorageClass::kUniform, nullptr,
                        ast::AttributeList{
                            ctx.dst->create<ast::BindingAttribute>(ub_binding),
                            ctx.dst->create<ast::GroupAttribute>(ub_group),
                        });

        // Fix up all references to the builtins with the offsets
        ctx.ReplaceAll([=, &ctx](const ast::Expression* expr) -> const ast::Expression* {
            if (auto* sem = ctx.src->Sem().Get(expr)) {
                if (auto* user = sem->As<sem::VariableUser>()) {
                    auto it = builtin_vars.find(user->Variable());
                    if (it != builtin_vars.end()) {
                        return ctx.dst->Add(ctx.CloneWithoutTransform(expr),
                                            ctx.dst->MemberAccessor(buffer_name, it->second));
                    }
                }
                if (auto* access = sem->As<sem::StructMemberAccess>()) {
                    auto it = builtin_members.find(access->Member());
                    if (it != builtin_members.end()) {
                        return ctx.dst->Add(ctx.CloneWithoutTransform(expr),
                                            ctx.dst->MemberAccessor(buffer_name, it->second));
                    }
                }
            }
            // Not interested in this experssion. Just clone.
            return nullptr;
        });
    }

    ctx.Clone();

    outputs.Add<Data>(has_vertex_or_instance_index);
}

}  // namespace tint::transform
