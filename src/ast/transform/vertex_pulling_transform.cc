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

#include "src/ast/transform/vertex_pulling_transform.h"

#include "src/ast/array_accessor_expression.h"
#include "src/ast/as_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/expression.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/module.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/struct.h"
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
#include "src/ast/variable_decl_statement.h"
#include "src/context.h"

namespace tint {
namespace ast {
namespace transform {

namespace {
// TODO(idanr): What to do if these names are already used?
static const char kVertexBufferNamePrefix[] = "tint_pulling_vertex_buffer_";
static const char kStructBufferName[] = "data";
static const char kPullingPosVarName[] = "tint_pulling_pos";
static const char kDefaultVertexIndexName[] = "tint_pulling_vertex_index";
static const char kDefaultInstanceIndexName[] = "tint_pulling_instance_index";
}  // namespace

VertexPullingTransform::VertexPullingTransform(Context* ctx, Module* mod)
    : ctx_(ctx), mod_(mod) {}

VertexPullingTransform::~VertexPullingTransform() = default;

void VertexPullingTransform::SetVertexState(
    std::unique_ptr<VertexStateDescriptor> vertex_state) {
  vertex_state_ = std::move(vertex_state);
}

void VertexPullingTransform::SetEntryPoint(std::string entry_point) {
  entry_point_name_ = std::move(entry_point);
}

void VertexPullingTransform::SetPullingBufferBindingSet(uint32_t number) {
  pulling_set_ = number;
}

bool VertexPullingTransform::Run() {
  // Check SetVertexState was called
  if (vertex_state_ == nullptr) {
    SetError("SetVertexState not called");
    return false;
  }

  // Find entry point
  EntryPoint* entry_point = nullptr;
  for (const auto& entry : mod_->entry_points()) {
    if (entry->name() == entry_point_name_ ||
        (entry->name().empty() &&
         entry->function_name() == entry_point_name_)) {
      entry_point = entry.get();
      break;
    }
  }

  if (entry_point == nullptr) {
    SetError("Vertex stage entry point not found");
    return false;
  }

  // Check entry point is the right stage
  if (entry_point->stage() != PipelineStage::kVertex) {
    SetError("Entry point is not for vertex stage");
    return false;
  }

  // Save the vertex function
  auto* vertex_func = mod_->FindFunctionByName(entry_point->function_name());

  // TODO(idanr): Need to check shader locations in descriptor cover all
  // attributes

  // TODO(idanr): Make sure we covered all error cases, to guarantee the
  // following stages will pass

  FindOrInsertVertexIndexIfUsed();
  FindOrInsertInstanceIndexIfUsed();
  ConvertVertexInputVariablesToPrivate();
  AddVertexStorageBuffers();
  AddVertexPullingPreamble(vertex_func);

  return true;
}

void VertexPullingTransform::SetError(const std::string& error) {
  error_ = error;
}

std::string VertexPullingTransform::GetVertexBufferName(uint32_t index) {
  return kVertexBufferNamePrefix + std::to_string(index);
}

void VertexPullingTransform::FindOrInsertVertexIndexIfUsed() {
  bool uses_vertex_step_mode = false;
  for (const VertexBufferLayoutDescriptor& buffer_layout :
       vertex_state_->vertex_buffers) {
    if (buffer_layout.step_mode == InputStepMode::kVertex) {
      uses_vertex_step_mode = true;
      break;
    }
  }
  if (!uses_vertex_step_mode) {
    return;
  }

  // Look for an existing vertex index builtin
  for (auto& v : mod_->global_variables()) {
    if (!v->IsDecorated() || v->storage_class() != StorageClass::kInput) {
      continue;
    }

    for (auto& d : v->AsDecorated()->decorations()) {
      if (d->IsBuiltin() && d->AsBuiltin()->value() == Builtin::kVertexIdx) {
        vertex_index_name_ = v->name();
        return;
      }
    }
  }

  // We didn't find a vertex index builtin, so create one
  vertex_index_name_ = kDefaultVertexIndexName;

  auto var = std::make_unique<DecoratedVariable>(std::make_unique<Variable>(
      vertex_index_name_, StorageClass::kInput, GetI32Type()));

  VariableDecorationList decorations;
  decorations.push_back(
      std::make_unique<BuiltinDecoration>(Builtin::kVertexIdx));

  var->set_decorations(std::move(decorations));
  mod_->AddGlobalVariable(std::move(var));
}

void VertexPullingTransform::FindOrInsertInstanceIndexIfUsed() {
  bool uses_instance_step_mode = false;
  for (const VertexBufferLayoutDescriptor& buffer_layout :
       vertex_state_->vertex_buffers) {
    if (buffer_layout.step_mode == InputStepMode::kInstance) {
      uses_instance_step_mode = true;
      break;
    }
  }
  if (!uses_instance_step_mode) {
    return;
  }

  // Look for an existing instance index builtin
  for (auto& v : mod_->global_variables()) {
    if (!v->IsDecorated() || v->storage_class() != StorageClass::kInput) {
      continue;
    }

    for (auto& d : v->AsDecorated()->decorations()) {
      if (d->IsBuiltin() && d->AsBuiltin()->value() == Builtin::kInstanceIdx) {
        instance_index_name_ = v->name();
        return;
      }
    }
  }

  // We didn't find an instance index builtin, so create one
  instance_index_name_ = kDefaultInstanceIndexName;

  auto var = std::make_unique<DecoratedVariable>(std::make_unique<Variable>(
      instance_index_name_, StorageClass::kInput, GetI32Type()));

  VariableDecorationList decorations;
  decorations.push_back(
      std::make_unique<BuiltinDecoration>(Builtin::kInstanceIdx));

  var->set_decorations(std::move(decorations));
  mod_->AddGlobalVariable(std::move(var));
}

void VertexPullingTransform::ConvertVertexInputVariablesToPrivate() {
  for (auto& v : mod_->global_variables()) {
    if (!v->IsDecorated() || v->storage_class() != StorageClass::kInput) {
      continue;
    }

    for (auto& d : v->AsDecorated()->decorations()) {
      if (!d->IsLocation()) {
        continue;
      }

      uint32_t location = d->AsLocation()->value();
      // This is where the replacement happens. Expressions use identifier
      // strings instead of pointers, so we don't need to update any other place
      // in the AST.
      v = std::make_unique<Variable>(v->name(), StorageClass::kPrivate,
                                     v->type());
      location_to_var_[location] = v.get();
      break;
    }
  }
}

void VertexPullingTransform::AddVertexStorageBuffers() {
  // TODO(idanr): Make this readonly https://github.com/gpuweb/gpuweb/issues/935
  // The array inside the struct definition
  auto internal_array = std::make_unique<type::ArrayType>(GetU32Type());
  internal_array->set_array_stride(4u);
  auto* internal_array_type = ctx_->type_mgr().Get(std::move(internal_array));

  // Creating the struct type
  StructMemberList members;
  StructMemberDecorationList member_dec;
  member_dec.push_back(std::make_unique<StructMemberOffsetDecoration>(0u));
  members.push_back(std::make_unique<StructMember>(
      kStructBufferName, internal_array_type, std::move(member_dec)));
  auto* struct_type = ctx_->type_mgr().Get(std::make_unique<type::StructType>(
      std::make_unique<Struct>(StructDecoration::kBlock, std::move(members))));

  for (uint32_t i = 0; i < vertex_state_->vertex_buffers.size(); ++i) {
    // The decorated variable with struct type
    auto var = std::make_unique<DecoratedVariable>(std::make_unique<Variable>(
        GetVertexBufferName(i), StorageClass::kStorageBuffer, struct_type));

    // Add decorations
    VariableDecorationList decorations;
    decorations.push_back(std::make_unique<BindingDecoration>(i));
    decorations.push_back(std::make_unique<SetDecoration>(pulling_set_));
    var->set_decorations(std::move(decorations));

    mod_->AddGlobalVariable(std::move(var));
  }
}

void VertexPullingTransform::AddVertexPullingPreamble(Function* vertex_func) {
  // Assign by looking at the vertex descriptor to find attributes with matching
  // location.

  // A block statement allowing us to use append instead of insert
  auto block = std::make_unique<BlockStatement>();

  // Declare the |kPullingPosVarName| variable in the shader
  auto pos_declaration =
      std::make_unique<VariableDeclStatement>(std::make_unique<Variable>(
          kPullingPosVarName, StorageClass::kFunction, GetI32Type()));

  // |kPullingPosVarName| refers to the byte location of the current read. We
  // declare a variable in the shader to avoid having to reuse Expression
  // objects.
  block->append(std::move(pos_declaration));

  for (uint32_t i = 0; i < vertex_state_->vertex_buffers.size(); ++i) {
    const VertexBufferLayoutDescriptor& buffer_layout =
        vertex_state_->vertex_buffers[i];

    for (const VertexAttributeDescriptor& attribute_desc :
         buffer_layout.attributes) {
      auto it = location_to_var_.find(attribute_desc.shader_location);
      if (it == location_to_var_.end()) {
        continue;
      }
      auto* v = it->second;

      // Identifier to index by
      auto index_identifier = std::make_unique<IdentifierExpression>(
          buffer_layout.step_mode == InputStepMode::kVertex
              ? vertex_index_name_
              : instance_index_name_);

      // An expression for the start of the read in the buffer in bytes
      auto pos_value = std::make_unique<BinaryExpression>(
          BinaryOp::kAdd,
          std::make_unique<BinaryExpression>(
              BinaryOp::kMultiply, std::move(index_identifier),
              GenUint(static_cast<uint32_t>(buffer_layout.array_stride))),
          GenUint(static_cast<uint32_t>(attribute_desc.offset)));

      // Update position of the read
      auto set_pos_expr = std::make_unique<AssignmentStatement>(
          CreatePullingPositionIdent(), std::move(pos_value));
      block->append(std::move(set_pos_expr));

      block->append(std::make_unique<AssignmentStatement>(
          std::make_unique<IdentifierExpression>(v->name()),
          AccessByFormat(i, attribute_desc.format)));
    }
  }

  vertex_func->body()->insert(0, std::move(block));
}

std::unique_ptr<Expression> VertexPullingTransform::GenUint(uint32_t value) {
  return std::make_unique<ScalarConstructorExpression>(
      std::make_unique<UintLiteral>(GetU32Type(), value));
}

std::unique_ptr<Expression>
VertexPullingTransform::CreatePullingPositionIdent() {
  return std::make_unique<IdentifierExpression>(kPullingPosVarName);
}

std::unique_ptr<Expression> VertexPullingTransform::AccessByFormat(
    uint32_t buffer,
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

std::unique_ptr<Expression> VertexPullingTransform::AccessU32(
    uint32_t buffer,
    std::unique_ptr<Expression> pos) {
  // Here we divide by 4, since the buffer is uint32 not uint8. The input buffer
  // has byte offsets for each attribute, and we will convert it to u32 indexes
  // by dividing. Then, that element is going to be read, and if needed,
  // unpacked into an appropriate variable. All reads should end up here as a
  // base case.
  return std::make_unique<ArrayAccessorExpression>(
      std::make_unique<MemberAccessorExpression>(
          std::make_unique<IdentifierExpression>(GetVertexBufferName(buffer)),
          std::make_unique<IdentifierExpression>(kStructBufferName)),
      std::make_unique<BinaryExpression>(BinaryOp::kDivide, std::move(pos),
                                         GenUint(4)));
}

std::unique_ptr<Expression> VertexPullingTransform::AccessI32(
    uint32_t buffer,
    std::unique_ptr<Expression> pos) {
  // as<T> reinterprets bits
  return std::make_unique<AsExpression>(GetI32Type(),
                                        AccessU32(buffer, std::move(pos)));
}

std::unique_ptr<Expression> VertexPullingTransform::AccessF32(
    uint32_t buffer,
    std::unique_ptr<Expression> pos) {
  // as<T> reinterprets bits
  return std::make_unique<AsExpression>(GetF32Type(),
                                        AccessU32(buffer, std::move(pos)));
}

std::unique_ptr<Expression> VertexPullingTransform::AccessPrimitive(
    uint32_t buffer,
    std::unique_ptr<Expression> pos,
    VertexFormat format) {
  // This function uses a position expression to read, rather than using the
  // position variable. This allows us to read from offset positions relative to
  // |kPullingPosVarName|. We can't call AccessByFormat because it reads only
  // from the position variable.
  switch (format) {
    case VertexFormat::kU32:
      return AccessU32(buffer, std::move(pos));
    case VertexFormat::kI32:
      return AccessI32(buffer, std::move(pos));
    case VertexFormat::kF32:
      return AccessF32(buffer, std::move(pos));
    default:
      return nullptr;
  }
}

std::unique_ptr<Expression> VertexPullingTransform::AccessVec(
    uint32_t buffer,
    uint32_t element_stride,
    type::Type* base_type,
    VertexFormat base_format,
    uint32_t count) {
  ExpressionList expr_list;
  for (uint32_t i = 0; i < count; ++i) {
    // Offset read position by element_stride for each component
    auto cur_pos = std::make_unique<BinaryExpression>(
        BinaryOp::kAdd, CreatePullingPositionIdent(),
        GenUint(element_stride * i));
    expr_list.push_back(
        AccessPrimitive(buffer, std::move(cur_pos), base_format));
  }

  return std::make_unique<TypeConstructorExpression>(
      ctx_->type_mgr().Get(
          std::make_unique<type::VectorType>(base_type, count)),
      std::move(expr_list));
}

type::Type* VertexPullingTransform::GetU32Type() {
  return ctx_->type_mgr().Get(std::make_unique<type::U32Type>());
}

type::Type* VertexPullingTransform::GetI32Type() {
  return ctx_->type_mgr().Get(std::make_unique<type::I32Type>());
}

type::Type* VertexPullingTransform::GetF32Type() {
  return ctx_->type_mgr().Get(std::make_unique<type::F32Type>());
}

VertexBufferLayoutDescriptor::VertexBufferLayoutDescriptor() = default;
VertexBufferLayoutDescriptor::VertexBufferLayoutDescriptor(
    uint64_t in_array_stride,
    InputStepMode in_step_mode,
    std::vector<VertexAttributeDescriptor> in_attributes)
    : array_stride(std::move(in_array_stride)),
      step_mode(std::move(in_step_mode)),
      attributes(std::move(in_attributes)) {}
VertexBufferLayoutDescriptor::VertexBufferLayoutDescriptor(
    const VertexBufferLayoutDescriptor& other)
    : array_stride(other.array_stride),
      step_mode(other.step_mode),
      attributes(other.attributes) {}
VertexBufferLayoutDescriptor::~VertexBufferLayoutDescriptor() = default;

VertexStateDescriptor::VertexStateDescriptor() = default;
VertexStateDescriptor::VertexStateDescriptor(
    std::vector<VertexBufferLayoutDescriptor> in_vertex_buffers)
    : vertex_buffers(std::move(in_vertex_buffers)) {}
VertexStateDescriptor::VertexStateDescriptor(const VertexStateDescriptor& other)
    : vertex_buffers(other.vertex_buffers) {}
VertexStateDescriptor::~VertexStateDescriptor() = default;

}  // namespace transform
}  // namespace ast
}  // namespace tint
