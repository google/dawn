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
#include "src/program_builder.h"
#include "src/semantic/call.h"
#include "src/semantic/expression.h"
#include "src/semantic/function.h"
#include "src/semantic/intrinsic.h"
#include "src/semantic/member_accessor_expression.h"
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

}  // namespace

TypeDeterminer::TypeDeterminer(ProgramBuilder* builder)
    : builder_(builder), intrinsic_table_(IntrinsicTable::Create()) {}

TypeDeterminer::~TypeDeterminer() = default;

diag::List TypeDeterminer::Run(Program* program) {
  ProgramBuilder builder = program->CloneAsBuilder();
  TypeDeterminer td(&builder);
  if (!td.Determine()) {
    diag::List diagnostics;
    diagnostics.add_error(td.error());
    return diagnostics;
  }
  *program = Program(std::move(builder));
  return {};
}

void TypeDeterminer::set_error(const Source& src, const std::string& msg) {
  error_ = "";
  if (src.range.begin.line > 0) {
    error_ += std::to_string(src.range.begin.line) + ":" +
              std::to_string(src.range.begin.column) + ": ";
  }
  error_ += msg;
}

void TypeDeterminer::set_referenced_from_function_if_needed(VariableInfo* var,
                                                            bool local) {
  if (current_function_ == nullptr) {
    return;
  }
  if (var->storage_class == ast::StorageClass::kNone ||
      var->storage_class == ast::StorageClass::kFunction) {
    return;
  }

  current_function_->referenced_module_vars.Add(var);
  if (local) {
    current_function_->local_referenced_module_vars.Add(var);
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
  info->ancestor_entry_points.Add(ep_sym);

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
  current_function_ = function_infos_.Create<FunctionInfo>(func);
  symbol_to_function_[func->symbol()] = current_function_;
  function_to_info_.emplace(func, current_function_);

  variable_stack_.push_scope();
  for (auto* param : func->params()) {
    variable_stack_.set(param->symbol(), CreateVariableInfo(param));
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
    set_error(stmt->source(),
              "function variable has a non-function storage class");
    return false;
  }

  info->storage_class = ast::StorageClass::kFunction;
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
    variable_stack_.set(v->variable()->symbol(),
                        variable_to_info_.at(v->variable()));
    return DetermineResultType(v->variable()->constructor());
  }

  set_error(stmt->source(), "unknown statement type for type determination: " +
                                builder_->str(stmt));
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
    set_error(expr->source(), "invalid parent type (" +
                                  parent_type->type_name() +
                                  ") in array accessor");
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
  if (!DetermineResultType(call->func())) {
    return false;
  }
  if (!DetermineResultType(call->params())) {
    return false;
  }

  // The expression has to be an identifier as you can't store function pointers
  // but, if it isn't we'll just use the normal result determination to be on
  // the safe side.
  auto* ident = call->func()->As<ast::IdentifierExpression>();
  if (!ident) {
    set_error(call->source(), "call target is not an identifier");
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
        set_error(call->source(), "unable to find called function: " + name);
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
      set_error(call->source(),
                "v-0005: function must be declared before use: '" + name + "'");
      return false;
    }

    auto* function = iter->second;
    function_calls_.emplace(call, function);
  }

  return true;
}

