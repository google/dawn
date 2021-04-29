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

#include "src/transform/emit_vertex_point_size.h"

#include <unordered_map>
#include <utility>

#include "src/program_builder.h"
#include "src/sem/function.h"
#include "src/sem/statement.h"
#include "src/utils/get_or_create.h"

namespace tint {
namespace transform {

EmitVertexPointSize::EmitVertexPointSize() = default;
EmitVertexPointSize::~EmitVertexPointSize() = default;

Output EmitVertexPointSize::Run(const Program* in, const DataMap&) {
  ProgramBuilder out;
  CloneContext ctx(&out, in);

  std::unordered_map<sem::Type*, sem::StructType*> struct_map;
  for (auto* func : in->AST().Functions()) {
    if (func->pipeline_stage() != ast::PipelineStage::kVertex) {
      continue;
    }

    auto* sem_func = in->Sem().Get(func);

    // Create a struct for the return type that includes a point size member.
    auto* new_struct =
        utils::GetOrCreate(struct_map, sem_func->ReturnType(), [&]() {
          // Gather struct members.
          ast::StructMemberList new_struct_members;
          if (auto* struct_ty = sem_func->ReturnType()->As<sem::StructType>()) {
            for (auto* member : struct_ty->impl()->members()) {
              new_struct_members.push_back(ctx.Clone(member));
            }
          } else {
            auto* ret_type = ctx.Clone(sem_func->ReturnType());
            auto ret_type_decos = ctx.Clone(func->return_type_decorations());
            new_struct_members.push_back(
                out.Member("position", ret_type, std::move(ret_type_decos)));
          }

          // Append a new member for the point size.
          new_struct_members.push_back(
              out.Member(out.Symbols().New("tint_pointsize"), out.ty.f32(),
                         {out.Builtin(ast::Builtin::kPointSize)}));

          // Create the new output struct.
          return out.Structure(out.Sym(), new_struct_members);
        });

    // Replace return values using new output struct type constructors.
    for (auto* ret : sem_func->ReturnStatements()) {
      auto* ret_sem = in->Sem().Get(ret);

      ast::ExpressionList new_ret_values;
      if (auto* struct_ty = sem_func->ReturnType()->As<sem::StructType>()) {
        std::function<ast::Expression*()> ret_value = [&]() {
          return ctx.Clone(ret->value());
        };

        if (!ret->value()->Is<ast::IdentifierExpression>()) {
          // Capture the original return value in a local temporary.
          auto* new_struct_ty = ctx.Clone(struct_ty);
          auto* temp = out.Const(out.Sym(), new_struct_ty, ret_value());
          ctx.InsertBefore(ret_sem->Block()->statements(), ret, out.Decl(temp));
          ret_value = [&, temp]() { return out.Expr(temp); };
        }

        for (auto* member : struct_ty->impl()->members()) {
          auto member_sym = ctx.Clone(member->symbol());
          new_ret_values.push_back(out.MemberAccessor(ret_value(), member_sym));
        }
      } else {
        new_ret_values.push_back(ctx.Clone(ret->value()));
      }

      // Append the point size and replace the return statement.
      new_ret_values.push_back(out.Expr(1.f));
      ctx.Replace(ret, out.Return(ret->source(),
                                  out.Construct(new_struct, new_ret_values)));
    }

    // Rewrite the function header with the new return type.
    auto func_sym = ctx.Clone(func->symbol());
    auto params = ctx.Clone(func->params());
    auto* body = ctx.Clone(func->body());
    auto decos = ctx.Clone(func->decorations());
    auto* new_func = out.create<ast::Function>(
        func->source(), func_sym, std::move(params), new_struct, body,
        std::move(decos), ast::DecorationList{});
    ctx.Replace(func, new_func);
  }

  ctx.Clone();

  return Output(Program(std::move(out)));
}

}  // namespace transform
}  // namespace tint
