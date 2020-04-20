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

#include "src/reader/spirv/function.h"

#include <utility>

#include "source/opt/basic_block.h"
#include "source/opt/function.h"
#include "source/opt/instruction.h"
#include "source/opt/module.h"
#include "src/ast/as_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/storage_class.h"
#include "src/ast/uint_literal.h"
#include "src/ast/unary_op.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/reader/spirv/fail_stream.h"
#include "src/reader/spirv/parser_impl.h"

namespace tint {
namespace reader {
namespace spirv {

namespace {

// Gets the AST unary opcode for the given SPIR-V opcode, if any
// @param opcode SPIR-V opcode
// @param ast_unary_op return parameter
// @returns true if it was a unary operation
bool GetUnaryOp(SpvOp opcode, ast::UnaryOp* ast_unary_op) {
  switch (opcode) {
    case SpvOpSNegate:
    case SpvOpFNegate:
      *ast_unary_op = ast::UnaryOp::kNegation;
      return true;
    case SpvOpLogicalNot:
    case SpvOpNot:
      *ast_unary_op = ast::UnaryOp::kNot;
      return true;
    default:
      break;
  }
  return false;
}

// Converts a SPIR-V opcode to its corresponding AST binary opcode, if any
// @param opcode SPIR-V opcode
// @returns the AST binary op for the given opcode, or kNone
ast::BinaryOp ConvertBinaryOp(SpvOp opcode) {
  switch (opcode) {
    case SpvOpIAdd:
    case SpvOpFAdd:
      return ast::BinaryOp::kAdd;
    case SpvOpISub:
    case SpvOpFSub:
      return ast::BinaryOp::kSubtract;
    case SpvOpIMul:
    case SpvOpFMul:
      return ast::BinaryOp::kMultiply;
    case SpvOpUDiv:
    case SpvOpSDiv:
    case SpvOpFDiv:
      return ast::BinaryOp::kDivide;
    case SpvOpUMod:
    case SpvOpSMod:
    case SpvOpFMod:
      return ast::BinaryOp::kModulo;
    case SpvOpShiftLeftLogical:
      return ast::BinaryOp::kShiftLeft;
    case SpvOpShiftRightLogical:
      return ast::BinaryOp::kShiftRight;
    case SpvOpShiftRightArithmetic:
      return ast::BinaryOp::kShiftRightArith;
    case SpvOpIEqual:
    case SpvOpFOrdEqual:
      return ast::BinaryOp::kEqual;
    case SpvOpINotEqual:
    case SpvOpFOrdNotEqual:
      return ast::BinaryOp::kNotEqual;
    case SpvOpBitwiseAnd:
      return ast::BinaryOp::kAnd;
    case SpvOpBitwiseOr:
      return ast::BinaryOp::kOr;
    case SpvOpBitwiseXor:
      return ast::BinaryOp::kXor;
    case SpvOpLogicalAnd:
      return ast::BinaryOp::kLogicalAnd;
    case SpvOpLogicalOr:
      return ast::BinaryOp::kLogicalOr;
    default:
      break;
  }
  // It's not clear what OpSMod should map to.
  // https://bugs.chromium.org/p/tint/issues/detail?id=52
  return ast::BinaryOp::kNone;
}

}  // namespace

FunctionEmitter::FunctionEmitter(ParserImpl* pi,
                                 const spvtools::opt::Function& function)
    : parser_impl_(*pi),
      ast_module_(pi->get_module()),
      ir_context_(*(pi->ir_context())),
      def_use_mgr_(ir_context_.get_def_use_mgr()),
      constant_mgr_(ir_context_.get_constant_mgr()),
      type_mgr_(ir_context_.get_type_mgr()),
      fail_stream_(pi->fail_stream()),
      namer_(pi->namer()),
      function_(function) {}

FunctionEmitter::~FunctionEmitter() = default;

bool FunctionEmitter::Emit() {
  if (failed()) {
    return false;
  }
  // We only care about functions with bodies.
  if (function_.cbegin() == function_.cend()) {
    return true;
  }

  if (!EmitFunctionDeclaration()) {
    return false;
  }

  if (!EmitBody()) {
    return false;
  }

  // Set the body of the AST function node.
  parser_impl_.get_module().functions().back()->set_body(std::move(ast_body_));

  return success();
}

bool FunctionEmitter::EmitFunctionDeclaration() {
  if (failed()) {
    return false;
  }

  const auto name = namer_.Name(function_.result_id());
  // Surprisingly, the "type id" on an OpFunction is the result type of the
  // function, not the type of the function.  This is the one exceptional case
  // in SPIR-V where the type ID is not the type of the result ID.
  auto* ret_ty = parser_impl_.ConvertType(function_.type_id());
  if (failed()) {
    return false;
  }
  if (ret_ty == nullptr) {
    return Fail()
           << "internal error: unregistered return type for function with ID "
           << function_.result_id();
  }

  ast::VariableList ast_params;
  function_.ForEachParam(
      [this, &ast_params](const spvtools::opt::Instruction* param) {
        auto* ast_type = parser_impl_.ConvertType(param->type_id());
        if (ast_type != nullptr) {
          ast_params.emplace_back(parser_impl_.MakeVariable(
              param->result_id(), ast::StorageClass::kNone, ast_type));
        } else {
          // We've already logged an error and emitted a diagnostic. Do nothing
          // here.
        }
      });
  if (failed()) {
    return false;
  }

  auto ast_fn =
      std::make_unique<ast::Function>(name, std::move(ast_params), ret_ty);
  ast_module_.AddFunction(std::move(ast_fn));

  return success();
}

ast::type::Type* FunctionEmitter::GetVariableStoreType(
    const spvtools::opt::Instruction& var_decl_inst) {
  const auto type_id = var_decl_inst.type_id();
  auto* var_ref_type = type_mgr_->GetType(type_id);
  if (!var_ref_type) {
    Fail() << "internal error: variable type id " << type_id
           << " has no registered type";
    return nullptr;
  }
  auto* var_ref_ptr_type = var_ref_type->AsPointer();
  if (!var_ref_ptr_type) {
    Fail() << "internal error: variable type id " << type_id
           << " is not a pointer type";
    return nullptr;
  }
  auto var_store_type_id = type_mgr_->GetId(var_ref_ptr_type->pointee_type());
  return parser_impl_.ConvertType(var_store_type_id);
}

bool FunctionEmitter::EmitBody() {
  if (!EmitFunctionVariables()) {
    return false;
  }
  if (!EmitFunctionBodyStatements()) {
    return false;
  }
  return success();
}

bool FunctionEmitter::EmitFunctionVariables() {
  if (failed()) {
    return false;
  }
  for (auto& inst : *function_.entry()) {
    if (inst.opcode() != SpvOpVariable) {
      continue;
    }
    auto* var_store_type = GetVariableStoreType(inst);
    if (failed()) {
      return false;
    }
    auto var = parser_impl_.MakeVariable(
        inst.result_id(), ast::StorageClass::kFunction, var_store_type);
    if (inst.NumInOperands() > 1) {
      // SPIR-V initializers are always constants.
      // (OpenCL also allows the ID of an OpVariable, but we don't handle that
      // here.)
      var->set_constructor(
          parser_impl_.MakeConstantExpression(inst.GetSingleWordInOperand(1))
              .expr);
    }
    // TODO(dneto): Add the initializer via Variable::set_constructor.
    auto var_decl_stmt =
        std::make_unique<ast::VariableDeclStatement>(std::move(var));
    ast_body_.emplace_back(std::move(var_decl_stmt));
    // Save this as an already-named value.
    identifier_values_.insert(inst.result_id());
  }
  return success();
}

TypedExpression FunctionEmitter::MakeExpression(uint32_t id) {
  if (failed()) {
    return {};
  }
  if (identifier_values_.count(id)) {
    return TypedExpression(
        parser_impl_.ConvertType(def_use_mgr_->GetDef(id)->type_id()),
        std::make_unique<ast::IdentifierExpression>(namer_.Name(id)));
  }
  if (singly_used_values_.count(id)) {
    auto expr = std::move(singly_used_values_[id]);
    singly_used_values_.erase(id);
    return expr;
  }
  const auto* spirv_constant = constant_mgr_->FindDeclaredConstant(id);
  if (spirv_constant) {
    return parser_impl_.MakeConstantExpression(id);
  }
  const auto* inst = def_use_mgr_->GetDef(id);
  if (inst == nullptr) {
    Fail() << "ID " << id << " does not have a defining SPIR-V instruction";
    return {};
  }
  switch (inst->opcode()) {
    case SpvOpVariable:
      // This occurs for module-scope variables.
      return TypedExpression(parser_impl_.ConvertType(inst->type_id()),
                             std::make_unique<ast::IdentifierExpression>(
                                 namer_.Name(inst->result_id())));
    default:
      break;
  }
  Fail() << "unhandled expression for ID " << id << "\n" << inst->PrettyPrint();
  return {};
}

bool FunctionEmitter::EmitFunctionBodyStatements() {
  // TODO(dneto): For now, emit only regular statements in the entry block.
  // We'll use assignments as markers in the tests, to be able to tell where
  // code is placed in control flow. First prove that we can emit assignments.
  return EmitStatementsInBasicBlock(*function_.entry());
}

bool FunctionEmitter::EmitStatementsInBasicBlock(
    const spvtools::opt::BasicBlock& bb) {
  const auto* terminator = bb.terminator();
  const auto* merge = bb.GetMergeInst();  // Might be nullptr
  // Emit regular statements.
  for (auto& inst : bb) {
    if (&inst == terminator || &inst == merge || inst.opcode() == SpvOpLabel ||
        inst.opcode() == SpvOpVariable) {
      continue;
    }
    if (!EmitStatement(inst)) {
      return false;
    }
  }
  // TODO(dneto): Handle the terminator
  return true;
}

bool FunctionEmitter::EmitConstDefinition(
    const spvtools::opt::Instruction& inst,
    TypedExpression ast_expr) {
  if (!ast_expr.expr) {
    return false;
  }
  auto ast_const =
      parser_impl_.MakeVariable(inst.result_id(), ast::StorageClass::kNone,
                                parser_impl_.ConvertType(inst.type_id()));
  if (!ast_const) {
    return false;
  }
  ast_const->set_constructor(std::move(ast_expr.expr));
  ast_const->set_is_const(true);
  ast_body_.emplace_back(
      std::make_unique<ast::VariableDeclStatement>(std::move(ast_const)));
  // Save this as an already-named value.
  identifier_values_.insert(inst.result_id());
  return success();
}

bool FunctionEmitter::EmitStatement(const spvtools::opt::Instruction& inst) {
  // Handle combinatorial instructions first.
  auto combinatorial_expr = MaybeEmitCombinatorialValue(inst);
  if (combinatorial_expr.expr != nullptr) {
    if (def_use_mgr_->NumUses(&inst) == 1) {
      // If it's used once, then defer emitting the expression until it's used.
      // Any supporting statements have already been emitted.
      singly_used_values_.insert(
          std::make_pair(inst.result_id(), std::move(combinatorial_expr)));
      return success();
    }
    // Otherwise, generate a const definition for it now and later use
    // the const's name at the uses of the value.
    return EmitConstDefinition(inst, std::move(combinatorial_expr));
  }
  if (failed()) {
    return false;
  }

  switch (inst.opcode()) {
    case SpvOpStore: {
      // TODO(dneto): Order of evaluation?
      auto lhs = MakeExpression(inst.GetSingleWordInOperand(0));
      auto rhs = MakeExpression(inst.GetSingleWordInOperand(1));
      ast_body_.emplace_back(std::make_unique<ast::AssignmentStatement>(
          std::move(lhs.expr), std::move(rhs.expr)));
      return success();
    }
    case SpvOpLoad:
      // Memory accesses must be issued in SPIR-V program order.
      // So represent a load by a new const definition.
      return EmitConstDefinition(
          inst, MakeExpression(inst.GetSingleWordInOperand(0)));
    case SpvOpFunctionCall:
      // TODO(dneto): Fill this out.  Make this pass, for existing tests
      return success();
    default:
      break;
  }
  return Fail() << "unhandled instruction with opcode " << inst.opcode();
}

TypedExpression FunctionEmitter::MaybeEmitCombinatorialValue(
    const spvtools::opt::Instruction& inst) {
  if (inst.result_id() == 0) {
    return {};
  }

  // TODO(dneto): Fill in the following cases.

  auto operand = [this, &inst](uint32_t operand_index) {
    auto expr =
        this->MakeExpression(inst.GetSingleWordInOperand(operand_index));
    return parser_impl_.RectifyOperandSignedness(inst.opcode(),
                                                 std::move(expr));
  };

  ast::type::Type* ast_type =
      inst.type_id() != 0 ? parser_impl_.ConvertType(inst.type_id()) : nullptr;

  auto binary_op = ConvertBinaryOp(inst.opcode());
  if (binary_op != ast::BinaryOp::kNone) {
    auto arg0 = operand(0);
    auto arg1 = operand(1);
    auto binary_expr = std::make_unique<ast::BinaryExpression>(
        binary_op, std::move(arg0.expr), std::move(arg1.expr));
    auto* forced_result_ty =
        parser_impl_.ForcedResultType(inst.opcode(), arg0.type);
    if (forced_result_ty && forced_result_ty != ast_type) {
      return {ast_type, std::make_unique<ast::AsExpression>(
                            ast_type, std::move(binary_expr))};
    }
    return {ast_type, std::move(binary_expr)};
  }

  auto unary_op = ast::UnaryOp::kNegation;
  if (GetUnaryOp(inst.opcode(), &unary_op)) {
    auto arg0 = operand(0);
    auto unary_expr = std::make_unique<ast::UnaryOpExpression>(
        unary_op, std::move(arg0.expr));
    auto* forced_result_ty =
        parser_impl_.ForcedResultType(inst.opcode(), arg0.type);
    if (forced_result_ty && forced_result_ty != ast_type) {
      return {ast_type, std::make_unique<ast::AsExpression>(
                            ast_type, std::move(unary_expr))};
    }
    return {ast_type, std::move(unary_expr)};
  }

  // builtin readonly function
  // glsl.std.450 readonly function

  // Instructions:
  //    OpCopyObject
  //    OpUndef
  //    OpBitcast
  //    OpSatConvertSToU
  //    OpSatConvertUToS
  //    OpSatConvertFToS
  //    OpSatConvertFToU
  //    OpSatConvertSToF
  //    OpSatConvertUToF
  //    OpUConvert
  //    OpSConvert
  //    OpFConvert
  //    OpConvertPtrToU // Not in WebGPU
  //    OpConvertUToPtr // Not in WebGPU
  //    OpPtrCastToGeneric // Not in Vulkan
  //    OpGenericCastToPtr // Not in Vulkan
  //    OpGenericCastToPtrExplicit // Not in Vulkan
  //
  //    OpAccessChain
  //    OpInBoundsAccessChain
  //    OpArrayLength
  //    OpVectorExtractDynamic
  //    OpVectorInsertDynamic
  //    OpCompositeExtract
  //    OpCompositeInsert

  return {};
}

}  // namespace spirv
}  // namespace reader
}  // namespace tint
