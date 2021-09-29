// Copyright 2021 The Tint Authors.
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

#include "src/transform/module_scope_var_to_entry_point_param.h"

#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/ast/disable_validation_decoration.h"
#include "src/program_builder.h"
#include "src/sem/call.h"
#include "src/sem/function.h"
#include "src/sem/statement.h"
#include "src/sem/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::ModuleScopeVarToEntryPointParam);

namespace tint {
namespace transform {
namespace {
// Returns `true` if `type` is or contains a matrix type.
bool ContainsMatrix(const sem::Type* type) {
  type = type->UnwrapRef();
  if (type->Is<sem::Matrix>()) {
    return true;
  } else if (auto* ary = type->As<sem::Array>()) {
    return ContainsMatrix(ary->ElemType());
  } else if (auto* str = type->As<sem::Struct>()) {
    for (auto* member : str->Members()) {
      if (ContainsMatrix(member->Type())) {
        return true;
      }
    }
  }
  return false;
}
}  // namespace

/// State holds the current transform state.
struct ModuleScopeVarToEntryPointParam::State {
  /// The clone context.
  CloneContext& ctx;

  /// Constructor
  /// @param context the clone context
  explicit State(CloneContext& context) : ctx(context) {}

  /// Clone any struct types that are contained in `ty` (including `ty` itself),
  /// and add it to the global declarations now, so that they precede new global
  /// declarations that need to reference them.
  /// @param ty the type to clone
  void CloneStructTypes(const sem::Type* ty) {
    if (auto* str = ty->As<sem::Struct>()) {
      if (!cloned_structs_.emplace(str).second) {
        // The struct has already been cloned.
        return;
      }

      // Recurse into members.
      for (auto* member : str->Members()) {
        CloneStructTypes(member->Type());
      }

      // Clone the struct and add it to the global declaration list.
      // Remove the old declaration.
      auto* ast_str = str->Declaration();
      ctx.dst->AST().AddTypeDecl(ctx.Clone(const_cast<ast::Struct*>(ast_str)));
      ctx.Remove(ctx.src->AST().GlobalDeclarations(), ast_str);
    } else if (auto* arr = ty->As<sem::Array>()) {
      CloneStructTypes(arr->ElemType());
    }
  }

