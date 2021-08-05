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

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/ast/disable_validation_decoration.h"
#include "src/program_builder.h"
#include "src/sem/call.h"
#include "src/sem/function.h"
#include "src/sem/statement.h"
#include "src/sem/variable.h"
#include "src/transform/array_length_from_uniform.h"
#include "src/transform/canonicalize_entry_point_io.h"
#include "src/transform/external_texture_transform.h"
#include "src/transform/inline_pointer_lets.h"
#include "src/transform/manager.h"
#include "src/transform/pad_array_elements.h"
#include "src/transform/promote_initializers_to_const_var.h"
#include "src/transform/simplify.h"
#include "src/transform/wrap_arrays_in_structs.h"
#include "src/transform/zero_init_workgroup_memory.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::Msl);
TINT_INSTANTIATE_TYPEINFO(tint::transform::Msl::Config);
TINT_INSTANTIATE_TYPEINFO(tint::transform::Msl::Result);

namespace tint {
namespace transform {

Msl::Msl() = default;
Msl::~Msl() = default;

Output Msl::Run(const Program* in, const DataMap& inputs) {
  Manager manager;
  DataMap internal_inputs;

  auto* cfg = inputs.Get<Config>();

  // Build the configs for the internal transforms.
  uint32_t buffer_size_ubo_index = kDefaultBufferSizeUniformIndex;
  uint32_t fixed_sample_mask = 0xFFFFFFFF;
  bool emit_point_size = false;
  if (cfg) {
    buffer_size_ubo_index = cfg->buffer_size_ubo_index;
    fixed_sample_mask = cfg->fixed_sample_mask;
    emit_point_size = cfg->emit_vertex_point_size;
  }
  auto array_length_from_uniform_cfg = ArrayLengthFromUniform::Config(
      sem::BindingPoint{0, buffer_size_ubo_index});
  auto entry_point_io_cfg = CanonicalizeEntryPointIO::Config(
      CanonicalizeEntryPointIO::ShaderStyle::kMsl, fixed_sample_mask,
      emit_point_size);

  // Use the SSBO binding numbers as the indices for the buffer size lookups.
  for (auto* var : in->AST().GlobalVariables()) {
    auto* global = in->Sem().Get<sem::GlobalVariable>(var);
    if (global && global->StorageClass() == ast::StorageClass::kStorage) {
      array_length_from_uniform_cfg.bindpoint_to_size_index.emplace(
          global->BindingPoint(), global->BindingPoint().binding);
    }
  }

  if (!cfg || !cfg->disable_workgroup_init) {
    // ZeroInitWorkgroupMemory must come before CanonicalizeEntryPointIO as
    // ZeroInitWorkgroupMemory may inject new builtin parameters.
    manager.Add<ZeroInitWorkgroupMemory>();
  }
  manager.Add<CanonicalizeEntryPointIO>();
  manager.Add<ExternalTextureTransform>();
  manager.Add<PromoteInitializersToConstVar>();
  manager.Add<WrapArraysInStructs>();
  manager.Add<PadArrayElements>();
  manager.Add<InlinePointerLets>();
  manager.Add<Simplify>();
  // ArrayLengthFromUniform must come after InlinePointerLets and Simplify, as
  // it assumes that the form of the array length argument is &var.array.
  manager.Add<ArrayLengthFromUniform>();
  internal_inputs.Add<ArrayLengthFromUniform::Config>(
      std::move(array_length_from_uniform_cfg));
  internal_inputs.Add<CanonicalizeEntryPointIO::Config>(
      std::move(entry_point_io_cfg));
  auto out = manager.Run(in, internal_inputs);
  if (!out.program.IsValid()) {
    return out;
  }

  ProgramBuilder builder;
  CloneContext ctx(&builder, &out.program);
  // TODO(jrprice): Consider making this a standalone transform, with target
  // storage class(es) as transform options.
  HandleModuleScopeVariables(ctx);
  ctx.Clone();

  auto result = std::make_unique<Result>(
      out.data.Get<ArrayLengthFromUniform::Result>()->needs_buffer_sizes);

  builder.SetTransformApplied(this);
  return Output{Program(std::move(builder)), std::move(result)};
}

void Msl::HandleModuleScopeVariables(CloneContext& ctx) const {
  // MSL does not allow private and workgroup variables at module-scope, so we
  // push these declarations into the entry point function and then pass them as
  // pointer parameters to any function that references them.
  // Similarly, texture and sampler types are converted to entry point
  // parameters and passed by value to functions that need them.
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
  // [[stage(compute), workgroup_size(1)]]
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
  // [[stage(compute), workgroup_size(1)]]
  // fn main() {
  //   var<private> v : f32 = 2.0;
  //   foo(&v);
  // }
  // ```

  // Predetermine the list of function calls that need to be replaced.
  using CallList = std::vector<const ast::CallExpression*>;
  std::unordered_map<const ast::Function*, CallList> calls_to_replace;

  std::vector<ast::Function*> functions_to_process;

  // Build a list of functions that transitively reference any private or
  // workgroup variables, or texture/sampler variables.
  for (auto* func_ast : ctx.src->AST().Functions()) {
    auto* func_sem = ctx.src->Sem().Get(func_ast);

    bool needs_processing = false;
    for (auto* var : func_sem->ReferencedModuleVariables()) {
      if (var->StorageClass() == ast::StorageClass::kPrivate ||
          var->StorageClass() == ast::StorageClass::kWorkgroup ||
          var->StorageClass() == ast::StorageClass::kUniformConstant) {
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

  // Build a list of `&ident` expressions. We'll use this later to avoid
  // generating expressions of the form `&*ident`, which break WGSL validation
  // rules when this expression is passed to a function.
  // TODO(jrprice): We should add support for bidirectional SEM tree traversal
  // so that we can do this on the fly instead.
  std::unordered_map<ast::IdentifierExpression*, ast::UnaryOpExpression*>
      ident_to_address_of;
  for (auto* node : ctx.src->ASTNodes().Objects()) {
    auto* address_of = node->As<ast::UnaryOpExpression>();
    if (!address_of || address_of->op() != ast::UnaryOp::kAddressOf) {
      continue;
    }
    if (auto* ident = address_of->expr()->As<ast::IdentifierExpression>()) {
      ident_to_address_of[ident] = address_of;
    }
  }

  for (auto* func_ast : functions_to_process) {
    auto* func_sem = ctx.src->Sem().Get(func_ast);
    bool is_entry_point = func_ast->IsEntryPoint();

    // Map module-scope variables onto their function-scope replacement.
    std::unordered_map<const sem::Variable*, Symbol> var_to_symbol;

    for (auto* var : func_sem->ReferencedModuleVariables()) {
      if (var->StorageClass() != ast::StorageClass::kPrivate &&
          var->StorageClass() != ast::StorageClass::kWorkgroup &&
          var->StorageClass() != ast::StorageClass::kUniformConstant) {
        continue;
      }

      // This is the symbol for the variable that replaces the module-scope var.
      auto new_var_symbol = ctx.dst->Sym();

      auto* store_type = CreateASTTypeFor(ctx, var->Type()->UnwrapRef());

      if (is_entry_point) {
        if (store_type->is_handle()) {
          // For a texture or sampler variable, redeclare it as an entry point
          // parameter. Disable entry point parameter validation.
          auto* disable_validation =
              ctx.dst->ASTNodes().Create<ast::DisableValidationDecoration>(
                  ctx.dst->ID(), ast::DisabledValidation::kEntryPointParameter);
          auto decos = ctx.Clone(var->Declaration()->decorations());
          decos.push_back(disable_validation);
          auto* param = ctx.dst->Param(new_var_symbol, store_type, decos);
          ctx.InsertFront(func_ast->params(), param);
        } else {
          // For a private or workgroup variable, redeclare it at function
          // scope. Disable storage class validation on this variable.
          auto* disable_validation =
              ctx.dst->ASTNodes().Create<ast::DisableValidationDecoration>(
                  ctx.dst->ID(), ast::DisabledValidation::kIgnoreStorageClass);
          auto* constructor = ctx.Clone(var->Declaration()->constructor());
          auto* local_var = ctx.dst->Var(
              new_var_symbol, store_type, var->StorageClass(), constructor,
              ast::DecorationList{disable_validation});
          ctx.InsertFront(func_ast->body()->statements(),
                          ctx.dst->Decl(local_var));
        }
      } else {
        // For a regular function, redeclare the variable as a parameter.
        // Use a pointer for non-handle types.
        auto* param_type = store_type;
        if (!store_type->is_handle()) {
          param_type = ctx.dst->ty.pointer(param_type, var->StorageClass());
        }
        ctx.InsertBack(func_ast->params(),
                       ctx.dst->Param(new_var_symbol, param_type));
      }

      // Replace all uses of the module-scope variable.
      // For non-entry points, dereference non-handle pointer parameters.
      for (auto* user : var->Users()) {
        if (user->Stmt()->Function() == func_ast) {
          ast::Expression* expr = ctx.dst->Expr(new_var_symbol);
          if (!is_entry_point && !store_type->is_handle()) {
            // If this identifier is used by an address-of operator, just remove
            // the address-of instead of adding a deref, since we already have a
            // pointer.
            auto* ident = user->Declaration()->As<ast::IdentifierExpression>();
            if (ident_to_address_of.count(ident)) {
              ctx.Replace(ident_to_address_of[ident], expr);
              continue;
            }

            expr = ctx.dst->Deref(expr);
          }
          ctx.Replace(user->Declaration(), expr);
        }
      }

      var_to_symbol[var] = new_var_symbol;
    }

    // Pass the variables as pointers to any functions that need them.
    for (auto* call : calls_to_replace[func_ast]) {
      auto* target = ctx.src->AST().Functions().Find(call->func()->symbol());
      auto* target_sem = ctx.src->Sem().Get(target);

      // Add new arguments for any variables that are needed by the callee.
      // For entry points, pass non-handle types as pointers.
      for (auto* target_var : target_sem->ReferencedModuleVariables()) {
        if (target_var->StorageClass() == ast::StorageClass::kPrivate ||
            target_var->StorageClass() == ast::StorageClass::kWorkgroup ||
            target_var->StorageClass() == ast::StorageClass::kUniformConstant) {
          ast::Expression* arg = ctx.dst->Expr(var_to_symbol[target_var]);
          if (is_entry_point && !target_var->Type()->UnwrapRef()->is_handle()) {
            arg = ctx.dst->AddressOf(arg);
          }
          ctx.InsertBack(call->params(), arg);
        }
      }
    }
  }

  // Now remove all module-scope variables with these storage classes.
  for (auto* var_ast : ctx.src->AST().GlobalVariables()) {
    auto* var_sem = ctx.src->Sem().Get(var_ast);
    if (var_sem->StorageClass() == ast::StorageClass::kPrivate ||
        var_sem->StorageClass() == ast::StorageClass::kWorkgroup ||
        var_sem->StorageClass() == ast::StorageClass::kUniformConstant) {
      ctx.Remove(ctx.src->AST().GlobalDeclarations(), var_ast);
    }
  }
}

Msl::Config::Config(uint32_t buffer_size_ubo_idx,
                    uint32_t sample_mask,
                    bool emit_point_size,
                    bool disable_wi)
    : buffer_size_ubo_index(buffer_size_ubo_idx),
      fixed_sample_mask(sample_mask),
      emit_vertex_point_size(emit_point_size),
      disable_workgroup_init(disable_wi) {}
Msl::Config::Config(const Config&) = default;
Msl::Config::~Config() = default;

Msl::Result::Result(bool needs_buffer_sizes)
    : needs_storage_buffer_sizes(needs_buffer_sizes) {}
Msl::Result::Result(const Result&) = default;
Msl::Result::~Result() = default;

}  // namespace transform
}  // namespace tint
