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

#include <sstream>
#include <utility>

#include "spirv/unified1/spirv.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/binding_decoration.h"
#include "src/ast/bool_literal.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/constructor_expression.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/else_statement.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/int_literal.h"
#include "src/ast/location_decoration.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/set_decoration.h"
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/uint_literal.h"
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

}  // namespace

Builder::Builder() : scope_stack_({}) {}

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

  for (const auto& var : m.global_variables()) {
    if (!GenerateGlobalVariable(var.get())) {
      return false;
    }
  }

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

bool Builder::GenerateAssignStatement(ast::AssignmentStatement* assign) {
  auto lhs_id = GenerateExpression(assign->lhs());
  if (lhs_id == 0) {
    return false;
  }
  auto rhs_id = GenerateExpression(assign->rhs());
  if (rhs_id == 0) {
    return false;
  }

  GenerateStore(lhs_id, rhs_id);
  return true;
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

uint32_t Builder::GenerateExpression(ast::Expression* expr) {
  if (expr->IsBinary()) {
    return GenerateBinaryExpression(expr->AsBinary());
  }
  if (expr->IsConstructor()) {
    return GenerateConstructorExpression(expr->AsConstructor(), false);
  }
  if (expr->IsIdentifier()) {
    return GenerateIdentifierExpression(expr->AsIdentifier());
  }

  error_ = "unknown expression type";
  return 0;
}

uint32_t Builder::GenerateExpressionAndLoad(ast::Expression* expr) {
  auto id = GenerateExpression(expr);
  if (id == 0) {
    return false;
  }

  // Only need to load identifiers
  if (!expr->IsIdentifier()) {
    return id;
  }
  if (spirv_id_to_variable_.find(id) == spirv_id_to_variable_.end()) {
    error_ = "missing generated ID for variable";
    return 0;
  }
  auto var = spirv_id_to_variable_[id];
  if (var->is_const()) {
    return id;
  }

  auto type_id = GenerateTypeIfNeeded(expr->result_type());
  auto result = result_op();
  auto result_id = result.to_i();
  push_function_inst(spv::Op::OpLoad,
                     {Operand::Int(type_id), result, Operand::Int(id)});

  return result_id;
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

  push_function_var(
      {Operand::Int(type_id), result, Operand::Int(ConvertStorageClass(sc))});
  if (var->has_constructor()) {
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
  }

  push_type(spv::Op::OpVariable, std::move(ops));

  if (var->IsDecorated()) {
    for (const auto& deco : var->AsDecorated()->decorations()) {
      if (deco->IsBuiltin()) {
        push_debug(spv::Op::OpDecorate,
                   {Operand::Int(var_id), Operand::Int(SpvDecorationBuiltIn),
                    Operand::Int(ConvertBuiltin(deco->AsBuiltin()->value()))});
      } else if (deco->IsLocation()) {
        push_debug(spv::Op::OpDecorate,
                   {Operand::Int(var_id), Operand::Int(SpvDecorationLocation),
                    Operand::Int(deco->AsLocation()->value())});
      } else if (deco->IsBinding()) {
        push_debug(spv::Op::OpDecorate,
                   {Operand::Int(var_id), Operand::Int(SpvDecorationBinding),
                    Operand::Int(deco->AsBinding()->value())});
      } else if (deco->IsSet()) {
        push_debug(
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

uint32_t Builder::GenerateIdentifierExpression(
    ast::IdentifierExpression* expr) {
  // TODO(dsinclair): handle names with namespaces in them ...

  uint32_t val = 0;
  if (!scope_stack_.get(expr->name()[0], &val)) {
    error_ = "unable to find name for identifier: " + expr->name()[0];
    return 0;
  }

  return val;
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
    auto init = expr->AsTypeConstructor();
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
      auto id =
          GenerateConstructorExpression(e->AsConstructor(), is_global_init);
      if (id == 0) {
        return 0;
      }

      out << "_" << id;
      ops.push_back(Operand::Int(id));
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

  error_ = "unknown constructor expression";
  return 0;
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

uint32_t Builder::GenerateBinaryExpression(ast::BinaryExpression* expr) {
  auto lhs_id = GenerateExpressionAndLoad(expr->lhs());
  if (lhs_id == 0) {
    return 0;
  }
  auto rhs_id = GenerateExpressionAndLoad(expr->rhs());
  if (rhs_id == 0) {
    return 0;
  }

  auto result = result_op();
  auto result_id = result.to_i();

  auto type_id = GenerateTypeIfNeeded(expr->result_type());
  if (type_id == 0) {
    return 0;
  }

  // Handle int and float and the vectors of those types. Other types
  // should have been rejected by validation.
  auto lhs_type = expr->lhs()->result_type();
  bool lhs_is_float_or_vec =
      lhs_type->IsF32() ||
      (lhs_type->IsVector() && lhs_type->AsVector()->type()->IsF32());
  bool lhs_is_unsigned =
      lhs_type->IsU32() ||
      (lhs_type->IsVector() && lhs_type->AsVector()->type()->IsU32());

  spv::Op op = spv::Op::OpNop;
  if (expr->IsAnd()) {
    op = spv::Op::OpBitwiseAnd;
  } else if (expr->IsAdd()) {
    op = lhs_is_float_or_vec ? spv::Op::OpFAdd : spv::Op::OpIAdd;
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
  } else if (expr->IsNotEqual()) {
    op = lhs_is_float_or_vec ? spv::Op::OpFOrdNotEqual : spv::Op::OpINotEqual;
  } else if (expr->IsOr()) {
    op = spv::Op::OpBitwiseOr;
  } else if (expr->IsXor()) {
    op = spv::Op::OpBitwiseXor;
  } else {
    return 0;
  }

  push_function_inst(op, {Operand::Int(type_id), result, Operand::Int(lhs_id),
                          Operand::Int(rhs_id)});
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
  // TODO(dsinclair): The branch should be optional based on how the
  // StatementList ended ...
  push_function_inst(spv::Op::OpBranch, {Operand::Int(merge_block_id)});

  // Start the false block if needed
  if (false_block_id != merge_block_id) {
    push_function_inst(spv::Op::OpLabel, {Operand::Int(false_block_id)});

    auto* else_stmt = else_stmts[cur_else_idx].get();
    // Handle the else case by just outputting the statements.
    if (!else_stmt->HasCondition()) {
      if (!GenerateStatementList(else_stmt->body())) {
        return false;
      }
      // TODO(dsinclair): The branch should be optional based on how the
      // StatementList ended ...
      push_function_inst(spv::Op::OpBranch, {Operand::Int(merge_block_id)});
    } else {
      if (!GenerateConditionalBlock(else_stmt->condition(), else_stmt->body(),
                                    cur_else_idx + 1, else_stmts)) {
        return false;
      }
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
  if (stmt->IsIf()) {
    return GenerateIfStatement(stmt->AsIf());
  }
  if (stmt->IsReturn()) {
    return GenerateReturnStatement(stmt->AsReturn());
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
