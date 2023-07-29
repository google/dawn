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

#include "src/tint/lang/wgsl/ast/transform/first_index_offset.h"

#include <memory>
#include <unordered_map>
#include <utility>

#include "src/tint/lang/core/builtin/builtin_value.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/lang/wgsl/sem/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::FirstIndexOffset);
TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::FirstIndexOffset::BindingPoint);
TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::FirstIndexOffset::Data);

namespace tint::ast::transform {
namespace {

// Uniform buffer member names
constexpr char kFirstVertexName[] = "first_vertex_index";
constexpr char kFirstInstanceName[] = "first_instance_index";

bool ShouldRun(const Program* program) {
    for (auto* fn : program->AST().Functions()) {
        if (fn->PipelineStage() == PipelineStage::kVertex) {
            return true;
        }
    }
    return false;
}

}  // namespace

FirstIndexOffset::BindingPoint::BindingPoint() = default;
FirstIndexOffset::BindingPoint::BindingPoint(uint32_t b, uint32_t g) : binding(b), group(g) {}
FirstIndexOffset::BindingPoint::~BindingPoint() = default;

FirstIndexOffset::Data::Data(bool has_vtx_index, bool has_inst_index)
    : has_vertex_index(has_vtx_index), has_instance_index(has_inst_index) {}
FirstIndexOffset::Data::Data(const Data&) = default;
FirstIndexOffset::Data::~Data() = default;

FirstIndexOffset::FirstIndexOffset() = default;
FirstIndexOffset::~FirstIndexOffset() = default;

Transform::ApplyResult FirstIndexOffset::Apply(const Program* src,
                                               const DataMap& inputs,
                                               DataMap& outputs) const {
    if (!ShouldRun(src)) {
        return SkipTransform;
    }

    ProgramBuilder b;
    program::CloneContext ctx{&b, src, /* auto_clone_symbols */ true};

    // Get the uniform buffer binding point
    uint32_t ub_binding = binding_;
    uint32_t ub_group = group_;
    if (auto* binding_point = inputs.Get<BindingPoint>()) {
        ub_binding = binding_point->binding;
        ub_group = binding_point->group;
    }

    // Map of builtin usages
    std::unordered_map<const sem::Variable*, const char*> builtin_vars;
    std::unordered_map<const type::StructMember*, const char*> builtin_members;

    bool has_vertex_index = false;
    bool has_instance_index = false;

    // Traverse the AST scanning for builtin accesses via variables (includes
    // parameters) or structure member accesses.
    for (auto* node : ctx.src->ASTNodes().Objects()) {
        if (auto* var = node->As<Variable>()) {
            for (auto* attr : var->attributes) {
                if (auto* builtin_attr = attr->As<BuiltinAttribute>()) {
                    builtin::BuiltinValue builtin = src->Sem().Get(builtin_attr)->Value();
                    if (builtin == builtin::BuiltinValue::kVertexIndex) {
                        auto* sem_var = ctx.src->Sem().Get(var);
                        builtin_vars.emplace(sem_var, kFirstVertexName);
                        has_vertex_index = true;
                    }
                    if (builtin == builtin::BuiltinValue::kInstanceIndex) {
                        auto* sem_var = ctx.src->Sem().Get(var);
                        builtin_vars.emplace(sem_var, kFirstInstanceName);
                        has_instance_index = true;
                    }
                }
            }
        }
        if (auto* member = node->As<StructMember>()) {
            for (auto* attr : member->attributes) {
                if (auto* builtin_attr = attr->As<BuiltinAttribute>()) {
                    builtin::BuiltinValue builtin = src->Sem().Get(builtin_attr)->Value();
                    if (builtin == builtin::BuiltinValue::kVertexIndex) {
                        auto* sem_mem = ctx.src->Sem().Get(member);
                        builtin_members.emplace(sem_mem, kFirstVertexName);
                        has_vertex_index = true;
                    }
                    if (builtin == builtin::BuiltinValue::kInstanceIndex) {
                        auto* sem_mem = ctx.src->Sem().Get(member);
                        builtin_members.emplace(sem_mem, kFirstInstanceName);
                        has_instance_index = true;
                    }
                }
            }
        }
    }

    if (has_vertex_index || has_instance_index) {
        // Add uniform buffer members and calculate byte offsets
        tint::Vector<const StructMember*, 8> members;
        members.Push(b.Member(kFirstVertexName, b.ty.u32()));
        members.Push(b.Member(kFirstInstanceName, b.ty.u32()));
        auto* struct_ = b.Structure(b.Sym(), std::move(members));

        // Create a global to hold the uniform buffer
        Symbol buffer_name = b.Sym();
        b.GlobalVar(buffer_name, b.ty.Of(struct_), builtin::AddressSpace::kUniform,
                    tint::Vector{
                        b.Binding(AInt(ub_binding)),
                        b.Group(AInt(ub_group)),
                    });

        // Fix up all references to the builtins with the offsets
        ctx.ReplaceAll([=, &ctx](const Expression* expr) -> const Expression* {
            if (auto* sem = ctx.src->Sem().GetVal(expr)) {
                if (auto* user = sem->UnwrapLoad()->As<sem::VariableUser>()) {
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
            // Not interested in this expression. Just clone.
            return nullptr;
        });
    }

    outputs.Add<Data>(has_vertex_index, has_instance_index);

    ctx.Clone();
    return Program(std::move(b));
}

}  // namespace tint::ast::transform