bool TypeDeterminer::DetermineIntrinsicCall(
    ast::CallExpression* call,
    semantic::IntrinsicType intrinsic_type) {
  using Parameter = semantic::Parameter;
  using Parameters = semantic::Parameters;
  using Usage = Parameter::Usage;

  std::vector<type::Type*> arg_tys;
  arg_tys.reserve(call->params().size());
  for (auto* expr : call->params()) {
    arg_tys.emplace_back(TypeOf(expr));
  }

  std::string name = semantic::str(intrinsic_type);

  // TODO(bclayton): Add these to the IntrinsicTable
  if (semantic::IsTextureIntrinsic(intrinsic_type)) {
    Parameters params;

    auto& ty = builder_->ty;

    auto* texture = arg_tys[0]->UnwrapAll()->As<type::Texture>();
    if (!texture) {
      set_error(call->source(), "invalid first argument for " + name);
      return false;
    }

    bool is_array = type::IsTextureArray(texture->dim());
    bool is_multisampled = texture->Is<type::MultisampledTexture>();
    switch (intrinsic_type) {
      case IntrinsicType::kTextureDimensions:
        params.emplace_back(Parameter{texture, Usage::kTexture});
        if (arg_tys.size() > params.size()) {
          params.emplace_back(Parameter{ty.i32(), Usage::kLevel});
        }
        break;
      case IntrinsicType::kTextureNumLayers:
      case IntrinsicType::kTextureNumLevels:
      case IntrinsicType::kTextureNumSamples:
        params.emplace_back(Parameter{texture, Usage::kTexture});
        break;
      case IntrinsicType::kTextureLoad:
        params.emplace_back(Parameter{texture, Usage::kTexture});
        params.emplace_back(Parameter{arg_tys[1], Usage::kCoords});
        if (is_array) {
          params.emplace_back(Parameter{ty.i32(), Usage::kArrayIndex});
        }
        if (arg_tys.size() > params.size()) {
          if (is_multisampled) {
            params.emplace_back(Parameter{ty.i32(), Usage::kSampleIndex});
          } else {
            params.emplace_back(Parameter{ty.i32(), Usage::kLevel});
          }
        }
        break;
      case IntrinsicType::kTextureSample:
        params.emplace_back(Parameter{texture, Usage::kTexture});
        params.emplace_back(Parameter{arg_tys[1], Usage::kSampler});
        params.emplace_back(Parameter{arg_tys[2], Usage::kCoords});
        if (is_array) {
          params.emplace_back(Parameter{ty.i32(), Usage::kArrayIndex});
        }
        if (arg_tys.size() > params.size()) {
          params.emplace_back(
              Parameter{arg_tys[params.size()], Usage::kOffset});
        }
        break;
      case IntrinsicType::kTextureSampleBias:
        params.emplace_back(Parameter{texture, Usage::kTexture});
        params.emplace_back(Parameter{arg_tys[1], Usage::kSampler});
        params.emplace_back(Parameter{arg_tys[2], Usage::kCoords});
        if (is_array) {
          params.emplace_back(Parameter{ty.i32(), Usage::kArrayIndex});
        }
        params.emplace_back(Parameter{ty.f32(), Usage::kBias});
        if (arg_tys.size() > params.size()) {
          params.emplace_back(
              Parameter{arg_tys[params.size()], Usage::kOffset});
        }
        break;
      case IntrinsicType::kTextureSampleLevel:
        params.emplace_back(Parameter{texture, Usage::kTexture});
        params.emplace_back(Parameter{arg_tys[1], Usage::kSampler});
        params.emplace_back(Parameter{arg_tys[2], Usage::kCoords});
        if (is_array) {
          params.emplace_back(Parameter{ty.i32(), Usage::kArrayIndex});
        }
        params.emplace_back(Parameter{ty.i32(), Usage::kLevel});
        if (arg_tys.size() > params.size()) {
          params.emplace_back(
              Parameter{arg_tys[params.size()], Usage::kOffset});
        }
        break;
      case IntrinsicType::kTextureSampleCompare:
        params.emplace_back(Parameter{texture, Usage::kTexture});
        params.emplace_back(Parameter{arg_tys[1], Usage::kSampler});
        params.emplace_back(Parameter{arg_tys[2], Usage::kCoords});
        if (is_array) {
          params.emplace_back(Parameter{ty.i32(), Usage::kArrayIndex});
        }
        params.emplace_back(Parameter{ty.f32(), Usage::kDepthRef});
        if (arg_tys.size() > params.size()) {
          params.emplace_back(
              Parameter{arg_tys[params.size()], Usage::kOffset});
        }
        break;
      case IntrinsicType::kTextureSampleGrad:
        params.emplace_back(Parameter{texture, Usage::kTexture});
        params.emplace_back(Parameter{arg_tys[1], Usage::kSampler});
        params.emplace_back(Parameter{arg_tys[2], Usage::kCoords});
        if (is_array) {
          params.emplace_back(Parameter{ty.i32(), Usage::kArrayIndex});
        }
        params.emplace_back(Parameter{arg_tys[params.size()], Usage::kDdx});
        params.emplace_back(Parameter{arg_tys[params.size()], Usage::kDdy});
        if (arg_tys.size() > params.size()) {
          params.emplace_back(
              Parameter{arg_tys[params.size()], Usage::kOffset});
        }
        break;
      case IntrinsicType::kTextureStore:
        params.emplace_back(Parameter{texture, Usage::kTexture});
        params.emplace_back(Parameter{arg_tys[1], Usage::kCoords});
        if (is_array) {
          params.emplace_back(Parameter{ty.i32(), Usage::kArrayIndex});
        }
        params.emplace_back(Parameter{arg_tys[params.size()], Usage::kValue});
        break;
      default:
        set_error(call->source(),
                  "Internal compiler error: Unreachable intrinsic " + name);
        return false;
    }

    if (arg_tys.size() != params.size()) {
      set_error(call->source(), "incorrect number of arguments for " + name +
                                    ", got " + std::to_string(arg_tys.size()) +
                                    " and expected " +
                                    std::to_string(params.size()));
      return false;
    }

    // Set the function return type
    type::Type* return_type = nullptr;
    switch (intrinsic_type) {
      case IntrinsicType::kTextureDimensions: {
        auto* i32 = builder_->create<type::I32>();
        switch (texture->dim()) {
          default:
            set_error(call->source(), "invalid texture dimensions");
            break;
          case type::TextureDimension::k1d:
          case type::TextureDimension::k1dArray:
            return_type = i32;
            break;
          case type::TextureDimension::k2d:
          case type::TextureDimension::k2dArray:
            return_type = builder_->create<type::Vector>(i32, 2);
            break;
          case type::TextureDimension::k3d:
          case type::TextureDimension::kCube:
          case type::TextureDimension::kCubeArray:
            return_type = builder_->create<type::Vector>(i32, 3);
            break;
        }
        break;
      }
      case IntrinsicType::kTextureNumLayers:
      case IntrinsicType::kTextureNumLevels:
      case IntrinsicType::kTextureNumSamples:
        return_type = builder_->create<type::I32>();
        break;
      case IntrinsicType::kTextureStore:
        return_type = builder_->create<type::Void>();
        break;
      default: {
        if (texture->Is<type::DepthTexture>()) {
          return_type = builder_->create<type::F32>();
        } else {
          type::Type* type = nullptr;
          if (auto* storage = texture->As<type::StorageTexture>()) {
            type = storage->type();
          } else if (auto* sampled = texture->As<type::SampledTexture>()) {
            type = sampled->type();
          } else if (auto* msampled =
                         texture->As<type::MultisampledTexture>()) {
            type = msampled->type();
          } else {
            set_error(call->source(),
                      "unknown texture type for texture sampling");
            return false;
          }
          return_type = builder_->create<type::Vector>(type, 4);
        }
      }
    }

    auto* intrinsic = builder_->create<semantic::Intrinsic>(
        intrinsic_type, return_type, params);
    builder_->Sem().Add(call, builder_->create<semantic::Call>(intrinsic));
    return true;
  }

  auto result = intrinsic_table_->Lookup(*builder_, intrinsic_type, arg_tys);
  if (!result.intrinsic) {
    // Intrinsic lookup failed.
    set_error(call->source(), result.error);

    // TODO(bclayton): https://crbug.com/tint/487
    // The Validator expects intrinsic signature mismatches to still produce
    // type information. The rules for what the Validator expects are rather
    // bespoke. Try to match what the Validator expects. As the Validator's
    // checks on intrinsics is now almost entirely covered by the
    // IntrinsicTable, we should remove the Validator checks on intrinsic
    // signatures and remove these hacks.
    semantic::Parameters parameters;
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
    builder_->Sem().Add(call, builder_->create<semantic::Call>(intrinsic));
    return false;
  }

  builder_->Sem().Add(call, builder_->create<semantic::Call>(result.intrinsic));
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

    set_referenced_from_function_if_needed(var, true);
    return true;
  }

  auto iter = symbol_to_function_.find(symbol);
  if (iter != symbol_to_function_.end()) {
    // Identifier is to a function, which has no type (currently).
    return true;
  }

  std::string name = builder_->Symbols().NameFor(symbol);
  if (MatchIntrinsicType(name) != IntrinsicType::kNone) {
    // Identifier is to an intrinsic function, which has no type (currently).
    return true;
  }

  set_error(expr->source(),
            "v-0006: identifier must be declared before use: " + name);
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
      set_error(expr->source(), "struct member " +
                                    builder_->Symbols().NameFor(symbol) +
                                    " not found");
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
    set_error(
        expr->source(),
        "v-0007: invalid use of member accessor on a non-vector/non-struct " +
            data_type->type_name());
    return false;
  }

  builder_->Sem().Add(
      expr,
      builder_->create<semantic::MemberAccessorExpression>(ret, is_swizzle));

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

  set_error(expr->source(), "Unknown binary expression");
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

