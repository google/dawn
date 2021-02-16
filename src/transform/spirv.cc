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

#include "src/ast/variable.h"
#include "src/program_builder.h"
#include "src/semantic/expression.h"
#include "src/semantic/variable.h"

namespace tint {
namespace transform {

Spirv::Spirv() = default;
Spirv::~Spirv() = default;

Transform::Output Spirv::Run(const Program* in) {
  ProgramBuilder out;
  CloneContext ctx(&out, in);
  HandleSampleMaskBuiltins(ctx);
  ctx.Clone();
  return Output{Program(std::move(out))};
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
        auto* var_arr =
            ctx.dst->Var(var->source(), var_name, var->declared_storage_class(),
                         type, nullptr, ctx.Clone(var->decorations()));
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
