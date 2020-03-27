// Copyright 2020 The Tint Authors.  //
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

#include "src/writer/spirv/builder.h"

#include <utility>

#include "spirv/unified1/spirv.h"
#include "src/ast/bool_literal.h"
#include "src/ast/float_literal.h"
#include "src/ast/int_literal.h"
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/uint_literal.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

uint32_t size_of(const std::vector<Instruction>& instructions) {
  uint32_t size = 0;
  for (const auto& inst : instructions)
    size += inst.word_length();

  return size;
}

uint32_t pipeline_stage_to_execution_model(ast::PipelineStage stage) {
  SpvExecutionModel model = SpvExecutionModelVertex;

  switch (stage) {
    case ast::PipelineStage::kFragment:
      model = SpvExecutionModelFragment;
      break;
    case ast::PipelineStage::kVertex:
      model = SpvExecutionModelVertex;
      break;
    case ast::PipelineStage::kCompute:
      model = SpvExecutionModelGLCompute;
      break;
    case ast::PipelineStage::kNone:
      model = SpvExecutionModelMax;
      break;
  }
  return model;
}

}  // namespace

Builder::Builder() = default;

Builder::~Builder() = default;

bool Builder::Build(const ast::Module& m) {
  push_preamble(spv::Op::OpCapability, {Operand::Int(SpvCapabilityShader)});
  push_preamble(spv::Op::OpCapability,
                {Operand::Int(SpvCapabilityVulkanMemoryModel)});

  for (const auto& imp : m.imports()) {
    GenerateImport(imp.get());
  }

  push_preamble(spv::Op::OpMemoryModel,
                {Operand::Int(SpvAddressingModelLogical),
                 Operand::Int(SpvMemoryModelVulkanKHR)});

  for (const auto& func : m.functions()) {
    if (!GenerateFunction(func.get())) {
      return false;
    }
  }

  for (const auto& ep : m.entry_points()) {
    if (!GenerateEntryPoint(ep.get())) {
      return false;
    }
  }

  return true;
}

Operand Builder::result_op() {
  return Operand::Int(next_id());
}

uint32_t Builder::total_size() const {
  // The 5 covers the magic, version, generator, id bound and reserved.
  uint32_t size = 5;

  size += size_of(preamble_);
  size += size_of(debug_);
  size += size_of(annotations_);
  size += size_of(types_);
  size += size_of(instructions_);

  return size;
}

void Builder::iterate(std::function<void(const Instruction&)> cb) const {
  for (const auto& inst : preamble_) {
    cb(inst);
  }
  for (const auto& inst : debug_) {
    cb(inst);
  }
  for (const auto& inst : annotations_) {
    cb(inst);
  }
  for (const auto& inst : types_) {
    cb(inst);
  }
  for (const auto& inst : instructions_) {
    cb(inst);
  }
}

bool Builder::GenerateEntryPoint(ast::EntryPoint* ep) {
  auto name = ep->name();
  if (name.empty()) {
    name = ep->function_name();
  }

  auto id = id_for_func_name(ep->function_name());
  if (id == 0) {
    error_ = "unable to find ID for function: " + ep->function_name();
    return false;
  }

  auto stage = pipeline_stage_to_execution_model(ep->stage());
  if (stage == SpvExecutionModelMax) {
    error_ = "Unknown pipeline stage provided";
    return false;
  }

  push_preamble(spv::Op::OpEntryPoint,
                {Operand::Int(stage), Operand::Int(id), Operand::String(name)});

  return true;
}

bool Builder::GenerateFunction(ast::Function* func) {
  uint32_t func_type_id = GenerateFunctionTypeIfNeeded(func);
  if (func_type_id == 0) {
    return false;
  }

  auto func_op = result_op();
  auto func_id = func_op.to_i();

  push_debug(spv::Op::OpName,
             {Operand::Int(func_id), Operand::String(func->name())});

  auto ret_id = GenerateTypeIfNeeded(func->return_type());
  if (ret_id == 0) {
    return false;
  }

  // TODO(dsinclair): Handle parameters
  push_inst(spv::Op::OpFunction, {Operand::Int(ret_id), func_op,
                                  Operand::Int(SpvFunctionControlMaskNone),
                                  Operand::Int(func_type_id)});
  push_inst(spv::Op::OpLabel, {result_op()});

  // TODO(dsinclair): Function body ...

  push_inst(spv::Op::OpFunctionEnd, {});

  func_name_to_id_[func->name()] = func_id;
  return true;
}

