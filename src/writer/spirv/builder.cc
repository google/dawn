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

#include <limits>
#include <sstream>
#include <utility>

#include "spirv/unified1/spirv.h"
#include "src/ast/array_accessor_expression.h"
#include "src/ast/as_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/binding_decoration.h"
#include "src/ast/block_statement.h"
#include "src/ast/bool_literal.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/call_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/case_statement.h"
#include "src/ast/cast_expression.h"
#include "src/ast/constructor_expression.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/else_statement.h"
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
#include "src/ast/type/array_type.h"
#include "src/ast/type/depth_texture_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
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
#include "src/ast/variable_decl_statement.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

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
  return !stmts->empty() && stmts->last()->IsFallthrough();
}

// A terminator is anything which will case a SPIR-V terminator to be emitted.
// This means things like breaks, fallthroughs and continues which all emit an
// OpBranch or return for the OpReturn emission.
bool LastIsTerminator(const ast::BlockStatement* stmts) {
  if (stmts->empty()) {
    return false;
  }

  auto* last = stmts->last();
  return last->IsBreak() || last->IsContinue() || last->IsDiscard() ||
         last->IsReturn() || last->IsFallthrough();
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

/// Returns the matrix type that is |type| or that is wrapped by
/// one or more levels of an arrays inside of |type|.
/// @param type the given type, which must not be null
/// @returns the nested matrix type, or nullptr if none
ast::type::MatrixType* GetNestedMatrixType(ast::type::Type* type) {
  while (type->IsArray()) {
    type = type->AsArray()->type();
  }
  return type->IsMatrix() ? type->AsMatrix() : nullptr;
}

}  // namespace

Builder::AccessorInfo::AccessorInfo() : source_id(0), source_type(nullptr) {}

Builder::AccessorInfo::~AccessorInfo() {}

Builder::Builder(ast::Module* mod) : mod_(mod), scope_stack_({}) {}

Builder::~Builder() = default;