void TypeDeterminer::SetType(ast::Expression* expr, type::Type* type) const {
  return builder_->Sem().Add(expr,
                             builder_->create<semantic::Expression>(type));
}

void TypeDeterminer::CreateSemanticNodes() const {
  auto& sem = builder_->Sem();

  for (auto it : variable_to_info_) {
    auto* var = it.first;
    auto* info = it.second;
    sem.Add(var,
            builder_->create<semantic::Variable>(var, info->storage_class));
  }

  auto remap_vars = [&sem](const std::vector<VariableInfo*>& in) {
    std::vector<const semantic::Variable*> out;
    out.reserve(in.size());
    for (auto* info : in) {
      out.emplace_back(sem.Get(info->declaration));
    }
    return out;
  };

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

  for (auto it : function_calls_) {
    auto* call = it.first;
    auto* func_info = it.second;
    auto* sem_func = func_info_to_sem_func.at(func_info);
    builder_->Sem().Add(call, builder_->create<semantic::Call>(sem_func));
  }
}

TypeDeterminer::VariableInfo::VariableInfo(ast::Variable* decl)
    : declaration(decl), storage_class(decl->declared_storage_class()) {}

TypeDeterminer::VariableInfo::~VariableInfo() = default;

TypeDeterminer::FunctionInfo::FunctionInfo(ast::Function* decl)
    : declaration(decl) {}

TypeDeterminer::FunctionInfo::~FunctionInfo() = default;

}  // namespace tint
