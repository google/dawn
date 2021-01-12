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

#include <algorithm>
#include <iostream>
#include <limits>
#include <sstream>
#include <utility>

#include "spirv/unified1/GLSL.std.450.h"
#include "spirv/unified1/spirv.h"
#include "src/ast/array_accessor_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/binding_decoration.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/block_statement.h"
#include "src/ast/bool_literal.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/call_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/case_statement.h"
#include "src/ast/constant_id_decoration.h"
#include "src/ast/constructor_expression.h"
#include "src/ast/else_statement.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/intrinsic.h"
#include "src/ast/location_decoration.h"
#include "src/ast/loop_statement.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/null_literal.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/set_decoration.h"
#include "src/ast/sint_literal.h"
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/switch_statement.h"
#include "src/ast/type/access_control_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/depth_texture_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/multisampled_texture_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/sampled_texture_type.h"
#include "src/ast/type/storage_texture_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/texture_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/uint_literal.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/writer/append_vector.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

const char kGLSLstd450[] = "GLSL.std.450";

uint32_t size_of(const InstructionList& instructions) {
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

bool LastIsFallthrough(const ast::BlockStatement* stmts) {
  return !stmts->empty() && stmts->last()->Is<ast::FallthroughStatement>();
}

// A terminator is anything which will case a SPIR-V terminator to be emitted.
// This means things like breaks, fallthroughs and continues which all emit an
// OpBranch or return for the OpReturn emission.
bool LastIsTerminator(const ast::BlockStatement* stmts) {
  if (stmts->empty()) {
    return false;
  }

  auto* last = stmts->last();
  return last->Is<ast::BreakStatement>() ||
         last->Is<ast::ContinueStatement>() ||
         last->Is<ast::DiscardStatement>() ||
         last->Is<ast::ReturnStatement>() ||
         last->Is<ast::FallthroughStatement>();
}

uint32_t IndexFromName(char name) {
  switch (name) {
    case 'x':
    case 'r':
      return 0;
    case 'y':
    case 'g':
      return 1;
    case 'z':
    case 'b':
      return 2;
    case 'w':
    case 'a':
      return 3;
  }
  return std::numeric_limits<uint32_t>::max();
}

/// Returns the matrix type that is `type` or that is wrapped by
/// one or more levels of an arrays inside of `type`.
/// @param type the given type, which must not be null
/// @returns the nested matrix type, or nullptr if none
ast::type::Matrix* GetNestedMatrixType(ast::type::Type* type) {
  while (auto* arr = type->As<ast::type::Array>()) {
    type = arr->type();
  }
  return type->As<ast::type::Matrix>();
}

uint32_t intrinsic_to_glsl_method(ast::type::Type* type,
                                  ast::Intrinsic intrinsic) {
  switch (intrinsic) {
    case ast::Intrinsic::kAbs:
      if (type->is_float_scalar_or_vector()) {
        return GLSLstd450FAbs;
      } else {
        return GLSLstd450SAbs;
      }
    case ast::Intrinsic::kAcos:
      return GLSLstd450Acos;
    case ast::Intrinsic::kAsin:
      return GLSLstd450Asin;
    case ast::Intrinsic::kAtan:
      return GLSLstd450Atan;
    case ast::Intrinsic::kAtan2:
      return GLSLstd450Atan2;
    case ast::Intrinsic::kCeil:
      return GLSLstd450Ceil;
    case ast::Intrinsic::kClamp:
      if (type->is_float_scalar_or_vector()) {
        return GLSLstd450NClamp;
      } else if (type->is_unsigned_scalar_or_vector()) {
        return GLSLstd450UClamp;
      } else {
        return GLSLstd450SClamp;
      }
    case ast::Intrinsic::kCos:
      return GLSLstd450Cos;
    case ast::Intrinsic::kCosh:
      return GLSLstd450Cosh;
    case ast::Intrinsic::kCross:
      return GLSLstd450Cross;
    case ast::Intrinsic::kDeterminant:
      return GLSLstd450Determinant;
    case ast::Intrinsic::kDistance:
      return GLSLstd450Distance;
    case ast::Intrinsic::kExp:
      return GLSLstd450Exp;
    case ast::Intrinsic::kExp2:
      return GLSLstd450Exp2;
    case ast::Intrinsic::kFaceForward:
      return GLSLstd450FaceForward;
    case ast::Intrinsic::kFloor:
      return GLSLstd450Floor;
    case ast::Intrinsic::kFma:
      return GLSLstd450Fma;
    case ast::Intrinsic::kFract:
      return GLSLstd450Fract;
    case ast::Intrinsic::kFrexp:
      return GLSLstd450Frexp;
    case ast::Intrinsic::kInverseSqrt:
      return GLSLstd450InverseSqrt;
    case ast::Intrinsic::kLdexp:
      return GLSLstd450Ldexp;
    case ast::Intrinsic::kLength:
      return GLSLstd450Length;
    case ast::Intrinsic::kLog:
      return GLSLstd450Log;
    case ast::Intrinsic::kLog2:
      return GLSLstd450Log2;
    case ast::Intrinsic::kMax:
      if (type->is_float_scalar_or_vector()) {
        return GLSLstd450NMax;
      } else if (type->is_unsigned_scalar_or_vector()) {
        return GLSLstd450UMax;
      } else {
        return GLSLstd450SMax;
      }
    case ast::Intrinsic::kMin:
      if (type->is_float_scalar_or_vector()) {
        return GLSLstd450NMin;
      } else if (type->is_unsigned_scalar_or_vector()) {
        return GLSLstd450UMin;
      } else {
        return GLSLstd450SMin;
      }
    case ast::Intrinsic::kMix:
      return GLSLstd450FMix;
    case ast::Intrinsic::kModf:
      return GLSLstd450Modf;
    case ast::Intrinsic::kNormalize:
      return GLSLstd450Normalize;
    case ast::Intrinsic::kPow:
      return GLSLstd450Pow;
    case ast::Intrinsic::kReflect:
      return GLSLstd450Reflect;
    case ast::Intrinsic::kRound:
      return GLSLstd450Round;
    case ast::Intrinsic::kSign:
      return GLSLstd450FSign;
    case ast::Intrinsic::kSin:
      return GLSLstd450Sin;
    case ast::Intrinsic::kSinh:
      return GLSLstd450Sinh;
    case ast::Intrinsic::kSmoothStep:
      return GLSLstd450SmoothStep;
    case ast::Intrinsic::kSqrt:
      return GLSLstd450Sqrt;
    case ast::Intrinsic::kStep:
      return GLSLstd450Step;
    case ast::Intrinsic::kTan:
      return GLSLstd450Tan;
    case ast::Intrinsic::kTanh:
      return GLSLstd450Tanh;
    case ast::Intrinsic::kTrunc:
      return GLSLstd450Trunc;
    default:
      break;
  }
  return 0;
}

/// @return the vector element type if ty is a vector, otherwise return ty.
ast::type::Type* ElementTypeOf(ast::type::Type* ty) {
  if (auto* v = ty->As<ast::type::Vector>()) {
    return v->type();
  }
  return ty;
}

}  // namespace

Builder::AccessorInfo::AccessorInfo() : source_id(0), source_type(nullptr) {}

Builder::AccessorInfo::~AccessorInfo() {}

Builder::Builder(ast::Module* mod) : mod_(mod), scope_stack_({}) {}

Builder::~Builder() = default;