bool Builder::Build() {
  push_capability(SpvCapabilityShader);

  // TODO(dneto): Stop using the Vulkan memory model. crbug.com/tint/63
  push_capability(SpvCapabilityVulkanMemoryModel);
  push_preamble(spv::Op::OpExtension,
                {Operand::String("SPV_KHR_vulkan_memory_model")});

  for (const auto& imp : mod_->imports()) {
    GenerateImport(imp.get());
  }

  push_preamble(spv::Op::OpMemoryModel,
                {Operand::Int(SpvAddressingModelLogical),
                 Operand::Int(SpvMemoryModelVulkanKHR)});

  for (const auto& var : mod_->global_variables()) {
    if (!GenerateGlobalVariable(var.get())) {
      return false;
    }
  }

  for (const auto& func : mod_->functions()) {
    if (!GenerateFunction(func.get())) {
      return false;
    }
  }

  // Note, the entry points must be generated after the functions as they need
  // to be able to lookup the function information based on the name.
  for (const auto& ep : mod_->entry_points()) {
    if (!GenerateEntryPoint(ep.get())) {
      return false;
    }
  }
  for (const auto& ep : mod_->entry_points()) {
    if (!GenerateExecutionModes(ep.get())) {
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
  size += size_of(preamble_);
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
  for (const auto& func : functions_) {
    func.iterate(cb);
  }
}

void Builder::push_capability(uint32_t cap) {
  capabilities_.push_back(
      Instruction{spv::Op::OpCapability, {Operand::Int(cap)}});
}

void Builder::GenerateLabel(uint32_t id) {
  push_function_inst(spv::Op::OpLabel, {Operand::Int(id)});
  current_label_id_ = id;
}

uint32_t Builder::GenerateU32Literal(uint32_t val) {
  ast::type::U32Type u32;
  ast::SintLiteral lit(&u32, val);
  return GenerateLiteralIfNeeded(&lit);
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

  GenerateStore(lhs_id, rhs_id);
  return true;
}

bool Builder::GenerateBreakStatement(ast::BreakStatement*) {
  if (merge_stack_.empty()) {
    error_ = "Attempted to break without a merge block";
    return false;
  }
  push_function_inst(spv::Op::OpBranch, {Operand::Int(merge_stack_.back())});
  return true;
}

bool Builder::GenerateContinueStatement(ast::ContinueStatement*) {
  if (continue_stack_.empty()) {
    error_ = "Attempted to continue without a continue block";
    return false;
  }
  push_function_inst(spv::Op::OpBranch, {Operand::Int(continue_stack_.back())});
  return true;
}

// TODO(dsinclair): This is generating an OpKill but the semantics of kill
// haven't been defined for WGSL yet. So, this may need to change.
// https://github.com/gpuweb/gpuweb/issues/676
bool Builder::GenerateDiscardStatement(ast::DiscardStatement*) {
  push_function_inst(spv::Op::OpKill, {});
  return true;
}

bool Builder::GenerateEntryPoint(ast::EntryPoint* ep) {
  auto name = ep->name();
  if (name.empty()) {
    name = ep->function_name();
  }
  const auto id = id_for_entry_point(ep);
  if (id == 0) {
    return false;
  }

  auto stage = pipeline_stage_to_execution_model(ep->stage());
  if (stage == SpvExecutionModelMax) {
    error_ = "Unknown pipeline stage provided";
    return false;
  }

  OperandList operands = {Operand::Int(stage), Operand::Int(id),
                          Operand::String(name)};

  auto* func = func_name_to_func_[ep->function_name()];
  if (func == nullptr) {
    error_ = "processing an entry point when the function has not been seen.";
    return false;
  }

  for (const auto* var : func->referenced_module_variables()) {
    // For SPIR-V 1.3 we only output Input/output variables. If we update to
    // SPIR-V 1.4 or later this should be all variables.
    if (var->storage_class() != ast::StorageClass::kInput &&
        var->storage_class() != ast::StorageClass::kOutput) {
      continue;
    }

    uint32_t var_id;
    if (!scope_stack_.get(var->name(), &var_id)) {
      error_ = "unable to find ID for global variable: " + var->name();
      return false;
    }

    operands.push_back(Operand::Int(var_id));
  }
  push_preamble(spv::Op::OpEntryPoint, operands);

  return true;
}

bool Builder::GenerateExecutionModes(ast::EntryPoint* ep) {
  const auto id = id_for_entry_point(ep);
  if (id == 0) {
    return false;
  }

  // WGSL fragment shader origin is upper left
  if (ep->stage() == ast::PipelineStage::kFragment) {
    push_preamble(
        spv::Op::OpExecutionMode,
        {Operand::Int(id), Operand::Int(SpvExecutionModeOriginUpperLeft)});
  } else if (ep->stage() == ast::PipelineStage::kCompute) {
    // TODO(dsinclair): Support LocalSize other then (1, 1, 1)
    push_preamble(spv::Op::OpExecutionMode,
                  {Operand::Int(id), Operand::Int(SpvExecutionModeLocalSize),
                   Operand::Int(1), Operand::Int(1), Operand::Int(1)});
  }

  return true;
}

uint32_t Builder::GenerateExpression(ast::Expression* expr) {
  if (expr->IsArrayAccessor()) {
    return GenerateAccessorExpression(expr->AsArrayAccessor());
  }
  if (expr->IsAs()) {
    return GenerateAsExpression(expr->AsAs());
  }
  if (expr->IsBinary()) {
    return GenerateBinaryExpression(expr->AsBinary());
  }
  if (expr->IsCall()) {
    return GenerateCallExpression(expr->AsCall());
  }
  if (expr->IsCast()) {
    return GenerateCastExpression(expr->AsCast());
  }
  if (expr->IsConstructor()) {
    return GenerateConstructorExpression(expr->AsConstructor(), false);
  }
  if (expr->IsIdentifier()) {
    return GenerateIdentifierExpression(expr->AsIdentifier());
  }
  if (expr->IsMemberAccessor()) {
    return GenerateAccessorExpression(expr->AsMemberAccessor());
  }
  if (expr->IsUnaryOp()) {
    return GenerateUnaryOpExpression(expr->AsUnaryOp());
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
             {Operand::Int(func_id), Operand::String(func->name())});

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
  for (const auto& param : func->params()) {
    auto param_op = result_op();
    auto param_id = param_op.to_i();

    auto param_type_id = GenerateTypeIfNeeded(param->type());
    if (param_type_id == 0) {
      return false;
    }

    push_debug(spv::Op::OpName,
               {Operand::Int(param_id), Operand::String(param->name())});
    params.push_back(Instruction{spv::Op::OpFunctionParameter,
                                 {Operand::Int(param_type_id), param_op}});

    scope_stack_.set(param->name(), param_id);
  }

  push_function(Function{definition_inst, result_op(), std::move(params)});

  for (const auto& stmt : *(func->body())) {
    if (!GenerateStatement(stmt.get())) {
      return false;
    }
  }

  scope_stack_.pop_scope();

  func_name_to_id_[func->name()] = func_id;
  func_name_to_func_[func->name()] = func;
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
  for (const auto& param : func->params()) {
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
  }

  if (var->is_const()) {
    if (!var->has_constructor()) {
      error_ = "missing constructor for constant";
      return false;
    }
    scope_stack_.set(var->name(), init_id);
    spirv_id_to_variable_[init_id] = var;
    return true;
  }

  auto result = result_op();
  auto var_id = result.to_i();
  auto sc = ast::StorageClass::kFunction;
  ast::type::PointerType pt(var->type(), sc);
  auto type_id = GenerateTypeIfNeeded(&pt);
  if (type_id == 0) {
    return false;
  }

  push_debug(spv::Op::OpName,
             {Operand::Int(var_id), Operand::String(var->name())});

  // TODO(dsinclair) We could detect if the constructor is fully const and emit
  // an initializer value for the variable instead of doing the OpLoad.
  ast::NullLiteral nl(var->type()->UnwrapPtrIfNeeded());
  auto null_id = GenerateLiteralIfNeeded(&nl);
  if (null_id == 0) {
    return 0;
  }
  push_function_var({Operand::Int(type_id), result,
                     Operand::Int(ConvertStorageClass(sc)),
                     Operand::Int(null_id)});

  if (var->has_constructor()) {
    init_id = GenerateLoadIfNeeded(var->constructor()->result_type(), init_id);
    GenerateStore(var_id, init_id);
  }

  scope_stack_.set(var->name(), var_id);
  spirv_id_to_variable_[var_id] = var;

  return true;
}

void Builder::GenerateStore(uint32_t to, uint32_t from) {
  push_function_inst(spv::Op::OpStore, {Operand::Int(to), Operand::Int(from)});
}

bool Builder::GenerateGlobalVariable(ast::Variable* var) {
  uint32_t init_id = 0;
  if (var->has_constructor()) {
    if (!var->constructor()->IsConstructor()) {
      error_ = "scalar constructor expected";
      return false;
    }

    init_id = GenerateConstructorExpression(var->constructor()->AsConstructor(),
                                            true);
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
               {Operand::Int(init_id), Operand::String(var->name())});

    scope_stack_.set_global(var->name(), init_id);
    spirv_id_to_variable_[init_id] = var;
    return true;
  }

  auto result = result_op();
  auto var_id = result.to_i();

  auto sc = var->storage_class() == ast::StorageClass::kNone
                ? ast::StorageClass::kPrivate
                : var->storage_class();

  ast::type::PointerType pt(var->type(), sc);
  auto type_id = GenerateTypeIfNeeded(&pt);
  if (type_id == 0) {
    return false;
  }

  push_debug(spv::Op::OpName,
             {Operand::Int(var_id), Operand::String(var->name())});

  OperandList ops = {Operand::Int(type_id), result,
                     Operand::Int(ConvertStorageClass(sc))};
  if (var->has_constructor()) {
    ops.push_back(Operand::Int(init_id));
  } else {
    // If we don't have a constructor and we're an Output or Private variable
    // then WGSL requires an initializer.
    if (var->storage_class() == ast::StorageClass::kPrivate ||
        var->storage_class() == ast::StorageClass::kNone ||
        var->storage_class() == ast::StorageClass::kOutput) {
      ast::NullLiteral nl(var->type()->UnwrapPtrIfNeeded());
      init_id = GenerateLiteralIfNeeded(&nl);
      if (init_id == 0) {
        return 0;
      }
      ops.push_back(Operand::Int(init_id));
    }
  }

  push_type(spv::Op::OpVariable, std::move(ops));

  if (var->IsDecorated()) {
    for (const auto& deco : var->AsDecorated()->decorations()) {
      if (deco->IsBuiltin()) {
        push_annot(spv::Op::OpDecorate,
                   {Operand::Int(var_id), Operand::Int(SpvDecorationBuiltIn),
                    Operand::Int(ConvertBuiltin(deco->AsBuiltin()->value()))});
      } else if (deco->IsLocation()) {
        push_annot(spv::Op::OpDecorate,
                   {Operand::Int(var_id), Operand::Int(SpvDecorationLocation),
                    Operand::Int(deco->AsLocation()->value())});
      } else if (deco->IsBinding()) {
        push_annot(spv::Op::OpDecorate,
                   {Operand::Int(var_id), Operand::Int(SpvDecorationBinding),
                    Operand::Int(deco->AsBinding()->value())});
      } else if (deco->IsSet()) {
        push_annot(
            spv::Op::OpDecorate,
            {Operand::Int(var_id), Operand::Int(SpvDecorationDescriptorSet),
             Operand::Int(deco->AsSet()->value())});
      } else {
        error_ = "unknown decoration";
        return false;
      }
    }
  }
  scope_stack_.set_global(var->name(), var_id);
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

  // If the source is a pointer we access chain into it.
  if (info->source_type->IsPointer()) {
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

  push_function_inst(spv::Op::OpVectorExtractDynamic,
                     {Operand::Int(result_type_id), extract,
                      Operand::Int(info->source_id), Operand::Int(idx_id)});

  info->source_id = extract_id;
  info->source_type = expr->result_type();

  return true;
}

bool Builder::GenerateMemberAccessor(ast::MemberAccessorExpression* expr,
                                     AccessorInfo* info) {
  auto* data_type = expr->structure()
                        ->result_type()
                        ->UnwrapPtrIfNeeded()
                        ->UnwrapAliasesIfNeeded();

  // If the data_type is a structure we're accessing a member, if it's a
  // vector we're accessing a swizzle.
  if (data_type->IsStruct()) {
    if (!info->source_type->IsPointer()) {
      error_ =
          "Attempting to access a struct member on a non-pointer. Something is "
          "wrong";
      return false;
    }

    auto* strct = data_type->AsStruct()->impl();
    auto name = expr->member()->name();

    uint32_t i = 0;
    for (; i < strct->members().size(); ++i) {
      const auto& member = strct->members()[i];
      if (member->name() == name) {
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

  if (!data_type->IsVector()) {
    error_ = "Member accessor without a struct or vector. Something is wrong";
    return false;
  }

  auto swiz = expr->member()->name();
  // Single element swizzle is either an access chain or a composite extract
  if (swiz.size() == 1) {
    auto val = IndexFromName(swiz[0]);
    if (val == std::numeric_limits<uint32_t>::max()) {
      error_ = "invalid swizzle name: " + swiz;
      return false;
    }

    if (info->source_type->IsPointer()) {
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
      push_function_inst(spv::Op::OpCompositeExtract,
                         {Operand::Int(result_type_id), extract,
                          Operand::Int(info->source_id), Operand::Int(val)});

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

    push_function_inst(spv::Op::OpAccessChain, ops);

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

  push_function_inst(spv::Op::OpVectorShuffle, ops);
  info->source_id = result_id;
  info->source_type = expr->result_type();

  return true;
}

uint32_t Builder::GenerateAccessorExpression(ast::Expression* expr) {
  assert(expr->IsArrayAccessor() || expr->IsMemberAccessor());

  // Gather a list of all the member and array accessors that are in this chain.
  // The list is built in reverse order as that's the order we need to access
  // the chain.
  std::vector<ast::Expression*> accessors;
  ast::Expression* source = expr;
  while (true) {
    if (source->IsArrayAccessor()) {
      accessors.insert(accessors.begin(), source);
      source = source->AsArrayAccessor()->array();
    } else if (source->IsMemberAccessor()) {
      accessors.insert(accessors.begin(), source);
      source = source->AsMemberAccessor()->structure();
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

  std::vector<uint32_t> access_chain_indices;
  for (auto* accessor : accessors) {
    if (accessor->IsArrayAccessor()) {
      if (!GenerateArrayAccessor(accessor->AsArrayAccessor(), &info)) {
        return 0;
      }
    } else if (accessor->IsMemberAccessor()) {
      if (!GenerateMemberAccessor(accessor->AsMemberAccessor(), &info)) {
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

    push_function_inst(spv::Op::OpAccessChain, ops);
    info.source_id = result_id;
  }

  return info.source_id;
}

uint32_t Builder::GenerateIdentifierExpression(
    ast::IdentifierExpression* expr) {
  uint32_t val = 0;
  if (expr->has_path()) {
    auto* imp = mod_->FindImportByName(expr->path());
    if (imp == nullptr) {
      error_ = "unable to find import for " + expr->path();
      return 0;
    }
    val = imp->GetIdForMethod(expr->name());
    if (val == 0) {
      error_ = "unable to lookup: " + expr->name() + " in " + expr->path();
    }
    return val;
  }
  if (scope_stack_.get(expr->name(), &val)) {
    return val;
  }

  error_ = "unable to find name for identifier: " + expr->name();
  return 0;
}

uint32_t Builder::GenerateLoadIfNeeded(ast::type::Type* type, uint32_t id) {
  if (!type->IsPointer()) {
    return id;
  }

  auto type_id = GenerateTypeIfNeeded(type->UnwrapPtrIfNeeded());
  auto result = result_op();
  auto result_id = result.to_i();
  push_function_inst(spv::Op::OpLoad,
                     {Operand::Int(type_id), result, Operand::Int(id)});
  return result_id;
}

uint32_t Builder::GenerateUnaryOpExpression(ast::UnaryOpExpression* expr) {
  auto result = result_op();
  auto result_id = result.to_i();

  auto val_id = GenerateExpression(expr->expr());
  if (val_id == 0) {
    return 0;
  }

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
    op = spv::Op::OpNot;
  }
  if (op == spv::Op::OpNop) {
    error_ = "invalid unary op type";
    return 0;
  }

  push_function_inst(op, {Operand::Int(type_id), result, Operand::Int(val_id)});

  return result_id;
}

void Builder::GenerateImport(ast::Import* imp) {
  auto result = result_op();
  auto id = result.to_i();

  push_preamble(spv::Op::OpExtInstImport,
                {result, Operand::String(imp->path())});

  import_name_to_id_[imp->name()] = id;
}

uint32_t Builder::GenerateConstructorExpression(
    ast::ConstructorExpression* expr,
    bool is_global_init) {
  if (expr->IsScalarConstructor()) {
    return GenerateLiteralIfNeeded(expr->AsScalarConstructor()->literal());
  }
  if (expr->IsTypeConstructor()) {
    return GenerateTypeConstructorExpression(expr->AsTypeConstructor(),
                                             is_global_init);
  }

  error_ = "unknown constructor expression";
  return 0;
}

uint32_t Builder::GenerateTypeConstructorExpression(
    ast::TypeConstructorExpression* init,
    bool is_global_init) {
  auto type_id = GenerateTypeIfNeeded(init->type());
  if (type_id == 0) {
    return 0;
  }

  // Generate the zero initializer if there are no values provided.
  if (init->values().empty()) {
    ast::NullLiteral nl(init->type()->UnwrapPtrIfNeeded());
    return GenerateLiteralIfNeeded(&nl);
  }

  auto* result_type = init->type()->UnwrapPtrIfNeeded();
  if (result_type->IsVector()) {
    result_type = result_type->AsVector()->type();
  } else if (result_type->IsArray()) {
    result_type = result_type->AsArray()->type();
  } else if (result_type->IsMatrix()) {
    result_type = result_type->AsMatrix()->type();
  }

  std::ostringstream out;
  out << "__const";

  OperandList ops;
  bool constructor_is_const = true;
  for (const auto& e : init->values()) {
    if (!e->IsConstructor()) {
      if (is_global_init) {
        error_ = "constructor must be a constant expression";
        return 0;
      }
      constructor_is_const = false;
    }
  }

  bool result_is_constant_composite = constructor_is_const;
  bool result_is_spec_composite = false;
  for (const auto& e : init->values()) {
    uint32_t id = 0;
    if (constructor_is_const) {
      id = GenerateConstructorExpression(e->AsConstructor(), is_global_init);
    } else {
      id = GenerateExpression(e.get());
      id = GenerateLoadIfNeeded(e->result_type(), id);
    }
    if (id == 0) {
      return 0;
    }

    auto* value_type = e->result_type()->UnwrapPtrIfNeeded();

    // When handling vectors as the values there a few cases to take into
    // consideration:
    //  1. Module scoped vec3<f32>(vec2<f32>(1, 2), 3)  -> OpSpecConstantOp
    //  2. Function scoped vec3<f32>(vec2<f32>(1, 2), 3) -> OpCompositeExtract
    //  3. Either array<vec3<f32>, 1>(vec3<f32>(1, 2, 3))  -> use the ID.
    if (value_type->IsVector()) {
      auto* vec = value_type->AsVector();
      auto* vec_type = vec->type();

      // If the value we want is the same as what we have, use it directly.
      // This maps to case 3.
      if (result_type == value_type) {
        out << "_" << id;
        ops.push_back(Operand::Int(id));
      } else if (!is_global_init) {
        // A non-global initializer. Case 2.
        auto value_type_id = GenerateTypeIfNeeded(vec_type);
        if (value_type_id == 0) {
          return 0;
        }

        for (uint32_t i = 0; i < vec->size(); ++i) {
          auto extract = result_op();
          auto extract_id = extract.to_i();

          push_function_inst(spv::Op::OpCompositeExtract,
                             {Operand::Int(value_type_id), extract,
                              Operand::Int(id), Operand::Int(i)});

          out << "_" << extract_id;
          ops.push_back(Operand::Int(extract_id));

          // We no longer have a constant composite, but have to do a
          // composite construction as these calls are inside a function.
          result_is_constant_composite = false;
        }
      } else {
        // A global initializer, must use OpSpecConstantOp. Case 1.
        auto value_type_id = GenerateTypeIfNeeded(vec_type);
        if (value_type_id == 0) {
          return 0;
        }

        for (uint32_t i = 0; i < vec->size(); ++i) {
          auto extract = result_op();
          auto extract_id = extract.to_i();

          auto idx_id = GenerateU32Literal(i);
          if (idx_id == 0) {
            return 0;
          }
          push_type(spv::Op::OpSpecConstantOp,
                    {Operand::Int(value_type_id), extract,
                     Operand::Int(SpvOpCompositeExtract), Operand::Int(id),
                     Operand::Int(idx_id)});

          out << "_" << extract_id;
          ops.push_back(Operand::Int(extract_id));

          result_is_spec_composite = true;
        }
      }
    } else {
      out << "_" << id;
      ops.push_back(Operand::Int(id));
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
    push_function_inst(spv::Op::OpCompositeConstruct, ops);
  }
  return result.to_i();
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
  } else if (lit->IsSint()) {
    push_type(spv::Op::OpConstant, {Operand::Int(type_id), result,
                                    Operand::Int(lit->AsSint()->value())});
  } else if (lit->IsUint()) {
    push_type(spv::Op::OpConstant, {Operand::Int(type_id), result,
                                    Operand::Int(lit->AsUint()->value())});
  } else if (lit->IsFloat()) {
    push_type(spv::Op::OpConstant, {Operand::Int(type_id), result,
                                    Operand::Float(lit->AsFloat()->value())});
  } else if (lit->IsNull()) {
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

  push_function_inst(spv::Op::OpSelectionMerge,
                     {Operand::Int(merge_block_id),
                      Operand::Int(SpvSelectionControlMaskNone)});
  push_function_inst(spv::Op::OpBranchConditional,
                     {Operand::Int(lhs_id), Operand::Int(true_block_id),
                      Operand::Int(false_block_id)});

  // Output block to check the RHS
  GenerateLabel(block_id);
  auto rhs_id = GenerateExpression(expr->rhs());
  if (rhs_id == 0) {
    return 0;
  }
  rhs_id = GenerateLoadIfNeeded(expr->rhs()->result_type(), rhs_id);

  push_function_inst(spv::Op::OpBranch, {Operand::Int(merge_block_id)});

  // Output the merge block
  GenerateLabel(merge_block_id);

  auto result = result_op();
  auto result_id = result.to_i();

  push_function_inst(spv::Op::OpPhi,
                     {Operand::Int(type_id), result, Operand::Int(lhs_id),
                      Operand::Int(original_label_id), Operand::Int(rhs_id),
                      Operand::Int(block_id)});

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
  } else if (expr->IsShiftRight()) {
    // TODO(dsinclair): This depends on the type of the LHS if it's a
    // OpShiftRightLogical or OpShiftRightArithmetic
    // http://crbug.com/tint/84
    op = spv::Op::OpShiftRightLogical;
  } else if (expr->IsSubtract()) {
    op = lhs_is_float_or_vec ? spv::Op::OpFSub : spv::Op::OpISub;
  } else if (expr->IsXor()) {
    op = spv::Op::OpBitwiseXor;
  } else {
    error_ = "unknown binary expression";
    return 0;
  }

  push_function_inst(op, {Operand::Int(type_id), result, Operand::Int(lhs_id),
                          Operand::Int(rhs_id)});
  return result_id;
}

bool Builder::GenerateBlockStatement(const ast::BlockStatement* stmt) {
  scope_stack_.push_scope();
  for (const auto& block_stmt : *stmt) {
    if (!GenerateStatement(block_stmt.get())) {
      return false;
    }
  }
  scope_stack_.pop_scope();

  return true;
}

uint32_t Builder::GenerateCallExpression(ast::CallExpression* expr) {
  if (!expr->func()->IsIdentifier()) {
    error_ = "invalid function name";
    return 0;
  }

  auto* ident = expr->func()->AsIdentifier();

  if (!ident->has_path() && ast::intrinsic::IsIntrinsic(ident->name())) {
    return GenerateIntrinsic(ident->name(), expr);
  }

  auto type_id = GenerateTypeIfNeeded(expr->func()->result_type());
  if (type_id == 0) {
    return 0;
  }

  auto result = result_op();
  auto result_id = result.to_i();

  spv::Op op = spv::Op::OpNop;
  OperandList ops = {Operand::Int(type_id), result};

  // Handle regular function calls
  if (!ident->has_path()) {
    auto func_id = func_name_to_id_[ident->name()];
    if (func_id == 0) {
      error_ = "unable to find called function: " + ident->name();
      return 0;
    }
    ops.push_back(Operand::Int(func_id));

    op = spv::Op::OpFunctionCall;
  } else {
    // Imported function call
    auto set_iter = import_name_to_id_.find(ident->path());
    if (set_iter == import_name_to_id_.end()) {
      error_ = "unknown import " + ident->path();
      return 0;
    }
    auto set_id = set_iter->second;

    auto* imp = mod_->FindImportByName(ident->path());
    if (imp == nullptr) {
      error_ = "unknown import " + ident->path();
      return 0;
    }

    auto inst_id = imp->GetIdForMethod(ident->name());
    if (inst_id == 0) {
      error_ = "unknown method " + ident->name();
      return 0;
    }

    ops.push_back(Operand::Int(set_id));
    ops.push_back(Operand::Int(inst_id));

    op = spv::Op::OpExtInst;
  }

  for (const auto& param : expr->params()) {
    auto id = GenerateExpression(param.get());
    if (id == 0) {
      return 0;
    }
    id = GenerateLoadIfNeeded(param->result_type(), id);
    ops.push_back(Operand::Int(id));
  }

  push_function_inst(op, std::move(ops));

  return result_id;
}

uint32_t Builder::GenerateIntrinsic(const std::string& name,
                                    ast::CallExpression* call) {
  auto result = result_op();
  auto result_id = result.to_i();

  auto result_type_id = GenerateTypeIfNeeded(call->result_type());
  if (result_type_id == 0) {
    return 0;
  }

  OperandList params = {Operand::Int(result_type_id), result};
  for (const auto& p : call->params()) {
    auto val_id = GenerateExpression(p.get());
    if (val_id == 0) {
      return 0;
    }
    val_id = GenerateLoadIfNeeded(p->result_type(), val_id);

    params.push_back(Operand::Int(val_id));
  }

  if (ast::intrinsic::IsFineDerivative(name) ||
      ast::intrinsic::IsCoarseDerivative(name)) {
    push_capability(SpvCapabilityDerivativeControl);
  }

  spv::Op op = spv::Op::OpNop;
  if (name == "any") {
    op = spv::Op::OpAny;
  } else if (name == "all") {
    op = spv::Op::OpAll;
  } else if (name == "dot") {
    op = spv::Op::OpDot;
  } else if (name == "dpdx") {
    op = spv::Op::OpDPdx;
  } else if (name == "dpdx_coarse") {
    op = spv::Op::OpDPdxCoarse;
  } else if (name == "dpdx_fine") {
    op = spv::Op::OpDPdxFine;
  } else if (name == "dpdy") {
    op = spv::Op::OpDPdy;
  } else if (name == "dpdy_coarse") {
    op = spv::Op::OpDPdyCoarse;
  } else if (name == "dpdy_fine") {
    op = spv::Op::OpDPdyFine;
  } else if (name == "fwidth") {
    op = spv::Op::OpFwidth;
  } else if (name == "fwidth_coarse") {
    op = spv::Op::OpFwidthCoarse;
  } else if (name == "fwidth_fine") {
    op = spv::Op::OpFwidthFine;
  } else if (name == "is_inf") {
    op = spv::Op::OpIsInf;
  } else if (name == "is_nan") {
    op = spv::Op::OpIsNan;
  } else if (name == "outer_product") {
    op = spv::Op::OpOuterProduct;
  } else if (name == "select") {
    op = spv::Op::OpSelect;
  }
  if (op == spv::Op::OpNop) {
    error_ = "unable to determine operator for: " + name;
    return 0;
  }
  push_function_inst(op, params);

  return result_id;
}

uint32_t Builder::GenerateAsExpression(ast::AsExpression* as) {
  auto result = result_op();
  auto result_id = result.to_i();

  auto result_type_id = GenerateTypeIfNeeded(as->result_type());
  if (result_type_id == 0) {
    return 0;
  }

  auto val_id = GenerateExpression(as->expr());
  if (val_id == 0) {
    return 0;
  }
  val_id = GenerateLoadIfNeeded(as->expr()->result_type(), val_id);

  // Bitcast does not allow same types, just emit a CopyObject
  auto* to_type = as->result_type()->UnwrapPtrIfNeeded();
  auto* from_type = as->expr()->result_type()->UnwrapPtrIfNeeded();
  if (to_type->type_name() == from_type->type_name()) {
    push_function_inst(spv::Op::OpCopyObject, {Operand::Int(result_type_id),
                                               result, Operand::Int(val_id)});
    return result_id;
  }

  push_function_inst(spv::Op::OpBitcast, {Operand::Int(result_type_id), result,
                                          Operand::Int(val_id)});

  return result_id;
}

uint32_t Builder::GenerateCastExpression(ast::CastExpression* cast) {
  auto result = result_op();
  auto result_id = result.to_i();

  auto result_type_id = GenerateTypeIfNeeded(cast->result_type());
  if (result_type_id == 0) {
    return 0;
  }

  auto val_id = GenerateExpression(cast->expr());
  if (val_id == 0) {
    return 0;
  }
  val_id = GenerateLoadIfNeeded(cast->expr()->result_type(), val_id);

  auto* to_type = cast->result_type()->UnwrapPtrIfNeeded();
  auto* from_type = cast->expr()->result_type()->UnwrapPtrIfNeeded();

  spv::Op op = spv::Op::OpNop;
  if ((from_type->IsI32() && to_type->IsF32()) ||
      (from_type->is_signed_integer_vector() && to_type->is_float_vector())) {
    op = spv::Op::OpConvertSToF;
  } else if ((from_type->IsU32() && to_type->IsF32()) ||
             (from_type->is_unsigned_integer_vector() &&
              to_type->is_float_vector())) {
    op = spv::Op::OpConvertUToF;
  } else if ((from_type->IsF32() && to_type->IsI32()) ||
             (from_type->is_float_vector() &&
              to_type->is_signed_integer_vector())) {
    op = spv::Op::OpConvertFToS;
  } else if ((from_type->IsF32() && to_type->IsU32()) ||
             (from_type->is_float_vector() &&
              to_type->is_unsigned_integer_vector())) {
    op = spv::Op::OpConvertFToU;
  } else if ((from_type->IsU32() && to_type->IsU32()) ||
             (from_type->IsI32() && to_type->IsI32()) ||
             (from_type->IsF32() && to_type->IsF32()) ||
             (from_type->is_unsigned_integer_vector() &&
              to_type->is_unsigned_integer_vector()) ||
             (from_type->is_signed_integer_vector() &&
              to_type->is_signed_integer_vector()) ||
             (from_type->is_float_vector() && to_type->is_float_vector())) {
    op = spv::Op::OpCopyObject;
  } else if ((from_type->IsI32() && to_type->IsU32()) ||
             (from_type->IsU32() && to_type->IsI32()) ||
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

  push_function_inst(
      op, {Operand::Int(result_type_id), result, Operand::Int(val_id)});

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

  auto merge_block = result_op();
  auto merge_block_id = merge_block.to_i();

  push_function_inst(spv::Op::OpSelectionMerge,
                     {Operand::Int(merge_block_id),
                      Operand::Int(SpvSelectionControlMaskNone)});

  auto true_block = result_op();
  auto true_block_id = true_block.to_i();

  // if there are no more else statements we branch on false to the merge block
  // otherwise we branch to the false block
  auto false_block_id =
      cur_else_idx < else_stmts.size() ? next_id() : merge_block_id;

  push_function_inst(spv::Op::OpBranchConditional,
                     {Operand::Int(cond_id), Operand::Int(true_block_id),
                      Operand::Int(false_block_id)});

  // Output true block
  GenerateLabel(true_block_id);
  if (!GenerateBlockStatement(true_body)) {
    return false;
  }
  // We only branch if the last element of the body didn't already branch.
  if (!LastIsTerminator(true_body)) {
    push_function_inst(spv::Op::OpBranch, {Operand::Int(merge_block_id)});
  }

  // Start the false block if needed
  if (false_block_id != merge_block_id) {
    GenerateLabel(false_block_id);

    auto* else_stmt = else_stmts[cur_else_idx].get();
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
      push_function_inst(spv::Op::OpBranch, {Operand::Int(merge_block_id)});
    }
  }

  // Output the merge block
  GenerateLabel(merge_block_id);

  return true;
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
  for (const auto& item : stmt->body()) {
    if (item->IsDefault()) {
      case_ids.push_back(default_block_id);
      continue;
    }

    auto block = result_op();
    auto block_id = block.to_i();

    case_ids.push_back(block_id);
    for (const auto& selector : item->selectors()) {
      if (!selector->IsSint()) {
        error_ = "expected integer literal for switch case label";
        return false;
      }

      params.push_back(Operand::Int(selector->AsSint()->value()));
      params.push_back(Operand::Int(block_id));
    }
  }

  push_function_inst(spv::Op::OpSelectionMerge,
                     {Operand::Int(merge_block_id),
                      Operand::Int(SpvSelectionControlMaskNone)});
  push_function_inst(spv::Op::OpSwitch, params);

  bool generated_default = false;
  auto& body = stmt->body();
  // We output the case statements in order they were entered in the original
  // source. Each fallthrough goes to the next case entry, so is a forward
  // branch, otherwise the branch is to the merge block which comes after
  // the switch statement.
  for (uint32_t i = 0; i < body.size(); i++) {
    auto& item = body[i];

    if (item->IsDefault()) {
      generated_default = true;
    }

    GenerateLabel(case_ids[i]);
    if (!GenerateBlockStatement(item->body())) {
      return false;
    }

    if (LastIsFallthrough(item->body())) {
      if (i == (body.size() - 1)) {
        error_ = "fallthrough of last case statement is disallowed";
        return false;
      }
      push_function_inst(spv::Op::OpBranch, {Operand::Int(case_ids[i + 1])});
    } else if (!LastIsTerminator(item->body())) {
      push_function_inst(spv::Op::OpBranch, {Operand::Int(merge_block_id)});
    }
  }

  if (!generated_default) {
    GenerateLabel(default_block_id);
    push_function_inst(spv::Op::OpBranch, {Operand::Int(merge_block_id)});
  }

  merge_stack_.pop_back();

  GenerateLabel(merge_block_id);
  return true;
}

bool Builder::GenerateReturnStatement(ast::ReturnStatement* stmt) {
  if (stmt->has_value()) {
    auto val_id = GenerateExpression(stmt->value());
    if (val_id == 0) {
      return false;
    }
    push_function_inst(spv::Op::OpReturnValue, {Operand::Int(val_id)});
  } else {
    push_function_inst(spv::Op::OpReturn, {});
  }

  return true;
}

bool Builder::GenerateLoopStatement(ast::LoopStatement* stmt) {
  auto loop_header = result_op();
  auto loop_header_id = loop_header.to_i();
  push_function_inst(spv::Op::OpBranch, {Operand::Int(loop_header_id)});
  GenerateLabel(loop_header_id);

  auto merge_block = result_op();
  auto merge_block_id = merge_block.to_i();
  auto continue_block = result_op();
  auto continue_block_id = continue_block.to_i();

  auto body_block = result_op();
  auto body_block_id = body_block.to_i();

  push_function_inst(
      spv::Op::OpLoopMerge,
      {Operand::Int(merge_block_id), Operand::Int(continue_block_id),
       Operand::Int(SpvLoopControlMaskNone)});

  continue_stack_.push_back(continue_block_id);
  merge_stack_.push_back(merge_block_id);

  push_function_inst(spv::Op::OpBranch, {Operand::Int(body_block_id)});
  GenerateLabel(body_block_id);
  if (!GenerateBlockStatement(stmt->body())) {
    return false;
  }

  // We only branch if the last element of the body didn't already branch.
  if (!LastIsTerminator(stmt->body())) {
    push_function_inst(spv::Op::OpBranch, {Operand::Int(continue_block_id)});
  }

  GenerateLabel(continue_block_id);
  if (!GenerateBlockStatement(stmt->continuing())) {
    return false;
  }
  push_function_inst(spv::Op::OpBranch, {Operand::Int(loop_header_id)});

  merge_stack_.pop_back();
  continue_stack_.pop_back();

  GenerateLabel(merge_block_id);

  return true;
}

bool Builder::GenerateStatement(ast::Statement* stmt) {
  if (stmt->IsAssign()) {
    return GenerateAssignStatement(stmt->AsAssign());
  }
  if (stmt->IsBlock()) {
    return GenerateBlockStatement(stmt->AsBlock());
  }
  if (stmt->IsBreak()) {
    return GenerateBreakStatement(stmt->AsBreak());
  }
  if (stmt->IsCall()) {
    return GenerateCallExpression(stmt->AsCall()->expr()) != 0;
  }
  if (stmt->IsContinue()) {
    return GenerateContinueStatement(stmt->AsContinue());
  }
  if (stmt->IsDiscard()) {
    return GenerateDiscardStatement(stmt->AsDiscard());
  }
  if (stmt->IsFallthrough()) {
    // Do nothing here, the fallthrough gets handled by the switch code.
    return true;
  }
  if (stmt->IsIf()) {
    return GenerateIfStatement(stmt->AsIf());
  }
  if (stmt->IsLoop()) {
    return GenerateLoopStatement(stmt->AsLoop());
  }
  if (stmt->IsReturn()) {
    return GenerateReturnStatement(stmt->AsReturn());
  }
  if (stmt->IsSwitch()) {
    return GenerateSwitchStatement(stmt->AsSwitch());
  }
  if (stmt->IsVariableDecl()) {
    return GenerateVariableDeclStatement(stmt->AsVariableDecl());
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
  } else if (type->IsTexture()) {
    if (!GenerateTextureType(type->AsTexture(), result)) {
      return 0;
    }
  } else if (type->IsSampler()) {
    push_type(spv::Op::OpTypeSampler, {result});
  } else {
    error_ = "unable to convert type: " + type->type_name();
    return 0;
  }

  type_name_to_id_[type->type_name()] = id;
  return id;
}

// TODO(tommek): Cover multisampled textures here when they're included in AST
bool Builder::GenerateTextureType(ast::type::TextureType* texture,
                                  const Operand& result) {
  uint32_t array_literal = 0u;
  auto dim = texture->dim();
  if (dim == ast::type::TextureDimension::k1dArray ||
      dim == ast::type::TextureDimension::k2dArray ||
      dim == ast::type::TextureDimension::k2dMsArray ||
      dim == ast::type::TextureDimension::kCubeArray) {
    array_literal = 1u;
  }

  uint32_t dim_literal = SpvDim2D;
  if (dim == ast::type::TextureDimension::k1dArray ||
      dim == ast::type::TextureDimension::k1d) {
    dim_literal = SpvDim1D;
  }
  if (dim == ast::type::TextureDimension::k3d) {
    dim_literal = SpvDim3D;
  }
  if (dim == ast::type::TextureDimension::kCube ||
      dim == ast::type::TextureDimension::kCubeArray) {
    dim_literal = SpvDimCube;
  }

  uint32_t ms_literal = 0u;
  if (dim == ast::type::TextureDimension::k2dMs ||
      dim == ast::type::TextureDimension::k2dMsArray) {
    ms_literal = 1u;
  }

  uint32_t depth_literal = 0u;
  if (texture->IsDepth()) {
    depth_literal = 1u;
  }

  uint32_t sampled_literal = 2u;
  if (texture->IsSampled() || texture->IsDepth()) {
    sampled_literal = 1u;
  }

  uint32_t type_id = 0u;
  if (texture->IsDepth()) {
    ast::type::F32Type f32;
    type_id = GenerateTypeIfNeeded(&f32);
  } else if (texture->IsSampled()) {
    type_id = GenerateTypeIfNeeded(texture->AsSampled()->type());
  } else if (texture->IsStorage()) {
    if (texture->AsStorage()->access() == ast::type::StorageAccess::kWrite) {
      ast::type::VoidType void_type;
      type_id = GenerateTypeIfNeeded(&void_type);
    } else {
      type_id = GenerateTypeIfNeeded(texture->AsStorage()->type());
    }
  }
  if (type_id == 0u) {
    return false;
  }

  uint32_t format_literal = SpvImageFormat_::SpvImageFormatUnknown;
  if (texture->IsStorage()) {
    format_literal =
        convert_image_format_to_spv(texture->AsStorage()->image_format());
  }

  push_type(spv::Op::OpTypeImage,
            {result, Operand::Int(type_id), Operand::Int(dim_literal),
             Operand::Int(depth_literal), Operand::Int(array_literal),
             Operand::Int(ms_literal), Operand::Int(sampled_literal),
             Operand::Int(format_literal)});

  return true;
}

bool Builder::GenerateArrayType(ast::type::ArrayType* ary,
                                const Operand& result) {
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

  // SPIR-V explicitly requires no array stride if the array contains a struct
  // which has a Block decoration.
  if (ary->type()->IsStruct() && ary->type()->AsStruct()->IsBlockDecorated()) {
    return true;
  }
  if (ary->has_array_stride()) {
    push_annot(spv::Op::OpDecorate,
               {Operand::Int(result_id), Operand::Int(SpvDecorationArrayStride),
                Operand::Int(ary->array_stride())});
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
    error_ = "invalid storage class for pointer";
    return false;
  }

  push_type(spv::Op::OpTypePointer,
            {result, Operand::Int(stg_class), Operand::Int(pointee_id)});

  return true;
}

bool Builder::GenerateStructType(ast::type::StructType* struct_type,
                                 const Operand& result) {
  auto struct_id = result.to_i();
  auto* impl = struct_type->impl();

  if (!struct_type->name().empty()) {
    push_debug(spv::Op::OpName,
               {Operand::Int(struct_id), Operand::String(struct_type->name())});
  }

  OperandList ops;
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

  bool has_layout = false;
  for (const auto& deco : member->decorations()) {
    if (deco->IsOffset()) {
      push_annot(spv::Op::OpMemberDecorate,
                 {Operand::Int(struct_id), Operand::Int(idx),
                  Operand::Int(SpvDecorationOffset),
                  Operand::Int(deco->AsOffset()->offset())});
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
      if (!matrix_type->type()->IsF32()) {
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
    case ast::Builtin::kWorkgroupSize:
      return SpvBuiltInWorkgroupSize;
    case ast::Builtin::kLocalInvocationId:
      return SpvBuiltInLocalInvocationId;
    case ast::Builtin::kLocalInvocationIdx:
      return SpvBuiltInLocalInvocationIndex;
    case ast::Builtin::kGlobalInvocationId:
      return SpvBuiltInGlobalInvocationId;
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
  }
}

}  // namespace spirv
}  // namespace writer
}  // namespace tint
