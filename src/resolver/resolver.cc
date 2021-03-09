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

#include "src/resolver/resolver.h"

#include <algorithm>
#include <utility>

#include "src/ast/assignment_statement.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/break_statement.h"
#include "src/ast/call_statement.h"
#include "src/ast/continue_statement.h"
#include "src/ast/discard_statement.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/if_statement.h"
#include "src/ast/loop_statement.h"
#include "src/ast/return_statement.h"
#include "src/ast/switch_statement.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/semantic/call.h"
#include "src/semantic/function.h"
#include "src/semantic/member_accessor_expression.h"
#include "src/semantic/statement.h"
#include "src/semantic/variable.h"

namespace tint {
namespace resolver {
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

Resolver::Resolver(ProgramBuilder* builder)
    : builder_(builder), intrinsic_table_(IntrinsicTable::Create()) {}

Resolver::~Resolver() = default;

Resolver::BlockInfo::BlockInfo(Resolver::BlockInfo::Type ty,
                               Resolver::BlockInfo* p)
    : type(ty), parent(p) {}

Resolver::BlockInfo::~BlockInfo() = default;

void Resolver::set_referenced_from_function_if_needed(VariableInfo* var,
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

bool Resolver::Resolve() {
  bool result = ResolveInternal();

  // Even if resolving failed, create all the semantic nodes for information we
  // did generate.
  CreateSemanticNodes();

  return result;
}

bool Resolver::ResolveInternal() {
  for (auto* var : builder_->AST().GlobalVariables()) {
    variable_stack_.set_global(var->symbol(), CreateVariableInfo(var));

    if (var->has_constructor()) {
      if (!Expression(var->constructor())) {
        return false;
      }
    }
  }

  if (!Functions(builder_->AST().Functions())) {
    return false;
  }

  return true;
}

bool Resolver::Functions(const ast::FunctionList& funcs) {
  for (auto* func : funcs) {
    if (!Function(func)) {
      return false;
    }
  }
  return true;
}

bool Resolver::Function(ast::Function* func) {
  auto* func_info = function_infos_.Create<FunctionInfo>(func);

  ScopedAssignment<FunctionInfo*> sa(current_function_, func_info);

  variable_stack_.push_scope();
  for (auto* param : func->params()) {
    variable_stack_.set(param->symbol(), CreateVariableInfo(param));
  }

  if (!BlockStatement(func->body())) {
    return false;
  }
  variable_stack_.pop_scope();

  // Register the function information _after_ processing the statements. This
  // allows us to catch a function calling itself when determining the call
  // information as this function doesn't exist until it's finished.
  symbol_to_function_[func->symbol()] = func_info;
  function_to_info_.emplace(func, func_info);

  return true;
}

bool Resolver::BlockStatement(const ast::BlockStatement* stmt) {
  return BlockScope(BlockInfo::Type::kGeneric,
                    [&] { return Statements(stmt->list()); });
}

bool Resolver::Statements(const ast::StatementList& stmts) {
  for (auto* stmt : stmts) {
    if (auto* decl = stmt->As<ast::VariableDeclStatement>()) {
      if (!VariableDeclStatement(decl)) {
        return false;
      }
    }

    if (!VariableStorageClass(stmt)) {
      return false;
    }

    if (!Statement(stmt)) {
      return false;
    }
  }
  return true;
}

bool Resolver::VariableStorageClass(ast::Statement* stmt) {
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

bool Resolver::Statement(ast::Statement* stmt) {
  auto* sem_statement = builder_->create<semantic::Statement>(stmt);

  ScopedAssignment<semantic::Statement*> sa(current_statement_, sem_statement);

  if (auto* a = stmt->As<ast::AssignmentStatement>()) {
    return Expression(a->lhs()) && Expression(a->rhs());
  }
  if (auto* b = stmt->As<ast::BlockStatement>()) {
    return BlockStatement(b);
  }
  if (stmt->Is<ast::BreakStatement>()) {
    if (!current_block_->FindFirstParent(BlockInfo::Type::kLoop) &&
        !current_block_->FindFirstParent(BlockInfo::Type::kSwitchCase)) {
      diagnostics_.add_error("break statement must be in a loop or switch case",
                             stmt->source());
      return false;
    }
    return true;
  }
  if (auto* c = stmt->As<ast::CallStatement>()) {
    return Expression(c->expr());
  }
  if (auto* c = stmt->As<ast::CaseStatement>()) {
    return CaseStatement(c);
  }
  if (stmt->Is<ast::ContinueStatement>()) {
    // Set if we've hit the first continue statement in our parent loop
    if (auto* loop_block =
            current_block_->FindFirstParent(BlockInfo::Type::kLoop)) {
      if (loop_block->first_continue == size_t(~0)) {
        loop_block->first_continue = loop_block->decls.size();
      }
    } else {
      diagnostics_.add_error("continue statement must be in a loop",
                             stmt->source());
      return false;
    }

    return true;
  }
  if (stmt->Is<ast::DiscardStatement>()) {
    return true;
  }
  if (auto* e = stmt->As<ast::ElseStatement>()) {
    return Expression(e->condition()) && BlockStatement(e->body());
  }
  if (stmt->Is<ast::FallthroughStatement>()) {
    return true;
  }
  if (auto* i = stmt->As<ast::IfStatement>()) {
    return IfStatement(i);
  }
  if (auto* l = stmt->As<ast::LoopStatement>()) {
    // We don't call DetermineBlockStatement on the body and continuing block as
    // these would make their BlockInfo siblings as in the AST, but we want the
    // body BlockInfo to parent the continuing BlockInfo for semantics and
    // validation. Also, we need to set their types differently.
    return BlockScope(BlockInfo::Type::kLoop, [&] {
      if (!Statements(l->body()->list())) {
        return false;
      }

      if (l->has_continuing()) {
        if (!BlockScope(BlockInfo::Type::kLoopContinuing,
                        [&] { return Statements(l->continuing()->list()); })) {
          return false;
        }
      }

      return true;
    });
  }
  if (auto* r = stmt->As<ast::ReturnStatement>()) {
    return Expression(r->value());
  }
  if (auto* s = stmt->As<ast::SwitchStatement>()) {
    if (!Expression(s->condition())) {
      return false;
    }
    for (auto* case_stmt : s->body()) {
      if (!CaseStatement(case_stmt)) {
        return false;
      }
    }
    return true;
  }
  if (auto* v = stmt->As<ast::VariableDeclStatement>()) {
    variable_stack_.set(v->variable()->symbol(),
                        variable_to_info_.at(v->variable()));
    current_block_->decls.push_back(v->variable());
    return Expression(v->variable()->constructor());
  }

  diagnostics_.add_error(
      "unknown statement type for type determination: " + builder_->str(stmt),
      stmt->source());
  return false;
}

bool Resolver::CaseStatement(ast::CaseStatement* stmt) {
  return BlockScope(BlockInfo::Type::kSwitchCase,
                    [&] { return Statements(stmt->body()->list()); });
}

bool Resolver::IfStatement(ast::IfStatement* stmt) {
  if (!Expression(stmt->condition())) {
    return false;
  }

  auto* cond_type = TypeOf(stmt->condition())->UnwrapAll();
  if (cond_type != builder_->ty.bool_()) {
    diagnostics_.add_error("if statement condition must be bool, got " +
                               cond_type->FriendlyName(builder_->Symbols()),
                           stmt->condition()->source());
    return false;
  }

  if (!BlockStatement(stmt->body())) {
    return false;
  }

  for (auto* else_stmt : stmt->else_statements()) {
    if (!Statement(else_stmt)) {
      return false;
    }
  }
  return true;
}

bool Resolver::Expressions(const ast::ExpressionList& list) {
  for (auto* expr : list) {
    if (!Expression(expr)) {
      return false;
    }
  }
  return true;
}

bool Resolver::Expression(ast::Expression* expr) {
  // This is blindly called above, so in some cases the expression won't exist.
  if (!expr) {
    return true;
  }

  if (TypeOf(expr)) {
    return true;  // Already resolved
  }

  if (auto* a = expr->As<ast::ArrayAccessorExpression>()) {
    return ArrayAccessor(a);
  }
  if (auto* b = expr->As<ast::BinaryExpression>()) {
    return Binary(b);
  }
  if (auto* b = expr->As<ast::BitcastExpression>()) {
    return Bitcast(b);
  }
  if (auto* c = expr->As<ast::CallExpression>()) {
    return Call(c);
  }
  if (auto* c = expr->As<ast::ConstructorExpression>()) {
    return Constructor(c);
  }
  if (auto* i = expr->As<ast::IdentifierExpression>()) {
    return Identifier(i);
  }
  if (auto* m = expr->As<ast::MemberAccessorExpression>()) {
    return MemberAccessor(m);
  }
  if (auto* u = expr->As<ast::UnaryOpExpression>()) {
    return UnaryOp(u);
  }

  diagnostics_.add_error("unknown expression for type determination",
                         expr->source());
  return false;
}

bool Resolver::ArrayAccessor(ast::ArrayAccessorExpression* expr) {
  if (!Expression(expr->array())) {
    return false;
  }
  if (!Expression(expr->idx_expr())) {
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
      // will generate a Function storage class variable to store this into.
      ret = builder_->create<type::Pointer>(ret, ast::StorageClass::kFunction);
    }
  }
  SetType(expr, ret);

  return true;
}

bool Resolver::Bitcast(ast::BitcastExpression* expr) {
  if (!Expression(expr->expr())) {
    return false;
  }
  SetType(expr, expr->type());
  return true;
}

bool Resolver::Call(ast::CallExpression* call) {
  if (!Expressions(call->params())) {
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

  auto intrinsic_type = semantic::ParseIntrinsicType(name);
  if (intrinsic_type != IntrinsicType::kNone) {
    if (!IntrinsicCall(call, intrinsic_type)) {
      return false;
    }
  } else {
    if (current_function_) {
      auto callee_func_it = symbol_to_function_.find(ident->symbol());
      if (callee_func_it == symbol_to_function_.end()) {
        if (current_function_->declaration->symbol() == ident->symbol()) {
          diagnostics_.add_error("recursion is not permitted. '" + name +
                                     "' attempted to call itself.",
                                 call->source());
        } else {
          diagnostics_.add_error(
              "v-0006: unable to find called function: " + name,
              call->source());
        }
        return false;
      }
      auto* callee_func = callee_func_it->second;

      // Note: Requires called functions to be resolved first.
      // This is currently guaranteed as functions must be declared before use.
      current_function_->transitive_calls.add(callee_func);
      for (auto* transitive_call : callee_func->transitive_calls) {
        current_function_->transitive_calls.add(transitive_call);
      }

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

bool Resolver::IntrinsicCall(ast::CallExpression* call,
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

bool Resolver::Constructor(ast::ConstructorExpression* expr) {
  if (auto* ty = expr->As<ast::TypeConstructorExpression>()) {
    for (auto* value : ty->values()) {
      if (!Expression(value)) {
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

bool Resolver::Identifier(ast::IdentifierExpression* expr) {
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

    // If identifier is part of a loop continuing block, make sure it doesn't
    // refer to a variable that is bypassed by a continue statement in the
    // loop's body block.
    if (auto* continuing_block =
            current_block_->FindFirstParent(BlockInfo::Type::kLoopContinuing)) {
      auto* loop_block =
          continuing_block->FindFirstParent(BlockInfo::Type::kLoop);
      if (loop_block->first_continue != size_t(~0)) {
        auto& decls = loop_block->decls;
        // If our identifier is in loop_block->decls, make sure its index is
        // less than first_continue
        auto iter =
            std::find_if(decls.begin(), decls.end(),
                         [&symbol](auto* v) { return v->symbol() == symbol; });
        if (iter != decls.end()) {
          auto var_decl_index =
              static_cast<size_t>(std::distance(decls.begin(), iter));
          if (var_decl_index >= loop_block->first_continue) {
            diagnostics_.add_error(
                "continue statement bypasses declaration of '" +
                    builder_->Symbols().NameFor(symbol) +
                    "' in continuing block",
                expr->source());
            return false;
          }
        }
      }
    }

    return true;
  }

  auto iter = symbol_to_function_.find(symbol);
  if (iter != symbol_to_function_.end()) {
    diagnostics_.add_error("missing '(' for function call",
                           expr->source().End());
    return false;
  }

  std::string name = builder_->Symbols().NameFor(symbol);
  if (semantic::ParseIntrinsicType(name) != IntrinsicType::kNone) {
    diagnostics_.add_error("missing '(' for intrinsic call",
                           expr->source().End());
    return false;
  }

  diagnostics_.add_error(
      "v-0006: identifier must be declared before use: " + name,
      expr->source());
  return false;
}

bool Resolver::MemberAccessor(ast::MemberAccessorExpression* expr) {
  if (!Expression(expr->structure())) {
    return false;
  }

  auto* res = TypeOf(expr->structure());
  auto* data_type = res->UnwrapPtrIfNeeded()->UnwrapIfNeeded();

  type::Type* ret = nullptr;
  std::vector<uint32_t> swizzle;

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
    std::string str = builder_->Symbols().NameFor(expr->member()->symbol());
    auto size = str.size();
    swizzle.reserve(str.size());

    for (auto c : str) {
      switch (c) {
        case 'x':
        case 'r':
          swizzle.emplace_back(0);
          break;
        case 'y':
        case 'g':
          swizzle.emplace_back(1);
          break;
        case 'z':
        case 'b':
          swizzle.emplace_back(2);
          break;
        case 'w':
        case 'a':
          swizzle.emplace_back(3);
          break;
        default:
          diagnostics_.add_error(
              "invalid vector swizzle character",
              expr->member()->source().Begin() + swizzle.size());
          return false;
      }
    }

    if (size < 1 || size > 4) {
      diagnostics_.add_error("invalid vector swizzle size",
                             expr->member()->source());
      return false;
    }

    // All characters are valid, check if they're being mixed
    auto is_rgba = [](char c) {
      return c == 'r' || c == 'g' || c == 'b' || c == 'a';
    };
    auto is_xyzw = [](char c) {
      return c == 'x' || c == 'y' || c == 'z' || c == 'w';
    };
    if (!std::all_of(str.begin(), str.end(), is_rgba) &&
        !std::all_of(str.begin(), str.end(), is_xyzw)) {
      diagnostics_.add_error(
          "invalid mixing of vector swizzle characters rgba with xyzw",
          expr->member()->source());
      return false;
    }

    if (size == 1) {
      // A single element swizzle is just the type of the vector.
      ret = vec->type();
      // If we're extracting from a pointer, we return a pointer.
      if (auto* ptr = res->As<type::Pointer>()) {
        ret = builder_->create<type::Pointer>(ret, ptr->storage_class());
      }
    } else {
      // The vector will have a number of components equal to the length of
      // the swizzle. This assumes the validator will check that the swizzle
      // is correct.
      ret = builder_->create<type::Vector>(vec->type(),
                                           static_cast<uint32_t>(size));
    }
  } else {
    diagnostics_.add_error(
        "invalid use of member accessor on a non-vector/non-struct " +
            data_type->type_name(),
        expr->source());
    return false;
  }

  builder_->Sem().Add(expr,
                      builder_->create<semantic::MemberAccessorExpression>(
                          expr, ret, current_statement_, std::move(swizzle)));
  SetType(expr, ret);

  return true;
}

bool Resolver::Binary(ast::BinaryExpression* expr) {
  if (!Expression(expr->lhs()) || !Expression(expr->rhs())) {
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

bool Resolver::UnaryOp(ast::UnaryOpExpression* expr) {
  // Result type matches the parameter type.
  if (!Expression(expr->expr())) {
    return false;
  }

  auto* result_type = TypeOf(expr->expr())->UnwrapPtrIfNeeded();
  SetType(expr, result_type);
  return true;
}

bool Resolver::VariableDeclStatement(const ast::VariableDeclStatement* stmt) {
  auto* ctor = stmt->variable()->constructor();
  if (!ctor) {
    return true;
  }

  if (auto* sce = ctor->As<ast::ScalarConstructorExpression>()) {
    auto* lhs_type = stmt->variable()->type()->UnwrapAliasIfNeeded();
    auto* rhs_type = sce->literal()->type()->UnwrapAliasIfNeeded();

    if (lhs_type != rhs_type) {
      diagnostics_.add_error(
          "constructor expression type does not match variable type",
          stmt->source());
      return false;
    }
  }

  return true;
}

Resolver::VariableInfo* Resolver::CreateVariableInfo(ast::Variable* var) {
  auto* info = variable_infos_.Create(var);
  variable_to_info_.emplace(var, info);
  return info;
}

type::Type* Resolver::TypeOf(ast::Expression* expr) {
  auto it = expr_info_.find(expr);
  if (it != expr_info_.end()) {
    return it->second.type;
  }
  return nullptr;
}

void Resolver::SetType(ast::Expression* expr, type::Type* type) {
  assert(expr_info_.count(expr) == 0);
  expr_info_.emplace(expr, ExpressionInfo{type, current_statement_});
}

void Resolver::CreateSemanticNodes() const {
  auto& sem = builder_->Sem();

  // Collate all the 'ancestor_entry_points' - this is a map of function symbol
  // to all the entry points that transitively call the function.
  std::unordered_map<Symbol, std::vector<Symbol>> ancestor_entry_points;
  for (auto* func : builder_->AST().Functions()) {
    auto it = function_to_info_.find(func);
    if (it == function_to_info_.end()) {
      continue;  // Resolver has likely errored. Process what we can.
    }

    auto* info = it->second;
    if (!func->IsEntryPoint()) {
      continue;
    }
    for (auto* call : info->transitive_calls) {
      auto& vec = ancestor_entry_points[call->declaration->symbol()];
      vec.emplace_back(func->symbol());
    }
  }

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
        ancestor_entry_points[func->symbol()]);
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

template <typename F>
bool Resolver::BlockScope(BlockInfo::Type type, F&& callback) {
  BlockInfo block_info(type, current_block_);
  ScopedAssignment<BlockInfo*> sa(current_block_, &block_info);
  return callback();
}

Resolver::VariableInfo::VariableInfo(ast::Variable* decl)
    : declaration(decl), storage_class(decl->declared_storage_class()) {}

Resolver::VariableInfo::~VariableInfo() = default;

Resolver::FunctionInfo::FunctionInfo(ast::Function* decl) : declaration(decl) {}

Resolver::FunctionInfo::~FunctionInfo() = default;

}  // namespace resolver
}  // namespace tint