bool Builder::Build() {
  push_capability(SpvCapabilityShader);

  push_memory_model(spv::Op::OpMemoryModel,
                    {Operand::Int(SpvAddressingModelLogical),
                     Operand::Int(SpvMemoryModelGLSL450)});

  for (auto* var : mod_->global_variables()) {
    if (!GenerateGlobalVariable(var)) {
      return false;
    }
  }

  for (auto* func : mod_->functions()) {
    if (!GenerateFunction(func)) {
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

  size += size_of(capabilities_);
  size += size_of(extensions_);
  size += size_of(ext_imports_);
  size += size_of(memory_model_);
  size += size_of(entry_points_);
  size += size_of(execution_modes_);
  size += size_of(debug_);
  size += size_of(annotations_);
  size += size_of(types_);
  for (const auto& func : functions_) {
    size += func.word_length();
  }

  return size;
}

void Builder::iterate(std::function<void(const Instruction&)> cb) const {
  for (const auto& inst : capabilities_) {
    cb(inst);
  }
  for (const auto& inst : extensions_) {
    cb(inst);
  }
  for (const auto& inst : ext_imports_) {
    cb(inst);
  }
  for (const auto& inst : memory_model_) {
    cb(inst);
  }
  for (const auto& inst : entry_points_) {
    cb(inst);
  }
  for (const auto& inst : execution_modes_) {
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
  for (const auto& func : functions_) {
    func.iterate(cb);
  }
}

void Builder::push_capability(uint32_t cap) {
  if (capability_set_.count(cap) == 0) {
    capability_set_.insert(cap);
    capabilities_.push_back(
        Instruction{spv::Op::OpCapability, {Operand::Int(cap)}});
  }
}

bool Builder::GenerateLabel(uint32_t id) {
  if (!push_function_inst(spv::Op::OpLabel, {Operand::Int(id)})) {
    return false;
  }
  current_label_id_ = id;
  return true;
}

uint32_t Builder::GenerateU32Literal(uint32_t val) {
  ast::type::U32 u32;
  ast::SintLiteral lit(Source{}, &u32, val);
  return GenerateLiteralIfNeeded(nullptr, &lit);
}

bool Builder::GenerateAssignStatement(ast::AssignmentStatement* assign) {
  auto lhs_id = GenerateExpression(assign->lhs());
  if (lhs_id == 0) {
    return false;
  }
  auto rhs_id = GenerateExpression(assign->rhs());
  if (rhs_id == 0) {
    return false;
  }

  // If the thing we're assigning is a pointer then we must load it first.
  rhs_id = GenerateLoadIfNeeded(assign->rhs()->result_type(), rhs_id);

  return GenerateStore(lhs_id, rhs_id);
}

bool Builder::GenerateBreakStatement(ast::BreakStatement*) {
  if (merge_stack_.empty()) {
    error_ = "Attempted to break without a merge block";
    return false;
  }
  if (!push_function_inst(spv::Op::OpBranch,
                          {Operand::Int(merge_stack_.back())})) {
    return false;
  }
  return true;
}

bool Builder::GenerateContinueStatement(ast::ContinueStatement*) {
  if (continue_stack_.empty()) {
    error_ = "Attempted to continue without a continue block";
    return false;
  }
  if (!push_function_inst(spv::Op::OpBranch,
                          {Operand::Int(continue_stack_.back())})) {
    return false;
  }
  return true;
}

// TODO(dsinclair): This is generating an OpKill but the semantics of kill
// haven't been defined for WGSL yet. So, this may need to change.
// https://github.com/gpuweb/gpuweb/issues/676
bool Builder::GenerateDiscardStatement(ast::DiscardStatement*) {
  if (!push_function_inst(spv::Op::OpKill, {})) {
    return false;
  }
  return true;
}

bool Builder::GenerateEntryPoint(ast::Function* func, uint32_t id) {
  auto stage = pipeline_stage_to_execution_model(func->pipeline_stage());
  if (stage == SpvExecutionModelMax) {
    error_ = "Unknown pipeline stage provided";
    return false;
  }

  OperandList operands = {Operand::Int(stage), Operand::Int(id),
                          Operand::String(mod_->SymbolToName(func->symbol()))};

  for (const auto* var : func->referenced_module_variables()) {
    // For SPIR-V 1.3 we only output Input/output variables. If we update to
    // SPIR-V 1.4 or later this should be all variables.
    if (var->storage_class() != ast::StorageClass::kInput &&
        var->storage_class() != ast::StorageClass::kOutput) {
      continue;
    }

    uint32_t var_id;
    if (!scope_stack_.get(var->symbol(), &var_id)) {
      error_ = "unable to find ID for global variable: " +
               mod_->SymbolToName(var->symbol());
      return false;
    }

    operands.push_back(Operand::Int(var_id));
  }
  push_entry_point(spv::Op::OpEntryPoint, operands);

  return true;
}

bool Builder::GenerateExecutionModes(ast::Function* func, uint32_t id) {
  // WGSL fragment shader origin is upper left
  if (func->pipeline_stage() == ast::PipelineStage::kFragment) {
    push_execution_mode(
        spv::Op::OpExecutionMode,
        {Operand::Int(id), Operand::Int(SpvExecutionModeOriginUpperLeft)});
  } else if (func->pipeline_stage() == ast::PipelineStage::kCompute) {
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t z = 0;
    std::tie(x, y, z) = func->workgroup_size();
    push_execution_mode(
        spv::Op::OpExecutionMode,
        {Operand::Int(id), Operand::Int(SpvExecutionModeLocalSize),
         Operand::Int(x), Operand::Int(y), Operand::Int(z)});
  }

  for (auto builtin : func->referenced_builtin_variables()) {
    if (builtin.second->value() == ast::Builtin::kFragDepth) {
      push_execution_mode(
          spv::Op::OpExecutionMode,
          {Operand::Int(id), Operand::Int(SpvExecutionModeDepthReplacing)});
    }
  }

  return true;
}

uint32_t Builder::GenerateExpression(ast::Expression* expr) {
  if (auto* a = expr->As<ast::ArrayAccessorExpression>()) {
    return GenerateAccessorExpression(a);
  }
  if (auto* b = expr->As<ast::BinaryExpression>()) {
    return GenerateBinaryExpression(b);
  }
  if (auto* b = expr->As<ast::BitcastExpression>()) {
    return GenerateBitcastExpression(b);
  }
  if (auto* c = expr->As<ast::CallExpression>()) {
    return GenerateCallExpression(c);
  }
  if (auto* c = expr->As<ast::ConstructorExpression>()) {
    return GenerateConstructorExpression(nullptr, c, false);
  }
  if (auto* i = expr->As<ast::IdentifierExpression>()) {
    return GenerateIdentifierExpression(i);
  }
  if (auto* m = expr->As<ast::MemberAccessorExpression>()) {
    return GenerateAccessorExpression(m);
  }
  if (auto* u = expr->As<ast::UnaryOpExpression>()) {
    return GenerateUnaryOpExpression(u);
  }

  error_ = "unknown expression type: " + expr->str();
  return 0;
}

bool Builder::GenerateFunction(ast::Function* func) {
  uint32_t func_type_id = GenerateFunctionTypeIfNeeded(func);
  if (func_type_id == 0) {
    return false;
  }

  auto func_op = result_op();
  auto func_id = func_op.to_i();

  push_debug(spv::Op::OpName,
             {Operand::Int(func_id),
              Operand::String(mod_->SymbolToName(func->symbol()))});

  auto ret_id = GenerateTypeIfNeeded(func->return_type());
  if (ret_id == 0) {
    return false;
  }

  scope_stack_.push_scope();

  auto definition_inst = Instruction{
      spv::Op::OpFunction,
      {Operand::Int(ret_id), func_op, Operand::Int(SpvFunctionControlMaskNone),
       Operand::Int(func_type_id)}};

  InstructionList params;
  for (auto* param : func->params()) {
    auto param_op = result_op();
    auto param_id = param_op.to_i();

    auto param_type_id = GenerateTypeIfNeeded(param->type());
    if (param_type_id == 0) {
      return false;
    }

    push_debug(spv::Op::OpName,
               {Operand::Int(param_id),
                Operand::String(mod_->SymbolToName(param->symbol()))});
    params.push_back(Instruction{spv::Op::OpFunctionParameter,
                                 {Operand::Int(param_type_id), param_op}});

    scope_stack_.set(param->symbol(), param_id);
  }

  push_function(Function{definition_inst, result_op(), std::move(params)});

  for (auto* stmt : *func->body()) {
    if (!GenerateStatement(stmt)) {
      return false;
    }
  }

  if (func->IsEntryPoint()) {
    if (!GenerateEntryPoint(func, func_id)) {
      return false;
    }
    if (!GenerateExecutionModes(func, func_id)) {
      return false;
    }
  }

  scope_stack_.pop_scope();

  func_symbol_to_id_[func->symbol()] = func_id;

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

  OperandList ops = {func_op, Operand::Int(ret_id)};
  for (auto* param : func->params()) {
    auto param_type_id = GenerateTypeIfNeeded(param->type());
    if (param_type_id == 0) {
      return 0;
    }
    ops.push_back(Operand::Int(param_type_id));
  }

  push_type(spv::Op::OpTypeFunction, std::move(ops));

  type_name_to_id_[func->type_name()] = func_type_id;
  return func_type_id;
}

bool Builder::GenerateFunctionVariable(ast::Variable* var) {
  uint32_t init_id = 0;
  if (var->has_constructor()) {
    init_id = GenerateExpression(var->constructor());
    if (init_id == 0) {
      return false;
    }
    init_id = GenerateLoadIfNeeded(var->constructor()->result_type(), init_id);
  }

  if (var->is_const()) {
    if (!var->has_constructor()) {
      error_ = "missing constructor for constant";
      return false;
    }
    scope_stack_.set(var->symbol(), init_id);
    spirv_id_to_variable_[init_id] = var;
    return true;
  }

  auto result = result_op();
  auto var_id = result.to_i();
  auto sc = ast::StorageClass::kFunction;
  ast::type::Pointer pt(var->type(), sc);
  auto type_id = GenerateTypeIfNeeded(&pt);
  if (type_id == 0) {
    return false;
  }

  push_debug(spv::Op::OpName,
             {Operand::Int(var_id),
              Operand::String(mod_->SymbolToName(var->symbol()))});

  // TODO(dsinclair) We could detect if the constructor is fully const and emit
  // an initializer value for the variable instead of doing the OpLoad.
  ast::NullLiteral nl(Source{}, var->type()->UnwrapPtrIfNeeded());
  auto null_id = GenerateLiteralIfNeeded(var, &nl);
  if (null_id == 0) {
    return 0;
  }
  push_function_var({Operand::Int(type_id), result,
                     Operand::Int(ConvertStorageClass(sc)),
                     Operand::Int(null_id)});

  if (var->has_constructor()) {
    if (!GenerateStore(var_id, init_id)) {
      return false;
    }
  }

  scope_stack_.set(var->symbol(), var_id);
  spirv_id_to_variable_[var_id] = var;

  return true;
}

bool Builder::GenerateStore(uint32_t to, uint32_t from) {
  return push_function_inst(spv::Op::OpStore,
                            {Operand::Int(to), Operand::Int(from)});
}

bool Builder::GenerateGlobalVariable(ast::Variable* var) {
  uint32_t init_id = 0;
  if (var->has_constructor()) {
    if (!var->constructor()->Is<ast::ConstructorExpression>()) {
      error_ = "scalar constructor expected";
      return false;
    }

    init_id = GenerateConstructorExpression(
        var, var->constructor()->As<ast::ConstructorExpression>(), true);
    if (init_id == 0) {
      return false;
    }
  }

  if (var->is_const()) {
    if (!var->has_constructor()) {
      error_ = "missing constructor for constant";
      return false;
    }
    push_debug(spv::Op::OpName,
               {Operand::Int(init_id),
                Operand::String(mod_->SymbolToName(var->symbol()))});

    scope_stack_.set_global(var->symbol(), init_id);
    spirv_id_to_variable_[init_id] = var;
    return true;
  }

  auto result = result_op();
  auto var_id = result.to_i();

  auto sc = var->storage_class() == ast::StorageClass::kNone
                ? ast::StorageClass::kPrivate
                : var->storage_class();

  ast::type::Pointer pt(var->type(), sc);
  auto type_id = GenerateTypeIfNeeded(&pt);
  if (type_id == 0) {
    return false;
  }

  push_debug(spv::Op::OpName,
             {Operand::Int(var_id),
              Operand::String(mod_->SymbolToName(var->symbol()))});

  auto* type = var->type()->UnwrapAll();

  OperandList ops = {Operand::Int(type_id), result,
                     Operand::Int(ConvertStorageClass(sc))};
  if (var->has_constructor()) {
    ops.push_back(Operand::Int(init_id));
  } else if (auto* tex = type->As<ast::type::Texture>()) {
    // Decorate storage texture variables with NonRead/Writeable if needed.
    if (auto* storage = tex->As<ast::type::StorageTexture>()) {
      switch (storage->access()) {
        case ast::AccessControl::kWriteOnly:
          push_annot(
              spv::Op::OpDecorate,
              {Operand::Int(var_id), Operand::Int(SpvDecorationNonReadable)});
          break;
        case ast::AccessControl::kReadOnly:
          push_annot(
              spv::Op::OpDecorate,
              {Operand::Int(var_id), Operand::Int(SpvDecorationNonWritable)});
          break;
        case ast::AccessControl::kReadWrite:
          break;
      }
    }
  } else if (!type->Is<ast::type::Sampler>()) {
    // Certain cases require us to generate a constructor value.
    //
    // 1- ConstantId's must be attached to the OpConstant, if we have a
    //    variable with a constant_id that doesn't have a constructor we make
    //    one
    // 2- If we don't have a constructor and we're an Output or Private variable
    //    then WGSL requires an initializer.
    if (var->HasConstantIdDecoration()) {
      if (type->Is<ast::type::F32>()) {
        ast::FloatLiteral l(Source{}, type, 0.0f);
        init_id = GenerateLiteralIfNeeded(var, &l);
      } else if (type->Is<ast::type::U32>()) {
        ast::UintLiteral l(Source{}, type, 0);
        init_id = GenerateLiteralIfNeeded(var, &l);
      } else if (type->Is<ast::type::I32>()) {
        ast::SintLiteral l(Source{}, type, 0);
        init_id = GenerateLiteralIfNeeded(var, &l);
      } else if (type->Is<ast::type::Bool>()) {
        ast::BoolLiteral l(Source{}, type, false);
        init_id = GenerateLiteralIfNeeded(var, &l);
      } else {
        error_ = "invalid type for constant_id, must be scalar";
        return false;
      }
      if (init_id == 0) {
        return 0;
      }
      ops.push_back(Operand::Int(init_id));
    } else if (var->storage_class() == ast::StorageClass::kPrivate ||
               var->storage_class() == ast::StorageClass::kNone ||
               var->storage_class() == ast::StorageClass::kOutput) {
      ast::NullLiteral nl(Source{}, type);
      init_id = GenerateLiteralIfNeeded(var, &nl);
      if (init_id == 0) {
        return 0;
      }
      ops.push_back(Operand::Int(init_id));
    }
  }

  push_type(spv::Op::OpVariable, std::move(ops));

  for (auto* deco : var->decorations()) {
    if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
      push_annot(spv::Op::OpDecorate,
                 {Operand::Int(var_id), Operand::Int(SpvDecorationBuiltIn),
                  Operand::Int(ConvertBuiltin(builtin->value()))});
    } else if (auto* location = deco->As<ast::LocationDecoration>()) {
      push_annot(spv::Op::OpDecorate,
                 {Operand::Int(var_id), Operand::Int(SpvDecorationLocation),
                  Operand::Int(location->value())});
    } else if (auto* binding = deco->As<ast::BindingDecoration>()) {
      push_annot(spv::Op::OpDecorate,
                 {Operand::Int(var_id), Operand::Int(SpvDecorationBinding),
                  Operand::Int(binding->value())});
    } else if (auto* set = deco->As<ast::SetDecoration>()) {
      push_annot(spv::Op::OpDecorate, {Operand::Int(var_id),
                                       Operand::Int(SpvDecorationDescriptorSet),
                                       Operand::Int(set->value())});
    } else if (deco->Is<ast::ConstantIdDecoration>()) {
      // Spec constants are handled elsewhere
    } else {
      error_ = "unknown decoration";
      return false;
    }
  }

  scope_stack_.set_global(var->symbol(), var_id);
  spirv_id_to_variable_[var_id] = var;
  return true;
}

bool Builder::GenerateArrayAccessor(ast::ArrayAccessorExpression* expr,
                                    AccessorInfo* info) {
  auto idx_id = GenerateExpression(expr->idx_expr());
  if (idx_id == 0) {
    return 0;
  }
  idx_id = GenerateLoadIfNeeded(expr->idx_expr()->result_type(), idx_id);

  // If the source is a pointer we access chain into it. We also access chain
  // into an array of non-scalar types.
  if (info->source_type->Is<ast::type::Pointer>() ||
      (info->source_type->Is<ast::type::Array>() &&
       !info->source_type->As<ast::type::Array>()->type()->is_scalar())) {
    info->access_chain_indices.push_back(idx_id);
    info->source_type = expr->result_type();
    return true;
  }

  auto result_type_id = GenerateTypeIfNeeded(expr->result_type());
  if (result_type_id == 0) {
    return false;
  }

  // We don't have a pointer, so we have to extract value from the vector
  auto extract = result_op();
  auto extract_id = extract.to_i();

  if (!push_function_inst(
          spv::Op::OpVectorExtractDynamic,
          {Operand::Int(result_type_id), extract, Operand::Int(info->source_id),
           Operand::Int(idx_id)})) {
    return false;
  }

  info->source_id = extract_id;
  info->source_type = expr->result_type();

  return true;
}

bool Builder::GenerateMemberAccessor(ast::MemberAccessorExpression* expr,
                                     AccessorInfo* info) {
  auto* data_type =
      expr->structure()->result_type()->UnwrapPtrIfNeeded()->UnwrapIfNeeded();

  // If the data_type is a structure we're accessing a member, if it's a
  // vector we're accessing a swizzle.
  if (data_type->Is<ast::type::Struct>()) {
    if (!info->source_type->Is<ast::type::Pointer>()) {
      error_ =
          "Attempting to access a struct member on a non-pointer. Something is "
          "wrong";
      return false;
    }

    auto* strct = data_type->As<ast::type::Struct>()->impl();
    auto symbol = expr->member()->symbol();

    uint32_t i = 0;
    for (; i < strct->members().size(); ++i) {
      auto* member = strct->members()[i];
      if (member->symbol() == symbol) {
        break;
      }
    }

    auto idx_id = GenerateU32Literal(i);
    if (idx_id == 0) {
      return 0;
    }
    info->access_chain_indices.push_back(idx_id);
    info->source_type = expr->result_type();
    return true;
  }

  if (!data_type->Is<ast::type::Vector>()) {
    error_ = "Member accessor without a struct or vector. Something is wrong";
    return false;
  }

  // TODO(dsinclair): Swizzle stuff
  auto swiz = mod_->SymbolToName(expr->member()->symbol());
  // Single element swizzle is either an access chain or a composite extract
  if (swiz.size() == 1) {
    auto val = IndexFromName(swiz[0]);
    if (val == std::numeric_limits<uint32_t>::max()) {
      error_ = "invalid swizzle name: " + swiz;
      return false;
    }

    if (info->source_type->Is<ast::type::Pointer>()) {
      auto idx_id = GenerateU32Literal(val);
      if (idx_id == 0) {
        return 0;
      }
      info->access_chain_indices.push_back(idx_id);
    } else {
      auto result_type_id = GenerateTypeIfNeeded(expr->result_type());
      if (result_type_id == 0) {
        return 0;
      }

      auto extract = result_op();
      auto extract_id = extract.to_i();
      if (!push_function_inst(
              spv::Op::OpCompositeExtract,
              {Operand::Int(result_type_id), extract,
               Operand::Int(info->source_id), Operand::Int(val)})) {
        return false;
      }

      info->source_id = extract_id;
      info->source_type = expr->result_type();
    }
    return true;
  }

  // Store the type away as it may change if we run the access chain
  auto* incoming_type = info->source_type;

  // Multi-item extract is a VectorShuffle. We have to emit any existing access
  // chain data, then load the access chain and shuffle that.
  if (!info->access_chain_indices.empty()) {
    auto result_type_id = GenerateTypeIfNeeded(info->source_type);
    if (result_type_id == 0) {
      return 0;
    }
    auto extract = result_op();
    auto extract_id = extract.to_i();

    OperandList ops = {Operand::Int(result_type_id), extract,
                       Operand::Int(info->source_id)};
    for (auto id : info->access_chain_indices) {
      ops.push_back(Operand::Int(id));
    }

    if (!push_function_inst(spv::Op::OpAccessChain, ops)) {
      return false;
    }

    info->source_id = GenerateLoadIfNeeded(expr->result_type(), extract_id);
    info->source_type = expr->result_type()->UnwrapPtrIfNeeded();
    info->access_chain_indices.clear();
  }

  auto result_type_id = GenerateTypeIfNeeded(expr->result_type());
  if (result_type_id == 0) {
    return false;
  }

  auto vec_id = GenerateLoadIfNeeded(incoming_type, info->source_id);

  auto result = result_op();
  auto result_id = result.to_i();

  OperandList ops = {Operand::Int(result_type_id), result, Operand::Int(vec_id),
                     Operand::Int(vec_id)};

  for (uint32_t i = 0; i < swiz.size(); ++i) {
    auto val = IndexFromName(swiz[i]);
    if (val == std::numeric_limits<uint32_t>::max()) {
      error_ = "invalid swizzle name: " + swiz;
      return false;
    }

    ops.push_back(Operand::Int(val));
  }

  if (!push_function_inst(spv::Op::OpVectorShuffle, ops)) {
    return false;
  }
  info->source_id = result_id;
  info->source_type = expr->result_type();

  return true;
}

uint32_t Builder::GenerateAccessorExpression(ast::Expression* expr) {
  assert(expr->Is<ast::ArrayAccessorExpression>() ||
         expr->Is<ast::MemberAccessorExpression>());

  // Gather a list of all the member and array accessors that are in this chain.
  // The list is built in reverse order as that's the order we need to access
  // the chain.
  std::vector<ast::Expression*> accessors;
  ast::Expression* source = expr;
  while (true) {
    if (auto* array = source->As<ast::ArrayAccessorExpression>()) {
      accessors.insert(accessors.begin(), source);
      source = array->array();
    } else if (auto* member = source->As<ast::MemberAccessorExpression>()) {
      accessors.insert(accessors.begin(), source);
      source = member->structure();
    } else {
      break;
    }
  }

  AccessorInfo info;
  info.source_id = GenerateExpression(source);
  if (info.source_id == 0) {
    return 0;
  }
  info.source_type = source->result_type();

  // If our initial access is into an array of non-scalar types, and that array
  // is not a pointer, then we need to load that array into a variable in order
  // to access chain into the array.
  if (auto* array = accessors[0]->As<ast::ArrayAccessorExpression>()) {
    auto* ary_res_type = array->array()->result_type();

    if (!ary_res_type->Is<ast::type::Pointer>() &&
        (ary_res_type->Is<ast::type::Array>() &&
         !ary_res_type->As<ast::type::Array>()->type()->is_scalar())) {
      ast::type::Pointer ptr(ary_res_type, ast::StorageClass::kFunction);
      auto result_type_id = GenerateTypeIfNeeded(&ptr);
      if (result_type_id == 0) {
        return 0;
      }

      auto ary_result = result_op();

      ast::NullLiteral nl(Source{}, ary_res_type);
      auto init = GenerateLiteralIfNeeded(nullptr, &nl);

      // If we're access chaining into an array then we must be in a function
      push_function_var(
          {Operand::Int(result_type_id), ary_result,
           Operand::Int(ConvertStorageClass(ast::StorageClass::kFunction)),
           Operand::Int(init)});

      if (!push_function_inst(spv::Op::OpStore,
                              {ary_result, Operand::Int(info.source_id)})) {
        return false;
      }

      info.source_id = ary_result.to_i();
    }
  }

  std::vector<uint32_t> access_chain_indices;
  for (auto* accessor : accessors) {
    if (auto* array = accessor->As<ast::ArrayAccessorExpression>()) {
      if (!GenerateArrayAccessor(array, &info)) {
        return 0;
      }
    } else if (auto* member = accessor->As<ast::MemberAccessorExpression>()) {
      if (!GenerateMemberAccessor(member, &info)) {
        return 0;
      }

    } else {
      error_ = "invalid accessor in list: " + accessor->str();
      return 0;
    }
  }

  if (!info.access_chain_indices.empty()) {
    auto result_type_id = GenerateTypeIfNeeded(expr->result_type());
    if (result_type_id == 0) {
      return 0;
    }

    auto result = result_op();
    auto result_id = result.to_i();

    OperandList ops = {Operand::Int(result_type_id), result,
                       Operand::Int(info.source_id)};
    for (auto id : info.access_chain_indices) {
      ops.push_back(Operand::Int(id));
    }

    if (!push_function_inst(spv::Op::OpAccessChain, ops)) {
      return false;
    }
    info.source_id = result_id;
  }

  return info.source_id;
}

uint32_t Builder::GenerateIdentifierExpression(
    ast::IdentifierExpression* expr) {
  uint32_t val = 0;
  if (scope_stack_.get(expr->symbol(), &val)) {
    return val;
  }

  error_ = "unable to find variable with identifier: " +
           mod_->SymbolToName(expr->symbol());
  return 0;
}

uint32_t Builder::GenerateLoadIfNeeded(ast::type::Type* type, uint32_t id) {
  if (!type->Is<ast::type::Pointer>()) {
    return id;
  }

  auto type_id = GenerateTypeIfNeeded(type->UnwrapPtrIfNeeded());
  auto result = result_op();
  auto result_id = result.to_i();
  if (!push_function_inst(spv::Op::OpLoad,
                          {Operand::Int(type_id), result, Operand::Int(id)})) {
    return false;
  }
  return result_id;
}

uint32_t Builder::GenerateUnaryOpExpression(ast::UnaryOpExpression* expr) {
  auto result = result_op();
  auto result_id = result.to_i();

  auto val_id = GenerateExpression(expr->expr());
  if (val_id == 0) {
    return 0;
  }
  val_id = GenerateLoadIfNeeded(expr->expr()->result_type(), val_id);

  auto type_id = GenerateTypeIfNeeded(expr->result_type());
  if (type_id == 0) {
    return 0;
  }

  spv::Op op = spv::Op::OpNop;
  if (expr->op() == ast::UnaryOp::kNegation) {
    if (expr->result_type()->is_float_scalar_or_vector()) {
      op = spv::Op::OpFNegate;
    } else {
      op = spv::Op::OpSNegate;
    }
  } else if (expr->op() == ast::UnaryOp::kNot) {
    op = spv::Op::OpLogicalNot;
  }
  if (op == spv::Op::OpNop) {
    error_ = "invalid unary op type";
    return 0;
  }

  if (!push_function_inst(
          op, {Operand::Int(type_id), result, Operand::Int(val_id)})) {
    return false;
  }

  return result_id;
}

void Builder::GenerateGLSLstd450Import() {
  if (import_name_to_id_.find(kGLSLstd450) != import_name_to_id_.end()) {
    return;
  }

  auto result = result_op();
  auto id = result.to_i();

  push_ext_import(spv::Op::OpExtInstImport,
                  {result, Operand::String(kGLSLstd450)});

  import_name_to_id_[kGLSLstd450] = id;
}

uint32_t Builder::GenerateConstructorExpression(
    ast::Variable* var,
    ast::ConstructorExpression* expr,
    bool is_global_init) {
  if (auto* scalar = expr->As<ast::ScalarConstructorExpression>()) {
    return GenerateLiteralIfNeeded(var, scalar->literal());
  }
  if (auto* type = expr->As<ast::TypeConstructorExpression>()) {
    return GenerateTypeConstructorExpression(type, is_global_init);
  }

  error_ = "unknown constructor expression";
  return 0;
}

bool Builder::is_constructor_const(ast::Expression* expr, bool is_global_init) {
  auto* constructor = expr->As<ast::ConstructorExpression>();
  if (constructor == nullptr) {
    return false;
  }
  if (constructor->Is<ast::ScalarConstructorExpression>()) {
    return true;
  }

  auto* tc = constructor->As<ast::TypeConstructorExpression>();
  auto* result_type = tc->type()->UnwrapAll();
  for (size_t i = 0; i < tc->values().size(); ++i) {
    auto* e = tc->values()[i];

    if (!e->Is<ast::ConstructorExpression>()) {
      if (is_global_init) {
        error_ = "constructor must be a constant expression";
        return false;
      }
      return false;
    }
    if (!is_constructor_const(e, is_global_init)) {
      return false;
    }
    if (has_error()) {
      return false;
    }

    auto* sc = e->As<ast::ScalarConstructorExpression>();
    if (result_type->Is<ast::type::Vector>() && sc == nullptr) {
      return false;
    }

    // This should all be handled by |is_constructor_const| call above
    if (sc == nullptr) {
      continue;
    }

    ast::type::Type* subtype = result_type->UnwrapAll();
    if (auto* vec = subtype->As<ast::type::Vector>()) {
      subtype = vec->type()->UnwrapAll();
    } else if (auto* mat = subtype->As<ast::type::Matrix>()) {
      subtype = mat->type()->UnwrapAll();
    } else if (auto* arr = subtype->As<ast::type::Array>()) {
      subtype = arr->type()->UnwrapAll();
    } else if (auto* str = subtype->As<ast::type::Struct>()) {
      subtype = str->impl()->members()[i]->type()->UnwrapAll();
    }
    if (subtype != sc->result_type()->UnwrapAll()) {
      return false;
    }
  }
  return true;
}

uint32_t Builder::GenerateTypeConstructorExpression(
    ast::TypeConstructorExpression* init,
    bool is_global_init) {
  auto& values = init->values();

  // Generate the zero initializer if there are no values provided.
  if (values.empty()) {
    ast::NullLiteral nl(Source{}, init->type()->UnwrapPtrIfNeeded());
    return GenerateLiteralIfNeeded(nullptr, &nl);
  }

  std::ostringstream out;
  out << "__const";

  auto* result_type = init->type()->UnwrapAll();
  bool constructor_is_const = is_constructor_const(init, is_global_init);
  if (has_error()) {
    return 0;
  }

  bool can_cast_or_copy = result_type->is_scalar();

  if (auto* res_vec = result_type->As<ast::type::Vector>()) {
    if (res_vec->type()->is_scalar()) {
      auto* value_type = values[0]->result_type()->UnwrapAll();
      if (auto* val_vec = value_type->As<ast::type::Vector>()) {
        if (val_vec->type()->is_scalar()) {
          can_cast_or_copy = res_vec->size() == val_vec->size();
        }
      }
    }
  }

  if (can_cast_or_copy) {
    return GenerateCastOrCopyOrPassthrough(result_type, values[0]);
  }

  auto type_id = GenerateTypeIfNeeded(init->type());
  if (type_id == 0) {
    return 0;
  }

  bool result_is_constant_composite = constructor_is_const;
  bool result_is_spec_composite = false;

  if (auto* vec = result_type->As<ast::type::Vector>()) {
    result_type = vec->type();
  }

  OperandList ops;
  for (auto* e : values) {
    uint32_t id = 0;
    if (constructor_is_const) {
      id = GenerateConstructorExpression(
          nullptr, e->As<ast::ConstructorExpression>(), is_global_init);
    } else {
      id = GenerateExpression(e);
      id = GenerateLoadIfNeeded(e->result_type(), id);
    }
    if (id == 0) {
      return 0;
    }

    auto* value_type = e->result_type()->UnwrapPtrIfNeeded();
    // If the result and value types are the same we can just use the object.
    // If the result is not a vector then we should have validated that the
    // value type is a correctly sized vector so we can just use it directly.
    if (result_type == value_type || result_type->Is<ast::type::Matrix>() ||
        result_type->Is<ast::type::Array>() ||
        result_type->Is<ast::type::Struct>()) {
      out << "_" << id;

      ops.push_back(Operand::Int(id));
      continue;
    }

    // Both scalars, but not the same type so we need to generate a conversion
    // of the value.
    if (value_type->is_scalar() && result_type->is_scalar()) {
      id = GenerateCastOrCopyOrPassthrough(result_type, values[0]);
      out << "_" << id;
      ops.push_back(Operand::Int(id));
      continue;
    }

    // When handling vectors as the values there a few cases to take into
    // consideration:
    //  1. Module scoped vec3<f32>(vec2<f32>(1, 2), 3)  -> OpSpecConstantOp
    //  2. Function scoped vec3<f32>(vec2<f32>(1, 2), 3) ->  OpCompositeExtract
    //  3. Either array<vec3<f32>, 1>(vec3<f32>(1, 2, 3))  -> use the ID.
    //       -> handled above
    //
    // For cases 1 and 2, if the type is different we also may need to insert
    // a type cast.
    if (auto* vec = value_type->As<ast::type::Vector>()) {
      auto* vec_type = vec->type();

      auto value_type_id = GenerateTypeIfNeeded(vec_type);
      if (value_type_id == 0) {
        return 0;
      }

      for (uint32_t i = 0; i < vec->size(); ++i) {
        auto extract = result_op();
        auto extract_id = extract.to_i();

        if (!is_global_init) {
          // A non-global initializer. Case 2.
          if (!push_function_inst(spv::Op::OpCompositeExtract,
                                  {Operand::Int(value_type_id), extract,
                                   Operand::Int(id), Operand::Int(i)})) {
            return false;
          }

          // We no longer have a constant composite, but have to do a
          // composite construction as these calls are inside a function.
          result_is_constant_composite = false;
        } else {
          // A global initializer, must use OpSpecConstantOp. Case 1.
          auto idx_id = GenerateU32Literal(i);
          if (idx_id == 0) {
            return 0;
          }
          push_type(spv::Op::OpSpecConstantOp,
                    {Operand::Int(value_type_id), extract,
                     Operand::Int(SpvOpCompositeExtract), Operand::Int(id),
                     Operand::Int(idx_id)});

          result_is_spec_composite = true;
        }

        out << "_" << extract_id;
        ops.push_back(Operand::Int(extract_id));
      }
    } else {
      error_ = "Unhandled type cast value type";
      return 0;
    }
  }

  auto str = out.str();
  auto val = const_to_id_.find(str);
  if (val != const_to_id_.end()) {
    return val->second;
  }

  auto result = result_op();
  ops.insert(ops.begin(), result);
  ops.insert(ops.begin(), Operand::Int(type_id));

  const_to_id_[str] = result.to_i();

  if (result_is_spec_composite) {
    push_type(spv::Op::OpSpecConstantComposite, ops);
  } else if (result_is_constant_composite) {
    push_type(spv::Op::OpConstantComposite, ops);
  } else {
    if (!push_function_inst(spv::Op::OpCompositeConstruct, ops)) {
      return 0;
    }
  }

  return result.to_i();
}

uint32_t Builder::GenerateCastOrCopyOrPassthrough(ast::type::Type* to_type,
                                                  ast::Expression* from_expr) {
  auto result = result_op();
  auto result_id = result.to_i();

  auto result_type_id = GenerateTypeIfNeeded(to_type);
  if (result_type_id == 0) {
    return 0;
  }

  auto val_id = GenerateExpression(from_expr);
  if (val_id == 0) {
    return 0;
  }
  val_id = GenerateLoadIfNeeded(from_expr->result_type(), val_id);

  auto* from_type = from_expr->result_type()->UnwrapPtrIfNeeded();

  spv::Op op = spv::Op::OpNop;
  if ((from_type->Is<ast::type::I32>() && to_type->Is<ast::type::F32>()) ||
      (from_type->is_signed_integer_vector() && to_type->is_float_vector())) {
    op = spv::Op::OpConvertSToF;
  } else if ((from_type->Is<ast::type::U32>() &&
              to_type->Is<ast::type::F32>()) ||
             (from_type->is_unsigned_integer_vector() &&
              to_type->is_float_vector())) {
    op = spv::Op::OpConvertUToF;
  } else if ((from_type->Is<ast::type::F32>() &&
              to_type->Is<ast::type::I32>()) ||
             (from_type->is_float_vector() &&
              to_type->is_signed_integer_vector())) {
    op = spv::Op::OpConvertFToS;
  } else if ((from_type->Is<ast::type::F32>() &&
              to_type->Is<ast::type::U32>()) ||
             (from_type->is_float_vector() &&
              to_type->is_unsigned_integer_vector())) {
    op = spv::Op::OpConvertFToU;
  } else if ((from_type->Is<ast::type::Bool>() &&
              to_type->Is<ast::type::Bool>()) ||
             (from_type->Is<ast::type::U32>() &&
              to_type->Is<ast::type::U32>()) ||
             (from_type->Is<ast::type::I32>() &&
              to_type->Is<ast::type::I32>()) ||
             (from_type->Is<ast::type::F32>() &&
              to_type->Is<ast::type::F32>()) ||
             (from_type->Is<ast::type::Vector>() && (from_type == to_type))) {
    return val_id;
  } else if ((from_type->Is<ast::type::I32>() &&
              to_type->Is<ast::type::U32>()) ||
             (from_type->Is<ast::type::U32>() &&
              to_type->Is<ast::type::I32>()) ||
             (from_type->is_signed_integer_vector() &&
              to_type->is_unsigned_integer_vector()) ||
             (from_type->is_unsigned_integer_vector() &&
              to_type->is_integer_scalar_or_vector())) {
    op = spv::Op::OpBitcast;
  }
  if (op == spv::Op::OpNop) {
    error_ = "unable to determine conversion type for cast, from: " +
             from_type->type_name() + " to: " + to_type->type_name();
    return 0;
  }

  if (!push_function_inst(
          op, {Operand::Int(result_type_id), result, Operand::Int(val_id)})) {
    return 0;
  }

  return result_id;
}

uint32_t Builder::GenerateLiteralIfNeeded(ast::Variable* var,
                                          ast::Literal* lit) {
  auto type_id = GenerateTypeIfNeeded(lit->type());
  if (type_id == 0) {
    return 0;
  }

  auto name = lit->name();
  bool is_spec_constant = false;
  if (var && var->HasConstantIdDecoration()) {
    name = "__spec" + name;
    is_spec_constant = true;
  }

  auto val = const_to_id_.find(name);
  if (val != const_to_id_.end()) {
    return val->second;
  }

  auto result = result_op();
  auto result_id = result.to_i();

  if (is_spec_constant) {
    push_annot(spv::Op::OpDecorate,
               {Operand::Int(result_id), Operand::Int(SpvDecorationSpecId),
                Operand::Int(var->constant_id())});
  }

  if (auto* l = lit->As<ast::BoolLiteral>()) {
    if (l->IsTrue()) {
      push_type(is_spec_constant ? spv::Op::OpSpecConstantTrue
                                 : spv::Op::OpConstantTrue,
                {Operand::Int(type_id), result});
    } else {
      push_type(is_spec_constant ? spv::Op::OpSpecConstantFalse
                                 : spv::Op::OpConstantFalse,
                {Operand::Int(type_id), result});
    }
  } else if (auto* sl = lit->As<ast::SintLiteral>()) {
    push_type(is_spec_constant ? spv::Op::OpSpecConstant : spv::Op::OpConstant,
              {Operand::Int(type_id), result, Operand::Int(sl->value())});
  } else if (auto* ul = lit->As<ast::UintLiteral>()) {
    push_type(is_spec_constant ? spv::Op::OpSpecConstant : spv::Op::OpConstant,
              {Operand::Int(type_id), result, Operand::Int(ul->value())});
  } else if (auto* fl = lit->As<ast::FloatLiteral>()) {
    push_type(is_spec_constant ? spv::Op::OpSpecConstant : spv::Op::OpConstant,
              {Operand::Int(type_id), result, Operand::Float(fl->value())});
  } else if (lit->Is<ast::NullLiteral>()) {
    push_type(spv::Op::OpConstantNull, {Operand::Int(type_id), result});
  } else {
    error_ = "unknown literal type";
    return 0;
  }

  const_to_id_[name] = result_id;
  return result_id;
}

uint32_t Builder::GenerateShortCircuitBinaryExpression(
    ast::BinaryExpression* expr) {
  auto lhs_id = GenerateExpression(expr->lhs());
  if (lhs_id == 0) {
    return false;
  }
  lhs_id = GenerateLoadIfNeeded(expr->lhs()->result_type(), lhs_id);

  // Get the ID of the basic block where control flow will diverge. It's the
  // last basic block generated for the left-hand-side of the operator.
  auto original_label_id = current_label_id_;

  auto type_id = GenerateTypeIfNeeded(expr->result_type());
  if (type_id == 0) {
    return 0;
  }

  auto merge_block = result_op();
  auto merge_block_id = merge_block.to_i();

  auto block = result_op();
  auto block_id = block.to_i();

  auto true_block_id = block_id;
  auto false_block_id = merge_block_id;

  // For a logical or we want to only check the RHS if the LHS is failed.
  if (expr->IsLogicalOr()) {
    std::swap(true_block_id, false_block_id);
  }

  if (!push_function_inst(spv::Op::OpSelectionMerge,
                          {Operand::Int(merge_block_id),
                           Operand::Int(SpvSelectionControlMaskNone)})) {
    return 0;
  }
  if (!push_function_inst(spv::Op::OpBranchConditional,
                          {Operand::Int(lhs_id), Operand::Int(true_block_id),
                           Operand::Int(false_block_id)})) {
    return 0;
  }

  // Output block to check the RHS
  if (!GenerateLabel(block_id)) {
    return 0;
  }
  auto rhs_id = GenerateExpression(expr->rhs());
  if (rhs_id == 0) {
    return 0;
  }
  rhs_id = GenerateLoadIfNeeded(expr->rhs()->result_type(), rhs_id);

  // Get the block ID of the last basic block generated for the right-hand-side
  // expression. That block will be an immediate predecessor to the merge block.
  auto rhs_block_id = current_label_id_;
  if (!push_function_inst(spv::Op::OpBranch, {Operand::Int(merge_block_id)})) {
    return 0;
  }

  // Output the merge block
  if (!GenerateLabel(merge_block_id)) {
    return 0;
  }

  auto result = result_op();
  auto result_id = result.to_i();

  if (!push_function_inst(spv::Op::OpPhi,
                          {Operand::Int(type_id), result, Operand::Int(lhs_id),
                           Operand::Int(original_label_id),
                           Operand::Int(rhs_id), Operand::Int(rhs_block_id)})) {
    return 0;
  }

  return result_id;
}

uint32_t Builder::GenerateBinaryExpression(ast::BinaryExpression* expr) {
  // There is special logic for short circuiting operators.
  if (expr->IsLogicalAnd() || expr->IsLogicalOr()) {
    return GenerateShortCircuitBinaryExpression(expr);
  }

  auto lhs_id = GenerateExpression(expr->lhs());
  if (lhs_id == 0) {
    return 0;
  }
  lhs_id = GenerateLoadIfNeeded(expr->lhs()->result_type(), lhs_id);

  auto rhs_id = GenerateExpression(expr->rhs());
  if (rhs_id == 0) {
    return 0;
  }
  rhs_id = GenerateLoadIfNeeded(expr->rhs()->result_type(), rhs_id);

  auto result = result_op();
  auto result_id = result.to_i();

  auto type_id = GenerateTypeIfNeeded(expr->result_type());
  if (type_id == 0) {
    return 0;
  }

  // Handle int and float and the vectors of those types. Other types
  // should have been rejected by validation.
  auto* lhs_type = expr->lhs()->result_type()->UnwrapPtrIfNeeded();
  auto* rhs_type = expr->rhs()->result_type()->UnwrapPtrIfNeeded();
  bool lhs_is_float_or_vec = lhs_type->is_float_scalar_or_vector();
  bool lhs_is_unsigned = lhs_type->is_unsigned_scalar_or_vector();

  spv::Op op = spv::Op::OpNop;
  if (expr->IsAnd()) {
    op = spv::Op::OpBitwiseAnd;
  } else if (expr->IsAdd()) {
    op = lhs_is_float_or_vec ? spv::Op::OpFAdd : spv::Op::OpIAdd;
  } else if (expr->IsDivide()) {
    if (lhs_is_float_or_vec) {
      op = spv::Op::OpFDiv;
    } else if (lhs_is_unsigned) {
      op = spv::Op::OpUDiv;
    } else {
      op = spv::Op::OpSDiv;
    }
  } else if (expr->IsEqual()) {
    op = lhs_is_float_or_vec ? spv::Op::OpFOrdEqual : spv::Op::OpIEqual;
  } else if (expr->IsGreaterThan()) {
    if (lhs_is_float_or_vec) {
      op = spv::Op::OpFOrdGreaterThan;
    } else if (lhs_is_unsigned) {
      op = spv::Op::OpUGreaterThan;
    } else {
      op = spv::Op::OpSGreaterThan;
    }
  } else if (expr->IsGreaterThanEqual()) {
    if (lhs_is_float_or_vec) {
      op = spv::Op::OpFOrdGreaterThanEqual;
    } else if (lhs_is_unsigned) {
      op = spv::Op::OpUGreaterThanEqual;
    } else {
      op = spv::Op::OpSGreaterThanEqual;
    }
  } else if (expr->IsLessThan()) {
    if (lhs_is_float_or_vec) {
      op = spv::Op::OpFOrdLessThan;
    } else if (lhs_is_unsigned) {
      op = spv::Op::OpULessThan;
    } else {
      op = spv::Op::OpSLessThan;
    }
  } else if (expr->IsLessThanEqual()) {
    if (lhs_is_float_or_vec) {
      op = spv::Op::OpFOrdLessThanEqual;
    } else if (lhs_is_unsigned) {
      op = spv::Op::OpULessThanEqual;
    } else {
      op = spv::Op::OpSLessThanEqual;
    }
  } else if (expr->IsModulo()) {
    if (lhs_is_float_or_vec) {
      op = spv::Op::OpFMod;
    } else if (lhs_is_unsigned) {
      op = spv::Op::OpUMod;
    } else {
      op = spv::Op::OpSMod;
    }
  } else if (expr->IsMultiply()) {
    if (lhs_type->is_integer_scalar_or_vector()) {
      // If the left hand side is an integer then this _has_ to be OpIMul as
      // there there is no other integer multiplication.
      op = spv::Op::OpIMul;
    } else if (lhs_type->is_float_scalar() && rhs_type->is_float_scalar()) {
      // Float scalars multiply with OpFMul
      op = spv::Op::OpFMul;
    } else if (lhs_type->is_float_vector() && rhs_type->is_float_vector()) {
      // Float vectors must be validated to be the same size and then use OpFMul
      op = spv::Op::OpFMul;
    } else if (lhs_type->is_float_scalar() && rhs_type->is_float_vector()) {
      // Scalar * Vector we need to flip lhs and rhs types
      // because OpVectorTimesScalar expects <vector>, <scalar>
      std::swap(lhs_id, rhs_id);
      op = spv::Op::OpVectorTimesScalar;
    } else if (lhs_type->is_float_vector() && rhs_type->is_float_scalar()) {
      // float vector * scalar
      op = spv::Op::OpVectorTimesScalar;
    } else if (lhs_type->is_float_scalar() && rhs_type->is_float_matrix()) {
      // Scalar * Matrix we need to flip lhs and rhs types because
      // OpMatrixTimesScalar expects <matrix>, <scalar>
      std::swap(lhs_id, rhs_id);
      op = spv::Op::OpMatrixTimesScalar;
    } else if (lhs_type->is_float_matrix() && rhs_type->is_float_scalar()) {
      // float matrix * scalar
      op = spv::Op::OpMatrixTimesScalar;
    } else if (lhs_type->is_float_vector() && rhs_type->is_float_matrix()) {
      // float vector * matrix
      op = spv::Op::OpVectorTimesMatrix;
    } else if (lhs_type->is_float_matrix() && rhs_type->is_float_vector()) {
      // float matrix * vector
      op = spv::Op::OpMatrixTimesVector;
    } else if (lhs_type->is_float_matrix() && rhs_type->is_float_matrix()) {
      // float matrix * matrix
      op = spv::Op::OpMatrixTimesMatrix;
    } else {
      return 0;
    }
  } else if (expr->IsNotEqual()) {
    op = lhs_is_float_or_vec ? spv::Op::OpFOrdNotEqual : spv::Op::OpINotEqual;
  } else if (expr->IsOr()) {
    op = spv::Op::OpBitwiseOr;
  } else if (expr->IsShiftLeft()) {
    op = spv::Op::OpShiftLeftLogical;
  } else if (expr->IsShiftRight() && lhs_type->is_signed_scalar_or_vector()) {
    // A shift right with a signed LHS is an arithmetic shift.
    op = spv::Op::OpShiftRightArithmetic;
  } else if (expr->IsShiftRight()) {
    op = spv::Op::OpShiftRightLogical;
  } else if (expr->IsSubtract()) {
    op = lhs_is_float_or_vec ? spv::Op::OpFSub : spv::Op::OpISub;
  } else if (expr->IsXor()) {
    op = spv::Op::OpBitwiseXor;
  } else {
    error_ = "unknown binary expression";
    return 0;
  }

  if (!push_function_inst(op, {Operand::Int(type_id), result,
                               Operand::Int(lhs_id), Operand::Int(rhs_id)})) {
    return 0;
  }
  return result_id;
}

bool Builder::GenerateBlockStatement(const ast::BlockStatement* stmt) {
  scope_stack_.push_scope();
  for (auto* block_stmt : *stmt) {
    if (!GenerateStatement(block_stmt)) {
      return false;
    }
  }
  scope_stack_.pop_scope();

  return true;
}

uint32_t Builder::GenerateCallExpression(ast::CallExpression* expr) {
  auto* ident = expr->func()->As<ast::IdentifierExpression>();

  if (ident == nullptr) {
    error_ = "invalid function name";
    return 0;
  }

  if (ident->IsIntrinsic()) {
    return GenerateIntrinsic(ident, expr);
  }

  auto type_id = GenerateTypeIfNeeded(expr->func()->result_type());
  if (type_id == 0) {
    return 0;
  }

  auto result = result_op();
  auto result_id = result.to_i();

  OperandList ops = {Operand::Int(type_id), result};

  auto func_id = func_symbol_to_id_[ident->symbol()];
  if (func_id == 0) {
    error_ = "unable to find called function: " +
             mod_->SymbolToName(ident->symbol());
    return 0;
  }
  ops.push_back(Operand::Int(func_id));

  for (auto* param : expr->params()) {
    auto id = GenerateExpression(param);
    if (id == 0) {
      return 0;
    }
    id = GenerateLoadIfNeeded(param->result_type(), id);
    ops.push_back(Operand::Int(id));
  }

  if (!push_function_inst(spv::Op::OpFunctionCall, std::move(ops))) {
    return 0;
  }

  return result_id;
}

uint32_t Builder::GenerateIntrinsic(ast::IdentifierExpression* ident,
                                    ast::CallExpression* call) {
  auto result = result_op();
  auto result_id = result.to_i();

  auto result_type_id = GenerateTypeIfNeeded(call->result_type());
  if (result_type_id == 0) {
    return 0;
  }

  auto intrinsic = ident->intrinsic();

  if (ast::intrinsic::IsFineDerivative(intrinsic) ||
      ast::intrinsic::IsCoarseDerivative(intrinsic)) {
    push_capability(SpvCapabilityDerivativeControl);
  }

  if (ast::intrinsic::IsImageQueryIntrinsic(intrinsic)) {
    push_capability(SpvCapabilityImageQuery);
  }

  if (ast::intrinsic::IsTextureIntrinsic(intrinsic)) {
    if (!GenerateTextureIntrinsic(ident, call, Operand::Int(result_type_id),
                                  result)) {
      return 0;
    }
    return result_id;
  }

  OperandList params = {Operand::Int(result_type_id), result};

  spv::Op op = spv::Op::OpNop;
  if (intrinsic == ast::Intrinsic::kAny) {
    op = spv::Op::OpAny;
  } else if (intrinsic == ast::Intrinsic::kAll) {
    op = spv::Op::OpAll;
  } else if (intrinsic == ast::Intrinsic::kArrayLength) {
    if (call->params().empty()) {
      error_ = "missing param for runtime array length";
      return 0;
    }
    auto* arg = call->params()[0];

    auto* accessor = arg->As<ast::MemberAccessorExpression>();
    if (accessor == nullptr) {
      error_ = "invalid expression for array length";
      return 0;
    }

    auto struct_id = GenerateExpression(accessor->structure());
    if (struct_id == 0) {
      return 0;
    }
    params.push_back(Operand::Int(struct_id));

    auto* type = accessor->structure()->result_type()->UnwrapAll();
    if (!type->Is<ast::type::Struct>()) {
      error_ =
          "invalid type (" + type->type_name() + ") for runtime array length";
      return 0;
    }
    // Runtime array must be the last member in the structure
    params.push_back(Operand::Int(
        uint32_t(type->As<ast::type::Struct>()->impl()->members().size() - 1)));

    if (!push_function_inst(spv::Op::OpArrayLength, params)) {
      return 0;
    }
    return result_id;
  } else if (intrinsic == ast::Intrinsic::kCountOneBits) {
    op = spv::Op::OpBitCount;
  } else if (intrinsic == ast::Intrinsic::kDot) {
    op = spv::Op::OpDot;
  } else if (intrinsic == ast::Intrinsic::kDpdx) {
    op = spv::Op::OpDPdx;
  } else if (intrinsic == ast::Intrinsic::kDpdxCoarse) {
    op = spv::Op::OpDPdxCoarse;
  } else if (intrinsic == ast::Intrinsic::kDpdxFine) {
    op = spv::Op::OpDPdxFine;
  } else if (intrinsic == ast::Intrinsic::kDpdy) {
    op = spv::Op::OpDPdy;
  } else if (intrinsic == ast::Intrinsic::kDpdyCoarse) {
    op = spv::Op::OpDPdyCoarse;
  } else if (intrinsic == ast::Intrinsic::kDpdyFine) {
    op = spv::Op::OpDPdyFine;
  } else if (intrinsic == ast::Intrinsic::kFwidth) {
    op = spv::Op::OpFwidth;
  } else if (intrinsic == ast::Intrinsic::kFwidthCoarse) {
    op = spv::Op::OpFwidthCoarse;
  } else if (intrinsic == ast::Intrinsic::kFwidthFine) {
    op = spv::Op::OpFwidthFine;
  } else if (intrinsic == ast::Intrinsic::kIsInf) {
    op = spv::Op::OpIsInf;
  } else if (intrinsic == ast::Intrinsic::kIsNan) {
    op = spv::Op::OpIsNan;
  } else if (intrinsic == ast::Intrinsic::kOuterProduct) {
    op = spv::Op::OpOuterProduct;
  } else if (intrinsic == ast::Intrinsic::kReverseBits) {
    op = spv::Op::OpBitReverse;
  } else if (intrinsic == ast::Intrinsic::kSelect) {
    op = spv::Op::OpSelect;
  } else {
    GenerateGLSLstd450Import();

    auto set_iter = import_name_to_id_.find(kGLSLstd450);
    if (set_iter == import_name_to_id_.end()) {
      error_ = std::string("unknown import ") + kGLSLstd450;
      return 0;
    }
    auto set_id = set_iter->second;
    auto inst_id =
        intrinsic_to_glsl_method(ident->result_type(), ident->intrinsic());
    if (inst_id == 0) {
      error_ = "unknown method " + mod_->SymbolToName(ident->symbol());
      return 0;
    }

    params.push_back(Operand::Int(set_id));
    params.push_back(Operand::Int(inst_id));

    op = spv::Op::OpExtInst;
  }

  if (op == spv::Op::OpNop) {
    error_ = "unable to determine operator for: " +
             mod_->SymbolToName(ident->symbol());
    return 0;
  }

  for (auto* p : call->params()) {
    auto val_id = GenerateExpression(p);
    if (val_id == 0) {
      return false;
    }
    val_id = GenerateLoadIfNeeded(p->result_type(), val_id);

    params.emplace_back(Operand::Int(val_id));
  }

  if (!push_function_inst(op, params)) {
    return 0;
  }

  return result_id;
}

bool Builder::GenerateTextureIntrinsic(ast::IdentifierExpression* ident,
                                       ast::CallExpression* call,
                                       Operand result_type,
                                       Operand result_id) {
  auto* sig = static_cast<const ast::intrinsic::TextureSignature*>(
      ident->intrinsic_signature());
  assert(sig != nullptr);
  auto& pidx = sig->params.idx;
  auto const kNotUsed = ast::intrinsic::TextureSignature::Parameters::kNotUsed;

  assert(pidx.texture != kNotUsed);
  auto* texture_type = call->params()[pidx.texture]
                           ->result_type()
                           ->UnwrapAll()
                           ->As<ast::type::Texture>();

  auto op = spv::Op::OpNop;

  auto gen_param = [&](size_t idx) {
    auto* p = call->params()[idx];
    auto val_id = GenerateExpression(p);
    if (val_id == 0) {
      return Operand::Int(0);
    }
    val_id = GenerateLoadIfNeeded(p->result_type(), val_id);

    return Operand::Int(val_id);
  };

  // Custom function to call after the texture-intrinsic op has been generated.
  std::function<bool()> post_emission = [] { return true; };

  // Populate the spirv_params with common parameters
  OperandList spirv_params;
  spirv_params.reserve(8);  // Enough to fit most parameter lists

  // Extra image operands, appended to spirv_params.
  struct ImageOperand {
    SpvImageOperandsMask mask;
    Operand operand;
  };
  std::vector<ImageOperand> image_operands;
  image_operands.reserve(4);  // Enough to fit most parameter lists

  auto append_result_type_and_id_to_spirv_params = [&]() {
    spirv_params.emplace_back(std::move(result_type));
    spirv_params.emplace_back(std::move(result_id));
  };

  auto append_coords_to_spirv_params = [&]() -> bool {
    if (pidx.array_index != kNotUsed) {
      // Array index needs to be appended to the coordinates.
      auto* param_coords = call->params()[pidx.coords];
      auto* param_array_index = call->params()[pidx.array_index];

      if (!AppendVector(param_coords, param_array_index,
                        [&](ast::TypeConstructorExpression* packed) {
                          auto param =
                              GenerateTypeConstructorExpression(packed, false);
                          if (param == 0) {
                            return false;
                          }
                          spirv_params.emplace_back(Operand::Int(param));
                          return true;
                        })) {
        return false;
      }
    } else {
      spirv_params.emplace_back(gen_param(pidx.coords));  // coordinates
    }
    return true;
  };

  auto append_image_and_coords_to_spirv_params = [&]() -> bool {
    assert(pidx.sampler != kNotUsed);
    assert(pidx.texture != kNotUsed);
    auto sampler_param = gen_param(pidx.sampler);
    auto texture_param = gen_param(pidx.texture);
    auto sampled_image =
        GenerateSampledImage(texture_type, texture_param, sampler_param);

    // Populate the spirv_params with the common parameters
    spirv_params.emplace_back(Operand::Int(sampled_image));  // sampled image
    return append_coords_to_spirv_params();
  };

  switch (ident->intrinsic()) {
    case ast::Intrinsic::kTextureDimensions: {
      // Number of returned elements from OpImageQuerySize[Lod] may not match
      // those of textureDimensions().
      // This might be due to an extra vector scalar describing the number of
      // array elements or textureDimensions() returning a vec3 for cubes
      // when only width / height is returned by OpImageQuerySize[Lod]
      // (see https://github.com/gpuweb/gpuweb/issues/1345).
      // Handle these mismatches by swizzling the returned vector.
      std::vector<uint32_t> swizzle;
      uint32_t spirv_dims = 0;
      switch (texture_type->dim()) {
        case ast::type::TextureDimension::kNone:
          error_ = "texture dimension is kNone";
          return false;
        case ast::type::TextureDimension::k1d:
        case ast::type::TextureDimension::k2d:
        case ast::type::TextureDimension::k3d:
          break;  // No swizzle needed
        case ast::type::TextureDimension::k1dArray:
          swizzle = {0};   // Strip array index
          spirv_dims = 2;  // [width, array count]
          break;
        case ast::type::TextureDimension::kCube:
          swizzle = {0, 1, 1};  // Duplicate height for depth
          spirv_dims = 2;       // [width, height]
          break;
        case ast::type::TextureDimension::k2dArray:
          swizzle = {0, 1};  // Strip array index
          spirv_dims = 3;    // [width, height, array_count]
          break;
        case ast::type::TextureDimension::kCubeArray:
          swizzle = {0, 1, 1};  // Strip array index, duplicate height for depth
          spirv_dims = 3;       // [width, height, array_count]
          break;
      }

      if (swizzle.empty()) {
        append_result_type_and_id_to_spirv_params();
      } else {
        // Assign post_emission to swizzle the result of the call to
        // OpImageQuerySize[Lod].
        auto* element_type = ElementTypeOf(call->result_type());
        auto spirv_result = result_op();
        auto* spirv_result_type =
            mod_->create<ast::type::Vector>(element_type, spirv_dims);
        if (swizzle.size() > 1) {
          post_emission = [=] {
            OperandList operands{
                result_type,
                result_id,
                spirv_result,
                spirv_result,
            };
            for (auto idx : swizzle) {
              operands.emplace_back(Operand::Int(idx));
            }
            return push_function_inst(spv::Op::OpVectorShuffle, operands);
          };
        } else {
          post_emission = [=] {
            return push_function_inst(spv::Op::OpCompositeExtract,
                                      {result_type, result_id, spirv_result,
                                       Operand::Int(swizzle[0])});
          };
        }
        auto spirv_result_type_id = GenerateTypeIfNeeded(spirv_result_type);
        if (spirv_result_type_id == 0) {
          return false;
        }
        spirv_params.emplace_back(Operand::Int(spirv_result_type_id));
        spirv_params.emplace_back(spirv_result);
      }

      spirv_params.emplace_back(gen_param(pidx.texture));
      if (texture_type->Is<ast::type::MultisampledTexture>() ||
          texture_type->Is<ast::type::StorageTexture>()) {
        op = spv::Op::OpImageQuerySize;
      } else if (pidx.level != kNotUsed) {
        op = spv::Op::OpImageQuerySizeLod;
        spirv_params.emplace_back(gen_param(pidx.level));
      } else {
        ast::SintLiteral i32_0(Source{}, mod_->create<ast::type::I32>(), 0);
        op = spv::Op::OpImageQuerySizeLod;
        spirv_params.emplace_back(
            Operand::Int(GenerateLiteralIfNeeded(nullptr, &i32_0)));
      }
      break;
    }
    case ast::Intrinsic::kTextureLoad: {
      op = texture_type->Is<ast::type::StorageTexture>()
               ? spv::Op::OpImageRead
               : spv::Op::OpImageFetch;
      append_result_type_and_id_to_spirv_params();
      spirv_params.emplace_back(gen_param(pidx.texture));
      if (!append_coords_to_spirv_params()) {
        return false;
      }

      if (pidx.level != kNotUsed) {
        image_operands.emplace_back(
            ImageOperand{SpvImageOperandsLodMask, gen_param(pidx.level)});
      }

      if (pidx.sample_index != kNotUsed) {
        image_operands.emplace_back(ImageOperand{SpvImageOperandsSampleMask,
                                                 gen_param(pidx.sample_index)});
      }

      break;
    }
    case ast::Intrinsic::kTextureStore: {
      op = spv::Op::OpImageWrite;
      spirv_params.emplace_back(gen_param(pidx.texture));
      if (!append_coords_to_spirv_params()) {
        return false;
      }
      spirv_params.emplace_back(gen_param(pidx.value));
      break;
    }
    case ast::Intrinsic::kTextureSample: {
      op = spv::Op::OpImageSampleImplicitLod;
      append_result_type_and_id_to_spirv_params();
      if (!append_image_and_coords_to_spirv_params()) {
        return false;
      }
      break;
    }
    case ast::Intrinsic::kTextureSampleBias: {
      op = spv::Op::OpImageSampleImplicitLod;
      append_result_type_and_id_to_spirv_params();
      if (!append_image_and_coords_to_spirv_params()) {
        return false;
      }
      assert(pidx.bias != kNotUsed);
      image_operands.emplace_back(
          ImageOperand{SpvImageOperandsBiasMask, gen_param(pidx.bias)});
      break;
    }
    case ast::Intrinsic::kTextureSampleLevel: {
      op = spv::Op::OpImageSampleExplicitLod;
      append_result_type_and_id_to_spirv_params();
      if (!append_image_and_coords_to_spirv_params()) {
        return false;
      }
      assert(pidx.level != kNotUsed);
      image_operands.emplace_back(
          ImageOperand{SpvImageOperandsLodMask, gen_param(pidx.level)});
      break;
    }
    case ast::Intrinsic::kTextureSampleGrad: {
      op = spv::Op::OpImageSampleExplicitLod;
      append_result_type_and_id_to_spirv_params();
      if (!append_image_and_coords_to_spirv_params()) {
        return false;
      }
      assert(pidx.ddx != kNotUsed);
      assert(pidx.ddy != kNotUsed);
      image_operands.emplace_back(
          ImageOperand{SpvImageOperandsGradMask, gen_param(pidx.ddx)});
      image_operands.emplace_back(
          ImageOperand{SpvImageOperandsGradMask, gen_param(pidx.ddy)});
      break;
    }
    case ast::Intrinsic::kTextureSampleCompare: {
      op = spv::Op::OpImageSampleDrefExplicitLod;
      append_result_type_and_id_to_spirv_params();
      if (!append_image_and_coords_to_spirv_params()) {
        return false;
      }
      assert(pidx.depth_ref != kNotUsed);
      spirv_params.emplace_back(gen_param(pidx.depth_ref));

      ast::type::F32 f32;
      ast::FloatLiteral float_0(Source{}, &f32, 0.0);
      image_operands.emplace_back(ImageOperand{
          SpvImageOperandsLodMask,
          Operand::Int(GenerateLiteralIfNeeded(nullptr, &float_0))});
      break;
    }
    default:
      break;  // unreachable
  }

  if (pidx.offset != kNotUsed) {
    image_operands.emplace_back(
        ImageOperand{SpvImageOperandsOffsetMask, gen_param(pidx.offset)});
  }

  if (!image_operands.empty()) {
    std::sort(image_operands.begin(), image_operands.end(),
              [](auto& a, auto& b) { return a.mask < b.mask; });
    uint32_t mask = 0;
    for (auto& image_operand : image_operands) {
      mask |= image_operand.mask;
    }
    spirv_params.emplace_back(Operand::Int(mask));
    for (auto& image_operand : image_operands) {
      spirv_params.emplace_back(image_operand.operand);
    }
  }

  if (op == spv::Op::OpNop) {
    error_ = "unable to determine operator for: " +
             mod_->SymbolToName(ident->symbol());
    return false;
  }

  if (!push_function_inst(op, spirv_params)) {
    return false;
  }

  return post_emission();
}

uint32_t Builder::GenerateSampledImage(ast::type::Type* texture_type,
                                       Operand texture_operand,
                                       Operand sampler_operand) {
  uint32_t sampled_image_type_id = 0;
  auto val = texture_type_name_to_sampled_image_type_id_.find(
      texture_type->type_name());
  if (val != texture_type_name_to_sampled_image_type_id_.end()) {
    // The sampled image type is already created.
    sampled_image_type_id = val->second;
  } else {
    // We need to create the sampled image type and cache the result.
    auto sampled_image_type = result_op();
    sampled_image_type_id = sampled_image_type.to_i();
    auto texture_type_id = GenerateTypeIfNeeded(texture_type);
    push_type(spv::Op::OpTypeSampledImage,
              {sampled_image_type, Operand::Int(texture_type_id)});
    texture_type_name_to_sampled_image_type_id_[texture_type->type_name()] =
        sampled_image_type_id;
  }

  auto sampled_image = result_op();
  if (!push_function_inst(spv::Op::OpSampledImage,
                          {Operand::Int(sampled_image_type_id), sampled_image,
                           texture_operand, sampler_operand})) {
    return 0;
  }

  return sampled_image.to_i();
}

uint32_t Builder::GenerateBitcastExpression(ast::BitcastExpression* expr) {
  auto result = result_op();
  auto result_id = result.to_i();

  auto result_type_id = GenerateTypeIfNeeded(expr->result_type());
  if (result_type_id == 0) {
    return 0;
  }

  auto val_id = GenerateExpression(expr->expr());
  if (val_id == 0) {
    return 0;
  }
  val_id = GenerateLoadIfNeeded(expr->expr()->result_type(), val_id);

  // Bitcast does not allow same types, just emit a CopyObject
  auto* to_type = expr->result_type()->UnwrapPtrIfNeeded();
  auto* from_type = expr->expr()->result_type()->UnwrapPtrIfNeeded();
  if (to_type->type_name() == from_type->type_name()) {
    if (!push_function_inst(
            spv::Op::OpCopyObject,
            {Operand::Int(result_type_id), result, Operand::Int(val_id)})) {
      return 0;
    }
    return result_id;
  }

  if (!push_function_inst(spv::Op::OpBitcast, {Operand::Int(result_type_id),
                                               result, Operand::Int(val_id)})) {
    return 0;
  }

  return result_id;
}

bool Builder::GenerateConditionalBlock(
    ast::Expression* cond,
    const ast::BlockStatement* true_body,
    size_t cur_else_idx,
    const ast::ElseStatementList& else_stmts) {
  auto cond_id = GenerateExpression(cond);
  if (cond_id == 0) {
    return false;
  }
  cond_id = GenerateLoadIfNeeded(cond->result_type(), cond_id);

  auto merge_block = result_op();
  auto merge_block_id = merge_block.to_i();

  if (!push_function_inst(spv::Op::OpSelectionMerge,
                          {Operand::Int(merge_block_id),
                           Operand::Int(SpvSelectionControlMaskNone)})) {
    return false;
  }

  auto true_block = result_op();
  auto true_block_id = true_block.to_i();

  // if there are no more else statements we branch on false to the merge
  // block otherwise we branch to the false block
  auto false_block_id =
      cur_else_idx < else_stmts.size() ? next_id() : merge_block_id;

  if (!push_function_inst(spv::Op::OpBranchConditional,
                          {Operand::Int(cond_id), Operand::Int(true_block_id),
                           Operand::Int(false_block_id)})) {
    return false;
  }

  // Output true block
  if (!GenerateLabel(true_block_id)) {
    return false;
  }
  if (!GenerateBlockStatement(true_body)) {
    return false;
  }
  // We only branch if the last element of the body didn't already branch.
  if (!LastIsTerminator(true_body)) {
    if (!push_function_inst(spv::Op::OpBranch,
                            {Operand::Int(merge_block_id)})) {
      return false;
    }
  }

  // Start the false block if needed
  if (false_block_id != merge_block_id) {
    if (!GenerateLabel(false_block_id)) {
      return false;
    }

    auto* else_stmt = else_stmts[cur_else_idx];
    // Handle the else case by just outputting the statements.
    if (!else_stmt->HasCondition()) {
      if (!GenerateBlockStatement(else_stmt->body())) {
        return false;
      }
    } else {
      if (!GenerateConditionalBlock(else_stmt->condition(), else_stmt->body(),
                                    cur_else_idx + 1, else_stmts)) {
        return false;
      }
    }
    if (!LastIsTerminator(else_stmt->body())) {
      if (!push_function_inst(spv::Op::OpBranch,
                              {Operand::Int(merge_block_id)})) {
        return false;
      }
    }
  }

  // Output the merge block
  return GenerateLabel(merge_block_id);
}

bool Builder::GenerateIfStatement(ast::IfStatement* stmt) {
  if (!GenerateConditionalBlock(stmt->condition(), stmt->body(), 0,
                                stmt->else_statements())) {
    return false;
  }
  return true;
}

bool Builder::GenerateSwitchStatement(ast::SwitchStatement* stmt) {
  auto merge_block = result_op();
  auto merge_block_id = merge_block.to_i();

  merge_stack_.push_back(merge_block_id);

  auto cond_id = GenerateExpression(stmt->condition());
  if (cond_id == 0) {
    return false;
  }
  cond_id = GenerateLoadIfNeeded(stmt->condition()->result_type(), cond_id);

  auto default_block = result_op();
  auto default_block_id = default_block.to_i();

  OperandList params = {Operand::Int(cond_id), Operand::Int(default_block_id)};

  std::vector<uint32_t> case_ids;
  for (const auto* item : stmt->body()) {
    if (item->IsDefault()) {
      case_ids.push_back(default_block_id);
      continue;
    }

    auto block = result_op();
    auto block_id = block.to_i();

    case_ids.push_back(block_id);
    for (auto* selector : item->selectors()) {
      if (!selector->Is<ast::SintLiteral>()) {
        error_ = "expected integer literal for switch case label";
        return false;
      }

      params.push_back(Operand::Int(selector->As<ast::SintLiteral>()->value()));
      params.push_back(Operand::Int(block_id));
    }
  }

  if (!push_function_inst(spv::Op::OpSelectionMerge,
                          {Operand::Int(merge_block_id),
                           Operand::Int(SpvSelectionControlMaskNone)})) {
    return false;
  }
  if (!push_function_inst(spv::Op::OpSwitch, params)) {
    return false;
  }

  bool generated_default = false;
  auto& body = stmt->body();
  // We output the case statements in order they were entered in the original
  // source. Each fallthrough goes to the next case entry, so is a forward
  // branch, otherwise the branch is to the merge block which comes after
  // the switch statement.
  for (uint32_t i = 0; i < body.size(); i++) {
    auto* item = body[i];

    if (item->IsDefault()) {
      generated_default = true;
    }

    if (!GenerateLabel(case_ids[i])) {
      return false;
    }
    if (!GenerateBlockStatement(item->body())) {
      return false;
    }

    if (LastIsFallthrough(item->body())) {
      if (i == (body.size() - 1)) {
        error_ = "fallthrough of last case statement is disallowed";
        return false;
      }
      if (!push_function_inst(spv::Op::OpBranch,
                              {Operand::Int(case_ids[i + 1])})) {
        return false;
      }
    } else if (!LastIsTerminator(item->body())) {
      if (!push_function_inst(spv::Op::OpBranch,
                              {Operand::Int(merge_block_id)})) {
        return false;
      }
    }
  }

  if (!generated_default) {
    if (!GenerateLabel(default_block_id)) {
      return false;
    }
    if (!push_function_inst(spv::Op::OpBranch,
                            {Operand::Int(merge_block_id)})) {
      return false;
    }
  }

  merge_stack_.pop_back();

  return GenerateLabel(merge_block_id);
}

bool Builder::GenerateReturnStatement(ast::ReturnStatement* stmt) {
  if (stmt->has_value()) {
    auto val_id = GenerateExpression(stmt->value());
    if (val_id == 0) {
      return false;
    }
    val_id = GenerateLoadIfNeeded(stmt->value()->result_type(), val_id);
    if (!push_function_inst(spv::Op::OpReturnValue, {Operand::Int(val_id)})) {
      return false;
    }
  } else {
    if (!push_function_inst(spv::Op::OpReturn, {})) {
      return false;
    }
  }

  return true;
}

bool Builder::GenerateLoopStatement(ast::LoopStatement* stmt) {
  auto loop_header = result_op();
  auto loop_header_id = loop_header.to_i();
  if (!push_function_inst(spv::Op::OpBranch, {Operand::Int(loop_header_id)})) {
    return false;
  }
  if (!GenerateLabel(loop_header_id)) {
    return false;
  }

  auto merge_block = result_op();
  auto merge_block_id = merge_block.to_i();
  auto continue_block = result_op();
  auto continue_block_id = continue_block.to_i();

  auto body_block = result_op();
  auto body_block_id = body_block.to_i();

  if (!push_function_inst(
          spv::Op::OpLoopMerge,
          {Operand::Int(merge_block_id), Operand::Int(continue_block_id),
           Operand::Int(SpvLoopControlMaskNone)})) {
    return false;
  }

  continue_stack_.push_back(continue_block_id);
  merge_stack_.push_back(merge_block_id);

  if (!push_function_inst(spv::Op::OpBranch, {Operand::Int(body_block_id)})) {
    return false;
  }
  if (!GenerateLabel(body_block_id)) {
    return false;
  }
  if (!GenerateBlockStatement(stmt->body())) {
    return false;
  }

  // We only branch if the last element of the body didn't already branch.
  if (!LastIsTerminator(stmt->body())) {
    if (!push_function_inst(spv::Op::OpBranch,
                            {Operand::Int(continue_block_id)})) {
      return false;
    }
  }

  if (!GenerateLabel(continue_block_id)) {
    return false;
  }
  if (!GenerateBlockStatement(stmt->continuing())) {
    return false;
  }
  if (!push_function_inst(spv::Op::OpBranch, {Operand::Int(loop_header_id)})) {
    return false;
  }

  merge_stack_.pop_back();
  continue_stack_.pop_back();

  return GenerateLabel(merge_block_id);
}

bool Builder::GenerateStatement(ast::Statement* stmt) {
  if (auto* a = stmt->As<ast::AssignmentStatement>()) {
    return GenerateAssignStatement(a);
  }
  if (auto* b = stmt->As<ast::BlockStatement>()) {
    return GenerateBlockStatement(b);
  }
  if (auto* b = stmt->As<ast::BreakStatement>()) {
    return GenerateBreakStatement(b);
  }
  if (auto* c = stmt->As<ast::CallStatement>()) {
    return GenerateCallExpression(c->expr()) != 0;
  }
  if (auto* c = stmt->As<ast::ContinueStatement>()) {
    return GenerateContinueStatement(c);
  }
  if (auto* d = stmt->As<ast::DiscardStatement>()) {
    return GenerateDiscardStatement(d);
  }
  if (stmt->Is<ast::FallthroughStatement>()) {
    // Do nothing here, the fallthrough gets handled by the switch code.
    return true;
  }
  if (auto* i = stmt->As<ast::IfStatement>()) {
    return GenerateIfStatement(i);
  }
  if (auto* l = stmt->As<ast::LoopStatement>()) {
    return GenerateLoopStatement(l);
  }
  if (auto* r = stmt->As<ast::ReturnStatement>()) {
    return GenerateReturnStatement(r);
  }
  if (auto* s = stmt->As<ast::SwitchStatement>()) {
    return GenerateSwitchStatement(s);
  }
  if (auto* v = stmt->As<ast::VariableDeclStatement>()) {
    return GenerateVariableDeclStatement(v);
  }

  error_ = "Unknown statement: " + stmt->str();
  return false;
}

bool Builder::GenerateVariableDeclStatement(ast::VariableDeclStatement* stmt) {
  return GenerateFunctionVariable(stmt->variable());
}

uint32_t Builder::GenerateTypeIfNeeded(ast::type::Type* type) {
  if (type == nullptr) {
    error_ = "attempting to generate type from null type";
    return 0;
  }

  // The alias is a wrapper around the subtype, so emit the subtype
  if (auto* alias = type->As<ast::type::Alias>()) {
    return GenerateTypeIfNeeded(alias->type());
  }

  auto val = type_name_to_id_.find(type->type_name());
  if (val != type_name_to_id_.end()) {
    return val->second;
  }

  auto result = result_op();
  auto id = result.to_i();

  if (auto* ac = type->As<ast::type::AccessControl>()) {
    auto* subtype = ac->type()->UnwrapIfNeeded();
    if (!subtype->Is<ast::type::Struct>()) {
      error_ = "Access control attached to non-struct type.";
      return 0;
    }
    if (!GenerateStructType(subtype->As<ast::type::Struct>(),
                            ac->access_control(), result)) {
      return 0;
    }
  } else if (auto* arr = type->As<ast::type::Array>()) {
    if (!GenerateArrayType(arr, result)) {
      return 0;
    }
  } else if (type->Is<ast::type::Bool>()) {
    push_type(spv::Op::OpTypeBool, {result});
  } else if (type->Is<ast::type::F32>()) {
    push_type(spv::Op::OpTypeFloat, {result, Operand::Int(32)});
  } else if (type->Is<ast::type::I32>()) {
    push_type(spv::Op::OpTypeInt, {result, Operand::Int(32), Operand::Int(1)});
  } else if (auto* mat = type->As<ast::type::Matrix>()) {
    if (!GenerateMatrixType(mat, result)) {
      return 0;
    }
  } else if (auto* ptr = type->As<ast::type::Pointer>()) {
    if (!GeneratePointerType(ptr, result)) {
      return 0;
    }
  } else if (auto* str = type->As<ast::type::Struct>()) {
    if (!GenerateStructType(str, ast::AccessControl::kReadWrite, result)) {
      return 0;
    }
  } else if (type->Is<ast::type::U32>()) {
    push_type(spv::Op::OpTypeInt, {result, Operand::Int(32), Operand::Int(0)});
  } else if (auto* vec = type->As<ast::type::Vector>()) {
    if (!GenerateVectorType(vec, result)) {
      return 0;
    }
  } else if (type->Is<ast::type::Void>()) {
    push_type(spv::Op::OpTypeVoid, {result});
  } else if (auto* tex = type->As<ast::type::Texture>()) {
    if (!GenerateTextureType(tex, result)) {
      return 0;
    }
  } else if (type->Is<ast::type::Sampler>()) {
    push_type(spv::Op::OpTypeSampler, {result});

    // Register both of the sampler type names. In SPIR-V they're the same
    // sampler type, so we need to match that when we do the dedup check.
    type_name_to_id_["__sampler_sampler"] = id;
    type_name_to_id_["__sampler_comparison"] = id;

  } else {
    error_ = "unable to convert type: " + type->type_name();
    return 0;
  }

  type_name_to_id_[type->type_name()] = id;
  return id;
}

// TODO(tommek): Cover multisampled textures here when they're included in AST
bool Builder::GenerateTextureType(ast::type::Texture* texture,
                                  const Operand& result) {
  uint32_t array_literal = 0u;
  const auto dim = texture->dim();
  if (dim == ast::type::TextureDimension::k1dArray ||
      dim == ast::type::TextureDimension::k2dArray ||
      dim == ast::type::TextureDimension::kCubeArray) {
    array_literal = 1u;
  }

  uint32_t dim_literal = SpvDim2D;
  if (dim == ast::type::TextureDimension::k1dArray ||
      dim == ast::type::TextureDimension::k1d) {
    dim_literal = SpvDim1D;
    if (texture->Is<ast::type::SampledTexture>()) {
      push_capability(SpvCapabilitySampled1D);
    } else {
      assert(texture->Is<ast::type::StorageTexture>());
      push_capability(SpvCapabilityImage1D);
    }
  }
  if (dim == ast::type::TextureDimension::k3d) {
    dim_literal = SpvDim3D;
  }
  if (dim == ast::type::TextureDimension::kCube ||
      dim == ast::type::TextureDimension::kCubeArray) {
    dim_literal = SpvDimCube;
  }

  uint32_t ms_literal = 0u;
  if (texture->Is<ast::type::MultisampledTexture>()) {
    ms_literal = 1u;
  }

  uint32_t depth_literal = 0u;
  if (texture->Is<ast::type::DepthTexture>()) {
    depth_literal = 1u;
  }

  uint32_t sampled_literal = 2u;
  if (texture->Is<ast::type::MultisampledTexture>() ||
      texture->Is<ast::type::SampledTexture>() ||
      texture->Is<ast::type::DepthTexture>()) {
    sampled_literal = 1u;
  }

  if (dim == ast::type::TextureDimension::kCubeArray) {
    if (texture->Is<ast::type::SampledTexture>() ||
        texture->Is<ast::type::DepthTexture>()) {
      push_capability(SpvCapabilitySampledCubeArray);
    }
  }

  uint32_t type_id = 0u;
  if (texture->Is<ast::type::DepthTexture>()) {
    ast::type::F32 f32;
    type_id = GenerateTypeIfNeeded(&f32);
  } else if (auto* s = texture->As<ast::type::SampledTexture>()) {
    type_id = GenerateTypeIfNeeded(s->type());
  } else if (auto* ms = texture->As<ast::type::MultisampledTexture>()) {
    type_id = GenerateTypeIfNeeded(ms->type());
  } else if (auto* st = texture->As<ast::type::StorageTexture>()) {
    type_id = GenerateTypeIfNeeded(st->type());
  }
  if (type_id == 0u) {
    return false;
  }

  uint32_t format_literal = SpvImageFormat_::SpvImageFormatUnknown;
  if (auto* t = texture->As<ast::type::StorageTexture>()) {
    format_literal = convert_image_format_to_spv(t->image_format());
  }

  push_type(spv::Op::OpTypeImage,
            {result, Operand::Int(type_id), Operand::Int(dim_literal),
             Operand::Int(depth_literal), Operand::Int(array_literal),
             Operand::Int(ms_literal), Operand::Int(sampled_literal),
             Operand::Int(format_literal)});

  return true;
}

bool Builder::GenerateArrayType(ast::type::Array* ary, const Operand& result) {
  auto elem_type = GenerateTypeIfNeeded(ary->type());
  if (elem_type == 0) {
    return false;
  }

  auto result_id = result.to_i();
  if (ary->IsRuntimeArray()) {
    push_type(spv::Op::OpTypeRuntimeArray, {result, Operand::Int(elem_type)});
  } else {
    auto len_id = GenerateU32Literal(ary->size());
    if (len_id == 0) {
      return false;
    }

    push_type(spv::Op::OpTypeArray,
              {result, Operand::Int(elem_type), Operand::Int(len_id)});
  }

  if (ary->has_array_stride()) {
    push_annot(spv::Op::OpDecorate,
               {Operand::Int(result_id), Operand::Int(SpvDecorationArrayStride),
                Operand::Int(ary->array_stride())});
  }
  return true;
}

bool Builder::GenerateMatrixType(ast::type::Matrix* mat,
                                 const Operand& result) {
  ast::type::Vector col_type(mat->type(), mat->rows());
  auto col_type_id = GenerateTypeIfNeeded(&col_type);
  if (has_error()) {
    return false;
  }

  push_type(spv::Op::OpTypeMatrix,
            {result, Operand::Int(col_type_id), Operand::Int(mat->columns())});
  return true;
}

bool Builder::GeneratePointerType(ast::type::Pointer* ptr,
                                  const Operand& result) {
  auto pointee_id = GenerateTypeIfNeeded(ptr->type());
  if (pointee_id == 0) {
    return false;
  }

  auto stg_class = ConvertStorageClass(ptr->storage_class());
  if (stg_class == SpvStorageClassMax) {
    error_ = "invalid storage class for pointer";
    return false;
  }

  push_type(spv::Op::OpTypePointer,
            {result, Operand::Int(stg_class), Operand::Int(pointee_id)});

  return true;
}

bool Builder::GenerateStructType(ast::type::Struct* struct_type,
                                 ast::AccessControl access_control,
                                 const Operand& result) {
  auto struct_id = result.to_i();
  auto* impl = struct_type->impl();

  if (struct_type->symbol().IsValid()) {
    push_debug(spv::Op::OpName,
               {Operand::Int(struct_id),
                Operand::String(mod_->SymbolToName(struct_type->symbol()))});
  }

  OperandList ops;
  ops.push_back(result);

  if (impl->IsBlockDecorated()) {
    push_annot(spv::Op::OpDecorate,
               {Operand::Int(struct_id), Operand::Int(SpvDecorationBlock)});
  }

  auto& members = impl->members();
  for (uint32_t i = 0; i < members.size(); ++i) {
    auto mem_id = GenerateStructMember(struct_id, i, members[i]);
    if (mem_id == 0) {
      return false;
    }

    // We're attaching the access control to the members of the struct instead
    // of to the variable. The reason we do this is that WGSL models the
    // access as part of the type. If we attach to the variable, it's no
    // longer part of the type in the SPIR-V backend, but part of the
    // variable. This differs from the modeling and other backends. Attaching
    // to the struct members means the access control stays part of the type
    // where it logically makes the most sense.
    if (access_control == ast::AccessControl::kReadOnly) {
      push_annot(spv::Op::OpMemberDecorate,
                 {Operand::Int(struct_id), Operand::Int(i),
                  Operand::Int(SpvDecorationNonWritable)});
    }

    ops.push_back(Operand::Int(mem_id));
  }

  push_type(spv::Op::OpTypeStruct, std::move(ops));
  return true;
}

uint32_t Builder::GenerateStructMember(uint32_t struct_id,
                                       uint32_t idx,
                                       ast::StructMember* member) {
  push_debug(spv::Op::OpMemberName,
             {Operand::Int(struct_id), Operand::Int(idx),
              Operand::String(mod_->SymbolToName(member->symbol()))});

  bool has_layout = false;
  for (auto* deco : member->decorations()) {
    if (auto* offset = deco->As<ast::StructMemberOffsetDecoration>()) {
      push_annot(
          spv::Op::OpMemberDecorate,
          {Operand::Int(struct_id), Operand::Int(idx),
           Operand::Int(SpvDecorationOffset), Operand::Int(offset->offset())});
      has_layout = true;
    } else {
      error_ = "unknown struct member decoration";
      return 0;
    }
  }

  if (has_layout) {
    // Infer and emit matrix layout.
    auto* matrix_type = GetNestedMatrixType(member->type());
    if (matrix_type) {
      push_annot(spv::Op::OpMemberDecorate,
                 {Operand::Int(struct_id), Operand::Int(idx),
                  Operand::Int(SpvDecorationColMajor)});
      if (!matrix_type->type()->Is<ast::type::F32>()) {
        error_ = "matrix scalar element type must be f32";
        return 0;
      }
      const auto scalar_elem_size = 4;
      const auto effective_row_count = (matrix_type->rows() == 2) ? 2 : 4;
      push_annot(spv::Op::OpMemberDecorate,
                 {Operand::Int(struct_id), Operand::Int(idx),
                  Operand::Int(SpvDecorationMatrixStride),
                  Operand::Int(effective_row_count * scalar_elem_size)});
    }
  }

  return GenerateTypeIfNeeded(member->type());
}

bool Builder::GenerateVectorType(ast::type::Vector* vec,
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
    case ast::StorageClass::kPrivate:
      return SpvStorageClassPrivate;
    case ast::StorageClass::kFunction:
      return SpvStorageClassFunction;
    case ast::StorageClass::kNone:
      break;
  }
  return SpvStorageClassMax;
}

SpvBuiltIn Builder::ConvertBuiltin(ast::Builtin builtin) const {
  switch (builtin) {
    case ast::Builtin::kPosition:
      return SpvBuiltInPosition;
    case ast::Builtin::kVertexIdx:
      return SpvBuiltInVertexIndex;
    case ast::Builtin::kInstanceIdx:
      return SpvBuiltInInstanceIndex;
    case ast::Builtin::kFrontFacing:
      return SpvBuiltInFrontFacing;
    case ast::Builtin::kFragCoord:
      return SpvBuiltInFragCoord;
    case ast::Builtin::kFragDepth:
      return SpvBuiltInFragDepth;
    case ast::Builtin::kLocalInvocationId:
      return SpvBuiltInLocalInvocationId;
    case ast::Builtin::kLocalInvocationIdx:
      return SpvBuiltInLocalInvocationIndex;
    case ast::Builtin::kGlobalInvocationId:
      return SpvBuiltInGlobalInvocationId;
    case ast::Builtin::kPointSize:
      return SpvBuiltInPointSize;
    case ast::Builtin::kNone:
      break;
  }
  return SpvBuiltInMax;
}

SpvImageFormat Builder::convert_image_format_to_spv(
    const ast::type::ImageFormat format) {
  switch (format) {
    case ast::type::ImageFormat::kR8Unorm:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatR8;
    case ast::type::ImageFormat::kR8Snorm:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatR8Snorm;
    case ast::type::ImageFormat::kR8Uint:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatR8ui;
    case ast::type::ImageFormat::kR8Sint:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatR8i;
    case ast::type::ImageFormat::kR16Uint:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatR16ui;
    case ast::type::ImageFormat::kR16Sint:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatR16i;
    case ast::type::ImageFormat::kR16Float:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatR16f;
    case ast::type::ImageFormat::kRg8Unorm:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatRg8;
    case ast::type::ImageFormat::kRg8Snorm:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatRg8Snorm;
    case ast::type::ImageFormat::kRg8Uint:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatRg8ui;
    case ast::type::ImageFormat::kRg8Sint:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatRg8i;
    case ast::type::ImageFormat::kR32Uint:
      return SpvImageFormatR32ui;
    case ast::type::ImageFormat::kR32Sint:
      return SpvImageFormatR32i;
    case ast::type::ImageFormat::kR32Float:
      return SpvImageFormatR32f;
    case ast::type::ImageFormat::kRg16Uint:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatRg16ui;
    case ast::type::ImageFormat::kRg16Sint:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatRg16i;
    case ast::type::ImageFormat::kRg16Float:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatRg16f;
    case ast::type::ImageFormat::kRgba8Unorm:
      return SpvImageFormatRgba8;
    case ast::type::ImageFormat::kRgba8UnormSrgb:
      return SpvImageFormatUnknown;
    case ast::type::ImageFormat::kRgba8Snorm:
      return SpvImageFormatRgba8Snorm;
    case ast::type::ImageFormat::kRgba8Uint:
      return SpvImageFormatRgba8ui;
    case ast::type::ImageFormat::kRgba8Sint:
      return SpvImageFormatRgba8i;
    case ast::type::ImageFormat::kBgra8Unorm:
      return SpvImageFormatUnknown;
    case ast::type::ImageFormat::kBgra8UnormSrgb:
      return SpvImageFormatUnknown;
    case ast::type::ImageFormat::kRgb10A2Unorm:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatRgb10A2;
    case ast::type::ImageFormat::kRg11B10Float:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatR11fG11fB10f;
    case ast::type::ImageFormat::kRg32Uint:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatRg32ui;
    case ast::type::ImageFormat::kRg32Sint:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatRg32i;
    case ast::type::ImageFormat::kRg32Float:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatRg32f;
    case ast::type::ImageFormat::kRgba16Uint:
      return SpvImageFormatRgba16ui;
    case ast::type::ImageFormat::kRgba16Sint:
      return SpvImageFormatRgba16i;
    case ast::type::ImageFormat::kRgba16Float:
      return SpvImageFormatRgba16f;
    case ast::type::ImageFormat::kRgba32Uint:
      return SpvImageFormatRgba32ui;
    case ast::type::ImageFormat::kRgba32Sint:
      return SpvImageFormatRgba32i;
    case ast::type::ImageFormat::kRgba32Float:
      return SpvImageFormatRgba32f;
    case ast::type::ImageFormat::kNone:
      return SpvImageFormatUnknown;
  }
  return SpvImageFormatUnknown;
}

bool Builder::push_function_inst(spv::Op op, const OperandList& operands) {
  if (functions_.empty()) {
    std::ostringstream ss;
    ss << "Internal error: trying to add SPIR-V instruction " << int(op)
       << " outside a function";
    error_ = ss.str();
    return false;
  }
  functions_.back().push_inst(op, operands);
  return true;
}

}  // namespace spirv
}  // namespace writer
}  // namespace tint
