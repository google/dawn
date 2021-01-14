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
#include "src/ast/discard_statement.h"
#include "src/ast/else_statement.h"
#include "src/ast/fallthrough_statement.h"
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
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"

namespace tint {

TypeDeterminer::TypeDeterminer(ast::Module* mod) : mod_(mod) {}

TypeDeterminer::~TypeDeterminer() = default;

void TypeDeterminer::set_error(const Source& src, const std::string& msg) {
  error_ = "";
  if (src.range.begin.line > 0) {
    error_ += std::to_string(src.range.begin.line) + ":" +
              std::to_string(src.range.begin.column) + ": ";
  }
  error_ += msg;
}

void TypeDeterminer::set_referenced_from_function_if_needed(ast::Variable* var,
                                                            bool local) {
  if (current_function_ == nullptr) {
    return;
  }
  if (var->storage_class() == ast::StorageClass::kNone ||
      var->storage_class() == ast::StorageClass::kFunction) {
    return;
  }

  current_function_->add_referenced_module_variable(var);
  if (local) {
    current_function_->add_local_referenced_module_variable(var);
  }
}

bool TypeDeterminer::Determine() {
  std::vector<ast::type::StorageTexture*> storage_textures;
  for (auto& it : mod_->types()) {
    if (auto* storage =
            it.second->UnwrapIfNeeded()->As<ast::type::StorageTexture>()) {
      storage_textures.emplace_back(storage);
    }
  }

  for (auto* storage : storage_textures) {
    if (!DetermineStorageTextureSubtype(storage)) {
      set_error(Source{}, "unable to determine storage texture subtype for: " +
                              storage->type_name());
      return false;
    }
  }

  for (auto* var : mod_->global_variables()) {
    variable_stack_.set_global(var->symbol(), var);

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
  for (auto* func : mod_->functions()) {
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
  symbol_to_function_[fn_sym]->add_ancestor_entry_point(ep_sym);

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
  symbol_to_function_[func->symbol()] = func;

  current_function_ = func;

  variable_stack_.push_scope();
  for (auto* param : func->params()) {
    variable_stack_.set(param->symbol(), param);
  }

  if (!DetermineStatements(func->body())) {
    return false;
  }
  variable_stack_.pop_scope();

  current_function_ = nullptr;

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
    variable_stack_.set(v->variable()->symbol(), v->variable());
    return DetermineResultType(v->variable()->constructor());
  }

  set_error(stmt->source(),
            "unknown statement type for type determination: " + stmt->str());
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
  auto* parent_type = res->UnwrapAll();
  ast::type::Type* ret = nullptr;
  if (auto* arr = parent_type->As<ast::type::Array>()) {
    ret = arr->type();
  } else if (auto* vec = parent_type->As<ast::type::Vector>()) {
    ret = vec->type();
  } else if (auto* mat = parent_type->As<ast::type::Matrix>()) {
    ret = mod_->create<ast::type::Vector>(mat->type(), mat->rows());
  } else {
    set_error(expr->source(), "invalid parent type (" +
                                  parent_type->type_name() +
                                  ") in array accessor");
    return false;
  }

  // If we're extracting from a pointer, we return a pointer.
  if (auto* ptr = res->As<ast::type::Pointer>()) {
    ret = mod_->create<ast::type::Pointer>(ret, ptr->storage_class());
  } else if (auto* arr = parent_type->As<ast::type::Array>()) {
    if (!arr->type()->is_scalar()) {
      // If we extract a non-scalar from an array then we also get a pointer. We
      // will generate a Function storage class variable to store this
      // into.
      ret = mod_->create<ast::type::Pointer>(ret, ast::StorageClass::kFunction);
    }
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
  if (auto* ident = expr->func()->As<ast::IdentifierExpression>()) {
    if (ident->IsIntrinsic()) {
      if (!DetermineIntrinsic(ident, expr)) {
        return false;
      }
    } else {
      if (current_function_) {
        caller_to_callee_[current_function_->symbol()].push_back(
            ident->symbol());

        auto* callee_func = mod_->FindFunctionBySymbol(ident->symbol());
        if (callee_func == nullptr) {
          set_error(expr->source(), "unable to find called function: " +
                                        mod_->SymbolToName(ident->symbol()));
          return false;
        }

        // We inherit any referenced variables from the callee.
        for (auto* var : callee_func->referenced_module_variables()) {
          set_referenced_from_function_if_needed(var, false);
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
    auto func_sym = expr->func()->As<ast::IdentifierExpression>()->symbol();
    set_error(expr->source(),
              "v-0005: function must be declared before use: '" +
                  mod_->SymbolToName(func_sym) + "'");
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
      set_error(expr->source(), "incorrect number of parameters for " +
                                    mod_->SymbolToName(ident->symbol()));
      return false;
    }

    // The result type must be the same as the type of the parameter.
    auto* param_type = expr->params()[0]->result_type()->UnwrapPtrIfNeeded();
    expr->func()->set_result_type(param_type);
    return true;
  }
  if (ident->intrinsic() == ast::Intrinsic::kAny ||
      ident->intrinsic() == ast::Intrinsic::kAll) {
    expr->func()->set_result_type(mod_->create<ast::type::Bool>());
    return true;
  }
  if (ident->intrinsic() == ast::Intrinsic::kArrayLength) {
    expr->func()->set_result_type(mod_->create<ast::type::U32>());
    return true;
  }
  if (ast::intrinsic::IsFloatClassificationIntrinsic(ident->intrinsic())) {
    if (expr->params().size() != 1) {
      set_error(expr->source(), "incorrect number of parameters for " +
                                    mod_->SymbolToName(ident->symbol()));
      return false;
    }

    auto* bool_type = mod_->create<ast::type::Bool>();

    auto* param_type = expr->params()[0]->result_type()->UnwrapPtrIfNeeded();
    if (auto* vec = param_type->As<ast::type::Vector>()) {
      expr->func()->set_result_type(
          mod_->create<ast::type::Vector>(bool_type, vec->size()));
    } else {
      expr->func()->set_result_type(bool_type);
    }
    return true;
  }
  if (ast::intrinsic::IsTextureIntrinsic(ident->intrinsic())) {
    ast::intrinsic::TextureSignature::Parameters param;

    auto* texture_param = expr->params()[0];
    if (!texture_param->result_type()->UnwrapAll()->Is<ast::type::Texture>()) {
      set_error(expr->source(), "invalid first argument for " +
                                    mod_->SymbolToName(ident->symbol()));
      return false;
    }
    ast::type::Texture* texture =
        texture_param->result_type()->UnwrapAll()->As<ast::type::Texture>();

    bool is_array = ast::type::IsTextureArray(texture->dim());
    bool is_multisampled = texture->Is<ast::type::MultisampledTexture>();
    switch (ident->intrinsic()) {
      case ast::Intrinsic::kTextureDimensions:
        param.idx.texture = param.count++;
        if (expr->params().size() > param.count) {
          param.idx.level = param.count++;
        }
        break;
      case ast::Intrinsic::kTextureNumLayers:
      case ast::Intrinsic::kTextureNumLevels:
      case ast::Intrinsic::kTextureNumSamples:
        param.idx.texture = param.count++;
        break;
      case ast::Intrinsic::kTextureLoad:
        param.idx.texture = param.count++;
        param.idx.coords = param.count++;
        if (is_array) {
          param.idx.array_index = param.count++;
        }
        if (expr->params().size() > param.count) {
          if (is_multisampled) {
            param.idx.sample_index = param.count++;
          } else {
            param.idx.level = param.count++;
          }
        }
        break;
      case ast::Intrinsic::kTextureSample:
        param.idx.texture = param.count++;
        param.idx.sampler = param.count++;
        param.idx.coords = param.count++;
        if (is_array) {
          param.idx.array_index = param.count++;
        }
        if (expr->params().size() > param.count) {
          param.idx.offset = param.count++;
        }
        break;
      case ast::Intrinsic::kTextureSampleBias:
        param.idx.texture = param.count++;
        param.idx.sampler = param.count++;
        param.idx.coords = param.count++;
        if (is_array) {
          param.idx.array_index = param.count++;
        }
        param.idx.bias = param.count++;
        if (expr->params().size() > param.count) {
          param.idx.offset = param.count++;
        }
        break;
      case ast::Intrinsic::kTextureSampleLevel:
        param.idx.texture = param.count++;
        param.idx.sampler = param.count++;
        param.idx.coords = param.count++;
        if (is_array) {
          param.idx.array_index = param.count++;
        }
        param.idx.level = param.count++;
        if (expr->params().size() > param.count) {
          param.idx.offset = param.count++;
        }
        break;
      case ast::Intrinsic::kTextureSampleCompare:
        param.idx.texture = param.count++;
        param.idx.sampler = param.count++;
        param.idx.coords = param.count++;
        if (is_array) {
          param.idx.array_index = param.count++;
        }
        param.idx.depth_ref = param.count++;
        if (expr->params().size() > param.count) {
          param.idx.offset = param.count++;
        }
        break;
      case ast::Intrinsic::kTextureSampleGrad:
        param.idx.texture = param.count++;
        param.idx.sampler = param.count++;
        param.idx.coords = param.count++;
        if (is_array) {
          param.idx.array_index = param.count++;
        }
        param.idx.ddx = param.count++;
        param.idx.ddy = param.count++;
        if (expr->params().size() > param.count) {
          param.idx.offset = param.count++;
        }
        break;
      case ast::Intrinsic::kTextureStore:
        param.idx.texture = param.count++;
        param.idx.coords = param.count++;
        if (is_array) {
          param.idx.array_index = param.count++;
        }
        param.idx.value = param.count++;
        break;
      default:
        set_error(expr->source(),
                  "Internal compiler error: Unreachable intrinsic " +
                      std::to_string(static_cast<int>(ident->intrinsic())));
        return false;
    }

    if (expr->params().size() != param.count) {
      set_error(expr->source(),
                "incorrect number of parameters for " +
                    mod_->SymbolToName(ident->symbol()) + ", got " +
                    std::to_string(expr->params().size()) + " and expected " +
                    std::to_string(param.count));
      return false;
    }

    ident->set_intrinsic_signature(
        std::make_unique<ast::intrinsic::TextureSignature>(param));

    // Set the function return type
    ast::type::Type* return_type = nullptr;
    switch (ident->intrinsic()) {
      case ast::Intrinsic::kTextureDimensions: {
        auto* i32 = mod_->create<ast::type::I32>();
        switch (texture->dim()) {
          default:
            set_error(expr->source(), "invalid texture dimensions");
            break;
          case ast::type::TextureDimension::k1d:
          case ast::type::TextureDimension::k1dArray:
            return_type = i32;
            break;
          case ast::type::TextureDimension::k2d:
          case ast::type::TextureDimension::k2dArray:
            return_type = mod_->create<ast::type::Vector>(i32, 2);
            break;
          case ast::type::TextureDimension::k3d:
          case ast::type::TextureDimension::kCube:
          case ast::type::TextureDimension::kCubeArray:
            return_type = mod_->create<ast::type::Vector>(i32, 3);
            break;
        }
        break;
      }
      case ast::Intrinsic::kTextureNumLayers:
      case ast::Intrinsic::kTextureNumLevels:
      case ast::Intrinsic::kTextureNumSamples:
        return_type = mod_->create<ast::type::I32>();
        break;
      case ast::Intrinsic::kTextureStore:
        return_type = mod_->create<ast::type::Void>();
        break;
      default: {
        if (texture->Is<ast::type::DepthTexture>()) {
          return_type = mod_->create<ast::type::F32>();
        } else {
          ast::type::Type* type = nullptr;
          if (auto* storage = texture->As<ast::type::StorageTexture>()) {
            type = storage->type();
          } else if (auto* sampled = texture->As<ast::type::SampledTexture>()) {
            type = sampled->type();
          } else if (auto* msampled =
                         texture->As<ast::type::MultisampledTexture>()) {
            type = msampled->type();
          } else {
            set_error(expr->source(),
                      "unknown texture type for texture sampling");
            return false;
          }
          return_type = mod_->create<ast::type::Vector>(type, 4);
        }
      }
    }
    expr->func()->set_result_type(return_type);

    return true;
  }
  if (ident->intrinsic() == ast::Intrinsic::kDot) {
    expr->func()->set_result_type(mod_->create<ast::type::F32>());
    return true;
  }
  if (ident->intrinsic() == ast::Intrinsic::kSelect) {
    if (expr->params().size() != 3) {
      set_error(expr->source(), "incorrect number of parameters for " +
                                    mod_->SymbolToName(ident->symbol()) +
                                    " expected 3 got " +
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
    error_ = "unable to find intrinsic " + mod_->SymbolToName(ident->symbol());
    return false;
  }

  if (expr->params().size() != data->param_count) {
    set_error(expr->source(), "incorrect number of parameters for " +
                                  mod_->SymbolToName(ident->symbol()) +
                                  ". Expected " +
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
                    "incorrect type for " +
                        mod_->SymbolToName(ident->symbol()) + ". " +
                        "Requires float or int, scalar or vector values");
          return false;
        }
        break;
      case IntrinsicDataType::kFloatScalarOrVector:
        if (!result_types.back()->is_float_scalar_or_vector()) {
          set_error(expr->source(),
                    "incorrect type for " +
                        mod_->SymbolToName(ident->symbol()) + ". " +
                        "Requires float scalar or float vector values");
          return false;
        }

        break;
      case IntrinsicDataType::kIntScalarOrVector:
        if (!result_types.back()->is_integer_scalar_or_vector()) {
          set_error(expr->source(),
                    "incorrect type for " +
                        mod_->SymbolToName(ident->symbol()) + ". " +
                        "Requires integer scalar or integer vector values");
          return false;
        }
        break;
      case IntrinsicDataType::kFloatVector:
        if (!result_types.back()->is_float_vector()) {
          set_error(expr->source(), "incorrect type for " +
                                        mod_->SymbolToName(ident->symbol()) +
                                        ". " + "Requires float vector values");
          return false;
        }
        if (data->vector_size > 0 &&
            result_types.back()->As<ast::type::Vector>()->size() !=
                data->vector_size) {
          set_error(expr->source(), "incorrect vector size for " +
                                        mod_->SymbolToName(ident->symbol()) +
                                        ". " + "Requires " +
                                        std::to_string(data->vector_size) +
                                        " elements");
          return false;
        }
        break;
      case IntrinsicDataType::kMatrix:
        if (!result_types.back()->Is<ast::type::Matrix>()) {
          set_error(expr->source(), "incorrect type for " +
                                        mod_->SymbolToName(ident->symbol()) +
                                        ". Requires matrix value");
          return false;
        }
        break;
    }
  }

  // Verify all the parameter types match
  for (size_t i = 1; i < data->param_count; ++i) {
    if (result_types[0] != result_types[i]) {
      set_error(expr->source(), "mismatched parameter types for " +
                                    mod_->SymbolToName(ident->symbol()));
      return false;
    }
  }

  // Handle functions which aways return the type, even if a vector is
  // provided.
  if (ident->intrinsic() == ast::Intrinsic::kLength ||
      ident->intrinsic() == ast::Intrinsic::kDistance) {
    expr->func()->set_result_type(
        result_types[0]->is_float_scalar()
            ? result_types[0]
            : result_types[0]->As<ast::type::Vector>()->type());
    return true;
  }
  // The determinant returns the component type of the columns
  if (ident->intrinsic() == ast::Intrinsic::kDeterminant) {
    expr->func()->set_result_type(
        result_types[0]->As<ast::type::Matrix>()->type());
    return true;
  }
  expr->func()->set_result_type(result_types[0]);
  return true;
}

bool TypeDeterminer::DetermineConstructor(ast::ConstructorExpression* expr) {
  if (auto* ty = expr->As<ast::TypeConstructorExpression>()) {
    for (auto* value : ty->values()) {
      if (!DetermineResultType(value)) {
        return false;
      }
    }
    expr->set_result_type(ty->type());
  } else {
    expr->set_result_type(
        expr->As<ast::ScalarConstructorExpression>()->literal()->type());
  }
  return true;
}

bool TypeDeterminer::DetermineIdentifier(ast::IdentifierExpression* expr) {
  auto symbol = expr->symbol();
  ast::Variable* var;
  if (variable_stack_.get(symbol, &var)) {
    // A constant is the type, but a variable is always a pointer so synthesize
    // the pointer around the variable type.
    if (var->is_const()) {
      expr->set_result_type(var->type());
    } else if (var->type()->Is<ast::type::Pointer>()) {
      expr->set_result_type(var->type());
    } else {
      expr->set_result_type(
          mod_->create<ast::type::Pointer>(var->type(), var->storage_class()));
    }

    set_referenced_from_function_if_needed(var, true);
    return true;
  }

  auto iter = symbol_to_function_.find(symbol);
  if (iter != symbol_to_function_.end()) {
    expr->set_result_type(iter->second->return_type());
    return true;
  }

  if (!SetIntrinsicIfNeeded(expr)) {
    set_error(expr->source(),
              "v-0006: identifier must be declared before use: " +
                  mod_->SymbolToName(symbol));
    return false;
  }
  return true;
}

bool TypeDeterminer::SetIntrinsicIfNeeded(ast::IdentifierExpression* ident) {
  auto name = mod_->SymbolToName(ident->symbol());
  if (name == "abs") {
    ident->set_intrinsic(ast::Intrinsic::kAbs);
  } else if (name == "acos") {
    ident->set_intrinsic(ast::Intrinsic::kAcos);
  } else if (name == "all") {
    ident->set_intrinsic(ast::Intrinsic::kAll);
  } else if (name == "any") {
    ident->set_intrinsic(ast::Intrinsic::kAny);
  } else if (name == "arrayLength") {
    ident->set_intrinsic(ast::Intrinsic::kArrayLength);
  } else if (name == "asin") {
    ident->set_intrinsic(ast::Intrinsic::kAsin);
  } else if (name == "atan") {
    ident->set_intrinsic(ast::Intrinsic::kAtan);
  } else if (name == "atan2") {
    ident->set_intrinsic(ast::Intrinsic::kAtan2);
  } else if (name == "ceil") {
    ident->set_intrinsic(ast::Intrinsic::kCeil);
  } else if (name == "clamp") {
    ident->set_intrinsic(ast::Intrinsic::kClamp);
  } else if (name == "cos") {
    ident->set_intrinsic(ast::Intrinsic::kCos);
  } else if (name == "cosh") {
    ident->set_intrinsic(ast::Intrinsic::kCosh);
  } else if (name == "countOneBits") {
    ident->set_intrinsic(ast::Intrinsic::kCountOneBits);
  } else if (name == "cross") {
    ident->set_intrinsic(ast::Intrinsic::kCross);
  } else if (name == "determinant") {
    ident->set_intrinsic(ast::Intrinsic::kDeterminant);
  } else if (name == "distance") {
    ident->set_intrinsic(ast::Intrinsic::kDistance);
  } else if (name == "dot") {
    ident->set_intrinsic(ast::Intrinsic::kDot);
  } else if (name == "dpdx") {
    ident->set_intrinsic(ast::Intrinsic::kDpdx);
  } else if (name == "dpdxCoarse") {
    ident->set_intrinsic(ast::Intrinsic::kDpdxCoarse);
  } else if (name == "dpdxFine") {
    ident->set_intrinsic(ast::Intrinsic::kDpdxFine);
  } else if (name == "dpdy") {
    ident->set_intrinsic(ast::Intrinsic::kDpdy);
  } else if (name == "dpdyCoarse") {
    ident->set_intrinsic(ast::Intrinsic::kDpdyCoarse);
  } else if (name == "dpdyFine") {
    ident->set_intrinsic(ast::Intrinsic::kDpdyFine);
  } else if (name == "exp") {
    ident->set_intrinsic(ast::Intrinsic::kExp);
  } else if (name == "exp2") {
    ident->set_intrinsic(ast::Intrinsic::kExp2);
  } else if (name == "faceForward") {
    ident->set_intrinsic(ast::Intrinsic::kFaceForward);
  } else if (name == "floor") {
    ident->set_intrinsic(ast::Intrinsic::kFloor);
  } else if (name == "fma") {
    ident->set_intrinsic(ast::Intrinsic::kFma);
  } else if (name == "fract") {
    ident->set_intrinsic(ast::Intrinsic::kFract);
  } else if (name == "frexp") {
    ident->set_intrinsic(ast::Intrinsic::kFrexp);
  } else if (name == "fwidth") {
    ident->set_intrinsic(ast::Intrinsic::kFwidth);
  } else if (name == "fwidthCoarse") {
    ident->set_intrinsic(ast::Intrinsic::kFwidthCoarse);
  } else if (name == "fwidthFine") {
    ident->set_intrinsic(ast::Intrinsic::kFwidthFine);
  } else if (name == "inverseSqrt") {
    ident->set_intrinsic(ast::Intrinsic::kInverseSqrt);
  } else if (name == "isFinite") {
    ident->set_intrinsic(ast::Intrinsic::kIsFinite);
  } else if (name == "isInf") {
    ident->set_intrinsic(ast::Intrinsic::kIsInf);
  } else if (name == "isNan") {
    ident->set_intrinsic(ast::Intrinsic::kIsNan);
  } else if (name == "isNormal") {
    ident->set_intrinsic(ast::Intrinsic::kIsNormal);
  } else if (name == "ldexp") {
    ident->set_intrinsic(ast::Intrinsic::kLdexp);
  } else if (name == "length") {
    ident->set_intrinsic(ast::Intrinsic::kLength);
  } else if (name == "log") {
    ident->set_intrinsic(ast::Intrinsic::kLog);
  } else if (name == "log2") {
    ident->set_intrinsic(ast::Intrinsic::kLog2);
  } else if (name == "max") {
    ident->set_intrinsic(ast::Intrinsic::kMax);
  } else if (name == "min") {
    ident->set_intrinsic(ast::Intrinsic::kMin);
  } else if (name == "mix") {
    ident->set_intrinsic(ast::Intrinsic::kMix);
  } else if (name == "modf") {
    ident->set_intrinsic(ast::Intrinsic::kModf);
  } else if (name == "normalize") {
    ident->set_intrinsic(ast::Intrinsic::kNormalize);
  } else if (name == "pow") {
    ident->set_intrinsic(ast::Intrinsic::kPow);
  } else if (name == "reflect") {
    ident->set_intrinsic(ast::Intrinsic::kReflect);
  } else if (name == "reverseBits") {
    ident->set_intrinsic(ast::Intrinsic::kReverseBits);
  } else if (name == "round") {
    ident->set_intrinsic(ast::Intrinsic::kRound);
  } else if (name == "select") {
    ident->set_intrinsic(ast::Intrinsic::kSelect);
  } else if (name == "sign") {
    ident->set_intrinsic(ast::Intrinsic::kSign);
  } else if (name == "sin") {
    ident->set_intrinsic(ast::Intrinsic::kSin);
  } else if (name == "sinh") {
    ident->set_intrinsic(ast::Intrinsic::kSinh);
  } else if (name == "smoothStep") {
    ident->set_intrinsic(ast::Intrinsic::kSmoothStep);
  } else if (name == "sqrt") {
    ident->set_intrinsic(ast::Intrinsic::kSqrt);
  } else if (name == "step") {
    ident->set_intrinsic(ast::Intrinsic::kStep);
  } else if (name == "tan") {
    ident->set_intrinsic(ast::Intrinsic::kTan);
  } else if (name == "tanh") {
    ident->set_intrinsic(ast::Intrinsic::kTanh);
  } else if (name == "textureDimensions") {
    ident->set_intrinsic(ast::Intrinsic::kTextureDimensions);
  } else if (name == "textureNumLayers") {
    ident->set_intrinsic(ast::Intrinsic::kTextureNumLayers);
  } else if (name == "textureNumLevels") {
    ident->set_intrinsic(ast::Intrinsic::kTextureNumLevels);
  } else if (name == "textureNumSamples") {
    ident->set_intrinsic(ast::Intrinsic::kTextureNumSamples);
  } else if (name == "textureLoad") {
    ident->set_intrinsic(ast::Intrinsic::kTextureLoad);
  } else if (name == "textureStore") {
    ident->set_intrinsic(ast::Intrinsic::kTextureStore);
  } else if (name == "textureSample") {
    ident->set_intrinsic(ast::Intrinsic::kTextureSample);
  } else if (name == "textureSampleBias") {
    ident->set_intrinsic(ast::Intrinsic::kTextureSampleBias);
  } else if (name == "textureSampleCompare") {
    ident->set_intrinsic(ast::Intrinsic::kTextureSampleCompare);
  } else if (name == "textureSampleGrad") {
    ident->set_intrinsic(ast::Intrinsic::kTextureSampleGrad);
  } else if (name == "textureSampleLevel") {
    ident->set_intrinsic(ast::Intrinsic::kTextureSampleLevel);
  } else if (name == "trunc") {
    ident->set_intrinsic(ast::Intrinsic::kTrunc);
  } else {
    return false;
  }
  return true;
}

bool TypeDeterminer::DetermineMemberAccessor(
    ast::MemberAccessorExpression* expr) {
  if (!DetermineResultType(expr->structure())) {
    return false;
  }

  auto* res = expr->structure()->result_type();
  auto* data_type = res->UnwrapPtrIfNeeded()->UnwrapIfNeeded();

  ast::type::Type* ret = nullptr;
  if (auto* ty = data_type->As<ast::type::Struct>()) {
    auto* strct = ty->impl();
    auto symbol = expr->member()->symbol();

    for (auto* member : strct->members()) {
      if (member->symbol() == symbol) {
        ret = member->type();
        break;
      }
    }

    if (ret == nullptr) {
      set_error(expr->source(),
                "struct member " + mod_->SymbolToName(symbol) + " not found");
      return false;
    }

    // If we're extracting from a pointer, we return a pointer.
    if (auto* ptr = res->As<ast::type::Pointer>()) {
      ret = mod_->create<ast::type::Pointer>(ret, ptr->storage_class());
    }
  } else if (auto* vec = data_type->As<ast::type::Vector>()) {
    // TODO(dsinclair): Swizzle, record into the identifier experesion

    auto size = mod_->SymbolToName(expr->member()->symbol()).size();
    if (size == 1) {
      // A single element swizzle is just the type of the vector.
      ret = vec->type();
      // If we're extracting from a pointer, we return a pointer.
      if (auto* ptr = res->As<ast::type::Pointer>()) {
        ret = mod_->create<ast::type::Pointer>(ret, ptr->storage_class());
      }
    } else {
      // The vector will have a number of components equal to the length of the
      // swizzle. This assumes the validator will check that the swizzle
      // is correct.
      ret = mod_->create<ast::type::Vector>(vec->type(), size);
    }
  } else {
    set_error(
        expr->source(),
        "v-0007: invalid use of member accessor on a non-vector/non-struct " +
            data_type->type_name());
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
    auto* bool_type = mod_->create<ast::type::Bool>();
    auto* param_type = expr->lhs()->result_type()->UnwrapPtrIfNeeded();
    if (auto* vec = param_type->As<ast::type::Vector>()) {
      expr->set_result_type(
          mod_->create<ast::type::Vector>(bool_type, vec->size()));
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
    auto* lhs_mat = lhs_type->As<ast::type::Matrix>();
    auto* rhs_mat = rhs_type->As<ast::type::Matrix>();
    auto* lhs_vec = lhs_type->As<ast::type::Vector>();
    auto* rhs_vec = rhs_type->As<ast::type::Vector>();
    if (lhs_mat && rhs_mat) {
      expr->set_result_type(mod_->create<ast::type::Matrix>(
          lhs_mat->type(), lhs_mat->rows(), rhs_mat->columns()));
    } else if (lhs_mat && rhs_vec) {
      expr->set_result_type(
          mod_->create<ast::type::Vector>(lhs_mat->type(), lhs_mat->rows()));
    } else if (lhs_vec && rhs_mat) {
      expr->set_result_type(
          mod_->create<ast::type::Vector>(rhs_mat->type(), rhs_mat->columns()));
    } else if (lhs_mat) {
      // matrix * scalar
      expr->set_result_type(lhs_type);
    } else if (rhs_mat) {
      // scalar * matrix
      expr->set_result_type(rhs_type);
    } else if (lhs_vec && rhs_vec) {
      expr->set_result_type(lhs_type);
    } else if (lhs_vec) {
      // Vector * scalar
      expr->set_result_type(lhs_type);
    } else if (rhs_vec) {
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
    ast::type::StorageTexture* tex) {
  if (tex->type() != nullptr) {
    return true;
  }

  switch (tex->image_format()) {
    case ast::type::ImageFormat::kR8Uint:
    case ast::type::ImageFormat::kR16Uint:
    case ast::type::ImageFormat::kRg8Uint:
    case ast::type::ImageFormat::kR32Uint:
    case ast::type::ImageFormat::kRg16Uint:
    case ast::type::ImageFormat::kRgba8Uint:
    case ast::type::ImageFormat::kRg32Uint:
    case ast::type::ImageFormat::kRgba16Uint:
    case ast::type::ImageFormat::kRgba32Uint: {
      tex->set_type(mod_->create<ast::type::U32>());
      return true;
    }

    case ast::type::ImageFormat::kR8Sint:
    case ast::type::ImageFormat::kR16Sint:
    case ast::type::ImageFormat::kRg8Sint:
    case ast::type::ImageFormat::kR32Sint:
    case ast::type::ImageFormat::kRg16Sint:
    case ast::type::ImageFormat::kRgba8Sint:
    case ast::type::ImageFormat::kRg32Sint:
    case ast::type::ImageFormat::kRgba16Sint:
    case ast::type::ImageFormat::kRgba32Sint: {
      tex->set_type(mod_->create<ast::type::I32>());
      return true;
    }

    case ast::type::ImageFormat::kR8Unorm:
    case ast::type::ImageFormat::kRg8Unorm:
    case ast::type::ImageFormat::kRgba8Unorm:
    case ast::type::ImageFormat::kRgba8UnormSrgb:
    case ast::type::ImageFormat::kBgra8Unorm:
    case ast::type::ImageFormat::kBgra8UnormSrgb:
    case ast::type::ImageFormat::kRgb10A2Unorm:
    case ast::type::ImageFormat::kR8Snorm:
    case ast::type::ImageFormat::kRg8Snorm:
    case ast::type::ImageFormat::kRgba8Snorm:
    case ast::type::ImageFormat::kR16Float:
    case ast::type::ImageFormat::kR32Float:
    case ast::type::ImageFormat::kRg16Float:
    case ast::type::ImageFormat::kRg11B10Float:
    case ast::type::ImageFormat::kRg32Float:
    case ast::type::ImageFormat::kRgba16Float:
    case ast::type::ImageFormat::kRgba32Float: {
      tex->set_type(mod_->create<ast::type::F32>());
      return true;
    }

    case ast::type::ImageFormat::kNone:
      break;
  }

  return false;
}

}  // namespace tint
