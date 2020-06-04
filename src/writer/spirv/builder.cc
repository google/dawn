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
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/binding_decoration.h"
#include "src/ast/bool_literal.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/call_expression.h"
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
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/uint_literal.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"

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

bool LastIsFallthrough(const ast::StatementList& stmts) {
  return !stmts.empty() && stmts.back()->IsFallthrough();
}

// A terminator is anything which will case a SPIR-V terminator to be emitted.
// This means things like breaks, fallthroughs and continues which all emit an
// OpBranch or return for the OpReturn emission.
bool LastIsTerminator(const ast::StatementList& stmts) {
  if (stmts.empty()) {
    return false;
  }

  auto* last = stmts.back().get();
  // TODO(dneto): Conditional break and conditional continue should return
  // false.
  return last->IsBreak() || last->IsContinue() || last->IsReturn() ||
         last->IsKill() || last->IsFallthrough();
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

}  // namespace

Builder::AccessorInfo::AccessorInfo() : source_id(0), source_type(nullptr) {}

Builder::AccessorInfo::~AccessorInfo() {}

Builder::Builder(ast::Module* mod) : mod_(mod), scope_stack_({}) {}

Builder::~Builder() = default;

bool Builder::Build() {
  push_preamble(spv::Op::OpCapability, {Operand::Int(SpvCapabilityShader)});

  // TODO(dneto): Stop using the Vulkan memory model. crbug.com/tint/63
  push_preamble(spv::Op::OpCapability,
                {Operand::Int(SpvCapabilityVulkanMemoryModel)});
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
bool Builder::GenerateKillStatement(ast::KillStatement*) {
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

  std::vector<Operand> operands = {Operand::Int(stage), Operand::Int(id),
                                   Operand::String(name)};
  // TODO(dsinclair): This could be made smarter by only listing the
  // input/output variables which are used by the entry point instead of just
  // listing all module scoped variables of type input/output.
  for (const auto& var : mod_->global_variables()) {
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

  // TODO(dsinclair): Handle parameters

  auto definition_inst = Instruction{
      spv::Op::OpFunction,
      {Operand::Int(ret_id), func_op, Operand::Int(SpvFunctionControlMaskNone),
       Operand::Int(func_type_id)}};
  std::vector<Instruction> params;
  push_function(Function{definition_inst, result_op(), std::move(params)});

  scope_stack_.push_scope();

  for (const auto& stmt : func->body()) {
    if (!GenerateStatement(stmt.get())) {
      return false;
    }
  }

  scope_stack_.pop_scope();

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

  std::vector<Operand> ops = {Operand::Int(type_id), result,
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
  auto* data_type = expr->structure()->result_type()->UnwrapPtrIfNeeded();
  while (data_type->IsAlias()) {
    data_type = data_type->AsAlias()->type();
  }

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

    std::vector<Operand> ops = {Operand::Int(result_type_id), extract,
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

  std::vector<Operand> ops = {Operand::Int(result_type_id), result,
                              Operand::Int(vec_id), Operand::Int(vec_id)};

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

    std::vector<Operand> ops = {Operand::Int(result_type_id), result,
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

  std::ostringstream out;
  out << "__const";

  std::vector<Operand> ops;
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

    auto* result_type = e->result_type()->UnwrapPtrIfNeeded();

    // If we're putting a vector into the constructed composite we need to
    // extract each of the values and insert them individually
    if (result_type->IsVector()) {
      auto* vec = result_type->AsVector();
      auto result_type_id = GenerateTypeIfNeeded(vec->type());
      if (result_type_id == 0) {
        return 0;
      }

      for (uint32_t i = 0; i < vec->size(); ++i) {
        auto extract = result_op();
        auto extract_id = extract.to_i();

        push_function_inst(spv::Op::OpCompositeExtract,
                           {Operand::Int(result_type_id), extract,
                            Operand::Int(id), Operand::Int(i)});

        out << "_" << extract_id;
        ops.push_back(Operand::Int(extract_id));
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

  if (constructor_is_const) {
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

uint32_t Builder::GenerateBinaryExpression(ast::BinaryExpression* expr) {
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

uint32_t Builder::GenerateCallExpression(ast::CallExpression* expr) {
  if (!expr->func()->IsIdentifier()) {
    error_ = "invalid function name";
    return 0;
  }

  auto* ident = expr->func()->AsIdentifier();

  if (!ident->has_path() && ast::intrinsic::IsIntrinsic(ident->name())) {
    return GenerateIntrinsic(ident->name(), expr);
  }

  // TODO(dsinclair): Support regular function calls
  if (!ident->has_path()) {
    error_ = "function calls not supported yet.";
    return 0;
  }

  auto type_id = GenerateTypeIfNeeded(expr->func()->result_type());
  if (type_id == 0) {
    return 0;
  }

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

  auto result = result_op();
  auto result_id = result.to_i();

  std::vector<Operand> ops{Operand::Int(type_id), result, Operand::Int(set_id),
                           Operand::Int(inst_id)};

  for (const auto& param : expr->params()) {
    auto id = GenerateExpression(param.get());
    if (id == 0) {
      return 0;
    }
    ops.push_back(Operand::Int(GenerateLoadIfNeeded(param->result_type(), id)));
  }

  push_function_inst(spv::Op::OpExtInst, std::move(ops));

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

  std::vector<Operand> params = {Operand::Int(result_type_id), result};
  for (const auto& p : call->params()) {
    auto val_id = GenerateExpression(p.get());
    if (val_id == 0) {
      return 0;
    }
    val_id = GenerateLoadIfNeeded(p->result_type(), val_id);

    params.push_back(Operand::Int(val_id));
  }

  spv::Op op = spv::Op::OpNop;
  if (name == "any") {
    op = spv::Op::OpAny;
  } else if (name == "all") {
    op = spv::Op::OpAll;
  } else if (name == "dot") {
    op = spv::Op::OpDot;
  } else if (name == "is_inf") {
    op = spv::Op::OpIsInf;
  } else if (name == "is_nan") {
    op = spv::Op::OpIsNan;
  }
  if (op == spv::Op::OpNop) {
    error_ = "unable to determine operator for: " + name;
    return 0;
  }
  push_function_inst(op, params);

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
  if (from_type->IsI32() && to_type->IsF32()) {
    op = spv::Op::OpConvertSToF;
  } else if (from_type->IsU32() && to_type->IsF32()) {
    op = spv::Op::OpConvertUToF;
  } else if (from_type->IsF32() && to_type->IsI32()) {
    op = spv::Op::OpConvertFToS;
  } else if (from_type->IsF32() && to_type->IsU32()) {
    op = spv::Op::OpConvertFToU;
  } else if ((from_type->IsU32() && to_type->IsU32()) ||
             (from_type->IsI32() && to_type->IsI32()) ||
             (from_type->IsF32() && to_type->IsF32())) {
    op = spv::Op::OpCopyObject;
  } else if ((from_type->IsI32() && to_type->IsU32()) ||
             (from_type->IsU32() && to_type->IsI32())) {
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
    const ast::StatementList& true_body,
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
  push_function_inst(spv::Op::OpLabel, {true_block});
  if (!GenerateStatementList(true_body)) {
    return false;
  }
  // We only branch if the last element of the body didn't already branch.
  if (!LastIsTerminator(true_body)) {
    push_function_inst(spv::Op::OpBranch, {Operand::Int(merge_block_id)});
  }

  // Start the false block if needed
  if (false_block_id != merge_block_id) {
    push_function_inst(spv::Op::OpLabel, {Operand::Int(false_block_id)});

    auto* else_stmt = else_stmts[cur_else_idx].get();
    // Handle the else case by just outputting the statements.
    if (!else_stmt->HasCondition()) {
      if (!GenerateStatementList(else_stmt->body())) {
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
  push_function_inst(spv::Op::OpLabel, {merge_block});

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

  std::vector<Operand> params = {Operand::Int(cond_id),
                                 Operand::Int(default_block_id)};

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

    push_function_inst(spv::Op::OpLabel, {Operand::Int(case_ids[i])});
    if (!GenerateStatementList(item->body())) {
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
    push_function_inst(spv::Op::OpLabel, {Operand::Int(default_block_id)});
    push_function_inst(spv::Op::OpBranch, {Operand::Int(merge_block_id)});
  }

  merge_stack_.pop_back();

  push_function_inst(spv::Op::OpLabel, {Operand::Int(merge_block_id)});
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
  push_function_inst(spv::Op::OpLabel, {loop_header});

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
  push_function_inst(spv::Op::OpLabel, {body_block});
  if (!GenerateStatementList(stmt->body())) {
    return false;
  }

  // We only branch if the last element of the body didn't already branch.
  if (!LastIsTerminator(stmt->body())) {
    push_function_inst(spv::Op::OpBranch, {Operand::Int(continue_block_id)});
  }

  push_function_inst(spv::Op::OpLabel, {continue_block});
  if (!GenerateStatementList(stmt->continuing())) {
    return false;
  }
  push_function_inst(spv::Op::OpBranch, {Operand::Int(loop_header_id)});

  merge_stack_.pop_back();
  continue_stack_.pop_back();

  push_function_inst(spv::Op::OpLabel, {merge_block});

  return true;
}

bool Builder::GenerateStatementList(const ast::StatementList& list) {
  for (const auto& inst : list) {
    if (!GenerateStatement(inst.get())) {
      return false;
    }
  }
  return true;
}

bool Builder::GenerateStatement(ast::Statement* stmt) {
  if (stmt->IsAssign()) {
    return GenerateAssignStatement(stmt->AsAssign());
  }
  if (stmt->IsBreak()) {
    return GenerateBreakStatement(stmt->AsBreak());
  }
  if (stmt->IsContinue()) {
    return GenerateContinueStatement(stmt->AsContinue());
  }
  if (stmt->IsFallthrough()) {
    // Do nothing here, the fallthrough gets handled by the switch code.
    return true;
  }
  if (stmt->IsIf()) {
    return GenerateIfStatement(stmt->AsIf());
  }
  if (stmt->IsKill()) {
    return GenerateKillStatement(stmt->AsKill());
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

  error_ = "Unknown statement";
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
    auto len_id = GenerateU32Literal(ary->size());
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
    case ast::Builtin::kNumWorkgroups:
      return SpvBuiltInNumWorkgroups;
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

}  // namespace spirv
}  // namespace writer
}  // namespace tint
