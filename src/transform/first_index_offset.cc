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

#include "src/transform/first_index_offset.h"

#include <memory>
#include <unordered_map>
#include <utility>

#include "src/ast/struct_block_decoration.h"
#include "src/program_builder.h"
#include "src/sem/function.h"
#include "src/sem/member_accessor_expression.h"
#include "src/sem/struct.h"
#include "src/sem/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::FirstIndexOffset::BindingPoint);
TINT_INSTANTIATE_TYPEINFO(tint::transform::FirstIndexOffset::Data);

namespace tint {
namespace transform {
namespace {

// Uniform buffer member names
constexpr char kFirstVertexName[] = "first_vertex_index";
constexpr char kFirstInstanceName[] = "first_instance_index";

}  // namespace

FirstIndexOffset::BindingPoint::BindingPoint() = default;
FirstIndexOffset::BindingPoint::BindingPoint(uint32_t b, uint32_t g)
    : binding(b), group(g) {}
FirstIndexOffset::BindingPoint::~BindingPoint() = default;

FirstIndexOffset::Data::Data(bool has_vtx_index,
                             bool has_inst_index,
                             uint32_t first_vtx_offset,
                             uint32_t first_inst_offset)
    : has_vertex_index(has_vtx_index),
      has_instance_index(has_inst_index),
      first_vertex_offset(first_vtx_offset),
      first_instance_offset(first_inst_offset) {}
FirstIndexOffset::Data::Data(const Data&) = default;
FirstIndexOffset::Data::~Data() = default;

FirstIndexOffset::FirstIndexOffset() = default;
FirstIndexOffset::~FirstIndexOffset() = default;

Output FirstIndexOffset::Run(const Program* in, const DataMap& data) {
  // Get the uniform buffer binding point
  uint32_t ub_binding = binding_;
  uint32_t ub_group = group_;
  if (auto* binding_point = data.Get<BindingPoint>()) {
    ub_binding = binding_point->binding;
    ub_group = binding_point->group;
  }

  ProgramBuilder out;
  CloneContext ctx(&out, in);

  // Map of builtin usages
  std::unordered_map<const sem::Variable*, const char*> builtin_vars;
  std::unordered_map<const sem::StructMember*, const char*> builtin_members;

  bool has_vertex_index = false;
  bool has_instance_index = false;

  // Traverse the AST scanning for builtin accesses via variables (includes
  // parameters) or structure member accesses.
  for (auto* node : in->ASTNodes().Objects()) {
    if (auto* var = node->As<ast::Variable>()) {
      for (ast::Decoration* dec : var->decorations()) {
        if (auto* builtin_dec = dec->As<ast::BuiltinDecoration>()) {
          ast::Builtin builtin = builtin_dec->value();
          if (builtin == ast::Builtin::kVertexIndex) {
            auto* sem_var = ctx.src->Sem().Get(var);
            builtin_vars.emplace(sem_var, kFirstVertexName);
            has_vertex_index = true;
          }
          if (builtin == ast::Builtin::kInstanceIndex) {
            auto* sem_var = ctx.src->Sem().Get(var);
            builtin_vars.emplace(sem_var, kFirstInstanceName);
            has_instance_index = true;
          }
        }
      }
    }
    if (auto* member = node->As<ast::StructMember>()) {
      for (ast::Decoration* dec : member->decorations()) {
        if (auto* builtin_dec = dec->As<ast::BuiltinDecoration>()) {
          ast::Builtin builtin = builtin_dec->value();
          if (builtin == ast::Builtin::kVertexIndex) {
            auto* sem_mem = ctx.src->Sem().Get(member);
            builtin_members.emplace(sem_mem, kFirstVertexName);
            has_vertex_index = true;
          }
          if (builtin == ast::Builtin::kInstanceIndex) {
            auto* sem_mem = ctx.src->Sem().Get(member);
            builtin_members.emplace(sem_mem, kFirstInstanceName);
            has_instance_index = true;
          }
        }
      }
    }
  }

  // Byte offsets on the uniform buffer
  uint32_t vertex_index_offset = 0;
  uint32_t instance_index_offset = 0;

  if (has_vertex_index || has_instance_index) {
    // Add uniform buffer members and calculate byte offsets
    uint32_t offset = 0;
    ast::StructMemberList members;
    if (has_vertex_index) {
      members.push_back(ctx.dst->Member(kFirstVertexName, ctx.dst->ty.u32()));
      vertex_index_offset = offset;
      offset += 4;
    }
    if (has_instance_index) {
      members.push_back(ctx.dst->Member(kFirstInstanceName, ctx.dst->ty.u32()));
      instance_index_offset = offset;
      offset += 4;
    }
    auto* struct_type =
        ctx.dst->Structure(ctx.dst->Sym(), std::move(members),
                           {ctx.dst->create<ast::StructBlockDecoration>()});

    // Create a global to hold the uniform buffer
    Symbol buffer_name = ctx.dst->Sym();
    ctx.dst->Global(buffer_name, struct_type, ast::StorageClass::kUniform,
                    nullptr,
                    ast::DecorationList{
                        ctx.dst->create<ast::BindingDecoration>(ub_binding),
                        ctx.dst->create<ast::GroupDecoration>(ub_group),
                    });

    // Fix up all references to the builtins with the offsets
    ctx.ReplaceAll([=, &ctx](ast::Expression* expr) -> ast::Expression* {
      if (auto* sem = ctx.src->Sem().Get(expr)) {
        if (auto* user = sem->As<sem::VariableUser>()) {
          auto it = builtin_vars.find(user->Variable());
          if (it != builtin_vars.end()) {
            return ctx.dst->Add(
                ctx.CloneWithoutTransform(expr),
                ctx.dst->MemberAccessor(buffer_name, it->second));
          }
        }
        if (auto* access = sem->As<sem::StructMemberAccess>()) {
          auto it = builtin_members.find(access->Member());
          if (it != builtin_members.end()) {
            return ctx.dst->Add(
                ctx.CloneWithoutTransform(expr),
                ctx.dst->MemberAccessor(buffer_name, it->second));
          }
        }
      }
      // Not interested in this experssion. Just clone.
      return nullptr;
    });
  }

  ctx.Clone();

  return Output(
      Program(std::move(out)),
      std::make_unique<Data>(has_vertex_index, has_instance_index,
                             vertex_index_offset, instance_index_offset));
}

}  // namespace transform
}  // namespace tint
