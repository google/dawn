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

#include "src/type_determiner.h"

#include <memory>
#include <utility>
#include <vector>

#include "src/ast/array_accessor_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/block_statement.h"
#include "src/ast/break_statement.h"
#include "src/ast/call_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/case_statement.h"
#include "src/ast/continue_statement.h"
#include "src/ast/discard_statement.h"
#include "src/ast/else_statement.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/loop_statement.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/switch_statement.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/diagnostic/formatter.h"
#include "src/program_builder.h"
#include "src/semantic/call.h"
#include "src/semantic/expression.h"
#include "src/semantic/function.h"
#include "src/semantic/intrinsic.h"
#include "src/semantic/member_accessor_expression.h"
#include "src/semantic/statement.h"
#include "src/semantic/variable.h"
#include "src/type/array_type.h"
#include "src/type/bool_type.h"
#include "src/type/depth_texture_type.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/matrix_type.h"
#include "src/type/multisampled_texture_type.h"
#include "src/type/pointer_type.h"
#include "src/type/sampled_texture_type.h"
#include "src/type/storage_texture_type.h"
#include "src/type/struct_type.h"
#include "src/type/texture_type.h"
#include "src/type/u32_type.h"
#include "src/type/vector_type.h"
#include "src/type/void_type.h"

namespace tint {
namespace {

using IntrinsicType = tint::semantic::IntrinsicType;

// Helper class that temporarily assigns a value to a reference for the scope of
// the object. Once the ScopedAssignment is destructed, the original value is
// restored.
template <typename T>
class ScopedAssignment {
 public:
  ScopedAssignment(T& ref, T val) : ref_(ref) {
    old_value_ = ref;
    ref = val;
  }
  ~ScopedAssignment() { ref_ = old_value_; }

