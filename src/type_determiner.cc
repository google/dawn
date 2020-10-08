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
#include "src/ast/else_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/intrinsic.h"
#include "src/ast/loop_statement.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/switch_statement.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
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
#include "src/ast/type_constructor_expression.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"

namespace tint {

TypeDeterminer::TypeDeterminer(Context* ctx, ast::Module* mod)
    : ctx_(*ctx), mod_(mod) {}

TypeDeterminer::~TypeDeterminer() = default;

void TypeDeterminer::set_error(const Source& src, const std::string& msg) {
  error_ = "";
  if (src.line > 0) {
    error_ +=
        std::to_string(src.line) + ":" + std::to_string(src.column) + ": ";
  }
  error_ += msg;
}

void TypeDeterminer::set_referenced_from_function_if_needed(
    ast::Variable* var) {
  if (current_function_ == nullptr) {
    return;
  }
  if (var->storage_class() == ast::StorageClass::kNone ||
      var->storage_class() == ast::StorageClass::kFunction) {
    return;
  }

  current_function_->add_referenced_module_variable(var);
}

bool TypeDeterminer::Determine() {
  for (auto& iter : ctx_.type_mgr().types()) {
    auto& type = iter.second;
    if (!type->IsTexture() || !type->AsTexture()->IsStorage()) {
      continue;
    }
    if (!DetermineStorageTextureSubtype(type->AsTexture()->AsStorage())) {
      set_error(Source{}, "unable to determine storage texture subtype for: " +
                              type->type_name());
      return false;
    }
  }

  for (const auto& var : mod_->global_variables()) {
    variable_stack_.set_global(var->name(), var.get());

    if (var->has_constructor()) {
      if (!DetermineResultType(var->constructor())) {
        return false;
      }
    }
  }

  if (!DetermineFunctions(mod_->functions())) {
    return false;
  }

  // Walk over the caller to callee information and update functions with which
  // entry points call those functions.
  for (const auto& func : mod_->functions()) {
    if (!func->IsEntryPoint()) {
      continue;
    }
    for (const auto& callee : caller_to_callee_[func->name()]) {
      set_entry_points(callee, func->name());
    }
  }

  return true;
}

void TypeDeterminer::set_entry_points(const std::string& fn_name,
                                      const std::string& ep_name) {
  name_to_function_[fn_name]->add_ancestor_entry_point(ep_name);

  for (const auto& callee : caller_to_callee_[fn_name]) {
    set_entry_points(callee, ep_name);
  }
}

bool TypeDeterminer::DetermineFunctions(const ast::FunctionList& funcs) {
  for (const auto& func : funcs) {
    if (!DetermineFunction(func.get())) {
      return false;
    }
  }
  return true;
}

bool TypeDeterminer::DetermineFunction(ast::Function* func) {
  name_to_function_[func->name()] = func;

  current_function_ = func;

  variable_stack_.push_scope();
  for (const auto& param : func->params()) {
    variable_stack_.set(param->name(), param.get());
  }

  if (!DetermineStatements(func->body())) {
    return false;
  }
  variable_stack_.pop_scope();

  current_function_ = nullptr;

  return true;
}

bool TypeDeterminer::DetermineStatements(const ast::BlockStatement* stmts) {
  for (const auto& stmt : *stmts) {
    if (!DetermineVariableStorageClass(stmt.get())) {
      return false;
    }

    if (!DetermineResultType(stmt.get())) {
      return false;
    }
  }
  return true;
}

bool TypeDeterminer::DetermineVariableStorageClass(ast::Statement* stmt) {
  if (!stmt->IsVariableDecl()) {
    return true;
  }

  auto* var = stmt->AsVariableDecl()->variable();
  // Nothing to do for const
  if (var->is_const()) {
    return true;
  }

  if (var->storage_class() == ast::StorageClass::kFunction) {
    return true;
  }

  if (var->storage_class() != ast::StorageClass::kNone) {
    set_error(stmt->source(),
              "function variable has a non-function storage class");
    return false;
  }

  var->set_storage_class(ast::StorageClass::kFunction);
  return true;
}

bool TypeDeterminer::DetermineResultType(ast::Statement* stmt) {
  if (stmt->IsAssign()) {
    auto* a = stmt->AsAssign();
    return DetermineResultType(a->lhs()) && DetermineResultType(a->rhs());
  }
  if (stmt->IsBlock()) {
    return DetermineStatements(stmt->AsBlock());
  }
  if (stmt->IsBreak()) {
    return true;
  }
  if (stmt->IsCall()) {
    return DetermineResultType(stmt->AsCall()->expr());
  }
  if (stmt->IsCase()) {
    auto* c = stmt->AsCase();
    return DetermineStatements(c->body());
  }
  if (stmt->IsContinue()) {
    return true;
  }
  if (stmt->IsDiscard()) {
    return true;
  }
  if (stmt->IsElse()) {
    auto* e = stmt->AsElse();
    return DetermineResultType(e->condition()) &&
           DetermineStatements(e->body());
  }
  if (stmt->IsFallthrough()) {
    return true;
  }
  if (stmt->IsIf()) {
    auto* i = stmt->AsIf();
    if (!DetermineResultType(i->condition()) ||
        !DetermineStatements(i->body())) {
      return false;
    }

    for (const auto& else_stmt : i->else_statements()) {
      if (!DetermineResultType(else_stmt.get())) {
        return false;
      }
    }
    return true;
  }
  if (stmt->IsLoop()) {
    auto* l = stmt->AsLoop();
    return DetermineStatements(l->body()) &&
           DetermineStatements(l->continuing());
  }
  if (stmt->IsReturn()) {
    auto* r = stmt->AsReturn();
    return DetermineResultType(r->value());
  }
  if (stmt->IsSwitch()) {
    auto* s = stmt->AsSwitch();
    if (!DetermineResultType(s->condition())) {
      return false;
    }
    for (const auto& case_stmt : s->body()) {
      if (!DetermineResultType(case_stmt.get())) {
        return false;
      }
    }
    return true;
  }
  if (stmt->IsVariableDecl()) {
    auto* v = stmt->AsVariableDecl();
    variable_stack_.set(v->variable()->name(), v->variable());
    return DetermineResultType(v->variable()->constructor());
  }

  set_error(stmt->source(),
            "unknown statement type for type determination: " + stmt->str());
  return false;
}

bool TypeDeterminer::DetermineResultType(const ast::ExpressionList& list) {
  for (const auto& expr : list) {
    if (!DetermineResultType(expr.get())) {
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

  if (expr->IsArrayAccessor()) {
    return DetermineArrayAccessor(expr->AsArrayAccessor());
  }
  if (expr->IsBinary()) {
    return DetermineBinary(expr->AsBinary());
  }
  if (expr->IsBitcast()) {
    return DetermineBitcast(expr->AsBitcast());
  }
  if (expr->IsCall()) {
    return DetermineCall(expr->AsCall());
  }
  if (expr->IsConstructor()) {
    return DetermineConstructor(expr->AsConstructor());
  }
  if (expr->IsIdentifier()) {
    return DetermineIdentifier(expr->AsIdentifier());
  }
  if (expr->IsMemberAccessor()) {
    return DetermineMemberAccessor(expr->AsMemberAccessor());
  }
  if (expr->IsUnaryOp()) {
    return DetermineUnaryOp(expr->AsUnaryOp());
  }

  set_error(expr->source(), "unknown expression for type determination");
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

  auto* res = expr->array()->result_type();
  auto* parent_type = res->UnwrapAliasPtrAlias();
  ast::type::Type* ret = nullptr;
  if (parent_type->IsArray()) {
    ret = parent_type->AsArray()->type();
  } else if (parent_type->IsVector()) {
    ret = parent_type->AsVector()->type();
  } else if (parent_type->IsMatrix()) {
    auto* m = parent_type->AsMatrix();
    ret = ctx_.type_mgr().Get(
        std::make_unique<ast::type::VectorType>(m->type(), m->rows()));
  } else {
    set_error(expr->source(), "invalid parent type (" +
                                  parent_type->type_name() +
                                  ") in array accessor");
    return false;
  }

  // If we're extracting from a pointer, we return a pointer.
  if (res->IsPointer()) {
    ret = ctx_.type_mgr().Get(std::make_unique<ast::type::PointerType>(
        ret, res->AsPointer()->storage_class()));
  } else if (parent_type->IsArray() &&
             !parent_type->AsArray()->type()->is_scalar()) {
    // If we extract a non-scalar from an array then we also get a pointer. We
    // will generate a Function storage class variable to store this
    // into.
    ret = ctx_.type_mgr().Get(std::make_unique<ast::type::PointerType>(
        ret, ast::StorageClass::kFunction));
  }
  expr->set_result_type(ret);

  return true;
}

bool TypeDeterminer::DetermineBitcast(ast::BitcastExpression* expr) {
  if (!DetermineResultType(expr->expr())) {
    return false;
  }

  expr->set_result_type(expr->type());
  return true;
}

bool TypeDeterminer::DetermineCall(ast::CallExpression* expr) {
  if (!DetermineResultType(expr->func())) {
    return false;
  }
  if (!DetermineResultType(expr->params())) {
    return false;
  }

  // The expression has to be an identifier as you can't store function pointers
  // but, if it isn't we'll just use the normal result determination to be on
  // the safe side.
  if (expr->func()->IsIdentifier()) {
    auto* ident = expr->func()->AsIdentifier();

    if (ident->IsIntrinsic()) {
      if (!DetermineIntrinsic(ident, expr)) {
        return false;
      }
    } else {
      if (current_function_) {
        caller_to_callee_[current_function_->name()].push_back(ident->name());

        auto* callee_func = mod_->FindFunctionByName(ident->name());
        if (callee_func == nullptr) {
          set_error(expr->source(),
                    "unable to find called function: " + ident->name());
          return false;
        }

        // We inherit any referenced variables from the callee.
        for (auto* var : callee_func->referenced_module_variables()) {
          set_referenced_from_function_if_needed(var);
        }
      }

      // An identifier with a single name is a function call, not an import
      // lookup which we can handle with the regular identifier lookup.
      if (!DetermineResultType(ident)) {
        return false;
      }
    }
  } else {
    if (!DetermineResultType(expr->func())) {
      return false;
    }
  }

  if (!expr->func()->result_type()) {
    auto func_name = expr->func()->AsIdentifier()->name();
    set_error(
        expr->source(),
        "v-0005: function must be declared before use: '" + func_name + "'");
    return false;
  }

  expr->set_result_type(expr->func()->result_type());
  return true;
}

namespace {

enum class IntrinsicDataType {
  kFloatOrIntScalarOrVector,
  kFloatScalarOrVector,
  kIntScalarOrVector,
  kFloatVector,
  kMatrix,
};
struct IntrinsicData {
  ast::Intrinsic intrinsic;
  uint8_t param_count;
  IntrinsicDataType data_type;
  uint8_t vector_size;
};

// Note, this isn't all the intrinsics. Some are handled specially before
// we get to the generic code. See the DetermineIntrinsic code below.
constexpr const IntrinsicData kIntrinsicData[] = {
    {ast::Intrinsic::kAbs, 1, IntrinsicDataType::kFloatOrIntScalarOrVector, 0},
    {ast::Intrinsic::kAcos, 1, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kAsin, 1, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kAtan, 1, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kAtan2, 2, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kCeil, 1, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kClamp, 3, IntrinsicDataType::kFloatOrIntScalarOrVector,
     0},
    {ast::Intrinsic::kCos, 1, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kCosh, 1, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kCountOneBits, 1, IntrinsicDataType::kIntScalarOrVector,
     0},
    {ast::Intrinsic::kCross, 2, IntrinsicDataType::kFloatVector, 3},
    {ast::Intrinsic::kDeterminant, 1, IntrinsicDataType::kMatrix, 0},
    {ast::Intrinsic::kDistance, 2, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kExp, 1, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kExp2, 1, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kFaceForward, 3, IntrinsicDataType::kFloatScalarOrVector,
     0},
    {ast::Intrinsic::kFloor, 1, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kFma, 3, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kFract, 1, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kFrexp, 2, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kInverseSqrt, 1, IntrinsicDataType::kFloatScalarOrVector,
     0},
    {ast::Intrinsic::kLdexp, 2, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kLength, 1, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kLog, 1, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kLog2, 1, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kMax, 2, IntrinsicDataType::kFloatOrIntScalarOrVector, 0},
    {ast::Intrinsic::kMin, 2, IntrinsicDataType::kFloatOrIntScalarOrVector, 0},
    {ast::Intrinsic::kMix, 3, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kModf, 2, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kNormalize, 1, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kPow, 2, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kReflect, 2, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kReverseBits, 1, IntrinsicDataType::kIntScalarOrVector, 0},
    {ast::Intrinsic::kRound, 1, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kSign, 1, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kSin, 1, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kSinh, 1, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kSmoothStep, 3, IntrinsicDataType::kFloatScalarOrVector,
     0},
    {ast::Intrinsic::kSqrt, 1, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kStep, 2, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kTan, 1, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kTanh, 1, IntrinsicDataType::kFloatScalarOrVector, 0},
    {ast::Intrinsic::kTrunc, 1, IntrinsicDataType::kFloatScalarOrVector, 0},
};

constexpr const uint32_t kIntrinsicDataCount =
    sizeof(kIntrinsicData) / sizeof(IntrinsicData);

}  // namespace

bool TypeDeterminer::DetermineIntrinsic(ast::IdentifierExpression* ident,
                                        ast::CallExpression* expr) {
  if (ast::intrinsic::IsDerivative(ident->intrinsic())) {
    if (expr->params().size() != 1) {
      set_error(expr->source(),
                "incorrect number of parameters for " + ident->name());
      return false;
    }

    // The result type must be the same as the type of the parameter.
    auto* param_type = expr->params()[0]->result_type()->UnwrapPtrIfNeeded();
    expr->func()->set_result_type(param_type);
    return true;
  }
  if (ident->intrinsic() == ast::Intrinsic::kAny ||
      ident->intrinsic() == ast::Intrinsic::kAll) {
    expr->func()->set_result_type(
        ctx_.type_mgr().Get(std::make_unique<ast::type::BoolType>()));
    return true;
  }
  if (ident->intrinsic() == ast::Intrinsic::kArrayLength) {
    expr->func()->set_result_type(
        ctx_.type_mgr().Get(std::make_unique<ast::type::U32Type>()));
    return true;
  }
  if (ast::intrinsic::IsFloatClassificationIntrinsic(ident->intrinsic())) {
    if (expr->params().size() != 1) {
      set_error(expr->source(),
                "incorrect number of parameters for " + ident->name());
      return false;
    }

    auto* bool_type =
        ctx_.type_mgr().Get(std::make_unique<ast::type::BoolType>());

    auto* param_type = expr->params()[0]->result_type()->UnwrapPtrIfNeeded();
    if (param_type->IsVector()) {
      expr->func()->set_result_type(
          ctx_.type_mgr().Get(std::make_unique<ast::type::VectorType>(
              bool_type, param_type->AsVector()->size())));
    } else {
      expr->func()->set_result_type(bool_type);
    }
    return true;
  }
  if (ast::intrinsic::IsTextureIntrinsic(ident->intrinsic())) {
    // TODO: Remove the LOD param from textureLoad on storage textures when
    // https://github.com/gpuweb/gpuweb/pull/1032 gets merged.
    uint32_t num_of_params =
        (ident->intrinsic() == ast::Intrinsic::kTextureLoad ||
         ident->intrinsic() == ast::Intrinsic::kTextureSample)
            ? 3
            : 4;
    if (expr->params().size() != num_of_params) {
      set_error(expr->source(),
                "incorrect number of parameters for " + ident->name() +
                    ", got " + std::to_string(expr->params().size()) +
                    " and expected " + std::to_string(num_of_params));
      return false;
    }

    if (ident->intrinsic() == ast::Intrinsic::kTextureSampleCompare) {
      expr->func()->set_result_type(
          ctx_.type_mgr().Get(std::make_unique<ast::type::F32Type>()));
      return true;
    }

    auto& texture_param = expr->params()[0];
    if (!texture_param->result_type()->UnwrapPtrIfNeeded()->IsTexture()) {
      set_error(expr->source(), "invalid first argument for " + ident->name());
      return false;
    }
    ast::type::TextureType* texture =
        texture_param->result_type()->UnwrapPtrIfNeeded()->AsTexture();

    if (!texture->IsStorage() &&
        !(texture->IsSampled() || texture->IsMultisampled())) {
      set_error(expr->source(), "invalid texture for " + ident->name());
      return false;
    }

    ast::type::Type* type = nullptr;
    if (texture->IsStorage()) {
      type = texture->AsStorage()->type();
    } else if (texture->IsSampled()) {
      type = texture->AsSampled()->type();
    } else if (texture->IsMultisampled()) {
      type = texture->AsMultisampled()->type();
    } else {
      set_error(expr->source(), "unknown texture type for texture sampling");
      return false;
    }
    expr->func()->set_result_type(
        ctx_.type_mgr().Get(std::make_unique<ast::type::VectorType>(type, 4)));
    return true;
  }
  if (ident->intrinsic() == ast::Intrinsic::kDot) {
    expr->func()->set_result_type(
        ctx_.type_mgr().Get(std::make_unique<ast::type::F32Type>()));
    return true;
  }
  if (ident->intrinsic() == ast::Intrinsic::kOuterProduct) {
    if (expr->params().size() != 2) {
      set_error(expr->source(),
                "incorrect number of parameters for " + ident->name());
      return false;
    }

    auto* param0_type = expr->params()[0]->result_type()->UnwrapPtrIfNeeded();
    auto* param1_type = expr->params()[1]->result_type()->UnwrapPtrIfNeeded();
    if (!param0_type->IsVector() || !param1_type->IsVector()) {
      set_error(expr->source(), "invalid parameter type for " + ident->name());
      return false;
    }

    expr->func()->set_result_type(
        ctx_.type_mgr().Get(std::make_unique<ast::type::MatrixType>(
            ctx_.type_mgr().Get(std::make_unique<ast::type::F32Type>()),
            param0_type->AsVector()->size(), param1_type->AsVector()->size())));
    return true;
  }
  if (ident->intrinsic() == ast::Intrinsic::kSelect) {
    if (expr->params().size() != 3) {
      set_error(expr->source(), "incorrect number of parameters for " +
                                    ident->name() + " expected 3 got " +
                                    std::to_string(expr->params().size()));
      return false;
    }

    // The result type must be the same as the type of the parameter.
    auto* param_type = expr->params()[0]->result_type()->UnwrapPtrIfNeeded();
    expr->func()->set_result_type(param_type);
    return true;
  }

  const IntrinsicData* data = nullptr;
  for (uint32_t i = 0; i < kIntrinsicDataCount; ++i) {
    if (ident->intrinsic() == kIntrinsicData[i].intrinsic) {
      data = &kIntrinsicData[i];
      break;
    }
  }
  if (data == nullptr) {
    error_ = "unable to find intrinsic " + ident->name();
    return false;
  }

  if (expr->params().size() != data->param_count) {
    set_error(expr->source(), "incorrect number of parameters for " +
                                  ident->name() + ". Expected " +
                                  std::to_string(data->param_count) + " got " +
                                  std::to_string(expr->params().size()));
    return false;
  }

  std::vector<ast::type::Type*> result_types;
  for (uint32_t i = 0; i < data->param_count; ++i) {
    result_types.push_back(
        expr->params()[i]->result_type()->UnwrapPtrIfNeeded());

    switch (data->data_type) {
      case IntrinsicDataType::kFloatOrIntScalarOrVector:
        if (!result_types.back()->is_float_scalar_or_vector() &&
            !result_types.back()->is_integer_scalar_or_vector()) {
          set_error(expr->source(),
                    "incorrect type for " + ident->name() + ". " +
                        "Requires float or int, scalar or vector values");
          return false;
        }
        break;
      case IntrinsicDataType::kFloatScalarOrVector:
        if (!result_types.back()->is_float_scalar_or_vector()) {
          set_error(expr->source(),
                    "incorrect type for " + ident->name() + ". " +
                        "Requires float scalar or float vector values");
          return false;
        }

        break;
      case IntrinsicDataType::kIntScalarOrVector:
        if (!result_types.back()->is_integer_scalar_or_vector()) {
          set_error(expr->source(),
                    "incorrect type for " + ident->name() + ". " +
                        "Requires integer scalar or integer vector values");
          return false;
        }
        break;
      case IntrinsicDataType::kFloatVector:
        if (!result_types.back()->is_float_vector()) {
          set_error(expr->source(), "incorrect type for " + ident->name() +
                                        ". " + "Requires float vector values");
          return false;
        }
        if (data->vector_size > 0 &&
            result_types.back()->AsVector()->size() != data->vector_size) {
          set_error(expr->source(), "incorrect vector size for " +
                                        ident->name() + ". " + "Requires " +
                                        std::to_string(data->vector_size) +
                                        " elements");
          return false;
        }
        break;
      case IntrinsicDataType::kMatrix:
        if (!result_types.back()->IsMatrix()) {
          set_error(expr->source(), "incorrect type for " + ident->name() +
                                        ". Requires matrix value");
          return false;
        }
        break;
    }
  }

  // Verify all the parameter types match
  for (size_t i = 1; i < data->param_count; ++i) {
    if (result_types[0] != result_types[i]) {
      set_error(expr->source(),
                "mismatched parameter types for " + ident->name());
      return false;
    }
  }

  // Handle functions which aways return the type, even if a vector is
  // provided.
  if (ident->intrinsic() == ast::Intrinsic::kLength ||
      ident->intrinsic() == ast::Intrinsic::kDistance) {
    expr->func()->set_result_type(result_types[0]->is_float_scalar()
                                      ? result_types[0]
                                      : result_types[0]->AsVector()->type());
    return true;
  }
  // The determinant returns the component type of the columns
  if (ident->intrinsic() == ast::Intrinsic::kDeterminant) {
    expr->func()->set_result_type(result_types[0]->AsMatrix()->type());
    return true;
  }
  expr->func()->set_result_type(result_types[0]);
  return true;
}

bool TypeDeterminer::DetermineConstructor(ast::ConstructorExpression* expr) {
  if (expr->IsTypeConstructor()) {
    auto* ty = expr->AsTypeConstructor();
    for (const auto& value : ty->values()) {
      if (!DetermineResultType(value.get())) {
        return false;
      }
    }
    expr->set_result_type(ty->type());
  } else {
    expr->set_result_type(expr->AsScalarConstructor()->literal()->type());
  }
  return true;
}

bool TypeDeterminer::DetermineIdentifier(ast::IdentifierExpression* expr) {
  auto name = expr->name();
  ast::Variable* var;
  if (variable_stack_.get(name, &var)) {
    // A constant is the type, but a variable is always a pointer so synthesize
    // the pointer around the variable type.
    if (var->is_const()) {
      expr->set_result_type(var->type());
    } else if (var->type()->IsPointer()) {
      expr->set_result_type(var->type());
    } else {
      expr->set_result_type(
          ctx_.type_mgr().Get(std::make_unique<ast::type::PointerType>(
              var->type(), var->storage_class())));
    }

    set_referenced_from_function_if_needed(var);
    return true;
  }

  auto iter = name_to_function_.find(name);
  if (iter != name_to_function_.end()) {
    expr->set_result_type(iter->second->return_type());
    return true;
  }

  SetIntrinsicIfNeeded(expr);

  return true;
}

void TypeDeterminer::SetIntrinsicIfNeeded(ast::IdentifierExpression* ident) {
  if (ident->name() == "abs") {
    ident->set_intrinsic(ast::Intrinsic::kAbs);
  } else if (ident->name() == "acos") {
    ident->set_intrinsic(ast::Intrinsic::kAcos);
  } else if (ident->name() == "all") {
    ident->set_intrinsic(ast::Intrinsic::kAll);
  } else if (ident->name() == "any") {
    ident->set_intrinsic(ast::Intrinsic::kAny);
  } else if (ident->name() == "arrayLength") {
    ident->set_intrinsic(ast::Intrinsic::kArrayLength);
  } else if (ident->name() == "asin") {
    ident->set_intrinsic(ast::Intrinsic::kAsin);
  } else if (ident->name() == "atan") {
    ident->set_intrinsic(ast::Intrinsic::kAtan);
  } else if (ident->name() == "atan2") {
    ident->set_intrinsic(ast::Intrinsic::kAtan2);
  } else if (ident->name() == "ceil") {
    ident->set_intrinsic(ast::Intrinsic::kCeil);
  } else if (ident->name() == "clamp") {
    ident->set_intrinsic(ast::Intrinsic::kClamp);
  } else if (ident->name() == "cos") {
    ident->set_intrinsic(ast::Intrinsic::kCos);
  } else if (ident->name() == "cosh") {
    ident->set_intrinsic(ast::Intrinsic::kCosh);
  } else if (ident->name() == "countOneBits") {
    ident->set_intrinsic(ast::Intrinsic::kCountOneBits);
  } else if (ident->name() == "cross") {
    ident->set_intrinsic(ast::Intrinsic::kCross);
  } else if (ident->name() == "determinant") {
    ident->set_intrinsic(ast::Intrinsic::kDeterminant);
  } else if (ident->name() == "distance") {
    ident->set_intrinsic(ast::Intrinsic::kDistance);
  } else if (ident->name() == "dot") {
    ident->set_intrinsic(ast::Intrinsic::kDot);
  } else if (ident->name() == "dpdx") {
    ident->set_intrinsic(ast::Intrinsic::kDpdx);
  } else if (ident->name() == "dpdxCoarse") {
    ident->set_intrinsic(ast::Intrinsic::kDpdxCoarse);
  } else if (ident->name() == "dpdxFine") {
    ident->set_intrinsic(ast::Intrinsic::kDpdxFine);
  } else if (ident->name() == "dpdy") {
    ident->set_intrinsic(ast::Intrinsic::kDpdy);
  } else if (ident->name() == "dpdyCoarse") {
    ident->set_intrinsic(ast::Intrinsic::kDpdyCoarse);
  } else if (ident->name() == "dpdyFine") {
    ident->set_intrinsic(ast::Intrinsic::kDpdyFine);
  } else if (ident->name() == "exp") {
    ident->set_intrinsic(ast::Intrinsic::kExp);
  } else if (ident->name() == "exp2") {
    ident->set_intrinsic(ast::Intrinsic::kExp2);
  } else if (ident->name() == "faceForward") {
    ident->set_intrinsic(ast::Intrinsic::kFaceForward);
  } else if (ident->name() == "floor") {
    ident->set_intrinsic(ast::Intrinsic::kFloor);
  } else if (ident->name() == "fma") {
    ident->set_intrinsic(ast::Intrinsic::kFma);
  } else if (ident->name() == "fract") {
    ident->set_intrinsic(ast::Intrinsic::kFract);
  } else if (ident->name() == "frexp") {
    ident->set_intrinsic(ast::Intrinsic::kFrexp);
  } else if (ident->name() == "fwidth") {
    ident->set_intrinsic(ast::Intrinsic::kFwidth);
  } else if (ident->name() == "fwidthCoarse") {
    ident->set_intrinsic(ast::Intrinsic::kFwidthCoarse);
  } else if (ident->name() == "fwidthFine") {
    ident->set_intrinsic(ast::Intrinsic::kFwidthFine);
  } else if (ident->name() == "inverseSqrt") {
    ident->set_intrinsic(ast::Intrinsic::kInverseSqrt);
  } else if (ident->name() == "isFinite") {
    ident->set_intrinsic(ast::Intrinsic::kIsFinite);
  } else if (ident->name() == "isInf") {
    ident->set_intrinsic(ast::Intrinsic::kIsInf);
  } else if (ident->name() == "isNan") {
    ident->set_intrinsic(ast::Intrinsic::kIsNan);
  } else if (ident->name() == "isNormal") {
    ident->set_intrinsic(ast::Intrinsic::kIsNormal);
  } else if (ident->name() == "ldexp") {
    ident->set_intrinsic(ast::Intrinsic::kLdexp);
  } else if (ident->name() == "length") {
    ident->set_intrinsic(ast::Intrinsic::kLength);
  } else if (ident->name() == "log") {
    ident->set_intrinsic(ast::Intrinsic::kLog);
  } else if (ident->name() == "log2") {
    ident->set_intrinsic(ast::Intrinsic::kLog2);
  } else if (ident->name() == "max") {
    ident->set_intrinsic(ast::Intrinsic::kMax);
  } else if (ident->name() == "min") {
    ident->set_intrinsic(ast::Intrinsic::kMin);
  } else if (ident->name() == "mix") {
    ident->set_intrinsic(ast::Intrinsic::kMix);
  } else if (ident->name() == "modf") {
    ident->set_intrinsic(ast::Intrinsic::kModf);
  } else if (ident->name() == "normalize") {
    ident->set_intrinsic(ast::Intrinsic::kNormalize);
  } else if (ident->name() == "outerProduct") {
    ident->set_intrinsic(ast::Intrinsic::kOuterProduct);
  } else if (ident->name() == "pow") {
    ident->set_intrinsic(ast::Intrinsic::kPow);
  } else if (ident->name() == "reflect") {
    ident->set_intrinsic(ast::Intrinsic::kReflect);
  } else if (ident->name() == "reverseBits") {
    ident->set_intrinsic(ast::Intrinsic::kReverseBits);
  } else if (ident->name() == "round") {
    ident->set_intrinsic(ast::Intrinsic::kRound);
  } else if (ident->name() == "select") {
    ident->set_intrinsic(ast::Intrinsic::kSelect);
  } else if (ident->name() == "sign") {
    ident->set_intrinsic(ast::Intrinsic::kSign);
  } else if (ident->name() == "sin") {
    ident->set_intrinsic(ast::Intrinsic::kSin);
  } else if (ident->name() == "sinh") {
    ident->set_intrinsic(ast::Intrinsic::kSinh);
  } else if (ident->name() == "smoothStep") {
    ident->set_intrinsic(ast::Intrinsic::kSmoothStep);
  } else if (ident->name() == "sqrt") {
    ident->set_intrinsic(ast::Intrinsic::kSqrt);
  } else if (ident->name() == "step") {
    ident->set_intrinsic(ast::Intrinsic::kStep);
  } else if (ident->name() == "tan") {
    ident->set_intrinsic(ast::Intrinsic::kTan);
  } else if (ident->name() == "tanh") {
    ident->set_intrinsic(ast::Intrinsic::kTanh);
  } else if (ident->name() == "textureLoad") {
    ident->set_intrinsic(ast::Intrinsic::kTextureLoad);
  } else if (ident->name() == "textureSample") {
    ident->set_intrinsic(ast::Intrinsic::kTextureSample);
  } else if (ident->name() == "textureSampleBias") {
    ident->set_intrinsic(ast::Intrinsic::kTextureSampleBias);
  } else if (ident->name() == "textureSampleCompare") {
    ident->set_intrinsic(ast::Intrinsic::kTextureSampleCompare);
  } else if (ident->name() == "textureSampleLevel") {
    ident->set_intrinsic(ast::Intrinsic::kTextureSampleLevel);
  } else if (ident->name() == "trunc") {
    ident->set_intrinsic(ast::Intrinsic::kTrunc);
  }
}

bool TypeDeterminer::DetermineMemberAccessor(
    ast::MemberAccessorExpression* expr) {
  if (!DetermineResultType(expr->structure())) {
    return false;
  }

  auto* res = expr->structure()->result_type();
  auto* data_type = res->UnwrapPtrIfNeeded()->UnwrapAliasesIfNeeded();

  ast::type::Type* ret = nullptr;
  if (data_type->IsStruct()) {
    auto* strct = data_type->AsStruct()->impl();
    auto name = expr->member()->name();

    for (const auto& member : strct->members()) {
      if (member->name() == name) {
        ret = member->type();
        break;
      }
    }

    if (ret == nullptr) {
      set_error(expr->source(), "struct member " + name + " not found");
      return false;
    }

    // If we're extracting from a pointer, we return a pointer.
    if (res->IsPointer()) {
      ret = ctx_.type_mgr().Get(std::make_unique<ast::type::PointerType>(
          ret, res->AsPointer()->storage_class()));
    }
  } else if (data_type->IsVector()) {
    auto* vec = data_type->AsVector();

    auto size = expr->member()->name().size();
    if (size == 1) {
      // A single element swizzle is just the type of the vector.
      ret = vec->type();
      // If we're extracting from a pointer, we return a pointer.
      if (res->IsPointer()) {
        ret = ctx_.type_mgr().Get(std::make_unique<ast::type::PointerType>(
            ret, res->AsPointer()->storage_class()));
      }
    } else {
      // The vector will have a number of components equal to the length of the
      // swizzle. This assumes the validator will check that the swizzle
      // is correct.
      ret = ctx_.type_mgr().Get(
          std::make_unique<ast::type::VectorType>(vec->type(), size));
    }
  } else {
    set_error(expr->source(),
              "invalid type " + data_type->type_name() + " in member accessor");
    return false;
  }

  expr->set_result_type(ret);

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
    expr->set_result_type(expr->lhs()->result_type()->UnwrapPtrIfNeeded());
    return true;
  }
  // Result type is a scalar or vector of boolean type
  if (expr->IsLogicalAnd() || expr->IsLogicalOr() || expr->IsEqual() ||
      expr->IsNotEqual() || expr->IsLessThan() || expr->IsGreaterThan() ||
      expr->IsLessThanEqual() || expr->IsGreaterThanEqual()) {
    auto* bool_type =
        ctx_.type_mgr().Get(std::make_unique<ast::type::BoolType>());
    auto* param_type = expr->lhs()->result_type()->UnwrapPtrIfNeeded();
    if (param_type->IsVector()) {
      expr->set_result_type(
          ctx_.type_mgr().Get(std::make_unique<ast::type::VectorType>(
              bool_type, param_type->AsVector()->size())));
    } else {
      expr->set_result_type(bool_type);
    }
    return true;
  }
  if (expr->IsMultiply()) {
    auto* lhs_type = expr->lhs()->result_type()->UnwrapPtrIfNeeded();
    auto* rhs_type = expr->rhs()->result_type()->UnwrapPtrIfNeeded();

    // Note, the ordering here matters. The later checks depend on the prior
    // checks having been done.
    if (lhs_type->IsMatrix() && rhs_type->IsMatrix()) {
      expr->set_result_type(
          ctx_.type_mgr().Get(std::make_unique<ast::type::MatrixType>(
              lhs_type->AsMatrix()->type(), lhs_type->AsMatrix()->rows(),
              rhs_type->AsMatrix()->columns())));

    } else if (lhs_type->IsMatrix() && rhs_type->IsVector()) {
      auto* mat = lhs_type->AsMatrix();
      expr->set_result_type(ctx_.type_mgr().Get(
          std::make_unique<ast::type::VectorType>(mat->type(), mat->rows())));
    } else if (lhs_type->IsVector() && rhs_type->IsMatrix()) {
      auto* mat = rhs_type->AsMatrix();
      expr->set_result_type(
          ctx_.type_mgr().Get(std::make_unique<ast::type::VectorType>(
              mat->type(), mat->columns())));
    } else if (lhs_type->IsMatrix()) {
      // matrix * scalar
      expr->set_result_type(lhs_type);
    } else if (rhs_type->IsMatrix()) {
      // scalar * matrix
      expr->set_result_type(rhs_type);
    } else if (lhs_type->IsVector() && rhs_type->IsVector()) {
      expr->set_result_type(lhs_type);
    } else if (lhs_type->IsVector()) {
      // Vector * scalar
      expr->set_result_type(lhs_type);
    } else if (rhs_type->IsVector()) {
      // Scalar * vector
      expr->set_result_type(rhs_type);
    } else {
      // Scalar * Scalar
      expr->set_result_type(lhs_type);
    }

    return true;
  }

  set_error(expr->source(), "Unknown binary expression");
  return false;
}

bool TypeDeterminer::DetermineUnaryOp(ast::UnaryOpExpression* expr) {
  // Result type matches the parameter type.
  if (!DetermineResultType(expr->expr())) {
    return false;
  }
  expr->set_result_type(expr->expr()->result_type()->UnwrapPtrIfNeeded());
  return true;
}

bool TypeDeterminer::DetermineStorageTextureSubtype(
    ast::type::StorageTextureType* tex) {
  if (tex->type() != nullptr) {
    return true;
  }

  switch (tex->image_format()) {
    case ast::type::ImageFormat::kR8Unorm:
    case ast::type::ImageFormat::kRg8Unorm:
    case ast::type::ImageFormat::kRgba8Unorm:
    case ast::type::ImageFormat::kRgba8UnormSrgb:
    case ast::type::ImageFormat::kBgra8Unorm:
    case ast::type::ImageFormat::kBgra8UnormSrgb:
    case ast::type::ImageFormat::kRgb10A2Unorm:
    case ast::type::ImageFormat::kR8Uint:
    case ast::type::ImageFormat::kR16Uint:
    case ast::type::ImageFormat::kRg8Uint:
    case ast::type::ImageFormat::kR32Uint:
    case ast::type::ImageFormat::kRg16Uint:
    case ast::type::ImageFormat::kRgba8Uint:
    case ast::type::ImageFormat::kRg32Uint:
    case ast::type::ImageFormat::kRgba16Uint:
    case ast::type::ImageFormat::kRgba32Uint: {
      tex->set_type(
          ctx_.type_mgr().Get(std::make_unique<ast::type::U32Type>()));
      return true;
    }

    case ast::type::ImageFormat::kR8Snorm:
    case ast::type::ImageFormat::kRg8Snorm:
    case ast::type::ImageFormat::kRgba8Snorm:
    case ast::type::ImageFormat::kR8Sint:
    case ast::type::ImageFormat::kR16Sint:
    case ast::type::ImageFormat::kRg8Sint:
    case ast::type::ImageFormat::kR32Sint:
    case ast::type::ImageFormat::kRg16Sint:
    case ast::type::ImageFormat::kRgba8Sint:
    case ast::type::ImageFormat::kRg32Sint:
    case ast::type::ImageFormat::kRgba16Sint:
    case ast::type::ImageFormat::kRgba32Sint: {
      tex->set_type(
          ctx_.type_mgr().Get(std::make_unique<ast::type::I32Type>()));
      return true;
    }

    case ast::type::ImageFormat::kR16Float:
    case ast::type::ImageFormat::kR32Float:
    case ast::type::ImageFormat::kRg16Float:
    case ast::type::ImageFormat::kRg11B10Float:
    case ast::type::ImageFormat::kRg32Float:
    case ast::type::ImageFormat::kRgba16Float:
    case ast::type::ImageFormat::kRgba32Float: {
      tex->set_type(
          ctx_.type_mgr().Get(std::make_unique<ast::type::F32Type>()));
      return true;
    }

    case ast::type::ImageFormat::kNone:
      break;
  }

  return false;
}

}  // namespace tint
