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
#include "src/ast/type/array_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/uint_literal.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"

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

void VertexPulling::SetPullingBufferBindingSet(uint32_t number) {
  cfg.pulling_set = number;
}

Transform::Output VertexPulling::Run(ast::Module* in) {
  Output out;
  out.module = in->Clone();

  ast::Module* mod = &out.module;

  // Check SetVertexState was called
  if (!cfg.vertex_state_set) {
    diag::Diagnostic err;
    err.severity = diag::Severity::Error;
    err.message = "SetVertexState not called";
    out.diagnostics.add(std::move(err));
    return out;
  }

  // Find entry point
  auto* func = mod->FindFunctionByNameAndStage(cfg.entry_point_name,
                                               ast::PipelineStage::kVertex);
  if (func == nullptr) {
    diag::Diagnostic err;
    err.severity = diag::Severity::Error;
    err.message = "Vertex stage entry point not found";
    out.diagnostics.add(std::move(err));
    return out;
  }

  // Save the vertex function
  auto* vertex_func = mod->FindFunctionByName(func->name());

  // TODO(idanr): Need to check shader locations in descriptor cover all
  // attributes

  // TODO(idanr): Make sure we covered all error cases, to guarantee the
  // following stages will pass

  State state{mod, cfg};
  state.FindOrInsertVertexIndexIfUsed();
  state.FindOrInsertInstanceIndexIfUsed();
  state.ConvertVertexInputVariablesToPrivate();
  state.AddVertexStorageBuffers();
  state.AddVertexPullingPreamble(vertex_func);

  return out;
}

VertexPulling::Config::Config() = default;
VertexPulling::Config::Config(const Config&) = default;
VertexPulling::Config::~Config() = default;

VertexPulling::State::State(ast::Module* m, const Config& c) : mod(m), cfg(c) {}

VertexPulling::State::~State() = default;

std::string VertexPulling::State::GetVertexBufferName(uint32_t index) {
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
  for (auto* v : mod->global_variables()) {
    if (v->storage_class() != ast::StorageClass::kInput) {
      continue;
    }

    for (auto* d : v->decorations()) {
      if (auto* builtin = d->As<ast::BuiltinDecoration>()) {
        if (builtin->value() == ast::Builtin::kVertexIdx) {
          vertex_index_name = v->name();
          return;
        }
      }
    }
  }

  // We didn't find a vertex index builtin, so create one
  vertex_index_name = kDefaultVertexIndexName;

  auto* var =
      mod->create<ast::Variable>(Source{},                   // source
                                 vertex_index_name,          // name
                                 ast::StorageClass::kInput,  // storage_class
                                 GetI32Type(),               // type
                                 false,                      // is_const
                                 nullptr,                    // constructor
                                 ast::VariableDecorationList{
                                     // decorations
                                     mod->create<ast::BuiltinDecoration>(
                                         ast::Builtin::kVertexIdx, Source{}),
                                 });

  mod->AddGlobalVariable(var);
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
  for (auto* v : mod->global_variables()) {
    if (v->storage_class() != ast::StorageClass::kInput) {
      continue;
    }

    for (auto* d : v->decorations()) {
      if (auto* builtin = d->As<ast::BuiltinDecoration>()) {
        if (builtin->value() == ast::Builtin::kInstanceIdx) {
          instance_index_name = v->name();
          return;
        }
      }
    }
  }

  // We didn't find an instance index builtin, so create one
  instance_index_name = kDefaultInstanceIndexName;

  auto* var =
      mod->create<ast::Variable>(Source{},                   // source
                                 instance_index_name,        // name
                                 ast::StorageClass::kInput,  // storage_class
                                 GetI32Type(),               // type
                                 false,                      // is_const
                                 nullptr,                    // constructor
                                 ast::VariableDecorationList{
                                     // decorations
                                     mod->create<ast::BuiltinDecoration>(
                                         ast::Builtin::kInstanceIdx, Source{}),
                                 });
  mod->AddGlobalVariable(var);
}

