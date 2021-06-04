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

#include "src/transform/msl.h"

#include <unordered_map>
#include <utility>
#include <vector>

#include "src/ast/disable_validation_decoration.h"
#include "src/program_builder.h"
#include "src/sem/call.h"
#include "src/sem/function.h"
#include "src/sem/statement.h"
#include "src/sem/variable.h"
#include "src/transform/canonicalize_entry_point_io.h"
#include "src/transform/external_texture_transform.h"
#include "src/transform/manager.h"
#include "src/transform/promote_initializers_to_const_var.h"

namespace tint {
namespace transform {

Msl::Msl() = default;
Msl::~Msl() = default;

Output Msl::Run(const Program* in, const DataMap&) {
  Manager manager;
  DataMap data;
  manager.Add<CanonicalizeEntryPointIO>();
  manager.Add<ExternalTextureTransform>();
  manager.Add<PromoteInitializersToConstVar>();
  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::BuiltinStyle::kParameter);
  auto out = manager.Run(in, data);
  if (!out.program.IsValid()) {
    return out;
  }

  ProgramBuilder builder;
  CloneContext ctx(&builder, &out.program);
  // TODO(jrprice): Consider making this a standalone transform, with target
  // storage class(es) as transform options.
  HandlePrivateAndWorkgroupVariables(ctx);
  ctx.Clone();
  return Output{Program(std::move(builder))};
}

void Msl::HandlePrivateAndWorkgroupVariables(CloneContext& ctx) const {
  // MSL does not allow private and workgroup variables at module-scope, so we
  // push these declarations into the entry point function and then pass them as
  // pointer parameters to any function that references them.
  //
  // Since WGSL does not allow function-scope variables to have these storage
  // classes, we annotate the new variable declarations with an attribute that
  // bypasses that validation rule.
  //
  // Before:
  // ```
  // var<private> v : f32 = 2.0;
  //
  // fn foo() {
  //   v = v + 1.0;
  // }
  //
  // [[stage(compute)]]
  // fn main() {
  //   foo();
  // }
  // ```
  //
  // After:
  // ```
  // fn foo(v : ptr<private, f32>) {
  //   *v = *v + 1.0;
  // }
  //
  // [[stage(compute)]]
  // fn main() {
  //   var<private> v : f32 = 2.0;
  //   let v_ptr : ptr<private, f32> = &f32;
  //   foo(v_ptr);
  // }
  // ```

  // Predetermine the list of function calls that need to be replaced.
  using CallList = std::vector<const ast::CallExpression*>;
  std::unordered_map<const ast::Function*, CallList> calls_to_replace;

  std::vector<ast::Function*> functions_to_process;

  // Build a list of functions that transitively reference any private or
  // workgroup variables.
  for (auto* func_ast : ctx.src->AST().Functions()) {
    auto* func_sem = ctx.src->Sem().Get(func_ast);

    bool needs_processing = false;
    for (auto* var : func_sem->ReferencedModuleVariables()) {
      if (var->StorageClass() == ast::StorageClass::kPrivate ||
          var->StorageClass() == ast::StorageClass::kWorkgroup) {
        needs_processing = true;
        break;
      }
    }

    if (needs_processing) {
      functions_to_process.push_back(func_ast);

      // Find all of the calls to this function that will need to be replaced.
      for (auto* call : func_sem->CallSites()) {
        auto* call_sem = ctx.src->Sem().Get(call);
        calls_to_replace[call_sem->Stmt()->Function()].push_back(call);
      }
    }
  }

  for (auto* func_ast : functions_to_process) {
    auto* func_sem = ctx.src->Sem().Get(func_ast);

    // Map module-scope variables onto their function-scope replacement.
    std::unordered_map<const sem::Variable*, Symbol> var_to_symbol;

    for (auto* var : func_sem->ReferencedModuleVariables()) {
      if (var->StorageClass() != ast::StorageClass::kPrivate &&
          var->StorageClass() != ast::StorageClass::kWorkgroup) {
        continue;
      }

      // This is the symbol for the pointer that replaces the module-scope var.
      auto new_var_symbol = ctx.dst->Sym();

      auto* store_type = CreateASTTypeFor(&ctx, var->Type()->UnwrapRef());

      if (func_ast->IsEntryPoint()) {
        // For an entry point, redeclare the variable at function-scope.
        // Disable storage class validation on this variable.
        auto* disable_validation =
            ctx.dst->ASTNodes().Create<ast::DisableValidationDecoration>(
                ctx.dst->ID(),
                ast::DisabledValidation::kFunctionVarStorageClass);
        auto* constructor = ctx.Clone(var->Declaration()->constructor());
        auto* local_var =
            ctx.dst->Var(ctx.dst->Sym(), store_type, var->StorageClass(),
                         constructor, {disable_validation});
        ctx.InsertBefore(func_ast->body()->statements(),
                         *func_ast->body()->begin(), ctx.dst->Decl(local_var));

        // Now take the address of the variable.
        auto* ptr = ctx.dst->Const(new_var_symbol, nullptr,
                                   ctx.dst->AddressOf(local_var));
        ctx.InsertBefore(func_ast->body()->statements(),
                         *func_ast->body()->begin(), ctx.dst->Decl(ptr));
      } else {
        // For a regular function, redeclare the variable as a pointer function
        // parameter.
        auto* ptr_type = ctx.dst->ty.pointer(store_type, var->StorageClass());
        ctx.InsertBack(func_ast->params(),
                       ctx.dst->Param(new_var_symbol, ptr_type));
      }

      // Replace all uses of the module-scope variable with the pointer
      // replacement (dereferenced).
      for (auto* user : var->Users()) {
        if (user->Stmt()->Function() == func_ast) {
          ctx.Replace(user->Declaration(), ctx.dst->Deref(new_var_symbol));
        }
      }

      var_to_symbol[var] = new_var_symbol;
    }

    // Pass the pointers through to any functions that need them.
    for (auto* call : calls_to_replace[func_ast]) {
      auto* target = ctx.src->AST().Functions().Find(call->func()->symbol());
      auto* target_sem = ctx.src->Sem().Get(target);

      // Add new arguments for any referenced private and workgroup variables.
      for (auto* target_var : target_sem->ReferencedModuleVariables()) {
        if (target_var->StorageClass() == ast::StorageClass::kPrivate ||
            target_var->StorageClass() == ast::StorageClass::kWorkgroup) {
          ctx.InsertBack(call->params(),
                         ctx.dst->Expr(var_to_symbol[target_var]));
        }
      }
    }
  }

  // Now remove all module-scope private and workgroup variables.
  for (auto* var : ctx.src->AST().GlobalVariables()) {
    if (var->declared_storage_class() == ast::StorageClass::kPrivate ||
        var->declared_storage_class() == ast::StorageClass::kWorkgroup) {
      ctx.Remove(ctx.src->AST().GlobalDeclarations(), var);
    }
  }
}

}  // namespace transform
}  // namespace tint
