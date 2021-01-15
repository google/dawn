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

ast::Variable* clone_variable_with_new_name(ast::CloneContext* ctx,
                                            ast::Variable* in,
                                            std::string new_name) {
  return ctx->mod->create<ast::Variable>(
      ctx->Clone(in->source()),            // source
      ctx->mod->RegisterSymbol(new_name),  // symbol
      in->storage_class(),                 // storage_class
      ctx->Clone(in->type()),              // type
      in->is_const(),                      // is_const
      ctx->Clone(in->constructor()),       // constructor
      ctx->Clone(in->decorations()));      // decorations
}

}  // namespace

FirstIndexOffset::FirstIndexOffset(uint32_t binding, uint32_t group)
    : binding_(binding), group_(group) {}

FirstIndexOffset::~FirstIndexOffset() = default;

Transform::Output FirstIndexOffset::Run(ast::Module* in) {
  // First do a quick check to see if the transform has already been applied.
  for (ast::Variable* var : in->global_variables()) {
    if (auto* dec_var = var->As<ast::Variable>()) {
      if (dec_var->symbol() == in->RegisterSymbol(kBufferName)) {
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

  Symbol vertex_index_sym;
  Symbol instance_index_sym;

  // Lazilly construct the UniformBuffer on first call to
  // maybe_create_buffer_var()
  ast::Variable* buffer_var = nullptr;
  auto maybe_create_buffer_var = [&](ast::Module* mod) {
    if (buffer_var == nullptr) {
      buffer_var = AddUniformBuffer(mod);
    }
  };

  // Clone the AST, renaming the kVertexIdx and kInstanceIdx builtins, and add
  // a CreateFirstIndexOffset() statement to each function that uses one of
  // these builtins.

  Output out;
  ast::CloneContext(&out.module, in)
      .ReplaceAll(
          [&](ast::CloneContext* ctx, ast::Variable* var) -> ast::Variable* {
            for (ast::VariableDecoration* dec : var->decorations()) {
              if (auto* blt_dec = dec->As<ast::BuiltinDecoration>()) {
                ast::Builtin blt_type = blt_dec->value();
                if (blt_type == ast::Builtin::kVertexIdx) {
                  vertex_index_sym = var->symbol();
                  has_vertex_index_ = true;
                  return clone_variable_with_new_name(
                      ctx, var,
                      kIndexOffsetPrefix + in->SymbolToName(var->symbol()));
                } else if (blt_type == ast::Builtin::kInstanceIdx) {
                  instance_index_sym = var->symbol();
                  has_instance_index_ = true;
                  return clone_variable_with_new_name(
                      ctx, var,
                      kIndexOffsetPrefix + in->SymbolToName(var->symbol()));
                }
              }
            }
            return nullptr;  // Just clone var
          })
      .ReplaceAll(  // Note: This happens in the same pass as the rename above
                    // which determines the original builtin variable names,
                    // but this should be fine, as variables are cloned first.
          [&](ast::CloneContext* ctx, ast::Function* func) -> ast::Function* {
            maybe_create_buffer_var(ctx->mod);
            if (buffer_var == nullptr) {
              return nullptr;  // no transform need, just clone func
            }
            ast::StatementList statements;
            for (const auto& data :
                 func->local_referenced_builtin_variables()) {
              if (data.second->value() == ast::Builtin::kVertexIdx) {
                statements.emplace_back(CreateFirstIndexOffset(
                    in->SymbolToName(vertex_index_sym), kFirstVertexName,
                    buffer_var, ctx->mod));
              } else if (data.second->value() == ast::Builtin::kInstanceIdx) {
                statements.emplace_back(CreateFirstIndexOffset(
                    in->SymbolToName(instance_index_sym), kFirstInstanceName,
                    buffer_var, ctx->mod));
              }
            }
            return CloneWithStatementsAtStart(ctx, func, statements);
          })
      .Clone();

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
        mod->create<ast::StructMemberOffsetDecoration>(Source{}, offset));
    members.push_back(mod->create<ast::StructMember>(
        Source{}, mod->RegisterSymbol(kFirstVertexName), u32_type,
        std::move(member_dec)));
    vertex_index_offset_ = offset;
    offset += 4;
  }

  if (has_instance_index_) {
    ast::StructMemberDecorationList member_dec;
    member_dec.push_back(
        mod->create<ast::StructMemberOffsetDecoration>(Source{}, offset));
    members.push_back(mod->create<ast::StructMember>(
        Source{}, mod->RegisterSymbol(kFirstInstanceName), u32_type,
        std::move(member_dec)));
    instance_index_offset_ = offset;
    offset += 4;
  }

  ast::StructDecorationList decos;
  decos.push_back(mod->create<ast::StructBlockDecoration>(Source{}));

  auto* struct_type = mod->create<ast::type::Struct>(
      mod->RegisterSymbol(kStructName),
      mod->create<ast::Struct>(Source{}, std::move(members), std::move(decos)));

  auto* idx_var = mod->create<ast::Variable>(
      Source{},                          // source
      mod->RegisterSymbol(kBufferName),  // symbol
      ast::StorageClass::kUniform,       // storage_class
      struct_type,                       // type
      false,                             // is_const
      nullptr,                           // constructor
      ast::VariableDecorationList{
          mod->create<ast::BindingDecoration>(Source{}, binding_),
          mod->create<ast::GroupDecoration>(Source{}, group_),
      });  // decorations

  mod->AddGlobalVariable(idx_var);

  mod->AddConstructedType(struct_type);

  return idx_var;
}

ast::VariableDeclStatement* FirstIndexOffset::CreateFirstIndexOffset(
    const std::string& original_name,
    const std::string& field_name,
    ast::Variable* buffer_var,
    ast::Module* mod) {
  auto* buffer =
      mod->create<ast::IdentifierExpression>(Source{}, buffer_var->symbol());

  auto lhs_name = kIndexOffsetPrefix + original_name;
  auto* constructor = mod->create<ast::BinaryExpression>(
      Source{}, ast::BinaryOp::kAdd,
      mod->create<ast::IdentifierExpression>(Source{},
                                             mod->RegisterSymbol(lhs_name)),
      mod->create<ast::MemberAccessorExpression>(
          Source{}, buffer,
          mod->create<ast::IdentifierExpression>(
              Source{}, mod->RegisterSymbol(field_name))));
  auto* var =
      mod->create<ast::Variable>(Source{},                            // source
                                 mod->RegisterSymbol(original_name),  // symbol
                                 ast::StorageClass::kNone,  // storage_class
                                 mod->create<ast::type::U32>(),   // type
                                 true,                            // is_const
                                 constructor,                     // constructor
                                 ast::VariableDecorationList{});  // decorations
  return mod->create<ast::VariableDeclStatement>(Source{}, var);
}

}  // namespace transform
}  // namespace tint
