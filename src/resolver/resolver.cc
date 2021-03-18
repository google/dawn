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
#include "src/ast/struct_block_decoration.h"
#include "src/ast/switch_statement.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/semantic/array.h"
#include "src/semantic/call.h"
#include "src/semantic/function.h"
#include "src/semantic/member_accessor_expression.h"
#include "src/semantic/statement.h"
#include "src/semantic/struct.h"
#include "src/semantic/variable.h"
#include "src/type/access_control_type.h"
#include "src/utils/math.h"

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

// Helper function that returns the range union of two source locations. The
// `start` and `end` locations are assumed to refer to the same source file.
Source CombineSourceRange(const Source& start, const Source& end) {
  return Source(Source::Range(start.range.begin, end.range.end),
                start.file_path, start.file_content);
}

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

// https://gpuweb.github.io/gpuweb/wgsl.html#storable-types
bool Resolver::IsStorable(type::Type* type) {
  type = type->UnwrapIfNeeded();
  if (type->is_scalar() || type->Is<type::Vector>() ||
      type->Is<type::Matrix>()) {
    return true;
  }
  if (type::Array* arr = type->As<type::Array>()) {
    return IsStorable(arr->type());
  }
  if (type::Struct* str = type->As<type::Struct>()) {
    for (const auto* member : str->impl()->members()) {
      if (!IsStorable(member->type())) {
        return false;
      }
    }
    return true;
  }
  return false;
}

// https://gpuweb.github.io/gpuweb/wgsl.html#host-shareable-types
bool Resolver::IsHostSharable(type::Type* type) {
  type = type->UnwrapIfNeeded();
  if (type->IsAnyOf<type::I32, type::U32, type::F32>()) {
    return true;
  }
  if (auto* vec = type->As<type::Vector>()) {
    return IsHostSharable(vec->type());
  }
  if (auto* mat = type->As<type::Matrix>()) {
    return IsHostSharable(mat->type());
  }
  if (auto* arr = type->As<type::Array>()) {
    return IsHostSharable(arr->type());
  }
  if (auto* str = type->As<type::Struct>()) {
    for (auto* member : str->impl()->members()) {
      if (!IsHostSharable(member->type())) {
        return false;
      }
    }
    return true;
  }
  return false;
}