  /// Process the module.
  void Process() {
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

      // We aggregate all workgroup variables into a struct to avoid hitting
      // MSL's limit for threadgroup memory arguments.
      Symbol workgroup_parameter_symbol;
      ast::StructMemberList workgroup_parameter_members;
      auto workgroup_param = [&]() {
        if (!workgroup_parameter_symbol.IsValid()) {
          workgroup_parameter_symbol = ctx.dst->Sym();
        }
        return workgroup_parameter_symbol;
      };

      for (auto* var : func_sem->ReferencedModuleVariables()) {
        if (var->StorageClass() != ast::StorageClass::kPrivate &&
            var->StorageClass() != ast::StorageClass::kWorkgroup &&
            var->StorageClass() != ast::StorageClass::kUniformConstant) {
          continue;
        }

        // This is the symbol for the variable that replaces the module-scope
        // var.
        auto new_var_symbol = ctx.dst->Sym();

        // Helper to create an AST node for the store type of the variable.
        auto store_type = [&]() {
          return CreateASTTypeFor(ctx, var->Type()->UnwrapRef());
        };

        // Track whether the new variable is a pointer or not.
        bool is_pointer = false;

        if (is_entry_point) {
          if (var->Type()->UnwrapRef()->is_handle()) {
            // For a texture or sampler variable, redeclare it as an entry point
            // parameter. Disable entry point parameter validation.
            auto* disable_validation =
                ctx.dst->ASTNodes().Create<ast::DisableValidationDecoration>(
                    ctx.dst->ID(),
                    ast::DisabledValidation::kEntryPointParameter);
            auto decos = ctx.Clone(var->Declaration()->decorations());
            decos.push_back(disable_validation);
            auto* param = ctx.dst->Param(new_var_symbol, store_type(), decos);
            ctx.InsertFront(func_ast->params(), param);
          } else if (var->StorageClass() == ast::StorageClass::kWorkgroup &&
                     ContainsMatrix(var->Type())) {
            // Due to a bug in the MSL compiler, we use a threadgroup memory
            // argument for any workgroup allocation that contains a matrix.
            // See crbug.com/tint/938.
            // TODO(jrprice): Do this for all other workgroup variables too.

            // Create a member in the workgroup parameter struct.
            auto member = ctx.Clone(var->Declaration()->symbol());
            workgroup_parameter_members.push_back(
                ctx.dst->Member(member, store_type()));
            CloneStructTypes(var->Type()->UnwrapRef());

            // Create a function-scope variable that is a pointer to the member.
            auto* member_ptr = ctx.dst->AddressOf(ctx.dst->MemberAccessor(
                ctx.dst->Deref(workgroup_param()), member));
            auto* local_var =
                ctx.dst->Const(new_var_symbol,
                               ctx.dst->ty.pointer(
                                   store_type(), ast::StorageClass::kWorkgroup),
                               member_ptr);
            ctx.InsertFront(func_ast->body()->statements(),
                            ctx.dst->Decl(local_var));
            is_pointer = true;
          } else {
            // For any other private or workgroup variable, redeclare it at
            // function scope. Disable storage class validation on this
            // variable.
            auto* disable_validation =
                ctx.dst->ASTNodes().Create<ast::DisableValidationDecoration>(
                    ctx.dst->ID(),
                    ast::DisabledValidation::kIgnoreStorageClass);
            auto* constructor = ctx.Clone(var->Declaration()->constructor());
            auto* local_var = ctx.dst->Var(
                new_var_symbol, store_type(), var->StorageClass(), constructor,
                ast::DecorationList{disable_validation});
            ctx.InsertFront(func_ast->body()->statements(),
                            ctx.dst->Decl(local_var));
          }
        } else {
          // For a regular function, redeclare the variable as a parameter.
          // Use a pointer for non-handle types.
          auto* param_type = store_type();
          ast::DecorationList attributes;
          if (!param_type->is_handle()) {
            param_type = ctx.dst->ty.pointer(param_type, var->StorageClass());
            is_pointer = true;

            // Disable validation of arguments passed to this pointer parameter,
            // as we will sometimes pass pointers to struct members.
            attributes.push_back(
                ctx.dst->ASTNodes().Create<ast::DisableValidationDecoration>(
                    ctx.dst->ID(),
                    ast::DisabledValidation::kIgnoreInvalidPointerArgument));
          }
          ctx.InsertBack(
              func_ast->params(),
              ctx.dst->Param(new_var_symbol, param_type, attributes));
        }

        // Replace all uses of the module-scope variable.
        // For non-entry points, dereference non-handle pointer parameters.
        for (auto* user : var->Users()) {
          if (user->Stmt()->Function() == func_ast) {
            ast::Expression* expr = ctx.dst->Expr(new_var_symbol);
            if (is_pointer) {
              // If this identifier is used by an address-of operator, just
              // remove the address-of instead of adding a deref, since we
              // already have a pointer.
              auto* ident =
                  user->Declaration()->As<ast::IdentifierExpression>();
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

      if (!workgroup_parameter_members.empty()) {
        // Create the workgroup memory parameter.
        // The parameter is a struct that contains members for each workgroup
        // variable.
        auto* str = ctx.dst->Structure(ctx.dst->Sym(),
                                       std::move(workgroup_parameter_members));
        auto* param_type = ctx.dst->ty.pointer(ctx.dst->ty.Of(str),
                                               ast::StorageClass::kWorkgroup);
        auto* disable_validation =
            ctx.dst->ASTNodes().Create<ast::DisableValidationDecoration>(
                ctx.dst->ID(), ast::DisabledValidation::kEntryPointParameter);
        auto* param =
            ctx.dst->Param(workgroup_param(), param_type, {disable_validation});
        ctx.InsertFront(func_ast->params(), param);
      }

      // Pass the variables as pointers to any functions that need them.
      for (auto* call : calls_to_replace[func_ast]) {
        auto* target = ctx.src->AST().Functions().Find(call->func()->symbol());
        auto* target_sem = ctx.src->Sem().Get(target);

        // Add new arguments for any variables that are needed by the callee.
        // For entry points, pass non-handle types as pointers.
        for (auto* target_var : target_sem->ReferencedModuleVariables()) {
          bool is_handle = target_var->Type()->UnwrapRef()->is_handle();
          bool is_workgroup_matrix =
              target_var->StorageClass() == ast::StorageClass::kWorkgroup &&
              ContainsMatrix(target_var->Type());
          if (target_var->StorageClass() == ast::StorageClass::kPrivate ||
              target_var->StorageClass() == ast::StorageClass::kWorkgroup ||
              target_var->StorageClass() ==
                  ast::StorageClass::kUniformConstant) {
            ast::Expression* arg = ctx.dst->Expr(var_to_symbol[target_var]);
            if (is_entry_point && !is_handle && !is_workgroup_matrix) {
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

 private:
  std::unordered_set<const sem::Struct*> cloned_structs_;
};

ModuleScopeVarToEntryPointParam::ModuleScopeVarToEntryPointParam() = default;

ModuleScopeVarToEntryPointParam::~ModuleScopeVarToEntryPointParam() = default;

void ModuleScopeVarToEntryPointParam::Run(CloneContext& ctx,
                                          const DataMap&,
                                          DataMap&) {
  State state{ctx};
  state.Process();
  ctx.Clone();
}

}  // namespace transform
}  // namespace tint