uint32_t Builder::GenerateFunctionTypeIfNeeded(ast::Function* func) {
  auto val = type_name_to_id_.find(func->type_name());
  if (val != type_name_to_id_.end()) {
    return val->second;
  }

  auto func_op = result_op();
  auto func_type_id = func_op.to_i();

  auto ret_id = GenerateTypeIfNeeded(func->return_type());
  if (ret_id == 0) {
    return 0;
  }

  // TODO(dsinclair): Handle parameters
  push_type(spv::Op::OpTypeFunction, {func_op, Operand::Int(ret_id)});

  type_name_to_id_[func->type_name()] = func_type_id;
  return func_type_id;
}

void Builder::GenerateImport(ast::Import* imp) {
  auto result = result_op();
  auto id = result.to_i();

  push_preamble(spv::Op::OpExtInstImport,
                {result, Operand::String(imp->path())});

  import_name_to_id_[imp->name()] = id;
}

uint32_t Builder::GenerateLiteralIfNeeded(ast::Literal* lit) {
  auto type_id = GenerateTypeIfNeeded(lit->type());
  if (type_id == 0) {
    return 0;
  }
  auto name = lit->name();
  auto val = const_to_id_.find(name);
  if (val != const_to_id_.end()) {
    return val->second;
  }

  auto result = result_op();
  auto result_id = result.to_i();

  if (lit->IsBool()) {
    if (lit->AsBool()->IsTrue()) {
      push_type(spv::Op::OpConstantTrue, {Operand::Int(type_id), result});
    } else {
      push_type(spv::Op::OpConstantFalse, {Operand::Int(type_id), result});
    }
  } else if (lit->IsInt()) {
    push_type(spv::Op::OpConstant, {Operand::Int(type_id), result,
                                    Operand::Int(lit->AsInt()->value())});
  } else if (lit->IsUint()) {
    push_type(spv::Op::OpConstant, {Operand::Int(type_id), result,
                                    Operand::Int(lit->AsUint()->value())});
  } else if (lit->IsFloat()) {
    push_type(spv::Op::OpConstant, {Operand::Int(type_id), result,
                                    Operand::Float(lit->AsFloat()->value())});
  } else {
    error_ = "unknown literal type";
    return 0;
  }

  const_to_id_[name] = result_id;
  return result_id;
}

uint32_t Builder::GenerateTypeIfNeeded(ast::type::Type* type) {
  if (type->IsAlias()) {
    return GenerateTypeIfNeeded(type->AsAlias()->type());
  }

  auto val = type_name_to_id_.find(type->type_name());
  if (val != type_name_to_id_.end()) {
    return val->second;
  }

  auto result = result_op();
  auto id = result.to_i();

  if (type->IsArray()) {
    if (!GenerateArrayType(type->AsArray(), result)) {
      return 0;
    }
  } else if (type->IsBool()) {
    push_type(spv::Op::OpTypeBool, {result});
  } else if (type->IsF32()) {
    push_type(spv::Op::OpTypeFloat, {result, Operand::Int(32)});
  } else if (type->IsI32()) {
    push_type(spv::Op::OpTypeInt, {result, Operand::Int(32), Operand::Int(1)});
  } else if (type->IsMatrix()) {
    if (!GenerateMatrixType(type->AsMatrix(), result)) {
      return 0;
    }
  } else if (type->IsPointer()) {
    if (!GeneratePointerType(type->AsPointer(), result)) {
      return 0;
    }
  } else if (type->IsStruct()) {
    if (!GenerateStructType(type->AsStruct(), result)) {
      return 0;
    }
  } else if (type->IsU32()) {
    push_type(spv::Op::OpTypeInt, {result, Operand::Int(32), Operand::Int(0)});
  } else if (type->IsVector()) {
    if (!GenerateVectorType(type->AsVector(), result)) {
      return 0;
    }
  } else if (type->IsVoid()) {
    push_type(spv::Op::OpTypeVoid, {result});
  } else {
    error_ = "unable to convert type: " + type->type_name();
    return 0;
  }

  type_name_to_id_[type->type_name()] = id;
  return id;
}

bool Builder::GenerateArrayType(ast::type::ArrayType* ary,
                                const Operand& result) {
  auto elem_type = GenerateTypeIfNeeded(ary->type());
  if (elem_type == 0) {
    return false;
  }

  if (ary->IsRuntimeArray()) {
    push_type(spv::Op::OpTypeRuntimeArray, {result, Operand::Int(elem_type)});
  } else {
    ast::type::U32Type u32;
    ast::IntLiteral ary_size(&u32, ary->size());
    auto len_id = GenerateLiteralIfNeeded(&ary_size);
    if (len_id == 0) {
      return false;
    }

    push_type(spv::Op::OpTypeArray,
              {result, Operand::Int(elem_type), Operand::Int(len_id)});
  }
  return true;
}

