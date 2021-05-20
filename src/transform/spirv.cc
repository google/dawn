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

#include "src/transform/spirv.h"

#include <string>
#include <utility>

#include "src/ast/call_statement.h"
#include "src/ast/return_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/program_builder.h"
#include "src/sem/block_statement.h"
#include "src/sem/function.h"
#include "src/sem/statement.h"
#include "src/sem/struct.h"
#include "src/sem/variable.h"
#include "src/transform/external_texture_transform.h"
#include "src/transform/manager.h"
#include "src/transform/var_for_dynamic_index.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::Spirv::Config);

namespace tint {
namespace transform {

Spirv::Spirv() = default;
Spirv::~Spirv() = default;

Output Spirv::Run(const Program* in, const DataMap& data) {
  Manager manager;
  manager.Add<ExternalTextureTransform>();
  manager.Add<VarForDynamicIndex>();
  auto transformedInput = manager.Run(in, data);

  auto* cfg = data.Get<Config>();

  ProgramBuilder out;
  CloneContext ctx(&out, &transformedInput.program);
  HandleEntryPointIOTypes(ctx);
  ctx.Clone();

  // TODO(jrprice): Look into combining these transforms into a single clone.
  Program tmp(std::move(out));

  ProgramBuilder out2;
  CloneContext ctx2(&out2, &tmp);
  HandleSampleMaskBuiltins(ctx2);
  AddEmptyEntryPoint(ctx2);
  if (cfg && cfg->emit_vertex_point_size) {
    EmitVertexPointSize(ctx2);
  }
  ctx2.Clone();

  return Output{Program(std::move(out2))};
}

void Spirv::HandleEntryPointIOTypes(CloneContext& ctx) const {
  // Hoist entry point parameters, return values, and struct members out to
  // global variables. Declare and construct struct parameters in the function
  // body. Replace entry point return statements with calls to a function that
  // assigns the return value to the global output variables.
  //
  // Before:
  // ```
  // struct FragmentInput {
  //   [[builtin(sample_index)]] sample_index : u32;
  //   [[builtin(sample_mask)]] sample_mask : u32;
  // };
  // struct FragmentOutput {
  //   [[builtin(frag_depth)]] depth: f32;
  //   [[builtin(sample_mask)]] mask_out : u32;
  // };
  //
  // [[stage(fragment)]]
  // fn frag_main(
  //   [[builtin(position)]] coord : vec4<f32>,
  //   samples : FragmentInput
  // ) -> FragmentOutput {
  //   var output : FragmentOutput = FragmentOutput(1.0,
  //                                                samples.sample_mask);
  //   return output;
  // }
  // ```
  //
  // After:
  // ```
  // struct FragmentInput {
  //   sample_index : u32;
  //   sample_mask : u32;
  // };
  // struct FragmentOutput {
  //   depth: f32;
  //   mask_out : u32;
  // };
  //
  // [[builtin(position)]] var<in> coord : vec4<f32>,
  // [[builtin(sample_index)]] var<in> sample_index : u32,
  // [[builtin(sample_mask)]] var<in> sample_mask : u32,
  // [[builtin(frag_depth)]] var<out> depth: f32;
  // [[builtin(sample_mask)]] var<out> mask_out : u32;
  //
  // fn frag_main_ret(retval : FragmentOutput) {
  //   depth = reval.depth;
  //   mask_out = retval.mask_out;
  // }
  //
  // [[stage(fragment)]]
  // fn frag_main() {
  //   let samples : FragmentInput(sample_index, sample_mask);
  //   var output : FragmentOutput = FragmentOutput(1.0,
  //                                                samples.sample_mask);
  //   frag_main_ret(output);
  //   return;
  // }
  // ```

  // Strip entry point IO decorations from struct declarations.
  for (auto* ty : ctx.src->AST().ConstructedTypes()) {
    if (auto* struct_ty = ty->As<ast::Struct>()) {
      // Build new list of struct members without entry point IO decorations.
      ast::StructMemberList new_struct_members;
      for (auto* member : struct_ty->members()) {
        ast::DecorationList new_decorations = RemoveDecorations(
            &ctx, member->decorations(), [](const ast::Decoration* deco) {
              return deco
                  ->IsAnyOf<ast::BuiltinDecoration, ast::LocationDecoration>();
            });
        new_struct_members.push_back(
            ctx.dst->Member(ctx.Clone(member->symbol()),
                            ctx.Clone(member->type()), new_decorations));
      }

      // Redeclare the struct.
      auto new_struct_name = ctx.Clone(struct_ty->name());
      auto* new_struct =
          ctx.dst->create<ast::Struct>(new_struct_name, new_struct_members,
                                       ctx.Clone(struct_ty->decorations()));
      ctx.Replace(struct_ty, new_struct);
    }
  }

  for (auto* func_ast : ctx.src->AST().Functions()) {
    if (!func_ast->IsEntryPoint()) {
      continue;
    }
    auto* func = ctx.src->Sem().Get(func_ast);

    for (auto* param : func->Parameters()) {
      Symbol new_var = HoistToInputVariables(
          ctx, func_ast, param->Type(), param->Declaration()->type(),
          param->Declaration()->decorations());

      // Replace all uses of the function parameter with the new variable.
      for (auto* user : param->Users()) {
        ctx.Replace<ast::Expression>(user->Declaration(),
                                     ctx.dst->Expr(new_var));
      }
    }

    if (!func->ReturnType()->Is<sem::Void>()) {
      ast::StatementList stores;
      auto store_value_symbol = ctx.dst->Sym();
      HoistToOutputVariables(
          ctx, func_ast, func->ReturnType(), func_ast->return_type(),
          func_ast->return_type_decorations(), {}, store_value_symbol, stores);

      // Create a function that writes a return value to all output variables.
      auto* store_value = ctx.dst->Param(store_value_symbol,
                                         ctx.Clone(func_ast->return_type()));
      auto return_func_symbol = ctx.dst->Sym();
      auto* return_func = ctx.dst->create<ast::Function>(
          return_func_symbol, ast::VariableList{store_value},
          ctx.dst->ty.void_(), ctx.dst->create<ast::BlockStatement>(stores),
          ast::DecorationList{}, ast::DecorationList{});
      ctx.InsertBefore(ctx.src->AST().GlobalDeclarations(), func_ast,
                       return_func);

      // Replace all return statements with calls to the output function.
      for (auto* ret : func->ReturnStatements()) {
        auto* ret_sem = ctx.src->Sem().Get(ret);
        auto* call = ctx.dst->Call(return_func_symbol, ctx.Clone(ret->value()));
        ctx.InsertBefore(ret_sem->Block()->Declaration()->statements(), ret,
                         ctx.dst->create<ast::CallStatement>(call));
        ctx.Replace(ret, ctx.dst->Return());
      }
    }

    // Rewrite the function header to remove the parameters and return value.
    auto name = ctx.Clone(func_ast->symbol());
    auto* body = ctx.Clone(func_ast->body());
    auto decos = ctx.Clone(func_ast->decorations());
    auto* new_func = ctx.dst->create<ast::Function>(
        func_ast->source(), name, ast::VariableList{}, ctx.dst->ty.void_(),
        body, decos, ast::DecorationList{});
    ctx.Replace(func_ast, new_func);
  }
}

void Spirv::HandleSampleMaskBuiltins(CloneContext& ctx) const {
  // Find global variables decorated with [[builtin(sample_mask)]] and
  // change their type from `u32` to `array<u32, 1>`, as required by Vulkan.
  //
  // Before:
  // ```
  // [[builtin(sample_mask)]] var<out> mask_out : u32;
  // fn main() {
  //   mask_out = 1u;
  // }
  // ```
  // After:
  // ```
  // [[builtin(sample_mask)]] var<out> mask_out : array<u32, 1>;
  // fn main() {
  //   mask_out[0] = 1u;
  // }
  // ```

  for (auto* var : ctx.src->AST().GlobalVariables()) {
    for (auto* deco : var->decorations()) {
      if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
        if (builtin->value() != ast::Builtin::kSampleMask &&
            builtin->value() != ast::Builtin::kSampleMaskIn &&
            builtin->value() != ast::Builtin::kSampleMaskOut) {
          continue;
        }

        // Use the same name as the old variable.
        auto var_name = ctx.Clone(var->symbol());
        // Use `array<u32, 1>` for the new variable.
        auto* type = ctx.dst->ty.array(ctx.dst->ty.u32(), 1u);
        // Create the new variable.
        auto* var_arr = ctx.dst->Var(var->source(), var_name, type,
                                     var->declared_storage_class(), nullptr,
                                     ctx.Clone(var->decorations()));
        // Replace the variable with the arrayed version.
        ctx.Replace(var, var_arr);

        // Replace all uses of the old variable with `var_arr[0]`.
        for (auto* user : ctx.src->Sem().Get(var)->Users()) {
          auto* new_ident = ctx.dst->IndexAccessor(
              ctx.dst->Expr(var_arr->symbol()), ctx.dst->Expr(0));
          ctx.Replace<ast::Expression>(user->Declaration(), new_ident);
        }
      }
    }
  }
}

void Spirv::EmitVertexPointSize(CloneContext& ctx) const {
  // No-op if there are no vertex stages in the module.
  if (!ctx.src->AST().Functions().HasStage(ast::PipelineStage::kVertex)) {
    return;
  }

  // Create a module-scope pointsize builtin output variable.
  Symbol pointsize = ctx.dst->Symbols().New("tint_pointsize");
  ctx.dst->Global(pointsize, ctx.dst->ty.f32(), ast::StorageClass::kOutput,
                  nullptr, {ctx.dst->Builtin(ast::Builtin::kPointSize)});

  // Assign 1.0 to the global at the start of all vertex shader entry points.
  ctx.ReplaceAll([&ctx, pointsize](ast::Function* func) -> ast::Function* {
    if (func->pipeline_stage() != ast::PipelineStage::kVertex) {
      return nullptr;
    }
    return CloneWithStatementsAtStart(&ctx, func,
                                      {
                                          ctx.dst->Assign(pointsize, 1.0f),
                                      });
  });
}

void Spirv::AddEmptyEntryPoint(CloneContext& ctx) const {
  for (auto* func : ctx.src->AST().Functions()) {
    if (func->IsEntryPoint()) {
      return;
    }
  }
  ctx.dst->Func(ctx.dst->Sym("unused_entry_point"), {}, ctx.dst->ty.void_(), {},
                {ctx.dst->Stage(ast::PipelineStage::kCompute)});
}

Symbol Spirv::HoistToInputVariables(
    CloneContext& ctx,
    const ast::Function* func,
    sem::Type* ty,
    ast::Type* declared_ty,
    const ast::DecorationList& decorations) const {
  if (!ty->Is<sem::Struct>()) {
    // Base case: create a global variable and return.
    ast::DecorationList new_decorations =
        RemoveDecorations(&ctx, decorations, [](const ast::Decoration* deco) {
          return !deco->IsAnyOf<ast::BuiltinDecoration,
                                ast::LocationDecoration>();
        });
    auto global_var_symbol = ctx.dst->Sym();
    auto* global_var =
        ctx.dst->Var(global_var_symbol, ctx.Clone(declared_ty),
                     ast::StorageClass::kInput, nullptr, new_decorations);
    ctx.InsertBefore(ctx.src->AST().GlobalDeclarations(), func, global_var);
    return global_var_symbol;
  }

  // Recurse into struct members and build the initializer list.
  std::vector<Symbol> init_value_names;
  auto* struct_ty = ty->As<sem::Struct>();
  for (auto* member : struct_ty->Members()) {
    auto member_var = HoistToInputVariables(
        ctx, func, member->Type(), member->Declaration()->type(),
        member->Declaration()->decorations());
    init_value_names.emplace_back(member_var);
  }

  auto func_var_symbol = ctx.dst->Sym();
  if (func->body()->empty()) {
    // The return value should never get used.
    return func_var_symbol;
  }

  ast::ExpressionList init_values;
  for (auto name : init_value_names) {
    init_values.push_back(ctx.dst->Expr(name));
  }

  // Create a function-scope variable for the struct.
  auto* initializer = ctx.dst->Construct(ctx.Clone(declared_ty), init_values);
  auto* func_var =
      ctx.dst->Const(func_var_symbol, ctx.Clone(declared_ty), initializer);
  ctx.InsertBefore(func->body()->statements(), *func->body()->begin(),
                   ctx.dst->WrapInStatement(func_var));
  return func_var_symbol;
}

void Spirv::HoistToOutputVariables(CloneContext& ctx,
                                   const ast::Function* func,
                                   sem::Type* ty,
                                   ast::Type* declared_ty,
                                   const ast::DecorationList& decorations,
                                   std::vector<Symbol> member_accesses,
                                   Symbol store_value,
                                   ast::StatementList& stores) const {
  // Base case.
  if (!ty->Is<sem::Struct>()) {
    // Create a global variable.
    ast::DecorationList new_decorations =
        RemoveDecorations(&ctx, decorations, [](const ast::Decoration* deco) {
          return !deco->IsAnyOf<ast::BuiltinDecoration,
                                ast::LocationDecoration>();
        });
    auto global_var_symbol = ctx.dst->Sym();
    auto* global_var =
        ctx.dst->Var(global_var_symbol, ctx.Clone(declared_ty),
                     ast::StorageClass::kOutput, nullptr, new_decorations);
    ctx.InsertBefore(ctx.src->AST().GlobalDeclarations(), func, global_var);

    // Create the assignment instruction.
    ast::Expression* rhs = ctx.dst->Expr(store_value);
    for (auto member : member_accesses) {
      rhs = ctx.dst->MemberAccessor(rhs, member);
    }
    stores.push_back(ctx.dst->Assign(ctx.dst->Expr(global_var_symbol), rhs));

    return;
  }

  // Recurse into struct members.
  auto* struct_ty = ty->As<sem::Struct>();
  for (auto* member : struct_ty->Members()) {
    member_accesses.push_back(ctx.Clone(member->Declaration()->symbol()));
    HoistToOutputVariables(ctx, func, member->Type(),
                           member->Declaration()->type(),
                           member->Declaration()->decorations(),
                           member_accesses, store_value, stores);
    member_accesses.pop_back();
  }
}

Spirv::Config::Config(bool emit_vps) : emit_vertex_point_size(emit_vps) {}

Spirv::Config::Config(const Config&) = default;
Spirv::Config::~Config() = default;
Spirv::Config& Spirv::Config::operator=(const Config&) = default;

}  // namespace transform
}  // namespace tint
