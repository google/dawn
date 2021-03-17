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

#include "src/program_builder.h"
#include "src/semantic/variable.h"

namespace tint {
namespace transform {

Spirv::Spirv() = default;
Spirv::~Spirv() = default;

Transform::Output Spirv::Run(const Program* in) {
  ProgramBuilder out;
  CloneContext ctx(&out, in);
  HandleEntryPointIOTypes(ctx);
  ctx.Clone();

  // TODO(jrprice): Look into combining these transforms into a single clone.
  Program tmp(std::move(out));

  ProgramBuilder out2;
  CloneContext ctx2(&out2, &tmp);
  HandleSampleMaskBuiltins(ctx2);
  ctx2.Clone();

  return Output{Program(std::move(out2))};
}

void Spirv::HandleEntryPointIOTypes(CloneContext& ctx) const {
  // Hoist entry point parameters, return values, and struct members out to
  // global variables. Declare and construct struct parameters in the function
  // body. Replace entry point return statements with variable assignments.
  //
  // Before:
  // ```
  // struct FragmentInput {
  //   [[builtin(sample_index)]] sample_index : u32;
  //   [[builtin(sample_mask_in)]] sample_mask_in : u32;
  // };
  // struct FragmentOutput {
  //   [[builtin(frag_depth)]] depth: f32;
  //   [[builtin(sample_mask_out)]] mask_out : u32;
  // };
  //
  // [[stage(fragment)]]
  // fn fs_main(
  //   [[builtin(frag_coord)]] coord : vec4<f32>,
  //   samples : FragmentInput
  // ) -> FragmentOutput {
  //   return FragmentOutput(1.0, samples.sample_mask_in);
  // }
  // ```
  //
  // After:
  // ```
  // struct FragmentInput {
  //   sample_index : u32;
  //   sample_mask_in : u32;
  // };
  // struct FragmentOutput {
  //   depth: f32;
  //   mask_out : u32;
  // };
  //
  // [[builtin(frag_coord)]] var<in> coord : vec4<f32>,
  // [[builtin(sample_index)]] var<in> sample_index : u32,
  // [[builtin(sample_mask_in)]] var<in> sample_mask_in : u32,
  // [[builtin(frag_depth)]] var<out> depth: f32;
  // [[builtin(sample_mask_out)]] var<out> mask_out : u32;
  //
  // [[stage(fragment)]]
  // fn fs_main() -> void {
  //   const samples : FragmentInput(sample_index, sample_mask_in);
  //   depth = 1.0;
  //   mask_out = samples.sample_mask_in;
  //   return;
  // }
  // ```

  // TODO(jrprice): Hoist struct members decorated as entry point IO types out
  // of struct declarations, and redeclare the structs without the decorations.

  for (auto* func : ctx.src->AST().Functions()) {
    if (!func->IsEntryPoint()) {
      continue;
    }

    for (auto* param : func->params()) {
      // TODO(jrprice): Handle structures by moving the declaration and
      // construction to the function body.
      if (param->type()->Is<type::Struct>()) {
        TINT_UNIMPLEMENTED(ctx.dst->Diagnostics())
            << "structures as entry point parameters are not yet supported";
        continue;
      }

      // Create a new symbol for the global variable.
      auto var_symbol = ctx.dst->Symbols().New();
      // Create the global variable.
      ctx.dst->Global(var_symbol, ctx.Clone(param->type()),
                      ast::StorageClass::kInput, nullptr,
                      ctx.Clone(param->decorations()));

      // Replace all uses of the function parameter with the global variable.
      for (auto* user : ctx.src->Sem().Get(param)->Users()) {
        ctx.Replace<ast::Expression>(user->Declaration(),
                                     ctx.dst->Expr(var_symbol));
      }
    }

    // TODO(jrprice): Hoist the return type out to a global variable, and
    // replace return statements with variable assignments.
    if (!func->return_type()->Is<type::Void>()) {
      TINT_UNIMPLEMENTED(ctx.dst->Diagnostics())
          << "entry point return values are not yet supported";
      continue;
    }

    // Rewrite the function header to remove the parameters.
    // TODO(jrprice): Change return type to void when return values are handled.
    auto* new_func = ctx.dst->create<ast::Function>(
        func->source(), ctx.Clone(func->symbol()), ast::VariableList{},
        ctx.Clone(func->return_type()), ctx.Clone(func->body()),
        ctx.Clone(func->decorations()),
        ctx.Clone(func->return_type_decorations()));
    ctx.Replace(func, new_func);
  }
}

void Spirv::HandleSampleMaskBuiltins(CloneContext& ctx) const {
  // Find global variables decorated with [[builtin(sample_mask_{in,out})]] and
  // change their type from `u32` to `array<u32, 1>`, as required by Vulkan.
  //
  // Before:
  // ```
  // [[builtin(sample_mask_out)]] var<out> mask_out : u32;
  // fn main() -> void {
  //   mask_out = 1u;
  // }
  // ```
  // After:
  // ```
  // [[builtin(sample_mask_out)]] var<out> mask_out : array<u32, 1>;
  // fn main() -> void {
  //   mask_out[0] = 1u;
  // }
  // ```

  for (auto* var : ctx.src->AST().GlobalVariables()) {
    for (auto* deco : var->decorations()) {
      if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
        if (builtin->value() != ast::Builtin::kSampleMaskIn &&
            builtin->value() != ast::Builtin::kSampleMaskOut) {
          continue;
        }

        // Use the same name as the old variable.
        std::string var_name = ctx.src->Symbols().NameFor(var->symbol());
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

}  // namespace transform
}  // namespace tint