void VertexPulling::State::ConvertVertexInputVariablesToPrivate() {
  for (auto*& v : mod->global_variables()) {
    if (v->storage_class() != ast::StorageClass::kInput) {
      continue;
    }

    for (auto* d : v->decorations()) {
      if (auto* l = d->As<ast::LocationDecoration>()) {
        uint32_t location = l->value();
        // This is where the replacement happens. Expressions use identifier
        // strings instead of pointers, so we don't need to update any other
        // place in the AST.
        v = mod->create<ast::Variable>(
            Source{},                        // source
            v->name(),                       // name
            ast::StorageClass::kPrivate,     // storage_class
            v->type(),                       // type
            false,                           // is_const
            nullptr,                         // constructor
            ast::VariableDecorationList{});  // decorations
        location_to_var[location] = v;
        break;
      }
    }
  }
}

void VertexPulling::State::AddVertexStorageBuffers() {
  // TODO(idanr): Make this readonly https://github.com/gpuweb/gpuweb/issues/935
  // The array inside the struct definition
  auto* internal_array_type = mod->create<ast::type::Array>(
      GetU32Type(), 0,
      ast::ArrayDecorationList{
          mod->create<ast::StrideDecoration>(4u, Source{}),
      });

  // Creating the struct type
  ast::StructMemberList members;
  ast::StructMemberDecorationList member_dec;
  member_dec.push_back(
      mod->create<ast::StructMemberOffsetDecoration>(0u, Source{}));

  members.push_back(mod->create<ast::StructMember>(
      kStructBufferName, internal_array_type, std::move(member_dec)));

  ast::StructDecorationList decos;
  decos.push_back(mod->create<ast::StructBlockDecoration>(Source{}));

  auto* struct_type = mod->create<ast::type::Struct>(
      kStructName,
      mod->create<ast::Struct>(std::move(decos), std::move(members)));

  for (uint32_t i = 0; i < cfg.vertex_state.size(); ++i) {
    // The decorated variable with struct type
    auto* var = mod->create<ast::Variable>(
        Source{},                           // source
        GetVertexBufferName(i),             // name
        ast::StorageClass::kStorageBuffer,  // storage_class
        struct_type,                        // type
        false,                              // is_const
        nullptr,                            // constructor
        ast::VariableDecorationList{
            // decorations
            mod->create<ast::BindingDecoration>(i, Source{}),
            mod->create<ast::SetDecoration>(cfg.pulling_set, Source{}),
        });
    mod->AddGlobalVariable(var);
  }
  mod->AddConstructedType(struct_type);
}

void VertexPulling::State::AddVertexPullingPreamble(
    ast::Function* vertex_func) {
  // Assign by looking at the vertex descriptor to find attributes with matching
  // location.

  // A block statement allowing us to use append instead of insert
  auto* block = mod->create<ast::BlockStatement>();

  // Declare the |kPullingPosVarName| variable in the shader
  auto* pos_declaration =
      mod->create<ast::VariableDeclStatement>(mod->create<ast::Variable>(
          Source{},                         // source
          kPullingPosVarName,               // name
          ast::StorageClass::kFunction,     // storage_class
          GetI32Type(),                     // type
          false,                            // is_const
          nullptr,                          // constructor
          ast::VariableDecorationList{}));  // decorations

  // |kPullingPosVarName| refers to the byte location of the current read. We
  // declare a variable in the shader to avoid having to reuse Expression
  // objects.
  block->append(pos_declaration);

  for (uint32_t i = 0; i < cfg.vertex_state.size(); ++i) {
    const VertexBufferLayoutDescriptor& buffer_layout = cfg.vertex_state[i];

    for (const VertexAttributeDescriptor& attribute_desc :
         buffer_layout.attributes) {
      auto it = location_to_var.find(attribute_desc.shader_location);
      if (it == location_to_var.end()) {
        continue;
      }
      auto* v = it->second;

      // Identifier to index by
      auto* index_identifier = mod->create<ast::IdentifierExpression>(
          buffer_layout.step_mode == InputStepMode::kVertex
              ? vertex_index_name
              : instance_index_name);

      // An expression for the start of the read in the buffer in bytes
      auto* pos_value = mod->create<ast::BinaryExpression>(
          ast::BinaryOp::kAdd,
          mod->create<ast::BinaryExpression>(
              ast::BinaryOp::kMultiply, index_identifier,
              GenUint(static_cast<uint32_t>(buffer_layout.array_stride))),
          GenUint(static_cast<uint32_t>(attribute_desc.offset)));

      // Update position of the read
      auto* set_pos_expr = mod->create<ast::AssignmentStatement>(
          CreatePullingPositionIdent(), pos_value);
      block->append(set_pos_expr);

      block->append(mod->create<ast::AssignmentStatement>(
          mod->create<ast::IdentifierExpression>(v->name()),
          AccessByFormat(i, attribute_desc.format)));
    }
  }

  vertex_func->body()->insert(0, block);
}