bool Builder::GenerateMatrixType(ast::type::MatrixType* mat,
                                 const Operand& result) {
  ast::type::VectorType col_type(mat->type(), mat->rows());
  auto col_type_id = GenerateTypeIfNeeded(&col_type);
  if (has_error()) {
    return false;
  }

  push_type(spv::Op::OpTypeMatrix,
            {result, Operand::Int(col_type_id), Operand::Int(mat->columns())});
  return true;
}

bool Builder::GeneratePointerType(ast::type::PointerType* ptr,
                                  const Operand& result) {
  auto pointee_id = GenerateTypeIfNeeded(ptr->type());
  if (pointee_id == 0) {
    return false;
  }

  auto stg_class = ConvertStorageClass(ptr->storage_class());
  if (stg_class == SpvStorageClassMax) {
    return false;
  }

  push_type(spv::Op::OpTypePointer,
            {result, Operand::Int(stg_class), Operand::Int(pointee_id)});

  return true;
}

bool Builder::GenerateStructType(ast::type::StructType* struct_type,
                                 const Operand& result) {
  auto struct_id = result.to_i();
  auto impl = struct_type->impl();

  if (!struct_type->name().empty()) {
    push_debug(spv::Op::OpName,
               {Operand::Int(struct_id), Operand::String(struct_type->name())});
  }

  std::vector<Operand> ops;
  ops.push_back(result);

  if (impl->decoration() == ast::StructDecoration::kBlock) {
    push_annot(spv::Op::OpDecorate,
               {Operand::Int(struct_id), Operand::Int(SpvDecorationBlock)});
  } else {
    if (impl->decoration() != ast::StructDecoration::kNone) {
      error_ = "unknown struct decoration";
      return false;
    }
  }

  auto& members = impl->members();
  for (uint32_t i = 0; i < members.size(); ++i) {
    auto mem_id = GenerateStructMember(struct_id, i, members[i].get());
    if (mem_id == 0) {
      return false;
    }

    ops.push_back(Operand::Int(mem_id));
  }

  push_type(spv::Op::OpTypeStruct, std::move(ops));
  return true;
}

uint32_t Builder::GenerateStructMember(uint32_t struct_id,
                                       uint32_t idx,
                                       ast::StructMember* member) {
  push_debug(spv::Op::OpMemberName, {Operand::Int(struct_id), Operand::Int(idx),
                                     Operand::String(member->name())});

  for (const auto& deco : member->decorations()) {
    if (deco->IsOffset()) {
      push_annot(spv::Op::OpMemberDecorate,
                 {Operand::Int(struct_id), Operand::Int(idx),
                  Operand::Int(SpvDecorationOffset),
                  Operand::Int(deco->AsOffset()->offset())});
    } else {
      error_ = "unknown struct member decoration";
      return 0;
    }
  }

  return GenerateTypeIfNeeded(member->type());
}

bool Builder::GenerateVectorType(ast::type::VectorType* vec,
                                 const Operand& result) {
  auto type_id = GenerateTypeIfNeeded(vec->type());
  if (has_error()) {
    return false;
  }

  push_type(spv::Op::OpTypeVector,
            {result, Operand::Int(type_id), Operand::Int(vec->size())});
  return true;
}

SpvStorageClass Builder::ConvertStorageClass(ast::StorageClass klass) const {
  switch (klass) {
    case ast::StorageClass::kInput:
      return SpvStorageClassInput;
    case ast::StorageClass::kOutput:
      return SpvStorageClassOutput;
    case ast::StorageClass::kUniform:
      return SpvStorageClassUniform;
    case ast::StorageClass::kWorkgroup:
      return SpvStorageClassWorkgroup;
    case ast::StorageClass::kUniformConstant:
      return SpvStorageClassUniformConstant;
    case ast::StorageClass::kStorageBuffer:
      return SpvStorageClassStorageBuffer;
    case ast::StorageClass::kImage:
      return SpvStorageClassImage;
    case ast::StorageClass::kPushConstant:
      return SpvStorageClassPushConstant;
    case ast::StorageClass::kPrivate:
      return SpvStorageClassPrivate;
    case ast::StorageClass::kFunction:
      return SpvStorageClassFunction;
    case ast::StorageClass::kNone:
      break;
  }
  return SpvStorageClassMax;
}

}  // namespace spirv
}  // namespace writer
}  // namespace tint