 private:
  T& ref_;
  T old_value_;
};

}  // namespace

TypeDeterminer::TypeDeterminer(ProgramBuilder* builder)
    : builder_(builder), intrinsic_table_(IntrinsicTable::Create()) {}

TypeDeterminer::~TypeDeterminer() = default;

void TypeDeterminer::set_referenced_from_function_if_needed(VariableInfo* var,
                                                            bool local) {
  if (current_function_ == nullptr) {
    return;
  }
  if (var->storage_class == ast::StorageClass::kNone ||
      var->storage_class == ast::StorageClass::kFunction) {
    return;
  }

  current_function_->referenced_module_vars.add(var);
  if (local) {
    current_function_->local_referenced_module_vars.add(var);
  }
}

bool TypeDeterminer::Determine() {
  bool result = DetermineInternal();

  // Even if resolving failed, create all the semantic nodes for information we
  // did generate.
  CreateSemanticNodes();

  return result;
}

bool TypeDeterminer::DetermineInternal() {
  for (auto* var : builder_->AST().GlobalVariables()) {
    variable_stack_.set_global(var->symbol(), CreateVariableInfo(var));

    if (var->has_constructor()) {
      if (!DetermineResultType(var->constructor())) {
        return false;
      }
    }
  }

  if (!DetermineFunctions(builder_->AST().Functions())) {
    return false;
  }

  // Walk over the caller to callee information and update functions with
  // which entry points call those functions.
  for (auto* func : builder_->AST().Functions()) {
    if (!func->IsEntryPoint()) {
      continue;
    }
    for (const auto& callee : caller_to_callee_[func->symbol()]) {
      set_entry_points(callee, func->symbol());
    }
  }

  return true;
}

void TypeDeterminer::set_entry_points(const Symbol& fn_sym, Symbol ep_sym) {
  auto* info = symbol_to_function_.at(fn_sym);
  info->ancestor_entry_points.add(ep_sym);

  for (const auto& callee : caller_to_callee_[fn_sym]) {
    set_entry_points(callee, ep_sym);
  }
}

bool TypeDeterminer::DetermineFunctions(const ast::FunctionList& funcs) {
  for (auto* func : funcs) {
    if (!DetermineFunction(func)) {
      return false;
    }
  }
  return true;
}

bool TypeDeterminer::DetermineFunction(ast::Function* func) {
  auto* func_info = function_infos_.Create<FunctionInfo>(func);
  symbol_to_function_[func->symbol()] = func_info;
  function_to_info_.emplace(func, func_info);

  ScopedAssignment<FunctionInfo*> sa(current_function_, func_info);

  variable_stack_.push_scope();
  for (auto* param : func->params()) {
    variable_stack_.set(param->symbol(), CreateVariableInfo(param));
  }

  if (!DetermineStatements(func->body())) {
    return false;
  }
  variable_stack_.pop_scope();

  return true;
}

bool TypeDeterminer::DetermineStatements(const ast::BlockStatement* stmts) {
  for (auto* stmt : *stmts) {
    if (!DetermineVariableStorageClass(stmt)) {
      return false;
    }

    if (!DetermineResultType(stmt)) {
      return false;
    }
  }
  return true;
}

bool TypeDeterminer::DetermineVariableStorageClass(ast::Statement* stmt) {
  auto* var_decl = stmt->As<ast::VariableDeclStatement>();
  if (var_decl == nullptr) {
    return true;
  }

  auto* var = var_decl->variable();

  auto* info = CreateVariableInfo(var);
  variable_to_info_.emplace(var, info);

  // Nothing to do for const
  if (var->is_const()) {
    return true;
  }

  if (info->storage_class == ast::StorageClass::kFunction) {
    return true;
  }

  if (info->storage_class != ast::StorageClass::kNone) {
    diagnostics_.add_error("function variable has a non-function storage class",
                           stmt->source());
    return false;
  }

  info->storage_class = ast::StorageClass::kFunction;
  return true;
}

bool TypeDeterminer::DetermineResultType(ast::Statement* stmt) {
  auto* sem_statement = builder_->create<semantic::Statement>(stmt);

  ScopedAssignment<semantic::Statement*> sa(current_statement_, sem_statement);

  if (auto* a = stmt->As<ast::AssignmentStatement>()) {
    return DetermineResultType(a->lhs()) && DetermineResultType(a->rhs());
  }
  if (auto* b = stmt->As<ast::BlockStatement>()) {
    return DetermineStatements(b);
  }
  if (stmt->Is<ast::BreakStatement>()) {
    return true;
  }
  if (auto* c = stmt->As<ast::CallStatement>()) {
    return DetermineResultType(c->expr());
  }
  if (auto* c = stmt->As<ast::CaseStatement>()) {
    return DetermineStatements(c->body());
  }
  if (stmt->Is<ast::ContinueStatement>()) {
    return true;
  }
  if (stmt->Is<ast::DiscardStatement>()) {
    return true;
  }
  if (auto* e = stmt->As<ast::ElseStatement>()) {
    return DetermineResultType(e->condition()) &&
           DetermineStatements(e->body());
  }
  if (stmt->Is<ast::FallthroughStatement>()) {
    return true;
  }
  if (auto* i = stmt->As<ast::IfStatement>()) {
    if (!DetermineResultType(i->condition()) ||
        !DetermineStatements(i->body())) {
      return false;
    }

    for (auto* else_stmt : i->else_statements()) {
      if (!DetermineResultType(else_stmt)) {
        return false;
      }
    }
    return true;
  }
  if (auto* l = stmt->As<ast::LoopStatement>()) {
    return DetermineStatements(l->body()) &&
           DetermineStatements(l->continuing());
  }
  if (auto* r = stmt->As<ast::ReturnStatement>()) {
    return DetermineResultType(r->value());
  }
  if (auto* s = stmt->As<ast::SwitchStatement>()) {
    if (!DetermineResultType(s->condition())) {
      return false;
    }
    for (auto* case_stmt : s->body()) {
      if (!DetermineResultType(case_stmt)) {
        return false;
      }
    }
    return true;
  }
  if (auto* v = stmt->As<ast::VariableDeclStatement>()) {
    variable_stack_.set(v->variable()->symbol(),
                        variable_to_info_.at(v->variable()));
    return DetermineResultType(v->variable()->constructor());
  }

  diagnostics_.add_error(
      "unknown statement type for type determination: " + builder_->str(stmt),
      stmt->source());
  return false;
}

bool TypeDeterminer::DetermineResultType(const ast::ExpressionList& list) {
  for (auto* expr : list) {
    if (!DetermineResultType(expr)) {
      return false;
    }
  }
  return true;
}

bool TypeDeterminer::DetermineResultType(ast::Expression* expr) {
  // This is blindly called above, so in some cases the expression won't exist.
  if (!expr) {
    return true;
  }

  if (TypeOf(expr)) {
    return true;  // Already resolved
  }

  if (auto* a = expr->As<ast::ArrayAccessorExpression>()) {
    return DetermineArrayAccessor(a);
  }
  if (auto* b = expr->As<ast::BinaryExpression>()) {
    return DetermineBinary(b);
  }
  if (auto* b = expr->As<ast::BitcastExpression>()) {
    return DetermineBitcast(b);
  }
  if (auto* c = expr->As<ast::CallExpression>()) {
    return DetermineCall(c);
  }
  if (auto* c = expr->As<ast::ConstructorExpression>()) {
    return DetermineConstructor(c);
  }
  if (auto* i = expr->As<ast::IdentifierExpression>()) {
    return DetermineIdentifier(i);
  }
  if (auto* m = expr->As<ast::MemberAccessorExpression>()) {
    return DetermineMemberAccessor(m);
  }
  if (auto* u = expr->As<ast::UnaryOpExpression>()) {
    return DetermineUnaryOp(u);
  }

  diagnostics_.add_error("unknown expression for type determination",
                         expr->source());
  return false;
}

bool TypeDeterminer::DetermineArrayAccessor(
    ast::ArrayAccessorExpression* expr) {
  if (!DetermineResultType(expr->array())) {
    return false;
  }
  if (!DetermineResultType(expr->idx_expr())) {
    return false;
  }

  auto* res = TypeOf(expr->array());
  auto* parent_type = res->UnwrapAll();
  type::Type* ret = nullptr;
  if (auto* arr = parent_type->As<type::Array>()) {
    ret = arr->type();
  } else if (auto* vec = parent_type->As<type::Vector>()) {
    ret = vec->type();
  } else if (auto* mat = parent_type->As<type::Matrix>()) {
    ret = builder_->create<type::Vector>(mat->type(), mat->rows());
  } else {
    diagnostics_.add_error("invalid parent type (" + parent_type->type_name() +
                               ") in array accessor",
                           expr->source());
    return false;
  }

  // If we're extracting from a pointer, we return a pointer.
  if (auto* ptr = res->As<type::Pointer>()) {
    ret = builder_->create<type::Pointer>(ret, ptr->storage_class());
  } else if (auto* arr = parent_type->As<type::Array>()) {
    if (!arr->type()->is_scalar()) {
      // If we extract a non-scalar from an array then we also get a pointer. We
      // will generate a Function storage class variable to store this
      // into.
      ret = builder_->create<type::Pointer>(ret, ast::StorageClass::kFunction);
    }
  }
  SetType(expr, ret);

  return true;
}

bool TypeDeterminer::DetermineBitcast(ast::BitcastExpression* expr) {
  if (!DetermineResultType(expr->expr())) {
    return false;
  }
  SetType(expr, expr->type());
  return true;
}

bool TypeDeterminer::DetermineCall(ast::CallExpression* call) {
  if (!DetermineResultType(call->params())) {
    return false;
  }

  // The expression has to be an identifier as you can't store function pointers
  // but, if it isn't we'll just use the normal result determination to be on
  // the safe side.
  auto* ident = call->func()->As<ast::IdentifierExpression>();
  if (!ident) {
    diagnostics_.add_error("call target is not an identifier", call->source());
    return false;
  }

  auto name = builder_->Symbols().NameFor(ident->symbol());

  auto intrinsic_type = MatchIntrinsicType(name);
  if (intrinsic_type != IntrinsicType::kNone) {
    if (!DetermineIntrinsicCall(call, intrinsic_type)) {
      return false;
    }
  } else {
    if (current_function_) {
      caller_to_callee_[current_function_->declaration->symbol()].push_back(
          ident->symbol());

      auto callee_func_it = symbol_to_function_.find(ident->symbol());
      if (callee_func_it == symbol_to_function_.end()) {
        diagnostics_.add_error(
            "v-0006: unable to find called function: " + name, call->source());
        return false;
      }
      auto* callee_func = callee_func_it->second;

      // We inherit any referenced variables from the callee.
      for (auto* var : callee_func->referenced_module_vars) {
        set_referenced_from_function_if_needed(var, false);
      }
    }

    auto iter = symbol_to_function_.find(ident->symbol());
    if (iter == symbol_to_function_.end()) {
      diagnostics_.add_error(
          "v-0005: function must be declared before use: '" + name + "'",
          call->source());
      return false;
    }

    auto* function = iter->second;
    function_calls_.emplace(call,
                            FunctionCallInfo{function, current_statement_});
    SetType(call, function->declaration->return_type());
  }

  return true;
}

bool TypeDeterminer::DetermineIntrinsicCall(
    ast::CallExpression* call,
    semantic::IntrinsicType intrinsic_type) {
  std::vector<type::Type*> arg_tys;
  arg_tys.reserve(call->params().size());
  for (auto* expr : call->params()) {
    arg_tys.emplace_back(TypeOf(expr));
  }

  auto result = intrinsic_table_->Lookup(*builder_, intrinsic_type, arg_tys,
                                         call->source());
  if (!result.intrinsic) {
    // Intrinsic lookup failed.
    diagnostics_.add(result.diagnostics);

    // TODO(bclayton): https://crbug.com/tint/487
    // The Validator expects intrinsic signature mismatches to still produce
    // type information. The rules for what the Validator expects are rather
    // bespoke. Try to match what the Validator expects. As the Validator's
    // checks on intrinsics is now almost entirely covered by the
    // IntrinsicTable, we should remove the Validator checks on intrinsic
    // signatures and remove these hacks.
    semantic::ParameterList parameters;
    parameters.reserve(arg_tys.size());
    for (auto* arg : arg_tys) {
      parameters.emplace_back(semantic::Parameter{arg});
    }
    type::Type* ret_ty = nullptr;
    switch (intrinsic_type) {
      case IntrinsicType::kCross:
        ret_ty = builder_->ty.vec3<ProgramBuilder::f32>();
        break;
      case IntrinsicType::kDeterminant:
        ret_ty = builder_->create<type::F32>();
        break;
      case IntrinsicType::kArrayLength:
        ret_ty = builder_->create<type::U32>();
        break;
      default:
        ret_ty = arg_tys.empty() ? builder_->ty.void_() : arg_tys[0];
        break;
    }
    auto* intrinsic = builder_->create<semantic::Intrinsic>(intrinsic_type,
                                                            ret_ty, parameters);
    builder_->Sem().Add(call, builder_->create<semantic::Call>(
                                  call, intrinsic, current_statement_));
    SetType(call, ret_ty);
    return false;
  }

  builder_->Sem().Add(call, builder_->create<semantic::Call>(
                                call, result.intrinsic, current_statement_));
  SetType(call, result.intrinsic->ReturnType());
  return true;
}

bool TypeDeterminer::DetermineConstructor(ast::ConstructorExpression* expr) {
  if (auto* ty = expr->As<ast::TypeConstructorExpression>()) {
    for (auto* value : ty->values()) {
      if (!DetermineResultType(value)) {
        return false;
      }
    }
    SetType(expr, ty->type());
  } else {
    SetType(expr,
            expr->As<ast::ScalarConstructorExpression>()->literal()->type());
  }
  return true;
}

bool TypeDeterminer::DetermineIdentifier(ast::IdentifierExpression* expr) {
  auto symbol = expr->symbol();
  VariableInfo* var;
  if (variable_stack_.get(symbol, &var)) {
    // A constant is the type, but a variable is always a pointer so synthesize
    // the pointer around the variable type.
    if (var->declaration->is_const()) {
      SetType(expr, var->declaration->type());
    } else if (var->declaration->type()->Is<type::Pointer>()) {
      SetType(expr, var->declaration->type());
    } else {
      SetType(expr, builder_->create<type::Pointer>(var->declaration->type(),
                                                    var->storage_class));
    }

    var->users.push_back(expr);
    set_referenced_from_function_if_needed(var, true);
    return true;
  }

  auto iter = symbol_to_function_.find(symbol);
  if (iter != symbol_to_function_.end()) {
    diagnostics_.add_error("missing '(' for function call",
                           expr->source().End());
    return false;
  }

  std::string name = builder_->Symbols().NameFor(symbol);
  if (MatchIntrinsicType(name) != IntrinsicType::kNone) {
    diagnostics_.add_error("missing '(' for intrinsic call",
                           expr->source().End());
    return false;
  }

  diagnostics_.add_error(
      "v-0006: identifier must be declared before use: " + name,
      expr->source());
  return false;
}

IntrinsicType TypeDeterminer::MatchIntrinsicType(const std::string& name) {
  if (name == "abs") {
    return IntrinsicType::kAbs;
  } else if (name == "acos") {
    return IntrinsicType::kAcos;
  } else if (name == "all") {
    return IntrinsicType::kAll;
  } else if (name == "any") {
    return IntrinsicType::kAny;
  } else if (name == "arrayLength") {
    return IntrinsicType::kArrayLength;
  } else if (name == "asin") {
    return IntrinsicType::kAsin;
  } else if (name == "atan") {
    return IntrinsicType::kAtan;
  } else if (name == "atan2") {
    return IntrinsicType::kAtan2;
  } else if (name == "ceil") {
    return IntrinsicType::kCeil;
  } else if (name == "clamp") {
    return IntrinsicType::kClamp;
  } else if (name == "cos") {
    return IntrinsicType::kCos;
  } else if (name == "cosh") {
    return IntrinsicType::kCosh;
  } else if (name == "countOneBits") {
    return IntrinsicType::kCountOneBits;
  } else if (name == "cross") {
    return IntrinsicType::kCross;
  } else if (name == "determinant") {
    return IntrinsicType::kDeterminant;
  } else if (name == "distance") {
    return IntrinsicType::kDistance;
  } else if (name == "dot") {
    return IntrinsicType::kDot;
  } else if (name == "dpdx") {
    return IntrinsicType::kDpdx;
  } else if (name == "dpdxCoarse") {
    return IntrinsicType::kDpdxCoarse;
  } else if (name == "dpdxFine") {
    return IntrinsicType::kDpdxFine;
  } else if (name == "dpdy") {
    return IntrinsicType::kDpdy;
  } else if (name == "dpdyCoarse") {
    return IntrinsicType::kDpdyCoarse;
  } else if (name == "dpdyFine") {
    return IntrinsicType::kDpdyFine;
  } else if (name == "exp") {
    return IntrinsicType::kExp;
  } else if (name == "exp2") {
    return IntrinsicType::kExp2;
  } else if (name == "faceForward") {
    return IntrinsicType::kFaceForward;
  } else if (name == "floor") {
    return IntrinsicType::kFloor;
  } else if (name == "fma") {
    return IntrinsicType::kFma;
  } else if (name == "fract") {
    return IntrinsicType::kFract;
  } else if (name == "frexp") {
    return IntrinsicType::kFrexp;
  } else if (name == "fwidth") {
    return IntrinsicType::kFwidth;
  } else if (name == "fwidthCoarse") {
    return IntrinsicType::kFwidthCoarse;
  } else if (name == "fwidthFine") {
    return IntrinsicType::kFwidthFine;
  } else if (name == "inverseSqrt") {
    return IntrinsicType::kInverseSqrt;
  } else if (name == "isFinite") {
    return IntrinsicType::kIsFinite;
  } else if (name == "isInf") {
    return IntrinsicType::kIsInf;
  } else if (name == "isNan") {
    return IntrinsicType::kIsNan;
  } else if (name == "isNormal") {
    return IntrinsicType::kIsNormal;
  } else if (name == "ldexp") {
    return IntrinsicType::kLdexp;
  } else if (name == "length") {
    return IntrinsicType::kLength;
  } else if (name == "log") {
    return IntrinsicType::kLog;
  } else if (name == "log2") {
    return IntrinsicType::kLog2;
  } else if (name == "max") {
    return IntrinsicType::kMax;
  } else if (name == "min") {
    return IntrinsicType::kMin;
  } else if (name == "mix") {
    return IntrinsicType::kMix;
  } else if (name == "modf") {
    return IntrinsicType::kModf;
  } else if (name == "normalize") {
    return IntrinsicType::kNormalize;
  } else if (name == "pack4x8snorm") {
    return IntrinsicType::kPack4x8Snorm;
  } else if (name == "pack4x8unorm") {
    return IntrinsicType::kPack4x8Unorm;
  } else if (name == "pack2x16snorm") {
    return IntrinsicType::kPack2x16Snorm;
  } else if (name == "pack2x16unorm") {
    return IntrinsicType::kPack2x16Unorm;
  } else if (name == "pack2x16float") {
    return IntrinsicType::kPack2x16Float;
  } else if (name == "pow") {
    return IntrinsicType::kPow;
  } else if (name == "reflect") {
    return IntrinsicType::kReflect;
  } else if (name == "reverseBits") {
    return IntrinsicType::kReverseBits;
  } else if (name == "round") {
    return IntrinsicType::kRound;
  } else if (name == "select") {
    return IntrinsicType::kSelect;
  } else if (name == "sign") {
    return IntrinsicType::kSign;
  } else if (name == "sin") {
    return IntrinsicType::kSin;
  } else if (name == "sinh") {
    return IntrinsicType::kSinh;
  } else if (name == "smoothStep") {
    return IntrinsicType::kSmoothStep;
  } else if (name == "sqrt") {
    return IntrinsicType::kSqrt;
  } else if (name == "step") {
    return IntrinsicType::kStep;
  } else if (name == "tan") {
    return IntrinsicType::kTan;
  } else if (name == "tanh") {
    return IntrinsicType::kTanh;
  } else if (name == "textureDimensions") {
    return IntrinsicType::kTextureDimensions;
  } else if (name == "textureNumLayers") {
    return IntrinsicType::kTextureNumLayers;
  } else if (name == "textureNumLevels") {
    return IntrinsicType::kTextureNumLevels;
  } else if (name == "textureNumSamples") {
    return IntrinsicType::kTextureNumSamples;
  } else if (name == "textureLoad") {
    return IntrinsicType::kTextureLoad;
  } else if (name == "textureStore") {
    return IntrinsicType::kTextureStore;
  } else if (name == "textureSample") {
    return IntrinsicType::kTextureSample;
  } else if (name == "textureSampleBias") {
    return IntrinsicType::kTextureSampleBias;
  } else if (name == "textureSampleCompare") {
    return IntrinsicType::kTextureSampleCompare;
  } else if (name == "textureSampleGrad") {
    return IntrinsicType::kTextureSampleGrad;
  } else if (name == "textureSampleLevel") {
    return IntrinsicType::kTextureSampleLevel;
  } else if (name == "trunc") {
    return IntrinsicType::kTrunc;
  } else if (name == "unpack4x8snorm") {
    return IntrinsicType::kUnpack4x8Snorm;
  } else if (name == "unpack4x8unorm") {
    return IntrinsicType::kUnpack4x8Unorm;
  } else if (name == "unpack2x16snorm") {
    return IntrinsicType::kUnpack2x16Snorm;
  } else if (name == "unpack2x16unorm") {
    return IntrinsicType::kUnpack2x16Unorm;
  } else if (name == "unpack2x16float") {
    return IntrinsicType::kUnpack2x16Float;
  }
  return IntrinsicType::kNone;
}

bool TypeDeterminer::DetermineMemberAccessor(
    ast::MemberAccessorExpression* expr) {
  if (!DetermineResultType(expr->structure())) {
    return false;
  }

  auto* res = TypeOf(expr->structure());
  auto* data_type = res->UnwrapPtrIfNeeded()->UnwrapIfNeeded();

  type::Type* ret = nullptr;
  bool is_swizzle = false;

  if (auto* ty = data_type->As<type::Struct>()) {
    auto* strct = ty->impl();
    auto symbol = expr->member()->symbol();

    for (auto* member : strct->members()) {
      if (member->symbol() == symbol) {
        ret = member->type();
        break;
      }
    }

    if (ret == nullptr) {
      diagnostics_.add_error(
          "struct member " + builder_->Symbols().NameFor(symbol) + " not found",
          expr->source());
      return false;
    }

    // If we're extracting from a pointer, we return a pointer.
    if (auto* ptr = res->As<type::Pointer>()) {
      ret = builder_->create<type::Pointer>(ret, ptr->storage_class());
    }
  } else if (auto* vec = data_type->As<type::Vector>()) {
    is_swizzle = true;

    auto size = builder_->Symbols().NameFor(expr->member()->symbol()).size();
    if (size == 1) {
      // A single element swizzle is just the type of the vector.
      ret = vec->type();
      // If we're extracting from a pointer, we return a pointer.
      if (auto* ptr = res->As<type::Pointer>()) {
        ret = builder_->create<type::Pointer>(ret, ptr->storage_class());
      }
    } else {
      // The vector will have a number of components equal to the length of the
      // swizzle. This assumes the validator will check that the swizzle
      // is correct.
      ret = builder_->create<type::Vector>(vec->type(),
                                           static_cast<uint32_t>(size));
    }
  } else {
    diagnostics_.add_error(
        "v-0007: invalid use of member accessor on a non-vector/non-struct " +
            data_type->type_name(),
        expr->source());
    return false;
  }

  builder_->Sem().Add(expr,
                      builder_->create<semantic::MemberAccessorExpression>(
                          expr, ret, current_statement_, is_swizzle));
  SetType(expr, ret);

  return true;
}

bool TypeDeterminer::DetermineBinary(ast::BinaryExpression* expr) {
  if (!DetermineResultType(expr->lhs()) || !DetermineResultType(expr->rhs())) {
    return false;
  }

  // Result type matches first parameter type
  if (expr->IsAnd() || expr->IsOr() || expr->IsXor() || expr->IsShiftLeft() ||
      expr->IsShiftRight() || expr->IsAdd() || expr->IsSubtract() ||
      expr->IsDivide() || expr->IsModulo()) {
    SetType(expr, TypeOf(expr->lhs())->UnwrapPtrIfNeeded());
    return true;
  }
  // Result type is a scalar or vector of boolean type
  if (expr->IsLogicalAnd() || expr->IsLogicalOr() || expr->IsEqual() ||
      expr->IsNotEqual() || expr->IsLessThan() || expr->IsGreaterThan() ||
      expr->IsLessThanEqual() || expr->IsGreaterThanEqual()) {
    auto* bool_type = builder_->create<type::Bool>();
    auto* param_type = TypeOf(expr->lhs())->UnwrapPtrIfNeeded();
    type::Type* result_type = bool_type;
    if (auto* vec = param_type->As<type::Vector>()) {
      result_type = builder_->create<type::Vector>(bool_type, vec->size());
    }
    SetType(expr, result_type);
    return true;
  }
  if (expr->IsMultiply()) {
    auto* lhs_type = TypeOf(expr->lhs())->UnwrapPtrIfNeeded();
    auto* rhs_type = TypeOf(expr->rhs())->UnwrapPtrIfNeeded();

    // Note, the ordering here matters. The later checks depend on the prior
    // checks having been done.
    auto* lhs_mat = lhs_type->As<type::Matrix>();
    auto* rhs_mat = rhs_type->As<type::Matrix>();
    auto* lhs_vec = lhs_type->As<type::Vector>();
    auto* rhs_vec = rhs_type->As<type::Vector>();
    type::Type* result_type;
    if (lhs_mat && rhs_mat) {
      result_type = builder_->create<type::Matrix>(
          lhs_mat->type(), lhs_mat->rows(), rhs_mat->columns());
    } else if (lhs_mat && rhs_vec) {
      result_type =
          builder_->create<type::Vector>(lhs_mat->type(), lhs_mat->rows());
    } else if (lhs_vec && rhs_mat) {
      result_type =
          builder_->create<type::Vector>(rhs_mat->type(), rhs_mat->columns());
    } else if (lhs_mat) {
      // matrix * scalar
      result_type = lhs_type;
    } else if (rhs_mat) {
      // scalar * matrix
      result_type = rhs_type;
    } else if (lhs_vec && rhs_vec) {
      result_type = lhs_type;
    } else if (lhs_vec) {
      // Vector * scalar
      result_type = lhs_type;
    } else if (rhs_vec) {
      // Scalar * vector
      result_type = rhs_type;
    } else {
      // Scalar * Scalar
      result_type = lhs_type;
    }

    SetType(expr, result_type);
    return true;
  }

  diagnostics_.add_error("Unknown binary expression", expr->source());
  return false;
}

bool TypeDeterminer::DetermineUnaryOp(ast::UnaryOpExpression* expr) {
  // Result type matches the parameter type.
  if (!DetermineResultType(expr->expr())) {
    return false;
  }

  auto* result_type = TypeOf(expr->expr())->UnwrapPtrIfNeeded();
  SetType(expr, result_type);
  return true;
}

TypeDeterminer::VariableInfo* TypeDeterminer::CreateVariableInfo(
    ast::Variable* var) {
  auto* info = variable_infos_.Create(var);
  variable_to_info_.emplace(var, info);
  return info;
}

type::Type* TypeDeterminer::TypeOf(ast::Expression* expr) {
  auto it = expr_info_.find(expr);
  if (it != expr_info_.end()) {
    return it->second.type;
  }
  return nullptr;
}

void TypeDeterminer::SetType(ast::Expression* expr, type::Type* type) {
  assert(expr_info_.count(expr) == 0);
  expr_info_.emplace(expr, ExpressionInfo{type, current_statement_});
}

void TypeDeterminer::CreateSemanticNodes() const {
  auto& sem = builder_->Sem();

  // Create semantic nodes for all ast::Variables
  for (auto it : variable_to_info_) {
    auto* var = it.first;
    auto* info = it.second;
    std::vector<const semantic::Expression*> users;
    for (auto* user : info->users) {
      // Create semantic node for the identifier expression if necessary
      auto* sem_expr = sem.Get(user);
      if (sem_expr == nullptr) {
        auto* type = expr_info_.at(user).type;
        auto* stmt = expr_info_.at(user).statement;
        sem_expr = builder_->create<semantic::Expression>(user, type, stmt);
        sem.Add(user, sem_expr);
      }
      users.push_back(sem_expr);
    }
    sem.Add(var, builder_->create<semantic::Variable>(var, info->storage_class,
                                                      std::move(users)));
  }

  auto remap_vars = [&sem](const std::vector<VariableInfo*>& in) {
    std::vector<const semantic::Variable*> out;
    out.reserve(in.size());
    for (auto* info : in) {
      out.emplace_back(sem.Get(info->declaration));
    }
    return out;
  };

  // Create semantic nodes for all ast::Functions
  std::unordered_map<FunctionInfo*, semantic::Function*> func_info_to_sem_func;
  for (auto it : function_to_info_) {
    auto* func = it.first;
    auto* info = it.second;
    auto* sem_func = builder_->create<semantic::Function>(
        info->declaration, remap_vars(info->referenced_module_vars),
        remap_vars(info->local_referenced_module_vars),
        info->ancestor_entry_points);
    func_info_to_sem_func.emplace(info, sem_func);
    sem.Add(func, sem_func);
  }

  // Create semantic nodes for all ast::CallExpressions
  for (auto it : function_calls_) {
    auto* call = it.first;
    auto info = it.second;
    auto* sem_func = func_info_to_sem_func.at(info.function);
    sem.Add(call,
            builder_->create<semantic::Call>(call, sem_func, info.statement));
  }

  // Create semantic nodes for all remaining expression types
  for (auto it : expr_info_) {
    auto* expr = it.first;
    auto& info = it.second;
    if (sem.Get(expr)) {
      // Expression has already been assigned a semantic node
      continue;
    }
    sem.Add(expr, builder_->create<semantic::Expression>(expr, info.type,
                                                         info.statement));
  }
}

TypeDeterminer::VariableInfo::VariableInfo(ast::Variable* decl)
    : declaration(decl), storage_class(decl->declared_storage_class()) {}

TypeDeterminer::VariableInfo::~VariableInfo() = default;

TypeDeterminer::FunctionInfo::FunctionInfo(ast::Function* decl)
    : declaration(decl) {}

TypeDeterminer::FunctionInfo::~FunctionInfo() = default;

}  // namespace tint
