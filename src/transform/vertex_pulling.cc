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

#include "src/transform/vertex_pulling.h"

#include <utility>

#include "src/ast/array_accessor_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/stride_decoration.h"
#include "src/ast/struct.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/struct_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/uint_literal.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/clone_context.h"
#include "src/type/array_type.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/struct_type.h"
#include "src/type/u32_type.h"
#include "src/type/vector_type.h"

namespace tint {
namespace transform {
namespace {

static const char kVertexBufferNamePrefix[] = "_tint_pulling_vertex_buffer_";
static const char kStructBufferName[] = "_tint_vertex_data";
static const char kStructName[] = "TintVertexData";
static const char kPullingPosVarName[] = "_tint_pulling_pos";
static const char kDefaultVertexIndexName[] = "_tint_pulling_vertex_index";
static const char kDefaultInstanceIndexName[] = "_tint_pulling_instance_index";

}  // namespace

VertexPulling::VertexPulling() = default;
VertexPulling::~VertexPulling() = default;

void VertexPulling::SetVertexState(const VertexStateDescriptor& vertex_state) {
  cfg.vertex_state = vertex_state;
  cfg.vertex_state_set = true;
}

void VertexPulling::SetEntryPoint(std::string entry_point) {
  cfg.entry_point_name = std::move(entry_point);
}

void VertexPulling::SetPullingBufferBindingGroup(uint32_t number) {
  cfg.pulling_group = number;
}

void VertexPulling::SetPullingBufferBindingSet(uint32_t number) {
  cfg.pulling_group = number;
}

Transform::Output VertexPulling::Run(const Program* in) {
  // Check SetVertexState was called
  if (!cfg.vertex_state_set) {
    diag::Diagnostic err;
    err.severity = diag::Severity::Error;
    err.message = "SetVertexState not called";
    Output out;
    out.diagnostics.add(std::move(err));
    return out;
  }

  // Find entry point
  auto* func = in->AST().Functions().Find(
      in->Symbols().Get(cfg.entry_point_name), ast::PipelineStage::kVertex);
  if (func == nullptr) {
    diag::Diagnostic err;
    err.severity = diag::Severity::Error;
    err.message = "Vertex stage entry point not found";
    Output out;
    out.diagnostics.add(std::move(err));
    return out;
  }

  // TODO(idanr): Need to check shader locations in descriptor cover all
  // attributes

  // TODO(idanr): Make sure we covered all error cases, to guarantee the
  // following stages will pass
  Output out;

  CloneContext ctx(&out.program, in);
  State state{ctx, cfg};
  state.FindOrInsertVertexIndexIfUsed();
  state.FindOrInsertInstanceIndexIfUsed();
  state.ConvertVertexInputVariablesToPrivate();
  state.AddVertexStorageBuffers();

  for (auto& replacement : state.location_replacements) {
    ctx.Replace(replacement.from, replacement.to);
  }
  ctx.ReplaceAll([&](CloneContext*, ast::Function* f) -> ast::Function* {
    if (f == func) {
      return CloneWithStatementsAtStart(&ctx, f,
                                        {state.CreateVertexPullingPreamble()});
    }
    return nullptr;  // Just clone func
  });
  ctx.Clone();

  return out;
}

VertexPulling::Config::Config() = default;
VertexPulling::Config::Config(const Config&) = default;
VertexPulling::Config::~Config() = default;

VertexPulling::State::State(CloneContext& context, const Config& c)
    : ctx(context), cfg(c) {}

VertexPulling::State::State(const State&) = default;

VertexPulling::State::~State() = default;

std::string VertexPulling::State::GetVertexBufferName(uint32_t index) const {
  return kVertexBufferNamePrefix + std::to_string(index);
}

void VertexPulling::State::FindOrInsertVertexIndexIfUsed() {
  bool uses_vertex_step_mode = false;
  for (const VertexBufferLayoutDescriptor& buffer_layout : cfg.vertex_state) {
    if (buffer_layout.step_mode == InputStepMode::kVertex) {
      uses_vertex_step_mode = true;
      break;
    }
  }
  if (!uses_vertex_step_mode) {
    return;
  }

  // Look for an existing vertex index builtin
  for (auto* v : ctx.src->AST().GlobalVariables()) {
    if (v->storage_class() != ast::StorageClass::kInput) {
      continue;
    }

    for (auto* d : v->decorations()) {
      if (auto* builtin = d->As<ast::BuiltinDecoration>()) {
        if (builtin->value() == ast::Builtin::kVertexIndex) {
          vertex_index_name = ctx.src->Symbols().NameFor(v->symbol());
          return;
        }
      }
    }
  }

  // We didn't find a vertex index builtin, so create one
  vertex_index_name = kDefaultVertexIndexName;

  auto* var = ctx.dst->create<ast::Variable>(
      Source{},                                        // source
      ctx.dst->Symbols().Register(vertex_index_name),  // symbol
      ast::StorageClass::kInput,                       // storage_class
      GetI32Type(),                                    // type
      false,                                           // is_const
      nullptr,                                         // constructor
      ast::VariableDecorationList{
          ctx.dst->create<ast::BuiltinDecoration>(Source{},
                                                  ast::Builtin::kVertexIndex),
      });

  ctx.dst->AST().AddGlobalVariable(var);
}

void VertexPulling::State::FindOrInsertInstanceIndexIfUsed() {
  bool uses_instance_step_mode = false;
  for (const VertexBufferLayoutDescriptor& buffer_layout : cfg.vertex_state) {
    if (buffer_layout.step_mode == InputStepMode::kInstance) {
      uses_instance_step_mode = true;
      break;
    }
  }
  if (!uses_instance_step_mode) {
    return;
  }

  // Look for an existing instance index builtin
  for (auto* v : ctx.src->AST().GlobalVariables()) {
    if (v->storage_class() != ast::StorageClass::kInput) {
      continue;
    }

    for (auto* d : v->decorations()) {
      if (auto* builtin = d->As<ast::BuiltinDecoration>()) {
        if (builtin->value() == ast::Builtin::kInstanceIndex) {
          instance_index_name = ctx.src->Symbols().NameFor(v->symbol());
          return;
        }
      }
    }
  }

  // We didn't find an instance index builtin, so create one
  instance_index_name = kDefaultInstanceIndexName;

  auto* var = ctx.dst->create<ast::Variable>(
      Source{},                                          // source
      ctx.dst->Symbols().Register(instance_index_name),  // symbol
      ast::StorageClass::kInput,                         // storage_class
      GetI32Type(),                                      // type
      false,                                             // is_const
      nullptr,                                           // constructor
      ast::VariableDecorationList{
          ctx.dst->create<ast::BuiltinDecoration>(Source{},
                                                  ast::Builtin::kInstanceIndex),
      });
  ctx.dst->AST().AddGlobalVariable(var);
}

void VertexPulling::State::ConvertVertexInputVariablesToPrivate() {
  for (auto* v : ctx.src->AST().GlobalVariables()) {
    if (v->storage_class() != ast::StorageClass::kInput) {
      continue;
    }

    for (auto* d : v->decorations()) {
      if (auto* l = d->As<ast::LocationDecoration>()) {
        uint32_t location = l->value();
        // This is where the replacement is created. Expressions use identifier
        // strings instead of pointers, so we don't need to update any other
        // place in the AST.
        auto name = ctx.src->Symbols().NameFor(v->symbol());
        auto* replacement = ctx.dst->create<ast::Variable>(
            Source{},                           // source
            ctx.dst->Symbols().Register(name),  // symbol
            ast::StorageClass::kPrivate,        // storage_class
            ctx.Clone(v->type()),               // type
            false,                              // is_const
            nullptr,                            // constructor
            ast::VariableDecorationList{});     // decorations
        location_to_var[location] = replacement;
        location_replacements.emplace_back(LocationReplacement{v, replacement});
        break;
      }
    }
  }
}

void VertexPulling::State::AddVertexStorageBuffers() {
  // TODO(idanr): Make this readonly https://github.com/gpuweb/gpuweb/issues/935
  // The array inside the struct definition
  auto* internal_array_type = ctx.dst->create<type::Array>(
      GetU32Type(), 0,
      ast::ArrayDecorationList{
          ctx.dst->create<ast::StrideDecoration>(Source{}, 4u),
      });

  // Creating the struct type
  ast::StructMemberList members;
  ast::StructMemberDecorationList member_dec;
  member_dec.push_back(
      ctx.dst->create<ast::StructMemberOffsetDecoration>(Source{}, 0u));

  members.push_back(ctx.dst->create<ast::StructMember>(
      Source{}, ctx.dst->Symbols().Register(kStructBufferName),
      internal_array_type, std::move(member_dec)));

  ast::StructDecorationList decos;
  decos.push_back(ctx.dst->create<ast::StructBlockDecoration>(Source{}));

  auto* struct_type = ctx.dst->create<type::Struct>(
      ctx.dst->Symbols().Register(kStructName),
      ctx.dst->create<ast::Struct>(Source{}, std::move(members),
                                   std::move(decos)));

  for (uint32_t i = 0; i < cfg.vertex_state.size(); ++i) {
    // The decorated variable with struct type
    std::string name = GetVertexBufferName(i);
    auto* var = ctx.dst->create<ast::Variable>(
        Source{},                           // source
        ctx.dst->Symbols().Register(name),  // symbol
        ast::StorageClass::kStorage,        // storage_class
        struct_type,                        // type
        false,                              // is_const
        nullptr,                            // constructor
        ast::VariableDecorationList{
            ctx.dst->create<ast::BindingDecoration>(Source{}, i),
            ctx.dst->create<ast::GroupDecoration>(Source{}, cfg.pulling_group),
        });
    ctx.dst->AST().AddGlobalVariable(var);
  }
  ctx.dst->AST().AddConstructedType(struct_type);
}

ast::BlockStatement* VertexPulling::State::CreateVertexPullingPreamble() const {
  // Assign by looking at the vertex descriptor to find attributes with matching
  // location.

  ast::StatementList stmts;

  // Declare the |kPullingPosVarName| variable in the shader
  auto* pos_declaration = ctx.dst->create<ast::VariableDeclStatement>(
      Source{}, ctx.dst->create<ast::Variable>(
                    Source{},                                         // source
                    ctx.dst->Symbols().Register(kPullingPosVarName),  // symbol
                    ast::StorageClass::kFunction,     // storage_class
                    GetI32Type(),                     // type
                    false,                            // is_const
                    nullptr,                          // constructor
                    ast::VariableDecorationList{}));  // decorations

  // |kPullingPosVarName| refers to the byte location of the current read. We
  // declare a variable in the shader to avoid having to reuse Expression
  // objects.
  stmts.emplace_back(pos_declaration);

  for (uint32_t i = 0; i < cfg.vertex_state.size(); ++i) {
    const VertexBufferLayoutDescriptor& buffer_layout = cfg.vertex_state[i];

    for (const VertexAttributeDescriptor& attribute_desc :
         buffer_layout.attributes) {
      auto it = location_to_var.find(attribute_desc.shader_location);
      if (it == location_to_var.end()) {
        continue;
      }
      auto* v = it->second;

      auto name = buffer_layout.step_mode == InputStepMode::kVertex
                      ? vertex_index_name
                      : instance_index_name;
      // Identifier to index by
      auto* index_identifier = ctx.dst->create<ast::IdentifierExpression>(
          Source{}, ctx.dst->Symbols().Register(name));

      // An expression for the start of the read in the buffer in bytes
      auto* pos_value = ctx.dst->create<ast::BinaryExpression>(
          Source{}, ast::BinaryOp::kAdd,
          ctx.dst->create<ast::BinaryExpression>(
              Source{}, ast::BinaryOp::kMultiply, index_identifier,
              GenUint(static_cast<uint32_t>(buffer_layout.array_stride))),
          GenUint(static_cast<uint32_t>(attribute_desc.offset)));

      // Update position of the read
      auto* set_pos_expr = ctx.dst->create<ast::AssignmentStatement>(
          Source{}, CreatePullingPositionIdent(), pos_value);
      stmts.emplace_back(set_pos_expr);

      stmts.emplace_back(ctx.dst->create<ast::AssignmentStatement>(
          Source{},
          ctx.dst->create<ast::IdentifierExpression>(Source{}, v->symbol()),
          AccessByFormat(i, attribute_desc.format)));
    }
  }

  return ctx.dst->create<ast::BlockStatement>(Source{}, stmts);
}

ast::Expression* VertexPulling::State::GenUint(uint32_t value) const {
  return ctx.dst->create<ast::ScalarConstructorExpression>(
      Source{},
      ctx.dst->create<ast::UintLiteral>(Source{}, GetU32Type(), value));
}

ast::Expression* VertexPulling::State::CreatePullingPositionIdent() const {
  return ctx.dst->create<ast::IdentifierExpression>(
      Source{}, ctx.dst->Symbols().Register(kPullingPosVarName));
}

ast::Expression* VertexPulling::State::AccessByFormat(
    uint32_t buffer,
    VertexFormat format) const {
  // TODO(idanr): this doesn't account for the format of the attribute in the
  // shader. ex: vec<u32> in shader, and attribute claims VertexFormat::Float4
  // right now, we would try to assign a vec4<f32> to this attribute, but we
  // really need to assign a vec4<u32> by casting.
  // We could split this function to first do memory accesses and unpacking into
  // int/uint/float1-4/etc, then convert that variable to a var<in> with the
  // conversion defined in the WebGPU spec.
  switch (format) {
    case VertexFormat::kU32:
      return AccessU32(buffer, CreatePullingPositionIdent());
    case VertexFormat::kI32:
      return AccessI32(buffer, CreatePullingPositionIdent());
    case VertexFormat::kF32:
      return AccessF32(buffer, CreatePullingPositionIdent());
    case VertexFormat::kVec2F32:
      return AccessVec(buffer, 4, GetF32Type(), VertexFormat::kF32, 2);
    case VertexFormat::kVec3F32:
      return AccessVec(buffer, 4, GetF32Type(), VertexFormat::kF32, 3);
    case VertexFormat::kVec4F32:
      return AccessVec(buffer, 4, GetF32Type(), VertexFormat::kF32, 4);
    default:
      return nullptr;
  }
}

ast::Expression* VertexPulling::State::AccessU32(uint32_t buffer,
                                                 ast::Expression* pos) const {
  // Here we divide by 4, since the buffer is uint32 not uint8. The input buffer
  // has byte offsets for each attribute, and we will convert it to u32 indexes
  // by dividing. Then, that element is going to be read, and if needed,
  // unpacked into an appropriate variable. All reads should end up here as a
  // base case.
  auto vbuf_name = GetVertexBufferName(buffer);
  return ctx.dst->create<ast::ArrayAccessorExpression>(
      Source{},
      ctx.dst->create<ast::MemberAccessorExpression>(
          Source{},
          ctx.dst->create<ast::IdentifierExpression>(
              Source{}, ctx.dst->Symbols().Register(vbuf_name)),
          ctx.dst->create<ast::IdentifierExpression>(
              Source{}, ctx.dst->Symbols().Register(kStructBufferName))),
      ctx.dst->create<ast::BinaryExpression>(Source{}, ast::BinaryOp::kDivide,
                                             pos, GenUint(4)));
}

ast::Expression* VertexPulling::State::AccessI32(uint32_t buffer,
                                                 ast::Expression* pos) const {
  // as<T> reinterprets bits
  return ctx.dst->create<ast::BitcastExpression>(Source{}, GetI32Type(),
                                                 AccessU32(buffer, pos));
}

ast::Expression* VertexPulling::State::AccessF32(uint32_t buffer,
                                                 ast::Expression* pos) const {
  // as<T> reinterprets bits
  return ctx.dst->create<ast::BitcastExpression>(Source{}, GetF32Type(),
                                                 AccessU32(buffer, pos));
}

ast::Expression* VertexPulling::State::AccessPrimitive(
    uint32_t buffer,
    ast::Expression* pos,
    VertexFormat format) const {
  // This function uses a position expression to read, rather than using the
  // position variable. This allows us to read from offset positions relative to
  // |kPullingPosVarName|. We can't call AccessByFormat because it reads only
  // from the position variable.
  switch (format) {
    case VertexFormat::kU32:
      return AccessU32(buffer, pos);
    case VertexFormat::kI32:
      return AccessI32(buffer, pos);
    case VertexFormat::kF32:
      return AccessF32(buffer, pos);
    default:
      return nullptr;
  }
}

ast::Expression* VertexPulling::State::AccessVec(uint32_t buffer,
                                                 uint32_t element_stride,
                                                 type::Type* base_type,
                                                 VertexFormat base_format,
                                                 uint32_t count) const {
  ast::ExpressionList expr_list;
  for (uint32_t i = 0; i < count; ++i) {
    // Offset read position by element_stride for each component
    auto* cur_pos = ctx.dst->create<ast::BinaryExpression>(
        Source{}, ast::BinaryOp::kAdd, CreatePullingPositionIdent(),
        GenUint(element_stride * i));
    expr_list.push_back(AccessPrimitive(buffer, cur_pos, base_format));
  }

  return ctx.dst->create<ast::TypeConstructorExpression>(
      Source{}, ctx.dst->create<type::Vector>(base_type, count),
      std::move(expr_list));
}

type::Type* VertexPulling::State::GetU32Type() const {
  return ctx.dst->create<type::U32>();
}

type::Type* VertexPulling::State::GetI32Type() const {
  return ctx.dst->create<type::I32>();
}

type::Type* VertexPulling::State::GetF32Type() const {
  return ctx.dst->create<type::F32>();
}

VertexBufferLayoutDescriptor::VertexBufferLayoutDescriptor() = default;

VertexBufferLayoutDescriptor::VertexBufferLayoutDescriptor(
    uint64_t in_array_stride,
    InputStepMode in_step_mode,
    std::vector<VertexAttributeDescriptor> in_attributes)
    : array_stride(in_array_stride),
      step_mode(in_step_mode),
      attributes(std::move(in_attributes)) {}

VertexBufferLayoutDescriptor::VertexBufferLayoutDescriptor(
    const VertexBufferLayoutDescriptor& other) = default;

VertexBufferLayoutDescriptor& VertexBufferLayoutDescriptor::operator=(
    const VertexBufferLayoutDescriptor& other) = default;

VertexBufferLayoutDescriptor::~VertexBufferLayoutDescriptor() = default;

}  // namespace transform
}  // namespace tint
