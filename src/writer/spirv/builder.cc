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
#include <limits>
#include <utility>

#include "spirv/unified1/GLSL.std.450.h"
#include "src/ast/call_statement.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/override_decoration.h"
#include "src/sem/array.h"
#include "src/sem/call.h"
#include "src/sem/depth_texture_type.h"
#include "src/sem/function.h"
#include "src/sem/intrinsic.h"
#include "src/sem/member_accessor_expression.h"
#include "src/sem/multisampled_texture_type.h"
#include "src/sem/reference_type.h"
#include "src/sem/sampled_texture_type.h"
#include "src/sem/struct.h"
#include "src/sem/variable.h"
#include "src/utils/get_or_create.h"
#include "src/writer/append_vector.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using IntrinsicType = sem::IntrinsicType;

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

/// Returns the matrix type that is `type` or that is wrapped by
/// one or more levels of an arrays inside of `type`.
/// @param type the given type, which must not be null
/// @returns the nested matrix type, or nullptr if none
const sem::Matrix* GetNestedMatrixType(const sem::Type* type) {
  while (auto* arr = type->As<sem::Array>()) {
    type = arr->ElemType();
  }
  return type->As<sem::Matrix>();
}

uint32_t intrinsic_to_glsl_method(const sem::Intrinsic* intrinsic) {
  switch (intrinsic->Type()) {
    case IntrinsicType::kAbs:
      if (intrinsic->ReturnType()->is_float_scalar_or_vector()) {
        return GLSLstd450FAbs;
      } else {
        return GLSLstd450SAbs;
      }
    case IntrinsicType::kAcos:
      return GLSLstd450Acos;
    case IntrinsicType::kAsin:
      return GLSLstd450Asin;
    case IntrinsicType::kAtan:
      return GLSLstd450Atan;
    case IntrinsicType::kAtan2:
      return GLSLstd450Atan2;
    case IntrinsicType::kCeil:
      return GLSLstd450Ceil;
    case IntrinsicType::kClamp:
      if (intrinsic->ReturnType()->is_float_scalar_or_vector()) {
        return GLSLstd450NClamp;
      } else if (intrinsic->ReturnType()->is_unsigned_scalar_or_vector()) {
        return GLSLstd450UClamp;
      } else {
        return GLSLstd450SClamp;
      }
    case IntrinsicType::kCos:
      return GLSLstd450Cos;
    case IntrinsicType::kCosh:
      return GLSLstd450Cosh;
    case IntrinsicType::kCross:
      return GLSLstd450Cross;
    case IntrinsicType::kDeterminant:
      return GLSLstd450Determinant;
    case IntrinsicType::kDistance:
      return GLSLstd450Distance;
    case IntrinsicType::kExp:
      return GLSLstd450Exp;
    case IntrinsicType::kExp2:
      return GLSLstd450Exp2;
    case IntrinsicType::kFaceForward:
      return GLSLstd450FaceForward;
    case IntrinsicType::kFloor:
      return GLSLstd450Floor;
    case IntrinsicType::kFma:
      return GLSLstd450Fma;
    case IntrinsicType::kFract:
      return GLSLstd450Fract;
    case IntrinsicType::kFrexp:
      return GLSLstd450Frexp;
    case IntrinsicType::kInverseSqrt:
      return GLSLstd450InverseSqrt;
    case IntrinsicType::kLdexp:
      return GLSLstd450Ldexp;
    case IntrinsicType::kLength:
      return GLSLstd450Length;
    case IntrinsicType::kLog:
      return GLSLstd450Log;
    case IntrinsicType::kLog2:
      return GLSLstd450Log2;
    case IntrinsicType::kMax:
      if (intrinsic->ReturnType()->is_float_scalar_or_vector()) {
        return GLSLstd450NMax;
      } else if (intrinsic->ReturnType()->is_unsigned_scalar_or_vector()) {
        return GLSLstd450UMax;
      } else {
        return GLSLstd450SMax;
      }
    case IntrinsicType::kMin:
      if (intrinsic->ReturnType()->is_float_scalar_or_vector()) {
        return GLSLstd450NMin;
      } else if (intrinsic->ReturnType()->is_unsigned_scalar_or_vector()) {
        return GLSLstd450UMin;
      } else {
        return GLSLstd450SMin;
      }
    case IntrinsicType::kMix:
      return GLSLstd450FMix;
    case IntrinsicType::kModf:
      return GLSLstd450Modf;
    case IntrinsicType::kNormalize:
      return GLSLstd450Normalize;
    case IntrinsicType::kPack4x8Snorm:
      return GLSLstd450PackSnorm4x8;
    case IntrinsicType::kPack4x8Unorm:
      return GLSLstd450PackUnorm4x8;
    case IntrinsicType::kPack2x16Snorm:
      return GLSLstd450PackSnorm2x16;
    case IntrinsicType::kPack2x16Unorm:
      return GLSLstd450PackUnorm2x16;
    case IntrinsicType::kPack2x16Float:
      return GLSLstd450PackHalf2x16;
    case IntrinsicType::kPow:
      return GLSLstd450Pow;
    case IntrinsicType::kReflect:
      return GLSLstd450Reflect;
    case IntrinsicType::kRound:
      return GLSLstd450RoundEven;
    case IntrinsicType::kSign:
      return GLSLstd450FSign;
    case IntrinsicType::kSin:
      return GLSLstd450Sin;
    case IntrinsicType::kSinh:
      return GLSLstd450Sinh;
    case IntrinsicType::kSmoothStep:
      return GLSLstd450SmoothStep;
    case IntrinsicType::kSqrt:
      return GLSLstd450Sqrt;
    case IntrinsicType::kStep:
      return GLSLstd450Step;
    case IntrinsicType::kTan:
      return GLSLstd450Tan;
    case IntrinsicType::kTanh:
      return GLSLstd450Tanh;
    case IntrinsicType::kTrunc:
      return GLSLstd450Trunc;
    case IntrinsicType::kUnpack4x8Snorm:
      return GLSLstd450UnpackSnorm4x8;
    case IntrinsicType::kUnpack4x8Unorm:
      return GLSLstd450UnpackUnorm4x8;
    case IntrinsicType::kUnpack2x16Snorm:
      return GLSLstd450UnpackSnorm2x16;
    case IntrinsicType::kUnpack2x16Unorm:
      return GLSLstd450UnpackUnorm2x16;
    case IntrinsicType::kUnpack2x16Float:
      return GLSLstd450UnpackHalf2x16;
    default:
      break;
  }
  return 0;
}

/// @return the vector element type if ty is a vector, otherwise return ty.
const sem::Type* ElementTypeOf(const sem::Type* ty) {
  if (auto* v = ty->As<sem::Vector>()) {
    return v->type();
  }
  return ty;
}

}  // namespace

Builder::AccessorInfo::AccessorInfo() : source_id(0), source_type(nullptr) {}

Builder::AccessorInfo::~AccessorInfo() {}

Builder::Builder(const Program* program)
    : builder_(ProgramBuilder::Wrap(program)), scope_stack_({}) {}

Builder::~Builder() = default;

