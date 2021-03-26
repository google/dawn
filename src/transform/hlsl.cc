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

#include "src/transform/hlsl.h"

#include <utility>
#include <vector>

#include "src/ast/variable_decl_statement.h"
#include "src/program_builder.h"
#include "src/semantic/expression.h"
#include "src/semantic/statement.h"
#include "src/semantic/variable.h"

namespace tint {
namespace transform {

Hlsl::Hlsl() = default;
Hlsl::~Hlsl() = default;

Transform::Output Hlsl::Run(const Program* in) {
  ProgramBuilder out;
  CloneContext ctx(&out, in);
  PromoteArrayInitializerToConstVar(ctx);
  HandleEntryPointIOTypes(ctx);
  ctx.Clone();
  return Output{Program(std::move(out))};
}

void Hlsl::PromoteArrayInitializerToConstVar(CloneContext& ctx) const {
  // Scan the AST nodes for array initializers which need to be promoted to
  // their own constant declaration.

  // Note: Correct handling of arrays-of-arrays is guaranteed due to the
  // depth-first traversal of the ast::Node::Clone() methods:
  //
  // The inner-most array initializers are traversed first, and they are hoisted
  // to const variables declared just above the statement of use. The outer
  // array initializer will then be hoisted, inserting themselves between the
  // inner array declaration and the statement of use. This pattern applies
  // correctly to any nested depth.
  //
  // Depth-first traversal of the AST is guaranteed because AST nodes are fully
  // immutable and require their children to be constructed first so their
  // pointer can be passed to the parent's constructor.

  for (auto* src_node : ctx.src->ASTNodes().Objects()) {
    if (auto* src_init = src_node->As<ast::TypeConstructorExpression>()) {
      auto* src_sem_expr = ctx.src->Sem().Get(src_init);
      if (!src_sem_expr) {
        TINT_ICE(ctx.dst->Diagnostics())
            << "ast::TypeConstructorExpression has no semantic expression node";
        continue;
      }
      auto* src_sem_stmt = src_sem_expr->Stmt();
      if (!src_sem_stmt) {
        // Expression is outside of a statement. This usually means the
        // expression is part of a global (module-scope) constant declaration.
        // These must be constexpr, and so cannot contain the type of
        // expressions that must be sanitized.
        continue;
      }
      auto* src_stmt = src_sem_stmt->Declaration();

      if (auto* src_var_decl = src_stmt->As<ast::VariableDeclStatement>()) {
        if (src_var_decl->variable()->constructor() == src_init) {
          // This statement is just a variable declaration with the array
          // initializer as the constructor value. This is what we're
          // attempting to transform to, and so ignore.
          continue;
        }
      }

      if (auto* src_array_ty = src_sem_expr->Type()->As<type::Array>()) {
        // Create a new symbol for the constant
        auto dst_symbol = ctx.dst->Symbols().New();
        // Clone the array type
        auto* dst_array_ty = ctx.Clone(src_array_ty);
        // Clone the array initializer
        auto* dst_init = ctx.Clone(src_init);
        // Construct the constant that holds the array
        auto* dst_var = ctx.dst->Const(dst_symbol, dst_array_ty, dst_init);
        // Construct the variable declaration statement
        auto* dst_var_decl =
            ctx.dst->create<ast::VariableDeclStatement>(dst_var);
        // Construct the identifier for referencing the constant
        auto* dst_ident = ctx.dst->Expr(dst_symbol);

        // Insert the constant before the usage
        ctx.InsertBefore(src_stmt, dst_var_decl);
        // Replace the inlined array with a reference to the constant
        ctx.Replace(src_init, dst_ident);
      }
    }
  }
}

void Hlsl::HandleEntryPointIOTypes(CloneContext& ctx) const {
  // Collect entry point parameters into a struct.
  // Insert function-scope const declarations to replace those parameters.
  //
  // Before:
  // ```
  // [[stage(fragment)]]
  // fn frag_main([[builtin(frag_coord)]] coord : vec4<f32>,
  //              [[location(1)]] loc1 : f32,
  //              [[location(2)]] loc2 : vec4<u32>) -> void {
  //   var col : f32 = (coord.x * loc1);
  // }
  // ```
  //
  // After:
  // ```
  // struct frag_main_in {
  //   [[builtin(frag_coord)]] coord : vec4<f32>;
  //   [[location(1)]] loc1 : f32;
  //   [[location(2)]] loc2 : vec4<u32>
  // };

  // [[stage(fragment)]]
  // fn frag_main(in : frag_main_in) -> void {
  //   const coord : vec4<f32> = in.coord;
  //   const loc1 : f32 = in.loc1;
  //   const loc2 : vec4<u32> = in.loc2;
  //   var col : f32 = (coord.x * loc1);
  // }
  // ```

  for (auto* func : ctx.src->AST().Functions()) {
    if (!func->IsEntryPoint()) {
      continue;
    }

    // Build a new structure to hold the non-struct input parameters.
    ast::StructMemberList struct_members;
    for (auto* param : func->params()) {
      auto* type = ctx.src->Sem().Get(param)->Type();
      if (type->Is<type::Struct>()) {
        // Already a struct, nothing to do.
        continue;
      }

      if (param->decorations().size() != 1) {
        TINT_ICE(ctx.dst->Diagnostics()) << "Unsupported entry point parameter";
      }

      auto name = ctx.src->Symbols().NameFor(param->symbol());

      auto* deco = param->decorations()[0];
      if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
        // Create a struct member with the builtin decoration.
        struct_members.push_back(ctx.dst->Member(
            name, ctx.Clone(type), ast::DecorationList{ctx.Clone(builtin)}));
      } else if (auto* loc = deco->As<ast::LocationDecoration>()) {
        // Create a struct member with the location decoration.
        struct_members.push_back(ctx.dst->Member(
            name, ctx.Clone(type), ast::DecorationList{ctx.Clone(loc)}));
      } else {
        TINT_ICE(ctx.dst->Diagnostics())
            << "Unsupported entry point parameter decoration";
      }
    }

    if (struct_members.empty()) {
      // Nothing to do.
      continue;
    }

    ast::VariableList new_parameters;
    ast::StatementList new_body;

    // Create a struct type to hold all of the non-struct input parameters.
    auto* in_struct = ctx.dst->create<type::Struct>(
        ctx.dst->Symbols().New(),
        ctx.dst->create<ast::Struct>(struct_members, ast::DecorationList{}));
    ctx.InsertBefore(func, in_struct);

    // Create a new function parameter using this struct type.
    auto struct_param_symbol = ctx.dst->Symbols().New();
    auto* struct_param =
        ctx.dst->Var(struct_param_symbol, in_struct, ast::StorageClass::kNone);
    new_parameters.push_back(struct_param);

    // Replace the original parameters with function-scope constants.
    for (auto* param : func->params()) {
      auto* type = ctx.src->Sem().Get(param)->Type();
      if (type->Is<type::Struct>()) {
        // Keep struct parameters unchanged.
        new_parameters.push_back(ctx.Clone(param));
        continue;
      }

      auto name = ctx.src->Symbols().NameFor(param->symbol());

      // Create a function-scope const to replace the parameter.
      // Initialize it with the value extracted from the struct parameter.
      auto func_const_symbol = ctx.dst->Symbols().Register(name);
      auto* func_const =
          ctx.dst->Const(func_const_symbol, ctx.Clone(type),
                         ctx.dst->MemberAccessor(struct_param_symbol, name));

      new_body.push_back(ctx.dst->WrapInStatement(func_const));

      // Replace all uses of the function parameter with the function const.
      for (auto* user : ctx.src->Sem().Get(param)->Users()) {
        ctx.Replace<ast::Expression>(user->Declaration(),
                                     ctx.dst->Expr(func_const_symbol));
      }
    }

    // Copy over the rest of the function body unchanged.
    for (auto* stmt : func->body()->list()) {
      new_body.push_back(ctx.Clone(stmt));
    }

    // Rewrite the function header with the new parameters.
    auto* new_func = ctx.dst->create<ast::Function>(
        func->source(), ctx.Clone(func->symbol()), new_parameters,
        ctx.Clone(func->return_type()),
        ctx.dst->create<ast::BlockStatement>(new_body),
        ctx.Clone(func->decorations()),
        ctx.Clone(func->return_type_decorations()));
    ctx.Replace(func, new_func);
  }
}

}  // namespace transform
}  // namespace tint
