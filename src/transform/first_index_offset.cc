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

#include <cassert>
#include <utility>

#include "src/ast/array_accessor_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/break_statement.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/call_statement.h"
#include "src/ast/case_statement.h"
#include "src/ast/clone_context.h"
#include "src/ast/constructor_expression.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/else_statement.h"
#include "src/ast/expression.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/loop_statement.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/struct.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/struct_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/switch_statement.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/ast/variable_decoration.h"
#include "src/type_determiner.h"

namespace tint {
namespace transform {
namespace {

constexpr char kStructName[] = "TintFirstIndexOffsetData";
constexpr char kBufferName[] = "tint_first_index_data";
constexpr char kFirstVertexName[] = "tint_first_vertex_index";
constexpr char kFirstInstanceName[] = "tint_first_instance_index";
constexpr char kIndexOffsetPrefix[] = "tint_first_index_offset_";

ast::DecoratedVariable* clone_variable_with_new_name(ast::CloneContext* ctx,
                                                     ast::DecoratedVariable* in,
                                                     std::string new_name) {
  auto* var = ctx->mod->create<ast::Variable>(ctx->Clone(in->source()),
                                              new_name, in->storage_class(),
                                              ctx->Clone(in->type()));
  var->set_is_const(in->is_const());
  var->set_constructor(ctx->Clone(in->constructor()));

  auto* out = ctx->mod->create<ast::DecoratedVariable>(var);
  out->set_decorations(ctx->Clone(in->decorations()));
  return out;
}

}  // namespace

FirstIndexOffset::FirstIndexOffset(uint32_t binding, uint32_t set)
    : binding_(binding), set_(set) {}

FirstIndexOffset::~FirstIndexOffset() = default;

Transform::Output FirstIndexOffset::Run(ast::Module* in) {
  // First do a quick check to see if the transform has already been applied.
  for (ast::Variable* var : in->global_variables()) {
    if (auto* dec_var = var->As<ast::DecoratedVariable>()) {
      if (dec_var->name() == kBufferName) {
        diag::Diagnostic err;
        err.message = "First index offset transform has already been applied.";
        err.severity = diag::Severity::Error;
        Output out;
        out.diagnostics.add(std::move(err));
        return out;
      }
    }
  }

  // Running TypeDeterminer as we require local_referenced_builtin_variables()
  // to be populated. TODO(bclayton) - it should not be necessary to re-run the
  // type determiner if semantic information is already generated. Remove.
  TypeDeterminer td(in);
  if (!td.Determine()) {
    diag::Diagnostic err;
    err.severity = diag::Severity::Error;
    err.message = td.error();
    Output out;
    out.diagnostics.add(std::move(err));
    return out;
  }

  std::string vertex_index_name;
  std::string instance_index_name;

  Output out;

  // Lazilly construct the UniformBuffer on first call to
  // maybe_create_buffer_var()
  ast::Variable* buffer_var = nullptr;
  auto maybe_create_buffer_var = [&] {
    if (buffer_var == nullptr) {
      buffer_var = AddUniformBuffer(&out.module);
    }
  };

  // Clone the AST, renaming the kVertexIdx and kInstanceIdx builtins, and add
  // a CreateFirstIndexOffset() statement to each function that uses one of
  // these builtins.
  ast::CloneContext ctx(&out.module);
  ctx.ReplaceAll([&](ast::DecoratedVariable* var) -> ast::DecoratedVariable* {
    for (ast::VariableDecoration* dec : var->decorations()) {
      if (auto* blt_dec = dec->As<ast::BuiltinDecoration>()) {
        ast::Builtin blt_type = blt_dec->value();
        if (blt_type == ast::Builtin::kVertexIdx) {
          vertex_index_name = var->name();
          has_vertex_index_ = true;
          return clone_variable_with_new_name(&ctx, var,
                                              kIndexOffsetPrefix + var->name());
        } else if (blt_type == ast::Builtin::kInstanceIdx) {
          instance_index_name = var->name();
          has_instance_index_ = true;
          return clone_variable_with_new_name(&ctx, var,
                                              kIndexOffsetPrefix + var->name());
        }
      }
    }
    return nullptr;  // Just clone var
  });
  ctx.ReplaceAll(  // Note: This happens in the same pass as the rename above
                   // which determines the original builtin variable names,
                   // but this should be fine, as variables are cloned first.
      [&](ast::Function* func) -> ast::Function* {
        maybe_create_buffer_var();
        if (buffer_var == nullptr) {
          return nullptr;  // no transform need, just clone func
        }
        auto* body = ctx.mod->create<ast::BlockStatement>(
            ctx.Clone(func->body()->source()));
        for (const auto& data : func->local_referenced_builtin_variables()) {
          if (data.second->value() == ast::Builtin::kVertexIdx) {
            body->append(CreateFirstIndexOffset(
                vertex_index_name, kFirstVertexName, buffer_var, ctx.mod));
          } else if (data.second->value() == ast::Builtin::kInstanceIdx) {
            body->append(CreateFirstIndexOffset(
                instance_index_name, kFirstInstanceName, buffer_var, ctx.mod));
          }
        }
        for (auto* s : *func->body()) {
          body->append(ctx.Clone(s));
        }
        return ctx.mod->create<ast::Function>(
            ctx.Clone(func->source()), func->name(), ctx.Clone(func->params()),
            ctx.Clone(func->return_type()), ctx.Clone(body),
            ctx.Clone(func->decorations()));
      });

  in->Clone(&ctx);
  return out;
}

bool FirstIndexOffset::HasVertexIndex() {
  return has_vertex_index_;
}

bool FirstIndexOffset::HasInstanceIndex() {
  return has_instance_index_;
}

uint32_t FirstIndexOffset::GetFirstVertexOffset() {
  assert(has_vertex_index_);
  return vertex_index_offset_;
}

uint32_t FirstIndexOffset::GetFirstInstanceOffset() {
  assert(has_instance_index_);
  return instance_index_offset_;
}

ast::Variable* FirstIndexOffset::AddUniformBuffer(ast::Module* mod) {
  auto* u32_type = mod->create<ast::type::U32>();
  ast::StructMemberList members;
  uint32_t offset = 0;
  if (has_vertex_index_) {
    ast::StructMemberDecorationList member_dec;
    member_dec.push_back(
        mod->create<ast::StructMemberOffsetDecoration>(offset, Source{}));
    members.push_back(mod->create<ast::StructMember>(kFirstVertexName, u32_type,
                                                     std::move(member_dec)));
    vertex_index_offset_ = offset;
    offset += 4;
  }

  if (has_instance_index_) {
    ast::StructMemberDecorationList member_dec;
    member_dec.push_back(
        mod->create<ast::StructMemberOffsetDecoration>(offset, Source{}));
    members.push_back(mod->create<ast::StructMember>(
        kFirstInstanceName, u32_type, std::move(member_dec)));
    instance_index_offset_ = offset;
    offset += 4;
  }

  ast::StructDecorationList decos;
  decos.push_back(mod->create<ast::StructBlockDecoration>(Source{}));

  auto* struct_type = mod->create<ast::type::Struct>(
      kStructName,
      mod->create<ast::Struct>(std::move(decos), std::move(members)));

  auto* idx_var =
      mod->create<ast::DecoratedVariable>(mod->create<ast::Variable>(
          Source{}, kBufferName, ast::StorageClass::kUniform, struct_type));
  idx_var->set_decorations({
      mod->create<ast::BindingDecoration>(binding_, Source{}),
      mod->create<ast::SetDecoration>(set_, Source{}),
  });

  mod->AddGlobalVariable(idx_var);

  mod->AddConstructedType(struct_type);

  return idx_var;
}

ast::VariableDeclStatement* FirstIndexOffset::CreateFirstIndexOffset(
    const std::string& original_name,
    const std::string& field_name,
    ast::Variable* buffer_var,
    ast::Module* mod) {
  auto* buffer = mod->create<ast::IdentifierExpression>(buffer_var->name());
  auto* var = mod->create<ast::Variable>(Source{}, original_name,
                                         ast::StorageClass::kNone,
                                         mod->create<ast::type::U32>());

  var->set_is_const(true);
  var->set_constructor(mod->create<ast::BinaryExpression>(
      ast::BinaryOp::kAdd,
      mod->create<ast::IdentifierExpression>(kIndexOffsetPrefix + var->name()),
      mod->create<ast::MemberAccessorExpression>(
          buffer, mod->create<ast::IdentifierExpression>(field_name))));
  return mod->create<ast::VariableDeclStatement>(var);
}

}  // namespace transform
}  // namespace tint