bool Builder::Build() {
  push_capability(SpvCapabilityShader);

  push_memory_model(spv::Op::OpMemoryModel,
                    {Operand::Int(SpvAddressingModelLogical),
                     Operand::Int(SpvMemoryModelGLSL450)});

  for (auto* var : builder_.AST().GlobalVariables()) {
    if (!GenerateGlobalVariable(var)) {
      return false;
    }
  }

  for (auto* func : builder_.AST().Functions()) {
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

bool Builder::GenerateAssignStatement(ast::AssignmentStatement* assign) {
  auto lhs_id = GenerateExpression(assign->lhs());
  if (lhs_id == 0) {
    return false;
  }
  auto rhs_id = GenerateExpression(assign->rhs());
  if (rhs_id == 0) {
    return false;
  }

  // If the thing we're assigning is a reference then we must load it first.
  auto* type = TypeOf(assign->rhs());
  rhs_id = GenerateLoadIfNeeded(type, rhs_id);

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

  OperandList operands = {
      Operand::Int(stage), Operand::Int(id),
      Operand::String(builder_.Symbols().NameFor(func->symbol()))};

  auto* func_sem = builder_.Sem().Get(func);
  for (const auto* var : func_sem->ReferencedModuleVariables()) {
    // For SPIR-V 1.3 we only output Input/output variables. If we update to
    // SPIR-V 1.4 or later this should be all variables.
    if (var->StorageClass() != ast::StorageClass::kInput &&
        var->StorageClass() != ast::StorageClass::kOutput) {
      continue;
    }

    uint32_t var_id;
    if (!scope_stack_.get(var->Declaration()->symbol(), &var_id)) {
      error_ = "unable to find ID for global variable: " +
               builder_.Symbols().NameFor(var->Declaration()->symbol());
      return false;
    }

    operands.push_back(Operand::Int(var_id));
  }
  push_entry_point(spv::Op::OpEntryPoint, operands);

  return true;
}

bool Builder::GenerateExecutionModes(ast::Function* func, uint32_t id) {
  auto* func_sem = builder_.Sem().Get(func);

  // WGSL fragment shader origin is upper left
  if (func->pipeline_stage() == ast::PipelineStage::kFragment) {
    push_execution_mode(
        spv::Op::OpExecutionMode,
        {Operand::Int(id), Operand::Int(SpvExecutionModeOriginUpperLeft)});
  } else if (func->pipeline_stage() == ast::PipelineStage::kCompute) {
    auto& wgsize = func_sem->workgroup_size();

    // Check if the workgroup_size uses pipeline-overridable constants.
    if (wgsize[0].overridable_const || wgsize[1].overridable_const ||
        wgsize[2].overridable_const) {
      if (has_overridable_workgroup_size_) {
        // Only one stage can have a pipeline-overridable workgroup size.
        // TODO(crbug.com/tint/810): Use LocalSizeId to handle this scenario.
        TINT_ICE(builder_.Diagnostics())
            << "multiple stages using pipeline-overridable workgroup sizes";
      }
      has_overridable_workgroup_size_ = true;

      sem::U32 u32;
      sem::Vector vec3_u32(&u32, 3);
      uint32_t vec3_u32_type_id = GenerateTypeIfNeeded(&vec3_u32);
      if (vec3_u32_type_id == 0) {
        return 0;
      }

      OperandList wgsize_ops;
      auto wgsize_result = result_op();
      wgsize_ops.push_back(Operand::Int(vec3_u32_type_id));
      wgsize_ops.push_back(wgsize_result);

      // Generate OpConstant instructions for each dimension.
      for (int i = 0; i < 3; i++) {
        auto constant = ScalarConstant::U32(wgsize[i].value);
        if (wgsize[i].overridable_const) {
          // Make the constant specializable.
          auto* sem_const = builder_.Sem().Get(wgsize[i].overridable_const);
          if (!sem_const->IsPipelineConstant()) {
            TINT_ICE(builder_.Diagnostics())
                << "expected a pipeline-overridable constant";
          }
          constant.is_spec_op = true;
          constant.constant_id = sem_const->ConstantId();
        }

        auto result = GenerateConstantIfNeeded(constant);
        wgsize_ops.push_back(Operand::Int(result));
      }

      // Generate the WorkgroupSize builtin.
      push_type(spv::Op::OpSpecConstantComposite, wgsize_ops);
      push_annot(spv::Op::OpDecorate,
                 {wgsize_result, Operand::Int(SpvDecorationBuiltIn),
                  Operand::Int(SpvBuiltInWorkgroupSize)});
    } else {
      // Not overridable, so just use OpExecutionMode LocalSize.
      uint32_t x = wgsize[0].value;
      uint32_t y = wgsize[1].value;
      uint32_t z = wgsize[2].value;
      push_execution_mode(
          spv::Op::OpExecutionMode,
          {Operand::Int(id), Operand::Int(SpvExecutionModeLocalSize),
           Operand::Int(x), Operand::Int(y), Operand::Int(z)});
    }
  }

  for (auto builtin : func_sem->ReferencedBuiltinVariables()) {
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

  error_ = "unknown expression type: " + builder_.str(expr);
  return 0;
}

bool Builder::GenerateFunction(ast::Function* func_ast) {
  auto* func = builder_.Sem().Get(func_ast);

  uint32_t func_type_id = GenerateFunctionTypeIfNeeded(func);
  if (func_type_id == 0) {
    return false;
  }

  auto func_op = result_op();
  auto func_id = func_op.to_i();

  push_debug(spv::Op::OpName,
             {Operand::Int(func_id),
              Operand::String(builder_.Symbols().NameFor(func_ast->symbol()))});

  auto ret_id = GenerateTypeIfNeeded(func->ReturnType());
  if (ret_id == 0) {
    return false;
  }

  scope_stack_.push_scope();

  auto definition_inst = Instruction{
      spv::Op::OpFunction,
      {Operand::Int(ret_id), func_op, Operand::Int(SpvFunctionControlMaskNone),
       Operand::Int(func_type_id)}};

  InstructionList params;
  for (auto* param : func->Parameters()) {
    auto param_op = result_op();
    auto param_id = param_op.to_i();

    auto param_type_id = GenerateTypeIfNeeded(param->Type());
    if (param_type_id == 0) {
      return false;
    }

    push_debug(spv::Op::OpName, {Operand::Int(param_id),
                                 Operand::String(builder_.Symbols().NameFor(
                                     param->Declaration()->symbol()))});
    params.push_back(Instruction{spv::Op::OpFunctionParameter,
                                 {Operand::Int(param_type_id), param_op}});

    scope_stack_.set(param->Declaration()->symbol(), param_id);
  }

  push_function(Function{definition_inst, result_op(), std::move(params)});

  for (auto* stmt : *func_ast->body()) {
    if (!GenerateStatement(stmt)) {
      return false;
    }
  }

  if (func_ast->IsEntryPoint()) {
    if (!GenerateEntryPoint(func_ast, func_id)) {
      return false;
    }
    if (!GenerateExecutionModes(func_ast, func_id)) {
      return false;
    }
  }

  scope_stack_.pop_scope();

  func_symbol_to_id_[func_ast->symbol()] = func_id;

  return true;
}

uint32_t Builder::GenerateFunctionTypeIfNeeded(const sem::Function* func) {
  auto val = type_name_to_id_.find(func->Declaration()->type_name());
  if (val != type_name_to_id_.end()) {
    return val->second;
  }

  auto func_op = result_op();
  auto func_type_id = func_op.to_i();

  auto ret_id = GenerateTypeIfNeeded(func->ReturnType());
  if (ret_id == 0) {
    return 0;
  }

  OperandList ops = {func_op, Operand::Int(ret_id)};
  for (auto* param : func->Parameters()) {
    auto param_type_id = GenerateTypeIfNeeded(param->Type());
    if (param_type_id == 0) {
      return 0;
    }
    ops.push_back(Operand::Int(param_type_id));
  }

  push_type(spv::Op::OpTypeFunction, std::move(ops));

  type_name_to_id_[func->Declaration()->type_name()] = func_type_id;
  return func_type_id;
}

bool Builder::GenerateFunctionVariable(ast::Variable* var) {
  uint32_t init_id = 0;
  if (var->has_constructor()) {
    init_id = GenerateExpression(var->constructor());
    if (init_id == 0) {
      return false;
    }
    auto* type = TypeOf(var->constructor());
    if (type->Is<sem::Reference>()) {
      init_id = GenerateLoadIfNeeded(type, init_id);
    }
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
  auto* type = builder_.Sem().Get(var)->Type();
  auto type_id = GenerateTypeIfNeeded(type);
  if (type_id == 0) {
    return false;
  }

  push_debug(spv::Op::OpName,
             {Operand::Int(var_id),
              Operand::String(builder_.Symbols().NameFor(var->symbol()))});

  // TODO(dsinclair) We could detect if the constructor is fully const and emit
  // an initializer value for the variable instead of doing the OpLoad.
  auto null_id = GenerateConstantNullIfNeeded(type->UnwrapRef());
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
  auto* sem = builder_.Sem().Get(var);
  auto* type = sem->Type()->UnwrapRef();

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
      // Constants must have an initializer unless they have an override
      // decoration.
      if (!ast::HasDecoration<ast::OverrideDecoration>(var->decorations())) {
        error_ = "missing constructor for constant";
        return false;
      }

      // SPIR-V requires specialization constants to have initializers.
      if (type->Is<sem::F32>()) {
        ast::FloatLiteral l(ProgramID(), Source{}, 0.0f);
        init_id = GenerateLiteralIfNeeded(var, &l);
      } else if (type->Is<sem::U32>()) {
        ast::UintLiteral l(ProgramID(), Source{}, 0);
        init_id = GenerateLiteralIfNeeded(var, &l);
      } else if (type->Is<sem::I32>()) {
        ast::SintLiteral l(ProgramID(), Source{}, 0);
        init_id = GenerateLiteralIfNeeded(var, &l);
      } else if (type->Is<sem::Bool>()) {
        ast::BoolLiteral l(ProgramID(), Source{}, false);
        init_id = GenerateLiteralIfNeeded(var, &l);
      } else {
        error_ = "invalid type for pipeline constant ID, must be scalar";
        return false;
      }
      if (init_id == 0) {
        return 0;
      }
    }
    push_debug(spv::Op::OpName,
               {Operand::Int(init_id),
                Operand::String(builder_.Symbols().NameFor(var->symbol()))});

    scope_stack_.set_global(var->symbol(), init_id);
    spirv_id_to_variable_[init_id] = var;
    return true;
  }

  auto result = result_op();
  auto var_id = result.to_i();

  auto sc = sem->StorageClass() == ast::StorageClass::kNone
                ? ast::StorageClass::kPrivate
                : sem->StorageClass();

  auto type_id = GenerateTypeIfNeeded(sem->Type());
  if (type_id == 0) {
    return false;
  }

  push_debug(spv::Op::OpName,
             {Operand::Int(var_id),
              Operand::String(builder_.Symbols().NameFor(var->symbol()))});

  OperandList ops = {Operand::Int(type_id), result,
                     Operand::Int(ConvertStorageClass(sc))};

  if (var->has_constructor()) {
    ops.push_back(Operand::Int(init_id));
  } else {
    if (type->Is<sem::StorageTexture>() || type->Is<sem::Struct>()) {
      // type is a sem::Struct or a sem::StorageTexture
      switch (sem->AccessControl()) {
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
    if (!type->Is<sem::Sampler>()) {
      // If we don't have a constructor and we're an Output or Private
      // variable, then WGSL requires that we zero-initialize.
      if (sem->StorageClass() == ast::StorageClass::kPrivate ||
          sem->StorageClass() == ast::StorageClass::kOutput) {
        init_id = GenerateConstantNullIfNeeded(type);
        if (init_id == 0) {
          return 0;
        }
        ops.push_back(Operand::Int(init_id));
      }
    }
  }

  push_type(spv::Op::OpVariable, std::move(ops));

  for (auto* deco : var->decorations()) {
    if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
      push_annot(spv::Op::OpDecorate,
                 {Operand::Int(var_id), Operand::Int(SpvDecorationBuiltIn),
                  Operand::Int(
                      ConvertBuiltin(builtin->value(), sem->StorageClass()))});
    } else if (auto* location = deco->As<ast::LocationDecoration>()) {
      push_annot(spv::Op::OpDecorate,
                 {Operand::Int(var_id), Operand::Int(SpvDecorationLocation),
                  Operand::Int(location->value())});
    } else if (auto* binding = deco->As<ast::BindingDecoration>()) {
      push_annot(spv::Op::OpDecorate,
                 {Operand::Int(var_id), Operand::Int(SpvDecorationBinding),
                  Operand::Int(binding->value())});
    } else if (auto* group = deco->As<ast::GroupDecoration>()) {
      push_annot(spv::Op::OpDecorate, {Operand::Int(var_id),
                                       Operand::Int(SpvDecorationDescriptorSet),
                                       Operand::Int(group->value())});
    } else if (deco->Is<ast::OverrideDecoration>()) {
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
  auto* type = TypeOf(expr->idx_expr());
  idx_id = GenerateLoadIfNeeded(type, idx_id);

  // If the source is a reference, we access chain into it.
  // In the future, pointers may support access-chaining.
  // See https://github.com/gpuweb/gpuweb/pull/1580
  if (info->source_type->Is<sem::Reference>()) {
    info->access_chain_indices.push_back(idx_id);
    info->source_type = TypeOf(expr);
    return true;
  }

  auto result_type_id = GenerateTypeIfNeeded(TypeOf(expr));
  if (result_type_id == 0) {
    return false;
  }

  // We don't have a pointer, so we can just directly extract the value.
  auto extract = result_op();
  auto extract_id = extract.to_i();

  // If the index is a literal, we use OpCompositeExtract.
  if (auto* scalar = expr->idx_expr()->As<ast::ScalarConstructorExpression>()) {
    auto* literal = scalar->literal()->As<ast::IntLiteral>();
    if (!literal) {
      TINT_ICE(builder_.Diagnostics()) << "bad literal in array accessor";
      return false;
    }

    if (!push_function_inst(spv::Op::OpCompositeExtract,
                            {Operand::Int(result_type_id), extract,
                             Operand::Int(info->source_id),
                             Operand::Int(literal->value_as_u32())})) {
      return false;
    }

    info->source_id = extract_id;
    info->source_type = TypeOf(expr);

    return true;
  }

  // If the source is a vector, we use OpVectorExtractDynamic.
  if (info->source_type->Is<sem::Vector>()) {
    if (!push_function_inst(
            spv::Op::OpVectorExtractDynamic,
            {Operand::Int(result_type_id), extract,
             Operand::Int(info->source_id), Operand::Int(idx_id)})) {
      return false;
    }

    info->source_id = extract_id;
    info->source_type = TypeOf(expr);

    return true;
  }

  TINT_ICE(builder_.Diagnostics()) << "unsupported array accessor expression";
  return false;
}

bool Builder::GenerateMemberAccessor(ast::MemberAccessorExpression* expr,
                                     AccessorInfo* info) {
  auto* expr_sem = builder_.Sem().Get(expr);
  auto* expr_type = expr_sem->Type();

  if (auto* access = expr_sem->As<sem::StructMemberAccess>()) {
    uint32_t idx = access->Member()->Index();

    if (info->source_type->Is<sem::Reference>()) {
      auto idx_id = GenerateConstantIfNeeded(ScalarConstant::U32(idx));
      if (idx_id == 0) {
        return 0;
      }
      info->access_chain_indices.push_back(idx_id);
      info->source_type = expr_type;
    } else {
      auto result_type_id = GenerateTypeIfNeeded(expr_type);
      if (result_type_id == 0) {
        return false;
      }

      auto extract = result_op();
      auto extract_id = extract.to_i();
      if (!push_function_inst(
              spv::Op::OpCompositeExtract,
              {Operand::Int(result_type_id), extract,
               Operand::Int(info->source_id), Operand::Int(idx)})) {
        return false;
      }

      info->source_id = extract_id;
      info->source_type = expr_type;
    }

    return true;
  }

  if (auto* swizzle = expr_sem->As<sem::Swizzle>()) {
    // Single element swizzle is either an access chain or a composite extract
    auto& indices = swizzle->Indices();
    if (indices.size() == 1) {
      if (info->source_type->Is<sem::Reference>()) {
        auto idx_id = GenerateConstantIfNeeded(ScalarConstant::U32(indices[0]));
        if (idx_id == 0) {
          return 0;
        }
        info->access_chain_indices.push_back(idx_id);
      } else {
        auto result_type_id = GenerateTypeIfNeeded(expr_type);
        if (result_type_id == 0) {
          return 0;
        }

        auto extract = result_op();
        auto extract_id = extract.to_i();
        if (!push_function_inst(
                spv::Op::OpCompositeExtract,
                {Operand::Int(result_type_id), extract,
                 Operand::Int(info->source_id), Operand::Int(indices[0])})) {
          return false;
        }

        info->source_id = extract_id;
        info->source_type = expr_type;
      }
      return true;
    }

    // Store the type away as it may change if we run the access chain
    auto* incoming_type = info->source_type;

    // Multi-item extract is a VectorShuffle. We have to emit any existing
    // access chain data, then load the access chain and shuffle that.
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

      info->source_id = GenerateLoadIfNeeded(expr_type, extract_id);
      info->source_type = expr_type->UnwrapRef();
      info->access_chain_indices.clear();
    }

    auto result_type_id = GenerateTypeIfNeeded(expr_type);
    if (result_type_id == 0) {
      return false;
    }

    auto vec_id = GenerateLoadIfNeeded(incoming_type, info->source_id);

    auto result = result_op();
    auto result_id = result.to_i();

    OperandList ops = {Operand::Int(result_type_id), result,
                       Operand::Int(vec_id), Operand::Int(vec_id)};

    for (auto idx : indices) {
      ops.push_back(Operand::Int(idx));
    }

    if (!push_function_inst(spv::Op::OpVectorShuffle, ops)) {
      return false;
    }
    info->source_id = result_id;
    info->source_type = expr_type;
    return true;
  }

  TINT_ICE(builder_.Diagnostics())
      << "unhandled member index type: " << expr_sem->TypeInfo().name;
  return false;
}

uint32_t Builder::GenerateAccessorExpression(ast::Expression* expr) {
  if (!expr->IsAnyOf<ast::ArrayAccessorExpression,
                     ast::MemberAccessorExpression>()) {
    TINT_ICE(builder_.Diagnostics()) << "expression is not an accessor";
    return 0;
  }

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
  info.source_type = TypeOf(source);

  if (auto* access = accessors[0]->As<ast::ArrayAccessorExpression>()) {
    auto* array = TypeOf(access->array())->As<sem::Array>();
    bool literal_index =
        array && access->idx_expr()->Is<ast::ScalarConstructorExpression>();
    if (array && !literal_index) {
      TINT_ICE(builder_.Diagnostics())
          << "Dynamic index on array value should have been promoted to "
             "storage with the VarForDynamicIndex transform";
    }
  }

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
      error_ = "invalid accessor in list: " + builder_.str(accessor);
      return 0;
    }
  }

  if (!info.access_chain_indices.empty()) {
    auto* type = TypeOf(expr);
    auto result_type_id = GenerateTypeIfNeeded(type);
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
           builder_.Symbols().NameFor(expr->symbol());
  return 0;
}

uint32_t Builder::GenerateLoadIfNeeded(const sem::Type* type, uint32_t id) {
  if (auto* ref = type->As<sem::Reference>()) {
    type = ref->StoreType();
  } else {
    return id;
  }

  auto type_id = GenerateTypeIfNeeded(type);
  auto result = result_op();
  auto result_id = result.to_i();
  if (!push_function_inst(spv::Op::OpLoad,
                          {Operand::Int(type_id), result, Operand::Int(id)})) {
    return 0;
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

  spv::Op op = spv::Op::OpNop;
  switch (expr->op()) {
    case ast::UnaryOp::kNegation:
      if (TypeOf(expr)->is_float_scalar_or_vector()) {
        op = spv::Op::OpFNegate;
      } else {
        op = spv::Op::OpSNegate;
      }
      break;
    case ast::UnaryOp::kNot:
      op = spv::Op::OpLogicalNot;
      break;
    case ast::UnaryOp::kAddressOf:
    case ast::UnaryOp::kIndirection:
      // Address-of converts a reference to a pointer, and dereference converts
      // a pointer to a reference. These are the same thing in SPIR-V, so this
      // is a no-op.
      return val_id;
  }

  val_id = GenerateLoadIfNeeded(TypeOf(expr->expr()), val_id);

  auto type_id = GenerateTypeIfNeeded(TypeOf(expr));
  if (type_id == 0) {
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
  auto* result_type = TypeOf(tc)->UnwrapRef();
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
    if (result_type->Is<sem::Vector>() && sc == nullptr) {
      return false;
    }

    // This should all be handled by |is_constructor_const| call above
    if (sc == nullptr) {
      continue;
    }

    const sem::Type* subtype = result_type->UnwrapRef();
    if (auto* vec = subtype->As<sem::Vector>()) {
      subtype = vec->type();
    } else if (auto* mat = subtype->As<sem::Matrix>()) {
      subtype = mat->type();
    } else if (auto* arr = subtype->As<sem::Array>()) {
      subtype = arr->ElemType();
    } else if (auto* str = subtype->As<sem::Struct>()) {
      subtype = str->Members()[i]->Type();
    }
    if (subtype != TypeOf(sc)->UnwrapRef()) {
      return false;
    }
  }
  return true;
}

uint32_t Builder::GenerateTypeConstructorExpression(
    ast::TypeConstructorExpression* init,
    bool is_global_init) {
  auto& values = init->values();

  auto* result_type = TypeOf(init);

  // Generate the zero initializer if there are no values provided.
  if (values.empty()) {
    return GenerateConstantNullIfNeeded(result_type->UnwrapRef());
  }

  std::ostringstream out;
  out << "__const_" << init->type()->FriendlyName(builder_.Symbols()) << "_";

  result_type = result_type->UnwrapRef();
  bool constructor_is_const = is_constructor_const(init, is_global_init);
  if (has_error()) {
    return 0;
  }

  bool can_cast_or_copy = result_type->is_scalar();

  if (auto* res_vec = result_type->As<sem::Vector>()) {
    if (res_vec->type()->is_scalar()) {
      auto* value_type = TypeOf(values[0])->UnwrapRef();
      if (auto* val_vec = value_type->As<sem::Vector>()) {
        if (val_vec->type()->is_scalar()) {
          can_cast_or_copy = res_vec->size() == val_vec->size();
        }
      }
    }
  }

  if (can_cast_or_copy) {
    return GenerateCastOrCopyOrPassthrough(result_type, values[0]);
  }

  auto type_id = GenerateTypeIfNeeded(result_type);
  if (type_id == 0) {
    return 0;
  }

  bool result_is_constant_composite = constructor_is_const;
  bool result_is_spec_composite = false;

  if (auto* vec = result_type->As<sem::Vector>()) {
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
      id = GenerateLoadIfNeeded(TypeOf(e), id);
    }
    if (id == 0) {
      return 0;
    }

    auto* value_type = TypeOf(e)->UnwrapRef();
    // If the result and value types are the same we can just use the object.
    // If the result is not a vector then we should have validated that the
    // value type is a correctly sized vector so we can just use it directly.
    if (result_type == value_type || result_type->Is<sem::Matrix>() ||
        result_type->Is<sem::Array>() || result_type->Is<sem::Struct>()) {
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
    if (auto* vec = value_type->As<sem::Vector>()) {
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
          auto idx_id = GenerateConstantIfNeeded(ScalarConstant::U32(i));
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
  auto val = type_constructor_to_id_.find(str);
  if (val != type_constructor_to_id_.end()) {
    return val->second;
  }

  auto result = result_op();
  ops.insert(ops.begin(), result);
  ops.insert(ops.begin(), Operand::Int(type_id));

  type_constructor_to_id_[str] = result.to_i();

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

uint32_t Builder::GenerateCastOrCopyOrPassthrough(const sem::Type* to_type,
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
  val_id = GenerateLoadIfNeeded(TypeOf(from_expr), val_id);

  auto* from_type = TypeOf(from_expr)->UnwrapRef();

  spv::Op op = spv::Op::OpNop;
  if ((from_type->Is<sem::I32>() && to_type->Is<sem::F32>()) ||
      (from_type->is_signed_integer_vector() && to_type->is_float_vector())) {
    op = spv::Op::OpConvertSToF;
  } else if ((from_type->Is<sem::U32>() && to_type->Is<sem::F32>()) ||
             (from_type->is_unsigned_integer_vector() &&
              to_type->is_float_vector())) {
    op = spv::Op::OpConvertUToF;
  } else if ((from_type->Is<sem::F32>() && to_type->Is<sem::I32>()) ||
             (from_type->is_float_vector() &&
              to_type->is_signed_integer_vector())) {
    op = spv::Op::OpConvertFToS;
  } else if ((from_type->Is<sem::F32>() && to_type->Is<sem::U32>()) ||
             (from_type->is_float_vector() &&
              to_type->is_unsigned_integer_vector())) {
    op = spv::Op::OpConvertFToU;
  } else if ((from_type->Is<sem::Bool>() && to_type->Is<sem::Bool>()) ||
             (from_type->Is<sem::U32>() && to_type->Is<sem::U32>()) ||
             (from_type->Is<sem::I32>() && to_type->Is<sem::I32>()) ||
             (from_type->Is<sem::F32>() && to_type->Is<sem::F32>()) ||
             (from_type->Is<sem::Vector>() && (from_type == to_type))) {
    return val_id;
  } else if ((from_type->Is<sem::I32>() && to_type->Is<sem::U32>()) ||
             (from_type->Is<sem::U32>() && to_type->Is<sem::I32>()) ||
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
  ScalarConstant constant;

  auto* sem_var = builder_.Sem().Get(var);
  if (sem_var && sem_var->IsPipelineConstant()) {
    constant.is_spec_op = true;
    constant.constant_id = sem_var->ConstantId();
  }

  if (auto* l = lit->As<ast::BoolLiteral>()) {
    constant.kind = ScalarConstant::Kind::kBool;
    constant.value.b = l->IsTrue();
  } else if (auto* sl = lit->As<ast::SintLiteral>()) {
    constant.kind = ScalarConstant::Kind::kI32;
    constant.value.i32 = sl->value();
  } else if (auto* ul = lit->As<ast::UintLiteral>()) {
    constant.kind = ScalarConstant::Kind::kU32;
    constant.value.u32 = ul->value();
  } else if (auto* fl = lit->As<ast::FloatLiteral>()) {
    constant.kind = ScalarConstant::Kind::kF32;
    constant.value.f32 = fl->value();
  } else {
    error_ = "unknown literal type";
    return 0;
  }

  return GenerateConstantIfNeeded(constant);
}

uint32_t Builder::GenerateConstantIfNeeded(const ScalarConstant& constant) {
  auto it = const_to_id_.find(constant);
  if (it != const_to_id_.end()) {
    return it->second;
  }

  uint32_t type_id = 0;

  switch (constant.kind) {
    case ScalarConstant::Kind::kU32: {
      sem::U32 u32;
      type_id = GenerateTypeIfNeeded(&u32);
      break;
    }
    case ScalarConstant::Kind::kI32: {
      sem::I32 i32;
      type_id = GenerateTypeIfNeeded(&i32);
      break;
    }
    case ScalarConstant::Kind::kF32: {
      sem::F32 f32;
      type_id = GenerateTypeIfNeeded(&f32);
      break;
    }
    case ScalarConstant::Kind::kBool: {
      sem::Bool bool_;
      type_id = GenerateTypeIfNeeded(&bool_);
      break;
    }
  }

  if (type_id == 0) {
    return 0;
  }

  auto result = result_op();
  auto result_id = result.to_i();

  if (constant.is_spec_op) {
    push_annot(spv::Op::OpDecorate,
               {Operand::Int(result_id), Operand::Int(SpvDecorationSpecId),
                Operand::Int(constant.constant_id)});
  }

  switch (constant.kind) {
    case ScalarConstant::Kind::kU32: {
      push_type(
          constant.is_spec_op ? spv::Op::OpSpecConstant : spv::Op::OpConstant,
          {Operand::Int(type_id), result, Operand::Int(constant.value.u32)});
      break;
    }
    case ScalarConstant::Kind::kI32: {
      push_type(
          constant.is_spec_op ? spv::Op::OpSpecConstant : spv::Op::OpConstant,
          {Operand::Int(type_id), result, Operand::Int(constant.value.i32)});
      break;
    }
    case ScalarConstant::Kind::kF32: {
      push_type(
          constant.is_spec_op ? spv::Op::OpSpecConstant : spv::Op::OpConstant,
          {Operand::Int(type_id), result, Operand::Float(constant.value.f32)});
      break;
    }
    case ScalarConstant::Kind::kBool: {
      if (constant.value.b) {
        push_type(constant.is_spec_op ? spv::Op::OpSpecConstantTrue
                                      : spv::Op::OpConstantTrue,
                  {Operand::Int(type_id), result});
      } else {
        push_type(constant.is_spec_op ? spv::Op::OpSpecConstantFalse
                                      : spv::Op::OpConstantFalse,
                  {Operand::Int(type_id), result});
      }
      break;
    }
  }

  const_to_id_[constant] = result_id;
  return result_id;
}

uint32_t Builder::GenerateConstantNullIfNeeded(const sem::Type* type) {
  auto type_id = GenerateTypeIfNeeded(type);
  if (type_id == 0) {
    return 0;
  }

  auto name = type->type_name();

  auto it = const_null_to_id_.find(name);
  if (it != const_null_to_id_.end()) {
    return it->second;
  }

  auto result = result_op();
  auto result_id = result.to_i();

  push_type(spv::Op::OpConstantNull, {Operand::Int(type_id), result});

  const_null_to_id_[name] = result_id;
  return result_id;
}

uint32_t Builder::GenerateShortCircuitBinaryExpression(
    ast::BinaryExpression* expr) {
  auto lhs_id = GenerateExpression(expr->lhs());
  if (lhs_id == 0) {
    return false;
  }
  lhs_id = GenerateLoadIfNeeded(TypeOf(expr->lhs()), lhs_id);

  // Get the ID of the basic block where control flow will diverge. It's the
  // last basic block generated for the left-hand-side of the operator.
  auto original_label_id = current_label_id_;

  auto type_id = GenerateTypeIfNeeded(TypeOf(expr));
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
  rhs_id = GenerateLoadIfNeeded(TypeOf(expr->rhs()), rhs_id);

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
  lhs_id = GenerateLoadIfNeeded(TypeOf(expr->lhs()), lhs_id);

  auto rhs_id = GenerateExpression(expr->rhs());
  if (rhs_id == 0) {
    return 0;
  }
  rhs_id = GenerateLoadIfNeeded(TypeOf(expr->rhs()), rhs_id);

  auto result = result_op();
  auto result_id = result.to_i();

  auto type_id = GenerateTypeIfNeeded(TypeOf(expr));
  if (type_id == 0) {
    return 0;
  }

  // Handle int and float and the vectors of those types. Other types
  // should have been rejected by validation.
  auto* lhs_type = TypeOf(expr->lhs())->UnwrapRef();
  auto* rhs_type = TypeOf(expr->rhs())->UnwrapRef();
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
      error_ = "invalid multiply expression";
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
  auto result = GenerateBlockStatementWithoutScoping(stmt);
  scope_stack_.pop_scope();
  return result;
}

bool Builder::GenerateBlockStatementWithoutScoping(
    const ast::BlockStatement* stmt) {
  for (auto* block_stmt : *stmt) {
    if (!GenerateStatement(block_stmt)) {
      return false;
    }
  }
  return true;
}

uint32_t Builder::GenerateCallExpression(ast::CallExpression* expr) {
  auto* ident = expr->func()->As<ast::IdentifierExpression>();

  if (ident == nullptr) {
    error_ = "invalid function name";
    return 0;
  }

  auto* call = builder_.Sem().Get(expr);
  auto* target = call->Target();
  if (auto* intrinsic = target->As<sem::Intrinsic>()) {
    return GenerateIntrinsic(expr, intrinsic);
  }

  auto type_id = GenerateTypeIfNeeded(target->ReturnType());
  if (type_id == 0) {
    return 0;
  }

  auto result = result_op();
  auto result_id = result.to_i();

  OperandList ops = {Operand::Int(type_id), result};

  auto func_id = func_symbol_to_id_[ident->symbol()];
  if (func_id == 0) {
    error_ = "unable to find called function: " +
             builder_.Symbols().NameFor(ident->symbol());
    return 0;
  }
  ops.push_back(Operand::Int(func_id));

  size_t arg_idx = 0;
  for (auto* arg : expr->params()) {
    auto id = GenerateExpression(arg);
    if (id == 0) {
      return 0;
    }
    id = GenerateLoadIfNeeded(TypeOf(arg), id);
    if (id == 0) {
      return 0;
    }
    ops.push_back(Operand::Int(id));
    arg_idx++;
  }

  if (!push_function_inst(spv::Op::OpFunctionCall, std::move(ops))) {
    return 0;
  }

  return result_id;
}

uint32_t Builder::GenerateIntrinsic(ast::CallExpression* call,
                                    const sem::Intrinsic* intrinsic) {
  auto result = result_op();
  auto result_id = result.to_i();

  auto result_type_id = GenerateTypeIfNeeded(intrinsic->ReturnType());
  if (result_type_id == 0) {
    return 0;
  }

  if (intrinsic->IsFineDerivative() || intrinsic->IsCoarseDerivative()) {
    push_capability(SpvCapabilityDerivativeControl);
  }

  if (intrinsic->IsImageQuery()) {
    push_capability(SpvCapabilityImageQuery);
  }

  if (intrinsic->IsTexture()) {
    if (!GenerateTextureIntrinsic(call, intrinsic, Operand::Int(result_type_id),
                                  result)) {
      return 0;
    }
    return result_id;
  }

  if (intrinsic->IsBarrier()) {
    if (!GenerateControlBarrierIntrinsic(intrinsic)) {
      return 0;
    }
    return result_id;
  }

  OperandList params = {Operand::Int(result_type_id), result};

  spv::Op op = spv::Op::OpNop;
  switch (intrinsic->Type()) {
    case IntrinsicType::kAny:
      op = spv::Op::OpAny;
      break;
    case IntrinsicType::kAll:
      op = spv::Op::OpAll;
      break;
    case IntrinsicType::kArrayLength: {
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

      auto* type = TypeOf(accessor->structure())->UnwrapRef();
      if (!type->Is<sem::Struct>()) {
        error_ =
            "invalid type (" + type->type_name() + ") for runtime array length";
        return 0;
      }
      // Runtime array must be the last member in the structure
      params.push_back(Operand::Int(uint32_t(
          type->As<sem::Struct>()->Declaration()->members().size() - 1)));

      if (!push_function_inst(spv::Op::OpArrayLength, params)) {
        return 0;
      }
      return result_id;
    }
    case IntrinsicType::kCountOneBits:
      op = spv::Op::OpBitCount;
      break;
    case IntrinsicType::kDot:
      op = spv::Op::OpDot;
      break;
    case IntrinsicType::kDpdx:
      op = spv::Op::OpDPdx;
      break;
    case IntrinsicType::kDpdxCoarse:
      op = spv::Op::OpDPdxCoarse;
      break;
    case IntrinsicType::kDpdxFine:
      op = spv::Op::OpDPdxFine;
      break;
    case IntrinsicType::kDpdy:
      op = spv::Op::OpDPdy;
      break;
    case IntrinsicType::kDpdyCoarse:
      op = spv::Op::OpDPdyCoarse;
      break;
    case IntrinsicType::kDpdyFine:
      op = spv::Op::OpDPdyFine;
      break;
    case IntrinsicType::kFwidth:
      op = spv::Op::OpFwidth;
      break;
    case IntrinsicType::kFwidthCoarse:
      op = spv::Op::OpFwidthCoarse;
      break;
    case IntrinsicType::kFwidthFine:
      op = spv::Op::OpFwidthFine;
      break;
    case IntrinsicType::kIsInf:
      op = spv::Op::OpIsInf;
      break;
    case IntrinsicType::kIsNan:
      op = spv::Op::OpIsNan;
      break;
    case IntrinsicType::kReverseBits:
      op = spv::Op::OpBitReverse;
      break;
    case IntrinsicType::kSelect:
      op = spv::Op::OpSelect;
      break;
    default: {
      GenerateGLSLstd450Import();

      auto set_iter = import_name_to_id_.find(kGLSLstd450);
      if (set_iter == import_name_to_id_.end()) {
        error_ = std::string("unknown import ") + kGLSLstd450;
        return 0;
      }
      auto set_id = set_iter->second;
      auto inst_id = intrinsic_to_glsl_method(intrinsic);
      if (inst_id == 0) {
        error_ = "unknown method " + std::string(intrinsic->str());
        return 0;
      }

      params.push_back(Operand::Int(set_id));
      params.push_back(Operand::Int(inst_id));

      op = spv::Op::OpExtInst;
      break;
    }
  }

  if (op == spv::Op::OpNop) {
    error_ =
        "unable to determine operator for: " + std::string(intrinsic->str());
    return 0;
  }

  for (size_t i = 0; i < call->params().size(); i++) {
    auto* arg = call->params()[i];
    auto& param = intrinsic->Parameters()[i];
    auto val_id = GenerateExpression(arg);
    if (val_id == 0) {
      return false;
    }

    if (!param.type->Is<sem::Pointer>()) {
      val_id = GenerateLoadIfNeeded(TypeOf(arg), val_id);
    }

    params.emplace_back(Operand::Int(val_id));
  }

  if (!push_function_inst(op, params)) {
    return 0;
  }

  return result_id;
}

bool Builder::GenerateTextureIntrinsic(ast::CallExpression* call,
                                       const sem::Intrinsic* intrinsic,
                                       Operand result_type,
                                       Operand result_id) {
  using Usage = sem::Parameter::Usage;

  auto parameters = intrinsic->Parameters();
  auto arguments = call->params();

  // Generates the given expression, returning the operand ID
  auto gen = [&](ast::Expression* expr) {
    auto val_id = GenerateExpression(expr);
    if (val_id == 0) {
      return Operand::Int(0);
    }
    val_id = GenerateLoadIfNeeded(TypeOf(expr), val_id);

    return Operand::Int(val_id);
  };

  // Returns the argument with the given usage
  auto arg = [&](Usage usage) {
    int idx = sem::IndexOf(parameters, usage);
    return (idx >= 0) ? arguments[idx] : nullptr;
  };

  // Generates the argument with the given usage, returning the operand ID
  auto gen_arg = [&](Usage usage) {
    auto* argument = arg(usage);
    if (!argument) {
      TINT_ICE(builder_.Diagnostics())
          << "missing argument " << static_cast<int>(usage);
    }
    return gen(argument);
  };

  auto* texture = arg(Usage::kTexture);
  if (!texture) {
    TINT_ICE(builder_.Diagnostics()) << "missing texture argument";
  }

  auto* texture_type = TypeOf(texture)->UnwrapRef()->As<sem::Texture>();

  auto op = spv::Op::OpNop;

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

  // Appends `result_type` and `result_id` to `spirv_params`
  auto append_result_type_and_id_to_spirv_params = [&]() {
    spirv_params.emplace_back(std::move(result_type));
    spirv_params.emplace_back(std::move(result_id));
  };

  // Appends a result type and id to `spirv_params`, possibly adding a
  // post_emission step.
  //
  // If the texture is a depth texture, then this function wraps the result of
  // the op with a OpCompositeExtract to evaluate to the first element of the
  // returned vector. This is done as the WGSL texture reading functions for
  // depths return a single float scalar instead of a vector.
  //
  // If the texture is not a depth texture, then this function simply delegates
  // to calling append_result_type_and_id_to_spirv_params().
  auto append_result_type_and_id_to_spirv_params_for_read = [&]() {
    if (texture_type->Is<sem::DepthTexture>()) {
      auto* f32 = builder_.create<sem::F32>();
      auto* spirv_result_type = builder_.create<sem::Vector>(f32, 4);
      auto spirv_result = result_op();
      post_emission = [=] {
        return push_function_inst(
            spv::Op::OpCompositeExtract,
            {result_type, result_id, spirv_result, Operand::Int(0)});
      };
      auto spirv_result_type_id = GenerateTypeIfNeeded(spirv_result_type);
      if (spirv_result_type_id == 0) {
        return false;
      }
      spirv_params.emplace_back(Operand::Int(spirv_result_type_id));
      spirv_params.emplace_back(spirv_result);
      return true;
    }

    append_result_type_and_id_to_spirv_params();
    return true;
  };

  // Appends a result type and id to `spirv_params`, by first swizzling the
  // result of the op with `swizzle`.
  auto append_result_type_and_id_to_spirv_params_swizzled =
      [&](uint32_t spirv_result_width, std::vector<uint32_t> swizzle) {
        if (swizzle.empty()) {
          append_result_type_and_id_to_spirv_params();
        } else {
          // Assign post_emission to swizzle the result of the call to
          // OpImageQuerySize[Lod].
          auto* element_type = ElementTypeOf(TypeOf(call));
          auto spirv_result = result_op();
          auto* spirv_result_type =
              builder_.create<sem::Vector>(element_type, spirv_result_width);
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
        return true;
      };

  auto append_coords_to_spirv_params = [&]() -> bool {
    if (auto* array_index = arg(Usage::kArrayIndex)) {
      // Array index needs to be appended to the coordinates.
      auto* packed = AppendVector(&builder_, arg(Usage::kCoords), array_index);
      auto param = GenerateTypeConstructorExpression(packed, false);
      if (param == 0) {
        return false;
      }
      spirv_params.emplace_back(Operand::Int(param));
    } else {
      spirv_params.emplace_back(gen_arg(Usage::kCoords));  // coordinates
    }
    return true;
  };

  auto append_image_and_coords_to_spirv_params = [&]() -> bool {
    auto sampler_param = gen_arg(Usage::kSampler);
    auto texture_param = gen_arg(Usage::kTexture);
    auto sampled_image =
        GenerateSampledImage(texture_type, texture_param, sampler_param);

    // Populate the spirv_params with the common parameters
    spirv_params.emplace_back(Operand::Int(sampled_image));  // sampled image
    return append_coords_to_spirv_params();
  };

  switch (intrinsic->Type()) {
    case IntrinsicType::kTextureDimensions: {
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
        case ast::TextureDimension::kNone:
          error_ = "texture dimension is kNone";
          return false;
        case ast::TextureDimension::k1d:
        case ast::TextureDimension::k2d:
        case ast::TextureDimension::k3d:
          break;  // No swizzle needed
        case ast::TextureDimension::kCube:
          swizzle = {0, 1, 1};  // Duplicate height for depth
          spirv_dims = 2;       // [width, height]
          break;
        case ast::TextureDimension::k2dArray:
          swizzle = {0, 1};  // Strip array index
          spirv_dims = 3;    // [width, height, array_count]
          break;
        case ast::TextureDimension::kCubeArray:
          swizzle = {0, 1, 1};  // Strip array index, duplicate height for depth
          spirv_dims = 3;       // [width, height, array_count]
          break;
      }

      if (!append_result_type_and_id_to_spirv_params_swizzled(spirv_dims,
                                                              swizzle)) {
        return false;
      }

      spirv_params.emplace_back(gen_arg(Usage::kTexture));
      if (texture_type->Is<sem::MultisampledTexture>() ||
          texture_type->Is<sem::StorageTexture>()) {
        op = spv::Op::OpImageQuerySize;
      } else if (auto* level = arg(Usage::kLevel)) {
        op = spv::Op::OpImageQuerySizeLod;
        spirv_params.emplace_back(gen(level));
      } else {
        ast::SintLiteral i32_0(ProgramID(), Source{}, 0);
        op = spv::Op::OpImageQuerySizeLod;
        spirv_params.emplace_back(
            Operand::Int(GenerateLiteralIfNeeded(nullptr, &i32_0)));
      }
      break;
    }
    case IntrinsicType::kTextureNumLayers: {
      uint32_t spirv_dims = 0;
      switch (texture_type->dim()) {
        default:
          error_ = "texture is not arrayed";
          return false;
        case ast::TextureDimension::k2dArray:
        case ast::TextureDimension::kCubeArray:
          spirv_dims = 3;
          break;
      }

      // OpImageQuerySize[Lod] packs the array count as the last element of the
      // returned vector. Extract this.
      if (!append_result_type_and_id_to_spirv_params_swizzled(
              spirv_dims, {spirv_dims - 1})) {
        return false;
      }

      spirv_params.emplace_back(gen_arg(Usage::kTexture));

      if (texture_type->Is<sem::MultisampledTexture>() ||
          texture_type->Is<sem::StorageTexture>()) {
        op = spv::Op::OpImageQuerySize;
      } else {
        ast::SintLiteral i32_0(ProgramID(), Source{}, 0);
        op = spv::Op::OpImageQuerySizeLod;
        spirv_params.emplace_back(
            Operand::Int(GenerateLiteralIfNeeded(nullptr, &i32_0)));
      }
      break;
    }
    case IntrinsicType::kTextureNumLevels: {
      op = spv::Op::OpImageQueryLevels;
      append_result_type_and_id_to_spirv_params();
      spirv_params.emplace_back(gen_arg(Usage::kTexture));
      break;
    }
    case IntrinsicType::kTextureNumSamples: {
      op = spv::Op::OpImageQuerySamples;
      append_result_type_and_id_to_spirv_params();
      spirv_params.emplace_back(gen_arg(Usage::kTexture));
      break;
    }
    case IntrinsicType::kTextureLoad: {
      op = texture_type->Is<sem::StorageTexture>() ? spv::Op::OpImageRead
                                                   : spv::Op::OpImageFetch;
      append_result_type_and_id_to_spirv_params_for_read();
      spirv_params.emplace_back(gen_arg(Usage::kTexture));
      if (!append_coords_to_spirv_params()) {
        return false;
      }

      if (auto* level = arg(Usage::kLevel)) {
        image_operands.emplace_back(
            ImageOperand{SpvImageOperandsLodMask, gen(level)});
      }

      if (auto* sample_index = arg(Usage::kSampleIndex)) {
        image_operands.emplace_back(
            ImageOperand{SpvImageOperandsSampleMask, gen(sample_index)});
      }

      break;
    }
    case IntrinsicType::kTextureStore: {
      op = spv::Op::OpImageWrite;
      spirv_params.emplace_back(gen_arg(Usage::kTexture));
      if (!append_coords_to_spirv_params()) {
        return false;
      }
      spirv_params.emplace_back(gen_arg(Usage::kValue));
      break;
    }
    case IntrinsicType::kTextureSample: {
      op = spv::Op::OpImageSampleImplicitLod;
      append_result_type_and_id_to_spirv_params_for_read();
      if (!append_image_and_coords_to_spirv_params()) {
        return false;
      }
      break;
    }
    case IntrinsicType::kTextureSampleBias: {
      op = spv::Op::OpImageSampleImplicitLod;
      append_result_type_and_id_to_spirv_params_for_read();
      if (!append_image_and_coords_to_spirv_params()) {
        return false;
      }
      image_operands.emplace_back(
          ImageOperand{SpvImageOperandsBiasMask, gen_arg(Usage::kBias)});
      break;
    }
    case IntrinsicType::kTextureSampleLevel: {
      op = spv::Op::OpImageSampleExplicitLod;
      append_result_type_and_id_to_spirv_params_for_read();
      if (!append_image_and_coords_to_spirv_params()) {
        return false;
      }
      auto level = Operand::Int(0);
      if (TypeOf(arg(Usage::kLevel))->Is<sem::I32>()) {
        // Depth textures have i32 parameters for the level, but SPIR-V expects
        // F32. Cast.
        auto f32_type_id = GenerateTypeIfNeeded(builder_.create<sem::F32>());
        if (f32_type_id == 0) {
          return 0;
        }
        level = result_op();
        if (!push_function_inst(
                spv::Op::OpConvertSToF,
                {Operand::Int(f32_type_id), level, gen_arg(Usage::kLevel)})) {
          return 0;
        }
      } else {
        level = gen_arg(Usage::kLevel);
      }
      image_operands.emplace_back(ImageOperand{SpvImageOperandsLodMask, level});
      break;
    }
    case IntrinsicType::kTextureSampleGrad: {
      op = spv::Op::OpImageSampleExplicitLod;
      append_result_type_and_id_to_spirv_params_for_read();
      if (!append_image_and_coords_to_spirv_params()) {
        return false;
      }
      image_operands.emplace_back(
          ImageOperand{SpvImageOperandsGradMask, gen_arg(Usage::kDdx)});
      image_operands.emplace_back(
          ImageOperand{SpvImageOperandsGradMask, gen_arg(Usage::kDdy)});
      break;
    }
    case IntrinsicType::kTextureSampleCompare: {
      op = spv::Op::OpImageSampleDrefExplicitLod;
      append_result_type_and_id_to_spirv_params();
      if (!append_image_and_coords_to_spirv_params()) {
        return false;
      }
      spirv_params.emplace_back(gen_arg(Usage::kDepthRef));

      ast::FloatLiteral float_0(ProgramID(), Source{}, 0.0);
      image_operands.emplace_back(ImageOperand{
          SpvImageOperandsLodMask,
          Operand::Int(GenerateLiteralIfNeeded(nullptr, &float_0))});
      break;
    }
    default:
      TINT_UNREACHABLE(builder_.Diagnostics());
      return false;
  }

  if (auto* offset = arg(Usage::kOffset)) {
    image_operands.emplace_back(
        ImageOperand{SpvImageOperandsConstOffsetMask, gen(offset)});
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
    error_ =
        "unable to determine operator for: " + std::string(intrinsic->str());
    return false;
  }

  if (!push_function_inst(op, spirv_params)) {
    return false;
  }

  return post_emission();
}

bool Builder::GenerateControlBarrierIntrinsic(const sem::Intrinsic* intrinsic) {
  auto const op = spv::Op::OpControlBarrier;
  uint32_t execution = 0;
  uint32_t memory = 0;
  uint32_t semantics = 0;

  // TODO(crbug.com/tint/661): Combine sequential barriers to a single
  // instruction.
  if (intrinsic->Type() == sem::IntrinsicType::kWorkgroupBarrier) {
    execution = static_cast<uint32_t>(spv::Scope::Workgroup);
    memory = static_cast<uint32_t>(spv::Scope::Workgroup);
    semantics =
        static_cast<uint32_t>(spv::MemorySemanticsMask::AcquireRelease) |
        static_cast<uint32_t>(spv::MemorySemanticsMask::WorkgroupMemory);
  } else if (intrinsic->Type() == sem::IntrinsicType::kStorageBarrier) {
    execution = static_cast<uint32_t>(spv::Scope::Workgroup);
    memory = static_cast<uint32_t>(spv::Scope::Device);
    semantics =
        static_cast<uint32_t>(spv::MemorySemanticsMask::AcquireRelease) |
        static_cast<uint32_t>(spv::MemorySemanticsMask::UniformMemory);
  } else {
    error_ = "unexpected barrier intrinsic type ";
    error_ += sem::str(intrinsic->Type());
    return false;
  }

  auto execution_id = GenerateConstantIfNeeded(ScalarConstant::U32(execution));
  auto memory_id = GenerateConstantIfNeeded(ScalarConstant::U32(memory));
  auto semantics_id = GenerateConstantIfNeeded(ScalarConstant::U32(semantics));
  if (execution_id == 0 || memory_id == 0 || semantics_id == 0) {
    return false;
  }

  return push_function_inst(op, {
                                    Operand::Int(execution_id),
                                    Operand::Int(memory_id),
                                    Operand::Int(semantics_id),
                                });
}

uint32_t Builder::GenerateSampledImage(const sem::Type* texture_type,
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

  auto result_type_id = GenerateTypeIfNeeded(TypeOf(expr));
  if (result_type_id == 0) {
    return 0;
  }

  auto val_id = GenerateExpression(expr->expr());
  if (val_id == 0) {
    return 0;
  }
  val_id = GenerateLoadIfNeeded(TypeOf(expr->expr()), val_id);

  // Bitcast does not allow same types, just emit a CopyObject
  auto* to_type = TypeOf(expr)->UnwrapRef();
  auto* from_type = TypeOf(expr->expr())->UnwrapRef();
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
  cond_id = GenerateLoadIfNeeded(TypeOf(cond), cond_id);

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
  cond_id = GenerateLoadIfNeeded(TypeOf(stmt->condition()), cond_id);

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
      auto* int_literal = selector->As<ast::IntLiteral>();
      if (!int_literal) {
        error_ = "expected integer literal for switch case label";
        return false;
      }

      params.push_back(Operand::Int(int_literal->value_as_u32()));
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
        // This case is caught by Resolver validation
        TINT_UNREACHABLE(builder_.Diagnostics());
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
    val_id = GenerateLoadIfNeeded(TypeOf(stmt->value()), val_id);
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

  // We need variables from the body to be visible in the continuing block, so
  // manage scope outside of GenerateBlockStatement.
  scope_stack_.push_scope();

  if (!GenerateBlockStatementWithoutScoping(stmt->body())) {
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
  if (stmt->has_continuing()) {
    if (!GenerateBlockStatementWithoutScoping(stmt->continuing())) {
      return false;
    }
  }

  scope_stack_.pop_scope();

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

  error_ = "Unknown statement: " + builder_.str(stmt);
  return false;
}

bool Builder::GenerateVariableDeclStatement(ast::VariableDeclStatement* stmt) {
  return GenerateFunctionVariable(stmt->variable());
}

uint32_t Builder::GenerateTypeIfNeeded(const sem::Type* type) {
  if (type == nullptr) {
    error_ = "attempting to generate type from null type";
    return 0;
  }

  // Pointers and References both map to a SPIR-V pointer type.
  // Transform a Reference to a Pointer to prevent these having duplicated
  // definitions in the generated SPIR-V. Note that nested references are not
  // legal, so only considering the top-level type is fine.
  std::string type_name;
  if (auto* ref = type->As<sem::Reference>()) {
    type_name = sem::Pointer(ref->StoreType(), ref->StorageClass()).type_name();
  } else {
    type_name = type->type_name();
  }

  return utils::GetOrCreate(type_name_to_id_, type_name, [&]() -> uint32_t {
    auto result = result_op();
    auto id = result.to_i();
    if (auto* arr = type->As<sem::Array>()) {
      if (!GenerateArrayType(arr, result)) {
        return 0;
      }
    } else if (type->Is<sem::Bool>()) {
      push_type(spv::Op::OpTypeBool, {result});
    } else if (type->Is<sem::F32>()) {
      push_type(spv::Op::OpTypeFloat, {result, Operand::Int(32)});
    } else if (type->Is<sem::I32>()) {
      push_type(spv::Op::OpTypeInt,
                {result, Operand::Int(32), Operand::Int(1)});
    } else if (auto* mat = type->As<sem::Matrix>()) {
      if (!GenerateMatrixType(mat, result)) {
        return 0;
      }
    } else if (auto* ptr = type->As<sem::Pointer>()) {
      if (!GeneratePointerType(ptr, result)) {
        return 0;
      }
    } else if (auto* ref = type->As<sem::Reference>()) {
      if (!GenerateReferenceType(ref, result)) {
        return 0;
      }
    } else if (auto* str = type->As<sem::Struct>()) {
      if (!GenerateStructType(str, result)) {
        return 0;
      }
    } else if (type->Is<sem::U32>()) {
      push_type(spv::Op::OpTypeInt,
                {result, Operand::Int(32), Operand::Int(0)});
    } else if (auto* vec = type->As<sem::Vector>()) {
      if (!GenerateVectorType(vec, result)) {
        return 0;
      }
    } else if (type->Is<sem::Void>()) {
      push_type(spv::Op::OpTypeVoid, {result});
    } else if (auto* tex = type->As<sem::Texture>()) {
      if (!GenerateTextureType(tex, result)) {
        return 0;
      }

      if (auto* st = tex->As<sem::StorageTexture>()) {
        // Register all three access types of StorageTexture names. In SPIR-V,
        // we must output a single type, while the variable is annotated with
        // the access type. Doing this ensures we de-dupe.
        type_name_to_id_[builder_
                             .create<sem::StorageTexture>(
                                 st->dim(), st->image_format(),
                                 ast::AccessControl::kReadOnly, st->type())
                             ->type_name()] = id;
        type_name_to_id_[builder_
                             .create<sem::StorageTexture>(
                                 st->dim(), st->image_format(),
                                 ast::AccessControl::kWriteOnly, st->type())
                             ->type_name()] = id;
        type_name_to_id_[builder_
                             .create<sem::StorageTexture>(
                                 st->dim(), st->image_format(),
                                 ast::AccessControl::kReadWrite, st->type())
                             ->type_name()] = id;
      }

    } else if (type->Is<sem::Sampler>()) {
      push_type(spv::Op::OpTypeSampler, {result});

      // Register both of the sampler type names. In SPIR-V they're the same
      // sampler type, so we need to match that when we do the dedup check.
      type_name_to_id_["__sampler_sampler"] = id;
      type_name_to_id_["__sampler_comparison"] = id;

    } else {
      error_ = "unable to convert type: " + type->type_name();
      return 0;
    }

    return id;
  });
}

// TODO(tommek): Cover multisampled textures here when they're included in AST
bool Builder::GenerateTextureType(const sem::Texture* texture,
                                  const Operand& result) {
  uint32_t array_literal = 0u;
  const auto dim = texture->dim();
  if (dim == ast::TextureDimension::k2dArray ||
      dim == ast::TextureDimension::kCubeArray) {
    array_literal = 1u;
  }

  uint32_t dim_literal = SpvDim2D;
  if (dim == ast::TextureDimension::k1d) {
    dim_literal = SpvDim1D;
    if (texture->Is<sem::SampledTexture>()) {
      push_capability(SpvCapabilitySampled1D);
    } else if (texture->Is<sem::StorageTexture>()) {
      push_capability(SpvCapabilityImage1D);
    }
  }
  if (dim == ast::TextureDimension::k3d) {
    dim_literal = SpvDim3D;
  }
  if (dim == ast::TextureDimension::kCube ||
      dim == ast::TextureDimension::kCubeArray) {
    dim_literal = SpvDimCube;
  }

  uint32_t ms_literal = 0u;
  if (texture->Is<sem::MultisampledTexture>()) {
    ms_literal = 1u;
  }

  uint32_t depth_literal = 0u;
  if (texture->Is<sem::DepthTexture>()) {
    depth_literal = 1u;
  }

  uint32_t sampled_literal = 2u;
  if (texture->Is<sem::MultisampledTexture>() ||
      texture->Is<sem::SampledTexture>() || texture->Is<sem::DepthTexture>()) {
    sampled_literal = 1u;
  }

  if (dim == ast::TextureDimension::kCubeArray) {
    if (texture->Is<sem::SampledTexture>() ||
        texture->Is<sem::DepthTexture>()) {
      push_capability(SpvCapabilitySampledCubeArray);
    }
  }

  uint32_t type_id = 0u;
  if (texture->Is<sem::DepthTexture>()) {
    sem::F32 f32;
    type_id = GenerateTypeIfNeeded(&f32);
  } else if (auto* s = texture->As<sem::SampledTexture>()) {
    type_id = GenerateTypeIfNeeded(s->type());
  } else if (auto* ms = texture->As<sem::MultisampledTexture>()) {
    type_id = GenerateTypeIfNeeded(ms->type());
  } else if (auto* st = texture->As<sem::StorageTexture>()) {
    type_id = GenerateTypeIfNeeded(st->type());
  }
  if (type_id == 0u) {
    return false;
  }

  uint32_t format_literal = SpvImageFormat_::SpvImageFormatUnknown;
  if (auto* t = texture->As<sem::StorageTexture>()) {
    format_literal = convert_image_format_to_spv(t->image_format());
  }

  push_type(spv::Op::OpTypeImage,
            {result, Operand::Int(type_id), Operand::Int(dim_literal),
             Operand::Int(depth_literal), Operand::Int(array_literal),
             Operand::Int(ms_literal), Operand::Int(sampled_literal),
             Operand::Int(format_literal)});

  return true;
}

bool Builder::GenerateArrayType(const sem::Array* ary, const Operand& result) {
  auto elem_type = GenerateTypeIfNeeded(ary->ElemType());
  if (elem_type == 0) {
    return false;
  }

  auto result_id = result.to_i();
  if (ary->IsRuntimeSized()) {
    push_type(spv::Op::OpTypeRuntimeArray, {result, Operand::Int(elem_type)});
  } else {
    auto len_id = GenerateConstantIfNeeded(ScalarConstant::U32(ary->Count()));
    if (len_id == 0) {
      return false;
    }

    push_type(spv::Op::OpTypeArray,
              {result, Operand::Int(elem_type), Operand::Int(len_id)});
  }

  push_annot(spv::Op::OpDecorate,
             {Operand::Int(result_id), Operand::Int(SpvDecorationArrayStride),
              Operand::Int(ary->Stride())});
  return true;
}

bool Builder::GenerateMatrixType(const sem::Matrix* mat,
                                 const Operand& result) {
  sem::Vector col_type(mat->type(), mat->rows());
  auto col_type_id = GenerateTypeIfNeeded(&col_type);
  if (has_error()) {
    return false;
  }

  push_type(spv::Op::OpTypeMatrix,
            {result, Operand::Int(col_type_id), Operand::Int(mat->columns())});
  return true;
}

bool Builder::GeneratePointerType(const sem::Pointer* ptr,
                                  const Operand& result) {
  auto subtype_id = GenerateTypeIfNeeded(ptr->StoreType());
  if (subtype_id == 0) {
    return false;
  }

  auto stg_class = ConvertStorageClass(ptr->StorageClass());
  if (stg_class == SpvStorageClassMax) {
    error_ = "invalid storage class for pointer";
    return false;
  }

  push_type(spv::Op::OpTypePointer,
            {result, Operand::Int(stg_class), Operand::Int(subtype_id)});

  return true;
}

bool Builder::GenerateReferenceType(const sem::Reference* ref,
                                    const Operand& result) {
  auto subtype_id = GenerateTypeIfNeeded(ref->StoreType());
  if (subtype_id == 0) {
    return false;
  }

  auto stg_class = ConvertStorageClass(ref->StorageClass());
  if (stg_class == SpvStorageClassMax) {
    error_ = "invalid storage class for reference";
    return false;
  }

  push_type(spv::Op::OpTypePointer,
            {result, Operand::Int(stg_class), Operand::Int(subtype_id)});

  return true;
}

bool Builder::GenerateStructType(const sem::Struct* struct_type,
                                 const Operand& result) {
  auto struct_id = result.to_i();
  auto* impl = struct_type->Declaration();

  if (impl->name().IsValid()) {
    push_debug(spv::Op::OpName,
               {Operand::Int(struct_id),
                Operand::String(builder_.Symbols().NameFor(impl->name()))});
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
              Operand::String(builder_.Symbols().NameFor(member->symbol()))});

  // Note: This will generate layout annotations for *all* structs, whether or
  // not they are used in host-shareable variables. This is officially ok in
  // SPIR-V 1.0 through 1.3. If / when we migrate to using SPIR-V 1.4 we'll have
  // to only generate the layout info for structs used for certain storage
  // classes.

  auto* sem_member = builder_.Sem().Get(member);
  if (!sem_member) {
    error_ = "Struct member has no semantic information";
    return 0;
  }

  push_annot(
      spv::Op::OpMemberDecorate,
      {Operand::Int(struct_id), Operand::Int(idx),
       Operand::Int(SpvDecorationOffset), Operand::Int(sem_member->Offset())});

  // Infer and emit matrix layout.
  auto* matrix_type = GetNestedMatrixType(sem_member->Type());
  if (matrix_type) {
    push_annot(spv::Op::OpMemberDecorate,
               {Operand::Int(struct_id), Operand::Int(idx),
                Operand::Int(SpvDecorationColMajor)});
    if (!matrix_type->type()->Is<sem::F32>()) {
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

  return GenerateTypeIfNeeded(sem_member->Type());
}

bool Builder::GenerateVectorType(const sem::Vector* vec,
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
    case ast::StorageClass::kInvalid:
      return SpvStorageClassMax;
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
    case ast::StorageClass::kStorage:
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

SpvBuiltIn Builder::ConvertBuiltin(ast::Builtin builtin,
                                   ast::StorageClass storage) {
  switch (builtin) {
    case ast::Builtin::kPosition:
      if (storage == ast::StorageClass::kInput) {
        return SpvBuiltInFragCoord;
      } else if (storage == ast::StorageClass::kOutput) {
        return SpvBuiltInPosition;
      } else {
        TINT_ICE(builder_.Diagnostics()) << "invalid storage class for builtin";
        break;
      }
    case ast::Builtin::kVertexIndex:
      return SpvBuiltInVertexIndex;
    case ast::Builtin::kInstanceIndex:
      return SpvBuiltInInstanceIndex;
    case ast::Builtin::kFrontFacing:
      return SpvBuiltInFrontFacing;
    case ast::Builtin::kFragCoord:
      return SpvBuiltInFragCoord;
    case ast::Builtin::kFragDepth:
      return SpvBuiltInFragDepth;
    case ast::Builtin::kLocalInvocationId:
      return SpvBuiltInLocalInvocationId;
    case ast::Builtin::kLocalInvocationIndex:
      return SpvBuiltInLocalInvocationIndex;
    case ast::Builtin::kGlobalInvocationId:
      return SpvBuiltInGlobalInvocationId;
    case ast::Builtin::kPointSize:
      return SpvBuiltInPointSize;
    case ast::Builtin::kWorkgroupId:
      return SpvBuiltInWorkgroupId;
    case ast::Builtin::kSampleIndex:
      push_capability(SpvCapabilitySampleRateShading);
      return SpvBuiltInSampleId;
    case ast::Builtin::kSampleMask:
      return SpvBuiltInSampleMask;
    case ast::Builtin::kSampleMaskIn:
      return SpvBuiltInSampleMask;
    case ast::Builtin::kSampleMaskOut:
      return SpvBuiltInSampleMask;
    case ast::Builtin::kNone:
      break;
  }
  return SpvBuiltInMax;
}

SpvImageFormat Builder::convert_image_format_to_spv(
    const ast::ImageFormat format) {
  switch (format) {
    case ast::ImageFormat::kR8Unorm:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatR8;
    case ast::ImageFormat::kR8Snorm:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatR8Snorm;
    case ast::ImageFormat::kR8Uint:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatR8ui;
    case ast::ImageFormat::kR8Sint:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatR8i;
    case ast::ImageFormat::kR16Uint:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatR16ui;
    case ast::ImageFormat::kR16Sint:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatR16i;
    case ast::ImageFormat::kR16Float:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatR16f;
    case ast::ImageFormat::kRg8Unorm:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatRg8;
    case ast::ImageFormat::kRg8Snorm:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatRg8Snorm;
    case ast::ImageFormat::kRg8Uint:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatRg8ui;
    case ast::ImageFormat::kRg8Sint:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatRg8i;
    case ast::ImageFormat::kR32Uint:
      return SpvImageFormatR32ui;
    case ast::ImageFormat::kR32Sint:
      return SpvImageFormatR32i;
    case ast::ImageFormat::kR32Float:
      return SpvImageFormatR32f;
    case ast::ImageFormat::kRg16Uint:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatRg16ui;
    case ast::ImageFormat::kRg16Sint:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatRg16i;
    case ast::ImageFormat::kRg16Float:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatRg16f;
    case ast::ImageFormat::kRgba8Unorm:
      return SpvImageFormatRgba8;
    case ast::ImageFormat::kRgba8UnormSrgb:
      return SpvImageFormatUnknown;
    case ast::ImageFormat::kRgba8Snorm:
      return SpvImageFormatRgba8Snorm;
    case ast::ImageFormat::kRgba8Uint:
      return SpvImageFormatRgba8ui;
    case ast::ImageFormat::kRgba8Sint:
      return SpvImageFormatRgba8i;
    case ast::ImageFormat::kBgra8Unorm:
      return SpvImageFormatUnknown;
    case ast::ImageFormat::kBgra8UnormSrgb:
      return SpvImageFormatUnknown;
    case ast::ImageFormat::kRgb10A2Unorm:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatRgb10A2;
    case ast::ImageFormat::kRg11B10Float:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatR11fG11fB10f;
    case ast::ImageFormat::kRg32Uint:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatRg32ui;
    case ast::ImageFormat::kRg32Sint:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatRg32i;
    case ast::ImageFormat::kRg32Float:
      push_capability(SpvCapabilityStorageImageExtendedFormats);
      return SpvImageFormatRg32f;
    case ast::ImageFormat::kRgba16Uint:
      return SpvImageFormatRgba16ui;
    case ast::ImageFormat::kRgba16Sint:
      return SpvImageFormatRgba16i;
    case ast::ImageFormat::kRgba16Float:
      return SpvImageFormatRgba16f;
    case ast::ImageFormat::kRgba32Uint:
      return SpvImageFormatRgba32ui;
    case ast::ImageFormat::kRgba32Sint:
      return SpvImageFormatRgba32i;
    case ast::ImageFormat::kRgba32Float:
      return SpvImageFormatRgba32f;
    case ast::ImageFormat::kNone:
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
