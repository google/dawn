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

#include <memory>
#include <utility>

#include "src/ast/struct_block_decoration.h"
#include "src/program_builder.h"
#include "src/semantic/function.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::FirstIndexOffset::Data);

namespace tint {
namespace transform {
namespace {

constexpr char kStructName[] = "TintFirstIndexOffsetData";
constexpr char kBufferName[] = "tint_first_index_data";
constexpr char kFirstVertexName[] = "tint_first_vertex_index";
constexpr char kFirstInstanceName[] = "tint_first_instance_index";
constexpr char kIndexOffsetPrefix[] = "tint_first_index_offset_";

ast::Variable* clone_variable_with_new_name(CloneContext* ctx,
                                            ast::Variable* in,
                                            std::string new_name) {
  // Clone arguments outside of create() call to have deterministic ordering
  auto source = ctx->Clone(in->source());
  auto symbol = ctx->dst->Symbols().Register(new_name);
  auto* type = ctx->Clone(in->type());
  auto* constructor = ctx->Clone(in->constructor());
  auto decorations = ctx->Clone(in->decorations());
  return ctx->dst->create<ast::Variable>(
      source, symbol, in->declared_storage_class(), type, in->is_const(),
      constructor, decorations);
}

}  // namespace

FirstIndexOffset::Data::Data(bool has_vtx_index,
                             bool has_inst_index,
                             uint32_t first_vtx_offset,
                             uint32_t first_inst_offset)
    : has_vertex_index(has_vtx_index),
      has_instance_index(has_inst_index),
      first_vertex_offset(first_vtx_offset),
      first_instance_offset(first_inst_offset) {}

FirstIndexOffset::Data::Data(const Data&) = default;

FirstIndexOffset::Data::~Data() = default;

FirstIndexOffset::FirstIndexOffset(uint32_t binding, uint32_t group)
    : binding_(binding), group_(group) {}

FirstIndexOffset::~FirstIndexOffset() = default;

Transform::Output FirstIndexOffset::Run(const Program* in) {
  ProgramBuilder out;

  // First do a quick check to see if the transform has already been applied.
  for (ast::Variable* var : in->AST().GlobalVariables()) {
    if (auto* dec_var = var->As<ast::Variable>()) {
      if (dec_var->symbol() == in->Symbols().Get(kBufferName)) {
        out.Diagnostics().add_error(
            "First index offset transform has already been applied.");
        return Output(Program(std::move(out)));
      }
    }
  }

  State state{&out, binding_, group_};

  Symbol vertex_index_sym;
  Symbol instance_index_sym;

  // Lazily construct the UniformBuffer on first call to
  // maybe_create_buffer_var()
  ast::Variable* buffer_var = nullptr;
  auto maybe_create_buffer_var = [&]() {
    if (buffer_var == nullptr) {
      buffer_var = state.AddUniformBuffer();
    }
  };

  // Clone the AST, renaming the kVertexIndex and kInstanceIndex builtins, and
  // add a CreateFirstIndexOffset() statement to each function that uses one of
  // these builtins.

  CloneContext ctx(&out, in);
  ctx.ReplaceAll([&](ast::Variable* var) -> ast::Variable* {
    for (ast::Decoration* dec : var->decorations()) {
      if (auto* blt_dec = dec->As<ast::BuiltinDecoration>()) {
        ast::Builtin blt_type = blt_dec->value();
        if (blt_type == ast::Builtin::kVertexIndex) {
          vertex_index_sym = var->symbol();
          state.has_vertex_index = true;
          return clone_variable_with_new_name(
              &ctx, var,
              kIndexOffsetPrefix + in->Symbols().NameFor(var->symbol()));
        } else if (blt_type == ast::Builtin::kInstanceIndex) {
          instance_index_sym = var->symbol();
          state.has_instance_index = true;
          return clone_variable_with_new_name(
              &ctx, var,
              kIndexOffsetPrefix + in->Symbols().NameFor(var->symbol()));
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
        auto* func_sem = in->Sem().Get(func);
        ast::StatementList statements;
        for (const auto& data : func_sem->LocalReferencedBuiltinVariables()) {
          if (data.second->value() == ast::Builtin::kVertexIndex) {
            statements.emplace_back(state.CreateFirstIndexOffset(
                in->Symbols().NameFor(vertex_index_sym), kFirstVertexName,
                buffer_var));
          } else if (data.second->value() == ast::Builtin::kInstanceIndex) {
            statements.emplace_back(state.CreateFirstIndexOffset(
                in->Symbols().NameFor(instance_index_sym), kFirstInstanceName,
                buffer_var));
          }
        }
        return CloneWithStatementsAtStart(&ctx, func, statements);
      });
  ctx.Clone();

  return Output(Program(std::move(out)),
                std::make_unique<Data>(
                    state.has_vertex_index, state.has_instance_index,
                    state.vertex_index_offset, state.instance_index_offset));
}

ast::Variable* FirstIndexOffset::State::AddUniformBuffer() {
  auto* u32_type = dst->create<type::U32>();
  uint32_t offset = 0;
  ast::StructMemberList members;
  if (has_vertex_index) {
    members.push_back(dst->create<ast::StructMember>(
        Source{}, dst->Symbols().Register(kFirstVertexName), u32_type,
        ast::DecorationList{}));
    vertex_index_offset = offset;
    offset += 4;
  }

  if (has_instance_index) {
    members.push_back(dst->create<ast::StructMember>(
        Source{}, dst->Symbols().Register(kFirstInstanceName), u32_type,
        ast::DecorationList{}));
    instance_index_offset = offset;
    offset += 4;
  }

  ast::DecorationList decos;
  decos.push_back(dst->create<ast::StructBlockDecoration>(Source{}));

  auto* struct_type = dst->create<type::Struct>(
      dst->Symbols().Register(kStructName),
      dst->create<ast::Struct>(Source{}, std::move(members), std::move(decos)));

  auto* idx_var = dst->create<ast::Variable>(
      Source{},                              // source
      dst->Symbols().Register(kBufferName),  // symbol
      ast::StorageClass::kUniform,           // storage_class
      struct_type,                           // type
      false,                                 // is_const
      nullptr,                               // constructor
      ast::DecorationList{
          dst->create<ast::BindingDecoration>(Source{}, binding),
          dst->create<ast::GroupDecoration>(Source{}, group),
      });

  dst->AST().AddGlobalVariable(idx_var);

  dst->AST().AddConstructedType(struct_type);

  return idx_var;
}

ast::VariableDeclStatement* FirstIndexOffset::State::CreateFirstIndexOffset(
    const std::string& original_name,
    const std::string& field_name,
    ast::Variable* buffer_var) {
  auto* buffer =
      dst->create<ast::IdentifierExpression>(Source{}, buffer_var->symbol());

  auto lhs_name = kIndexOffsetPrefix + original_name;
  auto* constructor = dst->create<ast::BinaryExpression>(
      Source{}, ast::BinaryOp::kAdd,
      dst->create<ast::IdentifierExpression>(Source{},
                                             dst->Symbols().Register(lhs_name)),
      dst->create<ast::MemberAccessorExpression>(
          Source{}, buffer,
          dst->create<ast::IdentifierExpression>(
              Source{}, dst->Symbols().Register(field_name))));
  auto* var = dst->create<ast::Variable>(
      Source{},                                // source
      dst->Symbols().Register(original_name),  // symbol
      ast::StorageClass::kNone,                // storage_class
      dst->create<type::U32>(),                // type
      true,                                    // is_const
      constructor,                             // constructor
      ast::DecorationList{});                  // decorations
  return dst->create<ast::VariableDeclStatement>(Source{}, var);
}

}  // namespace transform
}  // namespace tint
