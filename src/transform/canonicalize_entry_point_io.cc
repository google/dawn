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

#include "src/transform/canonicalize_entry_point_io.h"

#include <algorithm>
#include <string>
#include <utility>

#include "src/program_builder.h"
#include "src/sem/block_statement.h"
#include "src/sem/function.h"
#include "src/sem/statement.h"
#include "src/sem/struct.h"
#include "src/sem/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::CanonicalizeEntryPointIO);
TINT_INSTANTIATE_TYPEINFO(tint::transform::CanonicalizeEntryPointIO::Config);

namespace tint {
namespace transform {

CanonicalizeEntryPointIO::CanonicalizeEntryPointIO() = default;
CanonicalizeEntryPointIO::~CanonicalizeEntryPointIO() = default;

namespace {

// Comparison function used to reorder struct members such that all members with
// location attributes appear first (ordered by location slot), followed by
// those with builtin attributes.
bool StructMemberComparator(const ast::StructMember* a,
                            const ast::StructMember* b) {
  auto* a_loc = ast::GetDecoration<ast::LocationDecoration>(a->decorations());
  auto* b_loc = ast::GetDecoration<ast::LocationDecoration>(b->decorations());
  auto* a_blt = ast::GetDecoration<ast::BuiltinDecoration>(a->decorations());
  auto* b_blt = ast::GetDecoration<ast::BuiltinDecoration>(b->decorations());
  if (a_loc) {
    if (!b_loc) {
      // `a` has location attribute and `b` does not: `a` goes first.
      return true;
    }
    // Both have location attributes: smallest goes first.
    return a_loc->value() < b_loc->value();
  } else {
    if (b_loc) {
      // `b` has location attribute and `a` does not: `b` goes first.
      return false;
    }
    // Both are builtins: order doesn't matter, just use enum value.
    return a_blt->value() < b_blt->value();
  }
}

}  // namespace

void CanonicalizeEntryPointIO::Run(CloneContext& ctx,
                                   const DataMap& inputs,
                                   DataMap&) {
  auto* cfg = inputs.Get<Config>();
  if (cfg == nullptr) {
    ctx.dst->Diagnostics().add_error(
        diag::System::Transform,
        "missing transform data for " + std::string(TypeInfo().name));
    return;
  }

  // Strip entry point IO decorations from struct declarations.
  // TODO(jrprice): This code is duplicated with the SPIR-V transform.
  for (auto* ty : ctx.src->AST().TypeDecls()) {
    if (auto* struct_ty = ty->As<ast::Struct>()) {
      // Build new list of struct members without entry point IO decorations.
      ast::StructMemberList new_struct_members;
      for (auto* member : struct_ty->members()) {
        ast::DecorationList new_decorations = RemoveDecorations(
            &ctx, member->decorations(), [](const ast::Decoration* deco) {
              return deco->IsAnyOf<
                  ast::BuiltinDecoration, ast::InterpolateDecoration,
                  ast::InvariantDecoration, ast::LocationDecoration>();
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

  // Returns true if `decos` contains a `sample_mask` builtin.
  auto has_sample_mask_builtin = [](const ast::DecorationList& decos) {
    if (auto* builtin = ast::GetDecoration<ast::BuiltinDecoration>(decos)) {
      return builtin->value() == ast::Builtin::kSampleMask;
    }
    return false;
  };

  for (auto* func_ast : ctx.src->AST().Functions()) {
    if (!func_ast->IsEntryPoint()) {
      continue;
    }

    auto* func = ctx.src->Sem().Get(func_ast);

    bool needs_fixed_sample_mask =
        func_ast->pipeline_stage() == ast::PipelineStage::kFragment &&
        cfg->fixed_sample_mask != 0xFFFFFFFF;

    ast::VariableList new_parameters;

    if (!func->Parameters().empty()) {
      // Collect all parameters and build a list of new struct members.
      auto new_struct_param_symbol = ctx.dst->Sym();
      ast::StructMemberList new_struct_members;
      for (auto* param : func->Parameters()) {
        if (cfg->builtin_style == BuiltinStyle::kParameter &&
            ast::HasDecoration<ast::BuiltinDecoration>(
                param->Declaration()->decorations())) {
          // If this parameter is a builtin and we are emitting those as
          // parameters, then just clone it as is.
          new_parameters.push_back(
              ctx.Clone(const_cast<ast::Variable*>(param->Declaration())));
          continue;
        }

        auto param_name = ctx.Clone(param->Declaration()->symbol());
        auto* param_ty = param->Type();
        auto* param_declared_ty = param->Declaration()->type();

        ast::Expression* func_const_initializer = nullptr;

        if (auto* str = param_ty->As<sem::Struct>()) {
          // Pull out all struct members and build initializer list.
          ast::ExpressionList init_values;
          for (auto* member : str->Members()) {
            if (member->Type()->Is<sem::Struct>()) {
              TINT_ICE(Transform, ctx.dst->Diagnostics())
                  << "nested pipeline IO struct";
            }

            ast::DecorationList new_decorations = RemoveDecorations(
                &ctx, member->Declaration()->decorations(),
                [](const ast::Decoration* deco) {
                  return !deco->IsAnyOf<
                      ast::BuiltinDecoration, ast::InterpolateDecoration,
                      ast::InvariantDecoration, ast::LocationDecoration>();
                });

            if (cfg->builtin_style == BuiltinStyle::kParameter &&
                ast::HasDecoration<ast::BuiltinDecoration>(
                    member->Declaration()->decorations())) {
              // If this struct member is a builtin and we are emitting those as
              // parameters, then move it to the parameter list.
              auto* member_ty = CreateASTTypeFor(&ctx, member->Type());
              auto new_param_name = ctx.dst->Sym();
              new_parameters.push_back(
                  ctx.dst->Param(new_param_name, member_ty, new_decorations));
              init_values.push_back(ctx.dst->Expr(new_param_name));
              continue;
            }

            auto member_name = ctx.Clone(member->Declaration()->symbol());
            auto* member_type = ctx.Clone(member->Declaration()->type());
            new_struct_members.push_back(
                ctx.dst->Member(member_name, member_type, new_decorations));
            init_values.push_back(
                ctx.dst->MemberAccessor(new_struct_param_symbol, member_name));
          }

          func_const_initializer =
              ctx.dst->Construct(ctx.Clone(param_declared_ty), init_values);
        } else {
          new_struct_members.push_back(
              ctx.dst->Member(param_name, ctx.Clone(param_declared_ty),
                              ctx.Clone(param->Declaration()->decorations())));
          func_const_initializer =
              ctx.dst->MemberAccessor(new_struct_param_symbol, param_name);
        }

        // Create a function-scope const to replace the parameter.
        // Initialize it with the value extracted from the new struct parameter.
        auto* func_const = ctx.dst->Const(
            param_name, ctx.Clone(param_declared_ty), func_const_initializer);
        ctx.InsertFront(func_ast->body()->statements(),
                        ctx.dst->WrapInStatement(func_const));

        // Replace all uses of the function parameter with the function const.
        for (auto* user : param->Users()) {
          ctx.Replace<ast::Expression>(user->Declaration(),
                                       ctx.dst->Expr(param_name));
        }
      }

      if (!new_struct_members.empty()) {
        // Sort struct members to satisfy HLSL interfacing matching rules.
        std::sort(new_struct_members.begin(), new_struct_members.end(),
                  StructMemberComparator);

        // Create the new struct type.
        auto in_struct_name = ctx.dst->Sym();
        auto* in_struct = ctx.dst->create<ast::Struct>(
            in_struct_name, new_struct_members, ast::DecorationList{});
        ctx.InsertBefore(ctx.src->AST().GlobalDeclarations(), func_ast,
                         in_struct);

        // Create a new function parameter using this struct type.
        auto* struct_param = ctx.dst->Param(
            new_struct_param_symbol, ctx.dst->ty.type_name(in_struct_name));
        new_parameters.push_back(struct_param);
      }
    }

    // Handle return type.
    auto* ret_type = func->ReturnType();
    std::function<ast::Type*()> new_ret_type;
    if (ret_type->Is<sem::Void>() && !needs_fixed_sample_mask) {
      new_ret_type = [&ctx] { return ctx.dst->ty.void_(); };
    } else {
      ast::StructMemberList new_struct_members;

      bool has_authored_sample_mask = false;

      if (auto* str = ret_type->As<sem::Struct>()) {
        // Rebuild struct with only the entry point IO attributes.
        for (auto* member : str->Members()) {
          if (member->Type()->Is<sem::Struct>()) {
            TINT_ICE(Transform, ctx.dst->Diagnostics())
                << "nested pipeline IO struct";
          }

          ast::DecorationList new_decorations = RemoveDecorations(
              &ctx, member->Declaration()->decorations(),
              [](const ast::Decoration* deco) {
                return !deco->IsAnyOf<
                    ast::BuiltinDecoration, ast::InterpolateDecoration,
                    ast::InvariantDecoration, ast::LocationDecoration>();
              });
          auto symbol = ctx.Clone(member->Declaration()->symbol());
          auto* member_ty = ctx.Clone(member->Declaration()->type());
          new_struct_members.push_back(
              ctx.dst->Member(symbol, member_ty, new_decorations));

          if (has_sample_mask_builtin(new_decorations)) {
            has_authored_sample_mask = true;
          }
        }
      } else if (!ret_type->Is<sem::Void>()) {
        auto* member_ty = ctx.Clone(func->Declaration()->return_type());
        auto decos = ctx.Clone(func_ast->return_type_decorations());
        new_struct_members.push_back(
            ctx.dst->Member("value", member_ty, std::move(decos)));

        if (has_sample_mask_builtin(func_ast->return_type_decorations())) {
          has_authored_sample_mask = true;
        }
      }

      // If a sample mask builtin is required and the shader source did not
      // contain one, create one now.
      if (needs_fixed_sample_mask && !has_authored_sample_mask) {
        new_struct_members.push_back(
            ctx.dst->Member(ctx.dst->Sym(), ctx.dst->ty.u32(),
                            {ctx.dst->Builtin(ast::Builtin::kSampleMask)}));
      }

      // Sort struct members to satisfy HLSL interfacing matching rules.
      std::sort(new_struct_members.begin(), new_struct_members.end(),
                StructMemberComparator);

      // Create the new struct type.
      auto out_struct_name = ctx.dst->Sym();
      auto* out_struct = ctx.dst->create<ast::Struct>(
          out_struct_name, new_struct_members, ast::DecorationList{});
      ctx.InsertBefore(ctx.src->AST().GlobalDeclarations(), func_ast,
                       out_struct);
      new_ret_type = [out_struct_name, &ctx] {
        return ctx.dst->ty.type_name(out_struct_name);
      };

      // Replace all return statements.
      for (auto* ret : func->ReturnStatements()) {
        auto* ret_sem = ctx.src->Sem().Get(ret);
        // Reconstruct the return value using the newly created struct.
        std::function<ast::Expression*()> new_ret_value = [&ctx, ret] {
          return ctx.Clone(ret->value());
        };

        ast::ExpressionList ret_values;
        if (ret_type->Is<sem::Struct>()) {
          if (!ret->value()->Is<ast::IdentifierExpression>()) {
            // Create a const to hold the return value expression to avoid
            // re-evaluating it multiple times.
            auto temp = ctx.dst->Sym();
            auto* ty = CreateASTTypeFor(&ctx, ret_type);
            auto* temp_var =
                ctx.dst->Decl(ctx.dst->Const(temp, ty, new_ret_value()));
            ctx.InsertBefore(ret_sem->Block()->Declaration()->statements(), ret,
                             temp_var);
            new_ret_value = [&ctx, temp] { return ctx.dst->Expr(temp); };
          }

          for (auto* member : new_struct_members) {
            ast::Expression* expr = nullptr;

            if (needs_fixed_sample_mask &&
                has_sample_mask_builtin(member->decorations())) {
              // Use the fixed sample mask, combining it with the authored value
              // if there is one.
              expr = ctx.dst->Expr(cfg->fixed_sample_mask);
              if (has_authored_sample_mask) {
                expr = ctx.dst->And(
                    ctx.dst->MemberAccessor(new_ret_value(), member->symbol()),
                    expr);
              }
            } else {
              expr = ctx.dst->MemberAccessor(new_ret_value(), member->symbol());
            }
            ret_values.push_back(expr);
          }
        } else {
          if (!ret_type->Is<sem::Void>()) {
            ret_values.push_back(new_ret_value());
          }

          if (needs_fixed_sample_mask) {
            // If the original return value was a sample mask, `and` it with the
            // fixed mask and return the result.
            // Otherwise, append the fixed mask to the list of return values,
            // since it will be the last element of the output struct.
            if (has_authored_sample_mask) {
              ret_values[0] =
                  ctx.dst->And(ret_values[0], cfg->fixed_sample_mask);
            } else {
              ret_values.push_back(ctx.dst->Expr(cfg->fixed_sample_mask));
            }
          }
        }

        auto* new_ret =
            ctx.dst->Return(ctx.dst->Construct(new_ret_type(), ret_values));
        ctx.Replace(ret, new_ret);
      }

      if (needs_fixed_sample_mask && func->ReturnStatements().empty()) {
        // There we no return statements but we need to return a fixed sample
        // mask, so add a return statement that does this.
        ctx.InsertBack(func_ast->body()->statements(),
                       ctx.dst->Return(ctx.dst->Construct(
                           new_ret_type(), cfg->fixed_sample_mask)));
      }
    }

    // Rewrite the function header with the new parameters.
    auto* new_func = ctx.dst->create<ast::Function>(
        func_ast->source(), ctx.Clone(func_ast->symbol()), new_parameters,
        new_ret_type(), ctx.Clone(func_ast->body()),
        ctx.Clone(func_ast->decorations()), ast::DecorationList{});
    ctx.Replace(func_ast, new_func);
  }

  ctx.Clone();
}

CanonicalizeEntryPointIO::Config::Config(BuiltinStyle builtins,
                                         uint32_t sample_mask)
    : builtin_style(builtins), fixed_sample_mask(sample_mask) {}

CanonicalizeEntryPointIO::Config::Config(const Config&) = default;
CanonicalizeEntryPointIO::Config::~Config() = default;

}  // namespace transform
}  // namespace tint