ast::Expression* VertexPulling::State::GenUint(uint32_t value) {
  return mod->create<ast::ScalarConstructorExpression>(
      mod->create<ast::UintLiteral>(GetU32Type(), value));
}

ast::Expression* VertexPulling::State::CreatePullingPositionIdent() {
  return mod->create<ast::IdentifierExpression>(kPullingPosVarName);
}

ast::Expression* VertexPulling::State::AccessByFormat(uint32_t buffer,
                                                      VertexFormat format) {
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
                                                 ast::Expression* pos) {
  // Here we divide by 4, since the buffer is uint32 not uint8. The input buffer
  // has byte offsets for each attribute, and we will convert it to u32 indexes
  // by dividing. Then, that element is going to be read, and if needed,
  // unpacked into an appropriate variable. All reads should end up here as a
  // base case.
  return mod->create<ast::ArrayAccessorExpression>(
      mod->create<ast::MemberAccessorExpression>(
          mod->create<ast::IdentifierExpression>(GetVertexBufferName(buffer)),
          mod->create<ast::IdentifierExpression>(kStructBufferName)),
      mod->create<ast::BinaryExpression>(ast::BinaryOp::kDivide, pos,
                                         GenUint(4)));
}

ast::Expression* VertexPulling::State::AccessI32(uint32_t buffer,
                                                 ast::Expression* pos) {
  // as<T> reinterprets bits
  return mod->create<ast::BitcastExpression>(GetI32Type(),
                                             AccessU32(buffer, pos));
}

ast::Expression* VertexPulling::State::AccessF32(uint32_t buffer,
                                                 ast::Expression* pos) {
  // as<T> reinterprets bits
  return mod->create<ast::BitcastExpression>(GetF32Type(),
                                             AccessU32(buffer, pos));
}

ast::Expression* VertexPulling::State::AccessPrimitive(uint32_t buffer,
                                                       ast::Expression* pos,
                                                       VertexFormat format) {
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
                                                 ast::type::Type* base_type,
                                                 VertexFormat base_format,
                                                 uint32_t count) {
  ast::ExpressionList expr_list;
  for (uint32_t i = 0; i < count; ++i) {
    // Offset read position by element_stride for each component
    auto* cur_pos = mod->create<ast::BinaryExpression>(
        ast::BinaryOp::kAdd, CreatePullingPositionIdent(),
        GenUint(element_stride * i));
    expr_list.push_back(AccessPrimitive(buffer, cur_pos, base_format));
  }

  return mod->create<ast::TypeConstructorExpression>(
      mod->create<ast::type::Vector>(base_type, count), std::move(expr_list));
}

ast::type::Type* VertexPulling::State::GetU32Type() {
  return mod->create<ast::type::U32>();
}

ast::type::Type* VertexPulling::State::GetI32Type() {
  return mod->create<ast::type::I32>();
}

ast::type::Type* VertexPulling::State::GetF32Type() {
  return mod->create<ast::type::F32>();
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