bool Resolver::ResolveInternal() {
  for (auto* ty : builder_->Types()) {
    if (auto* str = ty->As<type::Struct>()) {
      if (!Structure(str)) {
        return false;
      }
      continue;
    }
    if (auto* arr = ty->As<type::Array>()) {
      if (!Array(arr)) {
        return false;
      }
      continue;
    }
  }

  for (auto* var : builder_->AST().GlobalVariables()) {
    variable_stack_.set_global(var->symbol(), CreateVariableInfo(var));

    if (var->has_constructor()) {
      if (!Expression(var->constructor())) {
        return false;
      }
    }

    if (!ApplyStorageClassUsageToType(var->declared_storage_class(),
                                      var->type(), var->source())) {
      diagnostics_.add_note("while instantiating variable " +
                                builder_->Symbols().NameFor(var->symbol()),
                            var->source());
      return false;
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
    if (!Statement(stmt)) {
      return false;
    }
  }
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
    current_function_->return_statements.push_back(r);
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
    return VariableDeclStatement(v);
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
  if (auto* type_ctor = expr->As<ast::TypeConstructorExpression>()) {
    for (auto* value : type_ctor->values()) {
      if (!Expression(value)) {
        return false;
      }
    }
    SetType(expr, type_ctor->type());

    // Now that the argument types have been determined, make sure that they
    // obey the constructor type rules laid out in
    // https://gpuweb.github.io/gpuweb/wgsl.html#type-constructor-expr.
    if (auto* vec_type = type_ctor->type()->As<type::Vector>()) {
      return VectorConstructor(vec_type, type_ctor->values());
    }
    if (auto* mat_type = type_ctor->type()->As<type::Matrix>()) {
      return MatrixConstructor(mat_type, type_ctor->values());
    }
    // TODO(crbug.com/tint/634): Validate array constructor
  } else if (auto* scalar_ctor = expr->As<ast::ScalarConstructorExpression>()) {
    SetType(expr, scalar_ctor->literal()->type());
  } else {
    TINT_ICE(diagnostics_) << "unexpected constructor expression type";
  }
  return true;
}

bool Resolver::VectorConstructor(const type::Vector* vec_type,
                                 const ast::ExpressionList& values) {
  type::Type* elem_type = vec_type->type()->UnwrapAll();
  size_t value_cardinality_sum = 0;
  for (auto* value : values) {
    type::Type* value_type = TypeOf(value)->UnwrapAll();
    if (value_type->is_scalar()) {
      if (elem_type != value_type) {
        diagnostics_.add_error(
            "type in vector constructor does not match vector type: "
            "expected '" +
                elem_type->FriendlyName(builder_->Symbols()) + "', found '" +
                value_type->FriendlyName(builder_->Symbols()) + "'",
            value->source());
        return false;
      }

      value_cardinality_sum++;
    } else if (auto* value_vec = value_type->As<type::Vector>()) {
      type::Type* value_elem_type = value_vec->type()->UnwrapAll();
      // A mismatch of vector type parameter T is only an error if multiple
      // arguments are present. A single argument constructor constitutes a
      // type conversion expression.
      // NOTE: A conversion expression from a vec<bool> to any other vecN<T>
      // is disallowed (see
      // https://gpuweb.github.io/gpuweb/wgsl.html#conversion-expr).
      if (elem_type != value_elem_type &&
          (values.size() > 1u || value_vec->is_bool_vector())) {
        diagnostics_.add_error(
            "type in vector constructor does not match vector type: "
            "expected '" +
                elem_type->FriendlyName(builder_->Symbols()) + "', found '" +
                value_elem_type->FriendlyName(builder_->Symbols()) + "'",
            value->source());
        return false;
      }

      value_cardinality_sum += value_vec->size();
    } else {
      // A vector constructor can only accept vectors and scalars.
      diagnostics_.add_error(
          "expected vector or scalar type in vector constructor; found: " +
              value_type->FriendlyName(builder_->Symbols()),
          value->source());
      return false;
    }
  }

  // A correct vector constructor must either be a zero-value expression
  // or the number of components of all constructor arguments must add up
  // to the vector cardinality.
  if (value_cardinality_sum > 0 && value_cardinality_sum != vec_type->size()) {
    if (values.empty()) {
      TINT_ICE(diagnostics_)
          << "constructor arguments expected to be non-empty!";
    }
    const Source& values_start = values[0]->source();
    const Source& values_end = values[values.size() - 1]->source();
    diagnostics_.add_error(
        "attempted to construct '" +
            vec_type->FriendlyName(builder_->Symbols()) + "' with " +
            std::to_string(value_cardinality_sum) + " component(s)",
        CombineSourceRange(values_start, values_end));
    return false;
  }
  return true;
}

bool Resolver::MatrixConstructor(const type::Matrix* matrix_type,
                                 const ast::ExpressionList& values) {
  // Zero Value expression
  if (values.empty()) {
    return true;
  }

  type::Type* elem_type = matrix_type->type()->UnwrapAll();
  if (matrix_type->columns() != values.size()) {
    const Source& values_start = values[0]->source();
    const Source& values_end = values[values.size() - 1]->source();
    diagnostics_.add_error(
        "expected " + std::to_string(matrix_type->columns()) + " '" +
            VectorPretty(matrix_type->rows(), elem_type) + "' arguments in '" +
            matrix_type->FriendlyName(builder_->Symbols()) +
            "' constructor, found " + std::to_string(values.size()),
        CombineSourceRange(values_start, values_end));
    return false;
  }

  for (auto* value : values) {
    type::Type* value_type = TypeOf(value)->UnwrapAll();
    auto* value_vec = value_type->As<type::Vector>();

    if (!value_vec || value_vec->size() != matrix_type->rows() ||
        elem_type != value_vec->type()->UnwrapAll()) {
      diagnostics_.add_error(
          "expected argument type '" +
              VectorPretty(matrix_type->rows(), elem_type) + "' in '" +
              matrix_type->FriendlyName(builder_->Symbols()) +
              "' constructor, found '" +
              value_type->FriendlyName(builder_->Symbols()) + "'",
          value->source());
      return false;
    }
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

bool Resolver::ValidateBinary(ast::BinaryExpression* expr) {
  using Bool = type::Bool;
  using F32 = type::F32;
  using I32 = type::I32;
  using U32 = type::U32;
  using Matrix = type::Matrix;
  using Vector = type::Vector;

  auto* lhs_type = TypeOf(expr->lhs())->UnwrapPtrIfNeeded();
  auto* rhs_type = TypeOf(expr->rhs())->UnwrapPtrIfNeeded();

  auto* lhs_vec = lhs_type->As<Vector>();
  auto* lhs_vec_elem_type = lhs_vec ? lhs_vec->type() : nullptr;
  auto* rhs_vec = rhs_type->As<Vector>();
  auto* rhs_vec_elem_type = rhs_vec ? rhs_vec->type() : nullptr;

  const bool matching_types = lhs_type == rhs_type;
  const bool matching_vec_elem_types = lhs_vec_elem_type && rhs_vec_elem_type &&
                                       (lhs_vec_elem_type == rhs_vec_elem_type);

  // Binary logical expressions
  if (expr->IsLogicalAnd() || expr->IsLogicalOr()) {
    if (matching_types && lhs_type->Is<Bool>()) {
      return true;
    }
  }
  if (expr->IsOr() || expr->IsAnd()) {
    if (matching_types && lhs_type->Is<Bool>()) {
      return true;
    }
    if (matching_types && lhs_vec_elem_type && lhs_vec_elem_type->Is<Bool>()) {
      return true;
    }
  }

  // Arithmetic expressions
  if (expr->IsArithmetic()) {
    // Binary arithmetic expressions over scalars
    if (matching_types && lhs_type->IsAnyOf<I32, F32, U32>()) {
      return true;
    }

    // Binary arithmetic expressions over vectors
    if (matching_types && lhs_vec_elem_type &&
        lhs_vec_elem_type->IsAnyOf<I32, F32, U32>()) {
      return true;
    }
  }

  // Binary arithmetic expressions with mixed scalar, vector, and matrix
  // operands
  if (expr->IsMultiply()) {
    // Multiplication of a vector and a scalar
    if (lhs_type->Is<F32>() && rhs_vec_elem_type &&
        rhs_vec_elem_type->Is<F32>()) {
      return true;
    }
    if (lhs_vec_elem_type && lhs_vec_elem_type->Is<F32>() &&
        rhs_type->Is<F32>()) {
      return true;
    }

    auto* lhs_mat = lhs_type->As<Matrix>();
    auto* lhs_mat_elem_type = lhs_mat ? lhs_mat->type() : nullptr;
    auto* rhs_mat = rhs_type->As<Matrix>();
    auto* rhs_mat_elem_type = rhs_mat ? rhs_mat->type() : nullptr;

    // Multiplication of a matrix and a scalar
    if (lhs_type->Is<F32>() && rhs_mat_elem_type &&
        rhs_mat_elem_type->Is<F32>()) {
      return true;
    }
    if (lhs_mat_elem_type && lhs_mat_elem_type->Is<F32>() &&
        rhs_type->Is<F32>()) {
      return true;
    }

    // Vector times matrix
    if (lhs_vec_elem_type && lhs_vec_elem_type->Is<F32>() &&
        rhs_mat_elem_type && rhs_mat_elem_type->Is<F32>()) {
      return true;
    }

    // Matrix times vector
    if (lhs_mat_elem_type && lhs_mat_elem_type->Is<F32>() &&
        rhs_vec_elem_type && rhs_vec_elem_type->Is<F32>()) {
      return true;
    }

    // Matrix times matrix
    if (lhs_mat_elem_type && lhs_mat_elem_type->Is<F32>() &&
        rhs_mat_elem_type && rhs_mat_elem_type->Is<F32>()) {
      return true;
    }
  }

  // Comparison expressions
  if (expr->IsComparison()) {
    if (matching_types) {
      // Special case for bools: only == and !=
      if (lhs_type->Is<Bool>() && (expr->IsEqual() || expr->IsNotEqual())) {
        return true;
      }

      // For the rest, we can compare i32, u32, and f32
      if (lhs_type->IsAnyOf<I32, U32, F32>()) {
        return true;
      }
    }

    // Same for vectors
    if (matching_vec_elem_types) {
      if (lhs_vec_elem_type->Is<Bool>() &&
          (expr->IsEqual() || expr->IsNotEqual())) {
        return true;
      }

      if (lhs_vec_elem_type->IsAnyOf<I32, U32, F32>()) {
        return true;
      }
    }
  }

  // Binary bitwise operations
  if (expr->IsBitwise()) {
    if (matching_types && lhs_type->IsAnyOf<I32, U32>()) {
      return true;
    }
  }

  // Bit shift expressions
  if (expr->IsBitshift()) {
    // Type validation rules are the same for left or right shift, despite
    // differences in computation rules (i.e. right shift can be arithmetic or
    // logical depending on lhs type).

    if (lhs_type->IsAnyOf<I32, U32>() && rhs_type->Is<U32>()) {
      return true;
    }

    if (lhs_vec_elem_type && lhs_vec_elem_type->IsAnyOf<I32, U32>() &&
        rhs_vec_elem_type && rhs_vec_elem_type->Is<U32>()) {
      return true;
    }
  }

  diagnostics_.add_error(
      "Binary expression operand types are invalid for this operation",
      expr->source());
  return false;
}

bool Resolver::Binary(ast::BinaryExpression* expr) {
  if (!Expression(expr->lhs()) || !Expression(expr->rhs())) {
    return false;
  }

  if (!ValidateBinary(expr)) {
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
  if (auto* ctor = stmt->variable()->constructor()) {
    if (!Expression(ctor)) {
      return false;
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
  }

  auto* var = stmt->variable();

  auto* info = CreateVariableInfo(var);
  variable_to_info_.emplace(var, info);
  variable_stack_.set(var->symbol(), info);
  current_block_->decls.push_back(var);

  if (!var->is_const()) {
    if (info->storage_class != ast::StorageClass::kFunction) {
      if (info->storage_class != ast::StorageClass::kNone) {
        diagnostics_.add_error(
            "function variable has a non-function storage class",
            stmt->source());
        return false;
      }
      info->storage_class = ast::StorageClass::kFunction;
    }
  }

  if (!ApplyStorageClassUsageToType(info->storage_class, var->type(),
                                    var->source())) {
    diagnostics_.add_note("while instantiating variable " +
                              builder_->Symbols().NameFor(var->symbol()),
                          var->source());
    return false;
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
        remap_vars(info->local_referenced_module_vars), info->return_statements,
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

  // Create semantic nodes for all structs
  for (auto it : struct_info_) {
    auto* str = it.first;
    auto* info = it.second;
    builder_->Sem().Add(
        str, builder_->create<semantic::Struct>(
                 str, std::move(info->members), info->align, info->size,
                 info->size_no_padding, info->storage_class_usage));
  }
}

bool Resolver::DefaultAlignAndSize(type::Type* ty,
                                   uint32_t& align,
                                   uint32_t& size) {
  static constexpr uint32_t vector_size[] = {
      /* padding */ 0,
      /* padding */ 0,
      /*vec2*/ 8,
      /*vec3*/ 12,
      /*vec4*/ 16,
  };
  static constexpr uint32_t vector_align[] = {
      /* padding */ 0,
      /* padding */ 0,
      /*vec2*/ 8,
      /*vec3*/ 16,
      /*vec4*/ 16,
  };

  ty = ty->UnwrapAliasIfNeeded();
  if (ty->is_scalar()) {
    // Note: Also captures booleans, but these are not host-sharable.
    align = 4;
    size = 4;
    return true;
  } else if (auto* vec = ty->As<type::Vector>()) {
    if (vec->size() < 2 || vec->size() > 4) {
      TINT_UNREACHABLE(diagnostics_)
          << "Invalid vector size: vec" << vec->size();
      return false;
    }
    align = vector_align[vec->size()];
    size = vector_size[vec->size()];
    return true;
  } else if (auto* mat = ty->As<type::Matrix>()) {
    if (mat->columns() < 2 || mat->columns() > 4 || mat->rows() < 2 ||
        mat->rows() > 4) {
      TINT_UNREACHABLE(diagnostics_)
          << "Invalid matrix size: mat" << mat->columns() << "x" << mat->rows();
      return false;
    }
    align = vector_align[mat->rows()];
    size = vector_align[mat->rows()] * mat->columns();
    return true;
  } else if (auto* s = ty->As<type::Struct>()) {
    if (auto* si = Structure(s)) {
      align = si->align;
      size = si->size;
      return true;
    }
    return false;
  } else if (auto* arr = ty->As<type::Array>()) {
    if (auto* sem = Array(arr)) {
      align = sem->Align();
      size = sem->Size();
      return true;
    }
    return false;
  }
  TINT_UNREACHABLE(diagnostics_) << "Invalid type " << ty->TypeInfo().name;
  return false;
}

const semantic::Array* Resolver::Array(type::Array* arr) {
  if (auto* sem = builder_->Sem().Get(arr)) {
    // Semantic info already constructed for this array type
    return sem;
  }

  // First check the element type is legal
  auto* el_ty = arr->type();
  if (!IsStorable(el_ty)) {
    builder_->Diagnostics().add_error(
        std::string(el_ty->FriendlyName(builder_->Symbols())) +
        " cannot be used as an element type of an array");
    return nullptr;
  }

  auto create_semantic = [&](uint32_t stride) -> semantic::Array* {
    uint32_t el_align = 0;
    uint32_t el_size = 0;
    if (!DefaultAlignAndSize(arr->type(), el_align, el_size)) {
      return nullptr;
    }

    auto align = el_align;
    // WebGPU requires runtime arrays have at least one element, but the AST
    // records an element count of 0 for it.
    auto size = std::max<uint32_t>(arr->size(), 1) * stride;
    auto* sem = builder_->create<semantic::Array>(arr, align, size, stride);
    builder_->Sem().Add(arr, sem);
    return sem;
  };

  // Look for explicit stride via [[stride(n)]] decoration
  for (auto* deco : arr->decorations()) {
    if (auto* stride = deco->As<ast::StrideDecoration>()) {
      return create_semantic(stride->stride());
    }
  }

  // Calculate implicit stride
  uint32_t el_align = 0;
  uint32_t el_size = 0;
  if (!DefaultAlignAndSize(el_ty, el_align, el_size)) {
    return nullptr;
  }

  return create_semantic(utils::RoundUp(el_align, el_size));
}

bool Resolver::ValidateStructure(const type::Struct* st) {
  for (auto* member : st->impl()->members()) {
    if (auto* r = member->type()->UnwrapAll()->As<type::Array>()) {
      if (r->IsRuntimeArray()) {
        if (member != st->impl()->members().back()) {
          diagnostics_.add_error(
              "v-0015",
              "runtime arrays may only appear as the last member of a struct",
              member->source());
          return false;
        }
        if (!st->IsBlockDecorated()) {
          diagnostics_.add_error("v-0015",
                                 "a struct containing a runtime-sized array "
                                 "requires the [[block]] attribute: '" +
                                     builder_->Symbols().NameFor(st->symbol()) +
                                     "'",
                                 member->source());
          return false;
        }

        for (auto* deco : r->decorations()) {
          if (!deco->Is<ast::StrideDecoration>()) {
            diagnostics_.add_error("decoration is not valid for array types",
                                   deco->source());
            return false;
          }
        }
      }
    }

    for (auto* deco : member->decorations()) {
      if (!(deco->Is<ast::BuiltinDecoration>() ||
            deco->Is<ast::LocationDecoration>() ||
            deco->Is<ast::StructMemberOffsetDecoration>() ||
            deco->Is<ast::StructMemberSizeDecoration>() ||
            deco->Is<ast::StructMemberAlignDecoration>())) {
        diagnostics_.add_error("decoration is not valid for structure members",
                               deco->source());
        return false;
      }
    }
  }

  for (auto* deco : st->impl()->decorations()) {
    if (!(deco->Is<ast::StructBlockDecoration>())) {
      diagnostics_.add_error("decoration is not valid for struct declarations",
                             deco->source());
      return false;
    }
  }

  return true;
}

Resolver::StructInfo* Resolver::Structure(type::Struct* str) {
  auto info_it = struct_info_.find(str);
  if (info_it != struct_info_.end()) {
    // StructInfo already resolved for this structure type
    return info_it->second;
  }

  if (!ValidateStructure(str)) {
    return nullptr;
  }

  semantic::StructMemberList sem_members;
  sem_members.reserve(str->impl()->members().size());

  // Calculate the effective size and alignment of each field, and the overall
  // size of the structure.
  // For size, use the size attribute if provided, otherwise use the default
  // size for the type.
  // For alignment, use the alignment attribute if provided, otherwise use the
  // default alignment for the member type.
  // Diagnostic errors are raised if a basic rule is violated.
  // Validation of storage-class rules requires analysing the actual variable
  // usage of the structure, and so is performed as part of the variable
  // validation.
  // TODO(crbug.com/tint/628): Actually implement storage-class validation.
  uint32_t struct_size = 0;
  uint32_t struct_align = 1;

  for (auto* member : str->impl()->members()) {
    // First check the member type is legal
    if (!IsStorable(member->type())) {
      builder_->Diagnostics().add_error(
          std::string(member->type()->FriendlyName(builder_->Symbols())) +
          " cannot be used as the type of a structure member");
      return nullptr;
    }

    uint32_t offset = struct_size;
    uint32_t align = 0;
    uint32_t size = 0;
    if (!DefaultAlignAndSize(member->type(), align, size)) {
      return nullptr;
    }

    bool has_offset_deco = false;
    bool has_align_deco = false;
    bool has_size_deco = false;
    for (auto* deco : member->decorations()) {
      if (auto* o = deco->As<ast::StructMemberOffsetDecoration>()) {
        // Offset decorations are not part of the WGSL spec, but are emitted by
        // the SPIR-V reader.
        if (o->offset() < struct_size) {
          diagnostics_.add_error("offsets must be in ascending order",
                                 o->source());
          return nullptr;
        }
        offset = o->offset();
        align = 1;
        has_offset_deco = true;
      } else if (auto* a = deco->As<ast::StructMemberAlignDecoration>()) {
        if (a->align() <= 0 || !utils::IsPowerOfTwo(a->align())) {
          diagnostics_.add_error(
              "align value must be a positive, power-of-two integer",
              a->source());
          return nullptr;
        }
        align = a->align();
        has_align_deco = true;
      } else if (auto* s = deco->As<ast::StructMemberSizeDecoration>()) {
        if (s->size() < size) {
          diagnostics_.add_error(
              "size must be at least as big as the type's size (" +
                  std::to_string(size) + ")",
              s->source());
          return nullptr;
        }
        size = s->size();
        has_size_deco = true;
      }
    }

    if (has_offset_deco && (has_align_deco || has_size_deco)) {
      diagnostics_.add_error(
          "offset decorations cannot be used with align or size decorations",
          member->source());
      return nullptr;
    }

    offset = utils::RoundUp(align, offset);

    auto* sem_member =
        builder_->create<semantic::StructMember>(member, offset, align, size);
    builder_->Sem().Add(member, sem_member);
    sem_members.emplace_back(sem_member);

    struct_size = offset + size;
    struct_align = std::max(struct_align, align);
  }

  auto size_no_padding = struct_size;
  struct_size = utils::RoundUp(struct_align, struct_size);

  auto* info = struct_infos_.Create();
  info->members = std::move(sem_members);
  info->align = struct_align;
  info->size = struct_size;
  info->size_no_padding = size_no_padding;
  struct_info_.emplace(str, info);
  return info;
}

bool Resolver::ApplyStorageClassUsageToType(ast::StorageClass sc,
                                            type::Type* ty,
                                            Source usage) {
  ty = ty->UnwrapIfNeeded();

  if (auto* str = ty->As<type::Struct>()) {
    auto* info = Structure(str);
    if (!info) {
      return false;
    }
    if (info->storage_class_usage.count(sc)) {
      return true;  // Already applied
    }
    info->storage_class_usage.emplace(sc);
    for (auto* member : str->impl()->members()) {
      if (!ApplyStorageClassUsageToType(sc, member->type(), usage)) {
        std::stringstream err;
        err << "while analysing structure member "
            << str->FriendlyName(builder_->Symbols()) << "."
            << builder_->Symbols().NameFor(member->symbol());
        diagnostics_.add_note(err.str(), member->source());
        return false;
      }
    }
    return true;
  }

  if (auto* arr = ty->As<type::Array>()) {
    return ApplyStorageClassUsageToType(sc, arr->type(), usage);
  }

  if (ast::IsHostSharable(sc) && !IsHostSharable(ty)) {
    std::stringstream err;
    err << "Type '" << ty->FriendlyName(builder_->Symbols())
        << "' cannot be used in storage class '" << sc
        << "' as it is non-host-sharable";
    diagnostics_.add_error(err.str(), usage);
    return false;
  }

  return true;
}

template <typename F>
bool Resolver::BlockScope(BlockInfo::Type type, F&& callback) {
  BlockInfo block_info(type, current_block_);
  ScopedAssignment<BlockInfo*> sa(current_block_, &block_info);
  return callback();
}

std::string Resolver::VectorPretty(uint32_t size, type::Type* element_type) {
  type::Vector vec_type(element_type, size);
  return vec_type.FriendlyName(builder_->Symbols());
}

Resolver::VariableInfo::VariableInfo(ast::Variable* decl)
    : declaration(decl), storage_class(decl->declared_storage_class()) {}

Resolver::VariableInfo::~VariableInfo() = default;

Resolver::FunctionInfo::FunctionInfo(ast::Function* decl) : declaration(decl) {}
Resolver::FunctionInfo::~FunctionInfo() = default;

Resolver::StructInfo::StructInfo() = default;
Resolver::StructInfo::~StructInfo() = default;

}  // namespace resolver
}  // namespace tint
